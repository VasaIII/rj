#define SIGTRAN
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/mml/parser.h"
#include <modules/protocols/trunk/isup.h>
#include <modules/protocols/mobile/sccp.h>
#include <modules/protocols/sigtran.h>
#undef SIGTRAN

void* process_sigtran_thread(void *arg) {
	int user_index   = ((struct spthread_data_sigtran *) arg)->user_index;
	int parse_select = ((struct spthread_data_sigtran *) arg)->parse_select;
	int netcs_index  = ((struct spthread_data_sigtran *) arg)->netcs_index;
	int cnt;
	unsigned int USR_TR_local;
	unsigned int TRN_REC_local;

	FAST4ErrorTraceHandle(2, "process_sigtran_thread() Started for user_index=%d, parse_select=%d, netcs_index=%d\n",
			  user_index, parse_select, netcs_index);

	while(sigtran_user[user_index].reserved) {

		// This stands for: when there is no messages in receive buffer, check if there are messages to be sent from fifo
		TRN_REC_local = STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[netcs_index].transport_receive.read);
		while ((!NETCS_RECEIVED_DINGDONG(netcs_index)) ||
			   (!NETCS_RECEIVED_CHECK_STATUS(NETCS_USERDATAVALID, netcs_index, TRN_REC_local))) {

			if (!sigtran_user[user_index].reserved) break;

			if (sigtran_user[user_index].both.shuttingdown) {
				if (!sigtran_user[user_index].both.vASPDN_sent) {
					SIGTRAN__ASP_Down_ASPDN(netcs_index, &user_index);
				}
			}

			/* old with both mask and read/write check
			while ((NETCS_USER_TRANSMIT_FIFO_DINGDONG(netcs_index)) &&
				   (NETCS_USER_TRANSMIT_FIFO_R_CHECK_STATUS(NETCS_WRITELOCK, netcs_index))) {
			 */
			// While checking for packets to be received, stay in loop while there are messages in fifo to be sent
			while ((NETCS_USER_TRANSMIT_FIFO_DINGDONG(netcs_index)) &&
				   (NETCS_USER_TRANSMIT_FIFO_R_CHECK_STATUS(NETCS_WRITELOCK, netcs_index))) {

				USR_TR_local = NetCS_DataSet[netcs_index].fifo_user_transmit.buffer[STS_COUNTER_VALUE_FIFO_R(netcs_index)].user_transmit_index;

				FAST4ErrorTraceHandle(2, "process_sigtran_thread() Send message counters: FIFO_R=%d, FIFO_W=%d, USR_TR=%d\n",
						STS_COUNTER_VALUE(sts_netcs_fifo_user_transmit, NetCS_DataSet[netcs_index].fifo_user_transmit.read),
						STS_COUNTER_VALUE(sts_netcs_fifo_user_transmit, NetCS_DataSet[netcs_index].fifo_user_transmit.write),
						USR_TR_local);

				if (NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 1) {

					FAST6ErrorTraceHandle(2, "process_sigtran_thread(i'm Client)\nClient {ip=%s, port=%d, sock=%d}\nServer {ip=%s, port=%d, sock=unknown}\n",
							NetCS_DataSet[netcs_index].trn_status.client.ip,
							NetCS_DataSet[netcs_index].trn_status.client.port_from_command,
							NetCS_DataSet[netcs_index].trn_status.client.socket_id,
							NetCS_DataSet[netcs_index].trn_status.server.ip,
							NetCS_DataSet[netcs_index].trn_status.server.port);

					NetCS_SCTPClientSendStringToHostPort(
							NetCS_DataSet[netcs_index].trn_status.server.ip,
							NetCS_DataSet[netcs_index].trn_status.server.port,
							NetCS_DataSet[netcs_index].trn_status.client.port_from_command,
							0,
							NetCS_DataSet[netcs_index].trn_status.client.socket_id,
							NULL,
							(void *) NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].data,
							NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].size,
							NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].stream_no,
							1);
				} else if (NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 2) {
					FAST7ErrorTraceHandle(2, "process_sigtran_thread(i'm Server)\nClient {ip=%s, port=%d(netcs=%d), sock=unknown}\nServer {ip=%s, port=%d, sock=%d}\n",
							NetCS_DataSet[netcs_index].trn_status.client.ip,
							NetCS_DataSet[netcs_index].trn_status.client.port_from_command,
							NetCS_DataSet[netcs_index].trn_status.client.port_from_NETCS,
							NetCS_DataSet[netcs_index].trn_status.server.ip,
							NetCS_DataSet[netcs_index].trn_status.server.port,
							NetCS_DataSet[netcs_index].trn_status.server.socket_id);

					NetCS_SCTPClientSendStringToHostPort(
							NetCS_DataSet[netcs_index].trn_status.client.ip,
							NetCS_DataSet[netcs_index].trn_status.client.port_from_command,
							NetCS_DataSet[netcs_index].trn_status.server.port,
							0,
							NetCS_DataSet[netcs_index].trn_status.client.socket_id, // if i'm serve i send data to client socket, otherwise i get "broken pipe"
							NULL,
							(void *) NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].data,
							NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].size,
							NetCS_DataSet[netcs_index].user_transmit.buffer[USR_TR_local].stream_no,
							1);
				}

				NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO(userinfo, netcs_index, USR_TR_local);
			} // while(TRANSMIT)

			NETCS_USER_SLEEP(netcs_index);
		} // while(RECEIVE)

#ifndef FAST
		FAST3ErrorTraceHandle(2, "SIGTRAN SET-%d RECEIVE (%dbytes) >> ", netcs_index, NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].size);
		for(cnt=0;cnt<NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].size; cnt++)
			FAST2ErrorTraceHandle(2, "0x%X ", NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[cnt] & 0xff);
		FAST1ErrorTraceHandle(2, "\n");
