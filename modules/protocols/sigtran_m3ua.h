
#undef PREDEF
#ifdef SIGTRAN_M3UA
#define PREDEF
#else
#define PREDEF extern
#endif

// *********************** PARAMETERS ***********************
#pragma pack(push, 1)
enum Em3ua_fix_parameter_hdr_tag {
	// Specific paramters
	Network_Appearance 			= 0x0200,
	User_Cause 					= 0x0204,
	Congestion_Indications 		= 0x0205,
	Concerned_Destination 		= 0x0206,
	Routing_Key 				= 0x0207,
	Registration_Result 		= 0x0208,
	Deregistration_Result 		= 0x0209,
	Local_RK_Identifier 		= 0x020A,
	Destination_Point_Code 		= 0x020B,
	Service_Indicators 			= 0x020C,
	Originating_Point_Code_List	= 0x020E,
	Circuit_Range 				= 0x020F,
	Protocol_Data 				= 0x0210,
	Registration_Status 		= 0x0212,
	Deregistration_Status 		= 0x0213
};

struct SProtocol_Data {
	unsigned int  vOPC; // my OPC
	unsigned int  vDPC; // remote DPC
	unsigned char vSI;
	unsigned char vNI;
	unsigned char vMP;
	unsigned char vSLS;
};

#pragma pack(pop)
// *********************** MESSAGES ***********************
#pragma pack(push, 1)
enum Em3ua_fix_message_hdr_class_type {
	// Transfer_Messages
	Payload_Data_DATA						= 0x1,
	// Routing_Key_Management_Messages_RKM
	Registration_Request_REGREQ 			= 0x1,
	Registration_Response_REGRSP 			= 0x2,
	Deregistration_Request_DEREGREQ 		= 0x3,
	Deregistration_Response_DEREGRSP 		= 0x4
};


struct SPayload_Data_DATA {
	struct Ssigtran_fix_message_hdr    vsigtran_fix_message_hdr;
	struct Ssigtran_fix_parameter_hdr  vsigtran_fix_parameter_hdr;
	struct SProtocol_Data              vProtocol_Data;
};

#pragma pack(pop)
// *********************** M3UA DATA ***********************

enum Esccp_user_ssn254_bssap_state {
	BSC_undef,
	BSC_Reset_waiting_for_allowed,
	BSC_Reset_allowed,
	BSC_Reset_message_sent,
	BSC_Reset_message_received,
	BSC_reset_phase_completed
};

enum eSigtran_m3ua_UserAdaptation_Supported {
	sigtran_m3ua_SI_SSNM_messages											= 0x0,
	sigtran_m3ua_SI_SSNM_regular_messages									= 0x1,
	sigtran_m3ua_SI_SSNM_special_messages									= 0x2,
	sigtran_m3ua_SI_SCCP													= 0x3,
	sigtran_m3ua_SI_TUP														= 0x4,
	sigtran_m3ua_SI_N_ISUP													= 0x5,
	sigtran_m3ua_SI_DUP_call_and_circuit_related_messages					= 0x6,
	sigtran_m3ua_SI_DUP_facility_registration_and_cancellation_messages		= 0x7,
	sigtran_m3ua_SI_MTP_testing_user_part									= 0x8,
	sigtran_m3ua_SI_B_ISUP													= 0x9,
	sigtran_m3ua_SI_Satellite_ISUP											= 0xa,
	sigtran_m3ua_SI_Reserved1												= 0xb,
	sigtran_m3ua_SI_AAL_type_2_Signalling									= 0xc,
	sigtran_m3ua_SI_BICC													= 0xd,
	sigtran_m3ua_SI_GCP														= 0xe,
	sigtran_m3ua_SI_Reserved2												= 0xf
};

struct Sm3ua_user {
	unsigned char 	mml_m3acon_index;

	unsigned char 	M3uaAdaptation_ServiceIndicator;	// (enum eSigtran_m3ua_UserAdaptation_Supported), it is same value as SI from Protocol_Data_Configured
	void 		  *pM3uaAdaptation;

	bool		 	LoopActive;				// indicates wheather m3ua loop is active for this m3ua
	unsigned char 	LoopId;					// index in m3ua_user_loop[] (Sm3ua_user_loop)

	// MTP PARAMETERS
	struct SAffected_Point_Code {
		int ARRAY[128];
		int COUNTER;
		int MAX_COUNTER_VALUE;
	}VAffected_Point_Code;
 	// Protocol_Data_Received contains data from received M3UA Protocol_Data message
	// and is used for M3UA loop when on parse up i fill protocol data and on parse
	// down i match these data with ones stored in mml m3loop

