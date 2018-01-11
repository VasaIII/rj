#include <modules/common.h>
#include <modules/nethelper.h>

void UM_Select(void)
{
	  char work_buffer[500];

	  char *string1 = "cd /home3/etkfrbi/UltraMapper/umdrv_swb;rm umdrv_swb.zip;zip -R umdrv_swb '*'";
	  char *string2 = "cmdtool -I 'rm -rf /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/*;exit'";
	  char *string3 = "cmdtool -I 'mv /home7/etkfrbi/umdrv_swb/umdrv_swb.zip /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/umdrv_swb.zip;exit'";
	  char *string4 = "cmdtool -I 'cd /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb;unzip umdrv_swb.zip;exit'";
	  char *string5 = "cmdtool -I 'rm /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/umdrv_swb.zip;exit'";
	  char *string6 = "cd /home3/etkfrbi/UltraMapper/umdrv_swb;rm umdrv_swb.zip";

	  char Cmds[][MINIBUF] = {
	    "USER",
	    "PASS",
	    "CWD /home7/etkfrbi/umdrv_swb/\r\n",
	    "DELE umdrv_swb.zip\r\n",
	    "TYPE I\r\n",
	    "PASV\r\n",
	    "STOR umdrv_swb.zip\r\n",
	    "QUIT\r\n"};

	  // EXECUTE FOLLOWING:
	  //
	  // localhost% cd /home3/etkfrbi/UltraMapper/umdrv_swb
	  // localhost% if (-e umdrv_swb.zip) rm umdrv_swb.zip
	  // localhost% zip -R umdrv_swb '*.*'
	  // localhost% ftp koludarc
	  // ftp> rm /home7/etkfrbi/umdrv_swb/umdrv_swb.zip
	  // ftp> put /home3/etkfrbi/UltraMapper/umdrv_swb/umdrv_swb.zip /home7/etkfrbi/umdrv_swb/umdrv_swb.zip
	  // ftp> quit
	  // localhost% rlogin koludarc
	  // koludarc% rm -rf /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/*
	  // koludarc% mv /home7/etkfrbi/umdrv_swb/umdrv_swb.zip /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/umdrv_swb.zip
	  // koludarc% cd /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb
	  // koludarc% unzip umdrv_swb.zip
	  // koludarc% rm /view/etkfrbi_ultramapper/vobs/cello/phy/phy_subsys/umdrv_swb/umdrv_swb.zip
	  // koludarc% exit

	  char *phost_name = "fenoliga"; // host_station[]

	  char setenvMyDISPLAY[100];
	  sprintf(setenvMyDISPLAY, "xhost +;setenv DISPLAY %s:0.0;", User.station);

	  if (RemoteUser.station[0] == '\0')
	    ErrorTraceHandle(0, "\nRemote station name is missing !\n");

	  //phost_name =(char *)malloc(strlen((char *)argv[2]));
	  //strcpy(phost_name, (char *)argv[2]);

	  ErrorTraceHandle(1, "\n-------------------------------------------------\n");
	  ErrorTraceHandle(1,   "From FOLDER: /home3/etkfrbi/UltraMapper/umdrv_swb\n");
	  ErrorTraceHandle(1,   "to   FOLDER: /vobs/cello/phy/phy_subsys/umdrv_swb\n");
	  ErrorTraceHandle(1,   "-------------------------------------------------\n");
	  ErrorTraceHandle(1, "\n-------------------------------------------------------------\n");
	  ErrorTraceHandle(1,   "LOCAL zipping </umdrv_swb> folder to file <umdrv_swb.zip> ...\n");
	  ErrorTraceHandle(1,   "-------------------------------------------------------------\n\n");
	  system(string1);
	  sleep(1);
	  ErrorTraceHandle(1,   "\n-------------------------------------------\n");
	  ErrorTraceHandle(1,     "REMOTE cleaning vob folder </umdrv_swb> ...\n");
	  ErrorTraceHandle(1,     "-------------------------------------------\n");
	  strcpy(work_buffer, setenvMyDISPLAY);
	  strcat(work_buffer, string2);
	  NetPromptHost_rexec_CmdTool(phost_name, work_buffer);
	  sleep(1);
	  ErrorTraceHandle(1,   "\n-------------------------------------------------\n");
	  ErrorTraceHandle(1,     "FTP to host and transfer <umdrv_swb.zip> file ...\n");
	  ErrorTraceHandle(1,     "-------------------------------------------------\n\n");
	  FTP_flushCmds(phost_name, &Cmds);
	  sleep(1);
	  ErrorTraceHandle(1,   "\n---------------------------------------------------------------\n");
	  ErrorTraceHandle(1,     "REMOTE extracting <umdrv_swb.zip> to CC's folder <umdrv_swb>...\n");
	  ErrorTraceHandle(1,     "---------------------------------------------------------------\n");
	  strcpy(work_buffer, setenvMyDISPLAY);
	  strcat(work_buffer, string3);
	  NetPromptHost_rexec_CmdTool(phost_name, work_buffer);
	  sleep(1);
	  strcpy(work_buffer, setenvMyDISPLAY);
	  strcat(work_buffer, string4);
	  NetPromptHost_rexec_CmdTool(phost_name, work_buffer);
	  sleep(1);
	  strcpy(work_buffer, setenvMyDISPLAY);
	  strcat(work_buffer, string5);
	  NetPromptHost_rexec_CmdTool(phost_name, work_buffer);
	  sleep(1);
	  ErrorTraceHandle(1,   "\n------------------\n");
	  ErrorTraceHandle(1,     "LOCAL cleaning ...\n");
	  ErrorTraceHandle(1,     "------------------\n\n");
	  system(string6);
}
