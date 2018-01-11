#define ISUP_C
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include "modules/protocols/sigtran.h"
#include "modules/axe/application/pstn/pstn.h"
#include "modules/protocols/trunk/isup.h"
#undef ISUP_C

unsigned int isup_parse(unsigned int  m3ua_user_index,
						int 		  parse_select,
						void   		 *msg_m3ua_pstartbuff,
						void   		 *msg_isup_pstartbuff,
						void   		 *msg_m3ua_pendbuff)
{
	PARSEPREAMBLE("isup_parse");

	FAST6ErrorTraceHandle(2, "isup_parse(select=%d, m3ua_user_index=%d) UP: parse M3UA<0x%x-ISUP<0x%x->-0x%x>\n",
						 parse_select,
						 m3ua_user_index,
						 msg_m3ua_pstartbuff,
						 msg_isup_pstartbuff,
						 msg_m3ua_pendbuff);

	SAFE_MEMAREA_CHECK(ISUP_upstream_parsing_event, m3ua_user_index, msg_m3ua_pstartbuff, msg_isup_pstartbuff, msg_m3ua_pendbuff);

	switch ((enum ENETCS_ParsedProtocols) parse_select) {

	// ******************************************************************
	// Action on messages level (parse_select)
	// ******************************************************************

	ECASE_TRACE(case_string, isup_HEADER);
	{
		bool parsed 			= false;
		struct Sisup_fix_message_hdr *pparse_fix_hdr = (struct Sisup_fix_message_hdr *) msg_isup_pstartbuff;

		FAST4ErrorTraceHandle(2, "isup_HEADER(m3ua_user_index=%d): CIC=%d, Message Type=0x%x\n",
								m3ua_user_index,
								(short int) pparse_fix_hdr->cic,
								(char)pparse_fix_hdr->message_type);

		// ******************************************************************
		// Action on messages level, discrimination on message class
		// ******************************************************************

#ifndef FAST
		case_string[0] = 0;
#endif
		msg_isup_pstartbuff += sizeof(struct Sisup_fix_message_hdr);
		parsed = switch_up_isup_parse_message(m3ua_user_index, pparse_fix_hdr, msg_m3ua_pstartbuff, msg_isup_pstartbuff, msg_m3ua_pendbuff);

		// ******************************************************************
		// ReAction on messages level (parse_select) ... not needed, it is replied on upstream application progress
		// ******************************************************************
	}
	break; // isup_HEADER

	// ******************************************************************
	// parameter level (parse_select) .. no parameter level since parameters are not standalone subjects with parameter tag and parameter length
	// ******************************************************************

	}

	return m3ua_user_index;
}


