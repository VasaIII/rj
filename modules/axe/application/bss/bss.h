
#undef PREDEF
#ifdef BSS
#define PREDEF
#else
#define PREDEF extern
#endif

// *********************** BSS APPLICATION ***********************

PREDEF int i;

enum Ebss_bsc_applications {
	bsc_app		= 0x1
};

enum Everdict {
	FAIL,
	PASS
};

struct Spthread_data_bss_bsc_application { // pthread can send only one arg in function
	int dummy;	/* first variable in this structure was zeroed so i putted dummy untill debuged root of cause */
	int m3ua_user_index;
	int mml_call_index;
};

#define PTHREAD_SEND_TO_BSS_BSC_APPLICATION(m3ua_user_index, mml_call_index) \
	bss_bsc_application[mml_call_index].thread_arg.m3ua_user_index = m3ua_user_index; \
	bss_bsc_application[mml_call_index].thread_arg.mml_call_index  = mml_call_index; \
	FAST3ErrorTraceHandle(3, "PTHREAD_SEND_TO_BSS_BSC_APPLICATION(bss_bsc_application-%d) m3ua_user_index=%d\n", \
			mml_call_index, m3ua_user_index);

#define BSS_BSC_CALL_INSTANCE_MAX		10
struct Sbss_bsc_application {

	struct Scall_parameters     mml_call;
	struct Sbss_bsc_application_me {
		struct Sbsci_parameters mml_bsci;
		struct Sbtsi_parameters mml_btsi;
		struct Smei_parameters  mml_mei;
	} caller, called;

	pthread_t thread_id;
	struct Spthread_data_bss_bsc_application thread_arg;
} bss_bsc_application[BSS_BSC_CALL_INSTANCE_MAX]; /* mml_call_index should match this index even ones below are not created */

PREDEF unsigned int numberof_bss_bsc_applications; /* number of active application */


// *********************** DIALOGS ***********************

#define LU_MASK(mask) (1<<((mask-1)&0x1F)) // max mask value is 31

#define LU_STATE_CHALLENGE(en)  (LU_MASK(en)&sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state)
#define LU_STATE_MOVE(enold, ennew, enverdict) { \
						  FAST8ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(MOVE_STATE dialog-%d) old status (0x%.2x); remove state %s(0x%x); set state %s(0x%x); verdict=%s\n", dlg_ref, \
										        sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state, #enold, LU_MASK(enold), #ennew, LU_MASK(ennew), #enverdict); \
						  sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state = (~LU_MASK(enold))&sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state; \
						  sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state |= LU_MASK(ennew); \
						  sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.verdict = enverdict; \
						  FAST3ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(MOVE_STATE dialog-%d) new status (0x%.2x)\n", dlg_ref, \
										        sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state); }

#define LU_MSG(en)  (LU_MASK(en)&sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state)
#define LU_MSG_RECV(en) { sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state |= LU_MASK(en);  \
						  FAST5ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(LU_MSG_RECV dialog-%d) new status (0x%.2x); message received %s(internal-0x%x)\n", dlg_ref, \
										        sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state, #en, LU_MASK(en)); \
						}
#define LU_MSG_SEND(en) { sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state |= LU_MASK(en); \
						  FAST5ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(LU_MSG_SEND dialog-%d) new status (0x%.2x); message to send  %s(internal-0x%x)\n", dlg_ref, \
										        sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state, #en, LU_MASK(en)); \
						  switch (en) { \
						    case LU_send_SCCP__RLC: \
						        OSI_SCTP_M3UA_SCCP(m3ua_user_index, dlg_ref, RLC_Release_Complete_sccp); break; \
							case LU_send_SCCP__CR_BssMap__Complete_Layer3_Information_DtapMM__LOCATION_UPDATING_REQUEST: \
								OSI_SCTP_M3UA_SCCP_BSSAP_MAP(m3ua_user_index, dlg_ref, Complete_Layer3_Information); break; \
							case LU_send_SCCP__DF1_BssMap__Clear_Complete: \
								OSI_SCTP_M3UA_SCCP_BSSAP_MAP(m3ua_user_index, dlg_ref, Clear_Complete); break; \
							case LU_send_SCCP__DF1_BssMap__Cipher_Mode_Complete: \
								OSI_SCTP_M3UA_SCCP_BSSAP_MAP(m3ua_user_index, dlg_ref, Cipher_Mode_Complete); break; \
							\
							case LU_send_SCCP__DF1_DtapMM__AUTHENTICATION_RESPONSE: \
								OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(m3ua_user_index, dlg_ref, AUTHENTICATION_RESPONSE); break; \
							case LU_send_SCCP__DF1_DtapMM__TMSI_REALLOCATION_COMPLETE: \
								OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(m3ua_user_index, dlg_ref, TMSI_REALLOCATION_COMPLETE); break; \
							default: FAST3ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(LU_MSG_SEND) msessage %s(external-0x%x) not supported.\n", #en, en); break; \
						} }
