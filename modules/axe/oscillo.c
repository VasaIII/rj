#define OSCILLO_TRACE_PROJECT
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/trace/trace.h"
#include "modules/axe/trace/configfile_parser.h"
#include "modules/axe/trace/configfile_parser.y.tab.h"
#undef OSCILLO_TRACE_PROJECT

void oscillo_trace_project(void ) {
	struct sockaddr_in hostAddr;
	unsigned int buffer_read_index;

	NetCS_SocketConfigHost(NULL, 6666, &hostAddr);
	NetCS_ServerCreateOnPort(&hostAddr, 1, 1, NETCS_ONE_SET);
	ErrorTraceHandle(1, "oscillo_trace_project() Application started, project requests are listening on port 6666.\n");

	yyNetCS_set = NETCS_ONE_SET;
	while (true) {
		  Trace_ProjectReset();
		  Trace_ProjectDataAnalisysReset();
		  ErrorTraceHandle(1, "oscillo_trace_project() Trace project resetted.\n");

		  buffer_read_index = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[yyNetCS_set].transport_receive.read);
		  while ((!NETCS_RECEIVED_DINGDONG(yyNetCS_set)) ||
				 (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, yyNetCS_set, buffer_read_index))) {} // wait until buffer filled and released

		  yyinput_analyse_stop0_file1_buffer2 = 2; // parse project trigger data from socket buffer
		  ErrorTraceHandle(1, "oscillo_trace_project() Parsing trigger project request from socket ... starting\n");
		  yytraceparse();
		  ErrorTraceHandle(1, "oscillo_trace_project() Parsing trigger project request from socket ... finished\n");

		  yyinput_analyse_stop0_file1_buffer2 = 1; // parsing data from file
		  strcpy(yyparseFileName, Project.EditedFile[0].TriggerProjectInfo.TraceConfigFileName);
		  ErrorTraceHandle(1, "oscillo_trace_project() Parsing configuration info from file ... starting\n");
		  yytraceparse();
		  ErrorTraceHandle(1, "oscillo_trace_project() Parsing configuration info from file ... finished\n");

		  if (!Trace_ProjectOperation(0))
			  ErrorTraceHandle(1, "oscillo_trace_project() Trace analisys failed !\n");
		  else
			  ErrorTraceHandle(1, "oscillo_trace_project() Trace analisys successfull !\n");

		  ErrorTraceHandle(1, "oscillo_trace_project() Sending analysed data ... starting\n");

		  Trace_StoreToTableInFile(1);

		  ErrorTraceHandle(1, "oscillo_trace_project() Sending analysed data ... finished\n");
	}

}

