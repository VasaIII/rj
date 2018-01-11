#include <stdio.h>
#include <stdlib.h> // random()
#include <string.h>

// UAB include for RiscWatch header (.o file needs to be included in compiler)
#ifdef SPLIT
 #include "/home4/etkcello/DDE/RiscWatch/5.1/rwpi/rwpic.h"
#endif
#ifdef ZAGREB
 #include "/TCM/PROJECTS/Cello/software/Riskwatch/5.1/rwpi/rwpic.h"
#endif
#ifdef ALVSJO
 #include "/opt/hde/RISCWatch/5.1/rwpi/rwpic.h"
#endif

static void
RW_connect(char *rw_server);
static void
RW_disconnect(void);
static void
RW_conn_test(void);
static int
RW_process_cmd(char *cmd_str);
static void
RW_cmd_status(void);
static int
RW_execute_cmd(char *cmd_str);
static int
RW_process_cmd(char *cmd_str);


// ***********************************************************************************
//
// RiscWatch connect
//
// ***********************************************************************************
static void
RW_connect(char *rw_server)
{
  RWPIC_RC rc;   /* return code */

  ErrorTraceHandle(2,"ENTER: RW_connect\n");

  ErrorTraceHandle(2,"Connecting to %s...\n", rw_server);
  rc = RWPIC_Intf_Attach(rw_server, 6480);

  if (rc != RWPIC_OK)
    {
      ErrorTraceHandle(1,"Unable to connect! Code <%d>\n", rc);
      ErrorTraceHandle(1,"Reason: RISCWatch probably not started on remote server\n");
      ErrorTraceHandle(1,"Reason: Another program is connected to RISCWatch\n\n");
      exit(1);
    }

   ErrorTraceHandle(2,"   Connected to RW server ");
   if (RWPIC_Server_Get_HostName())
     ErrorTraceHandle(2,"%s", RWPIC_Server_Get_HostName());
   if (RWPIC_Server_Get_HostName() &&
       RWPIC_Server_Get_IPAddr())
     ErrorTraceHandle(2,":");
   if (RWPIC_Server_Get_IPAddr())
     ErrorTraceHandle(2,"%s", RWPIC_Server_Get_IPAddr());
   ErrorTraceHandle(2,"\n\n");

   // RW_conn_test();

   atexit(RW_disconnect); // registers function to be called when program is exited
}

// ***********************************************************************************
//
// RiscWatch disconnect
//
// ***********************************************************************************
static void
RW_disconnect(void)
{
   ErrorTraceHandle(2,"ENTER: RW_disconnect\n");

   RWPIC_Intf_Detach();

   ErrorTraceHandle(2,"Done!\n\n");
}

// ***********************************************************************************
//
// RiscWatch test connection
//
// ***********************************************************************************
static void
RW_cmd_test(void)
{
   ErrorTraceHandle(2,"ENTER: RW_cmd_test\n");

   ErrorTraceHandle(1,"Testing connection...");
   if (RWPIC_Intf_Test() == RWPIC_OK)
     ErrorTraceHandle(1,"passed!\n\n");
   else
   {
     ErrorTraceHandle(1,"failed!\n\n");
     exit(1);
   }
}

// ***********************************************************************************
//
// RiscWatch cmd status
//
// ***********************************************************************************
static void
RW_cmd_status(void)
{
   RWPIC_RC   rc;         /* return code */
   RWPIC_STAT stat_rec;   /* status record */

   ErrorTraceHandle(2,"ENTER: RW_cmd_status\n");

   rc = RWPIC_Status_Get(&stat_rec);
   if (rc != RWPIC_OK)
     ErrorTraceHandle(1,"   error getting status\n");
   else
   {
      switch ((RWPIC_PROC_STAT)stat_rec.proc_stat)
      {
         case RWPIC_PROC_STAT_CHKSTOP:
            ErrorTraceHandle(1,"   processor is check-stopped\n");
            break;
         case RWPIC_PROC_STAT_HALTED:
            ErrorTraceHandle(1,"   processor is halted\n");
            break;
         case RWPIC_PROC_STAT_POWEROFF:
            ErrorTraceHandle(1,"   processor is powered off\n");
            break;
         case RWPIC_PROC_STAT_RUNNING:
            ErrorTraceHandle(1,"   processor is running\n");
            break;
         case RWPIC_PROC_STAT_STOPPED:
            ErrorTraceHandle(1,"   processor is stopped\n");
            break;

         default:
            ErrorTraceHandle(1,"   unknown status!\n");
            break;
      }
   }
}

