
#undef PREDEF
#ifdef NETHELPER
#define PREDEF
#else
#define PREDEF extern
#endif

PREDEF struct user_data
{
  char 			name[20];
  char 			passwd[20];
  char 			station[100];
  char          IP[20];
  int  			port;
  int  			active_socket;
  char 			folder[500];
  char 			file[500];
}User, RemoteUser;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Client-Server interface
// -----------------------------------------------------------------------------

struct NetCS_pthread_data { // pthread can send only one arg in function
	int dummy;
	int socket_id;
	int udp0_tcp1_sctp2;
	int accept_connections;
	int netcs_index;
};
#define PTHREAD_SEND_OVER(val_socket_id, val_udp0_tcp1_sctp2, val_accept_connections, val_netcs_index) \
	NetCS_DataSet[val_netcs_index].trn_status.server.thread.arg.socket_id 			= val_socket_id; \
	NetCS_DataSet[val_netcs_index].trn_status.server.thread.arg.udp0_tcp1_sctp2 	= val_udp0_tcp1_sctp2; \
	NetCS_DataSet[val_netcs_index].trn_status.server.thread.arg.accept_connections 	= val_accept_connections; \
	NetCS_DataSet[val_netcs_index].trn_status.server.thread.arg.netcs_index 		= val_netcs_index;


PREDEF int NetCS_ServerCreateOnPort(struct sockaddr_in *server_bind_data,
									int udp0_tcp1_sctp2,
									int bin0_txt1,
									int netcs_index);
// NetCS_ServerCreateOnPort() creates server on port and select between serving txt
// printouts (storing it in text buffer) or binary stream (storing in pointer page structure).
//
// In case of Text printouts, NetCS_ServerCreateOnPort() creates thread with NetCS_ServerPortReceiveAndStoreBuffer()
PREDEF void *NetCS_Thread_ServerPortReceiveAndStoreBuffer(void *arg);

PREDEF int NetCS_ClientSocketConnect(struct sockaddr_in *remotehost_addr,
									 struct sockaddr_in *localhost_addr,
									 int 	udp0_tcp1_sctp2);

/* dimensioning per application */
/* SCTP/M3UA/ISSUP/BICC application */


#define NETCS_ONE_SET 								0
#define NETCS_MESSAGE_BUFFER_MAXSETS_MASK			0x7
#define NETCS_RECEIVED_PRINTOUT_BUFSIZE 			300

enum ENETCS_Users {
	netcs_sigtran = 1,
	netcs_mml,
	netcs_app_user
};

enum ENETCS_UserTransports {
	udp,
	tcp,
	sctp
};

/* use mask 0xFF00 to discriminate messages and parameters in lower two bytes 0x00FF*/
enum ENETCS_ParsedProtocols {
	fooprotocol		,
	m3ua    		= 0x10,
	m3ua_HEADER		,
	m3ua_PARAMETERS	,
	sccp  	        = 0x20,
	sccp_HEADER		,
	bssap  	        = 0x30,
	bssMap			,
	bssMap_HEADER	,
	bssDtap			,
	bssDtapMM		,
	bssDtap_HEADER  ,
	isup	        = 0x40, // isup received from line
	isup_HEADER
};

enum ENETCS_UserMode {
	unknown,
	client,
	server
};

#define NETCS_BUFFER_AVAILABLE		0
#define NETCS_READLOCK				0x1
#define NETCS_WRITELOCK				0x2
#define NETCS_USERDATAVALID			0x4

