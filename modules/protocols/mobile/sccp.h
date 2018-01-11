
#undef PREDEF
#ifdef SCCP
#define PREDEF
#else
#define PREDEF extern
#endif

// Mandatory fix 	  length part
// Mandatory variable length part (pointers)
// Optional part

// *********************** PARAMETERS ***********************
#pragma pack(push, 1)
enum Esccp_parameter_name_code {
	End_Of_Optional_Parameters		= 0x00,
	Destination_Local_Reference		= 0x01,
	Source_Local_Reference			= 0x02,
	Called_Party_Address			= 0x03,
	Calling_Party_Address			= 0x04,
	Protocol_Class					= 0x05,
	Segmenting_Ressasembling		= 0x06,
	Sequencing_Ressasembling		= 0x08,
	Credit							= 0x09,
	Release_Cause					= 0x0A,
	Return_Cause					= 0x0B,
	Error_Cause						= 0x0D,
	Refusal_Cause					= 0x0E,
	Data							= 0x0F,
	sccp_Segmentation				= 0x10,
	Hop_Counter_sccp				= 0x11, // there is also same ISUP enum
	Importance						= 0x12,
	Long_Data						= 0x13
};

struct SCalled_Party_Address__fixpart {
	unsigned char AI;
};
struct SCalling_Party_Address__fixpart {
	unsigned char AI;
};

#pragma pack(pop)
// *********************** MESSAGES ***********************
#pragma pack(push, 1)

enum Esccp_fix_message_hdr_type_code {
	CR_Connection_Request			= 0x01,
	CC_Connection_Confirm			= 0x02,
	CREF_Connection_Refused			= 0x03,
	RLSD_Released					= 0x04,
	RLC_Release_Complete_sccp		= 0x05, // there is also same ISUP enum
	DT1_Data_Form_1					= 0x06,
	UDT_Unidata						= 0x09,
	UDTS_Unidata_Service			= 0x0A,
	ERR_Protocol_Data_Unit_Service	= 0x0F,
	IT_Inactivity_Test				= 0x10,
	XUDT_Extended_Unidata			= 0x11,
	XUDTS_Extended_Unidata_Service	= 0x12,
	LUDT_Long_Unidata				= 0x13,
	LUDTS_Long_Unidata_Service		= 0x14
};
enum Esccp_Management_Message_type_code {
	Subsystem_Allowed_SSA			= 0x01,
	Subsystem_Prohibited_SSP		= 0x02,
	Subsystem_Status_Test_SST		= 0x03,
	Subsystem_Congested_SSC 		= 0x06
};

struct SData_SCCP_Management_Message{
	unsigned char Message_Type;
	unsigned char Affected_SSN;
	short int     Affected_SPC;
	unsigned char SubSystem_Multiplicity_Indicator;
};

struct Ssccp_Mandatory_Part__fixpart {
	unsigned char Length_Indicator;
};

struct Ssccp_Optional_Part__fixpart {
	unsigned char Parameter_Name;
	unsigned char Length_Indicator;
};

struct SUDT_Unidata__fixpart {
	unsigned char Message_Type;
	unsigned char Protocol_Class;
	unsigned char Offset_Called_Party_Address;
	unsigned char Offset_Calling_Party_Address;
	unsigned char Offset_Data;
};

struct SDT1_Data_Form_1__fixpart {
	unsigned char Message_Type;
	unsigned char Destination_Local_Reference[3];
	unsigned char Segmenting_Ressasembling;
	unsigned char Offset_Data;
};


struct SCR_Connection_Request {
	unsigned char Message_Type;
	unsigned char Source_Local_Reference[3];
	unsigned char Protocol_Class;
	unsigned char Offset_Called_Party_Address;
	unsigned char Offset_Optional_Part;
};

struct SCC_Connection_Confirm {
	unsigned char Message_Type;
	unsigned char Destination_Local_Reference[3];
	unsigned char Source_Local_Reference[3];
	unsigned char Protocol_Class;
	unsigned char Offset_Optional_Part;
};

