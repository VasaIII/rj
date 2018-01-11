#define BSS
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/mobile/sccp_bssap.h>
#include <modules/protocols/mobile/radio.h>
#include "modules/axe/application/bss/bss.h"
#undef BSS

int ErrorTraceHandle0dump_bss(FILE *fptrace);
void BSS_RESET(int m3ua_user_index);

void *process_bss_bsc_thread(void *arg) {
	int m3ua_user_index  = ((struct Spthread_data_bss_bsc_application *) arg)->m3ua_user_index;
	int mml_call_index   = ((struct Spthread_data_bss_bsc_application *) arg)->mml_call_index;

	int netcs_index 	 = sigtran_user[m3ua_user_index].netcs_index;

	unsigned int dlg_ref_local;

	struct timespec req, rem;

	// some prepared fast data
	char imsi_length; // imsi length for 1 mz/mt or range is same
	char user_type;   // enum Esccp_common_connection_ref_fifo_user_type
	char imsi_string[20];   // enum Esccp_common_connection_ref_fifo_user_type
	unsigned int imsi_range;

#define MAX_CPS 250

	unsigned int cps_configured, cps_list_cnt=0, cps_tot_cnt=0;
	struct sCpsList {
		int num;
		int num_keep;
	} cps_list[MAX_CPS]; // based on max hertz

	unsigned char iter_imsi_offset[20], iter_imsi_cnt, decimal_progress;

	FAST2ErrorTraceHandle(2, "process_bss_bsc_thread() Started for mml_call_index=%d\n", mml_call_index);

	if (!strcmp(bss_bsc_application[mml_call_index].mml_call.Ptype, "LU"))
		user_type = (enum Esccp_common_connection_ref_fifo_user_type) BSS_lu;
	else if (!strcmp(bss_bsc_application[mml_call_index].mml_call.Ptype, "MO_TO_MT"))
		user_type = (enum Esccp_common_connection_ref_fifo_user_type) BSS_call;

	BSS_RESET(m3ua_user_index);
	FAST1ErrorTraceHandle(2, "process_bss_bsc_thread() Reset completed.\n");

	// set dumping on BSS application level
	ErrorTraceHandle0dump = ErrorTraceHandle0dump_bss;

	imsi_length 	= strlen(bss_bsc_application[mml_call_index].caller.mml_mei.Pimsi);
	cps_configured	= atoi(bss_bsc_application[mml_call_index].mml_call.Pcps);

	memset(cps_list, 0, sizeof(cps_list));
	if (cps_configured<=1) {
		req.tv_sec  = 1;
	    req.tv_nsec = 0;
		ErrorTraceHandle(1, "process_bss_bsc_thread(mml_call_index=%d) cps uses sec = %d\n", mml_call_index, req.tv_sec);
	} else if (cps_configured<=250){
		req.tv_sec  = 0;
	    req.tv_nsec = 1000/*us*/*1000/*ms*/*1000/*s*/ /cps;
		ErrorTraceHandle(1, "process_bss_bsc_thread(mml_call_index=%d) cps uses nano = %d\n", mml_call_index, req.tv_nsec);
	} else {
		req.tv_sec  = 0;
		req.tv_nsec = 1;
		while (cps_tot_cnt != cps_configured) {
			if (cps_list_cnt==MAX_CPS) cps_list_cnt=0;
			cps_list[cps_list_cnt].num_keep++;
			cps_list_cnt++;
			cps_tot_cnt++;
		}
		ErrorTraceHandle(1, "process_bss_bsc_thread(mml_call_index=%d) cps uses lists.\n", mml_call_index);
	}


	cps_list_cnt = 0;

#define CALC_SLEEP \
	cps_list[cps_list_cnt].num++; \
	if (cps_list[cps_list_cnt].num==cps_list[cps_list_cnt].num_keep) { \
		cps_list[cps_list_cnt].num = 0; \
		if ((++cps_list_cnt)==MAX_CPS) cps_list_cnt=0; \
		nanosleep(&req, &rem); \
	};

	while(1) {

		memset(iter_imsi_offset, 0, sizeof(iter_imsi_offset));
		decimal_progress = 1;

		imsi_range   = atoi(bss_bsc_application[mml_call_index].caller.mml_mei.Prange);
		memcpy(imsi_string, bss_bsc_application[mml_call_index].caller.mml_mei.Pimsi, imsi_length);
		imsi_string[imsi_length] = '\0';

		FAST4ErrorTraceHandle(2, "process_bss_bsc_thread() LU series started with imsi=%s (length=%d), range=%d\n", imsi_string, imsi_length, imsi_range);

		while(imsi_range--) {
			POP_DIALOG_REF(bsc_app, netcs_index, dlg_ref_local);
			// fill dialog data
			sccp_common_connection_ref.fifo[dlg_ref_local].mml_call_index 						= mml_call_index;
			sccp_common_connection_ref.fifo[dlg_ref_local].user_type 							= user_type;
			sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.state 			= LU_state_initial;
			sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.verdict 		= FAIL;
			sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.IMSI_length 	= imsi_length;
			memcpy(sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.IMSI, imsi_string, imsi_length);
			// increment stats for number of active dialog references

			FAST7ErrorTraceHandle(2, "process_bss_bsc_thread(dialog-%d) LU started with state 0x%x and IMSI %s, decimal_progress=%d, first %d-%c\n",
					dlg_ref_local,
					LU_MASK(LU_state_initial),
					sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.IMSI,
					/*iter_imsi_offset[0], iter_imsi_offset[1], iter_imsi_offset[2], iter_imsi_offset[3], */
					decimal_progress,
					sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.IMSI[(imsi_length-1)],
					sccp_common_connection_ref.fifo[dlg_ref_local].vBSS_caller_locupdate.IMSI[(imsi_length-1)]);
			BSS_DIALOG_PROGRESS_LU(dlg_ref_local, m3ua_user_index, fooprotocol, (enum Ebss_bsc_dialog_LU_state) LU_state_initial);

			decimal_progress = 1;
			while (1) {
				if ((((++imsi_string[(imsi_length-1)-(decimal_progress-1)])-'0')&10) == 10) {
					imsi_string[(imsi_length-1)-(decimal_progress-1)] = '0';
					decimal_progress++;
				} else break;
			}

			if (mml.Ccall.par[mml_call_index].status != started)
				break;

			CALC_SLEEP;
		}
		FAST1ErrorTraceHandle(2, "process_bss_bsc_thread() LU series completed.\n");

		//while(sccp_common_connection_ref.stats_fifo_source_ref_counters_active) {}
		//break;

		if (mml.Ccall.par[mml_call_index].status != started)
			break;
	}

	ErrorTraceHandle(2, "process_bss_bsc_thread() LU call program finished.\n");

	mml.Ccall.par[mml_call_index].status = stopped;

	pthread_exit(NULL);
}

