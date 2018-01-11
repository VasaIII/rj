#include <pwd.h>

#include <unistd.h> // getpass() alarm()
#include <sys/stat.h>
#include <fcntl.h>

#include <poll.h> // pollfd
#include <sys/types.h>
#include <sys/uio.h>

#include <termios.h> // ioctl
#include <sys/stropts.h> // pooling

#define FTP
#include "modules/common.h"
#include "modules/nethelper.h"
#include "modules/protocols/ftp.h"
#undef FTP


#define SEND_STREAM_BUFSIZE     200000
#define PRINTOUT_BUFSIZE        1000

FILE *fp, *fptrace;

int STOR_BUFSIZE_set = SEND_STREAM_BUFSIZE; // for cello server 5000
char global_buffer_send[SEND_STREAM_BUFSIZE]; // ??? prebacit u memoriju tako da netreban alociravat bezveze array

int FLAG_ANALYSE_DOWNLOAD_FINISH;

int silent_printout_mode = 0; // should or not show ftp command sequence

char global_buffer_analyse[1000];

// ***********************************************************************************
//
//
//
// ***********************************************************************************
void FTP_flushCmds(char *remotehost_ip, char(*Cmds)[][MINIBUF]) {
	char *pCmds;

	// -=-=--=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-
	// Standart and customs that can be used in cmd sequence:
	//
	// PASV       - lasts only for one transfer, e.g. one socket communication channel is opened
	//              and should be closed after one data transfer (STOR) has finished.
	//            - moguce je da bez PASV mi nece da prebacuje fileove dobro sa STOR, a kazu da je to
	//              zbog: passive mode for firewalls (use if PORT creates empty file)
	// PORT       - jos nisan isproba ali je pisalo da kad mu posaljen PORT da server
	//              uspostavlja komunikaciju na moj port i to obicno port 20 ?
	// RETR       - download (from data communication channel)
	// preSTOR    - CUSTOM function: if local file name differs than remote file
	//              name to be stored (e.g. rename file), with this command different
	//              local file name can be defined, while in STOR original remote file name.
	//              FORMAT: preSTOR file_name
	//                      STOR folder/file_name\r\n
	// STOR       - upload (on data communication channel)
	// TYPE       - I (binary - .zip ...)
	//              A (ascii - .txt ...)
	//
	// Special purpose customs:
	//
	// analyse&download&finish - CUSTOM function: takes over ftp session and forces thread
	//              to remain active so that it can execute multiple RETR one after
	//              other in sequence on same socket (so that there is no need for creating
	//              new socket for each transfer).
	//              It should come after LIST so that result can be analysed and downloaded.
	//
	// download&finish - CUSTOM function: takes over multi RETR comands after one PORT.
	//              It comes before PORT, RETR /folder/filename1, PORT, RETR /folder/filename2 ...
	// -=-=--=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=---=-=-

	struct servent *FTP_service_info;
	int FTP_connection_sock_sd;
	int PASV_sock_sd;

	int PASV_server_addr[6]; // (159,107,237,228,129,252)
	int PASV_server_addr_active = 0;
	int PASV_server_port;
	char PASV_server_straddr[10];

	struct sockaddr_in hostAddr;
	int port = 35689; // bilo sta;

	char STOR_local_file[100] = ""; // to transfer filename from preSTOR to STOR

	char buffer[500];
	char *pBuff, *pBuff2;
	int count, count1, count2, count3;
	int error_code;
	struct sockaddr_in localhostAddr;
	struct sockaddr_in remotehostAddr;

	char first_hello_from_server[PRINTOUT_BUFSIZE];

	// in.ftpd(1M) is the Internet File Transfer Protocol (FTP) server process
	if ((FTP_service_info = getservbyname(
			"ftp", "tcp")/* in /etc/nsswitch.conf file getservbyname()
						 and getservbyport() functions sequentially search from the
						 beginning of the file until a matching protocol name or port number is found. */
		) == NULL)
		ErrorTraceHandle(0, "FTP_flushCmds() - getservbyname() error.\n");
	else
		ErrorTraceHandle(2, "FTP_flushCmds() FTP TCP service port <%d,%d>", FTP_service_info->s_port, ntohs(FTP_service_info->s_port));

	NetCS_SocketConfigHost(NULL, 0, &localhostAddr);
#ifdef _SOLARIS_
	NetCS_SocketConfigHost(remotehost_ip, FTP_service_info->s_port, &remotehostAddr);
#else
	// LINUX - port value shows 5376 instead 21 without usage of ntohs()
	NetCS_SocketConfigHost(remotehost_ip, ntohs(FTP_service_info->s_port), &remotehostAddr);
#endif
	if ((FTP_connection_sock_sd = NetCS_ClientSocketConnect(&remotehostAddr, &localhostAddr, 1)) != 0)
		{
		ErrorTraceHandle(
				2,
				"Connection established to FTP server port <%d> (my socket <%d>).\n",
				FTP_service_info->s_port, FTP_connection_sock_sd);
		NetCS_ServerPortReceiveAndStoreBuffer(FTP_connection_sock_sd, 1, 1, 0, NETCS_ONE_SET, 0); // receive first hello from server

		strcpy(first_hello_from_server, NetCS_DataSet[NETCS_ONE_SET].transport_receive.buffer[0].data);

		FLAG_ANALYSE_DOWNLOAD_FINISH = 0;

		count = 0;
		while (1) {
			ErrorTraceHandle(
					2,
					"\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

			// ########################
			// PRE-ENTER OF FTP COMMAND ... for debugg and trace
			// ########################

			if (!silent_printout_mode) {
				if (!strncmp((*Cmds)[count], "PASS", 4)) {
					ErrorTraceHandle(1, "\nFTP<%s>: PASSWORD *\n",
							remotehost_ip);
					ErrorTraceHandle(2, "\nFTP(%d)<%s>: PASS *\n", count,
							remotehost_ip);
				} else {
					if (!strncmp((*Cmds)[count], "RETR", 4))
						ErrorTraceHandle(1, "\nFTP<%s>: GET %s", remotehost_ip,
								(*Cmds)[count] + strlen("RETR "));
					else if (!strncmp((*Cmds)[count], "STOR", 4))
						ErrorTraceHandle(1, "\nFTP<%s>: UPLOAD FILE %s",
								remotehost_ip, (*Cmds)[count] + strlen("STOR "));
					else if (!strncmp((*Cmds)[count], "DELE", 4))
						ErrorTraceHandle(1, "\nFTP<%s>: DELETE FILE %s",
								remotehost_ip, (*Cmds)[count] + strlen("DELE "));
					//else if (!strncmp((*Cmds)[count], "USER", 4))
					//  ErrorTraceHandle(1, "\nFTP<%s>: %s", remotehost_ip, (*Cmds)[count]);
					else if (!strncmp((*Cmds)[count], "QUIT", 4))
						ErrorTraceHandle(1, "\nFTP<%s>: TRANSFER FINISHED !\n",
								remotehost_ip);

					ErrorTraceHandle(2, "\nFTP(%d)<%s>: %s", count,
							remotehost_ip, (*Cmds)[count]);
				}
			}

			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER           PASS
			// @@@@@@@@@@@@@@@@@@@@@@@@
			if (!strncmp((*Cmds)[count], "PASS", 4)) {
				//if (strstr(first_hello_from_server, "Cello FTP Server") != NULL)
				//sprintf(buffer, "PASS x\r\n", User.passwd); // cello server has password "x"
				//sprintf(buffer, "PASS 001\r\n", User.passwd); // cello server has password "001"
				//else
				sprintf(buffer, "PASS %s\r\n", User.passwd);

				pCmds = &buffer[0];

				ErrorTraceHandle(2, "FTP(OUT)>> PASS *\n");
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER           USER
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else if (!strncmp((*Cmds)[count], "USER", 4)) {
				sprintf(buffer, "USER %s\r\n", User.name);
				pCmds = &buffer[0];
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER           PORT
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else if (!strncmp((*Cmds)[count], "PORT", 4))
			// Specifies my IP:port to which the peer server should connect for the next file transfer.
			// Peer server returnes his IP:port from which it will connect to my socket IP:port (given in this PORT command).
			// This is interpreted as IP address a1.a2.a3.a4, port p1*256+p2
			{
				// If PORT command, we need to fill our IP:port
				port++; // increase port number in each use of PORT, because ports needs some time to shutdown

				// ndd /dev/tcp tcp_close_wait_interval
				// ndd -set /dev/tcp tcp_close_wait_interval 60000

				// adjust port if this one is busy and send socket

				NetCS_SocketConfigHost(NULL, port, &hostAddr);
				if ((port = NetCS_ServerCreateOnPort(&hostAddr, 1, 0, NETCS_ONE_SET)) == 0) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(0, "FTP_flushCmds() - NetCS_ServerCreateOnPort() error.\n");
				}

				sprintf(buffer, "PORT %d,%d,%d,%d,%d,%d\r\n", User.IP[0],
						User.IP[1], User.IP[2], User.IP[3], (port & 0xff00)
								>> 8, port & 0xff);
				pCmds = &buffer[0];

				// create thread, which will create new socket on our new created port and
				// preferably to accept files there for ftp's RETR command (receive - download)
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER           QUIT
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else if (!strncmp((*Cmds)[count], "QUIT", 4)) {
				if (NetCS_StreamThread_ID) // if thread used
				{
					pthread_join(NetCS_StreamThread_ID, NULL);
					NetCS_StreamThread_ID = 0;
				}
				ErrorTraceHandle(2, "All threads finished (QUIT).\n");
				sprintf(buffer, "QUIT\r\n");
				pCmds = &buffer[0];
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER               analyse&download&finish
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else if (!strcmp((*Cmds)[count], "analyse&download&finish")) {
				long no_write, no_write_total;

				if (NetCS_StreamThread_ID) // if thread used
				{
					pthread_join(NetCS_StreamThread_ID, NULL);
					NetCS_StreamThread_ID = 0;
				}
				// wait all threads to finish so that latest stream data can be fetched from latest thread.
				ErrorTraceHandle(2, "\nAll threads finished (start to analyse&download&finish).\n\n");
				FLAG_ANALYSE_DOWNLOAD_FINISH = 1; // from now until end of ftp session it takes over.

				ErrorTraceHandle(1, "Result will be stored in folder: <%s>\n", "./output"); // drwxr-xr-x
				mkdir("./output", (S_IRWXU | S_IRWXG | S_IRWXO));

				// Since by default after cmd PORT, memory pages are used and cmd LIST is also preceeded by cmd PORT,
				// here I will copy memory pages to global buffer to make easyer analysis of printout (listing of recursive folders).
				// (output should fit in global_buffer_analyse[])

				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=
				//
				// Read from stored pages in NetCS_ServerPortReceiveAndStoreStream()
				//
				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=

				nextPage = firstPage; // set to first page

				no_write_total = 0;

				if (nextPage->p_currentPage == NULL) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(0, "There is no data to download ! Stream Empty.\n");
				} else {
					while (1) {
						no_write = 0;
						pBuff = nextPage->p_currentPage;

						if (nextPage->p_currentPage == NULL) {
							NetCS_SocketClose(FTP_connection_sock_sd);
							if (PASV_server_addr_active)
								NetCS_SocketClose(PASV_sock_sd);

							ErrorTraceHandle(0, "FTP_flushCmds() - Mismatch in memory page data.\n");
						}

						while (no_write != nextPage->size_currentPage) // copy file to buffer array
						{
							global_buffer_analyse[no_write_total + no_write]
									= *pBuff;
							pBuff++;
							no_write++;
						}

						no_write_total = no_write_total + no_write;

						ErrorTraceHandle(2, "In global_buffer_analyse[] %d bytes are copied.\n", no_write_total);

						if (nextPage->p_nextPage == NULL)
							break;
						nextPage = nextPage->p_nextPage;
					}
				}

				global_buffer_analyse[no_write_total] = '\0';

				ErrorTraceHandle(2, "Finished with copying in global_buffer_analyse[].\n");

				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=

				if (!Analyse_GlobalBuffer_with_ls_Ra(Cmds)) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(0, "analyse&download&finish - FTP_flushCmds() - Failed to analyse ftp buffer.\n");
				}
				count = count + 2; // skip Cmds both entries for analyse&download&finish string and for folder name (inserted only to be transfered in FTP_flushCmds())
				continue; // skip this step in loop aswell and continue to execute PORT/RETR Cmds inserted by Analyse_GlobalBuffer_with_ls_Ra()
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER               analyse&download&finish
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else if (!strncmp((*Cmds)[count], "preSTOR", 6))
			// Used only before STOR if file name to be stored differs from local file name, e.g. renameing
			// storing file.
			// FORMAT: preSTOR file_name
			{
				strcpy(STOR_local_file, GetWord((*Cmds)[count], 0, 0, 1));
				count = count + 1; // skip to next cmd string
				continue; // skip this step in loop
			}
			// @@@@@@@@@@@@@@@@@@@@@@@@
			// PRE-ENTER               all
			// @@@@@@@@@@@@@@@@@@@@@@@@
			else {
				pCmds = (*Cmds)[count];
			}

			ErrorTraceHandle(2, "FTP(OUT)>> %s", pCmds);

			// #####################
			// EXECUTION FTP COMMAND
			// #####################
			if (send(FTP_connection_sock_sd, pCmds, strlen(pCmds), 0) == -1) {
				NetCS_SocketClose(FTP_connection_sock_sd);
				if (PASV_server_addr_active)
					NetCS_SocketClose(PASV_sock_sd);

				ErrorTraceHandle(0, "FTP_flushCmds() - send() error.\n");
			}

			// #############################
			// POST-EXECUTION OF FTP COMMAND
			// #############################

			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           QUIT
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			//if (strncmp((*Cmds)[count], "QUIT", 4)) // if not QUIT
			NetCS_ServerPortReceiveAndStoreBuffer(FTP_connection_sock_sd, 1, 1, 0, NETCS_ONE_SET, 0);

			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           LIST
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// if there is need for thread_if to finish (created from PORT before)
			if ((!strncmp((*Cmds)[count], "LIST", 4)) ||
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           RETR
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					(!strncmp((*Cmds)[count], "RETR", 4))) {
				// Wait until therad_id (from PORT) finishes with streaming of data
				// e.g. receiveing of file in case of RETR or receiveing of listing in case of LIST.
				ErrorTraceHandle(2,
						"Waiting until Comand executes (waiting thread) ... ");
				if (NetCS_StreamThread_ID) // if thread used
				{
					pthread_join(NetCS_StreamThread_ID, NULL);
					NetCS_StreamThread_ID = 0;
				}
				ErrorTraceHandle(2, "... all threads EXECUTED !\n");
			}

			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           RETR
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			ErrorTraceHandle(2, "Check for RETR.\n");
			if (!strncmp((*Cmds)[count], "RETR", 4))
			// After command has been sent over ftp, port that I have created has been accepting
			// file from peer ftp server and stored into pages, and now i need to create local file and copy this pages to it.
			//
			// RETR can only store files to its folder locations, it can't make folders that don't exist, i need to create it
			//
			// this is post analysis, so there is no longer need for quotes.
			{
				char c;
				long no_write, no_write_total;
				char RETR_remote_folder[500];
				char RETR_remote_filename[100];
				char RETR_local_folder_filename[500];
				char Cmds_tmp[MINIBUF];
				char Cmds_tmp2[MINIBUF];

				// since I added full path for RETR file-name, now I need to remove it (folder_name) because
				// for creating of local files only relative folder (subfolders) are needed (folder_name+subfolders and file)
				if (FLAG_ANALYSE_DOWNLOAD_FINISH) {
					char *pstart;
					// in case of FLAG_ANALYSE_DOWNLOAD_FINISH, (*Cmds)[6] has stored folder name in any form (with or withour / on end)
					// QUOTES
					strcpy(Cmds_tmp, (*Cmds)[6]);
					strcpy(Cmds_tmp2, (*Cmds)[count]);

					ErrorTraceHandle(
							2,
							"analyse&download&finish - Folder name on remote  : <%s>\n",
							Cmds_tmp);
					ErrorTraceHandle(
							2,
							"analyse&download&finish - Command RETR to analyse: <%s>\n",
							Cmds_tmp2);

					if ((pstart = strstr(Cmds_tmp2, Cmds_tmp)) != NULL) // if finded (remote folder and analysed result)
					{
						strcpy(Cmds_tmp, pstart + strlen(Cmds_tmp)); // find start of folder in string and add its length
						Cmds_tmp[strlen(Cmds_tmp) - 2] = '\0'; // cut \r\n from analyse&download&finish string
					} else {
						NetCS_SocketClose(FTP_connection_sock_sd);
						if (PASV_server_addr_active)
							NetCS_SocketClose(PASV_sock_sd);
						ErrorTraceHandle(0,
								"FTP_flushCmds() - 1 RETR syntax fault !\n");
					}
					ErrorTraceHandle(
							2,
							"analyse&download&finish - Result of adopting RETR: <%s>\n",
							Cmds_tmp);
				} else { // if there are quotes, remove them
					strcpy(Cmds_tmp2, GetWord((*Cmds)[count], 0, 0, 1));
					if (Cmds_tmp2[0] == '\"') {
						strcpy(Cmds_tmp, &Cmds_tmp2[1]); // shifted 1
						if (Cmds_tmp2[strlen(Cmds_tmp2) - 1] != '\"') {
							NetCS_SocketClose(FTP_connection_sock_sd);
							if (PASV_server_addr_active)
								NetCS_SocketClose(PASV_sock_sd);
							ErrorTraceHandle(0,
									"FTP_flushCmds() - 2 RETR syntax fault !\n");
						}
						Cmds_tmp[strlen(Cmds_tmp) - 1] = '\0';
					} else
						strcpy(Cmds_tmp, Cmds_tmp2);
				}
				// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
				// OUTPUT: form this point, Cmds_tmp should contain all relative subfolders (to download folder)
				//         without any quotes and \r \n, or just filename
				// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
				ErrorTraceHandle(2, "To ANALYSE: <%s>\n", Cmds_tmp);

				{
					char selected_format = 0;

					pBuff = &Cmds_tmp[0];

					if (!strncmp(Cmds_tmp, "/", 1)) {
						ErrorTraceHandle(2,
								"Recognised format: /folder/filename\n");
						selected_format = 1; // Format: /folder/umdrv_swb.zip
					} else if (!strncmp(Cmds_tmp, "./", 2)) {
						ErrorTraceHandle(2,
								"Recognised format: ./folder/filename\n");
						selected_format = 2; // Format: ./folder/umdrv_swb.zip
						pBuff = pBuff + 1; // skip dot
					} else {
						ErrorTraceHandle(2,
								"Recognised format: RETR filename\n");
						selected_format = 3; // Format: umdrv_swb.zip
					}

					count1 = 0;
					count2 = 0;

					while (*pBuff != '\0') {
						RETR_remote_folder[count1] = *pBuff; // format /x/y
						if (*pBuff == '/') // search for last slash (/)
						{
							count2 = 0;
							count3 = count1; // last occurence of /
						} else {
							RETR_remote_filename[count2] = *pBuff;
							count2++;
							RETR_remote_filename[count2] = '\0';
						}
						count1++;
						RETR_remote_folder[count1] = '\0';
						pBuff++;
					}

					if ((RETR_remote_folder[0] == '/') && (strlen(
							RETR_remote_folder) == 1)) //
					{
						ErrorTraceHandle(2,
								"RE-Recognised format: RETR /filename\n");
						selected_format == 3; // only filename
					}

					if (selected_format == 3)
						RETR_remote_folder[0] = '\0';
					else {
						RETR_remote_folder[count3] = '\0'; // cut it on last occurence of /
						ErrorTraceHandle(2, "Remote folder: %s\n",
								RETR_remote_folder);
					}

					RETR_remote_filename[count2] = '\0';
					ErrorTraceHandle(2, "Remote file  : %s\n",
							RETR_remote_filename);

					// QUOTES
					strcpy(RETR_local_folder_filename, ".");
					strcat(RETR_local_folder_filename, "/output"); // add local folder to store in ...
					if (selected_format != 3)
						strcat(RETR_local_folder_filename, RETR_remote_folder);
					strcat(RETR_local_folder_filename, "/");
					strcat(RETR_local_folder_filename, RETR_remote_filename);

					ErrorTraceHandle(2, "Redirecting to local file: %s\n",
							RETR_local_folder_filename);
				}

				if ((fp = fopen(RETR_local_folder_filename, "w")) == NULL) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(0,
							"FTP_flushCmds() - local open file error \n");
				} else {
					ErrorTraceHandle(2, "Local file <%s> created.\n",
							RETR_local_folder_filename);
				}

				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=
				//
				// Read from stored pages in ReceiveFromMySocket_stream()
				//
				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=

				nextPage = firstPage; // set to first page

				no_write_total = 0;

				if (nextPage->p_currentPage == NULL) {
					// if size of file is 0 then stream will be empty !
					//NetCS_SocketClose(FTP_connection_sock_sd);
					//if (PASV_server_addr_active) NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(2,
							"There is no data to download ! Stream Empty (file size is probably 0).\n");
				} else {
					while (1) {
						no_write = 0;
						pBuff = nextPage->p_currentPage;

						if (nextPage->p_currentPage == NULL) {
							NetCS_SocketClose(FTP_connection_sock_sd);
							if (PASV_server_addr_active)
								NetCS_SocketClose(PASV_sock_sd);

							ErrorTraceHandle(0,
									"FTP_flushCmds() - Mismatch in memory page data.\n");
						}

						while (no_write != nextPage->size_currentPage) // copy file to buffer array
						{
							putc(*pBuff, fp);
							pBuff++;
							no_write++;
						}

						no_write_total = no_write_total + no_write;

						ErrorTraceHandle(
								2,
								"Writing to file <offset:size:total>=<%d:%d:%d>.\n",
								nextPage->p_currentPage, no_write,
								no_write_total);

						if (nextPage->p_nextPage == NULL)
							break;
						nextPage = nextPage->p_nextPage;
					}
				}

				// -=-=-==-=-=-=-=-=-=-=-=-=---=--=-=-=-=-=-=-=-=-=-=-=-=

				ErrorTraceHandle(2,
						"... and copy FINISHED with bytes <%d> !\n",
						no_write_total);

				fclose(fp);
				ErrorTraceHandle(2, "File closed.\n");
			}

			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           STOR
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			ErrorTraceHandle(2, "Check for STOR.\n");
			if (!strncmp((*Cmds)[count], "STOR", 4)) {
				// After STOR has been sent over ftp, I need to open file and write it to socket
				//
				// Possible to define different STOR_local_file with preSTOR than remote file name.
				char c;
				long no_read_total, no_read, fsize;
				char STOR_remote_folder[100]; // not used yet
				char STOR_remote_filename[100];
				char *start_of_folder = NULL;
				char *file_name;

				// Format: STOR /folder/umdrv_swb.zip\r\n
				if (strstr((*Cmds)[count], "/") != NULL) {
					ErrorTraceHandle(2,
							"Recognised format: STOR /folder/filename\n");
					pBuff = strstr((*Cmds)[count], "/");
					start_of_folder = pBuff;
					count1 = 0;
					count2 = 0;
					while (*pBuff != '\r') {
						STOR_remote_folder[count1] = *pBuff;
						if (*pBuff == '/') // search for last slash (/)
							count2 = 0;
						else
							STOR_remote_filename[count2++] = *pBuff;
						count1++;
						pBuff++;
					}

					STOR_remote_filename[count2] = '\0';
					STOR_remote_folder[strlen(start_of_folder) - strlen(
							STOR_remote_filename) - 2] = '\0'; // cut filename

					ErrorTraceHandle(2, "Remote folder: %s\n",
							STOR_remote_folder);
					ErrorTraceHandle(2, "Remote file  : %s\n",
							STOR_remote_filename);
				}
				// Format: STOR umdrv_swb.zip\r\n
				else {
					ErrorTraceHandle(2, "Recognised format: STOR filename\n");
					pBuff = (*Cmds)[count];
					STOR_remote_folder[0] = '\0';
					count1 = 0;
					while (*pBuff != '\r') {
						if (*pBuff == ' ') // search for first space ( )
							count1 = 0;
						else
							STOR_remote_filename[count1++] = *pBuff;
						pBuff++;

					}
					STOR_remote_filename[count1] = '\0';
					ErrorTraceHandle(2, "Remote file: %s\n",
							STOR_remote_filename);
				}

				if (STOR_local_file[0] != '\0') // if preSTOR defined different local file name
					file_name = &STOR_local_file[0];
				else {
					file_name = &STOR_remote_filename[0];
					ErrorTraceHandle(2,
							"Remote file changed in preSTOR to: %s\n",
							STOR_remote_filename);
				}

				if ((fp = fopen(file_name, "r")) == NULL) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);

					ErrorTraceHandle(0,
							"FTP_flushCmds() - local open file error \n");
				}

				// obtain file size.
				fseek(fp, 0, SEEK_END);
				fsize = ftell(fp);
				rewind(fp);

				no_read_total = 0;

				// send file in STOR_BUFSIZE_set blocks
				while (1) {
					no_read = 0;
					while (no_read_total != fsize) // copy file to buffer array
					{
						global_buffer_send[no_read] = getc(fp);
						no_read++;
						no_read_total++;
						if (no_read == STOR_BUFSIZE_set)
							break;
					}

					global_buffer_send[no_read] = '\0';

					if (no_read > 0) {
						// send data to TCP socket opened on port received from PASV and send there in
						// max chunks of SEND_STREAM_BUFSIZE (200000)
						if (send(PASV_sock_sd, global_buffer_send, no_read, 0)
								== -1) {
							NetCS_SocketClose(FTP_connection_sock_sd);
							NetCS_SocketClose(PASV_sock_sd);
							ErrorTraceHandle(
									0,
									"FTP_flushCmds() - data send(socket=%d) error.\n",
									PASV_sock_sd);
						} else
							ErrorTraceHandle(2,
									"Transferred %d bytes of file. !\n",
									no_read);
					} else {
						NetCS_SocketClose(FTP_connection_sock_sd);
						NetCS_SocketClose(PASV_sock_sd);
						ErrorTraceHandle(0, "FTP_flushCmds() - read error.\n");
					}

					if (no_read_total == fsize) {
						ErrorTraceHandle(
								2,
								"Finished with file transfer, total %d bytes !\n",
								no_read_total);
						break;
					}
				}

				fclose(fp);
				// close socket that was opened for this transfer (PASV commad) and
				// open new one with another PASV for another transfer
				if (PASV_server_addr_active) {
					NetCS_SocketClose(PASV_sock_sd);
					PASV_server_addr_active = 0;
				}
			}

			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           PASV
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// Jos nije skroz implementirano, jer za svaku komandu kojoj treba prethodit PASV
			// se i ucini PASV, pa treba tako i hendlat sockete i aktivne flagove.
			ErrorTraceHandle(2, "Check for PASV.\n");
			if (!strncmp((*Cmds)[count], "PASV", 4)) {
				// Now that the server has replied to the PASV request, two channels are open:
				//   the first (the original one) is the communication channel where the requests are sent and
				//   the second is the data channel where the data is transfered.

				// input is last global_buffer_analyse[] from ReceiveFromMySocket()
				pBuff = &global_buffer_analyse[0];
				// RESULT: 227 Entering Passive Mode (159,107,237,228,129,252)
				count1 = 0;
				while (*pBuff) {
					switch (*pBuff) {
					case '(':
						count2 = 0;
						count1 = 1;
						break;
					case ',':
						PASV_server_straddr[count2++] = '\0';
						PASV_server_addr[count1 - 1]
								= atoi(PASV_server_straddr);
						ErrorTraceHandle(2, "New IP (%d) is <%d>\n",
								count1 - 1, PASV_server_addr[count1 - 1]);
						count2 = 0;
						count1++;
						break;
					case ')':
						PASV_server_straddr[count2++] = '\0';
						PASV_server_addr[count1 - 1]
								= atoi(PASV_server_straddr);
						ErrorTraceHandle(2, "New IP (%d) is <%d>\n",
								count1 - 1, PASV_server_addr[count1 - 1]);
						count1 = 0;
						break;
					default:
						if (count1)
							PASV_server_straddr[count2++] = *pBuff;
						break;
					}
					pBuff++;
				}

				//sprintf(buffer, "%d.%d.%d.%d",PASV_server_addr[0], PASV_server_addr[1], PASV_server_addr[2], PASV_server_addr[3]);
				PASV_server_port = PASV_server_addr[4] * 256 + PASV_server_addr[5];

				NetCS_SocketConfigHost(remotehost_ip, PASV_server_port, &remotehostAddr);
				if ((PASV_sock_sd = NetCS_ClientSocketConnect(&remotehostAddr, &localhostAddr, 1)) == 0) {
					NetCS_SocketClose(FTP_connection_sock_sd);
					if (PASV_server_addr_active)
						NetCS_SocketClose(PASV_sock_sd);
					ErrorTraceHandle(2, "NOT Connected to PASV IP address.\n");
					}
				else
				{
					ErrorTraceHandle(2, "Connected to PASV IP address.\n");
					PASV_server_addr_active = 1;
				}

				ErrorTraceHandle(2, "Connected to PASV IP:PORT(%d) address.\n", PASV_server_port);



				/* // .. and also listen for RETR purpose
				 if (listen(PASV_sock_sd, 5) == -1)
				 {
				 NetCS_SocketClose(FTP_connection_sock_sd);
				 NetCS_SocketClose(PASV_sock_sd);
				 ErrorTraceHandle(0, "FTP_flushCmds() - Server Socket error - listen()\n");
				 }

				 ErrorTraceHandle(2, "Connected to new port <%d> received from server <%s>.\n", PASV_server_port, peerhost_ip_string);
				 ErrorTraceHandle(2, "... and listening on socket (RETR).%d.\n", PASV_sock_sd);

				 ErrorTraceHandle(2, "Receiveing data ... ");
				 if(error_code=pthread_create(&NetCS_StreamThread_ID, NULL, ReceiveFromMySocket_stream, (void *)PASV_sock_sd) != 0)
				 {
				 NetCS_SocketClose(FTP_connection_sock_sd);
				 NetCS_SocketClose(PASV_sock_sd);
				 ErrorTraceHandle(0, "FTP_flushCmds() - pthread_create() error.\n");
				 }
				 ErrorTraceHandle(2, "... data RECEIVED !\n");
				 */
			}

			ErrorTraceHandle(2, "Check for QUIT.\n");
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			// POST-EXECUTION           QUIT
			// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			if (!strncmp((*Cmds)[count], "QUIT", 4))
				break; // last command executed
			else
				count++; // next command

			ErrorTraceHandle(
					2,
					"\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

			ErrorTraceHandle(2, "counted to next loop ...\n");
		} // end of command loop

		ErrorTraceHandle(2, "Exited FTP session.\n");
	}

	NetCS_SocketClose(FTP_connection_sock_sd);
	if (PASV_server_addr_active)
		NetCS_SocketClose(PASV_sock_sd);
}



