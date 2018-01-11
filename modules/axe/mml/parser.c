
#define AXE_MMLPARSER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/trunk/isup.h>
#include <modules/axe/application/bss/bss.h>
#include <modules/axe/application/pstn/pstn.h>
#undef AXE_MMLPARSER

void mml_init_data(void) {
	memset(&mml, 0, sizeof(Smml_data));

	NetCS_reset_all_usersets();
	sigtran_USER_INDEX_RESETALL();
	sigtran_m3ua_USER_INDEX_RESETALL();
	M3UA_USER_LOOP_RESETALL();
	sccp_fifo_RESETALL();

	BSS_APPLICATION_RESETALL();
	PSTN_APPLICATION_RESETALL();
}

/*
 *
 *
 * 		***********************************  MML INTERFACE *****************************************
 *
 *
 */

void mml_mmlcnf(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int mmlcnf_index = mml.active_cmd_par_index;
	int netcs_index = 0xFFFF;
	int trncon_index = 0xFFFF; // dummy
	int trnloc_index = 0xFFFF; // dummy
	int tmp_port, error_code;
	struct sockaddr_in hostAddr;

	ErrorTraceHandle(2, "mml_mmlcnf(mml-%d)\n", mmlcnf_index);

	if (!strcmp(mml.Cmmlcnf.par[mmlcnf_index].Ptrn, "tcp"))	{

		NETCS_INDEX_KEY(netcs_index, tcp, server, netcs_mml, mmlcnf, mmlcnf_index, dummy,	0);
		if (netcs_index == 0xFFFF)
			NETCS_INDEX_NEW(netcs_index, tcp, server, netcs_mml, mmlcnf, mmlcnf_index, dummy,	0);

		ErrorTraceHandle(2, "Listen TCP on localhost port %s \n", mml.Cmmlcnf.par[mmlcnf_index].Plpn);

		NetCS_SocketConfigHost(mml.Cmmlcnf.par[mmlcnf_index].Plip, atoi(mml.Cmmlcnf.par[mmlcnf_index].Plpn), &hostAddr);
		if ((tmp_port = NetCS_ServerCreateOnPort(&hostAddr, 1, 1, netcs_index)) == 0)
			ErrorTraceHandle(0, "mml_mmlcnf() NetCS_ServerCreateOnPort() error, unable to create TCP server on port %d.\n", mml.Cmmlcnf.par[mmlcnf_index].Plpn);

		PTHREAD_SEND_OVER_MML(netcs_index);
	} else if (!strcmp(mml.Cmmlcnf.par[mmlcnf_index].Ptrn, "sctp")) {

		NETCS_INDEX_KEY(netcs_index, sctp, server, netcs_mml, trnloc, trnloc_index, trncon, trncon_index);
		if (netcs_index == 0xFFFF)
			NETCS_INDEX_NEW(netcs_index, sctp, server, netcs_mml, trnloc, trnloc_index, trncon, trncon_index);

		ErrorTraceHandle(2, "Listen STCP on localhost port %s \n", mml.Cmmlcnf.par[mmlcnf_index].Plpn);

		NetCS_SocketConfigHost(mml.Cmmlcnf.par[mmlcnf_index].Plip, atoi(mml.Cmmlcnf.par[mmlcnf_index].Plpn), &hostAddr);
		if ((tmp_port = NetCS_ServerCreateOnPort(&hostAddr, 2, 1, netcs_index)) == 0)
			ErrorTraceHandle(0, "mml_mmlcnf() NetCS_ServerCreateOnPort() error, unable to create SCTP server on port %d.\n", mml.Cmmlcnf.par[mmlcnf_index].Plpn);

		PTHREAD_SEND_OVER_MML(netcs_index);
	} else
		ErrorTraceHandle(2, "Unsupported transport %s.\n", mml.Cmmlcnf.par[mmlcnf_index].Ptrn);

	if ((error_code = pthread_create(&mml.Cmmlcnf.thread_id, NULL,
									 mml_thread_parse,
									 (void *) &send_over_mml)) != 0) {
		ErrorTraceHandle(0, "mml_mmlcnf() mml_thread_parse() thread failed to create.\n\n");
	}

	// set threat priority low
	struct sched_param param;
	int policy = SCHED_OTHER, ret, priority_min, priority_max;

	priority_min = sched_get_priority_min(policy);
	priority_max = sched_get_priority_max(policy);
	param.sched_priority = priority_min;

	if ((error_code = pthread_setschedparam(mml.Cmmlcnf.thread_id, policy, &param)) != 0) {
		ErrorTraceHandle(0, "mml_mmlcnf() pthread_setschedparam() unable to configure thread parameters.\n\n");
	}

	ErrorTraceHandle(1, "mml_mmlcnf() Thread created and configured.\n\n");
}

