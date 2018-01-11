
#undef PREDEF
#ifdef VOIP
#define PREDEF
#else
#define PREDEF extern
#endif


#define CHECKPOINT(en_chkpoint)	en_chkpoint
#define STATE(en_state) 		en_state
#define ACTION(en_state) 		en_state
#define EVENT(en_state) 		en_state



enum e_sip_uac {
	STATE(SIP_uac_idle),

			ACTION(SIP_uac_request_INVITE),
		CHECKPOINT(SIP_uac_request_INVITE_sent),
		CHECKPOINT(SIP_uac_request_nonINVITE_sent),


};

enum e_sip_uas {
	STATE(SIP_uas_idle),

		 EVENT(SIP_uas_request_received_INVITE),
		CHECKPOINT(SIP_uas_request_INVITE_received),
		CHECKPOINT(SIP_uas_request_nonINVITE_received),


};


struct e_sip_uac {
	unsigned int transaction;

	unsigned char request;
	unsigned char response;


};

struct e_sip_uas {
	unsigned int transaction;



};


enum e_sdp {
	STATE(SDP_offer_answer_idle),

	STATE(SDP_generate_offer),
			ACTION(SDP_offer_sent),
			EVENT( SDP_offer_received),
		CHECKPOINT(SDP_offer_GLARE),
			EVENT( SDP_answer_response_on_offer_received),
		CHECKPOINT(SDP_offer_PASS),
			EVENT( SDP_answer_reject_on_offer_received),
		CHECKPOINT(SDP_offer_REJECT),

	STATE(SDP_generate_answer_response),
			ACTION(SDP_answer_response_sent),
		CHECKPOINT(SDP_answer_PASS),

	STATE(SDP_generate_answer_reject),
			ACTION(SDP_answer_reject_sent),
		CHECKPOINT(SDP_answer_REJECT)
};
