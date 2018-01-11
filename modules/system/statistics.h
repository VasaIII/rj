
#undef PREDEF
#ifdef STATISTICS
#define PREDEF
#else
#define PREDEF extern
#endif

enum eCounter_users {
	 sts_netcs_transport_receive,
	 sts_netcs_user_transmit,
	 sts_netcs_fifo_user_transmit,

	 // layer specifics
	 sts_netcs_transport_receive_M3UA_Protocol_Data,

	 sts_sccp_dlg_ref,
	 sts_isup_cic_id
};

enum eCounter_purposes {
	// MACRO dimensioning counters
	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER_all_occupied,
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO_all_occupied,
	NETCS_RECEIVED_GET_WRITE_COUNTER_all_occupied,
	NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied,
	POP_DIALOG_REF_all_occupied,
	POP_ISUP_CIC_ID_all_occupied,

	// message in/out counters
	OSI_SCTP_M3UA_SCCP_BSSAP_MAP__CL3I,
	switch_up_bssDtap_parse_message__LOCATION_UPDATING_ACCEPT,
	switch_up_bssDtap_parse_message__LOCATION_UPDATING_REJECT,
	ISUP_call_PASS,
	ISUP_call_FAIL,

	// invalid event counters
	SIGTRAN_upstream_parsing_event_1,
	SIGTRAN_upstream_parsing_event_2,
	SIGTRAN_upstream_parsing_event_3,
	SIGTRAN_upstream_parsing_event_4,
	SIGTRAN_upstream_parsing_event_5,
	SIGTRAN_upstream_parsing_event_6,
	SIGTRAN_upstream_parsing_event_7,
	SIGTRAN_upstream_parsing_event_8,
	SIGTRAN_discard_message,
	SCCP_upstream_parsing_event,
	SCCP_upstream_parsing_event_ref_outbounds,
	SCCP_upstream_parsing_event_ref_notused,
	BSSAP_upstream_parsing_event,
	ISUP_upstream_parsing_event,
	ISUP_upstream_parsing_event_cic_outbounds,
	ISUP_upstream_parsing_event_cic_unequipped,

};


#define MASK_unsigned_int						0xFFFFFFFF

#define MASK_sts_netcs_transport_receive 		0xFFFF
#define MASK_sts_netcs_user_transmit	 		0xFFFF
#define MASK_sts_netcs_fifo_user_transmit		0xFFFF

#define MASK_sts_sccp_dlg_ref					0xFFFF

#define MASK_sts_isup_cic_id					0xFFF
#define INVALID_sts_isup_cic_id					MASK_sts_isup_cic_id



// SAFE may be used for local references of STS counters
#define SAFE_COUNTER_DECLARATION(user, variable) 	unsigned int variable /* ; is left out so that it can be used in function call definition */
#define SAFE_COUNTER_VALUE(user, variable) 			variable
#define SAFE_COUNTER_SET(user, variable, value)		{variable=value&MASK_##user;}
#define SAFE_COUNTER_INCREMENT(user, variable, inc)	{variable=(variable+inc)&MASK_##user;}
#define SAFE_COUNTER_DECREMENT(user, variable, inc)	{variable=((variable<inc)?0:(variable-inc))&MASK_##user;}

#define STS_COUNTER_DECLARATION(user, counter) 		unsigned int counter##_COUNTER; unsigned int counter##_STATISTIC;
#define STS_COUNTER_VALUE(user, counter) 			(counter##_COUNTER)
#define STS_COUNTER_SET(user, counter, value) 		{counter##_COUNTER=value&MASK_##user; counter##_STATISTIC=counter##_COUNTER;}
#define STS_COUNTER_INCREMENT(user, counter, inc)	{counter##_COUNTER=(counter##_COUNTER+inc)&MASK_##user; counter##_STATISTIC=counter##_COUNTER;}
#define STS_COUNTER_DECREMENT(user, counter, inc)	{counter##_COUNTER=((counter##_COUNTER<inc)?0:(counter##_COUNTER-inc))&MASK_##user;}

#define STS_STATISTIC_VALUE(counter)				counter##_STATISTIC
#define STS_STATISTIC_READ(counter)					counter##_STATISTIC; counter##_STATISTIC=0;

// nethelper.h simplifications


// SAFE_MEMAREA_CHECK can be used only in UPSTREAM messages (received from line), DOWNSTREAM is assumed not to be errorneus
#define SAFE_MEMAREA_CHECK(enum_event /*enum eCounter_purposes*/, sig_index/*sigtran index*/, mem_start, mem_inside, mem_end) \
		if (((void *)mem_start>(void *)mem_inside)||(((void *)mem_inside-1)>(void *)mem_end)) { \
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[sigtran_user[sig_index].netcs_index].stats.c[enum_event], 1); \
			FAST6ErrorTraceHandle(1, "SAFE_MEMAREA_CHECK: event-%d, sigtran-%d reported (%d-%d-%d)!", enum_event, sig_index, mem_start, mem_inside, mem_end); \
			return sig_index; \
		}

#define SAFE_SCCP_DIALOG_REF(sig_index/*sigtran index*/, ret_val, DLG_REF_in) \
		if (DLG_REF_in>MASK_sts_sccp_dlg_ref) { \
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[sigtran_user[sig_index].netcs_index].stats.c[SCCP_upstream_parsing_event_ref_outbounds], 1); \
			FAST4ErrorTraceHandle(1, "SAFE_SCCP_DIALOG_REF: event-%d, sigtran-%d, dlg_ref-%d reported !", SCCP_upstream_parsing_event_ref_outbounds, sig_index, DLG_REF_in); \
			return ret_val; \
		} else if (sccp_common_connection_ref.fifo[DLG_REF_in].mask!=DIALOG_REF_IN_PROGRESS) { \
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[sigtran_user[sig_index].netcs_index].stats.c[SCCP_upstream_parsing_event_ref_notused], 1); \
			FAST4ErrorTraceHandle(1, "SAFE_SCCP_DIALOG_REF: event-%d, sigtran-%d, dlg_ref-%d reported !", SCCP_upstream_parsing_event_ref_notused, sig_index, DLG_REF_in); \
			return ret_val; \
		}

#define SAFE_ISUP_CIC_ID(sig_index/*sigtran index*/, ret_val, CIC_ID_in) \
		if (CIC_ID_in>MASK_sts_isup_cic_id) { \
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[sigtran_user[sig_index].netcs_index].stats.c[ISUP_upstream_parsing_event_cic_outbounds], 1); \
			FAST4ErrorTraceHandle(1, "SAFE_ISUP_CIC_ID: event-%d, sigtran-%d, CIC-%d out of boundaries !", ISUP_upstream_parsing_event_cic_outbounds, sig_index, CIC_ID_in); \
			return ret_val; \
		} else if (isup_cic_id->data[CIC_ID_in].device_state==(enum eCicDeviceState) cic_unequipped) { \
			SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[sigtran_user[sig_index].netcs_index].stats.c[ISUP_upstream_parsing_event_cic_unequipped], 1); \
			FAST4ErrorTraceHandle(1, "SAFE_ISUP_CIC_ID: event-%d, sigtran-%d, CIC-%d unequipped !", ISUP_upstream_parsing_event_cic_unequipped, sig_index, CIC_ID_in); \
			return ret_val; \
		}
