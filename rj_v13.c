/*
v1		- Solaris only
v2  	- Solaris and Debian Linux
v3  	- Update differencies from Debian
v3.3 	- adds for efiolid
v6  	- Together with parallel v5 on Linux platform, update with SCTP
		 - v6 will contain makeup changes in rj.c
v6.1 	- SCTP client part on Solaris, while v5 on Linux have server SCTP part
v7  	- merge of v5 and v6, SCTP client and server, add also bloxy - m3ua/isup server
v7.1 	- improving sctp server
	  	  new buffer handling for transport users (tcp, udp, sctp)
v10  	- m3ua/isup server tested on debian-sea and suse-tsc blade
	    - thread arg to struct
		- added in NETCS pointer to user data structure
		- added binding server on provisioned ip address (if system has more than one) and port
v11  	- gsm LU tested, 14 000 000 LU calls (10 000 000 pass / 3 000 000 fail) approx. 1 hour; MSC load stabile at approx 52% with
	      plldp 12500
v11  	- mml enhanced
		- sccp SST SSA
	    - statistics
 	    - with new NETCS
v12 	2010.03.01 - 10000 lu cps
		2010.03.05 - started with isup
      	2010.03.08 - more isup moving to SEA
      	2010.03.11 - happy tesst isup
		2010.03.12 - isupl oad with cic printout
	    2011.05.10 - added SCTP multiple streams for BICC loop in NW3 (Fax Tone VBD Feature)
		2011.12.14 - mml names changed
        2011.12.15 - initialized flags to 0 for sctp_recvmsg() suddenly started reporting status "resource temporary unavailable"

v13     2018 cloud

 */

#define RJ_MAIN
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/axe.h>
#include <modules/cpp/ultramapper.h>
#include <modules/cpp/riscwatch.h>
#include <modules/protocols/ftp.h>
#include <modules/system/linux_procstats.h>
#include <modules/system/linux_sysinfo.h>
#undef RJ_MAIN

char setenvMyDISPLAY[100];
char setenvRJlocation[200];