bool switch_up_isup_parse_message(unsigned int 	 m3ua_user_index,
								  struct 		Sisup_fix_message_hdr *parse_fix_hdr,
								  void   		*msg_m3ua_pstartbuff,
								  void   		*msg_isup_pstartbuff,
								  void   		*msg_m3ua_pendbuff)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	int buffer_write_counter = 0xFFFF;
	PARSEPREAMBLE("isup");

	FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message() CIC-%d\n", parse_fix_hdr->cic);

	switch ((enum Eisup_fix_message_hdr_message_type) parse_fix_hdr->message_type) {

	RECV_MSG_TRACE_case(case_string, RSC_Reset_Circuit);
	{
		int cnt = parse_fix_hdr->cic;
		switch(ISUP_CIC_DEVICESTATE_GET(cnt)) {
		case cic_unequipped:
		case cic_idle:
		case cic_auto_blocked:
		case cic_manual_blocked:
			isup_cic_RESET(m3ua_user_index, cnt);
			ISUP_CIC_DEVICESTATE_SET(cnt, cic_idle);
		break;
		case cic_busy:
			// initiate progress for release call
		break;
		default:
			FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message(RSC_Reset_Circuit) Unsupported device state (%d)\n", ISUP_CIC_DEVICESTATE_GET(cnt));
		break;
		}
	}
	break;

	RECV_MSG_TRACE_case(case_string, GRS_Circuit_Group_Reset);
	{
		unsigned char *pGRS = (unsigned char *) msg_isup_pstartbuff;
		unsigned int cnt, cic_range = *(unsigned char *)pGRS;

		FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message(GRS_Circuit_Group_Reset) Range = 0x%x\n", cic_range);
		for (cnt=parse_fix_hdr->cic; cnt<(parse_fix_hdr->cic+cic_range); cnt++) {
			switch(ISUP_CIC_DEVICESTATE_GET(cnt)) {
			case cic_unequipped:
			case cic_idle:
			case cic_auto_blocked:
			case cic_manual_blocked:
				isup_cic_RESET(m3ua_user_index, cnt);
				ISUP_CIC_DEVICESTATE_SET(cnt, cic_idle);
			break;
			case cic_busy:
				// initiate progress for release call
			break;
			default:
				FAST3ErrorTraceHandle(2, "switch_up_isup_parse_message(GRS_Circuit_Group_Reset) CIC-%d, Unsupported device state (%d)\n", cnt, ISUP_CIC_DEVICESTATE_GET(cnt));
			break;
			}
		}
	}
	break;

	RECV_MSG_TRACE_case(case_string, UCIC_Unequipped_CIC);
	{
		int cnt = parse_fix_hdr->cic;
		switch(ISUP_CIC_DEVICESTATE_GET(cnt)) {
		case cic_unequipped:
		case cic_idle:
		case cic_auto_blocked:
		case cic_manual_blocked:
			isup_cic_RESET(m3ua_user_index, cnt);
			ISUP_CIC_DEVICESTATE_SET(cnt, cic_unequipped);
		break;
		case cic_busy:
			// initiate progress for release call
		break;
		default:
			FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message(UCIC_Unequipped_CIC) Unsupported device state (%d)\n", ISUP_CIC_DEVICESTATE_GET(cnt));
		break;
		}
	}
	break;

	RECV_MSG_TRACE_case(case_string, CGB_Circuit_Group_Blocking);	break;
	RECV_MSG_TRACE_case(case_string, CGU_Circuit_Group_Unblocking);	break;
	RECV_MSG_TRACE_case(case_string, CFN_Confusion);				break;



	RECV_MSG_TRACE_case(case_string, IAM_Initial_Address_Message);
	{
		char 		 Anum_length;
		char 		 Anum_string[20];
		char 		 Bnum_length;
		char 		 Bnum_string[20];
		unsigned char *pIAM = (unsigned char *) msg_isup_pstartbuff;

		// *************** Mandatory Fixed Part ***************
		// -
		// *************** Optional Parameter Part ***************

		// COT - ima indikator da li treba cekat COT ili ne

		SAFE_ISUP_CIC_ID(m3ua_user_index, false, parse_fix_hdr->cic);
		CONFIGURE_CIC_ID(Terminating, parse_fix_hdr->cic, ISUP_call_state_terminating_establishing, CALL_FAIL, Anum_length, Anum_string, Bnum_length, Bnum_string);
		// analiza
		PSTN_PROGRESS_CALL(parse_fix_hdr->cic, m3ua_user_index, isup, IAM_Initial_Address_Message);
	}
	break;

	RECV_MSG_TRACE_case(case_string, ACM_Address_Complete);
	{
		unsigned char *pACM = (unsigned char *) msg_isup_pstartbuff;
		short int BCI;

		// *************** Mandatory Fixed Part ***************

		// *************** Mandatory Fixed Parameter content - Backward_Call_Indicators ***************
		BCI = *(short int *)pACM;
		FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message(ACM_Address_Complete) BCI-0x%x\n", BCI);
		pACM++;

		// *************** Optional Parameter Part ***************
		if (*(short int *)pACM != 0)
			FAST1ErrorTraceHandle(2, "switch_up_isup_parse_message(ACM_Address_Complete) Optional parameters still not supported.\n");

		SAFE_ISUP_CIC_ID(m3ua_user_index, false, parse_fix_hdr->cic);
		PSTN_PROGRESS_CALL(parse_fix_hdr->cic, m3ua_user_index, isup, ACM_Address_Complete);
	}
	break;

	RECV_MSG_TRACE_case(case_string, ANM_Answer);
	{
		unsigned char *pANM = (unsigned char *) msg_isup_pstartbuff;

		// *************** Mandatory Fixed Part ***************
		// -
		// *************** Optional Parameter Part ***************
		if (*(short int *)pANM != 0)
			FAST2ErrorTraceHandle(2, "switch_up_isup_parse_message(ANM_Answer) CIC-%d, Optional parameters still not supported.\n", parse_fix_hdr->cic);

		SAFE_ISUP_CIC_ID(m3ua_user_index, false, parse_fix_hdr->cic);
		PSTN_PROGRESS_CALL(parse_fix_hdr->cic, m3ua_user_index, isup, ANM_Answer);
	}
	break;

	RECV_MSG_TRACE_case(case_string, REL_Release);
	{
		unsigned char *pREL = (unsigned char *) msg_isup_pstartbuff;

		// *************** Mandatory Fixed Part ***************
		// -
		// *************** Optional Parameter Part ***************


		// pogledat location, na osnovu njega moze i u normalnon speechu doc cause sa nekin problemon

		SAFE_ISUP_CIC_ID(m3ua_user_index, false, parse_fix_hdr->cic);
		PSTN_PROGRESS_CALL(parse_fix_hdr->cic, m3ua_user_index, isup, REL_Release);
	}
	break;

	RECV_MSG_TRACE_case(case_string, RLC_Release_Complete_isup);
	{
		unsigned char *pRLC = (unsigned char *) msg_isup_pstartbuff;

		// *************** Mandatory Fixed Part ***************
		// -
		// *************** Optional Parameter Part ***************

		SAFE_ISUP_CIC_ID(m3ua_user_index, false, parse_fix_hdr->cic);
		PSTN_PROGRESS_CALL(parse_fix_hdr->cic, m3ua_user_index, isup, RLC_Release_Complete_isup);
	}
	break;


	default:
		FAST3ErrorTraceHandle(2, "switch_up_isup_parse_message() CIC-%d, Message (id=%d) still not supported.\n", parse_fix_hdr->cic, parse_fix_hdr->message_type);
	break;
	}

	return RECV_MSG_TRACE_parsed;
}