void *mml_thread_parse(void *arg) { // mml.active_cmd_par_index should be valid key index before entering this function
	struct pthread_data_mml *preceive_in_thread =(struct pthread_data_mml *) arg;
	int netcs_index, cnt;
	bool sent_hello_message = false;
	int error_code;
	unsigned int TRN_REC_local;

	netcs_index = preceive_in_thread->netcs_index;

	ErrorTraceHandle(2, "mml_thread_parse(netcs_index-%d)\n", netcs_index);

	while(1) {

		mml_send_string("> ");

		TRN_REC_local = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[netcs_index].transport_receive.read);
		while ((!NETCS_RECEIVED_DINGDONG(netcs_index)) ||
			    (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, netcs_index, TRN_REC_local)))
		{
			if (!sent_hello_message) {
				sent_hello_message = mml_send_line("\nWelcome to Balkan !\n");
				mml_send_string("> ");
			}

			NETCS_USER_SLEEP(netcs_index);
		} // wait until buffer filled and released

		if (1) { // echo option
			sent_hello_message = mml_send_string(NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data);
		}

		ErrorTraceHandle(2, "MML SET-%d/buff-%d RECEIVED (%dbytes) >> ",
				        netcs_index, TRN_REC_local, NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].size);
		for(cnt=0;cnt<NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].size; cnt++)
			ErrorTraceHandle(2, "0x%X ", NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[cnt] & 0xff);
		ErrorTraceHandle(2, "\n");

		yyinput_analyse_buffer_set = netcs_index;
		yyinput_analyse_stop0_file1_buffer2 = 2;
		yymmlparse();

		NETCS_RECEIVED_CLR_STATUS(sigtran, netcs_index, TRN_REC_local, NETCS_USERDATAVALID);
		NETCS_RECEIVED_READED(sigtran, netcs_index);
	}

	pthread_exit(NULL);
}

bool mml_send_line(char *format, ...) {
	char reply[MINIBUF];
	int count, netcs_index = 0xFFFF;

	va_list list;
	va_start(list, format);
	vsprintf(reply, format, list);
	strcat(reply, "\r\n");

	mml_send_string(reply);
}

bool mml_send_string(char *format, ...) {
	char reply[MINIBUF];
	int count, netcs_index = 0xFFFF;

	va_list list;
	va_start(list, format);
	vsprintf(reply, format, list);

	for (count=0; count<PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) {
		  if (MML_PARAM_MASKS_CHECK(mmlcnf, count)) {
			NETCS_INDEX_KEY(netcs_index, tcp, server, netcs_mml, mmlcnf, count, dummy, 0);
			if ((netcs_index != 0xFFFF) && NetCS_DataSet[netcs_index].trn_status.client.socket_id != 0) {
				ErrorTraceHandle(2, "mml_send_string(mmlcnf-%d, netcs_index-%d) Reply to %s:%s using %s.\n",
								 count, netcs_index, mml.Cmmlcnf.par[count].Plip, mml.Cmmlcnf.par[count].Plpn, mml.Cmmlcnf.par[count].Ptrn);
				if (!strcmp(mml.Cmmlcnf.par[count].Ptrn, "tcp")) {
					if (send(NetCS_DataSet[netcs_index].trn_status.client.socket_id, reply, strlen(reply), 0) == -1) {
						NetCS_SocketClose(NetCS_DataSet[netcs_index].trn_status.client.socket_id);
						ErrorTraceHandle(1, "mml_send_string(mmlcnf-%d, netcs_index-%d) error %d (%s).\n", count, netcs_index, errno, strerror(errno));
						return false;
					} else
						return true;
				}
			}

		 }
	}

	return false;
}