// ***********************************************************************************
//
// MAIN
//
// ***********************************************************************************
int main(int argc,char *argv[])
{
  int          optionCharacter;
  extern char *optarg;
  extern int   optind, opterr, optopt;

  FILE *hex_p, *out_p;

  char a;
  char work_buffer[MINIBUF];

  int  session, i;
  char plain_string[MINIBUF], plain_string2[MINIBUF], plain_string3[MINIBUF];
  struct sIntValue {
	  char 	indicator;
	  int 	int_value;
  } k,K,t,T;
  int  UDP0_TCP1_SCTP2 = 1;

  bool  flag1 = false;
  bool  flag2 = false; // session8_flag_doublehex
  bool  flag3 = false;

  struct service_data
  {
	  char service_name[50];
	  char service_help[200];
	  unsigned int allowed;
  } sd[30] = {
		  {"-h", 				"HELP for all services.", 					1},

		  {"stat_avg", 			"Show CPU load average for 1,5,15 minutes.",1},
		  {"stat_proc",			"Show proc stasts.", 						1},

		  {"port_scan", 		"Scann all open ports on given host.", 		1},
		  {"port_string_send",	"Send string to IP:port.", 					1},
		  {"port_file_send",	"Send txt file to IP:port.", 			    1},
		  {"port_string_server","Listen to port, receive strings and reply.",1},

		  {"ftp_getdir", 		"Download remote folder with ftp.", 		1},
		  {"ftp_putfile",		"Upload one file with ftp.", 				1},

		  {"remote_shellcmd", 	"Scan Zg WorkStations for free CC use.", 	1},

		  {"tgen_pool", 		"Pool periodically data from TGEN.",		1},
		  {"signalling", 		"Telco signalling framework.", 								1},
		  {"oscillod", 		    "Oscillo daemon serving oscillo Java GUI.",	1},
		  {"ethserproxy", 		"Terminal Server ethernet-serial proxy.", 	1},

		  {"rwchat", 			"RiscWatch debugger command interface.", 	0},
		  {"uploadum", 			"Upload UltraMapper.", 						0},

		  {"test_ser", 			"Test.", 									0},
		  {"test_ethserproxytest", "Test.", 								0},
		  {"test_ftptest",	 	"Test.", 									0},
		  {"test",			 	"Test.",  									0},

		  {"",	 				"", 										99}
		 };

  memset(&User, 	  0, sizeof(User));
  memset(&RemoteUser, 0, sizeof(RemoteUser));
  plain_string[0]   = '\0';

  k.indicator = 0;
  K.indicator = 0;
  t.indicator = 0;
  T.indicator = 0;

  NetCs_Pid_THREADSAFE_MAINAPP = getpid();

  //strcpy(setenvRJlocation, argv[0]); // get home of rj
  //printf("home<%s>\n", setenvRJlocation);

  while ((optionCharacter = getopt((argc-1), &argv[1], "l:p:L:i:k:u:U:r:P:R:I:K:f:F:c:t:T:b:sdow")) != EOF)
	{
		switch(optionCharacter)
		{
			// LOCAL USER DATA
			case 'l':
				strcpy(User.name, optarg);
			break;
			case 'p':
				strcpy(User.passwd, optarg);
			break;
			case 'L':
				strcpy(User.station, optarg);
			break;
			case 'i':
				strcpy(User.IP, optarg);
			break;
			case 'k':
				k.int_value = atoi(optarg);
				k.indicator = 1;
			break;
			case 'u':
				strcpy(User.file, optarg);
			break;
			case 'U':
				strcpy(User.folder, optarg);
			break;


			// REMOTE USER DATA
			case 'r':
				strcpy(RemoteUser.name, optarg);
			break;
			case 'P':
				strcpy(RemoteUser.passwd, optarg);
			break;
			case 'R':
				strcpy(RemoteUser.station, optarg);
			break;
			case 'I':
				strcpy(RemoteUser.IP, optarg);
			break;
			case 'K':
				K.int_value = atoi(optarg);
				K.indicator = 1;
			break;
			case 'f':
				strcpy(RemoteUser.file, optarg);
			break;
			case 'F':
				strcpy(RemoteUser.folder, optarg);
			break;


			// OTHER DATA
			case 'c': // string
				strcpy(plain_string, optarg);
			break;
			case 't':
				t.int_value = atoi(optarg);
				t.indicator = 1;
			break;
			case 'T':
				T.int_value = atoi(optarg);
				T.indicator = 1;
			break;
			case 'b': // ftp STOR buffer size
				STOR_BUFSIZE_set = atoi(optarg);
				if ((STOR_BUFSIZE_set < 5000) ||
						(STOR_BUFSIZE_set> 70000))
				ErrorTraceHandle(0, "main() - Buffer size for upload should be in range <5000, 50000>.!\n");
			break;


			// FLAGS
			case 's':
				flag1 = true;
			break;
			case 'd':
				flag2 = true;
			break;
			case 'o':
				flag3 = true;
			break;
			case 'w':
				DEBUG_on_ERROR_mode = 1;
				DEBUG_on_TRACE_mode = 0;
			break;
			default:
			break;
		}
	}

	session = 0;

	if (argc >= 2) { // if [mininum = rj service_name] argc1=argv[0] ;  argc2=argv[1]
		i = 0;
		while (sd[i].allowed != 99) {
			if (!strcmp(argv[1], sd[i].service_name)) {
				session = i; // session recognised
				break;
			}
			i++;
		}
	}

	if (session == 0) {
		ErrorTraceHandle(1, "\nrj service_name [-lLirfFcpb _param] [-doh]\n");
		ErrorTraceHandle(1, "                [-l login_as_user ]\n");
		ErrorTraceHandle(1,
				"                [-p user_password (if ommited, user will be prompted)]\n");
		ErrorTraceHandle(1, "                [-w debug mode]\n");
		ErrorTraceHandle(1, "\n");
		ErrorTraceHandle(1, "For help, use: rj [service_name -h]\n");
		ErrorTraceHandle(1, "\n");
		ErrorTraceHandle(1, "Services supported:\n");

		i = 0;
		while (sd[i].allowed != 99) {
			if (sd[i].allowed)
				ErrorTraceHandle(1, "           %-20s -> %s \n",
						sd[i].service_name, sd[i].service_help);
			i++;
		}
		exit(0);
	} else /* session > 0 */ {
		if ((argc >= 3) && (!strcmp(argv[2], "-h"))) // this is [rj service_name -h]
		{
			i = 0;
			while (sd[i].allowed != 99) {
				sd[i].allowed = 0;
				i++;
			}
			sd[session].allowed = 1;
			argc = 1;
		}
	}

	//printf("argc=%d, argv[1]=%s\n", argc, argv[1]);
	//printf("RemoteUser.station=<%s>\n", RemoteUser.station);
	//printf("RemoteUser.file=<%s>\n", RemoteUser.file);
	//printf("RemoteUser.folder=<%s>\n", RemoteUser.folder);

	if (argc > 1) // this data are not needed if only help active
	{
		struct passwd *user_passw;
		struct hostent *tmp_ip;
		char *host_ip, *user_pw;
		uid_t uid;

		if (User.name[0] == '\0') // if not gived in arguments
		{
			if ((user_passw = getpwuid(uid = getuid()/* getuid() returns real user ID of  the  calling  process.
			 The real user ID identifies the person who is logged in.*/
			)/* getpwuid() function searches for a password entry with
			 the (numeric) user ID specified by the uid parameter.*/
			) == NULL)
				ErrorTraceHandle(0, "main(): getpwuid error.\n");
			strcpy(User.name, user_passw->pw_name);
		}

		if (User.passwd[0] == '\0') {
			if (!strncmp(User.name, "frbi", 6)) // if my station, set password, otherwise request it
				strcpy(User.passwd, "brodvasa");
			else if (!strncmp(User.name, "root", 6)) {// if cello station, set password, otherwise request it
				strcpy(User.name, "root"); // to outside there is no user etkcello, only in Split
				strcpy(User.passwd, "root");
			} else if (!strncmp(User.name, "etkfrbi", 6)) // if my station, set password, otherwise request it
				strcpy(User.passwd, "jurko123");
			else {
				user_pw = (char *) getpass("Enter password: ");
				/* getpass() function opens the process's controlling  ter-
				 minal,  writes  to  that  device  the null-terminated string
				 prompt, disables echoing, reads a string of characters up to
				 the  next  newline  character  or EOF, restores the terminal
				 state and closes the terminal. */
				if (user_pw == NULL)
					ErrorTraceHandle(0, "main(): getpass() error.\n");
				strcpy(User.passwd, user_pw);
			}
		}

		if (User.station[0] == '\0') {
			if (gethostname(User.station, 20) < 0) /* gethostname() function returns the standard host name
			 for the current processor, as previously set by sethostname(). */
				ErrorTraceHandle(0, "main(): gethostname error.\n");
		}

		//if (RemoteUser.station[0] == '\0') /* prompting station to execute commands */
		//	strcpy(RemoteUser.station, User.station);

		if ((tmp_ip = gethostbyname(User.station) /* gethostbyname() function searches host information (e.g. IP addr) for a
		 host with the hostname specified by the character-string*/
		) == NULL)
			ErrorTraceHandle(0, "main() - Can't determine remote host name !\n");

		host_ip = (char *) inet_ntoa((struct in_addr)(*((struct in_addr *) tmp_ip->h_addr)));

		//inet_pton(AF_INET, host_ip, &User.IP);

		for (i = 0; i < strlen(host_ip); i++)
			if (host_ip[i] == '.')
				host_ip[i] = ' ';

		User.IP[0] = (char) atoi((const char*) GetWord(host_ip, 0, 0, 0));
		User.IP[1] = (char) atoi((const char*) GetWord(host_ip, 0, 0, 1));
		User.IP[2] = (char) atoi((const char*) GetWord(host_ip, 0, 0, 2));
		User.IP[3] = (char) atoi((const char*) GetWord(host_ip, 0, 0, 3));

		//printf("User ws: <%s> <%d.%d.%d.%d>, User name: <%s> , User password: <%s>\n",  User.station, User.IP[0], User.IP[1], User.IP[2], User.IP[3], User.name, User.passwd);
	}

	sprintf(setenvMyDISPLAY, "xhost +;setenv DISPLAY %s:0.0;", User.station);

	if (argc == 1) {

		if (!strcmp(sd[session].service_name, "port_scan")) {
			ErrorTraceHandle(1, "Scan all available ports on given host.\n");
			ErrorTraceHandle(1, "             FORMAT:  rj port_scan -I RemoteUser.IP [-t port_offset]\n");
			ErrorTraceHandle(1, "                                  [-t port_offset]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj port_scan -I 159.107.227.43\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj port_scan -I 127.0.0.1 -t 5000\n");
			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "port_string_send")) {
			ErrorTraceHandle(1, "Send string to IP:port\n");
			ErrorTraceHandle(1, "             FORMAT:  rj port_string_send  -I RemoteUser.IP\n");
			ErrorTraceHandle(1, "                                           -K RemoteUser.port\n");
			ErrorTraceHandle(1, "                                           -t UDP0_TCP1_SCTP2\n");
			ErrorTraceHandle(1, "                                           -c command string to send\n");
			ErrorTraceHandle(1, "                                           -s [flag to leave connection ON]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "		  EXAMPLE: rj port_string_send -I 172.17.87.94 -K 4444 -c hello\n");
			ErrorTraceHandle(1, "		  EXAMPLE: rj port_string_send -I 127.0.0.1 -K 9000 -c print -t 1 -T 1\n");
			ErrorTraceHandle(1, "		  EXAMPLE: rj port_string_send -I 127.0.0.1 -K 9000 -c \"run UMTS_LU_PERIODIC cara=500\" -t 1\n");
			ErrorTraceHandle(1, "		  ALIAS:   alias rjtgen './rj port_string_send -I 127.0.0.1 -K 9000 -c \"\\!*\" -t 1' \n");
			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "port_string_server")) {
			ErrorTraceHandle(1, "Send string to IP:port\n");
			ErrorTraceHandle(1, "             FORMAT:  rj port_string_server -k port_number\n");
			ErrorTraceHandle(1, "                                            -t UDP0_TCP1_SCTP2\n");
			ErrorTraceHandle(1, "                                            -c command reply back or filename (flag -s)\n");
			ErrorTraceHandle(1, "                                            -s [flag to indicate filename in -c]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj port_string_server -k 4444 -t 2 -c hello\n");
			ErrorTraceHandle(1, "\n");
		}





		if (!strcmp(sd[session].service_name, "ftp_getdir")) {
			ErrorTraceHandle(1, "Download remote folder with FTP.\n");
			ErrorTraceHandle(1, "             - to check, compare folder size (remote with local) with \"du -sk folder_name\"\n");
			ErrorTraceHandle(1, "             - transfer type is binary\n");
			ErrorTraceHandle(1, "             - result is stored in ./output folder\n");
			ErrorTraceHandle(1, "             FORMAT:  rj ftp_getdir [-R RemoteUser.station]\n");
			ErrorTraceHandle(1, "                                    -F /folder_to_download\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R fenoliga -F /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R fenoliga -F /TCM/PROJECTS/Cello/software/Riskwatch/5.1\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R vanaheim -F /home3/etkfrbi/tmp \n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -p xxx -R vanaheim -F /home3/etkfrbi/tmp\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R ws9015.uab.ericsson.se -F /view/etkfrbi_ultramapper_cpp/vobs/cello/phy2/UMDRV-SWB_CNX9011339/src\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R krk -F /view/etkfrbi_supermapper/vobs/cello/drvbt/auto_test\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj ftp_getdir -R fenoliga -F /view/etkfrbi_supermapper/vobs/cello/drvbt/auto_test\n");
			ErrorTraceHandle(1, "\n");
		}
		if (!strcmp(sd[session].service_name, "ftp_putfile")){
			ErrorTraceHandle(1, "Transfer one file with FTP.\n");
			ErrorTraceHandle(1, "             FORMAT:   rj ftp_putfile [-R RemoteUser.station]\n");
			ErrorTraceHandle(1, "                                      [-b upload_buffer_size <5000, 50000>, def 70000]\n");
			ErrorTraceHandle(1, "                                      [-f file_name_here]\n");
			ErrorTraceHandle(1, "                                      -F /folder_name_there/file_name_there\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R localhost -F/home/frbi/Eclipse/workspace/REMOTEjob/Release/rj.c\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R cellonode1st.design.etk.ericsson.se -b 5000 -p 001 -f umdrv.ppc405 -F /c/loadmodules/umdriver.c41\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R cellonode32 -b 5000 -p 032 -f umdrv.ppc405 -F /c/loadmodules/ETC4100000_P1A01\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R fwbt05.uab.ericsson.se -b 5000 -p x -f umdrv.ppc405 -F /c/loadmodules/umdriver.c41\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -l etkcello -p cello -R taxila.design.etk.ericsson.se -F /home4/etkcello/etkfrbi/ET-C41/tbin/umdrv.ppc405.elf\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R premuda.design.etk.ericsson.se -F /home7/etkfrbi/My/umdrv.ppc405\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -l etkcello -p cello -R metis.design.etk.ericsson.se -F /home4/etkcello/etkfrbi/ET-C41/nbin/umdrv.ppc405\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R ws9015.uab.ericsson.se -F /home/etkfrbi/bin/rj\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ftp_putfile -R ws9015.uab.ericsson.se -F /view/etkfrbi_ultramapper_cpp/vobs/cello/phy2/UMDRV-SWB_CNX9011339/src/swu/x.x\n");
			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "rsh")) {
			ErrorTraceHandle(1, "Execute cmd on remote shell (user validated with local username and password).\n");
			ErrorTraceHandle(1, "             FORMAT:   rj rsh -R station -c shell_command\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj rsh -R eris -c ls\n");

			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "rwchat")){
			ErrorTraceHandle(1, "RiscWatch Interface\n");
			ErrorTraceHandle(1, "             FORMAT:  rj rwchat [-R RW_server]\n");
			ErrorTraceHandle(1, "                                -c command_string_to_send\n");
			ErrorTraceHandle(1, "                                [-d] enter double value of address for RiscWatch read or write\n");
			ErrorTraceHandle(1, "                                [-o] read from SuperMapper (default is UltraMapper)\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             List of NOT allowed RiscWatch commands:\n");
			ErrorTraceHandle(1, "               bot,capture,down,file,find,findb,finde,focus,funcdisp,goto,hidewins,ip\n");
			ErrorTraceHandle(1, "               line,memchk,pagedn,pageup,showip,srcdisp,srchpath,srcline,top,up,varinfo\n");
			ErrorTraceHandle(1, "               varvis,view,window");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             List of CUSTOM commands:\n");
			ErrorTraceHandle(1, "               quit   - terminate RISCWatch server\n");
			ErrorTraceHandle(1, "               status - read the processor status\n");
			ErrorTraceHandle(1, "               test   - test the communications connection\n");
			ErrorTraceHandle(1, "               lum    - load UltraMapper Cello5\n");
			ErrorTraceHandle(1, "               lsm4   - load SuperMapper Cello4\n");
			ErrorTraceHandle(1, "               tres   - restart TACT GEN_IO 1 & 2\n");
			ErrorTraceHandle(1, "               pmrst  - trigger GEN_IO 8 for UM PMRST\n");
			ErrorTraceHandle(1, "               test_aug - test AUG link on ET-C41 board\n");
			ErrorTraceHandle(1, "               R      - readh\n");
			ErrorTraceHandle(1, "               RR     - readh with range x y (max range 100 addresses)\n");
			ErrorTraceHandle(1, "               W      - writeh dmem\n");
			ErrorTraceHandle(1, "               WR     - writeh dmem with range x y (max range 100 addresses)\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -R atmtest10.uab.ericsson.se -c status\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -R ws7073 -c 'readh 0xC60e0028' [-d]\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -c 'R  0xC60e0028'\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -c 'RR 0xC60e0014 0xC60e0020'\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -R ws7073 -c 'writeh dmem 0xC60e0028 0x0000' [-d]\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -c 'W  0xC60e0028 0x0000'\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -c 'WR 0xC60e0014 0xC60e0020 0x0000'\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj rwchat -R ws7073 -c lum\n");
			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "seati")){
			ErrorTraceHandle(1, "Fiol MML interface\n");
			ErrorTraceHandle(1, "             FORMAT:  rj fiol\n");
			ErrorTraceHandle(1, "\n");
		}

		if (!strcmp(sd[session].service_name, "signalling")){
			ErrorTraceHandle(1, "Uses sigtran.config script.\n");
			ErrorTraceHandle(1, "             FORMAT:  rj signalling\n");
			ErrorTraceHandle(1, "\n");
		}
		if (!strcmp(sd[session].service_name, "oscillod")){
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             FORMAT:  rj oscillod\n");
			ErrorTraceHandle(1, "\n");
		}
		if (!strcmp(sd[session].service_name, "tgen_pool")){
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             FORMAT:  rj tgen_pool -I RemoteUser.IP\n");
			ErrorTraceHandle(1, "                                   -K RemoteUser.port\n");
			ErrorTraceHandle(1, "                                   -t pooling time\n");
			ErrorTraceHandle(1, "             EXAMPLE: ./rj tgen_pool -I 127.0.0.1 -K 9000 -t 1\n");
			ErrorTraceHandle(1, "\n");
		}
		if (!strcmp(sd[session].service_name, "ethserproxy")){
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             FORMAT:   rj ethserproxy -K port_number\n");
			ErrorTraceHandle(1, "                                      -c [/dev/ttya | /dev/ttyb]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj ethserproxy -c /dev/ttyb -K 2032\n");
			ErrorTraceHandle(1, "\n");
		}
      if (!strcmp(sd[session].service_name, "test_ethserproxytest")){
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             FORMAT:   rj test_ethserproxytest -K port_number\n");
			ErrorTraceHandle(1, "                              -c [/dev/ttya | /dev/ttyb]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE:  rj test_ethserproxytest -c /dev/ttyb -K 2032\n");
			ErrorTraceHandle(1, "\n");
      }

