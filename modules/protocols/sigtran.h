
#undef PREDEF
#ifdef SIGTRAN
#define PREDEF
#else
#define PREDEF extern
#endif

struct Ssigtran_fix_message_hdr {
	unsigned char 	version;
	unsigned char 	dummy_1;
	unsigned char 	class;
	unsigned char 	type;
	unsigned int   	length;
};
struct Ssigtran_fix_parameter_hdr {
	short int parameter_tag;
	short int length;
};

enum Esigtran_ASP_States {
	ASP_Down		= 0x1,
	ASP_Inactive	= 0x2,
	ASP_Active		= 0x3
};


enum Esigtran_fix_message_hdr_class {
	// Sigtran framework
	ManagementMessages_MGMT					= 0, // M3UA		SUA
	Transfer_Messages						= 1, // M3UA
	SS7_Management_Messages_SSNM			= 2, // M3UA		SUA
	ASP_State_Maintenance_Messages_ASPSM	= 3, // M3UA		SUA
	ASP_Traffic_Maintenance_Messages_ASPTM	= 4, // M3UA		SUA
	Conectionless_Messages 					= 7, // 			SUA
	Connection_Oriented_Messages		 	= 8, // 			SUA
	Routing_Key_Management_Messages_RKM 	= 9	 // M3UA		SUA
	/*  10-127 reserved */
	/* 128-255 reserved for IETF Defined Message Class extensions */
};

enum Esigtran_fix_message_hdr_class_type__ManagementMessages_MGMT {
	Error_ERR								= 0x0,
	Notify_NTFY								= 0x1
};

enum Esigtran_fix_message_hdr_class_type__SS7_Management_Messages_SSNM {
	Destination_Unavailable_DUNA			= 0x1,
	Destination_Available_DAVA				= 0x2,
	Destination_State_Audit_DAUD			= 0x3,
	Signalling_Congestion_SCON				= 0x4,
	Destination_User_Part_Unavailable_DUPU	= 0x5,
	Destination_Restricted_DRST				= 0x6
};

enum Esigtran_fix_message_hdr_class_type__ASP_State_Maintenance_Messages_ASPSM {
	ASP_Up_ASPUP							= 0x1,
	ASP_Down_ASPDN							= 0x2,
	Heartbeat_BEAT							= 0x3,
	ASP_Up_Acknowledgement_ASPUPACK			= 0x4,
	ASP_Down_Acknowledgement_ASPDNACK		= 0x5,
	Heartbeat_Acknowledgement_BEATACK		= 0x6
};

enum Esigtran_fix_message_hdr_class_type__ASP_Traffic_Maintenance_Messages_ASPTM {
	ASP_Active_ASPAC						= 0x1,
	ASP_Inactive_ASPIA						= 0x2,
	ASP_Active_Acknowledgement_ASPACACK		= 0x3,
	ASP_Inactive_Acknowledgement_ASPIAACK	= 0x4
};

/* 4 bytes */
enum Esigtran_fix_parameter_hdr_tag {
	// Common parmeters
	Info_String 				= 0x0004,
	Routing_Context 			= 0x0006,
	Diagnostic_information 		= 0x0007,
	Heartbeat_Data 				= 0x0009,
	Traffic_Mode_Type 			= 0x000B,
	Error_Code 					= 0x000C,
	Status 						= 0x000D,
	ASP_Identifier 				= 0x0011,
	Affected_Point_Code 		= 0x0012,
	Correlation_ID 				= 0x0013,
};

enum Esigtran_parameter_Error_Code {
	Invalid_Version 				= 0x01,
	Unsupported_Message_Class 		= 0x03,
	Unsupported_Message_Type 		= 0x04,
	Unsupported_Traffic_Mode_Type 	= 0x05,
	Unexpected_Message 				= 0x06,
	Protocol_Error 					= 0x07,
	Invalid_Stream_Identifier 		= 0x09,
	Refused_Management_Blocking 	= 0x0d,
	ASP_Identifier_Required 		= 0x0e,
	Invalid_ASP_Identifier 			= 0x0f,
	Invalid_Parameter_Value 		= 0x11,
	Parameter_Field_Error 			= 0X12,
	Unexpected_Parameter 			= 0X13,
	Destination_Status_Unknown 		= 0X14,
	Invalid_Network_Appearance 		= 0X15,
	Missing_Parameter 				= 0X16,
	Invalid_Routing_Context 		= 0X19,
	No_Configured_AS_for_ASP 		= 0X1A
};


struct spthread_data_sigtran { // pthread can send only one arg in function
	int dummy;	/* first variable in this structure was zeroed so i putted dummy untill debuged root of cause */
	int user_index;
	int parse_select;
	int netcs_index;
};