bool mml_help() {

	mml_send_line("Examples:\n");

	mml_send_line("*** Define local SS7 Signaling Point Code ***");
	mml_send_line("ss7loc:ownsp=2-101, sptype=sep;");

	mml_send_line("*** Define connection to remote ss7 Signaling Point Code ***");
	mml_send_line("ss7con:sp=2-300, ownsp=2-101, net=ip;\n");

	mml_send_line("*** Define local Transport endpoint (SCTP) ***");
	mml_send_line("trnloc:epid=ep_rnc5555, lip=\"172.17.87.222\", lpn=5555, user=m3ua, start;");
	mml_send_line("trnloc:epid=ep_rnc5555, lip=\"172.17.87.222\", lpn=5555, user=m3ua;");
	mml_send_line("trnloc:epid=ep_rnc5555, start;");
	mml_send_line("trnloc:epid=ep_rnc5555, stop;");
	mml_send_line("trnloc:epid=ep_rnc5555, delete;");
	mml_send_line("trnloc:print;");

	mml_send_line("*** Define connection to remote Transport endpoint (SCTP) ***");
	mml_send_line("trncon:said=as_rnc5555_santa4444, epid=ep_rnc5555, rip=\"172.17.85.67\", rpn=4444, mode=client, start;");
	mml_send_line("trncon:said=as_rnc5555_santa4444, epid=ep_rnc5555, rip=\"172.17.85.67\", rpn=4444, mode=client;");
	mml_send_line("trncon:said=as_rnc5555_santa4444, start;");
	mml_send_line("trncon:said=as_rnc5555_santa4444, stop;");
	mml_send_line("trncon:said=as_rnc5555_santa4444, delete;");
	mml_send_line("trncon:print;");

	mml_send_line("*** Configure connection for local M3UA client toward remote M3UA server ***");
	mml_send_line("m3acon:dest=2-300; said=as_rnc5555_santa4444, bmode=client, start;");
	mml_send_line("m3acon:dest=2-300; said=as_rnc5555_santa4444, bmode=client;");
	mml_send_line("m3acon:dest=2-300; start;");
	mml_send_line("m3acon:dest=2-300; stop;");
	mml_send_line("m3acon:dest=2-300; delete;");
	mml_send_line("m3acon:print;");

	mml_send_line("*** Assign remote DPC to SCCP ***");
	mml_send_line("sccpcf:sp=2-300,ssn=254;\n");

	return true;
}