#ifdef _SOLARIS_
      if (!strcmp(sd[session].service_name, "uploadum")) {
			ErrorTraceHandle(1, "Upload UltraMapper folders/files to Zg CC server private folder.\n");
			ErrorTraceHandle(1, " - start application from folder: (ST)/home3/etkfrbi/UltraMapper/umdrv_swb/\n");
			ErrorTraceHandle(1, " - if remote cmdtools are not appearing, execute \"xhost +\" before rj command.\n");
			ErrorTraceHandle(1, "             FORMAT:  rj uploadum[-R RemoteUser.station]\n");
			ErrorTraceHandle(1, "\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj uploadum -R kamenjak\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj uploadum -R fenoliga -l etkfrbi\n");
			ErrorTraceHandle(1, "             EXAMPLE: rj uploadum -R fenoliga\n");
			ErrorTraceHandle(1, "\n");
      }
#endif

    } else if (argc >= 2) {


		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "stat_avg")) {
			double loadavg[3];
			int i, bin=0xF51;
			getloadavg(loadavg, sizeof(loadavg) / sizeof(loadavg[0]));
			for (i = 0; i < (sizeof(loadavg)/sizeof(loadavg[0])); i++)
				printf(" %-2d minute moving load = %.2f\n", (bin>>(i*4))&0xF, loadavg[i]);
		}