#define LU_CHKPOINT(en) { sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state |= LU_MASK(en); \
						  FAST5ErrorTraceHandle(2, "BSS_DIALOG_PROGRESS_LU(LU_MSG_CHECKPOINT dialog-%d) newstatus (0x%.2x); checkpoint %s(internal-0x%x)\n", dlg_ref, \
										   sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.state, #en, en); }


enum Ebss_bsc_dialog_LU_state {
	LU_state_unknown = 0,
	LU_state_initial,
		LU_send_SCCP__CR_BssMap__Complete_Layer3_Information_DtapMM__LOCATION_UPDATING_REQUEST,
		LU_recv_SCCP__CC,	// LU_state_started
		LU_recv_SCCP__CREF, // LU_state_finishing	FAIL
	LU_state_started,

			LU_send_SCCP__DF1_BssMap__Classmark_Update,
		LU_Classmark_pass,

			LU_recv_SCCP__DF1_BssMap__Common_ID,
		LU_Common_ID_pass,

			LU_recv_SCCP__DF1_DtapMM__AUTHENTICATION_REQUEST,
			LU_send_SCCP__DF1_DtapMM__AUTHENTICATION_RESPONSE,	// LU_state_Auth_pass
			LU_recv_SCCP__DF1_DtapMM__AUTHENTICATION_REJECT,	// LU_state_finishing	FAIL
		LU_Auth_pass,

			LU_recv_SCCP__DF1_BssMap__Cipher_Mode_Command,
			LU_send_SCCP__DF1_BssMap__Cipher_Mode_Complete,		// LU_state_Cipher_pass
			LU_send_SCCP__DF1_BssMap__Cipher_Mode_Reject,		// LU_state_finishing	FAIL
		LU_Cipher_pass,

			LU_recv_SCCP__DF1_DtapMM__TMSI_REALLOCATION_COMMAND,
			LU_send_SCCP__DF1_DtapMM__TMSI_REALLOCATION_COMPLETE,
		LU_TMSI_pass,

			LU_recv_SCCP__DF1_DtapMM__LOCATION_UPDATING_ACCEPT,		// LU_state_finishing	PASS
			LU_recv_SCCP__DF1_DtapMM__LOCATION_UPDATING_REJECT, 	// LU_state_finishing 	FAIL

		LU_recv_SCCP__RLSD,
		LU_send_SCCP__RLC,	// LU_state_completed 	FAIL

		// LU_send_SCCP__DF1_BssMap__Clear_Request    ... timeout

	LU_state_finishing,
		LU_recv_SCCP__DF1_BssMap__Clear_Command,
		LU_send_SCCP__DF1_BssMap__Clear_Complete,
	LU_Clear_pass,

	LU_state_completed
};

PREDEF bool BSS_DIALOG_PROGRESS_LU(
		unsigned int dlg_ref,
		int m3ua_user_index,
		unsigned char userinfo,
		unsigned int in_mood_for);
PREDEF void BSS_APPLICATION_RESET(unsigned int id);
PREDEF void BSS_APPLICATION_RESETALL(void);
PREDEF void* process_bss_bsc_thread(void *arg);

