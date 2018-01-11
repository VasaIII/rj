
#define SCCP
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include <modules/protocols/sigtran.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/mobile/sccp_bssap.h>
#include <modules/protocols/mobile/radio.h>
#include "modules/axe/application/bss/bss.h"
#undef SCCP

unsigned int sccp_parse(unsigned int  m3ua_user_index,
						int 		  parse_select,
						void   		 *msg_m3ua_pstartbuff,
						void   		 *msg_sccp_pstartbuff,
						void   		 *msg_m3ua_pendbuff)
{
	PARSEPREAMBLE("sccp_parse");

	FAST6ErrorTraceHandle(2, "sccp_parse(select=%d, m3ua_user_index=%d) UP: parse M3UA<0x%x-SCCP<0x%x->-0x%x>\n",
						 parse_select,
						 m3ua_user_index,
						 msg_m3ua_pstartbuff,
						 msg_sccp_pstartbuff,
						 msg_m3ua_pendbuff);

	SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pstartbuff, msg_m3ua_pendbuff);

	switch ((enum ENETCS_ParsedProtocols) parse_select) {

	// ******************************************************************
	// Action on messages level (parse_select)
	// ******************************************************************

	ECASE_TRACE(case_string, sccp_HEADER);
	{
		bool parsed 			= false;
		unsigned char Vsccp_fix_message_hdr_type_code =  (unsigned char) *((unsigned char *) msg_sccp_pstartbuff); // 1 byte

		FAST3ErrorTraceHandle(2, "sccp_HEADER(m3ua_user_index=%d): Type code = %d\n",
							  m3ua_user_index, Vsccp_fix_message_hdr_type_code);

		// ******************************************************************
		// Action on messages level, discrimination on message class
		// ******************************************************************

		case_string[0] = 0;
		parsed = switch_up_sccp_parse_message(m3ua_user_index, Vsccp_fix_message_hdr_type_code, msg_m3ua_pstartbuff, msg_sccp_pstartbuff, msg_m3ua_pendbuff);

		// ******************************************************************
		// ReAction on messages level (parse_select) ... not needed, if sccp terminated message (management) it is replied on upstream analisys
		// ******************************************************************
	}
	break; // sccp_HEADER

	// ******************************************************************
	// parameter level (parse_select) .. no parameter level since parameters are not standalone subjects with parameter tag and parameter length (as in m3ua)
	// ******************************************************************

	}

	return m3ua_user_index;
}


