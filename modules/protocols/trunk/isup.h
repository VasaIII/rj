
#undef PREDEF
#ifdef ISUP_C
#define PREDEF
#else
#define PREDEF extern
#endif


/* **********************************************************

	CIC
	Message Type Code
	Mandatory Fixed Parameter mfA     				\
	Mandatory Fixed Parameter mfB     				|
	.												| ----> Mandatory Fixed Part
	.												|
	Mandatory Fixed Parameter mfF					/
	Pointer to Mandatory Variable Parameter mvA		\
	Pointer to Mandatory Variable Parameter mvB		|
	.												|
	.												|
	Pointer to Mandatory Variable Parameter mvF		|
	Pointer to start of  Optional Parameters		|
	. mvA length									| ---> Mandatory Variable Part
	. mvA content									|
	. mvB length									|
	. mvB content									|
	.												|
	.												|
	. mvF length									|
	. mvF content									/
	. oA  name										\
	. oA  length									|
	. oA  content									|
	. oB  name										|
	. oB  length									|
	. oB  content									| ---> Optional Part
	.												|
	.												|
	. oF  name										|
	. oF  length									|
	. oF  content									/





   *********************************************************/

// *********************** PARAMETERS ***********************
#pragma pack(push, 1)

enum Eisup_fix_message_hdr_message_type {
	ACM_Address_Complete  								= 0x06,
	ANM_Answer											= 0x09,
	BLO_Blocking										= 0x13,
	BLA_Blocking_Acknowledgment 						= 0x15,
	CMC_Call_Modification_Completed 					= 0x1d,
	CMRJ_Call_Modification_Reject 						= 0x1e,
	CMR_Call_Modification_Request 						= 0x1c,
	CPG_Call_Progress									= 0x2c,
	CGB_Circuit_Group_Blocking 							= 0x18,
	CGBA_Circuit_Group_Blocking_Acknowledgment 			= 0x1a,
	CQM_Circuit_Group_Query 							= 0x2a,
	CQR_Circuit_Group_Query_Response 					= 0x2b,
	GRS_Circuit_Group_Reset 							= 0x17,
	GRA_Circuit_Group_Reset_Acknowledgment 				= 0x29,
	CGU_Circuit_Group_Unblocking 						= 0x19,
	CGUA_Circuit_Group_Unblocking_Acknowledgment 		= 0x33,
	CRM_Circuit_Reservation 							= 0xea,
	CRA_Circuit_Reservation_Acknowledgment 				= 0xe9,
	CVR_Circuit_Validation_Response 					= 0xeb,
	CVT_Circuit_Validation_Test 						= 0xec,
	CSVR_CUG_Selection_and_Validation_Request 			= 0x25,
	CSVS_CUG_Selection_and_Validation_Response 			= 0x26,
	CRG_Charge_Information 								= 0x31,
	CFN_Confusion 										= 0x2f,
	CON_Connect 										= 0x07,
	COT_Continuity 										= 0x05,
	CCR_Continuity_Check_Request 						= 0x11,
	DRS_Delayed_Release 								= 0x27,
	EXM_Exit 											= 0xed,
	FAC_Facility 										= 0x33,
	FAA_Facility_Accepted 								= 0x20,
	FAD_Facility_Deactivated 							= 0x22,
	FAI_Facility_Information 							= 0x23,
	FRJ_Facility_Reject 								= 0x21,
	FAR_Facility_Request 								= 0x1f,
	FOT_Forward_Transfer 								= 0x08,
	IDR_Identification_Request 							= 0x36,
	IRS_Identification_Response 						= 0x37,
	IAM_Initial_Address_Message 						= 0x01,
	INF_Information 									= 0x04,
	INR_Information_Request 							= 0x03,
	LPA_Loop_Back_Acknowledgment 						= 0x24,
	NRM_Network_Resource_Management 					= 0x32,
	OLM_Overload 										= 0x30,
	PAM_Pass_along 										= 0x28,
	REL_Release 										= 0x0c,
	RLC_Release_Complete_isup							= 0x10, // there is also same SCCP enum
	RSC_Reset_Circuit 									= 0x12,
	RES_Resume 											= 0x1e,
	SGM_Segmentation 									= 0x38,
	SAM_Subsequent_Address 								= 0x02,
	SUS_Suspend 										= 0x0d,
	UBL_Unblocking 										= 0x14,
	UBA_Unblocking_Acknowledgment 						= 0x16,
	UCIC_Unequipped_CIC 								= 0x2e,
	UPA_User_Part_Available 							= 0x35,
	UPT_User_Part_Test 									= 0x34,
	USR_User_to_user_Information 						= 0x2d
};