#ifndef _SOLARIS_
#ifndef _SUSE_

		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "stat_proc")) {

			if (t.indicator == 0)
				ErrorTraceHandle(0, "\nMissing PID!\n");
			Procstat_SelectProcStats(t.int_value);
		}
#endif
#endif



		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "port_scan")) {
			char *phost_name;
			int type_number0_name1;
			int scan_port_offset;

			if (t.indicator == 0)
				scan_port_offset = 1;
			else
				scan_port_offset = t.int_value;

			if (RemoteUser.IP[0] == '\0')
				ErrorTraceHandle(0,"\nRemote ip address is missing !\n");

			if (RemoteUser.IP[0] != '\0') {
				phost_name = (char *) malloc(strlen(RemoteUser.station));
				strcpy(phost_name, RemoteUser.station);
				type_number0_name1 = 1;
			} else {
				phost_name = (char *) malloc(strlen(RemoteUser.IP));
				strcpy(phost_name, RemoteUser.IP);
				type_number0_name1 = 0;
			}

			ErrorTraceHandle(1, "\nScanning all ports on <%s> starting from %d\n", phost_name, scan_port_offset);

			Net_ScanAllHostPORTs(phost_name, type_number0_name1, scan_port_offset);
		}






		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "port_string_send")) {
			struct sockaddr_in remotehost_addr;
			struct sockaddr_in localhost_addr;
			int error_code, udp_sentbytes;
			size_t size;
			char *string=NULL;
			int timer = 0;
			time_t current_time, pooled_time, passed_time;
			unsigned int buffer_read_index;

			if ((RemoteUser.IP[0] == '\0') || (plain_string[0] == '\0') || (K.indicator == 0) || (t.indicator == 0))
				ErrorTraceHandle(0, "\nRemote station IP, port, string or transport type missing !\n");

			RemoteUser.port = K.int_value;
			UDP0_TCP1_SCTP2 = t.int_value;
			timer			= T.int_value;
			string 			= plain_string;
			strcat(plain_string, "\r\n");

			NetCS_SocketConfigHost(RemoteUser.IP, RemoteUser.port, &remotehost_addr);
			NetCS_SocketConfigHost(NULL, 		  0, 			   &localhost_addr);

			if (UDP0_TCP1_SCTP2 == 1) {
				ErrorTraceHandle(2, "Initiate TCP connection from ");
			} else if (UDP0_TCP1_SCTP2 == 2) {
				ErrorTraceHandle(2, "Initiate SCTP connection from ");
			} else if (UDP0_TCP1_SCTP2 == 0) {
				ErrorTraceHandle(2, "Send UDP messages from ");
			}
			ErrorTraceHandle(2, "%s:%d to ",
					inet_ntoa(localhost_addr.sin_addr),
					ntohs(localhost_addr.sin_port));
			ErrorTraceHandle(2, "%s:%d.\n",
					inet_ntoa(remotehost_addr.sin_addr),
					ntohs(remotehost_addr.sin_port));

			if (T.indicator != 0)
				ErrorTraceHandle(1, "Pool <%s> each %d seconds.\n", string, timer);

			if ((UDP0_TCP1_SCTP2 == 1) || (UDP0_TCP1_SCTP2 == 2)) {
				if ((NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id = NetCS_ClientSocketConnect(&remotehost_addr, &localhost_addr, UDP0_TCP1_SCTP2)) == -1)
				ErrorTraceHandle(0, "NetCS_ClientSocketConnect() Client Socket error !\n");

				PTHREAD_SEND_OVER(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id, UDP0_TCP1_SCTP2, 0, NETCS_ONE_SET);
				if (error_code = pthread_create(&NetCS_DataSet[NETCS_ONE_SET].trn_status.server.thread.thread_id, NULL,
											NetCS_Thread_ServerPortReceiveAndStoreBuffer,
											(void *) &NetCS_DataSet[NETCS_ONE_SET].trn_status.server.thread.arg) != 0) {
					NetCS_SocketClose(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id);
					ErrorTraceHandle(0, "NetCS_Thread_ServerPortReceiveAndStoreBuffer() thread failed to create !\n");
				}
			}

			while(1) {
				if (UDP0_TCP1_SCTP2 == 1) {
					if (send(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id, string, strlen(string), 0) == -1) {
				  	NetCS_SocketClose(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id);
				  	ErrorTraceHandle(0, "send() error %d (%s).\n", errno, strerror(errno));
					}
				} else if (UDP0_TCP1_SCTP2 == 2) {
					char send_string[2][MINIBUF];

					strcpy(send_string[0], plain_string);
					strcat(send_string[0], "\r\n");
					strcpy(send_string[1], "\0");

					ErrorTraceHandle(1, ">>%s", plain_string);

					NetCS_SCTPClientSendStringToHostPort(
							RemoteUser.IP,
							RemoteUser.port,
							0,
							0,
							NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
							&send_string,
							NULL,
							0,
							0,
							0);

	                sleep(1);

					NetCS_SCTPClientSendStringToHostPort(
							RemoteUser.IP,
							RemoteUser.port,
							0,
							0,
							NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
							&send_string,
							NULL,
							0,
							0,
							0);

	                sleep(1);

					NetCS_SCTPClientSendStringToHostPort(
							RemoteUser.IP,
							RemoteUser.port,
							0,
							0,
							NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
							&send_string,
							NULL,
							0,
							0,
							0);

	                sleep(1);

					NetCS_SCTPClientSendStringToHostPort(
							RemoteUser.IP,
							RemoteUser.port,
							0,
							0,
							NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
							&send_string,
							NULL,
							0,
							0,
							0);

				}

				time(&pooled_time);

READ_DATA:
				buffer_read_index = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[NETCS_ONE_SET].transport_receive.read);
				while ((!NETCS_RECEIVED_DINGDONG(NETCS_ONE_SET)) ||
					   (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, NETCS_ONE_SET, buffer_read_index))) {} // wait until buffer filled and released

				ErrorTraceHandle(1, "<<%s", NetCS_DataSet[NETCS_ONE_SET].transport_receive.buffer[buffer_read_index].data);
				NETCS_RECEIVED_CLR_STATUS(sctp, NETCS_ONE_SET, buffer_read_index, NETCS_USERDATAVALID);
				NETCS_RECEIVED_READED(sctp,	NETCS_ONE_SET);

				if (NETCS_RECEIVED_DINGDONG(NETCS_ONE_SET)) goto READ_DATA; // if there are more data


				if (timer == 0) {
					if (!flag1)
						break;
					else {
						getline(&string, &size, stdin);

						if (strstr(string, "exit"))
							break;
					}
				} else {
					while (1) {
						time(&current_time);
						passed_time = current_time-pooled_time;

						if ((passed_time!=0) && ((passed_time)%timer == 0)) {
							ErrorTraceHandle(1, "Pool again ...\n");
							break;
						}
					}
				}


			}

		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "port_file_send")) {
			char Cmds[2][MINIBUF];
			char *phost_name;
			//char *host_name = "testinst1st"; // o(host_name)x(allocated)->x=""

			if ((RemoteUser.station[0] == '\0') || (plain_string[0] == '\0') || (K.indicator == 0))
				ErrorTraceHandle(0, "\nRemote station name or ip address is missing !\n");

			phost_name = (char *) malloc(strlen(RemoteUser.station));
			strcpy(phost_name, RemoteUser.station);

			RemoteUser.port = K.int_value;

			strcpy(Cmds[0], plain_string);
			strcat(Cmds[0], "\r\n");
			strcpy(Cmds[1], "\0");

			{
				FILE *fptrace;
				char *tmp;
				char send_string[2][MINIBUF];

				if ((fptrace = fopen(plain_string, "r+")) == NULL)
				{
					ErrorTraceHandle(0, "\nFile name is missing !\n");
				}
				else
				{
					ErrorTraceHandle(1, "\nFile '%s' opened for reading.\n", plain_string);

					while ((tmp=fgets(tmp, MINIBUF * sizeof(char), fptrace)) != NULL)
					{
						ErrorTraceHandle(1, "\n ------ !\n");

						strcpy(send_string[0], tmp);
						strcat(send_string[0], "\r\n");
						strcpy(send_string[1], "\0");

						ErrorTraceHandle(1, " Send <%s> to <%s>:%d \n", send_string[0], RemoteUser.station, RemoteUser.port);

						NetCS_ClientSendStringToHostPort(phost_name, RemoteUser.port, &send_string);
					}
				}
				fclose(fptrace);
			}

			ErrorTraceHandle(1, " Finished with sending. \n");
		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "port_string_server")) {
			char *phost_name;
			int port;
			struct sockaddr_in hostAddr;
			char send_file[MAXIBUF+10];
			int  bytesreaded = 0;
			unsigned int buffer_read_index;

			if ((k.indicator == 0) || (t.indicator == 0) || (plain_string[0] == '\0'))
				ErrorTraceHandle(0, "\nServer port value, transport type or reply string is missing !\n");

			User.port 		= k.int_value;
			UDP0_TCP1_SCTP2 = t.int_value;

			ErrorTraceHandle(1, "\n\n----------------------------------------------\n");
			ErrorTraceHandle(1, "Listen UDP0_TCP1_SCTP2=%d on localhost port %2d \n", UDP0_TCP1_SCTP2, User.port);
			NetCS_SocketConfigHost(NULL, User.port, &hostAddr);
			if ((port = NetCS_ServerCreateOnPort(&hostAddr, UDP0_TCP1_SCTP2, 1, NETCS_ONE_SET)) == 0) {
				ErrorTraceHandle(0, "main() - NetCS_ServerCreateOnPort() error.\n");
			}

			/*ErrorTraceHandle(1, "main() - Accepted client from %s:%d:%d\n",
					NetCS_DataSet[NETCS_ONE_SET].trn_status.client.ip,
					NetCS_DataSet[NETCS_ONE_SET].trn_status.client.port_from_NETCS,
					NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id);*/

			ErrorTraceHandle(1, "--------------------------------------------------\n");

			if (flag1){ // -c is filename
				FILE *fptrace;
				char *tmp;
				char send_string[MINIBUF];

				if ((fptrace = fopen(plain_string, "r")) == NULL) {
					ErrorTraceHandle(0, "main() - Unable to open file <%s> for reading !\n", plain_string);
				} else {
					ErrorTraceHandle(1, "main() - File <%s> opened for reading.\n", plain_string);

					while ((tmp=fgets(send_string, MINIBUF, fptrace)) != NULL) {
						if (bytesreaded<MAXIBUF) {
							if (!bytesreaded)
								strcpy(send_file, send_string);
							else
								strcat(send_file, send_string);
						}
						bytesreaded += strlen(send_string);
					}
					strcat(send_file, "\r\n");
					ErrorTraceHandle(1, "main() - File <%s> closing.\n", plain_string);
				}
				fclose(fptrace);
			}


			while(1) {
				buffer_read_index = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[NETCS_ONE_SET].transport_receive.read);
				while ((!NETCS_RECEIVED_DINGDONG(NETCS_ONE_SET)) ||
					   (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, NETCS_ONE_SET, buffer_read_index))) {} // wait until buffer filled and released

				ErrorTraceHandle(1, ">>%s", NetCS_DataSet[NETCS_ONE_SET].transport_receive.buffer[buffer_read_index].data);
				NETCS_RECEIVED_CLR_STATUS(sctp, NETCS_ONE_SET, buffer_read_index, NETCS_USERDATAVALID);
				NETCS_RECEIVED_READED(sctp,	NETCS_ONE_SET);


				if (!flag1) { // -c is string
					strcat(plain_string, "\r\n");

					if (UDP0_TCP1_SCTP2 == 1) {
						ErrorTraceHandle(1, "main() - TCP Reply <%s> to <%s>:%d \n", plain_string, NetCS_DataSet[NETCS_ONE_SET].trn_status.client.ip, NetCS_DataSet[NETCS_ONE_SET].trn_status.client.port_from_NETCS);

						if (send(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
								plain_string,
								strlen(plain_string),
								0) == -1) {
						  NetCS_SocketClose(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id);
						  ErrorTraceHandle(0, "main() - send() error.\n");
						}
					} else if (UDP0_TCP1_SCTP2 == 2) {
						char send_string[2][MINIBUF];

						strcpy(send_string[0], plain_string);
						strcat(send_string[0], "\r\n");
						strcpy(send_string[1], "\0");

						ErrorTraceHandle(1, "<<%s", send_string[0]);

						NetCS_SCTPClientSendStringToHostPort(
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.ip,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.port_from_NETCS,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.server.port,
								0,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
								&send_string,
								NULL,
								0,
								0,
								0);
					}
				} else { // -c is filename
					if (UDP0_TCP1_SCTP2 == 1) {
						ErrorTraceHandle(1, "%s", send_file);

						if (send(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id, send_file, bytesreaded, 0) == -1) {
						  NetCS_SocketClose(NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id);
						  ErrorTraceHandle(0, "main() - send() error.\n");
						}
					} else if (UDP0_TCP1_SCTP2 == 2) {
						char send_string[2][MINIBUF];

						strcpy(send_string[0], plain_string);
						strcat(send_string[0], "\r\n");
						strcpy(send_string[1], "\0");

						ErrorTraceHandle(1, "<<%s", send_string[0]);

						NetCS_SCTPClientSendStringToHostPort(
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.ip,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.port_from_NETCS,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.server.port,
								0,
								NetCS_DataSet[NETCS_ONE_SET].trn_status.client.socket_id,
								&send_string,
								NULL,
								0,
								0,
								0);
					}
				}
			}

		}

		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "signalling")) {

			init_Hertz_value();
			ErrorTraceHandle(1, "Kernel clock tick rate is %d hertz.\n", Hertz);

			signalling_loop();
		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "oscillod")) {

			oscillo_trace_project();
		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "tgen_pool")) {

			if ((RemoteUser.IP[0] == '\0') || (K.indicator == 0) || (t.indicator == 0))
				ErrorTraceHandle(0, "\nRemote station IP, port or pooling time is missing !\n");
			RemoteUser.port = K.int_value;

			tgen_trace_project(RemoteUser.IP, K.int_value, t.int_value);
		}


		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "ftp_putfile"))
		// input [RemoteUser.file], RemoteUser.folder + file, RemoteUser.station
		{
			FTP_SelectPutfile();
		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "ftp_getdir")) {
			FTP_SelectGetdir();
		}




		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "remote_shellcmd")) {
			char Cmds[1][MINIBUF];
			char *phost_name;

			if (plain_string[0] == '\0')
				ErrorTraceHandle(0, "\nString is missing.!\n");

			if ((RemoteUser.station[0] == '\0') && (RemoteUser.IP[0] == '\0'))
				ErrorTraceHandle(0, "\nRemote station name or ip address is missing !\n");

			if (RemoteUser.station[0] != '\0') {
				phost_name = (char *) malloc(strlen(RemoteUser.station));
				strcpy(phost_name, RemoteUser.station);
			} else {
				phost_name = (char *) malloc(strlen(RemoteUser.IP));
				strcpy(phost_name, RemoteUser.IP);
			}

			strcpy(work_buffer, setenvMyDISPLAY);
			strcat(work_buffer, plain_string);

			ErrorTraceHandle(1, "Remotely executing <%s> on <%s>\n", work_buffer, phost_name);
			NetPromptHost_rexec_CmdTool(phost_name, work_buffer);
		}





		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "ethserproxy")) {
			Net_EthernetSerialProxy_Select();
		}