bool switch_up_sccp_parse_message(unsigned int 	 m3ua_user_index,
								  unsigned char Vsccp_fix_message_hdr_type_code,
								  void   		*msg_m3ua_pstartbuff,
								  void   		*msg_sccp_pstartbuff,
								  void   		*msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int buffer_write_counter = 0xFFFF;
	PARSEPREAMBLE("sccp");

	switch ((enum Esccp_fix_message_hdr_type_code) Vsccp_fix_message_hdr_type_code) {

	RECV_MSG_TRACE_case(case_string, UDT_Unidata);
	{
		unsigned char *msg_sccp_pworkingbuff = msg_sccp_pstartbuff;
		struct SUDT_Unidata__fixpart *pSUDT_Unidata_fixpart = (struct SUDT_Unidata__fixpart *) msg_sccp_pworkingbuff;

		FAST7ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): \nMessage_Type  = 0x%x\nProtocol_Class  = 0x%x\nOffset1  = %d\nOffset2  = %d\nOffset3  = %d\n",
				m3ua_user_index,
				(int) pSUDT_Unidata_fixpart->Message_Type,
				(int) pSUDT_Unidata_fixpart->Protocol_Class,
				(int) pSUDT_Unidata_fixpart->Offset_Called_Party_Address,
				(int) pSUDT_Unidata_fixpart->Offset_Calling_Party_Address,
				(int) pSUDT_Unidata_fixpart->Offset_Data);

		struct Ssccp_Mandatory_Part__fixpart *pFirst_mandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) (&(pSUDT_Unidata_fixpart->Offset_Called_Party_Address) + pSUDT_Unidata_fixpart->Offset_Called_Party_Address);
		msg_sccp_pworkingbuff = (unsigned char *) pFirst_mandatory_Part__fixpart;
		msg_sccp_pworkingbuff += sizeof(struct Ssccp_Mandatory_Part__fixpart);

		SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

		struct SCalled_Party_Address__fixpart *pCalled_Party_Address__fixpart  = (struct SCalled_Party_Address__fixpart *) msg_sccp_pworkingbuff;
		msg_sccp_pworkingbuff += sizeof(struct SCalled_Party_Address__fixpart);

		SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

		unsigned int  vCalled_Party_Address_SPC = 0;
		if (pCalled_Party_Address__fixpart->AI & 1/*SPC present*/) {vCalled_Party_Address_SPC = *(short int *)     (msg_sccp_pworkingbuff); msg_sccp_pworkingbuff += 2;}
		unsigned int  vCalled_Party_Address_SSN = 0;
		if (pCalled_Party_Address__fixpart->AI & 2/*SSN present*/) {vCalled_Party_Address_SSN = *(unsigned char *) (msg_sccp_pworkingbuff); msg_sccp_pworkingbuff += 1;}

		FAST5ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): Called Party Address{AI=0x%x, SPC=%d, SSN=%d}\n",
				m3ua_user_index,
				pCalled_Party_Address__fixpart->AI,
				vCalled_Party_Address_SPC,
				vCalled_Party_Address_SSN);

		struct Ssccp_Mandatory_Part__fixpart *pSecond_mandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) (&(pSUDT_Unidata_fixpart->Offset_Calling_Party_Address) + pSUDT_Unidata_fixpart->Offset_Calling_Party_Address);
		msg_sccp_pworkingbuff = (unsigned char *) pSecond_mandatory_Part__fixpart;
		msg_sccp_pworkingbuff += sizeof(struct Ssccp_Mandatory_Part__fixpart);

		SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

		struct SCalling_Party_Address__fixpart *pCalling_Party_Address__fixpart  = (struct SCalling_Party_Address__fixpart *) msg_sccp_pworkingbuff;
		msg_sccp_pworkingbuff += sizeof(struct SCalling_Party_Address__fixpart);

		SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

		unsigned int  vCalling_Party_Address_SPC = 0;
		if (pCalling_Party_Address__fixpart->AI & 1/*SPC present*/) {vCalling_Party_Address_SPC = *(short int *)     (msg_sccp_pworkingbuff); msg_sccp_pworkingbuff += 2;}
		unsigned int  vCalling_Party_Address_SSN = 0;
		if (pCalling_Party_Address__fixpart->AI & 2/*SSN present*/) {vCalling_Party_Address_SSN = *(unsigned char *) (msg_sccp_pworkingbuff); msg_sccp_pworkingbuff += 1;}

		FAST5ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): Calling Party Address{AI=0x%x, SPC=%d, SSN=%d}\n",
								m3ua_user_index,
								pCalling_Party_Address__fixpart->AI,
								vCalling_Party_Address_SPC,
								vCalling_Party_Address_SSN);

		if (vCalled_Party_Address_SSN == 1) /*SCCP Management*/ {

			struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart2 = (struct Ssccp_Mandatory_Part__fixpart *)
																			  (&(pSUDT_Unidata_fixpart->Offset_Data) + pSUDT_Unidata_fixpart->Offset_Data);
			msg_sccp_pworkingbuff = (unsigned char *) pMandatory_Part__fixpart2;
			msg_sccp_pworkingbuff += sizeof (struct Ssccp_Mandatory_Part__fixpart);

			SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

			struct SData_SCCP_Management_Message *pSData_SCCP_Management_Message = (struct SData_SCCP_Management_Message *) msg_sccp_pworkingbuff;

			FAST2ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): SCCP Management.\n", m3ua_user_index);

			switch ((enum Esccp_Management_Message_type_code) pSData_SCCP_Management_Message->Message_Type) {

				RECV_MSG_TRACE_case(case_string, Subsystem_Status_Test_SST);
				{
					struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;

					switch(pSData_SCCP_Management_Message->Affected_SSN) {
					case 254:
						if (pm3ua_user->sccp_user_ssn254_bssap.allowed != 1) {
							pm3ua_user->sccp_user_ssn254_bssap.allowed = 1;
							pm3ua_user->sccp_user_ssn254_bssap.state   = BSC_Reset_allowed;
							pm3ua_user->sccp_user_ssn254_bssap.own_sp  = pm3ua_user->Protocol_Data_Configured.vOPC;
							pm3ua_user->sccp_user_ssn254_bssap.peer_sp = pm3ua_user->Protocol_Data_Configured.vDPC;
						}
						// SST can arrive at any time, reply with SSA
						if (pm3ua_user->sccp_user_ssn254_bssap.allowed)
							OSI_SCTP_M3UA_SCCP__Subsystem_Allowed_SSA(m3ua_user_index);
					break;
					}
				}
				break;
				default:
					ErrorTraceHandle(0, "switch_up_sccp_parse_message(m3ua_user_index=%d): SCCP Management unsupported message %d\n",
											m3ua_user_index,
											(enum Esccp_Management_Message_type_code) pSData_SCCP_Management_Message->Message_Type);
				break;

			}

		} else if (vCalled_Party_Address_SSN == 4)   /*BSS OMAP*/ {
			ErrorTraceHandle(0, "switch_up_sccp_parse_message(m3ua_user_index=%d) BSS OMAP messages currently not supported.\n",
							  m3ua_user_index);

		} else if (vCalled_Party_Address_SSN == 254) /*BSS AP*/ {
			void *msg_bssap_pstartbuff;

			msg_bssap_pstartbuff = (&(pSUDT_Unidata_fixpart->Offset_Data) + pSUDT_Unidata_fixpart->Offset_Data);
			msg_bssap_pstartbuff += sizeof(struct Ssccp_Mandatory_Part__fixpart); // Length_Indicator

			SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_sccp_pworkingbuff, msg_m3ua_pendbuff);

			bssap_parse(m3ua_user_index,
					   (enum ENETCS_ParsedProtocols) bssap,
					   0xFFFFFFFF,
					   msg_m3ua_pstartbuff,
					   msg_sccp_pstartbuff,
					   msg_bssap_pstartbuff,
					   msg_m3ua_pendbuff);
		}

	}
	break;

	RECV_MSG_TRACE_case(case_string, DT1_Data_Form_1);
	{
		unsigned char *msg_sccp_pworkingbuff = msg_sccp_pstartbuff;
		struct SDT1_Data_Form_1__fixpart *pDT1_Data_Form_1__fixpart = (struct SDT1_Data_Form_1__fixpart *) msg_sccp_pworkingbuff;
		unsigned int own_dlg_ref = DIALOG_EXTRACT(pDT1_Data_Form_1__fixpart->Destination_Local_Reference);

		FAST6ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): \nMessage_Type  = 0x%x\nDestination_Local_Reference(my own)  = 0x%x\nSegmenting_Ressasembling  = 0x%x\nOffset_Data  = 0x%x\n",
				m3ua_user_index,
				pDT1_Data_Form_1__fixpart->Message_Type,
				own_dlg_ref,
				pDT1_Data_Form_1__fixpart->Segmenting_Ressasembling,
				pDT1_Data_Form_1__fixpart->Offset_Data);

		SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, own_dlg_ref); // there is certain amount of faulty own references received from peer

		switch((enum Esccp_common_connection_ref_fifo_user_type) sccp_common_connection_ref.fifo[own_dlg_ref].user_type) {
		case BSS_lu:
		{
			void *msg_bssap_pstartbuff;

			msg_bssap_pstartbuff = (&(pDT1_Data_Form_1__fixpart->Offset_Data) + pDT1_Data_Form_1__fixpart->Offset_Data);
			msg_bssap_pstartbuff += sizeof(struct Ssccp_Mandatory_Part__fixpart); // Length_Indicator

			SAFE_MEMAREA_CHECK(SCCP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_bssap_pstartbuff, msg_m3ua_pendbuff);

			bssap_parse(m3ua_user_index,
					   (enum ENETCS_ParsedProtocols) bssap,
					   own_dlg_ref,
					   msg_m3ua_pstartbuff,
					   msg_sccp_pstartbuff,
					   msg_bssap_pstartbuff,
					   msg_m3ua_pendbuff);
		}
		break;
		}

	}
	break;

	RECV_MSG_TRACE_case(case_string, CC_Connection_Confirm);
	{
		unsigned char *msg_sccp_pworkingbuff = msg_sccp_pstartbuff;
		struct SCC_Connection_Confirm *pmsg_CC_Connection_Confirm = (struct SCC_Connection_Confirm *) msg_sccp_pworkingbuff;
		unsigned int own_dlg_ref;
		unsigned int dest_dlg_ref;

		own_dlg_ref  = DIALOG_EXTRACT(pmsg_CC_Connection_Confirm->Destination_Local_Reference);
		dest_dlg_ref = DIALOG_EXTRACT(pmsg_CC_Connection_Confirm->Source_Local_Reference);

		FAST7ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): \nMessage_Type  = 0x%x\nDestination_Local_Reference  = 0x%x\nSource_Local_Reference = 0x%x\nProtocol_Class  = 0x%x\nOffset_Optional_Part = 0x%x\n",
				m3ua_user_index,
				pmsg_CC_Connection_Confirm->Message_Type,
				own_dlg_ref,
				dest_dlg_ref,
				pmsg_CC_Connection_Confirm->Protocol_Class,
				pmsg_CC_Connection_Confirm->Offset_Optional_Part);

		SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, own_dlg_ref);
		DIALOG_PROGRESS(sccp, netcs_index, own_dlg_ref, m3ua_user_index, CC_Connection_Confirm);
		// store destination dialog reference
		memcpy(sccp_common_connection_ref.fifo[own_dlg_ref].dest_dlg_ref, &pmsg_CC_Connection_Confirm->Source_Local_Reference, 3);
	}
	break;

	RECV_MSG_TRACE_case(case_string, RLSD_Released);
	{
		unsigned char *msg_sccp_pworkingbuff = msg_sccp_pstartbuff;
		struct SRLSD_Released *pmsg_RLSD_Released = (struct SRLSD_Released *) msg_sccp_pworkingbuff;
		unsigned int own_dlg_ref = DIALOG_EXTRACT(pmsg_RLSD_Released->Destination_Local_Reference);

		FAST6ErrorTraceHandle(2, "switch_up_sccp_parse_message(m3ua_user_index=%d): \nMessage_Type  = 0x%x\nSource_Local_Reference = 0x%x\nRelease_Cause  = 0x%x\nOffset_Optional_Part = 0x%x\n",
				m3ua_user_index,
				pmsg_RLSD_Released->Message_Type,
				own_dlg_ref,
				pmsg_RLSD_Released->Release_Cause,
				pmsg_RLSD_Released->Offset_Optional_Part);

		SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, own_dlg_ref);
		DIALOG_PROGRESS(sccp, netcs_index, own_dlg_ref, m3ua_user_index, RLSD_Released);
	}
	break;


	}

	return RECV_MSG_TRACE_parsed;
}