enum Eisup_fix_message_hdr_parameter_type {
	Access_Delivery_Information 						= 0x2e,
	Access_Transport 									= 0x03,
	Automatic_Congestion_Level 							= 0x27,
	Backward_Call_Indicators 							= 0x11,
	Business_Group 										= 0xc6,
	Call_Diversion_Information 							= 0x36,
	Call_History_Information 							= 0x2d,
	Call_Modification_Indicators 						= 0x17,
	Call_Reference 										= 0x01,
	Called_Party_Number 								= 0x04,
	Calling_Party_Number 								= 0x0a,
	Calling_Partys_Category 							= 0x09,
	Carrier_Identification 								= 0xc5,
	Carrier_Selection_Information 						= 0xee,
	Cause_Indicators 									= 0x12,
	Charge_Number 										= 0xeb,
	Circuit_Assignment_Map 								= 0x25,
	Circuit_Group_Characteristic_Indicator 				= 0xe5,
	Circuit_Group_Supervision_Message_Type_Ind  		= 0x15,
	Circuit_Identification_Name 						= 0xe8,
	Circuit_State_Indicator 							= 0x26,
	Circuit_Validation_Response_Indicator 				= 0xe6,
	CUG_Check_Response_Indicators 						= 0x1c,
	CUG_Interlock_Code 									= 0x1b,
	COMMON_LANGUAGE 									= 0xe9,
	Connected_Number 									= 0x21,
	Connection_Request 									= 0x0d,
	Continuity_Indicators 								= 0x10,
	Echo_Control_Information 							= 0x37,
	Egress												= 0xc3,
	End_of_Optional_Parameters 							= 0x00,
	Event_Information_Indicators 						= 0x24,
	Facility_Indicator 									= 0x18,
	Facility_Information_Indicators 					= 0x19,
	Forward_Call_Indicators 							= 0x07,
	Freephone_Indicators 								= 0x41,
	Generic_Address 									= 0xc0,
	Generic_Digits 										= 0xc1,
	Generic_Name 										= 0xc7,
	Generic_Notification 								= 0x2c,
	Generic_Number 										= 0xc0,
	Generic_Reference 									= 0x42,
	Hop_Counter_isup									= 0x3d, // there is also same SCCP message
	Index 												= 0x1b,
	Information_Indicators 								= 0x0f,
	Information_Request_Indicators 						= 0x0e,
	Jurisdiction 										= 0xc4,
	Location_Number 									= 0x3f,
	MCID_Request_Indicator 								= 0x3b,
	MCID_Response_Indicator 							= 0x3c,
	Message_Compatibility_Information 					= 0x38,
	MLPP_Precedence 									= 0x3a,
	Nature_of_Connection_Indicators 					= 0x06,
	Network_Specific_Facilities 						= 0x2f,
	Network_Transport 									= 0xef,
	Notification_Indicator 								= 0xe1,
	Operator_Services_Information 						= 0xc2,
	Optional_Backward_Call_Indicators 					= 0x29,
	Optional_Forward_Call_Indicators 					= 0x08,
	Original_Called_Number 								= 0x28,
	Originating_Line_Information 						= 0xea,
	Origination_ISC_Point_Code 							= 0x2b,
	Outgoing_Trunk_Group_Number 						= 0xe7,
	Parameter_Compatibility_Information 				= 0x39,
	Precedence											= 0x3b,
	Propagation_Delay_Counter 							= 0x31,
	Range_and_Status 									= 0x16,
	Redirecting_Number 									= 0x0b,
	Redirection_Information 							= 0x13,
	Redirection_Number 									= 0x0c,
	Redirection_Number_Restriction 						= 0x40,
	Remote_Operations 									= 0x32,
	Service_Activation 									= 0x33,
	Service_Code_Indicator 								= 0xec,
	Signaling_Point_Code 								= 0x1e,
	Special_Processing_Request 							= 0xed,
	Subsequent_Number 									= 0x05,
	Suspend_resume_Indicators 							= 0x22,
	Transaction_Request 								= 0xe3,
	Transit_Network_Selection 							= 0x23,
	Transmission_Medium_Requirement 					= 0x02,
	Transmission_Medium_Requirement_Prime 				= 0x3e,
	Transmission_Medium_Used 							= 0x35,
	User_Service_Information 							= 0x1d,
	User_Service_Information_Prime 						= 0x30,
	User_Teleservice_Information 						= 0x34,
	User_to_user_Indicators 							= 0x2a,
	User_to_user_Information 							= 0x20
};