// ***********************************************************************************
//
// Returns list of files with locations.
//
// ***********************************************************************************
int Analyse_GlobalBuffer_with_ls_Ra(char(*Cmds)[][MINIBUF]) {
	char *folder_name = (*Cmds)[6]; // transferred from Download_Folder()
	int Cmds_counter;
	//char global_buffer_tmp[RECEIVE_STREAM_BUFSIZE]; // = string na dnu filea ...
	int line = 0, word = 0;

	char flag_local_subfolder_active[1000]; // for creating local subfolders
	char flag_local_subfolder_file_active[1000];
	char flag_remote_subfolder_active[1000]; // for determine remote destination of folders and files
	char flag_remote_subfolder_file_active[1000];

	flag_local_subfolder_active[0] = '\0';
	flag_local_subfolder_file_active[0] = '\0';
	flag_remote_subfolder_active[0] = '\0';
	flag_remote_subfolder_file_active[0] = '\0';

	// (A)  /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/api_swu/src/:
	// (A1) /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv swb/api swu/src/:
	// (B)  /total 719
	// (C)  drwxr-xr-x   2 etkfrbi  cellocc      242 Jun  8 12:55 .
	// (D)  drwxr-xr-x   4 etkfrbi  cellocc      181 Jun  8 12:55 ..
	// (E)  -rw-r--r--   1 etkfrbi  cellocc    16682 Jun  3 15:25 api.c
	// (E1) -rw-r--r--   1 etkfrbi  cellocc    16682 Jun  3 15:25 api bla   bla.c

	if (global_buffer_analyse[0] == '\0') {
		ErrorTraceHandle(2, "There is no files in folder.");
		return 0;
	}

	// add result to trace file (it is to large to transfere it over function ErrorTraceHandle()
	if (DEBUG_on_TRACE_mode) {
		if (errortrace_printout_mode) // if able to create file
		{
			if ((fptrace = fopen("ErrorTrace_File.txt", "a+")) == NULL) {
				fprintf(stdout, "Unable to create trace error/trace file.\n");
				errortrace_printout_mode = 0;
			} else {
				fputs(global_buffer_analyse, fptrace);
				fclose(fptrace);
			}
		}
	}
	//strcpy(global_buffer_analyse, global_buffer_tmp);

	if (strlen(folder_name) == 0)
		return 0;

	Cmds_counter = 7; // start after folder name in Cmds[6]

	ErrorTraceHandle(2, "\nTarget folder on remote: %s\n", folder_name);
	GetWord(global_buffer_analyse, 1, 0, 0); // request for whole array, not for one element

	while (1) {
		// First find folder name in row, and coordinate action relative to this:
		// e.g.
		// folder_name = /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/api_swu/src/
		// search folder name in strings ...
		// finded      = /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/api_swu/src/.build.cs:

#ifdef ErrorTraceHandleTEST
		ErrorTraceHandle(4, "LINE(%d) <%s>\n", line, GetWord_sObjectMatrics[line][0].word);
#endif
		if (GetWord_sObjectMatrics[line][0].word[0] != '\0') // check if first word is null
		{
			char after_joined_all_words_in_line[1000];

			// join all words from one line into after_joined_all_words_in_line
			// this is used for folder check and if some (sub)folder had space in it - add them aswell (we will see this by difference in starting offsets of words)
			// e.g. /view/etkfrbi_ultramapper/vobs/cello/phy/ phy_subsys/umdrv_swb/api_swu/src
			{
				short space_name=0, i, add_spaces;

				strcpy(after_joined_all_words_in_line,
						GetWord_sObjectMatrics[line][space_name].word);
				space_name = 1;
				while (GetWord_sObjectMatrics[line][space_name].word[0] != '\0') {
					add_spaces = GetWord_sObjectMatrics[line][space_name].offset
							- GetWord_sObjectMatrics[line][space_name - 1].offset - strlen(
									GetWord_sObjectMatrics[line][space_name - 1].word);
					/*
					 ErrorTraceHandle(4, "LINE(%d) joining <%s>.\n", line, after_joined_all_words_in_line);
					 ErrorTraceHandle(4, "OFFSET %d,%d,%d = %d\n",
					 GetWord_sObjectMatrics[line][space_name-1].offset,
					 GetWord_sObjectMatrics[line][space_name].offset,
					 strlen(GetWord_sObjectMatrics[line][space_name-1].word),
					 add_spaces);
					 */
					for (i = 0; i < add_spaces; i++)
						// add spaces (1 part offset + length - 2 part offset)
						strcat(after_joined_all_words_in_line, " ");
					strcat(after_joined_all_words_in_line,
							GetWord_sObjectMatrics[line][space_name].word);
					space_name++;
				}
			} // now we should have whole /folder/file line (from LIST) joined into after_joined_all_words_in_line

#ifdef ErrorTraceHandleTEST
			ErrorTraceHandle(4, "LINE(%d) joined <%s>.\n", line, after_joined_all_words_in_line);
#endif
			if (!strncmp(folder_name, after_joined_all_words_in_line, strlen(
					folder_name))) // finded basic folder (A) in line.
			{
#ifdef ErrorTraceHandleTEST
				ErrorTraceHandle(4, "Folder name recognised in <%s>.\n", after_joined_all_words_in_line);
#endif
				if (strlen(folder_name) < (strlen(
						after_joined_all_words_in_line) - 1)) // if there is subfolder in (A) (when removed ":")
				{
					char mkdir_flag_local_subfolder_active[1000]; // mkdir should be prepared

					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					// FOLDER NAME input:                    /folder/subfolders/requested subfolder
					// LINE=after_joined_all_words_in_line:  /folder/subfolders/requested subfolder/subfolder:
					//
					// Remove basic folder name from LINE :
					// from /folder/subfolders/requested_subfolder/subfolder:
					// to                                 ./output/subfolder
					strcpy(flag_local_subfolder_active, ".");
					strcat(flag_local_subfolder_active, "/output"); // add local output folder
					strcat(
							flag_local_subfolder_active,
							(after_joined_all_words_in_line + strlen(
									folder_name) /* start after basic folder name */));
					flag_local_subfolder_active[strlen(
							flag_local_subfolder_active) - 1] = '\0'; // remove ":"
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					// QUOTES
					strcpy(mkdir_flag_local_subfolder_active,
							flag_local_subfolder_active);
					mkdir(mkdir_flag_local_subfolder_active, (S_IRWXU | S_IRWXG
							| S_IRWXO));
#ifdef ErrorTraceHandleTEST
					ErrorTraceHandle(3, "mkdir preformed on  : <%s>.\n", mkdir_flag_local_subfolder_active); // drwxr-xr-x
#endif
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					// FOLDER NAME input:                    /folder/subfolders/requested subfolder
					// LINE=after_joined_all_words_in_line:  /folder/subfolders/requested subfolder/subfolder:
					//
					// Remove basic folder name from LINE :
					// from /folder/subfolders/requested_subfolder/subfolder:
					// to                                        ./subfolder
					strcpy(flag_remote_subfolder_active, ".");
					strcat(flag_remote_subfolder_active,
							(after_joined_all_words_in_line + strlen(
									folder_name)));
					flag_remote_subfolder_active[strlen(
							flag_remote_subfolder_active) - 1] = '\0'; // remove ":"
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#ifdef ErrorTraceHandleTEST
					ErrorTraceHandle(3, "Remote subfolder extracted : <%s>.\n", flag_remote_subfolder_active);
#endif
					// if (flag_remote_subfolder_active[0] =='/') // if matters if starts with /subfolder or subfolder
				}
			}
			// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			// OUTPUT: flag_local_subfolder_active   (if we are operating in basic folder than this remains initialised null)
			// OUTPUT: flag_remote_subfolder_active  (if we are operating in basic folder than this remains initialised null)
			// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

			if (GetWord_sObjectMatrics[line][0].word[0] == '-') // if (C,D,E) is not directory, either -rw-rw-rw- or drwxr-xr-x shape.
			{
				// GetWord_sObjectMatrics[line][8].word - position of file name is 9 in request and 8 in array.
				// new improvement: file name can have SPACE., so it is not in 8(9), rather from 8(9)
#ifdef ErrorTraceHandleTEST
				ErrorTraceHandle(3, "File name recognised in flags <%s> off<%d>.\n", GetWord_sObjectMatrics[line][0].word, GetWord_sObjectMatrics[line][0].offset);
#endif
				if (GetWord_sObjectMatrics[line][8].word[0] == '\0') {
					ErrorTraceHandle(
							0,
							"Analyse_GlobalBuffer_with_ls_Ra() - Format of line is wrong, there should be 8 parameters in this line !");
					// -rw-r--r--   1 etkfrbi  cellocc     3699 May 12 12:15 build.spec
					break;
				}
				if (flag_remote_subfolder_active[0] != '\0') {
					int space_name, i, add_spaces;
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					// FOLDER NAME input:          /folder/subfolders/requested subfolder
					// LINE=GetWord_sObjectMatrics[line][0].word: /folder/subfolders/requested subfolder/subfolder:
					//
					// Create local file path if subfolder active:
					// from  ./sub folder
					// to    ./sub folder/file name
					strcpy(flag_local_subfolder_file_active,
							flag_remote_subfolder_active);
					strcat(flag_local_subfolder_file_active, "/");
					strcat(flag_local_subfolder_file_active,
							GetWord_sObjectMatrics[line][8].word);
					space_name = 9; // check for more parts of file name
					while (GetWord_sObjectMatrics[line][space_name].word[0] != '\0') // file name has space(s) ... calculate and add them
					{
						add_spaces = GetWord_sObjectMatrics[line][space_name].offset
								- GetWord_sObjectMatrics[line][space_name - 1].offset
								- strlen(GetWord_sObjectMatrics[line][space_name - 1].word);
						for (i = 0; i < add_spaces; i++)
							// add spaces (1 part offset + length - 2 part offset)
							strcat(flag_local_subfolder_file_active, " ");
						strcat(flag_local_subfolder_file_active,
								GetWord_sObjectMatrics[line][space_name].word);
						space_name++;
					}
					strcat(flag_local_subfolder_file_active, "\0");
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#ifdef ErrorTraceHandleTEST
					ErrorTraceHandle(3, "Subfolder active + file name: <%s>.\n", flag_local_subfolder_file_active);
#endif
				} else {
					int space_name, i, add_spaces;
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					// FOLDER NAME input:          /folder/subfolders/requested subfolder
					// LINE=GetWord_sObjectMatrics[line][0].word: /folder/subfolders/requested subfolder/subfolder:
					//
					// Create local file path if no subfolder active:
					// from  file name
					// to    ./file name
					strcpy(flag_local_subfolder_file_active, "./");
					strcat(flag_local_subfolder_file_active,
							GetWord_sObjectMatrics[line][8].word);
					space_name = 9; // check for mode parts of file name
					while (GetWord_sObjectMatrics[line][space_name].word[0] != '\0') // file name has space(s)
					{
						add_spaces = GetWord_sObjectMatrics[line][space_name].offset
								- GetWord_sObjectMatrics[line][space_name - 1].offset
								- strlen(GetWord_sObjectMatrics[line][space_name - 1].word);
						for (i = 0; i < add_spaces; i++)
							// add spaces (1 part offset + length - 2 part offset)
							strcat(flag_local_subfolder_file_active, " ");
						strcat(flag_local_subfolder_file_active,
								GetWord_sObjectMatrics[line][space_name].word);
						space_name++;
					}
					strcat(flag_local_subfolder_file_active, "\0");
					// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#ifdef ErrorTraceHandleTEST
					ErrorTraceHandle(3, "Subfolder not active, file name: <%s>.\n", flag_local_subfolder_file_active);
#endif
				}
				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				// OUTPUT: flag_local_subfolder_file_active
				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				// FOLDER NAME input:          /folder/subfolders/requested_subfolder
				// + flag_local_subfolder_file_active
				//
				// Create for PORT command :
				// from  /folder/subfolders/requested_subfolder
				// to    '/folder/subfolders/requested_subfolder/file_name'
				// QUOTES
				strcpy(flag_remote_subfolder_file_active, folder_name);
				strcat(flag_remote_subfolder_file_active,
						(flag_local_subfolder_file_active + 1)); // +1 to remove dot "."
				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				// OUTPUT: flag_remote_subfolder_file_active
				// -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#ifdef ErrorTraceHandleTEST
				ErrorTraceHandle(3, "Path + file name result : <%s>.\n", flag_remote_subfolder_file_active);
#endif
				strcpy((*Cmds)[Cmds_counter], "PORT\r\n");
				//strcpy((*Cmds)[Cmds_counter], "PASV\r\n");
#ifdef ErrorTraceHandleTEST
				ErrorTraceHandle(3, "FTP command added : <%s>\n", (*Cmds)[Cmds_counter]);
#endif
				Cmds_counter = Cmds_counter + 1;
				strcpy((*Cmds)[Cmds_counter], "RETR ");
				strcat((*Cmds)[Cmds_counter], flag_remote_subfolder_file_active);
				strcat((*Cmds)[Cmds_counter], "\r\n");
#ifdef ErrorTraceHandleTEST
				ErrorTraceHandle(3, "FTP command added : <%s>\n", (*Cmds)[Cmds_counter]);
#endif
				Cmds_counter = Cmds_counter + 1;
			}
		} else {
			ErrorTraceHandle(2, "LIST -R Folder analysis finished (EOF).\n");
			break;
		}
		line++;
	}
	strcpy((*Cmds)[Cmds_counter], "QUIT\r\n");
#ifdef ErrorTraceHandleTEST
	ErrorTraceHandle(3, "FTP command added : <%s>\n", (*Cmds)[Cmds_counter]);
#endif
	return 1;
}