	struct SProtocol_Data Protocol_Data_Configured;

	// SCCP SSN users
	struct SSCCPuser {
		unsigned char allowed; // means that SSN is up via SST/SSA

		// user data from sccpcf
		unsigned int own_sp;
		unsigned int peer_sp;

		int state; // enum Esccp_user_ssn254_bssap_state
	}sccp_user_ssn254_bssap;
};


// *********************** M3UA SCCP USER DATA ***********************



// *********************** M3UA LOOP DATA ***********************

#define M3UA_USER_LOOP_MAX_SIZE 	10
struct Sm3ua_user_loop {
	unsigned char reserved;
	unsigned int ownsp1; 				// source
	unsigned int netcs_index1; 			// source
	unsigned int m3ua_user_index1; 		// source
	int cicinc1;		 				// ISUP specific, in case of loop, increase cic for value of cicinc1
	unsigned int ownsp2; 				// dest
	unsigned int netcs_index2; 			// dest
	unsigned int m3ua_user_index2; 		// dest
} m3ua_user_loop[M3UA_USER_LOOP_MAX_SIZE];

#define MSG_M3UA_STATE_set(id, val, request) \
	{ ((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->V##val.state = request; \
	  time(&(((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->V##val.time)); /* seconds */ \
	  FAST4ErrorTraceHandle(1, "[user-%d,time %d] %s", id, ((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->V##val.time, (request==((enum Emsg_state)received))? "\n":"");\
	  }
#define MSG_M3UA_STATE_get(id, val, request) (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->V##val.state == (enum Emsg_state)request)

#define M3UA_USER_LOOP(key_netcs_index1, key_m3ua_user_index1, key_ownsp1, \
					   key_netcs_index2, key_m3ua_user_index2, key_ownsp2, key_cicinc) { \
	int cnt, id = 0xFFFF, first_available=0;\
	for(cnt=0; cnt<M3UA_USER_LOOP_MAX_SIZE; cnt++) { \
		if (m3ua_user_loop[cnt].reserved == 1) { \
			if ((m3ua_user_loop[cnt].netcs_index1 		== key_netcs_index1		) && \
				(m3ua_user_loop[cnt].m3ua_user_index1 	== key_m3ua_user_index1	) && \
				(m3ua_user_loop[cnt].ownsp1 	 	 	== atoi(key_ownsp1)		)) \
				{ id = cnt; break; } \
		} else id=cnt; } \
	if (id==0xFFFF) ErrorTraceHandle(1, "M3UA_USER_LOOP_ID: Max number (%d) of m3ua_user's reached \n", M3UA_USER_LOOP_MAX_SIZE); \
	else { \
		FAST8ErrorTraceHandle(2, "M3UA_USER_LOOP_ID: New m3ua_loop_user-%d (sctp_user-%d, m3ua_user-%d, ownsp=%d ---> sctp_user-%d, m3ua_user-%d, ownsp=%d).\n", \
								id, \
								key_netcs_index1, key_m3ua_user_index1, atoi(key_ownsp1), \
								key_netcs_index2, key_m3ua_user_index2, atoi(key_ownsp2)); \
		m3ua_user_loop[id].reserved 		= 1; \
		m3ua_user_loop[id].netcs_index1 	= key_netcs_index1; \
		m3ua_user_loop[id].m3ua_user_index1 = key_m3ua_user_index1; \
		m3ua_user_loop[id].ownsp1 			= atoi(key_ownsp1); \
		m3ua_user_loop[id].cicinc1 			= key_cicinc; \
		m3ua_user_loop[id].netcs_index2 	= key_netcs_index2; \
		m3ua_user_loop[id].m3ua_user_index2 = key_m3ua_user_index2; \
		m3ua_user_loop[id].ownsp2 			= atoi(key_ownsp2); \
		((struct Sm3ua_user *) sigtran_user[key_m3ua_user_index1].pUserAdaptation)->LoopActive = true; \
		((struct Sm3ua_user *) sigtran_user[key_m3ua_user_index1].pUserAdaptation)->LoopId	   = id; \
	} \
}

PREDEF void M3UA_USER_LOOP_RESET(int m3ua_user_loop_id);
PREDEF void M3UA_USER_LOOP_RESETALL(void);

PREDEF void M3UA__Payload_Data_DATA(
		unsigned int m3ua_user_index,
		unsigned int buffer_write_counter,
		unsigned int *osi_offset);

PREDEF void OSI_SCTP_M3UA__Destination_Available_DAVA(int m3ua_user_index);