// ************************** DATA RECEIVED SIDE ******************************
/* macros

                                                      TRN_REC_W   TRN_REC_R              USR_TR                  FIFO_W   FIFO_R
                                                           ---> | --->                 ---> | --->                 ---> | --->
	macro write actions:                                        |                           |                           |
	NETCS_THREADSAFE_MAINAPP                                    |                           |                           |
		NETCS_RECEIVED_FILL                           TRN_REC_W	|         TRN_REC_BUFF      |                           |
		NETCS_RECEIVED_SET_STATUS                                         TRN_REC_BUFF
		NETCS_RECEIVED_CLR_STATUS                                         TRN_REC_BUFF

	NETCS_THREADSAFE_MAINAPP_L1_THREAD
		NETCS_RECEIVED_READED                                     TRN_REC_R
		NETCS_RECEIVED_CLR_STATUS                                         TRN_REC_BUFF
		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER                                             USR_TR      USR_TR_BUFF
		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO                                                          FIFO_W              FIFO_W_BUFF
		NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO                                               USR_TR_BUFF         FIFO_R    FIFO_R_BUFF

	NETCS_THREADSAFE_MAINAPP_L2_THREAD
		NETCS_USER_TRANSMIT_GET_WRITE_COUNTER                                             USR_TR      USR_TR_BUFF
		NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO                                                          FIFO_W              FIFO_W_BUFF
		NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO                                               USR_TR_BUFF         FIFO_R    FIFO_R_BUFF




	     server Transport thread
	                |
	                v
	          accept(client)
                     * client accepted
	   ------------>|
	   |            v
	   |    NETCS_RECEIVED_GET_WRITE_COUNTER(TRN_REC_W)
       |            * get TRN_REC_BUFF buffer write counter TRN_REC_W, new data is expected to arrive from client
       |              first free buffer is locked with NETCS_WRITELOCK flag and counter value is returned to server
       |              so that server can write incoming client data in it, counter is increased for 1
	   |            |
	   |            v
       |    recv()/sctp_recvmsg()
	   |            * data received
	   |            |
	   |            v
       |    NETCS_RECEIVED_FILL();
	   |            |
	   |            v
       |    NETCS_RECEIVED_CLR_STATUS(NETCS_WRITELOCK);
       |            * NETCS_WRITELOCK flag is removed from TRN_REC_BUFF buffer, meaning that data writing is completed
       |    NETCS_RECEIVED_SET_STATUS(NETCS_USERDATAVALID);
	   |            * NETCS_USERDATAVALID flag is configured, data are valid to be read by Transport User
	   |            |
	   |            v
       |   NETCS_USER_WAKEUP();
	   |            |
	   --------------



	   server Transport User thread
	                |
	                v
	   ------------>|
	   |            v
	   |       TRN_REC_local
       |            * get Transport read counter TRN_REC_R
	   | ---------->|
	   | |          v
       | |  if  NETCS_RECEIVED_DINGDONG()
	   | |          * if Transport set read and write counters are equal
	   | |            NetCS_DataSet[set].transport_receive.write(TRN_REC_W) and NetCS_DataSet[set].transport_receive.read(TRN_REC_R) are sequential values
       | |  and NETCS_RECEIVED_CHECK_STATUS()
	   | |          * and TRN_REC_R counter data are NOT available (NETCS_USERDATAVALID) for reading
	   | |          |
	   | |          v
	   | |          |
	   | |          * Transport User dependent actions
	   | |            - for mml_thread_parse() server there is no action
	   | |          v
	   | |            - for process_sigtran_thread() stay is another loop while THERE ARE messages in sigtran fifo waiting to be sent
	   | |          |------------->|
	   | |          |     if  NETCS_USER_TRANSMIT_FIFO_DINGDONG()
	   | |          |              * Transport User set read and write counters are not equal
	   | |          |                NetCS_DataSet[set].fifo_user_transmit.write(FIFO_W) and NetCS_DataSet[set].fifo_user_transmit.read(FIFO_R) are sequential values
	   | |          |     and NETCS_USER_TRANSMIT_FIFO_R_CHECK_STATUS
	   | |          |              * and FIFO_R counter data are available (NETCS_WRITELOCK) for reading
	   | |          |              |
	   | |          |              .. send USR_TR buffer data with NetCS_SCTPClientSendStringToHostPort()
	   | |          |              |
	   | |          |         NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO
	   | |          |              * POPs USR_TR buffer index from FIFO_R and send it
	   | |          |              * unlock FIFO_R and step it up
	   | |          |              * unlock USR_TR buffer
	   | |          ---------------|
	   | |          |
	   | |          |
	   | |   NETCS_USER_SLEEP()
	   | |          * do not just loop here (and waste processor time) until data are received and are available, rather trigger on
	   | |            Transport set mutex condition (which is triggered from Transport layer receive server)
	   | |          |
	   | -----------|
	   |            v
	   |            |
	   |            * Transport User dependent actions
	   |              - for mml_thread_parse() server it is yymmlparse()
	   |              - for process_sigtran_thread() it is sigtran_parse()
	   |            v
	   |            |
	   |     NETCS_RECEIVED_CLR_STATUS()
	   |            * clear NETCS_USERDATAVALID flag from TRN_REC_BUFF buffer
	   |     NETCS_RECEIVED_READED()
	   |            * TRN_REC_BUFF buffer are readed and processed, increment TRN_REC_R counter
	   |            |
	   --------------



          To send message from Transport User application ...

          NETCS_USER_TRANSMIT_GET_WRITE_COUNTER();
                        * search to get first free available USR_TR buffer (start from last NetCS_DataSet[set].user_transmit.write location)
                        * use pNETCS_USER_TRANSMIT_BUFFER() as pointer to that USR_TR buffer data (&NetCS_DataSet[set].user_transmit.buffer[buffer_write_index].data)
          NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO();
                        * fetch first free FIFO counter from FIFO_W (NetCS_DataSet[set].fifo_user_transmit.write)
                        * PUSHes USR_TR to FIFO_W
                        * push USR_TR in current FIFO_W
                        * lock FIFO_W and step it up


 FIFO_R/FIFO_W and USR_TR works like this:
   - USR_TR structure (user_transmit) contains buffer data storage to be sent (&NetCS_DataSet[set].user_transmit.buffer[buffer_write_index].data)
     and last write location (NetCS_DataSet[set].user_transmit.write)
   - USR_TR structure is not FIFO, rather it is searching through indexes in range <last used for writing, first free>,
     to get first free data storage

   - another structure (fifo_user_transmit) contains only counters for reading and writing, and they are FIFO
   - this structure stores indexes of awaiting to be sent USR_TR buffers, in FIFO manner (so that they can be sent using DINGDONG)



 */

PREDEF pid_t NetCs_Pid_THREADSAFE_MAINAPP;

#ifdef NETHELPER
PREDEF unsigned int NETCS_MACROSAFE_0VALUE_ANY = 0;
#else
PREDEF unsigned int NETCS_MACROSAFE_0VALUE_ANY;
#endif