struct Sisup_Madatory_Variable_Part {
	unsigned char Length_Indicator;
};

struct Sisup_Optional_Part {
	unsigned char Parameter_Name;
	unsigned char Length_Indicator;
};

#pragma pack(pop)
// *********************** MESSAGES ***********************
#pragma pack(push, 1)

struct Sisup_fix_message_hdr {
	short int 		cic;
	unsigned char 	message_type;
};

struct SIAM_Initial_Address_Message__fixpart {
	short int 		cic;
	unsigned char 	message_type;
	unsigned char 	Nature_of_Connection_Indicators;
	short int	 	Forward_Call_Indicators;
	unsigned char 	Calling_Partys_Category;
	unsigned char 	Transmission_Medium_Requirement;
};

#pragma pack(pop)
// *********************** ISUP USER DATA ***********************
#pragma pack(push, 1)

enum eCicDeviceState {
	cic_unequipped = 0,
	cic_idle,
	cic_busy,
	cic_manual_blocked,
	cic_auto_blocked
};

struct Sisup_cic_id {
	struct Sisup_cic_id_data {
		struct Sisup_cic_id_data_num {
			unsigned char num[20];
			unsigned char length;
		} A, B;

		unsigned int 	device_state;		// ISUP call state 		(eCicDeviceState)

		unsigned int 	call_state;			// ISUP call state 		(Epstn_call_state)
		bool 			call_verdict;		// ISUP call verdict 	(Everdict)
	} data[MASK_sts_isup_cic_id+1];

	STS_COUNTER_DECLARATION(sts_isup_cic_id, originating_active);
	STS_COUNTER_DECLARATION(sts_isup_cic_id, terminating_active);
	STS_COUNTER_DECLARATION(sts_isup_cic_id, current);
};
#pragma pack(pop)
// ***************************************************************

#define SAFE_VALUE_ISUP_CIC_ID_start		SAFE_COUNTER_VALUE(sts_isup_cic_id, ISUP_CIC_ID_start)

