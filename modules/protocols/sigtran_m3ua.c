#define SIGTRAN_M3UA
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include "modules/protocols/sigtran.h"
#include "modules/protocols/trunk/isup.h"
#undef SIGTRAN_M3UA


void M3UA__Payload_Data_DATA(
	unsigned int m3ua_user_index,
	unsigned int buffer_write_counter,
	unsigned int *osi_offset)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	struct SPayload_Data_DATA *pmsg_Payload_Data_DATA;

	FAST4ErrorTraceHandle(2, "M3UA__Payload_Data_DATA() m3ua_user_index=%d, buffer_write_counter=%d, *osi_offset=%d\n", m3ua_user_index, buffer_write_counter, *osi_offset);

	if (*osi_offset == 0) { // fill all but length
		/* create m3ua packet into transmit_user buffer (with switched received opc-dpc), and latter in sccp part add rest*/
		pmsg_Payload_Data_DATA = (struct SPayload_Data_DATA *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, *osi_offset, buffer_write_counter);
		pmsg_Payload_Data_DATA->vsigtran_fix_message_hdr.version = 0x1;
		pmsg_Payload_Data_DATA->vsigtran_fix_message_hdr.dummy_1 = 0x0;
		pmsg_Payload_Data_DATA->vsigtran_fix_message_hdr.class   = (enum Esigtran_fix_message_hdr_class)   Transfer_Messages;
		pmsg_Payload_Data_DATA->vsigtran_fix_message_hdr.type    = (enum Em3ua_fix_message_hdr_class_type) Payload_Data_DATA;
		pmsg_Payload_Data_DATA->vsigtran_fix_parameter_hdr.parameter_tag = LITTLETOBIG((enum Em3ua_fix_parameter_hdr_tag) Protocol_Data, 2);
		pmsg_Payload_Data_DATA->vProtocol_Data.vOPC = LITTLETOBIG(((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vOPC, 4);
		pmsg_Payload_Data_DATA->vProtocol_Data.vDPC = LITTLETOBIG(((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vDPC, 4);
		pmsg_Payload_Data_DATA->vProtocol_Data.vSI  = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vSI;
		pmsg_Payload_Data_DATA->vProtocol_Data.vNI  = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vNI;
		pmsg_Payload_Data_DATA->vProtocol_Data.vMP  = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vMP;
		pmsg_Payload_Data_DATA->vProtocol_Data.vSLS = ((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->Protocol_Data_Configured.vSLS;
		*osi_offset += sizeof(struct SPayload_Data_DATA);
	} else { // fill length and padding
		unsigned char padding_chars[4] = {0,0,0,0}, *ppadding_chars = padding_chars;
		unsigned int  padding;
		unsigned int  m3ua_Payload_Data_length  = *osi_offset, length_offset;
		unsigned int  m3ua_Protocol_Data_length = m3ua_Payload_Data_length - sizeof(struct Ssigtran_fix_message_hdr);
		// The total length of a parameter (including Tag, Parameter Length and
		// Value fields) must be a multiple of 4 bytes. If the length of the parameter is
		// not a multiple of 4 bytes, the sender pads the parameter at the end (i.e.,
		// after the Parameter Value field) with all zero bytes.
		padding = (4 - m3ua_Payload_Data_length % 4);
		padding = (padding == 4)? 0: padding;

		pmsg_Payload_Data_DATA = (struct SPayload_Data_DATA *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, 0 /*m3ua offset is 0*/, buffer_write_counter);
		pmsg_Payload_Data_DATA->vsigtran_fix_message_hdr.length   = LITTLETOBIG(m3ua_Payload_Data_length + padding, 4);
		pmsg_Payload_Data_DATA->vsigtran_fix_parameter_hdr.length = LITTLETOBIG(m3ua_Protocol_Data_length, 2);

		if (padding)
			*osi_offset += padding;
	}

	SEND_MSG_TRACE("m3ua", Payload_Data_DATA);
}

void OSI_SCTP_M3UA__Destination_Available_DAVA(int m3ua_user_index)
{
	int netcs_index = sigtran_user[m3ua_user_index].netcs_index;
	int cnt, osi_offset = 0, buffer_write_counter = 0xFFFF;
	int SIZEINBYTESAffected_Point_Code = 0, m3acon_index = 0xFFFF;

	unsigned int unique_ss7con_m3acon_own_sp_counter = 0;
	unsigned int unique_ss7con_m3acon_own_sp_value[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];

	for (cnt=0;cnt<PARSER_MML_PARAM_MAX_NUMBER_MASK;cnt++) {
		if (MML_PARAM_MASKS_CHECK(ss7con, cnt)) {
			// get m3acon over dest
			MML_1KEY_get_index(m3acon, dest, mml.Css7con.par[cnt].Psp, m3acon_index);
			if (m3acon_index != 0xFFFF) {
				unique_ss7con_m3acon_own_sp_value[unique_ss7con_m3acon_own_sp_counter] = atoi(mml.Css7con.par[cnt].Pownsp);
				unique_ss7con_m3acon_own_sp_counter++;
			}
		}
	}

	if (((struct Sm3ua_user *) sigtran_user[m3ua_user_index].pUserAdaptation)->VAffected_Point_Code.COUNTER >0)
		SIZEINBYTESAffected_Point_Code = sizeof(struct Ssigtran_fix_parameter_hdr) + /* 4 bytes, only once */
										 unique_ss7con_m3acon_own_sp_counter * 4;

	if (unique_ss7con_m3acon_own_sp_counter == 0) {
		FAST1ErrorTraceHandle(1, "OSI_SCTP_M3UA__Destination_Available_DAVA() No own Signalling points defined with ss7con !\n");
	} else {
		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_counter);

		FAST3ErrorTraceHandle(2, "OSI_SCTP_M3UA__Destination_Available_DAVA() %d own Signalling points defined, affected %d.\n", unique_ss7con_m3acon_own_sp_counter, SIZEINBYTESAffected_Point_Code);

		struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_counter);
		pmsg_sigtran_fix_message_hdr->version = 1;
		pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
		pmsg_sigtran_fix_message_hdr->class   = (enum Esigtran_fix_message_hdr_class) SS7_Management_Messages_SSNM;
		pmsg_sigtran_fix_message_hdr->type    = Destination_Available_DAVA;
		pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8 + SIZEINBYTESAffected_Point_Code, 4);
		osi_offset += sizeof(struct Ssigtran_fix_message_hdr);

		struct Ssigtran_fix_parameter_hdr *pmsg_sigtran_fix_parameter_hdr = (struct Ssigtran_fix_parameter_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_counter);
		pmsg_sigtran_fix_parameter_hdr->parameter_tag 	= LITTLETOBIG(Affected_Point_Code, 2);
		pmsg_sigtran_fix_parameter_hdr->length 			= LITTLETOBIG(SIZEINBYTESAffected_Point_Code, 2);
		osi_offset += sizeof(struct Ssigtran_fix_parameter_hdr);

		unsigned int *pAffected_Point_Code = (unsigned int *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_counter);
		for(cnt=0; cnt<unique_ss7con_m3acon_own_sp_counter; cnt++, osi_offset += 4) {
			*pAffected_Point_Code = LITTLETOBIG(unique_ss7con_m3acon_own_sp_value[cnt], 4);
		}

		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_counter, 8 + SIZEINBYTESAffected_Point_Code);

		SEND_MSG_TRACE("m3ua", Destination_Available_DAVA);
	}

}


// *********************** M3UA INITIALIZATION ***********************

void M3UA_USER_LOOP_RESET(int m3ua_user_loop_id) {
	memset(&m3ua_user_loop[m3ua_user_loop_id], 0, sizeof(struct Sm3ua_user_loop));
}
void M3UA_USER_LOOP_RESETALL(void) {
	memset(&m3ua_user_loop, 0, sizeof(struct Sm3ua_user_loop)*M3UA_USER_LOOP_MAX_SIZE);
}
