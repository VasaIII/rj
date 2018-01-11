#define PSTN_C
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#include <modules/protocols/trunk/isup.h>
#include "modules/axe/application/pstn/pstn.h"
#undef PSTN_C

void* process_pstn_thread(void *arg) {
	int m3ua_user_index  				= ((struct Spthread_data_pstn_application *) arg)->m3ua_user_index;
	int mml_call_index   				= ((struct Spthread_data_pstn_application *) arg)->mml_call_index;
	int netcs_index 	 			 	= sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;

	unsigned int cic_id_local;

	struct timespec req, rem;

	// some prepared fast data
	char 		 Anum_length;
	char 		 Anum_string[20];
	char 		 Bnum_length;
	char 		 Bnum_string[20];
	unsigned int Bnum_range;

#define MAX_CPS 250
	unsigned int cps_configured, cps_list_cnt=0, cps_tot_cnt=0;
	struct sCpsList {
		int num;
		int num_keep;
	} cps_list[MAX_CPS]; // based on max hertz

	unsigned char iter_Bnum_offset[20], iter_Bnum_cnt, decimal_progress;

	FAST2ErrorTraceHandle(2, "process_pstn_thread() Started for mml_call_index=%d\n", mml_call_index);

	// set dumping on BSS application level
	//ErrorTraceHandle0dump = ErrorTraceHandle0dump_bss;

	Anum_length 	= strlen(pstn_application[mml_call_index].caller.mml_phonei.PAnum);
	Bnum_length 	= strlen(pstn_application[mml_call_index].caller.mml_phonei.PBnum);
	cps_configured	= atoi(pstn_application[mml_call_index].mml_call.Pcps);

	memset(cps_list, 0, sizeof(cps_list));
	if (cps_configured<=1) {
		req.tv_sec  = 1;
	    req.tv_nsec = 0;
		ErrorTraceHandle(1, "process_pstn_thread(mml_call_index=%d) cps uses sec = %d\n", mml_call_index, req.tv_sec);
	} else if (cps_configured<=250){
		req.tv_sec  = 0;
	    req.tv_nsec = 1000/*us*/*1000/*ms*/*1000/*s*/ /cps;
		ErrorTraceHandle(1, "process_pstn_thread(mml_call_index=%d) cps uses nano = %d\n", mml_call_index, req.tv_nsec);
	} else {
		req.tv_sec  = 0;
		req.tv_nsec = 1;
		while (cps_tot_cnt != cps_configured) {
			if (cps_list_cnt==MAX_CPS) cps_list_cnt=0;
			cps_list[cps_list_cnt].num_keep++;
			cps_list_cnt++;
			cps_tot_cnt++;
		}
		ErrorTraceHandle(1, "process_pstn_thread(mml_call_index=%d) cps uses lists.\n", mml_call_index);
	}

	cps_list_cnt = 0;


#define CALC_SLEEP \
	if (cps_configured<=250) { /*if within nanosleep() control*/ \
		nanosleep(&req, &rem); \
	} else { \
		cps_list[cps_list_cnt].num++; \
		if (cps_list[cps_list_cnt].num==cps_list[cps_list_cnt].num_keep) { \
			cps_list[cps_list_cnt].num = 0; \
			if ((++cps_list_cnt)==MAX_CPS) cps_list_cnt=0; \
			nanosleep(&req, &rem); \
		}; \
	}

#define CALC_DECIMAL_PREGRESS \
		decimal_progress = 1; \
		while (1) { \
			if ((((++Bnum_string[(Bnum_length-1)-(decimal_progress-1)])-'0')&10) == 10) { \
				Bnum_string[(Bnum_length-1)-(decimal_progress-1)] = '0'; \
				decimal_progress++; \
			} else break; \
		}

	while(1) {

		memset(iter_Bnum_offset, 0, sizeof(iter_Bnum_offset));
		decimal_progress = 1;

		memcpy(Anum_string, pstn_application[mml_call_index].caller.mml_phonei.PAnum, Anum_length);
		Anum_string[Anum_length] = '\0';

		Bnum_range   = atoi(pstn_application[mml_call_index].caller.mml_phonei.Prange);
		memcpy(Bnum_string, pstn_application[mml_call_index].caller.mml_phonei.PBnum, Bnum_length);
		Bnum_string[Bnum_length] = '\0';

		FAST6ErrorTraceHandle(2, "process_pstn_thread() PSTN CALL series started with Anum=%s(length=%d), Bnum=%s(length=%d), range=%d\n",
							  Anum_string, Anum_length, Bnum_string, Bnum_length, Bnum_range);

		while(Bnum_range--) {
			POP_ISUP_CIC_ID(pstn_app, netcs_index, cic_id_local);
			CONFIGURE_CIC_ID(Originating, cic_id_local, ISUP_call_state_originating_establishing, CALL_FAIL, Anum_length, Anum_string, Bnum_length, Bnum_string);
			PSTN_PROGRESS_CALL(cic_id_local, m3ua_user_index, isup, (enum Epstn_call_state) IAM_Initial_Address_Message);

			CALC_DECIMAL_PREGRESS;

			if (mml.Ccall.par[mml_call_index].status != started) break;

			CALC_SLEEP;
		}
		FAST1ErrorTraceHandle(2, "process_pstn_thread() PSTN CALL series completed.\n");

		if (mml.Ccall.par[mml_call_index].status != started) break;
	}

	ErrorTraceHandle(2, "process_pstn_thread() PSTN CALL program finished.\n");

	mml.Ccall.par[mml_call_index].status = stopped;

	pthread_exit(NULL);
}