// ***********************************************************************************
//
// Create request to ftp whole folder and recursive subfolders with files
//
// **********************************************************************************

void FTP_Init(void) {
	NetCS_StreamThread_ID = 0;
	firstPage = NULL;
	nextPage  = NULL;
	lastPage  = NULL;
}

void FTP_SelectPutfile(void) {
	char Cmds[7][MINIBUF] = { "USER", "PASS", "TYPE I\r\n",
			"PASV\r\n", "preSTOR ", // local filename (if flag_exist_user_file_name)
			"STOR ", // /remote_folder/remote_filename
			"QUIT\r\n" };
	char remote_folder[500];
	char remote_filename[100];
	int flag_exist_user_file_name = 0;
	int i;

	FTP_Init();

	silent_printout_mode = 0; // don't show commands, only results

	if ((RemoteUser.folder[0] == '\0') || (RemoteUser.station[0] == '\0'))
		ErrorTraceHandle(0,
				"\nRemote station name or ip address is missing !\n");

	if (RemoteUser.file[0] != '\0')
		flag_exist_user_file_name = 1;

	// RemoteUser.folder should have FORMAT: /folder_name/file_name
	if (RemoteUser.folder[strlen(RemoteUser.folder) - 1] == '/') // if folder requested finishes with "/" remove it !
		if (RemoteUser.folder[0] != '/')
			ErrorTraceHandle(0,
					"\nSyntex error in folder name, check format !\n");

	{ // separate folder name and file name from RemoteUser.folder input
		char *pBuff;
		int count1, count2, count3;

		RemoteUser.folder[strlen(RemoteUser.folder)] = '\0';

		pBuff = &RemoteUser.folder[0];
		count1 = 0;
		count2 = 0;
		while (*pBuff != '\0') {
			remote_folder[count1] = *pBuff; // format /x/y
			if (*pBuff == '/') // check and remember if last slash (/)
			{
				count2 = 0;
				count3 = count1; // last occurence of /
			} else {
				remote_filename[count2] = *pBuff;
				count2++;
			}
			count1++;
			pBuff++;
		}
		remote_folder[count3] = '\0'; // cut folder name on last occurence of slash (/)
		remote_filename[count2] = '\0';

		ErrorTraceHandle(2, "Remote folder: %s\n", remote_folder);
		ErrorTraceHandle(2, "Remote file  : %s\n", remote_filename);
	}

	if (flag_exist_user_file_name) {
		strcat(Cmds[4], RemoteUser.file);
		strcat(Cmds[4], "\r\n"); // preSTORE local file name
		strcat(Cmds[5], RemoteUser.folder);
		strcat(Cmds[5], "\r\n"); // STOR /remote_folder_name/remote_file_name

		ErrorTraceHandle(1, "\nFTP UPLOAD <%s:./:%s> TO <%s:%s:%s>\n\n",
				User.station, RemoteUser.file, RemoteUser.station, remote_folder,
				remote_filename);
	} else // RemoteUser.file not defined (same filename for local and host)
	{
		strcpy(Cmds[4], "STOR ");
		strcat(Cmds[4], RemoteUser.folder);
		strcat(Cmds[4], "\r\n"); // STORE /remote_folder_name/remote_file_name (preSTORE overwritten)
		strcpy(Cmds[5], "QUIT\r\n"); // QUIT

		ErrorTraceHandle(1, "\nFTP UPLOAD <%s:./:%s> TO <%s:%s:%s>\n\n",
				User.station, remote_filename, RemoteUser.station, remote_folder,
				remote_filename);
	}

	for (i = 0; i <= 5; i++)
		ErrorTraceHandle(3, "List of Commands: <%s>\n", Cmds[i]);

	FTP_flushCmds(RemoteUser.station, &Cmds);

	fprintf(stdout, "\n");

}

