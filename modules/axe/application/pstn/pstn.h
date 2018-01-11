
#undef PREDEF
#ifdef PSTN_C
#define PREDEF
#else
#define PREDEF extern
#endif

// *********************** PSTN APPLICATION ***********************

struct Spthread_data_pstn_application { // pthread can send only one arg in function
	int dummy;	/* first variable in this structure was zeroed so i putted dummy untill debuged root of cause */
	int m3ua_user_index;
	int mml_call_index;
};

#define PTHREAD_SEND_TO_PSTN_APPLICATION(m3ua_user_index, mml_call_index) \
	pstn_application[mml_call_index].thread_arg.m3ua_user_index = m3ua_user_index; \
	pstn_application[mml_call_index].thread_arg.mml_call_index  = mml_call_index; \
	FAST3ErrorTraceHandle(3, "PTHREAD_SEND_TO_PSTN_APPLICATION(pstn_application-%d) m3ua_user_index=%d\n", \
			mml_call_index, m3ua_user_index);

#define PSTN_CALL_INSTANCE_MAX		10
struct Spstn_application {

	struct Scall_parameters     mml_call;
	struct Spstn_application_caller {
		struct Sphonei_parameters mml_phonei;
	} caller, called;

	pthread_t thread_id;
	struct Spthread_data_pstn_application thread_arg;
} pstn_application[PSTN_CALL_INSTANCE_MAX]; /* mml_call_index should match this index even ones below are not created */
PREDEF unsigned int numberof_pstn_applications; /* number of active application */



// *********************** PSTN ***********************

#define PSTN_MASK(enum_id) (1<<(enum_id&0x1F)) // max enum_id value is 31

#define PSTN_STATE_CHALLENGE(en)  (PSTN_MASK(en)&(isup_cic_id->data[cic_id].call_state))
#define PSTN_STATE_MOVE(enold, ennew, enverdict) { \
			FAST8ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_STATE_MOVE) CIC-%d old status (0x%.2x); remove state %s(0x%x); set state %s(0x%x); verdict=%s\n", cic_id, \
								  isup_cic_id->data[cic_id].call_state, #enold, PSTN_MASK(enold), #ennew, PSTN_MASK(ennew), #enverdict); \
			isup_cic_id->data[cic_id].call_state   = (~PSTN_MASK(enold))&(isup_cic_id->data[cic_id].call_state); \
			isup_cic_id->data[cic_id].call_state  |= PSTN_MASK(ennew); \
			isup_cic_id->data[cic_id].call_verdict = enverdict; \
			FAST3ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_STATE_MOVE) CIC-%d new status (0x%.2x)\n", cic_id, \
								  isup_cic_id->data[cic_id].call_state); }
#define PSTN_CHKPOINT(en) { \
			isup_cic_id->data[cic_id].call_state |= PSTN_MASK(en); \
			FAST5ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_CHECKPOINT) CIC-%d newstatus (0x%.2x); checkpoint %s(internal-0x%x)\n", cic_id, \
								  isup_cic_id->data[cic_id].call_state, #en, en); }
#define PSTN_MSG(en)  (PSTN_MASK(en)&(isup_cic_id->data[cic_id].call_state))
#define PSTN_MSG_RECV(en) { \
			isup_cic_id->data[cic_id].call_state |= PSTN_MASK(en);  \
			FAST5ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_MSG_RECV) CIC-%d new status (0x%.2x); message received %s(internal-0x%x)\n", cic_id, \
								  isup_cic_id->data[cic_id].call_state, #en, PSTN_MASK(en)); }
#define PSTN_MSG_SEND(en) { \
			isup_cic_id->data[cic_id].call_state |= PSTN_MASK(en); \
			FAST5ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_MSG_SEND) CIC-%d new status (0x%.2x); message to send  %s(internal-0x%x)\n", cic_id, \
								  isup_cic_id->data[cic_id].call_state, #en, PSTN_MASK(en)); \
			switch (en) { \
			case ISUP_CIC_send__IAM_Initial_Address_Message: \
				OSI_SCTP_M3UA_ISUP(m3ua_user_index, cic_id, IAM_Initial_Address_Message); break; \
			case ISUP_CIC_send__ACM_Address_Complete: \
				OSI_SCTP_M3UA_ISUP(m3ua_user_index, cic_id, ACM_Address_Complete); break; \
			case ISUP_CIC_send__ANM_Answer: \
				OSI_SCTP_M3UA_ISUP(m3ua_user_index, cic_id, ANM_Answer); break; \
			case ISUP_CIC_send__REL_Release: \
				OSI_SCTP_M3UA_ISUP(m3ua_user_index, cic_id, REL_Release); break; \
			case ISUP_CIC_send__RLC_Release_Complete: \
				OSI_SCTP_M3UA_ISUP(m3ua_user_index, cic_id, RLC_Release_Complete_isup); break; \
			default: FAST3ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(PSTN_MSG_SEND) msessage %s(external-0x%x) not supported.\n", #en, en); break; \
			} }


enum Epstn_verdict {
	CALL_FAIL,
	CALL_PASS
};

enum Epstn_call_state {
	ISUP_call_state_NoCall,

	ISUP_call_state_originating_establishing,
			ISUP_CIC_send__IAM_Initial_Address_Message,
			ISUP_CIC_recv__ACM_Address_Complete,
			ISUP_CIC_recv__ANM_Answer,
		ISUP_checkpoint_Originating_Call_established,

	ISUP_call_state_terminating_establishing,
			ISUP_CIC_recv__IAM_Initial_Address_Message,
			ISUP_CIC_send__ACM_Address_Complete,
			ISUP_CIC_send__ANM_Answer,
		ISUP_checkpoint_Terminating_Call_established,

	ISUP_call_state_speech,

	ISUP_call_state_originating_timeout_releasing,
		ISUP_CIC_send__REL_Release,
		ISUP_CIC_recv__RLC_Release_Complete,

	ISUP_call_state_terminating_releasing,
		ISUP_CIC_recv__REL_Release,
		ISUP_CIC_send__RLC_Release_Complete,
};


PREDEF void*  process_pstn_thread(void *arg);
PREDEF void PSTN_APPLICATION_RESETALL(void);
PREDEF bool PSTN_PROGRESS_CALL(
				unsigned int cic_id,
				int m3ua_user_index,
				unsigned char userinfo,
				unsigned int in_mood_for);