enum ENETCS_MACROSAFE_IN {
	eNETCS_RECEIVED_GET_WRITE_COUNTER = 0,
	eNETCS_RECEIVED_FILL,
	eNETCS_RECEIVED_SET_STATUS,
	eNETCS_RECEIVED_CLR_STATUS,
	eNETCS_RECEIVED_READED,
	eNETCS_USER_TRANSMIT_GET_WRITE_COUNTER,
	eNETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO,
	eNETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO,
	ePOP_DIALOG_REF,
	ePUSH_DIALOG_REF,
	ePOP_ISUP_CIC_ID,
	ePUSH_ISUP_CIC_ID,
	ePUSH_ISUP_PREDEF_CIC_ID
};

#define NETCS_MACROSAFE_IN(set, id) //preempt_disable(); if (NETCS_MACROSAFE_0VALUE_ANY) NetCS_DataSet[set].stats.NETCS_MACROSAFE_0VALUE[id]++; NETCS_MACROSAFE_0VALUE_ANY=1;
				/*if (NETCS_MACROSAFE_0VALUE_ANY) { int i;\
					for(i=0;i<10;i++) ErrorTraceHandle(1, "NETCS_MACROSAFE_IN: NETCS_MACROSAFE_0VALUE[%d]=%d\n", i, NETCS_MACROSAFE_0VALUE[i]); \
					ErrorTraceHandle(0, "NETCS_MACROSAFE_IN: MACRO call on [%d] preempted one MACRO from above (any=%d)!\n", id, NETCS_MACROSAFE_0VALUE_ANY); }\ */
//				spinlock_t mr_lock = SPIN_LOCK_UNLOCKED; \
//				spin_lock(&mr_lock);

#define NETCS_MACROSAFE_OUT(set, id) //NETCS_MACROSAFE_0VALUE_ANY=0; preempt_enable();
//				spin_unlock(&mr_lock);


// macro helpers
#define STS_COUNTER_VALUE_TRN_REC_R(set)     			STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[set].transport_receive.read)
#define STS_COUNTER_VALUE_TRN_REC_W(set)     			STS_COUNTER_VALUE(sts_netcs_transport_receive, NetCS_DataSet[set].transport_receive.write)
#define STS_COUNTER_INCREMENT_TRN_REC_R(set, n)			STS_COUNTER_INCREMENT(sts_netcs_transport_receive, NetCS_DataSet[set].transport_receive.read, n)
#define STS_COUNTER_SET_TRN_REC_W(set, value) 			STS_COUNTER_SET(sts_netcs_transport_receive, NetCS_DataSet[set].transport_receive.write, value);
#define NETCS_TRN_REC_BUFF_SIZE(set, cnt)       		NetCS_DataSet[set].transport_receive.buffer[cnt].size
#define NETCS_TRN_REC_BUFF_DATA(set, cnt, nbytes)   	NetCS_DataSet[set].transport_receive.buffer[cnt].data[nbytes]
#define NETCS_TRN_REC_BUFF_MASK(set, cnt)				NetCS_DataSet[set].transport_receive.buffer[cnt].mask

#define NETCS_RECEIVED_GET_WRITE_COUNTER(userinfo, set, TRN_REC_W_out) {\
				unsigned int TRN_REC_occupied = 0; \
				SAFE_COUNTER_DECLARATION(sts_netcs_transport_receive, TRN_REC_start); \
				NETCS_MACROSAFE_IN(set, eNETCS_RECEIVED_GET_WRITE_COUNTER); \
				SAFE_COUNTER_SET(sts_netcs_transport_receive, TRN_REC_start, STS_COUNTER_VALUE_TRN_REC_W(set)); \
				FAST4ErrorTraceHandle(2, "NETCS_RECEIVED_GET_WRITE_COUNTER(start): %s SET-%d> TRN_REC-%d\n", \
									  	#userinfo, set, TRN_REC_start); \
				while(NETCS_TRN_REC_BUFF_MASK(set, TRN_REC_start) != NETCS_BUFFER_AVAILABLE) \
					{if (++TRN_REC_occupied>=MASK_sts_netcs_transport_receive) { \
						ErrorTraceHandle(2, "NETCS_RECEIVED_GET_WRITE_COUNTER(start): %s SET-%d> TRN_REC-%d (occ_counted=%d), all locations occupied (TRN_REC_OCC=%d), need to re-dimension size.\n", \
										 #userinfo, set,\
										 TRN_REC_start, \
										 TRN_REC_occupied, \
										 SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_RECEIVED_GET_WRITE_COUNTER_all_occupied])); \
										 TRN_REC_occupied = 0; \
						SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_RECEIVED_GET_WRITE_COUNTER_all_occupied], 1); \
					 } \
					 SAFE_COUNTER_INCREMENT(sts_netcs_transport_receive, TRN_REC_start, 1);} \
				TRN_REC_W_out = TRN_REC_start;\
				NETCS_TRN_REC_BUFF_MASK(set, TRN_REC_start) = NETCS_WRITELOCK;\
				STS_COUNTER_SET_TRN_REC_W(set, TRN_REC_start+1); /* not a DINGDONG dependent, no need to step up, but usefull to make buffer round robin */ \
				FAST5ErrorTraceHandle(2, "NETCS_RECEIVED_GET_WRITE_COUNTER(end):   %s SET-%d> TRN_REC_W-%d selected, TRN_REC_W-%d is next write counter\n", #userinfo, set, TRN_REC_W_out, STS_COUNTER_VALUE_TRN_REC_W(set)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_RECEIVED_GET_WRITE_COUNTER); \
				}