#endif

		user_index = sigtran_parse(&user_index,
								   (enum ENETCS_ParsedProtocols) parse_select,
								   TRN_REC_local,
								   (void *) &NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[0],
								   (void *) &NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[0],
								   (void *) &NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].size-1]);

		NETCS_RECEIVED_CLR_STATUS(sigtran, netcs_index, TRN_REC_local, NETCS_USERDATAVALID);
		NETCS_RECEIVED_READED(sigtran,	netcs_index);
	}

	FAST4ErrorTraceHandle(2, "process_sigtran_thread() Stopped for user_index=%d, parse_select=%d, netcs_index=%d\n",
					      user_index, parse_select, netcs_index);

	sigtran_USER_INDEX_RESET(user_index);
	pthread_exit(NULL);
}


unsigned int sigtran_parse(unsigned int *user_id,
						   int 			parse_select,
						   unsigned int TRN_REC_local,
						   void   		*msg_sigtran_pstartbuff,
						   void   		*msg_sigtran_pworkingbuff,
						   void   		*msg_sigtran_pendbuff)
{
	int netcs_index 	= sigtran_user[*user_id].netcs_index;
	PARSEPREAMBLE("sigtran_parse");

	SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_1, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuff, msg_sigtran_pendbuff);

	FAST6ErrorTraceHandle(2, "sigtran_parse(select=%d, user_id=%d) UP: parse M3UA<0x%x-working<0x%x->-0x%x>\n",
						 parse_select,
						 *user_id,
						 msg_sigtran_pstartbuff,
						 msg_sigtran_pworkingbuff,
						 msg_sigtran_pendbuff);

	switch ((enum ENETCS_ParsedProtocols) parse_select) {

	// ******************************************************************
	// Action on messages level (parse_select)
	// ******************************************************************

	ECASE_TRACE(case_string, m3ua_HEADER);
	{
		void  *msg_sigtran_pworkingbuf_new = msg_sigtran_pworkingbuff;
		struct Ssigtran_fix_message_hdr *pparse_fix_hdr = (struct Ssigtran_fix_message_hdr *) msg_sigtran_pworkingbuff;
		char *msg_sigtran_pworkingbuf_print;
		int cnt, msg_length_bytes_remain = 0;
		bool parsed = false;

		FAST4ErrorTraceHandle(2, "m3ua_HEADER(user_id=%d): Version = 0x%x\nLength  = %d\n",
				*user_id,
				(int) pparse_fix_hdr->version,
				(int) LITTLETOBIG(pparse_fix_hdr->length, 4));

		msg_sigtran_pworkingbuf_new += sizeof(struct Ssigtran_fix_message_hdr);
		msg_length_bytes_remain = msg_sigtran_pendbuff-msg_sigtran_pworkingbuf_new+1;
		SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_2, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);

#ifndef FAST
		msg_sigtran_pworkingbuf_print = msg_sigtran_pworkingbuff;
		for(cnt=0;cnt<(int) LITTLETOBIG(pparse_fix_hdr->length, 4); cnt++, msg_sigtran_pworkingbuf_print++)
			FAST2ErrorTraceHandle(2, "0x%x ", (*(char *)(msg_sigtran_pworkingbuf_print)) & 0xff);
		FAST1ErrorTraceHandle(2, "\n");
		sprintf(case_string, "m3ua_HEADER(user_id=%d): %s", *user_id, "Action on msg class = ");
#endif

		// ******************************************************************
		// Action on messages level, discrimination on message class
		// ******************************************************************

		if (!(parsed=switch_up_sigtran_parse_message(user_id, TRN_REC_local, pparse_fix_hdr, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff))) {
			FAST2ErrorTraceHandle(1, "m3ua_HEADER(user_id=%d)->switch_up_sigtran_parse_message ... message discarded.\n", *user_id);
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[netcs_index].stats.c[SIGTRAN_discard_message], 1);
			return *user_id;
		}

		FAST3ErrorTraceHandle(2, "m3ua_HEADER(user_id=%d): remains %d bytes to parse.\n", *user_id, msg_length_bytes_remain);

		if (msg_length_bytes_remain>0) {// if more message remains after parsing fix part
			*user_id = sigtran_parse(user_id, m3ua_PARAMETERS, TRN_REC_local, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);
		}

		FAST5ErrorTraceHandle(2, "m3ua_HEADER(user_id=%d): DOWN parse M3UA<0x%x-working<0x%x->-0x%x>\n",
								 *user_id,
								 msg_sigtran_pstartbuff,
								 msg_sigtran_pworkingbuf_new,
								 msg_sigtran_pendbuff);

		// ******************************************************************
		// ReAction on messages level (parse_select)
		// ******************************************************************

#ifndef FAST
		sprintf(case_string, "%s", "ReAction on msg class =");
#endif
		// ******* downstream processes only single message, if more tha one message
		// needs to be downstreamed at same time from higher layer, all layers should
		// be filled at that layer
		parsed = switch_down_sigtran_parse_message(user_id, TRN_REC_local, pparse_fix_hdr, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);
	}
	break; // m3ua_HEADER



	// ******************************************************************
	// parameter level (parse_select)
	// ******************************************************************

	ECASE_TRACE(case_string, m3ua_PARAMETERS);
	{
		int cnt, padding=0, msg_length_bytes_remain = 0;
		struct Ssigtran_fix_parameter_hdr *pparse_fix_parameter = (struct Ssigtran_fix_parameter_hdr *) msg_sigtran_pworkingbuff;
		struct Ssigtran_fix_parameter_hdr  parse_fix_parameter; // copy here due to later processing
		bool parsed 			= false;

		void *msg_sigtran_pworkingbuf_new   = msg_sigtran_pworkingbuff;
		char *msg_sigtran_pworkingbuf_print;

		parse_fix_parameter.parameter_tag = (short int) LITTLETOBIG(pparse_fix_parameter->parameter_tag, 2);
		parse_fix_parameter.length        = (short int) LITTLETOBIG(pparse_fix_parameter->length, 2);

		// The total length of a parameter (including Tag, Parameter Length and
		// Value fields) must be a multiple of 4 bytes. If the length of the parameter is
		// not a multiple of 4 bytes, the sender pads the parameter at the end (i.e.,
		// after the Parameter Value field) with all zero bytes.
		padding = (4 - parse_fix_parameter.length % 4);
		padding = (padding == 4)? 0: padding;
		parse_fix_parameter.length += padding;

		msg_sigtran_pworkingbuf_new  += sizeof(struct Ssigtran_fix_parameter_hdr); // shift for size of fix part (tag, length)
		SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_3, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);

		// print rest of message in HEX