void Download_Folder(char *host_name, char *folder_name) {
	char
			Cmds[MAX_LINE_LS][MINIBUF] = { "USER", "PASS",
					"TYPE I\r\n" };
	char Cmd_LIST[500] = "LIST -R ";
	char *Cmd_LIST_end = "\r\n\0";

	int i;

	strcat(Cmd_LIST, folder_name);
	strcat(Cmd_LIST, Cmd_LIST_end);

	strcpy(Cmds[3], "PORT\r\n");
	strcpy(Cmds[4], Cmd_LIST);
	strcpy(Cmds[5], "analyse&download&finish"); // local custom command

	if (folder_name[strlen(folder_name) - 1] == '/') // if folder requested finishes with "/" remove it !
		folder_name[strlen(folder_name) - 1] = '\0';

	strcpy(Cmds[6], folder_name); // for temporary transfer

	for (i = 0; i < 6; i++)
		ErrorTraceHandle(3, "List of Commands: <%s>\n", Cmds[i]);

	//Analyse_GlobalBuffer_with_ls_Ra(Cmds);
	FTP_flushCmds(host_name, &Cmds);
}

void FTP_SelectGetdir(void) {
	char *phost_name;
	char folders[500];
	char *pfolders = &folders[0];
	int i;

	FTP_Init();

	if ((RemoteUser.folder[0] == '\0'))
		ErrorTraceHandle(0, "\nRemote folder path is missing !\n");

	phost_name = (char *) malloc(strlen(RemoteUser.station));
	strcpy(phost_name, RemoteUser.station);
	strcpy(folders, RemoteUser.folder);

	ErrorTraceHandle(1, "\n-=-=-=-=-=-=-=-=\n");
	ErrorTraceHandle(1, "= Ftp download - %s >>> %s <%s>\n", phost_name,
			User.station, pfolders);
	ErrorTraceHandle(1, "-=-=-=-=-=-=-=-=\n");

	Download_Folder(phost_name, pfolders);

}

void FTP_SelectTest1() {
	char Cmds[][MINIBUF] = { "USER", "PASS", "TYPE I\r\n",
			"PORT\r\n", "RETR frane.txt\r\n", "PORT\r\n", "RETR mate.txt\r\n",
			"PORT\r\n", "RETR ante.txt\r\n", "QUIT\r\n" };

	char *host_name = "kamenjak";

	FTP_Init();

	ErrorTraceHandle(1, "\n\n---------------------------\n");
	ErrorTraceHandle(1, "Ftp and get multiple files:\n");
	ErrorTraceHandle(1, "---------------------------\n");

	FTP_flushCmds(host_name, &Cmds);

}