#define NETCS_RECEIVED_FILL(userinfo, set, TRN_REC_W_in, nbytes) { \
				unsigned int netcs_TRN_REC_W_local = TRN_REC_W_in; \
				NETCS_MACROSAFE_IN(set, eNETCS_RECEIVED_FILL); \
				if (nbytes!=0) \
					{ NETCS_TRN_REC_BUFF_SIZE(set, netcs_TRN_REC_W_local) = nbytes; \
					  NETCS_TRN_REC_BUFF_DATA(set, netcs_TRN_REC_W_local, nbytes) = '\0'; } \
				FAST7ErrorTraceHandle(2, "NETCS_RECEIVED_FILL: %s SET-%d> TRN_REC_W-%d set data bytes=%d, TRN_REC_W-%d is next write counter, read is on TRN_REC_R-%d\n", \
						              	     	#userinfo, set, netcs_TRN_REC_W_local, nbytes, \
				                      			STS_COUNTER_VALUE_TRN_REC_W(set), STS_COUNTER_VALUE_TRN_REC_R(set)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_RECEIVED_FILL); \
				}

#define NETCS_RECEIVED_SET_STATUS(userinfo, set, TRN_REC_in, bmask) { \
				unsigned int TRN_REC_mask = 0, netcs_TRN_REC_local = TRN_REC_in; \
				NETCS_MACROSAFE_IN(set, eNETCS_RECEIVED_SET_STATUS); \
				TRN_REC_mask = NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local); \
				NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local) |= bmask; \
				FAST7ErrorTraceHandle(2, "NETCS_RECEIVED_SET_STATUS: %s SET-%d> TRN_REC-%d, mask (0x%x) + %s = 0x%x\n", \
						              		#userinfo, set, netcs_TRN_REC_local, TRN_REC_mask, #bmask, NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_RECEIVED_SET_STATUS); \
				}

#define NETCS_RECEIVED_DINGDONG(set) (STS_COUNTER_VALUE_TRN_REC_W(set) != STS_COUNTER_VALUE_TRN_REC_R(set))
#define NETCS_RECEIVED_CHECK_STATUS(bmask, set, cnt) ((bmask & NETCS_TRN_REC_BUFF_MASK(set, cnt))==bmask)

#define NETCS_RECEIVED_CLR_STATUS(userinfo, set, TRN_REC_in, bmask) { \
				unsigned int TRN_REC_mask = 0, netcs_TRN_REC_local = TRN_REC_in; \
				NETCS_MACROSAFE_IN(set, eNETCS_RECEIVED_CLR_STATUS); \
				TRN_REC_mask = NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local); \
				NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local) = (~bmask)&NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local); \
				FAST7ErrorTraceHandle(2, "NETCS_RECEIVED_CLR_STATUS: %s SET-%d> TRN_REC-%d, mask (0x%x) - %s = 0x%x\n", \
						              #userinfo, set, netcs_TRN_REC_local, TRN_REC_mask, #bmask, NETCS_TRN_REC_BUFF_MASK(set, netcs_TRN_REC_local)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_RECEIVED_CLR_STATUS); \
				}

#define NETCS_RECEIVED_READED(userinfo, set) { \
				NETCS_MACROSAFE_IN(set, eNETCS_RECEIVED_READED); \
				STS_COUNTER_INCREMENT_TRN_REC_R(set, 1); \
				FAST5ErrorTraceHandle(2, "NETCS_RECEIVED_READED: %s SET-%d> TRN_REC_R-%d is next read counter, write is on TRN_REC_W-%d\n", \
						              #userinfo, set, STS_COUNTER_VALUE_TRN_REC_R(set), STS_COUNTER_VALUE_TRN_REC_W(set)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_RECEIVED_READED); \
				}

// ************************** USER TRANSMIT SIDE ******************************
// USER_TRANSMIT data are relying on lower layer (UDP/TCP/SCTP) from TRANSPORT_RECEIVE side which filled NetCS_sConfig,
// thus in this buffers we only write user layer data

/*
#define NETCS_USER_TRANSMIT_FILL(userinfo, set, offset, binary_data, binary_data_bytes, wrcnt) { \
								if ((binary_data_bytes!=0) && ((offset+binary_data_bytes) < NETCS_RECEIVED_PRINTOUT_BUFSIZE)) \
								{ \
									memcpy(&NetCS_DataSet[set].user_transmit.buffer[wrcnt].data[offset], binary_data, binary_data_bytes); \
									if (NetCS_DataSet[set].user_transmit.buffer[wrcnt].size < (offset+binary_data_bytes)) { \
										NetCS_DataSet[set].user_transmit.buffer[wrcnt].size = offset+binary_data_bytes; \
										NetCS_DataSet[set].user_transmit.buffer[wrcnt].data[offset+binary_data_bytes] = '\0'; \
									} \
									ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_FILL: %s SET-%d/buff-%d set data bytes = %d from offset = %d\n", #userinfo, set, wrcnt, binary_data_bytes, offset); \
								} }
*/