#ifndef FAST
		msg_sigtran_pworkingbuf_print = msg_sigtran_pworkingbuf_new;
		for(cnt=0;cnt<(parse_fix_parameter.length - 4 /*fix part*/); cnt++, msg_sigtran_pworkingbuf_print++)
			FAST2ErrorTraceHandle(2, "0x%x ", (*(char *)(msg_sigtran_pworkingbuf_print)) & 0xff);
		FAST1ErrorTraceHandle(2, "\n");

		FAST3ErrorTraceHandle(2, "m3ua_PARAMETERS: length        = %d (-padding of %d bytes)\n", parse_fix_parameter.length, padding);
		FAST2ErrorTraceHandle(2, "m3ua_PARAMETERS: tag           = 0x%x\n", 					 parse_fix_parameter.parameter_tag);
		sprintf(case_string, "m3ua_PARAMETERS: %s", "parameter     = ");
#endif

		if (!(parsed=switch_up_sigtran_parse_parameter(user_id, TRN_REC_local, pparse_fix_parameter, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff))) {
			FAST3ErrorTraceHandle(1, "m3ua_PARAMETERS(user_id=%d)->switch_up_sigtran_parse_parameter ... message discarded (parameter_tag=0x%x).\n", *user_id, parse_fix_parameter.parameter_tag);
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[netcs_index].stats.c[SIGTRAN_discard_message], 1);
			return *user_id;
		}

		msg_sigtran_pworkingbuf_new = msg_sigtran_pworkingbuf_new
								      - sizeof(struct Ssigtran_fix_parameter_hdr) // lenght includes fix part of parameter header
								      + parse_fix_parameter.length ; // shift for the length of parameter
		msg_length_bytes_remain = msg_sigtran_pendbuff-msg_sigtran_pworkingbuf_new+1;
		// ??? ovde se dogodilo par eventa ...
		SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_4, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);

		FAST2ErrorTraceHandle(2, "m3ua_PARAMETERS: remains %d bytes to parse.\n", msg_length_bytes_remain);
		if (msg_length_bytes_remain>0) {
			*user_id = sigtran_parse(user_id, m3ua_PARAMETERS, TRN_REC_local, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuf_new, msg_sigtran_pendbuff);
		}

		msg_sigtran_pworkingbuff = msg_sigtran_pworkingbuf_new; // return current position of msg_pcurrbuf so that down processing continues where m3ua parameters finished

		FAST4ErrorTraceHandle(2, "m3ua_PARAMETERS: DOWN: parse M3UA<0x%x-working<0x%x->-0x%x>\n",
								 msg_sigtran_pstartbuff,
								 msg_sigtran_pworkingbuf_new,
								 msg_sigtran_pendbuff);
	}

	break; // sm3ua_PARAMETERS

	}

	return *user_id;
}


