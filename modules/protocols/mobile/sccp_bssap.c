
#define BSSAP
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include <modules/protocols/sigtran.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/mobile/sccp_bssap.h>
#include <modules/protocols/mobile/radio.h>
#include <modules/axe/application/bss/bss.h>
#undef BSSAP

bool bssap_parse(
		unsigned int m3ua_user_id,
		int parse_select,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff)
{
	PARSEPREAMBLE("bssap_parse");

	FAST6ErrorTraceHandle(2, "bssap_parse(select=%d, m3ua_user_id=%d, dlg_ref=0x%x) UP:   parse from 0x%x-0x%x\n",
						parse_select,
						m3ua_user_id,
						dlg_ref,
						msg_m3ua_pstartbuff,
						msg_m3ua_pendbuff);

	switch ((enum ENETCS_ParsedProtocols) parse_select) {

	// ******************************************************************
	// Action on messages level (parse_select)
	// ******************************************************************

	ECASE_TRACE(case_string, bssap);
	{
		unsigned char *msg_bssap_workingbuff = (unsigned char *) msg_bssap_pstartbuff;
		char *print_msg_bssap_workingbuff;
		int cnt, bss_map0_dtap1, bss_length = 0;
		bool parsed;

		bss_map0_dtap1 = *(unsigned char *) msg_bssap_workingbuff++;
		msg_bssap_workingbuff += bss_map0_dtap1; // skip DLCI in case of Dtap

		SAFE_MEMAREA_CHECK(BSSAP_upstream_parsing_event, m3ua_user_id, msg_m3ua_pstartbuff, msg_bssap_workingbuff, msg_m3ua_pendbuff);

		bss_length = *(unsigned char *) msg_bssap_workingbuff++;
		FAST4ErrorTraceHandle(2,	"bssap_parse(m3ua_user_id=%d): Message_Type=%s\nLength=%d\n",
							m3ua_user_id,
							(bss_map0_dtap1) ? "DTAP" : "BSSMAP",
							bss_length);

#ifndef FAST
		// print rest of message in HEX
		print_msg_bssap_workingbuff = (unsigned char *) msg_bssap_workingbuff;
		for (cnt = 0; cnt < bss_length; cnt++)
			ErrorTraceHandle(2, "0x%x ",(*(char *) (print_msg_bssap_workingbuff++)) & 0xff);
		ErrorTraceHandle(2, "\n");
		sprintf(case_string, "bssap_parse(m3ua_user_id=%d): %s", m3ua_user_id, "Action on msg class = ");
#endif

		// ******************************************************************
		// Action on messages level, discrimination on message class
		// ******************************************************************

		switch (bss_map0_dtap1) {
		case 0:
			parsed = switch_up_bssMap_parse_message(
					m3ua_user_id,
					dlg_ref,
					msg_m3ua_pstartbuff,
					msg_sccp_pstartbuff,
					msg_bssap_workingbuff,
					msg_m3ua_pendbuff);
			break;
		case 1:
			sprintf(case_string, "%s", "Dtap protocol >> ");

			switch ((enum Ebss_Radio_param_ProtocolDiscriminator) *(unsigned char *) msg_bssap_workingbuff++) {
			ECASE_TRACE(case_string, Call_Control_call_Related_SS_messages);
			break;
			ECASE_TRACE(case_string, MobilityManagement_messages_nonGPRS_service);
			break;
			ECASE_TRACE(case_string, RadioResource_management_messages);
			break;
			ECASE_TRACE(case_string, MobilityManagement_messages_GPRS_service);
			break;
			ECASE_TRACE(case_string, SessionManagement_messages);
			break;
		default:
			ErrorTraceHandle(0, "bssap_parse(m3ua_user_id=%d) Dtap protocol(%d) not supported !\n",
					m3ua_user_id, *(unsigned char *) msg_bssap_workingbuff);
			break;
			}
			parsed = switch_up_bssDtap_parse_message(
					m3ua_user_id,
					dlg_ref,
					msg_m3ua_pstartbuff,
					msg_sccp_pstartbuff,
					msg_bssap_workingbuff,
					msg_m3ua_pendbuff);
			break;
		}

		FAST4ErrorTraceHandle(2, "bssap_parse(m3ua_user_id=%d): DOWN: parse from 0x%x-0x%x\n",
							m3ua_user_id,
							msg_bssap_pstartbuff,
							msg_m3ua_pendbuff);

		// ******************************************************************
		// ReAction on messages level (parse_select)
		// ******************************************************************

		sprintf(case_string, "%s", "ReAction on msg class =");

		switch (bss_map0_dtap1) {
		case 0:
			parsed = switch_down_bssMap_parse_message(
					m3ua_user_id,
					dlg_ref,
					msg_m3ua_pstartbuff,
					msg_sccp_pstartbuff,
					msg_bssap_workingbuff,
					msg_m3ua_pendbuff);
			break;
		case 1:
			parsed = switch_down_bssDtap_parse_message(
					m3ua_user_id,
					dlg_ref,
					msg_m3ua_pstartbuff,
					msg_sccp_pstartbuff,
					msg_bssap_workingbuff,
					msg_m3ua_pendbuff);
			break;
		}
	}
	break; // bssMap_HEADER


	// ******************************************************************
	// parameter level (parse_select) .. no parameter level since parameters are not standalone subjects with parameter tag and parameter length
	// ******************************************************************

	}

	return ECASE_TRACE_parsed;

}