// macro helpers
#define STS_COUNTER_VALUE_FIFO_R(set)     			STS_COUNTER_VALUE(sts_netcs_fifo_user_transmit, NetCS_DataSet[set].fifo_user_transmit.read)
#define STS_COUNTER_VALUE_FIFO_W(set)     			STS_COUNTER_VALUE(sts_netcs_fifo_user_transmit, NetCS_DataSet[set].fifo_user_transmit.write)
#define STS_COUNTER_SET_FIFO_R(set, value)			STS_COUNTER_SET(sts_netcs_fifo_user_transmit, NetCS_DataSet[set].fifo_user_transmit.read,  value)
#define STS_COUNTER_SET_FIFO_W(set, value)			STS_COUNTER_SET(sts_netcs_fifo_user_transmit, NetCS_DataSet[set].fifo_user_transmit.write, value)
#define NETCS_FIFO_BUFF_MASK(set, cnt)				NetCS_DataSet[set].fifo_user_transmit.buffer[cnt].mask
#define NETCS_FIFO_BUFF_USR_TR(set, cnt)    		NetCS_DataSet[set].fifo_user_transmit.buffer[cnt].user_transmit_index

#define STS_COUNTER_VALUE_USR_TR(set)     			STS_COUNTER_VALUE(sts_netcs_user_transmit, NetCS_DataSet[set].user_transmit.write)
#define STS_COUNTER_SET_USR_TR(set, value)  		STS_COUNTER_SET(sts_netcs_user_transmit, NetCS_DataSet[set].user_transmit.write, value)
#define NETCS_USR_TR_BUFF_MASK(set, index)			NetCS_DataSet[set].user_transmit.buffer[index].mask

// macros
#define pNETCS_USER_TRANSMIT_BUFFER(userinfo, set, offset, buffer_write_index) &NetCS_DataSet[set].user_transmit.buffer[buffer_write_index].data[offset]

#define NETCS_USER_TRANSMIT_FIFO_DINGDONG(set) (STS_COUNTER_VALUE_FIFO_W(set) != STS_COUNTER_VALUE_FIFO_R(set))
#define NETCS_USER_TRANSMIT_FIFO_R_CHECK_STATUS(bmask, set) ((bmask & NETCS_FIFO_BUFF_MASK(set, STS_COUNTER_VALUE_FIFO_R(set)))==bmask)


/*
	* One NETCS set can be used by more than one thread in case:
		- e.g. more bss_bsc application threads users are on same transport, thus same NETCS
		  Because of this FIFO_R should skip

	Pop and send macro (NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO) is called only from sigtran thread
	while push macro (NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO) may be called from sigtran and application threads.


 */



/*
	NETCS_USER_TRANSMIT_GET_WRITE_COUNTER
	This macro gets first available USR_TR
	* get first free USR_TR
	* lock USR_TR and step it up
*/
// this pops next free data buffer and puts lock on it, and step up data write counter
#define NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(userinfo, set, USR_TR_out) {\
				unsigned int USR_TR_occupied = 0; \
				SAFE_COUNTER_DECLARATION(sts_netcs_user_transmit, USR_TR_start); \
				NETCS_MACROSAFE_IN(set, eNETCS_USER_TRANSMIT_GET_WRITE_COUNTER); \
				SAFE_COUNTER_SET(sts_netcs_user_transmit, USR_TR_start, STS_COUNTER_VALUE_USR_TR(set)); \
				FAST4ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(enter): %s SET-%d> start at USR_TR-%d\n", \
									  #userinfo, set, USR_TR_start); \
				while(NETCS_USR_TR_BUFF_MASK(set, USR_TR_start) != NETCS_BUFFER_AVAILABLE) \
					{if (++USR_TR_occupied>=MASK_sts_netcs_user_transmit) { \
						ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_GET_WRITE_COUNTER: %s SET-%d> USR_TR-%d (occ_counted=%d), all locations occupied (USR_TR_OCC=%d), need to redimensione size.\n", \
										 #userinfo, set,\
										 USR_TR_start, \
										 USR_TR_occupied, \
										 SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied])); \
						USR_TR_occupied = 0; \
						SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_GET_WRITE_COUNTER_all_occupied], 1); \
					 } \
					 SAFE_COUNTER_INCREMENT(sts_netcs_user_transmit, USR_TR_start, 1);} \
				NETCS_USR_TR_BUFF_MASK(set, USR_TR_start) = NETCS_WRITELOCK;\
				STS_COUNTER_SET_USR_TR(set, USR_TR_start+1); /* not a DINGDONG dependent, no need to step up, but useful to make buffer round robin */ \
				USR_TR_out                = USR_TR_start;\
				FAST4ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_GET_WRITE_COUNTER(exit):  %s SET-%d> selected USR_TR-%d\n", #userinfo, set, USR_TR_out); \
				SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied], 1); \
				NETCS_MACROSAFE_OUT(set, eNETCS_USER_TRANSMIT_GET_WRITE_COUNTER); \
				}