bool switch_up_sigtran_parse_message(unsigned int 	*user_id,
									 unsigned int	TRN_REC_local,
									 struct 		Ssigtran_fix_message_hdr *pparse_fix_hdr,
									 void   		*msg_sigtran_pstartbuff,
									 void   		*msg_sigtran_pworkingbuff,
									 void   		*msg_sigtran_pendbuff)
{
	PARSEPREAMBLE("sigtran");
	int netcs_index 	= sigtran_user[*user_id].netcs_index;
	struct Ssigtran_fix_message_hdr  parse_fix_hdr;
	parse_fix_hdr.class = (char) pparse_fix_hdr->class;
	parse_fix_hdr.type  = (char) pparse_fix_hdr->type;

	SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_5, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuff, msg_sigtran_pendbuff);

	// **********************************************************************************
	// ***************************** SIGTRAN MESSAGES up ********************************
	// **********************************************************************************
	switch ((enum Esigtran_fix_message_hdr_class) parse_fix_hdr.class) {

	ECASE_TRACE(case_string, ManagementMessages_MGMT);

		switch ((enum Esigtran_fix_message_hdr_class_type__ManagementMessages_MGMT) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, Error_ERR);	break;
		RECV_MSG_TRACE_case(case_string, Notify_NTFY);	break;
		}

	break;

	ECASE_TRACE(case_string, SS7_Management_Messages_SSNM);

		switch ((enum Esigtran_fix_message_hdr_class_type__SS7_Management_Messages_SSNM) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, Destination_Unavailable_DUNA);				break;
		RECV_MSG_TRACE_case(case_string, Destination_Available_DAVA);				break;
		RECV_MSG_TRACE_case(case_string, Destination_State_Audit_DAUD);				break;
		RECV_MSG_TRACE_case(case_string, Signalling_Congestion_SCON);				break;
		RECV_MSG_TRACE_case(case_string, Destination_User_Part_Unavailable_DUPU);	break;
		RECV_MSG_TRACE_case(case_string, Destination_Restricted_DRST);				break;
		}

	break;

	ECASE_TRACE(case_string, ASP_State_Maintenance_Messages_ASPSM);

		switch ((enum Esigtran_fix_message_hdr_class_type__ASP_State_Maintenance_Messages_ASPSM) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, ASP_Up_ASPUP);
			// first message in activation process from client sequence
			sigtran_user[*user_id].server.vASPUP_received = true;
		break;
		RECV_MSG_TRACE_case(case_string, ASP_Up_Acknowledgement_ASPUPACK);
			sigtran_user[*user_id].client.vASPUPACK_received = true;
			sigtran_user[*user_id].client.waiting_handshake = 0;
		break;
		RECV_MSG_TRACE_case(case_string, ASP_Down_ASPDN);						break;
		RECV_MSG_TRACE_case(case_string, Heartbeat_BEAT);		 				break;
		RECV_MSG_TRACE_case(case_string, ASP_Down_Acknowledgement_ASPDNACK);
			sigtran_user[*user_id].reserved = 0;
			NETCS_USER_WAKEUP(netcs_index);
		break;
		RECV_MSG_TRACE_case(case_string, Heartbeat_Acknowledgement_BEATACK);	break;
		}

	break;

	ECASE_TRACE(case_string, ASP_Traffic_Maintenance_Messages_ASPTM);

		switch ((enum Esigtran_fix_message_hdr_class_type__ASP_Traffic_Maintenance_Messages_ASPTM) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, ASP_Active_ASPAC);
			sigtran_user[*user_id].server.vASPAC_received = true;
		break;
		RECV_MSG_TRACE_case(case_string, ASP_Active_Acknowledgement_ASPACACK);
			sigtran_user[*user_id].client.vASPACACK_received = true;
		break;
		RECV_MSG_TRACE_case(case_string, ASP_Inactive_ASPIA);		 				break;
		RECV_MSG_TRACE_case(case_string, ASP_Inactive_Acknowledgement_ASPIAACK);	break;
		}

	break;


	// **********************************************************************************
	// ******************************** M3UA MESSAGES up ********************************
	// **********************************************************************************
	ECASE_TRACE(case_string, Transfer_Messages);

		switch ((enum Em3ua_fix_message_hdr_class_type) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, Payload_Data_DATA);		break;
		}

	break;

	ECASE_TRACE(case_string, Routing_Key_Management_Messages_RKM);

		switch ((enum Em3ua_fix_message_hdr_class_type) parse_fix_hdr.type) {
		RECV_MSG_TRACE_case(case_string, Registration_Request_REGREQ);		break;
		RECV_MSG_TRACE_case(case_string, Registration_Response_REGRSP);		break;
		RECV_MSG_TRACE_case(case_string, Deregistration_Request_DEREGREQ);	break;
		RECV_MSG_TRACE_case(case_string, Deregistration_Response_DEREGRSP);	break;
		}

	break;
	}

	return RECV_MSG_TRACE_parsed;
}