// ***********************************************************************************
//
// RiscWatch execute cmd
//
// ***********************************************************************************
static int
RW_execute_cmd(char *cmd_str)
{
   RWPIC_RC frc;      /* functional return code */
   RWPIC_RC resp_rc;  /* response return code */
   int      rc = 1;   /* return code */
   int      binary;
   char     resp_str[1000];   /* response string */
   char    *resp_str_p = resp_str;

   ErrorTraceHandle(2,"ENTER: RW_execute_cmd\n");

   frc = RWPIC_Cmd_Exec(cmd_str);
   if (frc == RWPIC_OK)
   {
      if (strstr(cmd_str, "quit") == cmd_str)
      {
         ErrorTraceHandle(1,"RW terminated\n");
         rc = 0;
      }
      else
      {

         frc = RWPIC_Cmd_Recv_Data(&resp_rc, resp_str);

         if ((strstr(cmd_str, "assm ") == cmd_str) ||
             (strstr(cmd_str, "dis " ) == cmd_str) ||
             (strstr(cmd_str, "expr ") == cmd_str) ||
             (strstr(cmd_str, "readh ") == cmd_str) ||
             (strstr(cmd_str, "read ") == cmd_str))
         {
            if (frc == RWPIC_OK)
            {
               if (resp_rc == RWPIC_OK)
		 {
		   ErrorTraceHandle(2,"Result = %s\n", resp_str);
		   fprintf(stdout, "%s", resp_str, binary);
		   if (strstr(cmd_str, "readh ") == cmd_str)
		     {
		       HexString2Int(&resp_str_p, &binary);
		       fprintf(stdout, " - ");
		       Int2BinaryString(binary);
		     }
		 }
               else
		 ErrorTraceHandle(1,"Error = %s\n", resp_str);
            }
            else
	      ErrorTraceHandle(1,"Error receiving response\n");
         }
         else
         {
            if (frc == RWPIC_OK)
            {
               if (resp_rc == RWPIC_OK)
		 {
		   //ErrorTraceHandle(1,"Command successful\n");
		 }
               else
		 ErrorTraceHandle(1,"Error = %s\n", resp_str);
            }
            else
	      ErrorTraceHandle(1,"Error receiving response\n");
         }
      }
   }
   else
     ErrorTraceHandle(1,"Error executing command\n");

   return(rc);
}

// ***********************************************************************************
//
// RiscWatch process cmd
//
// ***********************************************************************************
static int
RW_process_cmd(char *cmd_str)
{
   int rc = 1;   /* return code */

   ErrorTraceHandle(2,"ENTER: RW_process_cmd <%s>\n", cmd_str);

   if (cmd_str[0])                  /* if a command string was entered */
   {
      if (!strcmp(cmd_str, "status"))
         RW_cmd_status();
      else if (!strcmp(cmd_str, "test"))
         RW_cmd_test();
      else
	rc = RW_execute_cmd(cmd_str);
   }
   else rc = 0;                     /* empty string means quit program */

   return(rc);
}