/*
	NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO
	This macro PUSHes USR_TR to FIFO_W
	* push USR_TR in current FIFO_W
	* lock FIFO_W and step it up
*/
#define NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO(userinfo, set, USR_TR_in, data_octets) {\
				unsigned int netcs_USR_TR_local = USR_TR_in, FIFO_W_occupied = 0; \
				SAFE_COUNTER_DECLARATION(sts_netcs_fifo_user_transmit, FIFO_W_start); \
				NETCS_MACROSAFE_IN(set, eNETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO); \
				SAFE_COUNTER_SET(sts_netcs_fifo_user_transmit, FIFO_W_start, STS_COUNTER_VALUE_FIFO_W(set)); \
				while(NETCS_FIFO_BUFF_MASK(set, FIFO_W_start) != NETCS_BUFFER_AVAILABLE) \
					{if (++FIFO_W_occupied>=MASK_sts_netcs_fifo_user_transmit) { \
						ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO: %s SET-%d> FIFO_W-%d (occ_counted=%d), all locations occupied, need to re dimension size.\n", \
										 #userinfo, set,\
										 FIFO_W_start, FIFO_W_occupied); \
						FIFO_W_occupied = 0; \
						SAFE_COUNTER_INCREMENT(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO_all_occupied], 1); \
					 } \
					 SAFE_COUNTER_INCREMENT(sts_netcs_fifo_user_transmit, FIFO_W_start, 1);} \
				NETCS_FIFO_BUFF_MASK(set,   FIFO_W_start) = NETCS_WRITELOCK; \
				NETCS_FIFO_BUFF_USR_TR(set, FIFO_W_start) = netcs_USR_TR_local; \
				if (data_octets < NETCS_RECEIVED_PRINTOUT_BUFSIZE) \
					NetCS_DataSet[set].user_transmit.buffer[netcs_USR_TR_local].size = data_octets; \
				else \
					FAST5ErrorTraceHandle(0, "NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO: %s SET-%d> USR_TR-%d data (%d) bigger than max buffer size (%d) !\n", \
										  #userinfo, set, netcs_USR_TR_local, NETCS_RECEIVED_PRINTOUT_BUFSIZE); \
				STS_COUNTER_SET_FIFO_W(set, FIFO_W_start+1); /* dependent on DINGDONG, step it up */ \
				FAST6ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO: %s SET-%d> push USR_TR-%d on FIFO_W-%d (FIFO_R-%d).\n", \
									  #userinfo, set, netcs_USR_TR_local, FIFO_W_start, STS_COUNTER_VALUE_FIFO_R(set)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_USER_TRANSMIT_PUSH_WRITE_COUNTER_TO_FIFO); \
				NETCS_USER_WAKEUP(set); \
				}

/*
	NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO
	This macro POPs USR_TR from FIFO_R and sends it
	* pop USR_TR from current FIFO_R
	* unlock FIFO_R and step it up
	* unlock USR_TR
*/
#define NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO(userinfo, set, USR_TR_in) {\
				unsigned int FIFO_R_local = 0, netcs_USR_TR_local = USR_TR_in; \
				NETCS_MACROSAFE_IN(set, eNETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO); \
				FIFO_R_local = STS_COUNTER_VALUE_FIFO_R(set); \
				NETCS_FIFO_BUFF_MASK(set,   FIFO_R_local) = NETCS_BUFFER_AVAILABLE; \
				NETCS_USR_TR_BUFF_MASK(set, netcs_USR_TR_local) = NETCS_BUFFER_AVAILABLE; \
				STS_COUNTER_SET_FIFO_R(set, FIFO_R_local+1); /* dependent on DINGDONG, step it up */ \
				SAFE_COUNTER_DECREMENT(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied], 1); \
				FAST8ErrorTraceHandle(2, "NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO: %s SET-%d> pop USR_TR_R-%d(mask=%d, occupied=%d) from FIFO_R-%d (FIFO_W-%d).\n", \
									  #userinfo, set, \
									  netcs_USR_TR_local, \
									  NetCS_DataSet[set].user_transmit.buffer[netcs_USR_TR_local].mask, \
									  SAFE_COUNTER_VALUE(unsigned_int, NetCS_DataSet[set].stats.c[NETCS_USER_TRANSMIT_POP_WRITE_COUNTER_all_occupied]), \
									  STS_COUNTER_VALUE_FIFO_R(set), \
									  STS_COUNTER_VALUE_FIFO_W(set)); \
				NETCS_MACROSAFE_OUT(set, eNETCS_USER_TRANSMIT_POP_WRITE_COUNTER_FROM_FIFO); \
				}

#define NETCS_USER_SLEEP(set) 	if (pthread_mutex_lock(&NETCS_MACROSAFE_MUTEX[set]) != 0) { \
										ErrorTraceHandle(0, "NETCS_USER_SLEEP: pthread_mutex_lock(netcs_index=%d).\n", set); \
								} else { \
										FAST2ErrorTraceHandle(3, "NETCS_USER_SLEEP: pthread_mutex_lock(netcs_index=%d).\n", set); \
								} \
								pthread_cond_wait(&NETCS_MACROSAFE_COND[set], &NETCS_MACROSAFE_MUTEX[set]); \
								if (pthread_mutex_unlock(&NETCS_MACROSAFE_MUTEX[set]) != 0) { \
										ErrorTraceHandle(0, "NETCS_USER_SLEEP: pthread_mutex_unlock(netcs_index=%d).\n", set); \
								} else { \
										FAST2ErrorTraceHandle(3, "NETCS_USER_SLEEP: pthread_mutex_unlock(netcs_index=%d).\n", set); \
								}\

#define NETCS_USER_WAKEUP(set)	pthread_cond_signal(&NETCS_MACROSAFE_COND[set]); \
								FAST2ErrorTraceHandle(3, "NETCS_USER_WAKEUP: pthread_cond_signal(netcs_index=%d).\n", set);


// mutex for keep loops sleep until data arrives or needs to be sent
PREDEF pthread_mutex_t NETCS_MACROSAFE_MUTEX[NETCS_MESSAGE_BUFFER_MAXSETS_MASK+1];
PREDEF pthread_cond_t  NETCS_MACROSAFE_COND[NETCS_MESSAGE_BUFFER_MAXSETS_MASK+1];