#define DIALOG_TRACE(en) case en: FAST3ErrorTraceHandle(2, "BSS_RESET %s(0x%x)\n", #en, en);

void BSS_RESET(int m3ua_user_index) {
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;

	SAFE_COUNTER_DECLARATION(sts_sccp_dlg_ref, dlg_ref);
	SAFE_COUNTER_SET(sts_sccp_dlg_ref, dlg_ref, 0xFFFF);

	while (pm3ua_user->sccp_user_ssn254_bssap.state != BSC_reset_phase_completed) {
		switch((enum Esccp_user_ssn254_bssap_state) pm3ua_user->sccp_user_ssn254_bssap.state) {
		DIALOG_TRACE(BSC_Reset_allowed)
		{
			/* send BSSAP Reset */
			OSI_SCTP_M3UA_SCCP_BSSAP_MAP(0/*no dialog*/, m3ua_user_index, Reset);
			pm3ua_user->sccp_user_ssn254_bssap.state   = BSC_Reset_message_sent;
		}
		break;
		DIALOG_TRACE(BSC_Reset_message_received)
		{
			pm3ua_user->sccp_user_ssn254_bssap.state = BSC_reset_phase_completed;
		}
		break;
		}
	}
}

void BSS_APPLICATION_RESET(unsigned int id) {
	FAST2ErrorTraceHandle(2, "BSS_APPLICATION_RESET(id=%d)\n", id);

	if (id<BSS_BSC_CALL_INSTANCE_MAX) {
		FAST2ErrorTraceHandle(2, "BSS_APPLICATION_RESET(id=%d) Reseted.\n", id);
		memset(&bss_bsc_application[id], 0, sizeof(struct Sbss_bsc_application));
		numberof_bss_bsc_applications = 0;
	}
}

void BSS_APPLICATION_RESETALL(void) {
	memset(&bss_bsc_application, 0, sizeof(struct Sbss_bsc_application)*BSS_BSC_CALL_INSTANCE_MAX);
	numberof_bss_bsc_applications = 0;
}

#define IN_MOOD_FOR(user, msg)  ((userinfo == user) && (in_mood_for == msg))