#define IN_MOOD_FOR(user, msg)  ((userinfo == user) && (in_mood_for == msg))

bool PSTN_PROGRESS_CALL(
		unsigned int cic_id,
		int m3ua_user_index,
		unsigned char userinfo,
		unsigned int in_mood_for) {
	int netcs_index 		  = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation; \

	FAST7ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(ENTER) CIC-%d, m3ua_user_index=%d, userinfo=0x%x, call_state=0x%.4x(0x%x), in_mood_for=0x%.4x\n",
						  cic_id,
						  m3ua_user_index,
						  userinfo,
						  isup_cic_id->data[cic_id].call_state,
						  PSTN_MASK(ISUP_call_state_terminating_establishing),
						  in_mood_for);

	if PSTN_STATE_CHALLENGE(ISUP_call_state_NoCall) // undefined state for start of progress
	{
	    FAST2ErrorTraceHandle(2, "PSTN_PROGRESS_CALL() CIC-%d in No Call state ... not supported.\n", cic_id);
		PSTN_STATE_MOVE(ISUP_call_state_NoCall, ISUP_call_state_NoCall, CALL_FAIL);
	}

	if PSTN_STATE_CHALLENGE(ISUP_call_state_originating_establishing) // originating IAM
	{
		if IN_MOOD_FOR(isup, IAM_Initial_Address_Message) {
			PSTN_MSG_SEND(ISUP_CIC_send__IAM_Initial_Address_Message);
		} else { /*IAM sent*/
			if IN_MOOD_FOR(isup, ACM_Address_Complete) {
				PSTN_MSG_RECV(ISUP_CIC_recv__ACM_Address_Complete);
			}
			if IN_MOOD_FOR(isup, REL_Release) {	// after IAM or ACM ... REL can come, this leads to FAIL state
				PSTN_STATE_MOVE(ISUP_call_state_originating_establishing, ISUP_call_state_terminating_releasing, CALL_FAIL);
			}
			if IN_MOOD_FOR(isup, ANM_Answer) {
				PSTN_MSG_RECV(ISUP_CIC_recv__ANM_Answer);
				PSTN_CHKPOINT(ISUP_checkpoint_Originating_Call_established);
				// set timeout !!!
				PSTN_STATE_MOVE(ISUP_call_state_originating_establishing, ISUP_call_state_speech, CALL_PASS);
			}
		}
	}

	if PSTN_STATE_CHALLENGE(ISUP_call_state_terminating_establishing) // terminating IAM
	{
		if IN_MOOD_FOR(isup, IAM_Initial_Address_Message) {
			PSTN_MSG_RECV(ISUP_CIC_recv__IAM_Initial_Address_Message);
			// timer na IAM za ACM 200-500ms
			PSTN_MSG_SEND(ISUP_CIC_send__ACM_Address_Complete);
			// timer na ACM za ANM 500-1000ms
			PSTN_MSG_SEND(ISUP_CIC_send__ANM_Answer);
			PSTN_CHKPOINT(ISUP_checkpoint_Terminating_Call_established);
			// no timeout set for terminating call
			PSTN_STATE_MOVE(ISUP_call_state_terminating_establishing, ISUP_call_state_speech, CALL_PASS);
		}
	}

	if (PSTN_STATE_CHALLENGE(ISUP_call_state_speech) &&
		PSTN_STATE_CHALLENGE(ISUP_checkpoint_Originating_Call_established))
	{
		// ISUP_call_state_speech
		// temporary without timeout
		PSTN_STATE_MOVE(ISUP_call_state_speech, ISUP_call_state_originating_timeout_releasing, CALL_PASS);
	}

	if PSTN_STATE_CHALLENGE(ISUP_call_state_originating_timeout_releasing)
	{
		if (!PSTN_MSG(ISUP_CIC_send__REL_Release)) { 		// originating side sends REL
			PSTN_MSG_SEND(ISUP_CIC_send__REL_Release);
		}

		if IN_MOOD_FOR(isup, RLC_Release_Complete_isup) { 	// terminating side RLC on sent REL
			PSTN_MSG_RECV(ISUP_CIC_recv__RLC_Release_Complete);

			PUSH_ISUP_CIC_ID(isup, netcs_index, 1, cic_id);
			PSTN_STATE_MOVE(ISUP_call_state_originating_timeout_releasing, ISUP_call_state_NoCall, CALL_PASS);
		    FAST2ErrorTraceHandle(2, "PSTN_PROGRESS_CALL() CIC-%d originating released by timeout.\n", cic_id);
		}
	}

	if (PSTN_STATE_CHALLENGE(ISUP_call_state_speech) ||
		PSTN_STATE_CHALLENGE(ISUP_call_state_terminating_releasing) ||
		PSTN_STATE_CHALLENGE(ISUP_call_state_originating_timeout_releasing)) {
		if IN_MOOD_FOR(isup, REL_Release) {					// terminating/originating side in speech receives REL
			PSTN_MSG_RECV(ISUP_CIC_recv__REL_Release);
			PSTN_MSG_SEND(ISUP_CIC_send__RLC_Release_Complete);
			if (PSTN_STATE_CHALLENGE(ISUP_call_state_terminating_releasing))
				PUSH_ISUP_CIC_ID(isup, netcs_index, 1, cic_id)
			else
				PUSH_ISUP_CIC_ID(isup, netcs_index, 0, cic_id)

			// check cause code of REL, even in speech it may be some fault, then call is FAIL
			PSTN_STATE_MOVE(ISUP_call_state_speech, 						ISUP_call_state_NoCall, CALL_PASS);
			PSTN_STATE_MOVE(ISUP_call_state_terminating_releasing, 			ISUP_call_state_NoCall, CALL_PASS);
			PSTN_STATE_MOVE(ISUP_call_state_originating_timeout_releasing, 	ISUP_call_state_NoCall, CALL_PASS);
		    FAST2ErrorTraceHandle(2, "PSTN_PROGRESS_CALL() CIC-%d terminating released by peer.\n", cic_id);
		}
	}

	FAST2ErrorTraceHandle(2, "PSTN_PROGRESS_CALL(EXIT) CIC-%d\n", cic_id);

	return true;
}

void PSTN_APPLICATION_RESETALL(void) {
	memset(&pstn_application, 0, sizeof(struct Spstn_application)*PSTN_CALL_INSTANCE_MAX);
	numberof_pstn_applications = 0;
}