void SCCP_param__Called_Party_Address(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;

	int AI=0, CPN_size;

	if 		(par_SSN == 254) { AI=0x43; CPN_size=4; } /* BSSAP */
	else if (par_SSN == 1)   { AI=0x42; CPN_size=2; } /* SCCPMG */

	struct SCalled_Party_Address__fixpart *pmsg_Called_Party_Address__fixpart = (struct SCalled_Party_Address__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_Called_Party_Address__fixpart->AI  			= AI;
	pmsg_Called_Party_Address__fixpart += sizeof(struct SCalled_Party_Address__fixpart);
	*osi_offset += sizeof(struct SCalled_Party_Address__fixpart);

	if (AI & 0x1) { // SPC
		*(short int *)pmsg_Called_Party_Address__fixpart = (short int) pm3ua_user->sccp_user_ssn254_bssap.peer_sp;
		pmsg_Called_Party_Address__fixpart += sizeof(short int);
		*osi_offset += sizeof(short int);
	}
	if (AI & 0x2) { // SSN
		*(char *)pmsg_Called_Party_Address__fixpart = (char) par_SSN;
		*osi_offset += sizeof(char);
	}
}

void SCCP_param__Calling_Party_Address(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;

	int AI=0, CPN_size;

	if 		(par_SSN == 254) { AI=0x43; CPN_size=4; } /* BSSAP */
	else if (par_SSN == 1)   { AI=0x42; CPN_size=2; } /* SCCPMG */

	struct SCalling_Party_Address__fixpart *pmsg_Calling_Party_Address__fixpart = (struct SCalling_Party_Address__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_Calling_Party_Address__fixpart->AI   			= AI;
	pmsg_Calling_Party_Address__fixpart += sizeof(struct SCalling_Party_Address__fixpart);
	*osi_offset += sizeof(struct SCalling_Party_Address__fixpart);

	if (AI & 0x1) { // SPC
		*(short int *)pmsg_Calling_Party_Address__fixpart = pm3ua_user->sccp_user_ssn254_bssap.own_sp;
		pmsg_Calling_Party_Address__fixpart += sizeof(short int);
		*osi_offset += sizeof(short int);
	}
	if (AI & 0x2) { // SSN
		*(char *)pmsg_Calling_Party_Address__fixpart = (char) par_SSN; /*BSSAP*/
		*osi_offset += sizeof(char);
	}
}


void SCCP__CR_Connection_Request(
		unsigned int 	m3ua_user_index,
		unsigned int 	dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int AI=0, CPN_size;
	FAST1ErrorTraceHandle(2, "SCCP__CR_Connection_Request()\n");

	if 		(par_SSN == 254) { AI=0x43; CPN_size=4; } /* BSSAP */
	else if (par_SSN == 1)   { AI=0x42; CPN_size=2; } /* SCCPMG */

	struct SCR_Connection_Request *pmsg_Connection_Request = (struct SCR_Connection_Request *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_Connection_Request->Message_Type 					= (enum Esccp_fix_message_hdr_type_code) CR_Connection_Request;
	*(unsigned int *) (pmsg_Connection_Request->Source_Local_Reference) = dlg_ref;
	pmsg_Connection_Request->Protocol_Class 				= 0x2;
	pmsg_Connection_Request->Offset_Called_Party_Address   	= 2;
	pmsg_Connection_Request->Offset_Optional_Part  			= 2+CPN_size;
	*osi_offset += sizeof(struct SCR_Connection_Request);

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pMandatory_Part__fixpart->Length_Indicator 				= CPN_size;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);
	SCCP_param__Called_Party_Address (m3ua_user_index, buffer_write_counter, osi_offset, par_SSN);

	BSSAP_MAP__Complete_Layer3_Information(m3ua_user_index, dlg_ref, buffer_write_counter, osi_offset);

	struct Ssccp_Optional_Part__fixpart *pOptional_Part__fixpart = (struct Ssccp_Optional_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pOptional_Part__fixpart->Parameter_Name 				= 0x4;
	pOptional_Part__fixpart->Length_Indicator 				= 0x4;
	*osi_offset += sizeof(struct Ssccp_Optional_Part__fixpart);
	SCCP_param__Calling_Party_Address(m3ua_user_index, buffer_write_counter, osi_offset, par_SSN);

	SEND_MSG_TRACE("sccp", CR_Connection_Request);

}


void SCCP__UDT_Unidata(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset,
		unsigned int    par_SSN)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int AI=0, CPN_size;
	FAST1ErrorTraceHandle(2, "SCCP__UDT_Unidata()\n");

	if 		(par_SSN == 254) { AI=0x43; CPN_size=4; } /* BSSAP */
	else if (par_SSN == 1)   { AI=0x42; CPN_size=2; } /* SCCPMG */

	struct SUDT_Unidata__fixpart *pmsg_UDT_Unidata__fixpart     = (struct SUDT_Unidata__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_UDT_Unidata__fixpart->Message_Type 					= (enum Esccp_fix_message_hdr_type_code) UDT_Unidata;
	pmsg_UDT_Unidata__fixpart->Protocol_Class 					= 0x0;
	pmsg_UDT_Unidata__fixpart->Offset_Called_Party_Address   	= 3;
	pmsg_UDT_Unidata__fixpart->Offset_Calling_Party_Address  	= 3+CPN_size;
	pmsg_UDT_Unidata__fixpart->Offset_Data						= 3+2*CPN_size;
	*osi_offset += sizeof(struct SUDT_Unidata__fixpart);

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart1 = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pMandatory_Part__fixpart1->Length_Indicator 				= CPN_size;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);
	SCCP_param__Called_Party_Address(m3ua_user_index, buffer_write_counter, osi_offset, par_SSN);

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart2 = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pMandatory_Part__fixpart2->Length_Indicator 				= CPN_size;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);
	SCCP_param__Calling_Party_Address(m3ua_user_index, buffer_write_counter, osi_offset, par_SSN);

	SEND_MSG_TRACE("sccp", UDT_Unidata);
}

void SCCP__DT1_Data_Form_1(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	FAST1ErrorTraceHandle(2, "SCCP__DT1_Data_Form_1()\n");

	struct SDT1_Data_Form_1__fixpart *pmsg_DT1_Data_Form_1__fixpart = (struct SDT1_Data_Form_1__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_DT1_Data_Form_1__fixpart->Message_Type 					= (enum Esccp_fix_message_hdr_type_code) DT1_Data_Form_1;
	pmsg_DT1_Data_Form_1__fixpart->Segmenting_Ressasembling   		= 0;
	pmsg_DT1_Data_Form_1__fixpart->Offset_Data  					= 1;
	memcpy(&(pmsg_DT1_Data_Form_1__fixpart->Destination_Local_Reference), sccp_common_connection_ref.fifo[own_dlg_ref].dest_dlg_ref, 3);
	*osi_offset += sizeof(struct SDT1_Data_Form_1__fixpart);

	SEND_MSG_TRACE("sccp", UDT_Unidata);
}


void SCCP__RLC_Release_Complete(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	FAST1ErrorTraceHandle(2, "SCCP__RLC_Release_Complete()\n");
	unsigned int tmp_own_dlg_ref = own_dlg_ref;

	struct SRLC_Release_Complete *pmsg_RLC_Release_Complete = (struct SRLC_Release_Complete *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_RLC_Release_Complete->Message_Type 				= (enum Esccp_fix_message_hdr_type_code) RLC_Release_Complete_sccp;
	memcpy(&(pmsg_RLC_Release_Complete->Destination_Local_Reference), sccp_common_connection_ref.fifo[own_dlg_ref].dest_dlg_ref, 3);
	memcpy(&(pmsg_RLC_Release_Complete->Source_Local_Reference),      &tmp_own_dlg_ref, 3);
	*osi_offset += sizeof(struct SRLC_Release_Complete);

	SEND_MSG_TRACE("sccp", RLC_Release_Complete_sccp);
}




void SCCP__Subsystem_Allowed_SSA(
		unsigned int 	m3ua_user_index,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;

	FAST1ErrorTraceHandle(2, "SCCP__Subsystem_Allowed_SSA()\n");

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pMandatory_Part__fixpart->Length_Indicator 		 = 5;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SData_SCCP_Management_Message *pmsg_Data_SCCP_Management_Message = (struct SData_SCCP_Management_Message *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_Data_SCCP_Management_Message->Message_Type  = (enum Esccp_Management_Message_type_code) Subsystem_Allowed_SSA;
	pmsg_Data_SCCP_Management_Message->Affected_SSN  = 254;
	pmsg_Data_SCCP_Management_Message->Affected_SPC  = pm3ua_user->sccp_user_ssn254_bssap.own_sp;
	pmsg_Data_SCCP_Management_Message->SubSystem_Multiplicity_Indicator = 0;
	*osi_offset += sizeof(struct SData_SCCP_Management_Message);

	SEND_MSG_TRACE("sccp", Subsystem_Allowed_SSA);
}


void OSI_SCTP_M3UA_SCCP__Subsystem_Allowed_SSA(
		int m3ua_user_index)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int buffer_write_counter = 0xFFFF;
	unsigned int osi_offset = 0;

	FAST1ErrorTraceHandle(2, "OSI_SCTP_M3UA_SCCP__Subsystem_Allowed_SSA()\n");

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_counter);

	M3UA__Payload_Data_DATA		(m3ua_user_index, buffer_write_counter, &osi_offset);
	SCCP__UDT_Unidata			(m3ua_user_index, buffer_write_counter, &osi_offset, 1 /*SSN=SCCPMG*/);
	SCCP__Subsystem_Allowed_SSA	(m3ua_user_index, buffer_write_counter, &osi_offset);
	M3UA__Payload_Data_DATA		(m3ua_user_index, buffer_write_counter, &osi_offset);

	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_counter, osi_offset);
}

void OSI_SCTP_M3UA_SCCP(
		unsigned int 	m3ua_user_index,
		unsigned int 	own_dlg_ref,
		unsigned int 	sccp_message_id)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int buffer_write_counter = 0xFFFF;
	unsigned int osi_offset = 0;

	switch ((enum Esccp_fix_message_hdr_type_code) sccp_message_id) {
	case RLC_Release_Complete_sccp:
		FAST2ErrorTraceHandle(2, "OSI_SCTP_M3UA_SCCP(msg-%d)\n", sccp_message_id);

		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_counter);

		M3UA__Payload_Data_DATA			(m3ua_user_index, buffer_write_counter, &osi_offset);
		switch ((enum Esccp_fix_message_hdr_type_code) sccp_message_id) {
		case RLC_Release_Complete_sccp:
			SCCP__RLC_Release_Complete	(m3ua_user_index, own_dlg_ref, buffer_write_counter, &osi_offset);
		break;
		}
		M3UA__Payload_Data_DATA			(m3ua_user_index, buffer_write_counter, &osi_offset);

		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_counter, osi_offset);

	break;
	default:
		ErrorTraceHandle(0, "OSI_SCTP_M3UA_SCCP(msg-%d) not supported.\n", sccp_message_id);
	break;
	}

}

void sccp_fifo_RESETALL(void) {
	int cnt;
	memset(&sccp_common_connection_ref, 0, sizeof(struct Ssccp_common_connection_ref));
}