#ifdef _SOLARIS_
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "rwchat")) {
			char *phost_name;

			if (plain_string[0] == '\0')
				ErrorTraceHandle(0, "\nString is missing.!\n");

			if ((RemoteUser.station[0] == '\0') && (RemoteUser.IP[0] == '\0'))
				ErrorTraceHandle(0, "\nRemote station name or ip address is missing !\n");

			if (RemoteUser.station[0] != '\0') {
				phost_name = (char *) malloc(strlen(RemoteUser.station));
				strcpy(phost_name, RemoteUser.station);
			} else {
				phost_name = (char *) malloc(strlen(RemoteUser.IP));
				strcpy(phost_name, RemoteUser.IP);
			}

			RW_Select(RemoteUser.station, plain_string, flag2, flag3);
		}

		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "uploadum")) {
			UM_Select();
		}
#endif


		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "test_ser")) {
			int ttyds, value;
			struct termios tt;

			if ((ttyds = open("/dev/ttyb", O_RDWR | O_NONBLOCK)) < 0)
				ErrorTraceHandle(0,
						"termserv - open() error, can't open %s.\n",
						plain_string);

			if ((value = fcntl(ttyds, F_GETFL, 0)) < 0)
				ErrorTraceHandle(0, "fcntl error for fd %d", ttyds);

			if ((value & O_ACCMODE) == O_RDONLY)
				fprintf(stdout, "read only");
			else if ((value & O_ACCMODE) == O_WRONLY)
				fprintf(stdout, "write only");
			else if ((value & O_ACCMODE) == O_RDWR)
				fprintf(stdout, "read write");
			else
				fprintf(stdout, "unknown access mode");

			if (value & O_APPEND)
				fprintf(stdout, ", append");
			if (value & O_NONBLOCK)
				fprintf(stdout, ", nonblocking");
			if (value & O_SYNC)
				fprintf(stdout, ", synchronous writes");

			putchar('\n');

			close(ttyds);
		} // end session 11