bool BSS_DIALOG_PROGRESS_LU(
		unsigned int dlg_ref,
		int m3ua_user_index,
		unsigned char userinfo,
		unsigned int in_mood_for) {
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;

	FAST5ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(ENTER) dlg_ref=%d, m3ua_user_index=%d, userinfo=%d, in_mood_for=(0x%.4x)\n",
							dlg_ref, m3ua_user_index, userinfo, in_mood_for);

	if LU_STATE_CHALLENGE(LU_state_initial)
	{
		if (!LU_MSG(LU_send_SCCP__CR_BssMap__Complete_Layer3_Information_DtapMM__LOCATION_UPDATING_REQUEST)) {
			LU_MSG_SEND(LU_send_SCCP__CR_BssMap__Complete_Layer3_Information_DtapMM__LOCATION_UPDATING_REQUEST);
		} else if IN_MOOD_FOR(sccp, CC_Connection_Confirm) {
			LU_MSG_RECV(LU_recv_SCCP__CC);
			LU_MSG_SEND(LU_send_SCCP__DF1_BssMap__Classmark_Update);
			LU_CHKPOINT(LU_Classmark_pass);
			LU_STATE_MOVE(LU_state_initial, LU_state_started, PASS);
		} else if IN_MOOD_FOR(sccp, CREF_Connection_Refused) {
			LU_MSG_RECV(LU_recv_SCCP__CREF);
			LU_STATE_MOVE(LU_state_initial, LU_state_finishing, FAIL);
		}
	}
	else if LU_STATE_CHALLENGE(LU_state_started)
	{
		if IN_MOOD_FOR(bssDtapMM, LOCATION_UPDATING_ACCEPT) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_DtapMM__LOCATION_UPDATING_ACCEPT);
			LU_STATE_MOVE(LU_state_started, LU_state_finishing, PASS);
		} else if IN_MOOD_FOR(bssDtapMM, LOCATION_UPDATING_REJECT) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_DtapMM__LOCATION_UPDATING_REJECT);
			LU_STATE_MOVE(LU_state_started, LU_state_finishing, FAIL);
		}

		if (IN_MOOD_FOR(bssDtapMM, AUTHENTICATION_REQUEST) && !LU_MSG(LU_Auth_pass)) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_DtapMM__AUTHENTICATION_REQUEST);
			LU_MSG_SEND(LU_send_SCCP__DF1_DtapMM__AUTHENTICATION_RESPONSE);
			LU_CHKPOINT(LU_Auth_pass);
		} else if (IN_MOOD_FOR(bssDtapMM, AUTHENTICATION_REJECT) && LU_STATE_CHALLENGE(LU_Auth_pass)) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_DtapMM__AUTHENTICATION_REJECT);
			LU_STATE_MOVE(LU_state_started, LU_state_finishing, FAIL);
		}

		if (IN_MOOD_FOR(bssMap, Cipher_Mode_Command) && !LU_STATE_CHALLENGE(LU_Cipher_pass)) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_BssMap__Cipher_Mode_Command);
			LU_MSG_SEND(LU_send_SCCP__DF1_BssMap__Cipher_Mode_Complete);
			LU_CHKPOINT(LU_Cipher_pass);
		} else if (IN_MOOD_FOR(bssMap, Cipher_Mode_Reject) && LU_STATE_CHALLENGE(LU_Cipher_pass)) {
			LU_MSG_RECV(LU_send_SCCP__DF1_BssMap__Cipher_Mode_Reject);
			LU_STATE_MOVE(LU_state_started, LU_state_finishing, FAIL);
		}

		if (IN_MOOD_FOR(bssDtapMM, TMSI_REALLOCATION_COMMAND) && !LU_STATE_CHALLENGE(LU_TMSI_pass)) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_DtapMM__TMSI_REALLOCATION_COMMAND);
			LU_MSG_SEND(LU_send_SCCP__DF1_DtapMM__TMSI_REALLOCATION_COMPLETE);
			LU_CHKPOINT(LU_TMSI_pass);
		}
	}
	else if LU_STATE_CHALLENGE(LU_state_finishing)
	{
		if IN_MOOD_FOR(bssMap, Clear_Command) {
			LU_MSG_RECV(LU_recv_SCCP__DF1_BssMap__Clear_Command);
			LU_MSG_SEND(LU_send_SCCP__DF1_BssMap__Clear_Complete);
			LU_CHKPOINT(LU_Clear_pass);
		}
	}

	if (LU_STATE_CHALLENGE(LU_state_started) || LU_STATE_CHALLENGE(LU_state_finishing))
	{
		if IN_MOOD_FOR(sccp, RLSD_Released) {
			LU_MSG_RECV(LU_recv_SCCP__RLSD);
			LU_MSG_SEND(LU_send_SCCP__RLC);
			LU_STATE_MOVE(LU_state_unknown, LU_state_completed, PASS);
		}
	}

	if LU_STATE_CHALLENGE(LU_state_completed) {
		PUSH_DIALOG_REF(bsc_app, netcs_index, dlg_ref);
	    FAST4ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(EXIT) close dlg_ref=%d, Remaining active dialogs=%d, next free=%d\n)\n",
	    		dlg_ref,
	    		SAFE_COUNTER_VALUE(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref_active),
	    		SCCP_FIFO_DLG_REF);
		return false;
	}

	FAST2ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(EXIT) dlg_ref=%d\n", dlg_ref);

	return true;
}