// for originating CIC requests
#define POP_ISUP_CIC_ID(userinfo, set, ISUP_CIC_ID_out) {\
					unsigned int ISUP_CIC_ID_round = 0; \
					SAFE_COUNTER_DECLARATION(sts_isup_cic_id, ISUP_CIC_ID_start); \
					NETCS_MACROSAFE_IN(set, ePOP_ISUP_CIC_ID); \
					SAFE_COUNTER_SET(sts_isup_cic_id, ISUP_CIC_ID_start,  STS_COUNTER_VALUE(sts_isup_cic_id,  isup_cic_id->current)); \
					SAFE_COUNTER_SET(sts_isup_cic_id, ISUP_CIC_ID_round, SAFE_COUNTER_VALUE(sts_isup_cic_id, ISUP_CIC_ID_start)); \
					while(isup_cic_id->data[SAFE_VALUE_ISUP_CIC_ID_start].device_state != (enum eCicDeviceState) cic_idle) { \
						 SAFE_COUNTER_INCREMENT(sts_isup_cic_id, ISUP_CIC_ID_start, 2/*take only even cic numbers, peer will use odd ones*/); \
						 if (ISUP_CIC_ID_round==ISUP_CIC_ID_start) { /*one round of CICs preformed*/ \
							ISUP_CIC_ID_round = MASK_sts_isup_cic_id;  /*set MASK as invalid value*/ \
							SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[POP_ISUP_CIC_ID_all_occupied], 1); \
							FAST4ErrorTraceHandle(2, "POP_ISUP_CIC_ID: %s SET-%d> CIC-%d, no available CICs, try later.\n", \
									              #userinfo, set, SAFE_VALUE_ISUP_CIC_ID_start); \
							break; \
						 } \
					} \
					if (ISUP_CIC_ID_round != MASK_sts_isup_cic_id) {\
						isup_cic_id->data[SAFE_VALUE_ISUP_CIC_ID_start].device_state = (enum eCicDeviceState) cic_busy; \
						ISUP_CIC_ID_out = SAFE_VALUE_ISUP_CIC_ID_start; \
						FAST5ErrorTraceHandle(2, "POP_ISUP_CIC_ID: %s SET-%d> CIC-%d (device_state=%d) fetched.\n", \
											  #userinfo, set, ISUP_CIC_ID_out, SAFE_VALUE_ISUP_CIC_ID_start); \
						STS_COUNTER_SET(sts_isup_cic_id, isup_cic_id->current, SAFE_VALUE_ISUP_CIC_ID_start + 2); \
						STS_COUNTER_INCREMENT(sts_isup_cic_id, isup_cic_id->originating_active, 1); \
					} \
					NETCS_MACROSAFE_OUT(set, ePOP_ISUP_CIC_ID); \
					}