void ISUP__IAM_Initial_Address_Message(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	int cnt, Called_party_number_length, Calling_party_number_length;
	int anum_length = isup_cic_id->data[cic_id].A.length;
	int bnum_length = isup_cic_id->data[cic_id].B.length;

	FAST1ErrorTraceHandle(2, "ISUP__IAM_Initial_Address_Message()\n");

	Called_party_number_length  = 2 + bnum_length + (bnum_length%2)*2;
	Calling_party_number_length = 2 + anum_length + (anum_length%2)*2;

	// *************** Mandatory Fix Part ***************
	struct SIAM_Initial_Address_Message__fixpart *pmsg_IAM_Initial_Address_Message = (struct SIAM_Initial_Address_Message__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_IAM_Initial_Address_Message->cic 								= cic_id;
	pmsg_IAM_Initial_Address_Message->message_type 						= (enum Eisup_fix_message_hdr_message_type) IAM_Initial_Address_Message;
	pmsg_IAM_Initial_Address_Message->Nature_of_Connection_Indicators   = 0x0;
	pmsg_IAM_Initial_Address_Message->Forward_Call_Indicators  			= 0x2001;	// ISUP all the way / orig user ISDN
	pmsg_IAM_Initial_Address_Message->Calling_Partys_Category  			= 0x0a; 	// ordinary subscriber
	pmsg_IAM_Initial_Address_Message->Transmission_Medium_Requirement  	= 0x0;		// speech
	*osi_offset += sizeof(struct SIAM_Initial_Address_Message__fixpart);

	// *************** Mandatory Variable Part and Optional Part pointers ***************
	unsigned char *pMadatory_Variable_Part 		= (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pMadatory_Variable_Part) = 2 /* offset to Mandatory Variable part (just skip one pointer (start Optional Part parameters)) */;
	*osi_offset += 1;
	unsigned char *pOptional_Part     			= (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pOptional_Part)			= 1 												/* take next octet */
												  + sizeof(struct Sisup_Madatory_Variable_Part) 	/* 1 octet (length octet for Mandatory Variable Parameter) */
												  + Called_party_number_length;
	*osi_offset += 1;

	// *************** Mandatory Variable Part ***************
	// *************** Mandatory Variable Parameter content - Called_Party_Number ***************
	struct Sisup_Madatory_Variable_Part *pparam_Called_party_number_fixpart = (struct Sisup_Madatory_Variable_Part *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pparam_Called_party_number_fixpart->Length_Indicator = Called_party_number_length;
	*osi_offset += sizeof(struct Sisup_Madatory_Variable_Part);

	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pparam++) = 0x03 						/* bit 0..6 Nature Of Address Indicator	- national (significant) number (0x3) */ |
								   ((bnum_length%2)?0x00:0x10) 	/*        7 Odd/Even 					*/;
	*osi_offset += 1;
	*(unsigned char *)(pparam++) = 0x0			/* bit 0..3 spare bits 								*/ |
								  (0x1&0x7)<<4 	/* bit 4..6 Numbering Plan Indicator 				- ISDN (Telephony) numbering plan (0x1) */ |
								  (0x0&0x1)<<7  /* bit 7    INN Internal Network Number indicator	*/;
	*osi_offset += 1;
	for (cnt=1; cnt<bnum_length-(bnum_length%2); cnt+=2) {
		*(unsigned char *)(pparam++) = (((isup_cic_id->data[cic_id].B.num[cnt+1]-'0')<<4)&0xf0)|((isup_cic_id->data[cic_id].B.num[cnt]-'0')&0xf);
	}
	if (bnum_length%2)
		*(unsigned char *)(pparam++) = (((isup_cic_id->data[cic_id].B.num[cnt]-'0')<<4)&0x0f);
	*osi_offset += bnum_length + (bnum_length%2)*2;

	// *************** Optional Parameter Part ***************
	// *************** Optional Parameter Part - Calling_Party_Number ***************
	struct Sisup_Optional_Part *pparam_Calling_Party_Number__fixpart = (struct Sisup_Optional_Part *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pparam_Calling_Party_Number__fixpart->Parameter_Name 	= (enum Eisup_fix_message_hdr_parameter_type) Calling_Party_Number;
	pparam_Calling_Party_Number__fixpart->Length_Indicator 	= Calling_party_number_length;
	*osi_offset += sizeof(struct Sisup_Optional_Part);

	pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pparam++) = 0x03 						/* bit 0..6 Nature Of Address Indicator	- national (significant) number (0x3) */ |
								   ((anum_length%2)?0x00:0x10) 	/*        7 Odd/Even 					*/;
	*osi_offset += 1;
	*(unsigned char *)(pparam++) = (0x1&0x3)	/* bit 0..1 Screening indicator							- user provided, verified and passed (0x1) */ |
								   (0x0&0x3)<<2	/* bit 2..3 Address Presentation Restricted Indicator 	*/ |
								   (0x1&0x7)<<4 /* bit 4..6 Numbering Plan Indicator 					- ISDN (Telephony) numbering plan (0x1) */ |
								   (0x0&0x1)<<7 /* bit 7    Number Incomplete indicator 				*/;
	*osi_offset += 1;
	for (cnt=1; cnt<anum_length-(anum_length%2); cnt+=2) {
		*(unsigned char *)(pparam++) = (((isup_cic_id->data[cic_id].A.num[cnt+1]-'0')<<4)&0xf0)|((isup_cic_id->data[cic_id].A.num[cnt]-'0')&0xf);
	}
	if (anum_length%2)
		*(unsigned char *)(pparam++) = (((isup_cic_id->data[cic_id].A.num[cnt]-'0')<<4)&0x0f);
	*osi_offset += anum_length + (anum_length%2)*2;

	// ako stavin A number, moze me dodatno pitat INF/INR pa se moze A izostavit

	// *************** Optional Parameter Part - Propagation_Delay_Counter ***************
	struct Sisup_Optional_Part *pparam_Propagation_Delay_Counter__fixpart = (struct Sisup_Optional_Part *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pparam_Propagation_Delay_Counter__fixpart->Parameter_Name 	= (enum Eisup_fix_message_hdr_parameter_type) Propagation_Delay_Counter;
	pparam_Propagation_Delay_Counter__fixpart->Length_Indicator = 2;
	*osi_offset += sizeof(struct Sisup_Optional_Part);

	pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pparam++) = 0; 			/* time in ms */
	*osi_offset += 2;

	// *************** Optional Parameter Part - Parameter_Compatibility_Information ***************
	pparam_Propagation_Delay_Counter__fixpart = (struct Sisup_Optional_Part *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pparam_Propagation_Delay_Counter__fixpart->Parameter_Name 	= (enum Eisup_fix_message_hdr_parameter_type) Parameter_Compatibility_Information;
	pparam_Propagation_Delay_Counter__fixpart->Length_Indicator = 2;
	*osi_offset += sizeof(struct Sisup_Optional_Part);

	pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pparam++) = (enum Eisup_fix_message_hdr_parameter_type) Propagation_Delay_Counter; /* Parameter Name */
	*(unsigned char *)(pparam++) = 0xc0; /* Instruction Indicator */
	*osi_offset += 2;

	SEND_MSG_TRACE("isup", IAM_Initial_Address_Message);

}


void ISUP__ACM_Address_Complete(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	FAST1ErrorTraceHandle(2, "ISUP__ACM_Address_Complete()\n");

	unsigned char *pACM = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	((struct Sisup_fix_message_hdr *) pACM)->cic 		  = (short int) cic_id;
	((struct Sisup_fix_message_hdr *) pACM)->message_type = (enum Eisup_fix_message_hdr_message_type) ACM_Address_Complete;
	pACM 		+= sizeof(struct Sisup_fix_message_hdr);
	*osi_offset += sizeof(struct Sisup_fix_message_hdr);

	// *************** Mandatory Fix Part ***************
	// *************** Mandatory Fix Parameter content - Backward_Call_Indicators ***************
	*(short int *) pACM = 0x424;
	pACM 		+= 2;
	*osi_offset += 2;

	// *************** Optional Part pointer ***************
	*(unsigned char *)(pACM) = 0; // no optional parameters
	*osi_offset += 1;

	SEND_MSG_TRACE("isup", ACM_Address_Complete);
}


void ISUP__ANM_Answer(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	FAST1ErrorTraceHandle(2, "ISUP__ANM_Answer()\n");

	unsigned char *pANM = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	((struct Sisup_fix_message_hdr *) pANM)->cic 		  = (short int) cic_id;
	((struct Sisup_fix_message_hdr *) pANM)->message_type = (enum Eisup_fix_message_hdr_message_type) ANM_Answer;
	pANM 		+= sizeof(struct Sisup_fix_message_hdr);
	*osi_offset += sizeof(struct Sisup_fix_message_hdr);

	// *************** Optional Part pointer ***************
	*(unsigned char *)(pANM) = 0; // no optional parameters
	*osi_offset += 1;

	SEND_MSG_TRACE("isup", ANM_Answer);
}


void ISUP__REL_Release(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;

	FAST1ErrorTraceHandle(2, "ISUP__REL_Release()\n");

	unsigned char *pREL = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(short int *) pREL = (short int) cic_id;
	pREL +=2;
	*(unsigned char *) (pREL++) = (enum Eisup_fix_message_hdr_message_type) REL_Release;
	*osi_offset += 3;

	// *************** Mandatory Variable Part and Optional Part pointers ***************
	unsigned char *pMadatory_Variable_Part 		= (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pMadatory_Variable_Part) = 2 /* offset to Mandatory Variable part (just skip one pointer (start Optional Part parameters)) */;
	*osi_offset += 1;
	unsigned char *pisup_Optional_Part 			= (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(unsigned char *)(pisup_Optional_Part)		= 0; // no optinal params to send
	*osi_offset += 1;

	// *************** Mandatory Variable Part ***************
	// *************** Mandatory Variable Parameter content - Cause_Indicator ***************
	struct Sisup_Madatory_Variable_Part *pparam_Cause_Indicator_fixpart = (struct Sisup_Madatory_Variable_Part *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	pparam_Cause_Indicator_fixpart->Length_Indicator = 2;
	*osi_offset += sizeof(struct Sisup_Madatory_Variable_Part);

	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	*(short int *)(pparam++) = LITTLETOBIG((short int) 0x8090, 2);
	*osi_offset += 2;

	SEND_MSG_TRACE("isup", REL_Release);
}


void ISUP__RLC_Release_Complete(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	buffer_write_counter,
		unsigned int 	*osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	FAST1ErrorTraceHandle(2, "ISUP__RLC_Release_Complete()\n");

	unsigned char *pRLC = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(isup, netcs_index, *osi_offset, buffer_write_counter);
	((struct Sisup_fix_message_hdr *) pRLC)->cic 		  = (short int) cic_id;
	((struct Sisup_fix_message_hdr *) pRLC)->message_type = (enum Eisup_fix_message_hdr_message_type) RLC_Release_Complete_isup;
	pRLC 		+= sizeof(struct Sisup_fix_message_hdr);
	*osi_offset += sizeof(struct Sisup_fix_message_hdr);

	// *************** Optional Part pointer ***************
	*(unsigned char *)(pRLC) = 0; // no optional parameters
	*osi_offset += 1;

	SEND_MSG_TRACE("isup", RLC_Release_Complete_isup);
}



void OSI_SCTP_M3UA_ISUP(
		unsigned int 	m3ua_user_index,
		unsigned int 	cic_id,
		unsigned int 	isup_message_id)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;
	int buffer_write_counter = 0xFFFF;
	unsigned int osi_offset = 0;

	switch ((enum Eisup_fix_message_hdr_message_type) isup_message_id) {
	case IAM_Initial_Address_Message:
	case ACM_Address_Complete:
	case ANM_Answer:
	case REL_Release:
	case RLC_Release_Complete_isup:
		FAST2ErrorTraceHandle(2, "OSI_SCTP_M3UA_ISUP(msg-%d)\n", isup_message_id);

		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_counter);

		M3UA__Payload_Data_DATA					(m3ua_user_index, buffer_write_counter, &osi_offset);
		switch ((enum Eisup_fix_message_hdr_message_type) isup_message_id) {
		case IAM_Initial_Address_Message:
			ISUP__IAM_Initial_Address_Message	(m3ua_user_index, cic_id, buffer_write_counter, &osi_offset);
		break;
		case ACM_Address_Complete:
			ISUP__ACM_Address_Complete			(m3ua_user_index, cic_id, buffer_write_counter, &osi_offset);
		break;
		case ANM_Answer:
			ISUP__ANM_Answer					(m3ua_user_index, cic_id, buffer_write_counter, &osi_offset);
		break;
		case REL_Release:
			ISUP__REL_Release					(m3ua_user_index, cic_id, buffer_write_counter, &osi_offset);
		break;
		case RLC_Release_Complete_isup:
			ISUP__RLC_Release_Complete			(m3ua_user_index, cic_id, buffer_write_counter, &osi_offset);
		break;
		}
		M3UA__Payload_Data_DATA					(m3ua_user_index, buffer_write_counter, &osi_offset);

		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_counter, osi_offset);

	break;
	default:
		ErrorTraceHandle(0, "OSI_SCTP_M3UA_ISUP(msg-%d) not supported.\n", isup_message_id);
	break;
	}

}

// *********************** SCCP INITIALIZATION DATA ***********************

void isup_cic_RESET(unsigned int m3ua_user_index, unsigned int cic_id) {
	struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->pM3uaAdaptation;

	FAST8ErrorTraceHandle(2, "isup_cic_RESET(m3ua_user_index=%d, CIC=%d) &data[0]=0x%x, data[0]=0x%x, size_data[]=0x%x, &data[1]=0x%x, calc_&data[1]=0x%x\n",
						  m3ua_user_index,
						  cic_id,
						  &(isup_cic_id->data[0]),
						  isup_cic_id->data[0],
						  sizeof(struct Sisup_cic_id_data),
						  &(isup_cic_id->data[1]),
						  (isup_cic_id->data)+sizeof(struct Sisup_cic_id_data)*1);

	memset((void *)(isup_cic_id->data)+sizeof(struct Sisup_cic_id_data)*cic_id, 0, sizeof(struct Sisup_cic_id_data));
	isup_cic_id->data[cic_id].call_state = PSTN_MASK(ISUP_call_state_NoCall);
}