int ErrorTraceHandle0dump_bss(FILE *fptrace) {
	char msg_out[MAXIBUF];
	unsigned int sts[10], netcs_index, count_min, count_max, range=100;

	mml_stats(fptrace);

	for (netcs_index=0; netcs_index<NETCS_MESSAGE_BUFFER_MAXSETS_MASK; netcs_index++)  {
		if ((NetCS_DataSet[netcs_index].trn_status.reserved) && (NetCS_DataSet[netcs_index].user == netcs_sigtran)) {

			sts[3] = STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].fifo_user_transmit.write);
			sts[4] = STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].fifo_user_transmit.read);
			sprintf(msg_out, "NetCS_DataSet[%d].fifo_user_transmit> FIFO_R-%d, FIFO_W-%d\n", netcs_index, sts[4], sts[3]);
			fputs(msg_out, fptrace);
			count_min =  (sts[4]<range)? 0:(sts[4]-range); // start from read-range
			count_max = ((sts[3]+range)>MASK_sts_netcs_fifo_user_transmit)? MASK_sts_netcs_fifo_user_transmit:(sts[3]+range); // end on write+range
			while(count_min!=count_max) {
				if (count_min==MASK_sts_netcs_fifo_user_transmit) count_min=0; // flip arround on max
				sprintf(msg_out, "FIFO_USR_TR-%.7d> mask=%d, USR_TR=%.5d", count_min,
						NetCS_DataSet[netcs_index].fifo_user_transmit.buffer[count_min].mask,
						NetCS_DataSet[netcs_index].fifo_user_transmit.buffer[count_min].user_transmit_index);
				if      (count_min==sts[3]) strcat(msg_out, " <-- FIFO_W");
				else if (count_min==sts[4]) strcat(msg_out, " <-- FIFO_R");
				strcat(msg_out, "\n");
				fputs(msg_out, fptrace);
				count_min++;
			}

			sts[2] = STS_STATISTIC_READ(NetCS_DataSet[netcs_index].user_transmit.write);
			sprintf(msg_out, "NetCS_DataSet[%d].user_transmit> USR_TR-%d\n", netcs_index, sts[2]);
			fputs(msg_out, fptrace);
			count_min =  (sts[2]<range)? 0:(sts[2]-range); // start from write-range
			count_max = ((sts[2]+range)>MASK_sts_netcs_user_transmit)? 	MASK_sts_netcs_user_transmit:(sts[2]+range); // end on write+range
			while(count_min!=count_max) {
				if (count_min==MASK_sts_netcs_user_transmit) count_min=0; // flip arround on max
				sprintf(msg_out, "USR_TR-%.7d> mask=%d, size=%.5d", count_min,
						NetCS_DataSet[netcs_index].user_transmit.buffer[count_min].mask,
						NetCS_DataSet[netcs_index].user_transmit.buffer[count_min].size);
				if      (count_min==sts[2]) strcat(msg_out, " <-- USR_TR");
				strcat(msg_out, "\n");
				fputs(msg_out, fptrace);
				count_min++;
			}

			sts[0] = STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].transport_receive.write);
			sts[1] = STS_STATISTIC_VALUE(NetCS_DataSet[netcs_index].transport_receive.read);
			sprintf(msg_out, "NetCS_DataSet[%d].transport_receive> TRN_REC_R-%d, TRN_REC_W-%d\n", netcs_index, sts[1], sts[0]);
			fputs(msg_out, fptrace);
			count_min =  (sts[1]<range)? 0:(sts[1]-range); // start from read-range
			count_max = ((sts[0]+range)>MASK_sts_netcs_transport_receive)? 	MASK_sts_netcs_transport_receive:(sts[0]+range); // end on write+range
			while(count_min!=count_max) {
				if (count_min==MASK_sts_netcs_transport_receive) count_min=0; // flip arround on max
				sprintf(msg_out, "TRN_REC-%.7d> mask=%d, size=%.5d", count_min,
						NetCS_DataSet[netcs_index].transport_receive.buffer[count_min].mask,
						NetCS_DataSet[netcs_index].transport_receive.buffer[count_min].size);
				if      (count_min==sts[0]) strcat(msg_out, " <-- TRN_REC_W");
				else if (count_min==sts[1]) strcat(msg_out, " <-- TRN_REC_R");
				strcat(msg_out, "\n");
				fputs(msg_out, fptrace);
				count_min++;
			}


		}

	}

}