void RW_Select(char *host_station, char *plain_string, int session8_flag_doublehex, int session8_flag_supermapper)
{
	  unsigned int hex_counter, hex_number_actions, hex_normal, hex_double, hex_bank, hex_skip_translation;
	  char hex_double_string[100], hex_string[100];
	  char *hex_double_string_p = hex_double_string, *hex_string_p = hex_string;
	  char plain_string2[1000], plain_string3[1000];

	  char plain_string_hex[100][1000];

	  if (plain_string[0] == '\0')
	    ErrorTraceHandle(0, "\nCommand string missing !\n");

	  strcpy(plain_string_hex[0], plain_string); // Copy plain_string from arguments to working one
	  hex_number_actions = 1;                    // default number of actions

	  RW_connect(host_station);

	  ErrorTraceHandle(2, "\nExecuting command <%s>\n", plain_string_hex[0]);

	  // SHORTCUT to read one register
	  if (!strncmp(plain_string_hex[0], "R ", 2))
	    {
	      sprintf(plain_string2, "readh %s", (plain_string_hex[0], 0, 0, 1));
	      strcpy(plain_string_hex[0], plain_string2);
	    }
	  // SHORTCUT to read range of registers
	  else if (!strncmp(plain_string_hex[0], "RR ", 3))
	    {
	      unsigned int range_val1=0, range_val2=0;

	      strcpy(hex_string, (const char *) (plain_string_hex[0], 0, 0, 1)); // read first range value
	      HexString2Int (&hex_string_p, &range_val1);

	      strcpy(hex_string, (const char *) (plain_string_hex[0], 0, 0, 2)); // read second range value
	      HexString2Int (&hex_string_p, &range_val2);

	      if (abs(range_val1-range_val2) > 100)
		ErrorTraceHandle(0, "\nMaximum reading range is 100 addresses !\n");
	      else if (range_val2 <= range_val1)
		ErrorTraceHandle(0, "\nSecond range address is smaller or equal to first range address !\n");

	      hex_number_actions = abs(range_val1-range_val2) + 1;

	      for (hex_counter = 0; hex_counter < hex_number_actions; hex_counter++)
		{
		  sprintf(plain_string_hex[hex_counter], "readh 0x%x", range_val1);

		  if(session8_flag_doublehex)
		    range_val1 +=2;
		  else
		    range_val1 ++;
		}
	    }
	  // SHORTCUT to write to one register
	  else if (!strncmp(plain_string_hex[0], "W ", 2))
	    {
	      strcpy(plain_string3, (const char *) (plain_string_hex[0], 0, 0, 1));
	      sprintf(plain_string2, "writeh dmem %s %s", plain_string3, (const char *) GetWord(plain_string_hex[0], 0, 0, 2));
	      strcpy(plain_string_hex[0], plain_string2);
	    }
	  // SHORTCUT to write to range of registers
	  else if (!strncmp(plain_string_hex[0], "WR ", 2))
	    {
	      unsigned int range_val1=0, range_val2=0;

	      strcpy(hex_string, (const char *) GetWord(plain_string_hex[0], 0, 0, 1)); // read first range value
	      HexString2Int (&hex_string_p, &range_val1);

	      strcpy(hex_string, (const char *) GetWord(plain_string_hex[0], 0, 0, 2)); // read second range value
	      HexString2Int (&hex_string_p, &range_val2);

	      strcpy(plain_string3, (const char *) GetWord(plain_string_hex[0], 0, 0, 3)); // read value to write for all

	      if (abs(range_val1-range_val2) > 100)
		ErrorTraceHandle(0, "\nMaximum reading range is 100 addresses !\n");
	      else if (range_val2 <= range_val1)
		ErrorTraceHandle(0, "\nSecond range address is smaller or equal to first range address !\n");

	      hex_number_actions = abs(range_val1-range_val2) + 1;

	      for (hex_counter = 0; hex_counter < hex_number_actions; hex_counter++)
		{
		  sprintf(plain_string_hex[hex_counter], "writeh dmem 0x%x %s", range_val1, plain_string3);

		  if(session8_flag_doublehex)
		    range_val1 +=2;
		  else
		    range_val1 ++;
		}
	    }
	  // SHORTCUT to load UM lm
	  else if (!strcmp(plain_string_hex[0], "lum"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "reset core");
	      strcpy(plain_string_hex[2], "exec /vobs/cello/phy2/UMDRV-SWB_CNX9011339/test/tbin/dbc_init.cmd");
	      strcpy(plain_string_hex[3], "exec /vobs/cello/phy2/UMDRV-SWB_CNX9011339/test/tbin/rwppc_UM_load.cmd");
	      strcpy(plain_string_hex[4], "run");

	      hex_number_actions = 5;
	    }
	  // SHORTCUT to load UM lm
	  else if (!strcmp(plain_string_hex[0], "lum-st"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "reset core");
	      strcpy(plain_string_hex[2], "exec /home4/etkcello/etkfrbi/ET-C41/tbin/dbc_init.cmd");
	      strcpy(plain_string_hex[3], "exec /home4/etkcello/etkfrbi/ET-C41/tbin/rwppc_UM_load.cmd");
	      strcpy(plain_string_hex[4], "run");

	      hex_number_actions = 5;
	    }
	  // SHORTCUT to load SM Cello4 lm
	  else if (!strcmp(plain_string_hex[0], "lsm4"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "reset core");
	      strcpy(plain_string_hex[2], "exec /vobs/cello/phy2/phy2_subsys/smdrv_swb/smdrvproc_swu/tb/tbin/dbc_init.cmd");
	      strcpy(plain_string_hex[3], "set EBC0_B1CR = 0xC181A000");
	      strcpy(plain_string_hex[4], "set EBC0_B1AP = 0x7f868480");
	      strcpy(plain_string_hex[5], "set EBC0_CFG = 0xB8400000");
	      strcpy(plain_string_hex[6], "reset core");
	      strcpy(plain_string_hex[7], "load bin /vobs/cello/phy2/phy2_subsys/smdrv_swb/smdrvproc_swu/tb/tbin/bin/CHIP.bin 0x850000");
	      strcpy(plain_string_hex[8], "load file /vobs/cello/phy2/phy2_subsys/smdrv_swb/smdrvproc_swu/tb/tbin/smdrv.ppc405.elf nozero");
	      strcpy(plain_string_hex[9], "bp set 0x3804b8"); // Cs_earlyErrorHandler_v()
	      strcpy(plain_string_hex[10], "bp set 0x38227c"); // Cs_kernelErrorHandler_b()

	      strcpy(plain_string_hex[11], "run");

	      hex_number_actions = 12;
	    }
	  // SHORTCUT to load SM Cello4 lm
	  else if (!strcmp(plain_string_hex[0], "lsm4-st"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "reset core");
	      strcpy(plain_string_hex[2], "exec /home4/etkcello/etkfrbi/ET-MC41/tbin/dbc_init.cmd");
	      strcpy(plain_string_hex[3], "set EBC0_B1CR = 0xC181A000");
	      strcpy(plain_string_hex[4], "set EBC0_B1AP = 0x7f868480");
	      strcpy(plain_string_hex[5], "set EBC0_CFG = 0xB8400000");
	      strcpy(plain_string_hex[6], "reset core");
	      strcpy(plain_string_hex[7], "load bin /home4/etkcello/etkfrbi/ET-MC41/tbin/bin/CHIP.bin 0x850000");
	      strcpy(plain_string_hex[8], "load file /home4/etkcello/etkfrbi/ET-MC41/tbin/smdrv.ppc405.elf nozero");
	      strcpy(plain_string_hex[9], "bp set 0x3804b8"); // Cs_earlyErrorHandler_v()
	      strcpy(plain_string_hex[10], "bp set 0x38227c"); // Cs_kernelErrorHandler_b()

	      strcpy(plain_string_hex[11], "run");

	      hex_number_actions = 12;
	    }
	  // SHORTCUT to restart GEN_IO TACT
	  else if (!strcmp(plain_string_hex[0], "tset"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "set GEN0_RESET.RST_1 = 1");
	      strcpy(plain_string_hex[2], "set GEN0_RESET.RST_2 = 1");
	      strcpy(plain_string_hex[3], "run");

	      hex_number_actions = 4;
	    }
	  // SHORTCUT to restart GEN_IO TACT
	  else if (!strcmp(plain_string_hex[0], "tres"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "set GEN0_RESET.RST_1 = 0");
	      strcpy(plain_string_hex[2], "set GEN0_RESET.RST_2 = 0");
	      strcpy(plain_string_hex[3], "run");

	      hex_number_actions = 4;
	    }
	  // SHORTCUT to trigger external PMRST
	  else if (!strcmp(plain_string_hex[0], "pmrst"))
	    {
	      strcpy(plain_string_hex[0], "stop");
	      strcpy(plain_string_hex[1], "readh 0xc60d00f6");
	      strcpy(plain_string_hex[2], "readh 0xc60d00f7");
	      strcpy(plain_string_hex[3], "set GEN0_IO_B.GEN_08 = 1");
	      strcpy(plain_string_hex[4], "set GEN0_IO_B.GEN_08 = 1");
	      strcpy(plain_string_hex[5], "set GEN0_IO_B.GEN_08 = 0");
	      strcpy(plain_string_hex[6], "set GEN0_IO_B.GEN_08 = 0");
	      strcpy(plain_string_hex[7], "readh 0xc60d00f6");
	      strcpy(plain_string_hex[8], "readh 0xc60d00f7");
	      strcpy(plain_string_hex[9], "run");

	      hex_number_actions = 10;
	    }
	  // SHORTCUT to test AUG
	  else if (!strcmp(plain_string_hex[0], "test_aug"))
	    {
	      int count = 0;

	      strcpy(plain_string_hex[count++], "stop");

	      strcpy(plain_string_hex[count++], "PRINT SET COW            [UMPR_GCR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000015 0x0401");
	      strcpy(plain_string_hex[count++], "readh 0xc6000015");

	      strcpy(plain_string_hex[count++], "PRINT \nreset BER high counter [UMPR_GCR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000023 0x0000");
	      strcpy(plain_string_hex[count++], "PRINT reset BER low counter  [UMPR_GCR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000024 0x0000");

	      strcpy(plain_string_hex[count++], "PRINT \nCLEAR local RPS delta    [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc60d0006 0x0006");
	      strcpy(plain_string_hex[count++], "readh 0xc60d0006");
	      strcpy(plain_string_hex[count++], "PRINT CLEAR remote RPS delta    [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xca0d0006 0x0006");
	      strcpy(plain_string_hex[count++], "readh 0xc60d0006");
	      strcpy(plain_string_hex[count++], "PRINT CLEAR BER over TPS [TMUX_TERRINS_CTLA]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc60d00d2 0x0000");
	      strcpy(plain_string_hex[count++], "readh 0xc60d00d2");

	      strcpy(plain_string_hex[count++], "PRINT one shot BER HIGH  [UMPR_GTR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000010 0x0200");
	      strcpy(plain_string_hex[count++], "readh 0xc6000010");
	      strcpy(plain_string_hex[count++], "PRINT one shot BER LOW   [UMPR_GTR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000010 0x0000");
	      strcpy(plain_string_hex[count++], "readh 0xc6000010");

	      strcpy(plain_string_hex[count++], "PRINT \nSET BER and REI over TPS   [TMUX_TERRINS_CTLA]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc60d00d2 0x000C");
	      strcpy(plain_string_hex[count++], "readh 0xc60d00d2");

	      strcpy(plain_string_hex[count++], "PRINT one shot BER HIGH  [UMPR_GTR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000010 0x0200");
	      strcpy(plain_string_hex[count++], "readh 0xc6000010");

	      strcpy(plain_string_hex[count++], "PRINT \nset COR            [UMPR_GCR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000015 0x0403");
	      strcpy(plain_string_hex[count++], "readh 0xc6000015");

	      strcpy(plain_string_hex[count++], "PRINT \nCheck LocalLoopError:");
	      strcpy(plain_string_hex[count++], "PRINT MATE1 B2 error,  bit 14 should be 1 [STSXC_MATE_EVENT1]");
	      strcpy(plain_string_hex[count++], "PRINT MATE1 REI error, bit 11 should be 1 [STSXC_MATE_EVENT1]");
	      strcpy(plain_string_hex[count++], "readh 0xc6110003");
	      strcpy(plain_string_hex[count++], "PRINT MATE1 B2 error count,  should be >1 [STSXC_B2ERR_MATE1]");
	      strcpy(plain_string_hex[count++], "readh 0xc611000c");
	      strcpy(plain_string_hex[count++], "PRINT MATE1 REI error count, should be >1 [STSXC_M1ERR_MATE1]");
	      strcpy(plain_string_hex[count++], "readh 0xc611000F");

	      strcpy(plain_string_hex[count++], "PRINT \nCheck if local has detected REI as result of inserting B2:");
	      strcpy(plain_string_hex[count++], "PRINT RPS REI error, bit 1 should be 1 [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "readh 0xc60d0006");

	      strcpy(plain_string_hex[count++], "PRINT \nCheck LosLofLoc:");
	      strcpy(plain_string_hex[count++], "PRINT RPS REI error, one of bit 3,4,5 should be 1 [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "readh 0xc60d0006");
	      strcpy(plain_string_hex[count++], "PRINT RPS REI error, one of bit 3,4,5 should be 1 [TMUX_RPS_STATE]");
	      strcpy(plain_string_hex[count++], "readh 0xc60d003C");

	      strcpy(plain_string_hex[count++], "PRINT \nCheck if remote RPS detected B2 error as result of inserting B2:");
	      strcpy(plain_string_hex[count++], "PRINT RPS B2 error,  bit 2 should be 1 [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "readh 0xca0d0006");
	      strcpy(plain_string_hex[count++], "PRINT \nCheck if remote RPS detected REI error as result of inserting REI:");
	      strcpy(plain_string_hex[count++], "PRINT RPS REI error, bit 1 should be 1 [TMUX_RPS_DLT]");
	      strcpy(plain_string_hex[count++], "readh 0xca0d0006");

	      strcpy(plain_string_hex[count++], "PRINT \nCLEAR BER over TPS [TMUX_TERRINS_CTLA]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc60d00d2 0x0000");
	      strcpy(plain_string_hex[count++], "readh 0xc60d00d2");

	      strcpy(plain_string_hex[count++], "PRINT one shot BER LOW   [UMPR_GTR]");
	      strcpy(plain_string_hex[count++], "writeh dmem 0xc6000010 0x0000");
	      strcpy(plain_string_hex[count++], "readh 0xc6000010");

	      hex_number_actions = count++;
	    }

	  for (hex_counter = 0; hex_counter < hex_number_actions; hex_counter++)
	    {
	      hex_skip_translation = 0;

	      // Check what is current active command BEFORE number translations
	      if (!strncmp(plain_string_hex[hex_counter], "writeh dmem ", 12))
		strcpy(hex_double_string, (const char *) GetWord(plain_string_hex[hex_counter], 0, 0, 2));
	      else if (!strncmp(plain_string_hex[hex_counter], "readh ", 5))
		strcpy(hex_double_string, (const char *) GetWord(plain_string_hex[hex_counter], 0, 0, 1));
	      else if(!strncmp(plain_string_hex[hex_counter], "PRINT", 5)) // FORMAT: "PRINT _____"
		{
		  ErrorTraceHandle(1, "\n%s", plain_string_hex[hex_counter]+6);
		  continue;
		}
	      else if(!strncmp(plain_string_hex[hex_counter], "SLEEP", 5)) // FORMAT: "SLEEP"
		{
		  sleep(1);
		  ErrorTraceHandle(1, "\nSLEEPING 1");
		  continue;
		}
	      else
		{
		  hex_skip_translation = 1;
		  ErrorTraceHandle(1, "\nExecuting command <%s>\n", plain_string_hex[hex_counter]);
		}

	      if (!hex_skip_translation)
		if (session8_flag_doublehex) // Enter with hex_normal[] or hex_double and get other one on output
		  {
		    ErrorTraceHandle(2, "\nAddress in double hex <%s>\n", hex_double_string);

		    HexString2Int (&hex_double_string_p, &hex_double);
		    ErrorTraceHandle(2, "\nAddress in double hex <%x>\n", hex_double);

		    if (session8_flag_supermapper)
		      {
			hex_normal =   (hex_double & 0xFFFFF) /2;
			hex_bank   = hex_double & 0xFFF00000;
			hex_normal = hex_bank | hex_normal;
		      }
		    else // UltraMapper
		      {
			hex_normal = (hex_double & 0xFFFFFF) /2;
			hex_bank   = hex_double & 0xFF000000;
			hex_normal = hex_bank | hex_normal;
		      }

		    ErrorTraceHandle(2, "\nAddress in normal hex <%x>\n", hex_normal);
		  }
		else
		  {
		    ErrorTraceHandle(2, "\nAddress in normal hex <%s>\n", hex_double_string);

		    HexString2Int (&hex_double_string_p, &hex_normal);
		    ErrorTraceHandle(2, "\nAddress in normal hex <%x>\n", hex_normal);

		    if (session8_flag_supermapper)
		      {
			hex_double =   (hex_normal & 0xFFFFF) *2;
			hex_bank   = hex_normal & 0xFFF00000;
			hex_double = hex_bank | hex_double;
		      }
		    else // UltraMapper
		      {
			hex_double =  (hex_normal & 0xFFFFFF) *2;
			hex_bank   = hex_normal & 0xFF000000;
			hex_double = hex_bank | hex_double;
		      }

		    ErrorTraceHandle(2, "\nAddress in double hex <%x>\n", hex_double);
		  }

	      // Check what is current active command AFTER number translations
	      if (!strncmp(plain_string_hex[hex_counter], "writeh dmem ", 12))
		{
		  fprintf(stdout, "\n%X - %X", hex_double, hex_normal); // print addresses
		  strcpy(plain_string3, (const char *) GetWord(plain_string_hex[hex_counter], 0, 0, 3));  // get value
		  sprintf(plain_string_hex[hex_counter], "writeh dmem 0x%x %s", hex_double, plain_string3);
		  ErrorTraceHandle(2, "\nExecuting modyfied command <%s>\n", plain_string_hex[hex_counter]);
		}
	      else if (!strncmp(plain_string_hex[hex_counter], "readh ", 5))
		{
		  fprintf(stdout, "\n%X - %X - ", hex_double, hex_normal);
		  sprintf(plain_string_hex[hex_counter], "readh 0x%x", hex_double);
		  ErrorTraceHandle(2, "\nExecuting modyfied command <%s>\n", plain_string_hex[hex_counter]);
		}

	      RW_process_cmd(plain_string_hex[hex_counter]);

	      if (hex_counter == (hex_number_actions-1))
		fprintf(stdout, "\n\n");
	    }
	  // All commands completed

}