bool mml_stats(FILE *fptrace) {
	int mml_stats_index = mml.active_cmd_par_index;
	unsigned int netcs_index, mml_call_index, cnt;
	char msg_out[MAXIBUF];

#define SEND_STATS if (fptrace == NULL) mml_send_line(msg_out); else {strcat(msg_out, "\n"); fputs(msg_out, fptrace);}

	if (fptrace != NULL) strcpy(mml.Cstats.par[mml_stats_index].Pinfo, "all");
	{ sprintf(msg_out, "Statistics(%s):", mml.Cstats.par[mml_stats_index].Pinfo); SEND_STATS;}

	for (netcs_index=0; netcs_index<NETCS_MESSAGE_BUFFER_MAXSETS_MASK; netcs_index++) {
		if (fptrace != NULL) sprintf(msg_out, "all");
		else sprintf(msg_out, "set%d", netcs_index);

		if((NetCS_DataSet[netcs_index].trn_status.reserved) &&
		   (strstr(mml.Cstats.par[mml_stats_index].Pinfo, msg_out))) {

			sprintf(msg_out, "SET[%d]<%s>", netcs_index, mml.Cstats.par[mml_stats_index].Pinfo); SEND_STATS;

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "net"))) {
				sprintf(msg_out, "Transport Statistics:"); SEND_STATS;
					sprintf(msg_out, " TRN_REC: Stored  %.7d   Accepted %.7d   DimSize %.7d   DimWarning %.7d",
							STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].transport_receive.write),
							STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].transport_receive.read),
							MASK_sts_netcs_transport_receive,
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[NETCS_RECEIVED_GET_WRITE_COUNTER_all_occupied])); SEND_STATS;
					sprintf(msg_out, " USR_TR:  Counter %.7d   Busy     %.7d   DimSize %.7d   DimWarning %.7d",
							STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].user_transmit.write),
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied]),
							MASK_sts_netcs_user_transmit,
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[NETCS_USER_TRANSMIT_GET_WRITE_COUNTER_all_occupied])); SEND_STATS;
					sprintf(msg_out, " FIFO:    Counter %.7d   Sent     %.7d   DimSize %.7d   DimWarning %.7d",
							STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].fifo_user_transmit.write),
							STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].fifo_user_transmit.read),
							MASK_sts_netcs_fifo_user_transmit,
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO_all_occupied])); SEND_STATS;
			}

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "bss"))) {
					sprintf(msg_out, "BSS Statistics:"); SEND_STATS;
					sprintf(msg_out, " DLG_REF: Counter %.7d   Busy     %.7d   DimSize %.7d   DimWarning %.7d",
							 STS_COUNTER_VALUE(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref),
							SAFE_COUNTER_VALUE(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref_active),
							MASK_sts_sccp_dlg_ref,
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[POP_DIALOG_REF_all_occupied])); SEND_STATS;

					for (mml_call_index=0; mml_call_index<PARSER_MML_PARAM_MAX_NUMBER_MASK; mml_call_index++) {
						if (mml.Ccall.par[mml_call_index].status == start) {
							sprintf(msg_out, "User <%s> Statistics:", mml.Ccall.par[mml_call_index].Pcaller); SEND_STATS;
							sprintf(msg_out, " LU:      Sent    %.7d   Pass     %.7d   Fail       %.7d",
									SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[OSI_SCTP_M3UA_SCCP_BSSAP_MAP__CL3I]),
									SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[switch_up_bssDtap_parse_message__LOCATION_UPDATING_ACCEPT]),
									SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[switch_up_bssDtap_parse_message__LOCATION_UPDATING_REJECT])); SEND_STATS;
						}
					}
			}

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "pstn"))) {
					char m3ua_user_index = ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->sigtran_user_id;
					struct Sisup_cic_id *isup_cic_id = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;

					sprintf(msg_out, "PSTN Statistics:"); SEND_STATS;
					sprintf(msg_out, " CIC:     Counter %.7d   Busy     %.7d   DimSize %.7d   DimWarning %.7d",
							STS_COUNTER_VALUE(sts_isup_cic_id, isup_cic_id->current),
							STS_COUNTER_VALUE(sts_isup_cic_id, isup_cic_id->originating_active) +
							STS_COUNTER_VALUE(sts_isup_cic_id, isup_cic_id->terminating_active),
							MASK_sts_isup_cic_id,
							SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[POP_DIALOG_REF_all_occupied])); SEND_STATS;

					for (mml_call_index=0; mml_call_index<PARSER_MML_PARAM_MAX_NUMBER_MASK; mml_call_index++) {
						if (mml.Ccall.par[mml_call_index].status == start) {
							sprintf(msg_out, "User <%s> Statistics:", mml.Ccall.par[mml_call_index].Pcaller); SEND_STATS;
							sprintf(msg_out, " ISUP CALLS:   Originating    %.7d   Terminating    %.7d   Pass     %.7d   Fail       %.7d",
									STS_STATISTIC_VALUE(isup_cic_id->originating_active),
									STS_STATISTIC_VALUE(isup_cic_id->terminating_active),
									SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[ISUP_call_PASS]),
									SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[netcs_index].stats.c[ISUP_call_FAIL])); SEND_STATS;
						}
					}
			}

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "cic"))) {
					char m3ua_user_index = ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->sigtran_user_id;
					struct Sisup_cic_id *isup_cic_id = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;

					sprintf(msg_out, "CIC Statistics:"); SEND_STATS;
					for (cnt=0; cnt<MASK_sts_isup_cic_id; cnt+=32) {
						sprintf(msg_out, " CIC-%.4d  %d %d %d %d %d  %d %d %d %d %d   %d %d %d %d %d  %d %d %d %d %d   %d %d %d %d %d  %d %d %d %d %d  %d %d ",
								cnt,
								isup_cic_id->data[cnt+0].device_state,
								isup_cic_id->data[cnt+1].device_state,
								isup_cic_id->data[cnt+2].device_state,
								isup_cic_id->data[cnt+3].device_state,
								isup_cic_id->data[cnt+4].device_state,
								isup_cic_id->data[cnt+5].device_state,
								isup_cic_id->data[cnt+6].device_state,
								isup_cic_id->data[cnt+7].device_state,
								isup_cic_id->data[cnt+8].device_state,
								isup_cic_id->data[cnt+9].device_state,
								isup_cic_id->data[cnt+10].device_state,
								isup_cic_id->data[cnt+11].device_state,
								isup_cic_id->data[cnt+12].device_state,
								isup_cic_id->data[cnt+13].device_state,
								isup_cic_id->data[cnt+14].device_state,
								isup_cic_id->data[cnt+15].device_state,
								isup_cic_id->data[cnt+16].device_state,
								isup_cic_id->data[cnt+17].device_state,
								isup_cic_id->data[cnt+18].device_state,
								isup_cic_id->data[cnt+19].device_state,
								isup_cic_id->data[cnt+20].device_state,
								isup_cic_id->data[cnt+21].device_state,
								isup_cic_id->data[cnt+22].device_state,
								isup_cic_id->data[cnt+23].device_state,
								isup_cic_id->data[cnt+24].device_state,
								isup_cic_id->data[cnt+25].device_state,
								isup_cic_id->data[cnt+26].device_state,
								isup_cic_id->data[cnt+27].device_state,
								isup_cic_id->data[cnt+28].device_state,
								isup_cic_id->data[cnt+29].device_state,
								isup_cic_id->data[cnt+30].device_state,
								isup_cic_id->data[cnt+31].device_state); SEND_STATS;
					}
			}

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "events"))) {
				sprintf(msg_out, "Upstream faulty events [%d]:", netcs_index); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_discard_message                      	 %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_discard_message]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_1                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_1]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_2                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_2]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_3                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_3]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_4                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_4]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_5                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_5]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_6                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_6]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_7                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_7]); SEND_STATS;
				sprintf(msg_out, " SIGTRAN_upstream_parsing_event_8                  %.3d", NetCS_DataSet[netcs_index].stats.c[SIGTRAN_upstream_parsing_event_8]); SEND_STATS;
				sprintf(msg_out, " SCCP_upstream_parsing_event                       %.3d", NetCS_DataSet[netcs_index].stats.c[SCCP_upstream_parsing_event]); SEND_STATS;
				sprintf(msg_out, " SCCP_upstream_parsing_event_ref_outbounds         %.3d", NetCS_DataSet[netcs_index].stats.c[SCCP_upstream_parsing_event_ref_outbounds]); SEND_STATS;
				sprintf(msg_out, " SCCP_upstream_parsing_event_ref_notused           %.3d", NetCS_DataSet[netcs_index].stats.c[SCCP_upstream_parsing_event_ref_notused]); SEND_STATS;
				sprintf(msg_out, " BSSAP_upstream_parsing_event                      %.3d", NetCS_DataSet[netcs_index].stats.c[BSSAP_upstream_parsing_event]); SEND_STATS;
			}

			if ((strstr(mml.Cstats.par[mml_stats_index].Pinfo, "all")) ||
				(strstr(mml.Cstats.par[mml_stats_index].Pinfo, "preempt"))) {
				sprintf(msg_out, "Macro preemption conflict statistic [%d]:", netcs_index); SEND_STATS;
				sprintf(msg_out, " NETCS_RECEIVED_FILL                               %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_RECEIVED_FILL]); SEND_STATS;
				sprintf(msg_out, " NETCS_RECEIVED_SET_STATUS                         %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_RECEIVED_SET_STATUS]); SEND_STATS;
				sprintf(msg_out, " NETCS_RECEIVED_CLR_STATUS                         %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_RECEIVED_CLR_STATUS]); SEND_STATS;
				sprintf(msg_out, " NETCS_RECEIVED_READED                             %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_RECEIVED_READED]); SEND_STATS;
				sprintf(msg_out, " NETCS_USER_TRANSMIT_GET_WRITE_COUNTER             %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_USER_TRANSMIT_GET_WRITE_COUNTER]); SEND_STATS;
				sprintf(msg_out, " NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO    %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO]); SEND_STATS;
				sprintf(msg_out, " NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO   %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[eNETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO]); SEND_STATS;
				sprintf(msg_out, " POP_DIALOG_REF                                    %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[ePOP_DIALOG_REF]); SEND_STATS;
				sprintf(msg_out, " PUSH_DIALOG_REF                                   %.3d", NetCS_DataSet[netcs_index].stats.NETCS_MACROSAFE_0VALUE[ePUSH_DIALOG_REF]); SEND_STATS;
			}


		}
	}

	mml_send_line("");
}

bool mml_dump() {

	mml_send_line("Dumping to trace file ...");
	ErrorTraceHandle(0, "Dumping.\n");
}

void NETCS_netcs_mml_udp_USER_INDEX_INIT(int id, int dummy1, int dummy2) {}
void NETCS_netcs_mml_tcp_USER_INDEX_INIT(int id, int dummy1, int dummy2) {}
void NETCS_netcs_mml_sctp_USER_INDEX_INIT(int id, int dummy1, int dummy2) {}
bool NETCS_netcs_mml_udp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy) {return true;}
bool NETCS_netcs_mml_tcp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy) {return true;}
bool NETCS_netcs_mml_sctp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy) {return true;}