#if 0
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "test_ethserproxytest")) {
			Net_EthernetSerialProxyTest_Select();
		}

		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "test_ftptest")) {
			FTP_SelectTest1();
		}


		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		if (!strcmp(sd[session].service_name, "test")) {
	        int fd_in, fd_out;
	        char buf[1024];

	        memset(buf, 0, 1024); /* clear buffer*/

	        fork(); /* It doesn't matter about child vs parent */

	        while (read(fd_in, buf, 2) > 0) { /* Loop through the infile */
	                printf("%d: %s", getpid(), buf);
	                /* Write a line */
	                sprintf(buf, "%d Hello, world!\n\r", getpid());
	                write(fd_out, buf, strlen(buf));
	                sleep(1);
	                memset(buf, 0, 1024); /* clear buffer*/
	        }
	        sleep(10);
	     }
#endif


	}
	exit(0);
}

// shell	stream	tcp	nowait	root	/usr/sbin/in.rshd	in.rshd
// login	stream	tcp	nowait	root	/usr/sbin/in.rlogind	in.rlogind
// exec 	stream	tcp	nowait	root	/usr/sbin/in.rexecd	in.rexecd
// comsat	dgram	udp	wait	root	/usr/sbin/in.comsat	in.comsat
// talk 	dgram	udp	wait	root	/usr/sbin/in.talkd	in.talkd


