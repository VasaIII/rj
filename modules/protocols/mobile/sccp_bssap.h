
#undef PREDEF
#ifdef BSSAP
#define PREDEF
#else
#define PREDEF extern
#endif

// *********************** BSS MAP PARAMETERS ***********************
#pragma pack(push, 1)


enum EbssMap_param_Message_Type {
	// Assignment messages
	// Handover messages
	// Release messages
	Clear_Command				= 0x20,///
	Clear_Complete				= 0x21,///
	Clear_Request				= 0x22,
	Sapi_N_Reject				= 0x25,
	Confusion					= 0x26,
	// Other connecton related messages
	Suspend						= 0x28,
	Resume						= 0x29,
	Connection_Oriented_Information	= 0x2a,
	Perform_Location_Request	= 0x2b,
	msg_LSA_Information			= 0x2c,
	Perform_Location_Response	= 0x2d,
	Perform_Location_Abort		= 0x2e,
	Common_ID					= 0x2f,

	// General messages
	Reset						= 0x30,///
	Reset_Ack					= 0x31,///
	Overload					= 0x32,
	Reserved					= 0x33,
	Reset_Circuit				= 0x34,///
	Reset_Circuit_Ack			= 0x35,///
	MSC_Invoke_Trace			= 0x36,
	BSS_Invoke_Trace			= 0x37,
	Conectionless_Information	= 0x3a,
	// Terrestial resource messages
	Block						= 0x40,///
	Blocking_Ack				= 0x41,///
	Unblock						= 0x42,///
	Unblocking_ack				= 0x43,///
	Circuit_Group_Block			= 0x44,
	Circuit_Group_Blocking_Ack	= 0x45,
	Circuit_Group_Unblock		= 0x46,
	Circuit_Group_Unblocking_Ack= 0x47,
	Unequipped_Circuit			= 0x48,
	Change_Circuit				= 0x4e,
	Change_Circuit_Ack			= 0x4f,
	// Radio Resource messages
	Resource_Request			= 0x50,
	Resource_Indication			= 0x51,
	Paging						= 0x52,///
	Cipher_Mode_Command			= 0x53,///
	Classmark_Update			= 0x54,
	Cipher_Mode_Complete		= 0x55,
	Queuing_Indication			= 0x56,
	Complete_Layer3_Information	= 0x57,///
	Classmark_Request			= 0x58,
	Cipher_Mode_Reject			= 0x59,
	Load_Indication				= 0x5a
	// VGCS/VBS
};

enum EbssMap_param_Element_Identifier {
	Circuit_Identity_Code			= 0x01,
	Reserved0,
	Resource_Available,
	Cause,
	Cell_Identifier,
	Priority,
	Layer_3_Header_Information,
	IMSI,
	TMSI,
	Encryption_Information,
	Channel_Type,
	Periodicity,
	Extended_Resource_Indicator,
	Number_Of_MSs,
	Reserved1,
	Reserved2,
	Reserved3,
	Classmark_Information_Type_2,
	Classmark_Information_Type_3,
	Interference_Band_To_Be_Used,
	RR_Cause,
	Reserved4,
	Layer_3_Information,
	DLCI,
	Downlink_DTX_Flag,
	Cell_Identifier_List,
	Response_Request,
	Resource_Indication_Method,
	Classmark_Information_Type_1,
	Circuit_Identity_Code_List,
	Diagnostic,
	Layer_3_Message_Contents,
	Chosen_Channel,
	Total_Resource_Accessible,
	Cipher_Response_Mode,
	Channel_Needed,
	Trace_Type,
	Triggerid,
	Trace_Reference,
	Transactionid,
	Mobile_Identity,
	OMCId,
	Forward_Indicaton,
	Chosen_Encryption_Algorithm,
	Circuit_Pool,
	Circuit_Pool_List,
	Time_Indication,
	Resource_Situation,
	Current_Channel_type_1,
	Queueing_Indicator,					/* 0x32 */
	Speech_Version						= 0x40,
	Assignment_Requirement				= 0x33,
	Talker_Flag							= 0x35,
	Connection_Release_Requested,
	Group_Call_Reference,
	eMLPP_Priority,
	Configuration_Evolution_Indication,
	Old_BSS_to_New_BSS_Information,
	LSA_Identifier,
	LSA_Identifier_List,
	LSA_Information,
	LCS_QoS,
	LSA_access_control_suppression,
	LCS_Priority						= 0x43,
	Location_Type,
	Location_Estimate,
	Positioning_Data,
	LCS_Cause,
	LCS_Client_Type,
	APDU,
	Network_Element_Identity,
	GPS_Assistance_Data,
	Deciphering_Keys,
	Return_Error_Request,
	Return_Error_Cause,
	bssMap_Segmentation,
	Service_Handover,
	Source_RNC_to_target_RNC_transparent_information_UMTS,
	Source_RNC_to_target_RNC_transparent_information_cdma2000,
	Reserved5							= 0x41,
	Reserved6							= 0x42
};


struct SbssMap_param_Cause {
	unsigned char Element_Identifier;
	unsigned char Length;
	unsigned char Cause_Value;
};

struct SbssMap_param_Cell_Identifier__fixpart {
	unsigned char Element_Identifier;
	unsigned char Length;
	unsigned char HALF_Spare_HALF_Cell_Identification_Discriminator;
	/* Cell_Identification is placed from octets 4-n,
	   depending on Cell_Identification_Discriminator */
};
struct SbssMap_param_Layer_3_Information {
	unsigned char Element_Identifier;
	unsigned char Length;
	/* Layer 3 information octets 3-n */
};
struct SbssMap_param_Chosen_Channel {
	unsigned char Element_Identifier;
};
struct SbssMap_param_LSA_Identifier_List {
	unsigned char Element_Identifier;
};

#pragma pack(pop)
// *********************** BSS MAP MESSAGES ***********************
#pragma pack(push, 1)

struct SbssMap_fix_Message_Type {
	unsigned char Message_Type;
	unsigned char Length;
};

struct SbssDtap_fix_Message_Type {
	unsigned char Message_Type;
	unsigned char DLCI;
	unsigned char Length;
};

struct SbssMap_message_Reset {
	unsigned char Message_Type;
	struct SbssMap_param_Cause Cause;
};

struct SbssMap_message_Complete_Layer3_Information__fixpart {
	unsigned char Message_Type;
	// Variable part mandatory parameters:
	// 		Cell_Identifier;
	// 		Layer_3_Information
	// Variable part optional parameters:
	// 		Chosen_Channel
	// 		LSA_Identifier_List
	// 		PADU
};


#pragma pack(pop)
// *********************** BSS DTAP PARAMETERS ***********************

// *********************** BSS DTAP MESSAGES ***********************


PREDEF bool bssap_parse(
		unsigned int m3ua_user_id,
		int parse_select,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff);

PREDEF bool switch_up_bssMap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff);

PREDEF bool switch_down_bssMap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff);

PREDEF bool switch_up_bssDtap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff);

PREDEF bool switch_down_bssDtap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff);

PREDEF void BSSAP_MAP__Reset(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset);

PREDEF void BSSAP_MAP__Complete_Layer3_Information(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset);

PREDEF void BSSAP_MAP__Cipher_Mode_Complete(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset);

PREDEF void BSSAP_MAP__Clear_Complete(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset);

PREDEF void OSI_SCTP_M3UA_SCCP_BSSAP_MAP(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int bssap_message_id);

PREDEF void OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int bssap_message_id);