// for terminating CIC requests
#define POP_ISUP_PREDEF_CIC_ID(userinfo, set, ISUP_CIC_ID_in_out) {\
					NETCS_MACROSAFE_IN(set, ePUSH_ISUP_PREDEF_CIC_ID); \
					if (isup_cic_id->data[ISUP_CIC_ID_in_out].device_state != (enum eCicDeviceState) cic_idle) {\
						FAST5ErrorTraceHandle(2, "POP_ISUP_PREDEF_CIC_ID: %s SET-%d> CIC-%d (device_state=%d) requested is not idle, ignore.\n", \
											  #userinfo, set, ISUP_CIC_ID_in_out, isup_cic_id->data[ISUP_CIC_ID_in_out].device_state); \
						ISUP_CIC_ID_in_out = INVALID_sts_isup_cic_id; \
					} else { \
						isup_cic_id->data[ISUP_CIC_ID_in_out].device_state = (enum eCicDeviceState) cic_busy; \
						FAST5ErrorTraceHandle(2, "POP_ISUP_PREDEF_CIC_ID: %s SET-%d> CIC-%d (device_state=%d) fetched.\n", \
											  #userinfo, set, ISUP_CIC_ID_in_out, isup_cic_id->data[ISUP_CIC_ID_in_out].device_state); \
											  STS_COUNTER_INCREMENT(sts_isup_cic_id, isup_cic_id->terminating_active, 2); \
					} \
					NETCS_MACROSAFE_OUT(set, ePUSH_ISUP_PREDEF_CIC_ID);


#define PUSH_ISUP_CIC_ID(userinfo, set, orig_0_term1, ISUP_CIC_ID_in) {\
					NETCS_MACROSAFE_IN(set, ePUSH_ISUP_CIC_ID); \
					FAST5ErrorTraceHandle(2, "PUSH_ISUP_CIC_ID(enter) %s SET-%d> CIC-%d (device_state=%d) releasing.\n", \
										  #userinfo, set, ISUP_CIC_ID_in, isup_cic_id->data[ISUP_CIC_ID_in].device_state); \
				    isup_cic_RESET(m3ua_user_index, ISUP_CIC_ID_in); \
					isup_cic_id->data[ISUP_CIC_ID_in].device_state = (enum eCicDeviceState) cic_idle; \
					if (orig_0_term1 == 0) STS_COUNTER_DECREMENT(sts_isup_cic_id, isup_cic_id->originating_active, 1) \
						else STS_COUNTER_DECREMENT(sts_isup_cic_id, isup_cic_id->terminating_active, 1) \
					FAST5ErrorTraceHandle(2, "PUSH_ISUP_CIC_ID(exit)  %s SET-%d> CIC-%d (device_state=%d) released.\n", \
										  #userinfo, set, ISUP_CIC_ID_in, isup_cic_id->data[ISUP_CIC_ID_in].device_state); \
					NETCS_MACROSAFE_OUT(set, ePUSH_ISUP_CIC_ID); \
					}


#define ISUP_CIC_DEVICESTATE_GET(cic_id) 			isup_cic_id->data[cic_id].device_state
#define ISUP_CIC_DEVICESTATE_SET(cic_id, new_state)	isup_cic_id->data[cic_id].device_state = new_state;
#define ISUP_CIC_CALLSTATE_GET(cic_id)   			isup_cic_id->data[cic_id].call_state
#define ISUP_CIC_CALLSTATE_SET(cic_id, new_state)   isup_cic_id->data[cic_id].call_state = PSTN_MASK(new_state);


#define CONFIGURE_CIC_ID(purpose, CFG_cic_id_local, CFG_call_state, CFG_call_verdict, CFG_Anum_length, CFG_Anum_string, CFG_Bnum_length, CFG_Bnum_string); \
		isup_cic_id->data[CFG_cic_id_local].call_state 		= PSTN_MASK(CFG_call_state); /* initial call state*/ \
		isup_cic_id->data[CFG_cic_id_local].call_verdict 	= CFG_call_verdict; \
		isup_cic_id->data[CFG_cic_id_local].A.length 		= CFG_Anum_length; \
		memcpy(isup_cic_id->data[CFG_cic_id_local].A.num, CFG_Anum_string, Anum_length); \
		isup_cic_id->data[CFG_cic_id_local].B.length 		= CFG_Bnum_length, Bnum_string; \
		memcpy(isup_cic_id->data[CFG_cic_id_local].B.num, CFG_Bnum_string, Bnum_length); \
		FAST8ErrorTraceHandle(2, "CONFIGURE_CIC_ID(CIC-%d) %s call_state=0x%x, Anum %s(%d), Bnum %s(%d)\n", \
				CFG_cic_id_local, \
				#purpose, \
				isup_cic_id->data[CFG_cic_id_local].call_state, \
				isup_cic_id->data[CFG_cic_id_local].A.num, \
				isup_cic_id->data[CFG_cic_id_local].A.length, \
				isup_cic_id->data[CFG_cic_id_local].B.num, \
				isup_cic_id->data[CFG_cic_id_local].B.length);


PREDEF unsigned int isup_parse(unsigned int  m3ua_user_index,
						int 		  parse_select,
						void   		 *msg_m3ua_pstartbuff,
						void   		 *msg_isup_pstartbuff,
						void   		 *msg_m3ua_pendbuff);

PREDEF bool switch_up_isup_parse_message(unsigned int 	 m3ua_user_index,
								  struct 		Sisup_fix_message_hdr *parse_fix_hdr,
								  void   		*msg_m3ua_pstartbuff,
								  void   		*msg_isup_pstartbuff,
								  void   		*msg_m3ua_pendbuff);

PREDEF void OSI_SCTP_M3UA_ISUP(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	isup_message_id);

PREDEF void isup_cic_RESET(unsigned int m3ua_user_index, unsigned int cic_id);