/*
 BUILD

 set CLIBS  = "-I/usr/openwin/include -L/usr/openwin/lib -L/usr/lib -lxview -lolgx -lX11 -lsocket -lnsl"
 set CMTAPP = "-lthread -lpthread -D_REENTRANT"
 set RISCWATCH = "/opt/hde/RISCWatch/5.1/rwpi/rwpic.o"
 set RISCWATCH_add = "-L/usr/dt/lib"

 gcc rj.c $RISCWATCH -o rj $CLIBS $CMTAPP $RISCWATCH_add
 */

/*
 // The /dev/tty1 device corresponds with your first console. When you read from it, you will
 // read from its keyboard buffer, and when you write to it, you will write to its screen.
 // /dev/tty2 corresponds to the second console, and so on. /dev/tty and /dev/tty0 are a bit
 // different: they correspond to the current console.

 */
/*
 #include <stdio.h>

 #define MAILPROG "/bin/mail"

 int main()
 {
 FILE *mail = popen(MAILPROG " -s 'Test Message' franeb@st.tel.hr", "w");
 if (!mail)
 {
 perror("popen");
 exit(1);
 }

 fprintf(mail, "This is a test.\n");

 if (pclose(mail))
 {
 fprintf(stderr, "mail failed!\n");
 exit(1);
 }
 }
 */