bool switch_up_bssMap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;
	PARSEPREAMBLE("bssMap");

	switch ((enum EbssMap_param_Message_Type) (*(unsigned char *) msg_bssap_pstartbuff)) {
	RECV_MSG_TRACE_case(case_string, Reset);				break;
	RECV_MSG_TRACE_case(case_string, Reset_Ack);
	{
		if (!pm3ua_user->sccp_user_ssn254_bssap.allowed)
			ErrorTraceHandle(0, "switch_up_bssap_parse_message(m3ua_user_id=%d) Reset_Ack received but sccp user not defined !\n",
							 m3ua_user_index);
		else
			pm3ua_user->sccp_user_ssn254_bssap.state = BSC_Reset_message_received;
	}
	break;
	RECV_MSG_TRACE_case(case_string, Common_ID);			break;
	RECV_MSG_TRACE_case(case_string, Cipher_Mode_Command);
		SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, dlg_ref);
		DIALOG_PROGRESS(bssMap, netcs_index, dlg_ref, m3ua_user_index, Cipher_Mode_Command);
	break;
	RECV_MSG_TRACE_case(case_string, Clear_Command);
		SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, dlg_ref);
		DIALOG_PROGRESS(bssMap, netcs_index, dlg_ref, m3ua_user_index, Clear_Command);
	break;

	}

	return RECV_MSG_TRACE_parsed;
}

bool switch_up_bssDtap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sm3ua_user *pm3ua_user = (struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation;
	PARSEPREAMBLE("Dtap");

	SAFE_SCCP_DIALOG_REF(m3ua_user_index, false, dlg_ref);

	switch ((*(unsigned char *) msg_bssap_pstartbuff)) {
	RECV_MSG_TRACE_case(case_string, AUTHENTICATION_REQUEST);
		DIALOG_PROGRESS(bssDtapMM, netcs_index, dlg_ref, m3ua_user_index, AUTHENTICATION_REQUEST);
	break;
	RECV_MSG_TRACE_case(case_string, AUTHENTICATION_REJECT);
		DIALOG_PROGRESS(bssDtapMM, netcs_index, dlg_ref, m3ua_user_index, AUTHENTICATION_REJECT);
	break;
	RECV_MSG_TRACE_case(case_string, TMSI_REALLOCATION_COMMAND);
		DIALOG_PROGRESS(bssDtapMM, netcs_index, dlg_ref, m3ua_user_index, TMSI_REALLOCATION_COMMAND);
	break;
	RECV_MSG_TRACE_case(case_string, LOCATION_UPDATING_ACCEPT);
		SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[netcs_index].stats.c[switch_up_bssDtap_parse_message__LOCATION_UPDATING_ACCEPT], 1);
		DIALOG_PROGRESS(bssDtapMM, netcs_index, dlg_ref, m3ua_user_index, LOCATION_UPDATING_ACCEPT);
	break;
	RECV_MSG_TRACE_case(case_string, LOCATION_UPDATING_REJECT);
		SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[netcs_index].stats.c[switch_up_bssDtap_parse_message__LOCATION_UPDATING_REJECT], 1);
		DIALOG_PROGRESS(bssDtapMM, netcs_index, dlg_ref, m3ua_user_index, LOCATION_UPDATING_REJECT);
	break;
	}

	return RECV_MSG_TRACE_parsed;
}

