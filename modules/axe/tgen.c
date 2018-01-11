
#define TGEN_TRACE_PROJECT
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/trace/trace.h"
#include "modules/axe/trace/configfile_parser.h"
#undef TGEN_TRACE_PROJECT


void tgen_trace_project(char *tgenIP, int tgenport, int pooltime) {
	struct sockaddr_in hostAddr;
	time_t current_time, pooled_time, passed_time;
	struct sockaddr_in localhost_addr;
	struct sockaddr_in remotehost_addr;
	int    tgen_sock_sd, count, error_code;
	char   poolCmds[2][MINIBUF] = {"print\r\n"};
	unsigned int buffer_read_index;

	Trace_ProjectReset();
	ErrorTraceHandle(2, "tgen_trace_project() Trace project reseted and started for TGEN on %s:%d with pooling time %d.\n",
			tgenIP, tgenport, pooltime);


	yyinput_analyse_stop0_file1_buffer2 = 1; // parse data from tgen.config file
	strcpy(yyparseFileName, "tgen.trigger");
	ErrorTraceHandle(2, "tgen_trace_project() Parsing trigger project request from %s ... starting\n", yyparseFileName);
	yytraceparse();
	ErrorTraceHandle(2, "tgen_trace_project() Parsing trigger project ... finished\n");
	yyinput_analyse_stop0_file1_buffer2 = 1; // parse data from tgen.config file
	strcpy(yyparseFileName, Project.EditedFile[0].TriggerProjectInfo.TraceConfigFileName);
    ErrorTraceHandle(2, "tgen_trace_project() Parsing configuration info from file %s ... starting\n", Project.EditedFile[0].TriggerProjectInfo.TraceConfigFileName);
    yytraceparse();
    ErrorTraceHandle(2, "tgen_trace_project() Parsing configuration info from file ... finished\n");

	ErrorTraceHandle(2, "tgen_trace_project() Initiate parsing of TGEN output with config file <%s>\n",
			Project.EditedFile[0].TriggerProjectInfo.TraceConfigFileName);




	NetCS_SocketConfigHost(NULL,   0, 		 &localhost_addr);
	NetCS_SocketConfigHost(tgenIP, tgenport, &remotehost_addr);
	if ((tgen_sock_sd = NetCS_ClientSocketConnect(&remotehost_addr, &localhost_addr, 1)) == -1)
		ErrorTraceHandle(0, "tgen_trace_project() Can not connect to remote destination.\n");

	yyNetCS_set = NETCS_ONE_SET;
	PTHREAD_SEND_OVER(tgen_sock_sd, 1, 0, yyNetCS_set);
	if (error_code = pthread_create(&NetCS_DataSet[yyNetCS_set].trn_status.server.thread.thread_id,
									NULL,
									NetCS_Thread_ServerPortReceiveAndStoreBuffer,
									(void *) &NetCS_DataSet[yyNetCS_set].trn_status.server.thread.arg) != 0) {
		NetCS_SocketClose(NetCS_DataSet[yyNetCS_set].trn_status.client.socket_id);
		ErrorTraceHandle(0, "tgen_trace_project() NetCS_Thread_ServerPortReceiveAndStoreBuffer() thread failed to create !.\n");
	}

	while (1)
	{
		ErrorTraceHandle(2, "tgen_trace_project() pooling ...\n");
		Trace_ProjectDataAnalisysReset();

		count = 0;
		while (1) {
			ErrorTraceHandle(2, "tgen_trace_project() SEND[%d]<%s>\n", count,  poolCmds[count]);
			if (send(tgen_sock_sd, poolCmds[count], strlen(poolCmds[count]), 0) == -1) {
			  ErrorTraceHandle(0, "tgen_trace_project() send() error %d (%s).\n", errno, strerror(errno));
			  NetCS_SocketClose(tgen_sock_sd);
			}
		  count ++; // next command
		  if (poolCmds[count][0] == '\0')
			break;
		}
		time(&pooled_time);

	    	Project.EditedFile[0].TriggerProjectInfo.DataFromSocket_SizeInBytes = 0;
READ_DATA:
		buffer_read_index = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[yyNetCS_set].transport_receive.read);
		while ((!NETCS_RECEIVED_DINGDONG(yyNetCS_set)) ||
			   (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, yyNetCS_set, buffer_read_index))) {} // wait until buffer filled and released

		if (Project.EditedFile[0].TriggerProjectInfo.DataFromSocket_SizeInBytes == 0)
			strncpy(Project.EditedFile[0].TriggerProjectInfo.DataFromSocket,
				NetCS_DataSet[yyNetCS_set].transport_receive.buffer[buffer_read_index].data,
				NetCS_DataSet[yyNetCS_set].transport_receive.buffer[buffer_read_index].size);
		else
			strncat(Project.EditedFile[0].TriggerProjectInfo.DataFromSocket,
				NetCS_DataSet[yyNetCS_set].transport_receive.buffer[buffer_read_index].data,
				NetCS_DataSet[yyNetCS_set].transport_receive.buffer[buffer_read_index].size);
		Project.EditedFile[0].TriggerProjectInfo.DataFromSocket_SizeInBytes += NetCS_DataSet[yyNetCS_set].transport_receive.buffer[buffer_read_index].size;

		Project.EditedFile[0].TriggerProjectInfo.DataFromSocket[Project.EditedFile[0].TriggerProjectInfo.DataFromSocket_SizeInBytes] = '\0';
		    NETCS_RECEIVED_CLR_STATUS(tcp, yyNetCS_set, buffer_read_index, NETCS_USERDATAVALID);
		    NETCS_RECEIVED_READED(tcp, yyNetCS_set);

	    	ErrorTraceHandle(2, "tgen_trace_project() RECEIVED <%s>, size=%d\n",
				Project.EditedFile[0].TriggerProjectInfo.DataFromSocket,
				Project.EditedFile[0].TriggerProjectInfo.DataFromSocket_SizeInBytes);

	    	if (NETCS_RECEIVED_DINGDONG(NETCS_ONE_SET)) goto READ_DATA; // if there are more data



	    	ErrorTraceHandle(2, "tgen_trace_project() Trace analisys started !\n");

	    	if (!Trace_ProjectOperation(1))
		    ErrorTraceHandle(2, "tgen_trace_project() Trace analisys failed !\n");
	    	else
		    ErrorTraceHandle(2, "tgen_trace_project() Trace analisys successfull !\n");

	    	ErrorTraceHandle(2, "tgen_trace_project() Storing analysed data ... starting\n");

	    	Trace_StoreToTableInFile(0);

	    	ErrorTraceHandle(2, "tgen_trace_project() Storing analysed data ... finished\n");

		while (1) {
			time(&current_time);
			passed_time = current_time-pooled_time;

			if ((passed_time!=0) && ((passed_time)%pooltime == 0)) {
				ErrorTraceHandle(2, "tgen_trace_project() pool again ...\n");
				break;
			}
		}
	}



	ErrorTraceHandle(1, "tgen_trace_project() tgen pool exit ...\n");
	NetCS_SocketClose(tgen_sock_sd);
}








