
#undef PREDEF
#ifdef RADIO
#define PREDEF
#else
#define PREDEF extern
#endif

// *********************** BSS MAP PARAMETERS ***********************
#pragma pack(push, 1)


// Table 10.1/3GPP TS 04.08 (page 1 of 2)
enum Ebss_Radio_message_RadioResourceManagement {
	// Channel establishment messages
	RR_INITIALISATION_REQUEST				,
	ADDITIONAL_ASSIGNMENT					,
	IMMEDIATE_ASSIGNMENT					,
	IMMEDIATE_ASSIGNMENT_EXTENDED			,
	IMMEDIATE_ASSIGNMENT_REJECT				,

	// Ciphering messages
	CIPHERING_MODE_COMMAND					,
	CIPHERING_MODE_COMPLETE					,

	// Configuration Change messages
	CONFIGURATION_CHANGE_COMMAND			,
	CONFIGURATION_CHANGE_ACK				,
	CONFIGURATION_CHANGE_REJECT				,

	// Handover messages
	ASSIGNMENT_COMMAND						,
	ASSIGNMENT_COMPLETE						,
	ASSIGNMENT_FAILURE						,
	HANDOVER_COMMAND						,
	HANDOVER_COMPLETE						,
	HANDOVER_FAILURE						,
	PHYSICAL_INFORMATION					,

	RR_CELL_CHANGE_ORDER					,
	PDCH_ASSIGNMENT_COMMAND					,

	// Channel release messages
	CHANNEL_RELEASE							,
	PARTIAL_RELEASE							,
	PARTIAL_RELEASE_COMPLETE				,

	// Paging and notification messages
	PAGING_REQUEST_TYPE_1					,
	PAGING_REQUEST_TYPE_2					,
	PAGING_REQUEST_TYPE_3					,
	PAGING_RESPONSE							,
	NOTIFICATION_NCH_9						,
	NOTIFICATION_FACCH						,
	NOTIFICATION_RESPONSE					,

	// System information messages
	SYSTEM_INFORMATION_TYPE_1				,
	SYSTEM_INFORMATION_TYPE_2				,
	SYSTEM_INFORMATION_TYPE_2bis			,
	SYSTEM_INFORMATION_TYPE_2ter			,
	SYSTEM_INFORMATION_TYPE_3				,
	SYSTEM_INFORMATION_TYPE_4				,
	SYSTEM_INFORMATION_TYPE_5				,
	SYSTEM_INFORMATION_TYPE_5bis			,
	SYSTEM_INFORMATION_TYPE_5ter			,
	SYSTEM_INFORMATION_TYPE_6				,
	SYSTEM_INFORMATION_TYPE_7				,
	SYSTEM_INFORMATION_TYPE_8				,
	SYSTEM_INFORMATION_TYPE_9				,
	SYSTEM_INFORMATION_TYPE_13				,
	SYSTEM_INFORMATION_TYPE_16				,
	SYSTEM_INFORMATION_TYPE_17				,

	// Specific messages for VBS/VGCS
	TALKER_INDICATION						,
	UPLINK_ACCESS							,
	UPLINK_BUSY								,
	UPLINK_FREE								,
	UPLINK_RELEASE							,
	VGCS_UPLINK_GRANT						,

	// Channel establishment messages
	EXTENDED_MEASUREMENT_ORDER				,
	EXTENDED_MEASUREMENT_REPORT				,

	// Miscellaneous messages
	CHANNEL_MODE_MODIFY						,
	CHANNEL_MODE_MODIFY_ACKNOWLEDGE			,
	CHANNEL_REQUEST							,
	CLASSMARK_CHANGE						,
	CLASSMARK_ENQUIRY						,
	FREQUENCY_REDEFINITION					,
	MEASUREMENT_REPORT						,
	SYNCHRONIZATION_CHANNEL_INFORMATION		,
	RR_STATUS								,
	GPRS_SUSPENSION_REQUEST					,

	// Application messages
	APPLICATION_INFORMATION
};

enum Ebss_Radio_message_MobilityManagement {
	// Registration messages
	IMSI_DETACH_INDICATION			=0x01,
	LOCATION_UPDATING_ACCEPT		=0x02,
	LOCATION_UPDATING_REJECT		=0x04,
	LOCATION_UPDATING_REQUEST		=0x08,

	// Security messages
	AUTHENTICATION_REJECT			=0x11,
	AUTHENTICATION_REQUEST			=0x12,
	AUTHENTICATION_RESPONSE			=0x14,
	IDENTITY_REQUEST				=0x18,
	IDENTITY_RESPONSE				=0x19,
	TMSI_REALLOCATION_COMMAND		=0x1a,
	TMSI_REALLOCATION_COMPLETE		=0x1b,

	// Connection management messages
	CM_SERVICE_ACCEPT				,
	CM_SERVICE_REJECT				,
	CM_SERVICE_ABORT				,
	CM_SERVICE_REQUEST				,
	CM_SERVICE_PROMPT				,
	CM_REESTABLISHMENT_REQUEST		,
	ABORT							,

	// Miscellaneous messages
	MM_NULL							,
	MM_STATUS						,
	MM_INFORMATION
};

enum Ebss_Radio_message_CallControl_callrelatedSS {
	// Call establishment messages
	ALERTING,
	CALL_CONFIRMED,
	CALL_PROCEEDING,
	CONNECT,
	CONNECT_ACK,
	EMERGENCY_SETUP,
	PROGRESS,
	CC_ESTABLISHMENT,
	CC_ESTABLISHMENT_CONFIRMED,
	RECALL,
	START_CC,
	SETUP,

	// Call information phase messages
	MODIFY,
	MODIFY_COMPLETE,
	MODIFY_REJECT,
	USER_INFORMATION,
	HOLD,
	HOLD_ACK,
	HOLD_REJECT,
	RETRIEVE,
	RETRIEVE_ACK,
	RETRIEVE_REJECT,

	// Call clearing messages
	DISCONNECT,
	RELEASE,
	RELEASE_COMPLETE,

	// Miscellaneous messages
	CONGESTION_CONTROL,
	NOTIFY,
	STATUS,
	STATUS_ENQUIRY,
	START_DTMF,
	STOP_DTMF,
	STOP_DTMF_ACK,
	START_DTMF_ACK,
	START_DTMF_REJECT,
	FACILITY
};


#pragma pack(pop)
// *********************** BSS MAP MESSAGES ***********************
#pragma pack(push, 1)

#define RADIO_TRACE(en)  case en: ErrorTraceHandle(2, "RADIO(%d) Discrimination %s(0x%x)\n", dlg_ref, #en, en);

enum Ebss_Radio_param_ProtocolDiscriminator {
	Call_Control_call_Related_SS_messages 		=0x3,
	MobilityManagement_messages_nonGPRS_service	=0x5,
	RadioResource_management_messages 			=0x6,
	MobilityManagement_messages_GPRS_service	=0x8,
	SessionManagement_messages					=0xa
};

#pragma pack(pop)
// *********************** BSS DTAP PARAMETERS ***********************

PREDEF void parse_radio_message(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset);

PREDEF void RADIO_MM__LOCATION_UPDATING_REQUEST(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset);

PREDEF void RADIO_MM__AUTHENTICATION_RESPONSE(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset);

PREDEF void RADIO_MM__TMSI_REALLOCATION_COMPLETE(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset);