bool switch_down_bssMap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	PARSEPREAMBLE("bssMap");

	switch ((enum EbssMap_param_Message_Type) (*(unsigned char *) msg_bssap_pstartbuff)) {
	RECV_MSG_TRACE_case(case_string, Reset);	break;
	}

	return ECASE_TRACE_parsed;
}

bool switch_down_bssDtap_parse_message(
		unsigned int m3ua_user_index,
		unsigned int dlg_ref,
		void *msg_m3ua_pstartbuff,
		void *msg_sccp_pstartbuff,
		void *msg_bssap_pstartbuff,
		void *msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	PARSEPREAMBLE("Dtap");

	switch ((enum EbssMap_param_Message_Type) (*(unsigned char *) msg_bssap_pstartbuff)) {
	}

	return ECASE_TRACE_parsed;
}

void BSSAP_MAP__Reset(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset) {
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;

	FAST1ErrorTraceHandle(2, "BSSAP_MAP__Reset()\n");

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pMandatory_Part__fixpart->Length_Indicator = 6;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SbssMap_fix_Message_Type *pmsg_SbssMap_fix_Message_Type = (struct SbssMap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_SbssMap_fix_Message_Type->Message_Type = 0;
	pmsg_SbssMap_fix_Message_Type->Length = 4;
	*osi_offset += sizeof(struct SbssMap_fix_Message_Type);

	struct SbssMap_message_Reset *pmsg_SbssMap_message_Reset = (struct SbssMap_message_Reset *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_SbssMap_message_Reset->Message_Type			= (enum EbssMap_param_Message_Type) Reset;
	pmsg_SbssMap_message_Reset->Cause.Element_Identifier= (enum EbssMap_param_Element_Identifier) Cause;
	pmsg_SbssMap_message_Reset->Cause.Length = 1;
	pmsg_SbssMap_message_Reset->Cause.Cause_Value = 0x20; /* Equipment failure */
	*osi_offset += sizeof(struct SbssMap_message_Reset);

	SEND_MSG_TRACE("bssMap", Reset);
}

void BSSAP_MAP__Complete_Layer3_Information(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int mml_caller_btsi_index;

	FAST1ErrorTraceHandle(2, "BSSAP_MAP__Complete_Layer3_Information()\n");

	struct Ssccp_Optional_Part__fixpart *pOptional_Part__fixpart = (struct Ssccp_Optional_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pOptional_Part__fixpart->Parameter_Name
			= (enum Esccp_parameter_name_code) Data;
	pOptional_Part__fixpart->Length_Indicator 	= 0x21;
	*osi_offset += sizeof(struct Ssccp_Optional_Part__fixpart);

	struct SbssMap_fix_Message_Type *pmsg_SbssMap_fix_Message_Type = (struct SbssMap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_SbssMap_fix_Message_Type->Message_Type = 0;
	pmsg_SbssMap_fix_Message_Type->Length 		= 31;
	*osi_offset += sizeof(struct SbssMap_fix_Message_Type);

	struct SbssMap_message_Complete_Layer3_Information__fixpart	*pmsg_Complete_Layer3_Information__fix = (struct SbssMap_message_Complete_Layer3_Information__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_Complete_Layer3_Information__fix->Message_Type	= (enum EbssMap_param_Message_Type) Complete_Layer3_Information;
	*osi_offset	+= sizeof(struct SbssMap_message_Complete_Layer3_Information__fixpart);

	// ************* Cell Identifier parameter *************
	struct SbssMap_param_Cell_Identifier__fixpart *pparam_Cell_Identifier__fixpart = (struct SbssMap_param_Cell_Identifier__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pparam_Cell_Identifier__fixpart->Element_Identifier = (enum EbssMap_param_Element_Identifier) Cell_Identifier;
	pparam_Cell_Identifier__fixpart->Length 			= 8; /* whole CGI means 4 octets of param Cell_Identification*/
	pparam_Cell_Identifier__fixpart->HALF_Spare_HALF_Cell_Identification_Discriminator = 0; /* 0000xxxx spare xxxx0000 whole CGI */
	*osi_offset += sizeof(struct SbssMap_param_Cell_Identifier__fixpart);

	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	// Cell Identification coding for GSM900 and DCS1800
	// octet 4	MCC dig 2	|	MCC digit 1
	// octet 5	 1 1 1 1	|	MCC digit 3
	// octet 6	MNC dig 2	|	MNC digit 1
	// octet 7             LAC
	// octet 8             LAC cont
	// octet 9             CI
	// octet 10            CI cont

	unsigned int mml_call_index = sccp_common_connection_ref.fifo[dlg_ref].mml_call_index;
	unsigned int cgi_mcc = (short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_mcc);
	*(unsigned char *) (pparam++) = (((cgi_mcc % 100) / 10) << 4) | (cgi_mcc % 10);
	*(unsigned char *) (pparam++) = 0xf0 | (cgi_mcc / 100);

	unsigned int cgi_mnc = (short int) atoi(mml.Cbtsi.par[mml_caller_btsi_index].Pcgi_mnc);
	*(unsigned char *) (pparam++) = ((cgi_mnc / 10) << 4)/*dig2*/| (cgi_mnc % 10)/*dig1*/;

	*(short int *) pparam = LITTLETOBIG((short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_lac), 2);
	pparam += 2;
	*(short int *) pparam = LITTLETOBIG((short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_ci), 2);
	*osi_offset += (1 + 1 + 1 + 2 + 2);

	// ************* Layer 3 Information parameter *************
	struct SbssMap_param_Layer_3_Information *pparam_Layer3_Information__variablepart = (struct SbssMap_param_Layer_3_Information *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pparam_Layer3_Information__variablepart->Element_Identifier = (enum EbssMap_param_Element_Identifier) Layer_3_Information;
	pparam_Layer3_Information__variablepart->Length = 18;
	*osi_offset += sizeof(struct SbssMap_param_Layer_3_Information);

	SEND_MSG_TRACE("bssMap", Complete_Layer3_Information);

	RADIO_MM__LOCATION_UPDATING_REQUEST(m3ua_user_index, dlg_ref, buffer_write_index, osi_offset);
}

void BSSAP_MAP__Cipher_Mode_Complete(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;

	FAST1ErrorTraceHandle(2, "BSSAP_MAP__Cipher_Mode_Complete()\n");

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pMandatory_Part__fixpart->Length_Indicator = 5;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SbssMap_fix_Message_Type *pmsg_SbssMap_fix_Message_Type = (struct SbssMap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_SbssMap_fix_Message_Type->Message_Type = 0;
	pmsg_SbssMap_fix_Message_Type->Length = 3;
	*osi_offset += sizeof(struct SbssMap_fix_Message_Type);

	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	*(unsigned char *) (pparam++) = (enum EbssMap_param_Message_Type) Cipher_Mode_Complete;
	//  Add optional parameter: Choosen Encryption algorithm
	*(unsigned char *) (pparam++) = (enum EbssMap_param_Element_Identifier) Chosen_Encryption_Algorithm;
	*(unsigned char *) pparam = 01; // no encryption used
	*osi_offset += 3;

	SEND_MSG_TRACE("bssMap", Cipher_Mode_Complete);
}

void BSSAP_MAP__Clear_Complete(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int buffer_write_index,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;

	FAST1ErrorTraceHandle(2, "BSSAP_MAP__Clear_Complete()\n");

	struct Ssccp_Mandatory_Part__fixpart *pMandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pMandatory_Part__fixpart->Length_Indicator = 3;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SbssMap_fix_Message_Type *pmsg_SbssMap_fix_Message_Type = (struct SbssMap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	pmsg_SbssMap_fix_Message_Type->Message_Type = 0;
	pmsg_SbssMap_fix_Message_Type->Length = 1;
	*osi_offset += sizeof(struct SbssMap_fix_Message_Type);

	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_index);
	*(unsigned char *) pparam = (enum EbssMap_param_Message_Type) Clear_Complete;
	*osi_offset += 1;

	SEND_MSG_TRACE("bssMap", Clear_Complete);
}

void OSI_SCTP_M3UA_SCCP_BSSAP_MAP(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int bssap_message_id)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int buffer_write_index = 0xFFFF;
	unsigned int osi_offset = 0;

	switch ((enum EbssMap_param_Message_Type) bssap_message_id) {
	case Reset:
	case Complete_Layer3_Information:
	case Cipher_Mode_Complete:
	case Clear_Complete:
		FAST3ErrorTraceHandle(2, "OSI_SCTP_M3UA_SCCP_BSSAP_MAP(msg-%d) dlg_ref=%d\n", bssap_message_id, dlg_ref);

		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);

		M3UA__Payload_Data_DATA(m3ua_user_index, buffer_write_index, &osi_offset);

		switch ((enum EbssMap_param_Message_Type) bssap_message_id) {
		case Reset:
			SCCP__UDT_Unidata				(m3ua_user_index, buffer_write_index, &osi_offset, 254 /*SSN=BSSAP*/);
			BSSAP_MAP__Reset				(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			break;
		case Complete_Layer3_Information:
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[netcs_index].stats.c[OSI_SCTP_M3UA_SCCP_BSSAP_MAP__CL3I], 1);
			SCCP__CR_Connection_Request		(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset, 254 /*SSN=BSSAP*/);
			break;
		case Cipher_Mode_Complete:
			SCCP__DT1_Data_Form_1			(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			BSSAP_MAP__Cipher_Mode_Complete	(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			break;
		case Clear_Complete:
			SCCP__DT1_Data_Form_1			(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			BSSAP_MAP__Clear_Complete		(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			break;
		}
		M3UA__Payload_Data_DATA(m3ua_user_index, buffer_write_index, &osi_offset);

		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, osi_offset);

		break;
	default:
		ErrorTraceHandle(0, "OSI_SCTP_M3UA_SCCP_BSSAP_MAP(msg-%d) not supported.\n", bssap_message_id);
		break;
	}
}

void OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(
		int m3ua_user_index,
		unsigned int dlg_ref,
		int bssap_message_id)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int buffer_write_index = 0xFFFF;
	unsigned int osi_offset = 0;


	switch ((enum EbssMap_param_Message_Type) bssap_message_id) {
	case AUTHENTICATION_RESPONSE:
	case TMSI_REALLOCATION_COMPLETE:
		FAST3ErrorTraceHandle(2, "OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(msg-%d) dlg_ref=%d\n", bssap_message_id, dlg_ref);

		sccp_common_connection_ref.fifo[dlg_ref].nsd += 1;

		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);

		M3UA__Payload_Data_DATA					(m3ua_user_index, buffer_write_index, &osi_offset);
		SCCP__DT1_Data_Form_1					(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);

		switch ((enum EbssMap_param_Message_Type) bssap_message_id) {
		case AUTHENTICATION_RESPONSE:
			RADIO_MM__AUTHENTICATION_RESPONSE	(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			break;
		case TMSI_REALLOCATION_COMPLETE:
			RADIO_MM__TMSI_REALLOCATION_COMPLETE(m3ua_user_index, dlg_ref, buffer_write_index, &osi_offset);
			break;
		}
		M3UA__Payload_Data_DATA					(m3ua_user_index, buffer_write_index, &osi_offset);

		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, osi_offset);

		break;
	default:
		ErrorTraceHandle(0, "OSI_SCTP_M3UA_SCCP_BSSAP_DTAP(msg-%d) not supported.\n", bssap_message_id);
		break;
	}
}