// *****************************************************************************

// ECASE_TRACE_parsed = true  -> message/parameter recognised, parsed
// ECASE_TRACE_parsed = false -> message/parameter not recognised, discarded
#define	PARSEPREAMBLE(str)	bool ECASE_TRACE_parsed=false, RECV_MSG_TRACE_parsed=false; char case_string[50]; strcpy(case_string, str);

#define ECASE_TRACE(case_string, en)  			case en: FAST4ErrorTraceHandle(2, "%s %s(0x%x)\n", case_string, #en, en); ECASE_TRACE_parsed = true;
#define RECV_MSG_TRACE_case(case_string, en) 	case en: FAST5ErrorTraceHandle(5, "[%9.10s-%d] <------------- %s (0x%.4x) ------------- \n", case_string, time(NULL), #en, en); RECV_MSG_TRACE_parsed = true;
#define SEND_MSG_TRACE(case_string, en)         FAST5ErrorTraceHandle(5, "[%9.10s-%d] -------------- %s (0x%.4x) ------------> \n", case_string, time(NULL), #en, en);

// *****************************************************************************


typedef struct NetCS_sMessageBuffer {

	struct NetCS_sStats {
		SAFE_COUNTER_DECLARATION(unsigned_int, c[30]);
		unsigned int NETCS_MACROSAFE_0VALUE[10];
	} stats;

	struct NetCS_sSendReceive {
		struct NetCS_sSendReceiveBuffer {
			char data[NETCS_RECEIVED_PRINTOUT_BUFSIZE];
			unsigned int  size;
			unsigned char mask;
			// sctp scpecifics
			unsigned char stream_no; // stream on which packet is received (for M3UA loop)
		} buffer[MASK_sts_netcs_transport_receive+1];
		STS_COUNTER_DECLARATION(transport_receive, write);
		STS_COUNTER_DECLARATION(transport_receive, read);
	} transport_receive; /* data received from transport layer TCP/UDP/SCTP */

	struct NetCS_sTransmit {
		struct NetCS_sTransmitBuffer {
			char data[NETCS_RECEIVED_PRINTOUT_BUFSIZE];
			unsigned int  size;
			unsigned char mask;
			// sctp scpecifics
			unsigned char stream_no; // stream on which packet will be sent (for M3UA loop)
		} buffer[MASK_sts_netcs_user_transmit+1];
		STS_COUNTER_DECLARATION(user_transmit, write);
	} user_transmit; /* data stored from upper layer to be transmitted */

	struct NetCS_sTransmit_fifo {
		struct NetCS_sTransmitBuffer_fifo {
			unsigned int user_transmit_index;
			unsigned char mask;
		} buffer[MASK_sts_netcs_fifo_user_transmit+1];
		STS_COUNTER_DECLARATION(fifo_user_transmit, write);
		STS_COUNTER_DECLARATION(fifo_user_transmit, read);
	} fifo_user_transmit; /* data prepared from upper layer to be transmitted */


	struct NetCS_sConfig_Transport_User {
		unsigned char reserved;
		unsigned char udp0_tcp1_sctp2; // ENETCS_UserTransports
		unsigned char mode_undef0_client1_server2;

		// if I'm server then my data are server data, client data are filled in NETCS functions
		// in this server case NETCS_INDEX_KEY() enters only trnloc data and trncon index is dummy #FFFF
		struct NetCS_sConfig_Transport_User_server {
			struct NetCS_sConfig_Transport_User_server_thread {
				struct NetCS_pthread_data arg;
				pthread_t thread_id;
			} thread;
			unsigned int 	socket_id;
			char    		ip[100];
			unsigned int   	port;
			char 			host[100];
			char 			name[20];
			char 			passwd[20];
		} server;

		// if I'm client then my data are client data, server data are configured with commands
		// in this client case NETCS_INDEX_KEY() enters both trnloc and trncon index-es
		struct NetCS_sConfig_Transport_User_client {
			unsigned int 	socket_id;
			char    		ip[100];
			unsigned int  	port_from_NETCS;
			unsigned int  	port_from_command;
			char 			host[100];
			char 			name[20];
			char 			passwd[20];
		} client;
	} trn_status;

	unsigned char user;	// ENETCS_Users
	void *puser_data;	// SSigtran_userof_NetCS_DataSet

} NetCS_MessageBuffer;
PREDEF NetCS_MessageBuffer NetCS_DataSet[NETCS_MESSAGE_BUFFER_MAXSETS_MASK+1];

typedef struct NetCS_sMessageBufferDescriptor {
	unsigned char SetIndex;
	unsigned char SetIndex_is_receive0_transmit1;
} NetCS_MessageBufferDescriptor;