bool switch_up_sigtran_parse_parameter(unsigned int *user_id,
									   unsigned int	TRN_REC_local,
									   struct 		Ssigtran_fix_parameter_hdr *pparse_fix_parameter,
									   void   		*msg_sigtran_pstartbuff,
									   void   		*msg_sigtran_pworkingbuff,
									   void   		*msg_sigtran_pendbuff)
{
	PARSEPREAMBLE("sigtran");
	struct Ssigtran_fix_parameter_hdr  parse_fix_parameter;
	parse_fix_parameter.parameter_tag = (short int) LITTLETOBIG(pparse_fix_parameter->parameter_tag, 2);

	SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_6, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuff, msg_sigtran_pendbuff);

	FAST3ErrorTraceHandle(2, "switch_up_sigtran_parse_parameter(user_id=%d) parameter_tag=0x%x\n", *user_id, parse_fix_parameter.parameter_tag);

	// **********************************************************************************
	// ***************************** SIGTRAN PARAMETERS up ******************************
	// **********************************************************************************
	switch ((enum Esigtran_fix_parameter_hdr_tag) (short int) parse_fix_parameter.parameter_tag) {

	RECV_MSG_TRACE_case(case_string, Error_Code);
	{
		unsigned int *pError_Code = (unsigned int *) msg_sigtran_pworkingbuff; // 4 bytes
		unsigned int  vError_Code = LITTLETOBIG(*pError_Code, 4);  // copy here due to later processing

		sprintf(case_string, "%s", "value         = ");

		switch ((enum Esigtran_parameter_Error_Code) vError_Code) {
		ECASE_TRACE(case_string, Invalid_Version);					break;
		ECASE_TRACE(case_string, Unsupported_Message_Class);		break;
		ECASE_TRACE(case_string, Unsupported_Message_Type);			break;
		ECASE_TRACE(case_string, Unsupported_Traffic_Mode_Type);	break;
		ECASE_TRACE(case_string, Unexpected_Message);				break;
		ECASE_TRACE(case_string, Protocol_Error);					break;
		ECASE_TRACE(case_string, Invalid_Stream_Identifier);		break;
		ECASE_TRACE(case_string, Refused_Management_Blocking);		break;
		ECASE_TRACE(case_string, ASP_Identifier_Required);			break;
		ECASE_TRACE(case_string, Invalid_ASP_Identifier);			break;
		ECASE_TRACE(case_string, Invalid_Parameter_Value);			break;
		ECASE_TRACE(case_string, Parameter_Field_Error);			break;
		ECASE_TRACE(case_string, Unexpected_Parameter);				break;
		ECASE_TRACE(case_string, Destination_Status_Unknown);		break;
		ECASE_TRACE(case_string, Invalid_Network_Appearance);		break;
		ECASE_TRACE(case_string, Missing_Parameter);				break;
		ECASE_TRACE(case_string, Invalid_Routing_Context);			break;
		ECASE_TRACE(case_string, No_Configured_AS_for_ASP);			break;
		}

	}
	break;

	RECV_MSG_TRACE_case(case_string, Diagnostic_information);		break;


	// **********************************************************************************
	// ******************************** M3UA PARAMETERS up ******************************
	// **********************************************************************************
	RECV_MSG_TRACE_case(case_string, Protocol_Data);
	{
		char *msg_m3ua_pworkingbuff_new = msg_sigtran_pworkingbuff;
		unsigned int *pOPC, *pDPC, vOPC, vDPC;
		unsigned char *pSI, *pNI, *pMP, *pSLS;
		char m3ua_si = ((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->M3uaAdaptation_ServiceIndicator;

		pOPC = (unsigned int *) msg_m3ua_pworkingbuff_new; // 4 bytes
		vDPC = LITTLETOBIG(*pOPC, 4);  // copy here due to later processing
		msg_m3ua_pworkingbuff_new +=4;
		pDPC = (unsigned int *) msg_m3ua_pworkingbuff_new; // 4 bytes
		vOPC = LITTLETOBIG(*pDPC, 4);  // copy here due to later processing
		msg_m3ua_pworkingbuff_new +=4;
		pSI  = (unsigned char *) msg_m3ua_pworkingbuff_new; // 1 byte
		msg_m3ua_pworkingbuff_new +=1;
		pNI  = (unsigned char *) msg_m3ua_pworkingbuff_new; // 1 byte
		msg_m3ua_pworkingbuff_new +=1;
		pMP  = (unsigned char *) msg_m3ua_pworkingbuff_new; // 1 byte
		msg_m3ua_pworkingbuff_new +=1;
		pSLS = (unsigned char *) msg_m3ua_pworkingbuff_new; // 1 byte
		msg_m3ua_pworkingbuff_new +=1; // skip pSLS

		FAST8ErrorTraceHandle(2, "switch_up_sigtran_parse_parameter() Protocol_Data upstream received:\nOPC=%d, DPC=%d, SI=%d, NI=%d, MP=%d, SLS=%d, TRN_REC_local-%d\n",
							  vDPC, vOPC, *pSI, *pNI, *pMP, *pSLS, TRN_REC_local);

		if (m3ua_si != *pSI) { // if configured and received Service Information does not match, ignore message
			FAST3ErrorTraceHandle(2, "switch_up_sigtran_parse_parameter() Configured SI (%d) and received SI (%d) in Protocol data does not match, ignore m3ua message.\n",
								  m3ua_si, *pSI);
			return ECASE_TRACE_parsed;
		}

		// Parse SCCP on M3UA param up since SCCP in integrated in M3UA Protocol_Data message and it's parameter length
		if (m3ua_si == 3 /* SCCP */) {
			*user_id = sccp_parse(*user_id,
								  (enum ENETCS_ParsedProtocols) sccp_HEADER,
								   msg_sigtran_pstartbuff,		/* start of m3ua */
								   msg_m3ua_pworkingbuff_new, 	/* start of SCCP */
								   msg_sigtran_pendbuff);
		} else if ((m3ua_si == 5 /* ISUP */) || (m3ua_si == 13 /* BICC */)) {

			if (!((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->LoopActive) { // if not loop configured for this destination
				*user_id = isup_parse(*user_id,
									  (enum ENETCS_ParsedProtocols) isup_HEADER,
									  msg_sigtran_pstartbuff,	 	/* start of m3ua */
									  msg_m3ua_pworkingbuff_new, 	/* start of ISUP */
									  msg_sigtran_pendbuff);
			} else { // loop configured, no parsing active just modify m3ua protocol data and maybe cic
				unsigned int match_m3ua_loop_index;
				unsigned int *pOPC_looped;
				unsigned int *pDPC_looped;
				unsigned int length;
				unsigned int netcs_index1, netcs_index2;
				unsigned int user_id2;
				int buffer_write_counter = 0xFFFF;
				int osi_offset = 0;

				match_m3ua_loop_index 	= ((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->LoopId;

				netcs_index1 			= m3ua_user_loop[match_m3ua_loop_index].netcs_index1;
				netcs_index2 			= m3ua_user_loop[match_m3ua_loop_index].netcs_index2;
				NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(m3ua_loopback, netcs_index2, buffer_write_counter);
				user_id2	 			= ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index2].puser_data)->sigtran_user_id;
				NetCS_DataSet[netcs_index2].user_transmit.buffer[buffer_write_counter].stream_no = NetCS_DataSet[netcs_index1].transport_receive.buffer[TRN_REC_local].stream_no;

				FAST4ErrorTraceHandle(2, "DEBUG: netcs_index1=%d, netcs_index2=%d, m3ua_loop_user=%d\n",
						netcs_index1,
						netcs_index2,
						match_m3ua_loop_index);

				unsigned char *precmsg = (unsigned char *) msg_sigtran_pstartbuff;
				unsigned char *pnewmsg = (unsigned char *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index2, osi_offset, buffer_write_counter);
				struct SProtocol_Data 		 *precmsg_Protocol_Data;
				struct Sisup_fix_message_hdr *precmsg_isup_fix_message_hdr;

				length		 = (unsigned int) LITTLETOBIG(((struct Ssigtran_fix_message_hdr *) precmsg)->length, 4);
				precmsg    	+= sizeof(struct Ssigtran_fix_message_hdr);
				precmsg    	+= sizeof(struct Ssigtran_fix_parameter_hdr);
				precmsg_Protocol_Data       = (struct SProtocol_Data        *) precmsg;
				precmsg_Protocol_Data->vOPC = LITTLETOBIG(((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vOPC, 4);
				precmsg_Protocol_Data->vDPC = LITTLETOBIG(((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vDPC, 4);
				precmsg_Protocol_Data->vSI  = ((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vSI;
				precmsg_Protocol_Data->vNI  = ((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vNI;
				precmsg_Protocol_Data->vMP  = ((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vMP;
				precmsg_Protocol_Data->vSLS = ((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vSLS;
				precmsg    	+= sizeof(struct SProtocol_Data);
				precmsg_isup_fix_message_hdr = (struct Sisup_fix_message_hdr *) precmsg;
				precmsg_isup_fix_message_hdr->cic = (short int) ((precmsg_isup_fix_message_hdr->cic + m3ua_user_loop[match_m3ua_loop_index].cicinc1)&MASK_sts_isup_cic_id);

				FAST10ErrorTraceHandle(2, "M3UA (length=%d) with Protocol_Data downstream sent:\nOPC=%d, DPC=%d, SI=%d, NI=%d, MP=%d, SLS=%d, IT_stream_no=%d, OT_stream_no=%d\n",
									length,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vOPC,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vDPC,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vSI,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vNI,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vMP,
									((struct Sm3ua_user *) sigtran_user[user_id2].pUserAdaptation)->Protocol_Data_Configured.vSLS,
									NetCS_DataSet[netcs_index1].transport_receive.buffer[TRN_REC_local].stream_no,
									NetCS_DataSet[netcs_index2].user_transmit.buffer[buffer_write_counter].stream_no);
				memcpy(pnewmsg, msg_sigtran_pstartbuff, length);

				NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index2, buffer_write_counter, length);

				SEND_MSG_TRACE("m3ua", Payload_Data_DATA);
			}
		}
	}
	break;

	RECV_MSG_TRACE_case(case_string, Affected_Point_Code);
	{
		char *msg_m3ua_pworkingbuff_new = msg_sigtran_pworkingbuff;
		int cnt, length;

		// reset Destination_State_Audit_DAUD parameters structure
		((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.COUNTER = 0;
		((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.MAX_COUNTER_VALUE = 128;

		length = ((pparse_fix_parameter->length-4/*fix part*/)/4);
		for (cnt=0; cnt<length; cnt++,
							    ((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.COUNTER++,
							    msg_m3ua_pworkingbuff_new+=4 /* go to next Affected_Point_Code value within same parameter tag */ )
		{
			SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_8, *user_id, msg_sigtran_pstartbuff, msg_m3ua_pworkingbuff_new, msg_sigtran_pendbuff);

			if (((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.COUNTER >=
				((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.MAX_COUNTER_VALUE)
			{
				FAST2ErrorTraceHandle(2, "maximum number of Affected_Point_Code in Destination_State_Audit_DAUD reached (%d).\n",
									 ((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.MAX_COUNTER_VALUE);
			} else {
				// Mask field is fixed to 0 so we can leave it in MSB
				((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.ARRAY[((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.COUNTER] = LITTLETOBIG(*(unsigned int *)msg_m3ua_pworkingbuff_new, 4);
				FAST2ErrorTraceHandle(2, "Affected_Point_Code = %d\n",
									 ((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.ARRAY[((struct Sm3ua_user *) sigtran_user[*user_id].pUserAdaptation)->VAffected_Point_Code.COUNTER]);
			}
		}
	}
	break;

	}

	return RECV_MSG_TRACE_parsed;
}


bool switch_down_sigtran_parse_message(unsigned int *user_id,
									   unsigned int	TRN_REC_local,
									   struct 		Ssigtran_fix_message_hdr *pparse_fix_hdr,
									   void   		*msg_sigtran_pstartbuff,
									   void   		*msg_sigtran_pworkingbuff,
									   void   		*msg_sigtran_pendbuff)
{
	PARSEPREAMBLE("sigtran");
	int netcs_index 	= sigtran_user[*user_id].netcs_index;
	int buffer_write_index = 0xFFFF;
	struct Ssigtran_fix_message_hdr  parse_fix_hdr;
	parse_fix_hdr.class = (char) pparse_fix_hdr->class;
	parse_fix_hdr.type  = (char) pparse_fix_hdr->type;

	SAFE_MEMAREA_CHECK(SIGTRAN_upstream_parsing_event_7, *user_id, msg_sigtran_pstartbuff, msg_sigtran_pworkingbuff, msg_sigtran_pendbuff);

	switch ((enum Esigtran_fix_message_hdr_class) parse_fix_hdr.class) {

	// **********************************************************************************
	// ***************************** SIGTRAN PARAMETERS down ****************************
	// **********************************************************************************
	ECASE_TRACE(case_string, ASP_State_Maintenance_Messages_ASPSM);
	{
		sprintf(case_string, "%s", "ReAction on ");

		switch ((enum Esigtran_fix_message_hdr_class_type__ASP_State_Maintenance_Messages_ASPSM) parse_fix_hdr.type) {

		ECASE_TRACE(case_string, ASP_Up_ASPUP);
			SIGTRAN__ASP_Up_Acknowledgement_ASPUPACK(netcs_index, user_id);
		break;

		ECASE_TRACE(case_string, ASP_Up_Acknowledgement_ASPUPACK);
			SIGTRAN__ASP_Active_ASPAC(netcs_index, user_id);
		break;

		}
	}
	break; // ASP_State_Maintenance_Messages_ASPSM

	ECASE_TRACE(case_string, ASP_Traffic_Maintenance_Messages_ASPTM);
	{
		sprintf(case_string, "%s", "ReAction on ");

		switch ((enum Esigtran_fix_message_hdr_class_type__ASP_Traffic_Maintenance_Messages_ASPTM) parse_fix_hdr.type) {

		ECASE_TRACE(case_string, ASP_Active_ASPAC);
			SIGTRAN__ASP_Active_Acknowledgement_ASPACACK(netcs_index, user_id);
		break;

		}
	}
	break; // ASP_Traffic_Maintenance_Messages_ASPTM

	// **********************************************************************************
	// ******************************** M3UA PARAMETERS down ****************************
	// **********************************************************************************
	ECASE_TRACE(case_string, SS7_Management_Messages_SSNM);
	{
		// Esigtran_fix_message_hdr_class_type__SS7_Management_Messages_SSNM preformed
		// here untill Destination_Available_DAVA is not adjusted for all sigtran
		switch ((enum Esigtran_fix_message_hdr_class_type__SS7_Management_Messages_SSNM) parse_fix_hdr.type) {

		// ECASE_TRACE(case_string, Destination_Unavailable_DUNA); FAST1ErrorTraceHandle(1, " ... or ... \n");
		ECASE_TRACE(case_string, Destination_State_Audit_DAUD);
		{
			// Number of processed Affected_Point_Code received either in:
			// * Destination_State_Audit_DAUD or
			// * Destination_Unavailable_DUNA

			if ((NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 2) && // if server
				!sigtran_user[*user_id].server.vASPUPACK_sent)     // send DAVA on DAUD/DUNA only after ASPAC/ASPACACK sequence
				return ECASE_TRACE_parsed;

			if ((NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 1) && // if client
				!sigtran_user[*user_id].client.vASPUPACK_received) // send DAVA on DAUD/DUNA only after ASPAC/ASPACACK sequence
				return ECASE_TRACE_parsed;


			// send DAVA
			if (!sigtran_user[*user_id].client.vDestinations_audited)
				OSI_SCTP_M3UA__Destination_Available_DAVA(*user_id);
			sigtran_user[*user_id].client.vDestinations_audited = true;

		}
		break; // Destination_State_Audit_DAUD

		}
	}
	break; // SS7_Management_Messages_SSNM

	ECASE_TRACE(case_string, Transfer_Messages);
		switch ((enum Em3ua_fix_message_hdr_class_type) parse_fix_hdr.type) {
		ECASE_TRACE(case_string, Payload_Data_DATA);	break;
		}
	break; // Transfer_Messages

	} // CLASS switch

	return RECV_MSG_TRACE_parsed;
}


void SIGTRAN__ASP_Up_ASPUP(
		unsigned int netcs_index,
		unsigned int user_id)
{
	int osi_offset = 0;
	int buffer_write_index = 0xFFFF;

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);
	struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_index);
	pmsg_sigtran_fix_message_hdr->version = 1;
	pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
	pmsg_sigtran_fix_message_hdr->class   = ASP_State_Maintenance_Messages_ASPSM;
	pmsg_sigtran_fix_message_hdr->type    = ASP_Up_ASPUP;
	pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8, 4);
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, 8);

	sigtran_user[user_id].server.vASPUPACK_sent = true;
	SEND_MSG_TRACE("m3ua", ASP_Up_ASPUP);
}


void SIGTRAN__ASP_Up_Acknowledgement_ASPUPACK(
		unsigned int netcs_index,
		unsigned int *user_id)
{
	int osi_offset = 0;
	int buffer_write_index = 0xFFFF;

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);
	struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_index);
	pmsg_sigtran_fix_message_hdr->version = 1;
	pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
	pmsg_sigtran_fix_message_hdr->class   = ASP_State_Maintenance_Messages_ASPSM;
	pmsg_sigtran_fix_message_hdr->type    = ASP_Up_Acknowledgement_ASPUPACK;
	pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8, 4);
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, 8);

	sigtran_user[*user_id].server.vASPUPACK_sent = true;
	SEND_MSG_TRACE("m3ua", ASP_Up_Acknowledgement_ASPUPACK);
}


void SIGTRAN__ASP_Down_ASPDN(
		unsigned int netcs_index,
		unsigned int *user_id)
{
	int osi_offset = 0;
	int buffer_write_index = 0xFFFF;

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);
	struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_index);
	pmsg_sigtran_fix_message_hdr->version = 1;
	pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
	pmsg_sigtran_fix_message_hdr->class   = ASP_State_Maintenance_Messages_ASPSM;
	pmsg_sigtran_fix_message_hdr->type    = ASP_Down_ASPDN;
	pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8, 4);
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, 8);

	sigtran_user[*user_id].both.vASPDN_sent = true;
	SEND_MSG_TRACE("m3ua", ASP_Down_ASPDN);
}


void SIGTRAN__ASP_Active_ASPAC(
		unsigned int netcs_index,
		unsigned int *user_id)
{
	int osi_offset = 0;
	int buffer_write_index = 0xFFFF;

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);
	struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_index);
	pmsg_sigtran_fix_message_hdr->version = 1;
	pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
	pmsg_sigtran_fix_message_hdr->class   = ASP_Traffic_Maintenance_Messages_ASPTM;
	pmsg_sigtran_fix_message_hdr->type    = ASP_Active_ASPAC;
	pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8, 4);
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, 8);

	sigtran_user[*user_id].client.vASPAC_sent = true;
	SEND_MSG_TRACE("m3ua", ASP_Active_ASPAC);
}


void SIGTRAN__ASP_Active_Acknowledgement_ASPACACK(
		unsigned int netcs_index,
		unsigned int *user_id)
{
	int osi_offset = 0;
	int buffer_write_index = 0xFFFF;

	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, netcs_index, buffer_write_index);
	struct Ssigtran_fix_message_hdr *pmsg_sigtran_fix_message_hdr = (struct Ssigtran_fix_message_hdr *) pNETCS_USER_TRANSMIT_BUFFER(sctp, netcs_index, osi_offset, buffer_write_index);
	pmsg_sigtran_fix_message_hdr->version = 1;
	pmsg_sigtran_fix_message_hdr->dummy_1 = 0;
	pmsg_sigtran_fix_message_hdr->class   = ASP_Traffic_Maintenance_Messages_ASPTM;
	pmsg_sigtran_fix_message_hdr->type    = ASP_Active_Acknowledgement_ASPACACK;
	pmsg_sigtran_fix_message_hdr->length  = LITTLETOBIG(8, 4);
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, netcs_index, buffer_write_index, 8);

	sigtran_user[*user_id].server.vASPACACK_sent = true;
	SEND_MSG_TRACE("m3ua", ASP_Active_Acknowledgement_ASPACACK);
}


void sigtran_USER_INDEX_RESETALL(void) {
	int cnt;
	for (cnt=0;cnt<SIGTRAN_USER_MAX_SIZE_MASK;cnt++) {
		if (sigtran_user[cnt].pUserAdaptation != NULL) {
			// free M3UA connected layer from Service Indicator (SI)
			if (((struct Sm3ua_user *) sigtran_user[cnt].pUserAdaptation)->M3uaAdaptation_ServiceIndicator == (enum eSigtran_m3ua_UserAdaptation_Supported) sigtran_m3ua_SI_N_ISUP) {
				if (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation != NULL) {
					free(((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation);
				}
			}
			// free M3UA user adaptation layer of sigtran
			free(sigtran_user[cnt].pUserAdaptation);
		}
		memset(&sigtran_user[cnt], 0, sizeof(struct Ssigtran_user));
	}
}

void sigtran_USER_INDEX_RESET(int id) {
	ErrorTraceHandle(1, "sigtran_USER_INDEX_RESET(user-%d)\n", id);

	if(id<SIGTRAN_USER_MAX_SIZE_MASK) {
		ErrorTraceHandle(1, "sigtran_USER_INDEX_RESET(user-%d) Reseting.\n", id);
		if (sigtran_user[id].pUserAdaptation != NULL) {
			// free M3UA connected layer from Service Indicator (SI)
			if (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->M3uaAdaptation_ServiceIndicator == (enum eSigtran_m3ua_UserAdaptation_Supported) sigtran_m3ua_SI_N_ISUP) {
				if (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation != NULL) {
					free(((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation);
				}
			}
			// free M3UA user adaptation layer of sigtran
			free(sigtran_user[id].pUserAdaptation);
		}
		// sigtran thread is stopped with sigtran_user[user_index].reserved variable
		memset(&sigtran_user[id], 0, sizeof(sigtran_user[id]));
	}
}


void sigtran_m3ua_USER_INDEX_RESETALL(void) {
	// done in sigran resetall
}


void UA_sigtran_m3ua_USER_INDEX_INIT(int id, int key_mml_m3acon_index) {
	int ss7con_index = 0xFFFF;

	sigtran_user[id].pUserAdaptation = malloc(sizeof(struct Sm3ua_user));
	memset(sigtran_user[id].pUserAdaptation, 0, sizeof(struct Sm3ua_user));

	MML_1KEY_get_index(ss7con, sp, mml.Cm3acon.par[key_mml_m3acon_index].Pdest, ss7con_index);
	((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->mml_m3acon_index 					= key_mml_m3acon_index;
	((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->M3uaAdaptation_ServiceIndicator	= atoi(mml.Css7con.par[ss7con_index].Psi);

	// check which upper M3UA layer is indicated in Service Indicator (SI) and allocate memory for him
	if (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->M3uaAdaptation_ServiceIndicator == (enum eSigtran_m3ua_UserAdaptation_Supported) sigtran_m3ua_SI_N_ISUP) {

		((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation = malloc(sizeof(struct Sisup_cic_id));
		memset((((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation), 0, sizeof(struct Sisup_cic_id));

		// temporary solution since noone sends me GRS ...
		struct Sisup_cic_id *isup_cic_id 	= ((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->pM3uaAdaptation;
		int cnt;
		for (cnt=0; cnt<100; cnt++)
			isup_cic_id->data[cnt].device_state = cic_idle;
		for (cnt=500; cnt<800; cnt++)
			isup_cic_id->data[cnt].device_state = cic_idle;
	}
}


bool UA_sigtran_m3ua_USER_INDEX_CMP(int id, int key_mml_m3acon_index) {
	if (sigtran_user[id].pUserAdaptation == NULL)
		ErrorTraceHandle(0, "UA_m3ua_USER_INDEX() sigtran_user-%d reserved but not allocated !\n", id);

	if (((struct Sm3ua_user *) sigtran_user[id].pUserAdaptation)->mml_m3acon_index == key_mml_m3acon_index)
		return true;
	return false;
}




void NETCS_netcs_sigtran_tcp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy) {}
void NETCS_netcs_sigtran_udp_USER_INDEX_INIT(int id, int key_mml_mmlcnf_index, int key_dummy) {}
void NETCS_netcs_sigtran_sctp_USER_INDEX_INIT(int id, int key_mml_trnloc_index, int key_mml_trncon_index) {
	NetCS_DataSet[id].user 	      = netcs_sigtran; // ENETCS_Users
	NetCS_DataSet[id].puser_data  = malloc(sizeof(struct SSigtran_userof_NetCS_DataSet));
	memset(NetCS_DataSet[id].puser_data, 0, sizeof(struct SSigtran_userof_NetCS_DataSet));
	((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[id].puser_data)->mml_trnloc_index = key_mml_trnloc_index;
	((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[id].puser_data)->mml_trncon_index = key_mml_trncon_index;
	// sigtran_user_id is configured in SIGTRAN_USER_INDEX_NEW macro
}




bool NETCS_netcs_sigtran_tcp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy) {
	return false;
}
bool NETCS_netcs_sigtran_udp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy) {
	return false;
}
bool NETCS_netcs_sigtran_sctp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_mml_trncon_index) {
	if (NetCS_DataSet[id].puser_data == NULL)
		ErrorTraceHandle(0, "NETCS_sigtran_sctp_USER_INDEX_CMP() user-%d declared as sigtran_sctp but not allocated !\n", id);

	if ((((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[id].puser_data)->mml_trnloc_index == key_mml_trnloc_index) &&
		(((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[id].puser_data)->mml_trncon_index == key_mml_trncon_index))
		return true;
	return false;
}




