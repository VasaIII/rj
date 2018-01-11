#define RADIO
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include <modules/protocols/sigtran.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/mobile/sccp_bssap.h>
#include <modules/protocols/mobile/radio.h>
#include <modules/axe/application/bss/bss.h>
#undef RADIO

void parse_radio_message(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	(void) dlg_ref;

	FAST1ErrorTraceHandle(2, "RADIO_Message_Discrimination()\n");

	// Protocol_Discriminator and Skip_Indicator
	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	*osi_offset +=1;
	switch((enum Ebss_Radio_param_ProtocolDiscriminator)(*pparam&0xF)) {
	RADIO_TRACE(Call_Control_call_Related_SS_messages);			break;
	RADIO_TRACE(MobilityManagement_messages_nonGPRS_service);
	{
		switch((enum Ebss_Radio_message_MobilityManagement)(*pparam)) {

		}
	}
	break;
	RADIO_TRACE(RadioResource_management_messages);				break;
	RADIO_TRACE(MobilityManagement_messages_GPRS_service);		break;
	RADIO_TRACE(SessionManagement_messages);					break;
	default:
		ErrorTraceHandle(0, "RADIO_Message_Discrimination() Non supported radio messages (%d).\n", (*pparam&0xF));
	break;
	};

	SEND_MSG_TRACE("bssDtapRR", LOCATION_UPDATING_REQUEST);
}



void RADIO_MM__LOCATION_UPDATING_REQUEST(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	(void) dlg_ref;

	FAST1ErrorTraceHandle(2, "RADIO_MM__LOCATION_UPDATING_REQUEST()\n");

	// Protocol_Discriminator and Skip_Indicator
	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	*(pparam++) = 0xF & (enum Ebss_Radio_param_ProtocolDiscriminator) MobilityManagement_messages_nonGPRS_service;
	// Message Type
	*(pparam++) = (enum Ebss_Radio_message_MobilityManagement) LOCATION_UPDATING_REQUEST;
	// Ciphering_Key_Sequence (no key available - 0111) and Location_Updating_Type (normal - 0000)
	*(pparam++) = 0x70;
	*osi_offset += 3;

	// LAI
	unsigned int mml_call_index = sccp_common_connection_ref.fifo[dlg_ref].mml_call_index;
	unsigned int cgi_mcc = (short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_mcc);
	*(unsigned char *)(pparam ++) = (((cgi_mcc%100)/10)<<4)|(cgi_mcc%10);
	*(unsigned char *)(pparam ++) = 0xf0|(cgi_mcc/100);
	unsigned int cgi_mnc = (short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_mnc);
	*(unsigned char *)(pparam++) = ((cgi_mnc/10)<<4)/*dig2*/|(cgi_mnc%10)/*dig1*/;
	*(short int *)pparam = LITTLETOBIG((short int) atoi(bss_bsc_application[mml_call_index].caller.mml_btsi.Pcgi_lac), 2);
	pparam +=2;
	*osi_offset += (1+1+1+2);

	// Mobile Station Classmark
	*(unsigned char *)(pparam++) = 0x20;
	*osi_offset += 1;

	// Mobile Identity
	unsigned int cnt, imsi_length = sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.IMSI_length;

	*(unsigned char *)(pparam++) = 0x08; // Length
	*osi_offset += 1;
	*(unsigned char *)(pparam++) = (((sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.IMSI[0]-'0')<<4)&0xf0)/*first digit*/
								   | 0x09 /*Type*/;
	*osi_offset += 1;
	for (cnt=1; cnt<imsi_length-(imsi_length%2); cnt+=2) {
		*(unsigned char *)(pparam++) = (((sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.IMSI[cnt+1]-'0')<<4)&0xf0)|((sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.IMSI[cnt]-'0')&0xf);
	}
	if ((imsi_length-1)%2)
		*(unsigned char *)(pparam++) = (((sccp_common_connection_ref.fifo[dlg_ref].vBSS_caller_locupdate.IMSI[cnt]-'0')<<4)&0xf0);
	*osi_offset += (imsi_length-1)/2+(imsi_length%2);

	SEND_MSG_TRACE("bssDtapMM", LOCATION_UPDATING_REQUEST);
}


void RADIO_MM__AUTHENTICATION_RESPONSE(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	(void) dlg_ref;

	FAST1ErrorTraceHandle(2, "RADIO_MM__AUTHENTICATION_RESPONSE()\n");

	struct Ssccp_Mandatory_Part__fixpart *psccp_Mandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	psccp_Mandatory_Part__fixpart->Length_Indicator = 9;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SbssDtap_fix_Message_Type *pmsg_bssDtap_fix_Message_Type = (struct SbssDtap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_bssDtap_fix_Message_Type->Message_Type	= 1;
	pmsg_bssDtap_fix_Message_Type->DLCI			= 0;
	pmsg_bssDtap_fix_Message_Type->Length		= 6;
	*osi_offset += sizeof(struct SbssDtap_fix_Message_Type);

	// Protocol_Discriminator and Skip_Indicator
	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	*(pparam++) = 0xF & (enum Ebss_Radio_param_ProtocolDiscriminator) MobilityManagement_messages_nonGPRS_service;
	// Message Type
	*(pparam++) = ((enum Ebss_Radio_message_MobilityManagement) AUTHENTICATION_RESPONSE) | ((sccp_common_connection_ref.fifo[dlg_ref].nsd&1)<<6);
	// Authenticaton Parameter SRES (signature calculated in mobile station)
	*(unsigned int *) (pparam) = LITTLETOBIG(0x12345678, 4);
	*osi_offset += 6;

	SEND_MSG_TRACE("bssDtapMM", AUTHENTICATION_RESPONSE);
}


void RADIO_MM__TMSI_REALLOCATION_COMPLETE(
		int m3ua_user_index,
		int dlg_ref,
		int buffer_write_counter,
		int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	(void) dlg_ref;

	FAST1ErrorTraceHandle(2, "RADIO_MM__TMSI_REALLOCATION_COMPLETE()\n");

	struct Ssccp_Mandatory_Part__fixpart *psccp_Mandatory_Part__fixpart = (struct Ssccp_Mandatory_Part__fixpart *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	psccp_Mandatory_Part__fixpart->Length_Indicator = 5;
	*osi_offset += sizeof(struct Ssccp_Mandatory_Part__fixpart);

	struct SbssDtap_fix_Message_Type *pmsg_bssDtap_fix_Message_Type = (struct SbssDtap_fix_Message_Type *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	pmsg_bssDtap_fix_Message_Type->Message_Type	= 1;
	pmsg_bssDtap_fix_Message_Type->DLCI			= 0;
	pmsg_bssDtap_fix_Message_Type->Length		= 2;
	*osi_offset += sizeof(struct SbssDtap_fix_Message_Type);

	// Protocol_Discriminator and Skip_Indicator
	unsigned char *pparam = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
	*(pparam++) = 0xF & (enum Ebss_Radio_param_ProtocolDiscriminator) MobilityManagement_messages_nonGPRS_service;
	// Message Type
	*(pparam++) = ((enum Ebss_Radio_message_MobilityManagement) TMSI_REALLOCATION_COMPLETE) | ((sccp_common_connection_ref.fifo[dlg_ref].nsd&1)<<6);
	// Authenticaton Parameter SRES (signature calculated in mobile station)
	*osi_offset += 2;

	SEND_MSG_TRACE("bssDtapMM", TMSI_REALLOCATION_COMPLETE);
}