#define PTHREAD_SEND_OVER_SIGTRAN(val_user_index, val_parse_select, val_netcs_index) \
	sigtran_user[val_user_index].netcs_user_thread_arg.user_index 	= val_user_index; \
	sigtran_user[val_user_index].netcs_user_thread_arg.parse_select = val_parse_select; \
	sigtran_user[val_user_index].netcs_user_thread_arg.netcs_index 	= val_netcs_index; \
	ErrorTraceHandle(3, "PTHREAD_SEND_OVER_SIGTRAN() user_index=%d, parse_select=%d, netcs_index=%d\n", \
			sigtran_user[val_user_index].netcs_user_thread_arg.user_index, \
			sigtran_user[val_user_index].netcs_user_thread_arg.parse_select, \
			sigtran_user[val_user_index].netcs_user_thread_arg.netcs_index);


enum eSigtran_UserAdaptation_Supported {
	sigtran_dummy,
	sigtran_m3ua,
	sigtran_sua,
	sigtran_iua
};

#define SIGTRAN_USER_MAX_SIZE_MASK		0xF
struct Ssigtran_user {
	unsigned char 	reserved;

	pthread_t    	netcs_user_thread_id;
	struct spthread_data_sigtran netcs_user_thread_arg;

	unsigned char 	UserAdaptation; 		// (enum eSigtran_UserAdaptation_Supported)
	void 		  *pUserAdaptation;

	unsigned char 	netcs_index;
	unsigned char 	mode_undef0_client1_server2;

	struct Ssigtran_client_user_mode {
		bool waiting_handshake;

		bool vASPUP_sent;
		time_t vASPUP_time_sent;
		bool vASPUPACK_received;

		bool vASPAC_sent;
		bool vASPACACK_received;

		bool vDestinations_audited;
	} client;

	struct Ssigtran_server_user_mode {
		bool waiting_handshake;

		bool vASPUP_received;
		bool vASPUPACK_sent;

		bool vASPAC_received;
		bool vASPACACK_sent;
	} server;

	struct Ssigtran_user_state {
		bool shuttingdown;

		bool vASPDN_sent;
		bool vASPDNACK_received;
	} both;

} sigtran_user[SIGTRAN_USER_MAX_SIZE_MASK+1];


#define SIGTRAN_USER_INDEX_KEY(id, key_netcs_index, ua, ua_key) { \
	int cnt; id = 0xFFFF;\
	for(cnt=0; cnt<SIGTRAN_USER_MAX_SIZE_MASK; cnt++) { \
		if ((sigtran_user[cnt].reserved == 1) && \
			(sigtran_user[cnt].UserAdaptation==(enum eSigtran_UserAdaptation_Supported) sigtran_m3ua) && \
			(sigtran_user[cnt].netcs_index == key_netcs_index) && \
			(NetCS_DataSet[key_netcs_index].trn_status.reserved == 1) && \
			UA_##ua##_USER_INDEX_CMP(cnt, ua_key)) /* condition per UA */\
			{ id = cnt; break; } \
		} \
	}