// e.g. NETCS_INDEX(var_to_store_id, sctp, mode, sigtran, trnloc, 0, trncon, 1)
// 									 ^(enum ENETCS_UserTransports)
//                                         ^(enum ENETCS_Users)
// 													^user enums...
#define NETCS_INDEX_KEY(id, transp, mode, key_user, user_key1_name, user_key1_val, user_key2_name, user_key2_val) \
{	int cnt; id = 0xFFFF; \
    FAST6ErrorTraceHandle(2, "NETCS_INDEX_KEY: Search netcs user for %s (%s-%d, %s-%d).\n", \
		                  #transp, #user_key1_name, user_key1_val, #user_key2_name, user_key2_val); \
	for(cnt=0; cnt<NETCS_MESSAGE_BUFFER_MAXSETS_MASK; cnt++) { \
		if ((NetCS_DataSet[cnt].trn_status.reserved 	   	    == 1) && \
				(NetCS_DataSet[cnt].user			    		== key_user) && \
				(NetCS_DataSet[cnt].trn_status.udp0_tcp1_sctp2	== transp) && \
			NETCS_##key_user##_##transp##_USER_INDEX_CMP(cnt, user_key1_val, user_key2_val)) \
			{ id = cnt; break; } \
		} }
#define NETCS_INDEX_NEW(id, transp, mode, key_user, user_key1_name, user_key1_val, user_key2_name, user_key2_val) \
	{ \
		int error_code; \
		int iter=0; id=0; \
		while (1) { \
			if(!NetCS_DataSet[id].trn_status.reserved) break; \
			id=(id+1)&NETCS_MESSAGE_BUFFER_MAXSETS_MASK; iter++; \
			if (iter==NETCS_MESSAGE_BUFFER_MAXSETS_MASK) ErrorTraceHandle(0, "NETCS_INDEX_NEW: Max number (%d) of netcs user's reached. %d\n", NETCS_MESSAGE_BUFFER_MAXSETS_MASK, id); \
		} \
		FAST7ErrorTraceHandle(2, "NETCS_INDEX_NEW: New user on netcs_index-%d on %s (%s-%d, %s-%d).\n", \
				              id, #transp, #user_key1_name, user_key1_val, #user_key2_name, user_key2_val); \
		NetCS_reset_userset(id); \
		NetCS_DataSet[id].trn_status.reserved 		  				= 1; \
		NetCS_DataSet[id].user 										= key_user; \
		NetCS_DataSet[id].trn_status.udp0_tcp1_sctp2 				= transp; \
		NetCS_DataSet[id].trn_status.mode_undef0_client1_server2 	= mode; \
		NETCS_##key_user##_##transp##_USER_INDEX_INIT(id, user_key1_val, user_key2_val); \
	}

#define NETCS_INDEX_RESET_TRANSPORT_DATA(id) \
	{ \
		memset(&NetCS_DataSet[id].transport_receive, 0, sizeof(struct NetCS_sSendReceive)); \
		memset(&NetCS_DataSet[id].user_transmit, 	 0, sizeof(struct NetCS_sTransmit)); \
		memset(&NetCS_DataSet[id].fifo_user_transmit,0, sizeof(struct NetCS_sTransmit_fifo)); \
		memset(&NetCS_DataSet[id].stats,			 0, sizeof(struct NetCS_sStats)); \
	}

// In case of binary stream, NetCS_ServerCreateOnPort() creates thread with NetCS_ServerPortReceiveAndStoreStream()
PREDEF void *NetCS_Thread_ServerPortReceiveAndStoreStream(void *arg);
#define NETCS_RECEIVED_STREAM_BUFSIZE   50000  // i've noticed that max stream for receive data is arround 50000
PREDEF pthread_t NetCS_StreamThread_ID;
// Structure filled by .. containting binary content received on socket stream
typedef struct NetCS_sFileArrayStruct {
	char *p_currentPage;
	long size_currentPage;
	struct NetCS_sFileArrayStruct *p_nextPage;
} NetCS_FileArrayStruct;
PREDEF NetCS_FileArrayStruct *firstPage, *nextPage, *lastPage, *tmpPage;

PREDEF void NetCS_ClientSendStringToHostPort(char *remotehost_ip, int remotehost_port, char (*Cmds)[][MINIBUF]);

PREDEF int  NetCS_SCTPClientSendStringToHostPort(char *remotehost_ip,
										  int  remotehost_port,
										  int  localuser_port,
										  int  connect_new_socket_needed,
										  int  connected_socket_id,
										  char (*Cmds)[][MINIBUF],
										  void *binary,
										  int  binary_length_octets,
										  int  sctp_stream_no,
										  int  txt0_bin1);

PREDEF int NetCS_ServerPortReceiveAndStoreBuffer(int my_sock_sd_server_client,
										  int mask_1screen,
										  int udp0_tcp1_sctp2,
										  int i_am_client0_server1, /* in case of SCTP client side receiving data */
										  int netcs_index,
										  unsigned int TRN_REC_local);

PREDEF void NetCS_SocketConfigHost(char *host_ip, int Port_ID, struct sockaddr_in *host_addr);
PREDEF void NetCS_SocketClose(int sock_id);
PREDEF void NetCS_reset_all_usersets(void);
PREDEF void NetCS_reset_userset(int userset_index);

PREDEF void Net_EthernetSerialProxy_Select(void);
PREDEF void Net_EthernetSerialProxyTest_Select(void);

PREDEF void Net_ScanAllHostPORTs(char *host_name_ip,
								 int host_name_ip_flag,
								 int scan_port_offset);

PREDEF void NetPromptHost_rexec_CmdTool(char *host_name, char *sh_string);
PREDEF void NetPromptHost_shell_CmdTool_with_cmds_and_get_result(char *host_name);
PREDEF void NetPromptHost_daytime(char *host_name);


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Windows Ports
//
// 135 - DCOM (SCM uses udp/tcp to dynamically assign ports for DCOM)
// 139 - Common Internet File System (CIFS)
//
// ANT-20 TCP-IP server (port 5001)