/*


/vobs/cello/civ/mo_scripts/tools/coco


COCO

#!/bin/tcsh -f
# source /vobs/cello/cade_struct/setup/setup
source /vobs/cello/civ/mo_scripts/tools/coco/setup

# /vobs/cello/civ/mo_scripts/tools/create_mos/cocomake.exp -node cellonode9 -cococonfig /vobs/cello/phy/admin/MOscripts/cellonode9/coco.config $argv
/vobs/cello/civ/mo_scripts/tools/coco/cocomake.exp -node cellonode31 -cococonfig /vobs/cello/phy/admin/MOscripts/node_cellonode31/coco.config $argv



COCO config fila

######################
# COCO 2.02 required #
######################

NAMEROOT="nameroot.ior"
bswhat_path="/vobs/cello/cade_struct/bin/bswhat"
motester_path="/vobs/cello/civ/mo_scripts/tools/ext_mo_tester.sh"

#######################
# Directory structure #
#######################

basedir="/vobs/cello/phy/admin/MOscripts"
commondir="/vobs/cello/aet/mo_scripts/node_config/common"
nodedir_prefix="node_"
security_dir="/vobs/cello/aet/mo_scripts/node_config/common/security"
sharp_files="targetinformation.xml,host.p12,hostattrcert.der"

#######################
# Configuration files #
#######################

software_xml="/vobs/cello/aet/mo_scripts/node_config/common/sw_config.xml"
configuration_xml="/vobs/cello/civ/mo_scripts/tools/create_mos/configuration.xml"
options_file="/vobs/cello/aet/mo_scripts/node_config/common/options.exp"

#############
# Simulator #
#############

simulator_path="/vobs/cello/civ/simcello_example/mp_lm/000300"
cvm_command="cvm -Xms6M -DSPARC=true -Dse.ericsson.security.PropertyFileLocation=/host/SecurityManagement.prp -Djava.class.path=/host/oms.jar:/host/cma.jar:/host/asms.jar:/host/vbjorb.jar: se.ericsson.cello.oms.Oms"
simulator_nameroot="/vobs/cello/civ/simcello_example/mp_lm/public_html/cello/ior_files/nameroot.ior"

#################
# Misc          #
#################

mxheap_value="77000"
mxheap_name="mxheap"
loadmodule_heappool="no"
remove_cv_name="cv-remove"
format_cv_name="cv-format"
unused_piutypes="unused_piutypes.mo"
hw_description_filename="hw.txt"
coco_debug="coco_debug.log"
result_log="result.log"
destdir="/tmp/coco"
nodetypes="MIN RBS RNC RNC-A RNC-A-OLD RNC-A-NEW RNC-C-NEW RNC-F-NEW RNC_3.5 RNC2_3.5 RXM RXI RNC-RXI SIM RBS_SIM AET"
terminal_password="iolan"
disable_clearcase_check="yes"
disable_terminal_port_killing="no"
disable_cv_remove_question="no"
default_cv_remove_answer="no"
format_node="no"
#format_node="yes"
# upload_lm="no"
# upload_mo="no"
upload_lm="yes"
upload_mo="yes"
backup_cxc_number="CXC1321445"
basic_cxc_number="CXC1321447"
netmask="255.255.248.0"
defaultroute="192.168.20.1"
update_armament="yes"
concatenate_mo_files="yes"

#################
# Scripts' name #
#################

parsemom_path="/vobs/cello/civ/mo_scripts/tools/create_mos/parsemom.tcl"
coco_lib_path="/vobs/cello/civ/mo_scripts/tools/create_mos/coco_lib.tcl"
create_traffic_lib_path="/vobs/cello/civ/mo_scripts/tools/create_mos/create_traffic_lib.tcl"
traffic_lib_ver_dep_path="/vobs/cello/civ/mo_scripts/tools/create_mos/traffic_lib_ver_dep.tcl"
create_mos_lib_path="/vobs/cello/civ/mo_scripts/tools/create_mos/create_mos_lib.tcl"
mos_lib_ver_dep_path="/vobs/cello/civ/mo_scripts/tools/create_mos/mos_lib_ver_dep.tcl"
get_nameroot_path="/vobs/cello/civ/mo_scripts/tools/get_nameroot.exp"
restart_path="/vobs/cello/civ/mo_scripts/tools/restart.exp"
cocomake_path="/vobs/cello/civ/mo_scripts/tools/create_mos/cocomake.exp"
configure_cello_path="/vobs/cello/civ/mo_scripts/tools/configure_cello"
cello_format_path="/vobs/cello/civ/mo_scripts/tools/cello_format.exp"
locoload_path="/vobs/cello/civ/test_utilities/loco/bin/locoload.exp"

# Error in the file CelloMOM.xml
# Corrected "BLACK" CelloMOM.xml for TB1-CELLO_4.2-1 stored in sysver/black_lm
mom_path="/vobs/cello/cma/cma_uml/CelloMOM.xml"
#mom_path="/vobs/cello/civ/sysver/black_lm/CelloMOM.xml"
upgrade_cello_path="/vobs/cello/civ/config_target/upgrade_cello.exp"
#filelist_path="/vobs/cello/civ/config_target/filelist.txt"
filelist_path="/vobs/cello/aet/mo_scripts/node_config/common/filelist.txt"

*/





/*

registrirat se na signal i na njega izvrsit terminate();
 signal(SIGTERM,terminate);
void terminate(void)
{

}

*/



/*



	  if (tcgetattr(ttyds, &ttyterm) != 0)
	    ErrorTraceHandle(0, "Could not get port attributes.\n");

	  ttyterm.c_cflag = (CLOCAL |  // set local
			     CREAD     // enable the receiver
			     );

	  // Set 8 bit characters, no parity, 1 stop (8N1)
	  ttyterm.c_cflag &= ~PARENB;        // Clear previous parity bits values
	  ttyterm.c_cflag &= ~CSTOPB;        // Clear previous stop bits values
	  ttyterm.c_cflag &= ~CSIZE;         // Clear previous charsize bits values
	  ttyterm.c_cflag |= CS8;            // Set for 8 bits.

	  ttyterm.c_cflag &= ~CRTSCTS;       // disable hardware flow control

	  ttyterm.c_iflag = IGNPAR;
	  ttyterm.c_iflag &= ~(IXON | IXOFF | IXANY); // disable software flow control

	  ttyterm.c_oflag &= ~OPOST; // output filtering to raw

	  ttyterm.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // completely raw access

	  ttyterm.c_cc[VMIN] = 0;  // inter-character timer unused
	  ttyterm.c_cc[VTIME] = 0; // nonblocking read


O_NDELAY: Don't wait for DCD signal line when opening the port


	  ioctl(ttyds, TCGETA, &ttyterm);

	  ttyterm.c_iflag = 0;
	  ttyterm.c_oflag = 0;

	  ttyterm.c_cflag = (B9600 |
			     CS8   |
			     CREAD |
			     CLOCAL);

	  ttyterm.c_lflag = 0;

	  ioctl(ttyds, TCSETA, &ttyterm);
	  ioctl(ttyds, TCFLSH, 2);



DCD - data carrier detect

*/