#define SIGTRAN_USER_INDEX_NEW(id, key_netcs_index, ua, ua_key) { \
		int iter=0; id=0; \
		while (1) { \
			if(!sigtran_user[id].reserved) break; \
			id=(id+1)&SIGTRAN_USER_MAX_SIZE_MASK; iter++; \
			if (iter==SIGTRAN_USER_MAX_SIZE_MASK) ErrorTraceHandle(0, "SIGTRAN_USER_INDEX_NEW: Max number (%d) of sigtran_user's reached \n", SIGTRAN_USER_MAX_SIZE_MASK); \
		} \
		ErrorTraceHandle(2, "SIGTRAN_USER_INDEX_NEW: New sigtran_user_index-%d (%s, %s-%d) on existing netcs_index-%d (trnloc-%d, trncon-%d), socket_id=%d.\n", \
				id, \
				#ua, #ua_key, ua_key,\
				key_netcs_index, \
				((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[key_netcs_index].puser_data)->mml_trnloc_index, \
				((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[key_netcs_index].puser_data)->mml_trncon_index, \
				NetCS_DataSet[key_netcs_index].trn_status.client.socket_id); \
		memset(&sigtran_user[id], 0, sizeof(sigtran_user[id])); \
		sigtran_user[id].reserved 			= 1; \
		sigtran_user[id].UserAdaptation 	= ua; \
		sigtran_user[id].pUserAdaptation 	= NULL; \
		sigtran_user[id].netcs_index 		= key_netcs_index; \
		((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[key_netcs_index].puser_data)->sigtran_user_id = id; /*needed for m3ua loop*/ \
		UA_##ua##_USER_INDEX_INIT(id, ua_key); \
}

//#define EPRINTcase2(str, en1, en2) case en1##__##en2: ErrorTraceHandle(2, "%s%s__%s\n", str, #en1, #en2);

// e.g. MSG_STATE(user_id, ASP_Up_ASPUP, sent)
#define MSG_STATE_set(id, val, request)  sigtran_user[id].V##val.state = request;
#define MSG_STATE_get(id, val, request) (sigtran_user[id].V##val.state == (enum Emsg_state)request)
enum Emsg_state {
	idle,
	sent,
	received,
};

struct SSigtran_userof_NetCS_DataSet {
	unsigned char sigtran_user_id;	// if i have netcs_set id and want to know which sigtran_user_id is connected to it

	// defined as int since undefined value is #FFFF
	unsigned int mml_trnloc_index;
	unsigned int mml_trncon_index;
	unsigned int mml_mmlcnf_index;
};

#include <modules/protocols/sigtran_m3ua.h>

PREDEF unsigned int sigtran_parse(unsigned int *user_id,
						   int 			parse_select,
						   unsigned int	TRN_REC_local,
						   void   		*msg_sigtran_pstartbuff,
						   void   		*msg_sigtran_pworkingbuff,
						   void   		*msg_sigtran_pendbuff);
PREDEF bool switch_up_sigtran_parse_message(unsigned int 	*user_id,
								     unsigned int	TRN_REC_local,
									 struct 		Ssigtran_fix_message_hdr *parse_fix_hdr,
									 void   		*msg_sigtran_pstartbuff,
									 void   		*msg_sigtran_pworkingbuff,
									 void   		*msg_sigtran_pendbuff);
PREDEF bool switch_down_sigtran_parse_message(unsigned int *user_id,
									   unsigned int	TRN_REC_local,
									   struct 		Ssigtran_fix_message_hdr *parse_fix_hdr,
									   void   		*msg_sigtran_pstartbuff,
									   void   		*msg_sigtran_pworkingbuff,
									   void   		*msg_sigtran_pendbuff);
PREDEF bool switch_up_sigtran_parse_parameter(unsigned int *user_id,
									   unsigned int	TRN_REC_local,
									   struct 		Ssigtran_fix_parameter_hdr *pparse_fix_parameter,
									   void   		*msg_sigtran_pstartbuff,
									   void   		*msg_sigtran_pworkingbuff,
									   void   		*msg_sigtran_pendbuff);

PREDEF void SIGTRAN__ASP_Up_Acknowledgement_ASPUPACK(
		unsigned int netcs_index,
		unsigned int *user_id);

PREDEF void SIGTRAN__ASP_Active_ASPAC(
		unsigned int netcs_index,
		unsigned int *user_id);

PREDEF void SIGTRAN__ASP_Active_Acknowledgement_ASPACACK(
		unsigned int netcs_index,
		unsigned int *user_id);

PREDEF void SIGTRAN__ASP_Down_ASPDN(
		unsigned int netcs_index,
		unsigned int *user_id);

PREDEF void* process_sigtran_thread(void *arg);

PREDEF void sigtran_USER_INDEX_RESET(int id);
PREDEF void sigtran_USER_INDEX_RESETALL(void);
PREDEF void sigtran_m3ua_USER_INDEX_RESETALL(void);

PREDEF void UA_sigtran_m3ua_USER_INDEX_INIT(int cnt, int key_mml_m3acon_index);
PREDEF bool UA_sigtran_m3ua_USER_INDEX_CMP(int cnt, int key_mml_m3acon_index);

PREDEF void NETCS_sigtran_tcp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy);
PREDEF void NETCS_sigtran_udp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy);
PREDEF void NETCS_sigtran_sctp_USER_INDEX_INIT(int id, int key_mml_trnloc_index, int key_mml_trncon_index);

PREDEF bool NETCS_sigtran_tcp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_sigtran_udp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_sigtran_sctp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_mml_trncon_index);

PREDEF bool NETCS_netcs_sigtran_tcp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_netcs_sigtran_udp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_netcs_sigtran_sctp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_mml_trncon_index);

PREDEF void NETCS_netcs_sigtran_tcp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy);
PREDEF void NETCS_netcs_sigtran_udp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy);
PREDEF void NETCS_netcs_sigtran_sctp_USER_INDEX_INIT(int id, int key_mml_trnloc_index, int key_mml_trncon_index);

PREDEF void SIGTRAN__ASP_Up_ASPUP(unsigned int netcs_index, unsigned int user_id);
PREDEF void SIGTRAN__ASP_Up_Acknowledgement_ASPUPACK(unsigned int netcs_index, unsigned int *user_id);