struct SRLSD_Released {
	unsigned char Message_Type;
	unsigned char Destination_Local_Reference[3];
	unsigned char Source_Local_Reference[3];
	unsigned char Release_Cause;
	unsigned char Offset_Optional_Part;
};

struct SRLC_Release_Complete {
	unsigned char Message_Type;
	unsigned char Destination_Local_Reference[3];
	unsigned char Source_Local_Reference[3];
};

#pragma pack(pop)

// *********************** SCCP USER DATA ***********************

enum Esccp_common_connection_ref_fifo_user_type {
	BSS_lu		= 0x1,
	BSS_call	= 0x2
};

#define DIALOG_REF_AVAILABLE			0x0
#define DIALOG_REF_IN_PROGRESS			0x1

struct Ssccp_common_connection_ref {
	struct Ssccp_common_connection_ref_fifo {

		short int user_type; // enum Esccp_common_connection_ref_fifo_user_type
		struct Ssccp_common_connection_ref_fifo_SSN254_bss_bsc_LU_data {
			char 			IMSI[20]; 	// 6-15 digits
			unsigned char 	IMSI_length;
			unsigned int 	state;		// LU state in dialog
			bool 			verdict;	// LU state in dialog
		} vBSS_caller_locupdate;

		unsigned int  mml_call_index; // used also to clean dialogs when thread is terminated
		unsigned int  dest_dlg_ref[3];
		unsigned char nsd; 			 // bit 7 is used for send sequence toggling (N) bit
		unsigned int  mask;			 // dialog status
	} fifo[MASK_sts_sccp_dlg_ref+1];

	SAFE_COUNTER_DECLARATION(sts_sccp_dlg_ref, fifo_source_ref_active);
	 STS_COUNTER_DECLARATION(sts_sccp_dlg_ref, fifo_source_ref);
} sccp_common_connection_ref;
// * this structure for sccp dialog references is common pool for all underlying protocols (m3ua)


#define SCCP_FIFO_DLG_REF           			STS_COUNTER_VALUE(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref)
#define SCCP_FIFO_DLG_REF_INCREMENT   			STS_COUNTER_INCREMENT(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref, 1)
#define SCCP_FIFO_DLG_REF_SET(value)			STS_COUNTER_SET(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref,  value)
#define SCCP_FIFO_DLG_REF_MASK(dlg_ref)   		sccp_common_connection_ref.fifo[dlg_ref].mask
#define SCCP_FIFO_DLG_REF_USER_TYPE(dlg_ref)	sccp_common_connection_ref.fifo[dlg_ref].user_type

#define SAFE_VALUE_DLG_REF_start				SAFE_COUNTER_VALUE(sts_sccp_dlg_ref, DLG_REF_start)

#define POP_DIALOG_REF(userinfo, set, DLG_REF_out) {\
					int DLG_REF_occupied = 0; \
					SAFE_COUNTER_DECLARATION(sts_sccp_dlg_ref, DLG_REF_start); \
					NETCS_MACROSAFE_IN(set, ePOP_DIALOG_REF); \
					SAFE_COUNTER_SET(sts_sccp_dlg_ref, DLG_REF_start, SCCP_FIFO_DLG_REF); \
					while(SCCP_FIFO_DLG_REF_MASK(SAFE_VALUE_DLG_REF_start) != DIALOG_REF_AVAILABLE) \
						{if (++DLG_REF_occupied>=MASK_sts_sccp_dlg_ref) { \
							FAST4ErrorTraceHandle(2, "POP_DIALOG_REF: %s SET-%d> DLG_REF-%d, all locations occupied, need to redimensione size.\n", \
									              #userinfo, set, SAFE_VALUE_DLG_REF_start); \
							DLG_REF_occupied = 0; \
							SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[POP_DIALOG_REF_all_occupied], 1); \
						 } \
						 SAFE_COUNTER_INCREMENT(sts_sccp_dlg_ref, DLG_REF_start, 1);} \
					SCCP_FIFO_DLG_REF_MASK(SAFE_VALUE_DLG_REF_start) = DIALOG_REF_IN_PROGRESS; \
					SCCP_FIFO_DLG_REF_SET( SAFE_VALUE_DLG_REF_start); \
					DLG_REF_out          = SAFE_VALUE_DLG_REF_start; \
					FAST4ErrorTraceHandle(2, "POP_DIALOG_REF: %s SET-%d> DLG_REF-%d fetched.\n", \
										  #userinfo, set, DLG_REF_out); \
					SAFE_COUNTER_INCREMENT(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref_active, 1); \
					NETCS_MACROSAFE_OUT(set, ePOP_DIALOG_REF); \
					}

#define PUSH_DIALOG_REF(userinfo, set, DLG_REF_in) {\
					NETCS_MACROSAFE_IN(set, ePUSH_DIALOG_REF); \
					SCCP_FIFO_DLG_REF_MASK(DLG_REF_in) = DIALOG_REF_AVAILABLE; \
					SAFE_COUNTER_DECREMENT(sts_sccp_dlg_ref, sccp_common_connection_ref.fifo_source_ref_active, 1); \
					FAST4ErrorTraceHandle(2, "PUSH_DIALOG_REF: %s SET-%d> DLG_REF-%d released.\n", \
										  #userinfo, set, DLG_REF_in); \
					NETCS_MACROSAFE_OUT(set, ePUSH_DIALOG_REF); \
					}

#define DIALOG_PROGRESS(userinfo, set, DLG_REF_in, m3ua_user_index, in_mood_for) {\
					switch((enum Esccp_common_connection_ref_fifo_user_type) SCCP_FIFO_DLG_REF_USER_TYPE(DLG_REF_in)) { \
					case BSS_lu: BSS_DIALOG_PROGRESS_LU(DLG_REF_in, m3ua_user_index, userinfo, in_mood_for); break;\
					default: FAST5ErrorTraceHandle(1, "DIALOG_PROGRESS: %s SET-%d> DLG_REF-%d user type (%d) not supported !\n", \
												   #userinfo, set, DLG_REF_in, SCCP_FIFO_DLG_REF_USER_TYPE(DLG_REF_in)); break; \
					}}

#define DIALOG_EXTRACT(array) (array[2]<<8*2) |	(array[1]<<8*1) |(array[0]<<8*0)

// *********************** SCCP INITIALIZATION DATA ***********************

PREDEF unsigned int sccp_parse(unsigned int  m3ua_user_index,
			int    parse_select,
			void  *msg_m3ua_pstartbuff,
			void  *msg_sccp_pstartbuff,
			void  *msg_m3ua_pendbuff);

PREDEF bool switch_up_sccp_parse_message(unsigned int 	 m3ua_user_index,
			unsigned char Vsccp_fix_message_hdr_type_code,
			void  *msg_m3ua_pstartbuff,
			void  *msg_sccp_pstartbuff,
			void  *msg_m3ua_pendbuff);

PREDEF void SCCP_param__Called_Party_Address(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN);

PREDEF void SCCP_param__Calling_Party_Address(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN);

PREDEF void SCCP__CR_Connection_Request(
		unsigned int 	m3ua_user_index,
		unsigned int 	dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN);

PREDEF void SCCP__UDT_Unidata(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN);

PREDEF void SCCP__DT1_Data_Form_1(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset);

PREDEF void SCCP__RLC_Release_Complete(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset);

PREDEF void SCCP__Subsystem_Allowed_SSA(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset);

PREDEF void OSI_SCTP_M3UA_SCCP__Subsystem_Allowed_SSA(
		int m3ua_user_index);

PREDEF void OSI_SCTP_M3UA_SCCP(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	sccp_message_id);

PREDEF void sccp_fifo_RESETALL(void);
