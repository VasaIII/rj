
#define NETHELPER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/protocols/sigtran.h>
#undef NETHELPER

void NetCS_reset_userset(int userset_index) {

	FAST2ErrorTraceHandle(2, "NetCS_reset_userset(userset_index=%d)\n", userset_index);

	if((userset_index!=0xFFFF) && NetCS_DataSet[userset_index].trn_status.reserved) {
		FAST2ErrorTraceHandle(2, "NetCS_reset_userset(userset_index=%d) Reseted.\n", userset_index);
		if (NetCS_DataSet[userset_index].puser_data != NULL)
			free(NetCS_DataSet[userset_index].puser_data);
		// thread is killed when socket is closed
		// if (NetCS_DataSet[userset_index].trn_status.server.thread.thread_id != NULL)
		//  pthread_kill(NetCS_DataSet[userset_index].trn_status.server.thread.thread_id, 9);
		if (NetCS_DataSet[userset_index].trn_status.server.socket_id != 0)
			NetCS_SocketClose(NetCS_DataSet[userset_index].trn_status.server.socket_id);
		if (NetCS_DataSet[userset_index].trn_status.client.socket_id != 0)
			NetCS_SocketClose(NetCS_DataSet[userset_index].trn_status.client.socket_id);
		memset(&NetCS_DataSet[userset_index], 0, sizeof(struct NetCS_sMessageBuffer));
	}
}

void NetCS_reset_all_usersets(void) {
	int cnt, error_code;

	FAST1ErrorTraceHandle(2, "NetCS_reset_all_usersets()\n");

	for (cnt=0;cnt<NETCS_MESSAGE_BUFFER_MAXSETS_MASK;cnt++) {
		if (NetCS_DataSet[cnt].puser_data != NULL)
			free(NetCS_DataSet[cnt].puser_data);
		memset(&NetCS_DataSet[cnt], 0, sizeof(struct NetCS_sMessageBuffer));

		// keep mutex owned by main application

		// rj: pthread_mutex_lock.c:87: __pthread_mutex_lock: Assertion `mutex->__data.__owner == 0' failed.
		// http://gcc.gnu.org/ml/gcc-bugs/2006-10/msg00805.html
		// http://stackoverflow.com/questions/1105745/pthread-mutex-assertion-error/1105776

		// Mutual Exclusion (Mutex) Locks are used for synchronising threads
		error_code = pthread_mutex_init(&NETCS_MACROSAFE_MUTEX[cnt], NULL);
		error_code = pthread_cond_init (&NETCS_MACROSAFE_COND[cnt],  NULL);
		if (error_code != 0) {
			ErrorTraceHandle(0, "NetCS_reset_all_usersets() mutex or cond failed.\n");
		}

		// http://www.cs.cf.ac.uk/Dave/C/node31.html#SECTION003110000000000000000
		// to unlock the mutex, it must be locked and the calling thread must be the one that last locked the mutex

	}
}


// ***********************************************************************************
// Configure socket's connection peer: IPv4/6, remote IP:PORT
//
// ***********************************************************************************
void NetCS_SocketConfigHost(char *host_ip, int Port_ID, struct sockaddr_in *host_addr)
{
  //FAST3ErrorTraceHandle(2, "NetCS_SocketConfigHost() %s:%d\n", host_ip, Port_ID);

  host_addr->sin_family      = AF_INET;            // Use IPv4
  host_addr->sin_port        = htons(Port_ID);     // Specify PORT
  if (host_ip == NULL)
	  host_addr->sin_addr.s_addr = INADDR_ANY; // INADDR_ANY is a constant hardcoded to 0.0.0.0.
  else
	  host_addr->sin_addr.s_addr = inet_addr(host_ip); // extern socket - host name or IP

  memset(&(host_addr->sin_zero), '\0', 8);
}


// ***********************************************************************************
// Kill socket. Debug info included.
//
// ***********************************************************************************
void NetCS_SocketClose(int sock_id)
{
  shutdown(sock_id, 2);
  close(sock_id);
  FAST2ErrorTraceHandle(2, "Socket terminated <%d>\n", sock_id);
}


int NetCS_ClientSocketConnect(struct sockaddr_in *remotehost_addr,
							  struct sockaddr_in *localhost_addr,
							  int 	 udp0_tcp1_sctp2)
{
	int sock_client;

	FAST3ErrorTraceHandle(2, "NetCS_ClientSocketConnect() Enter with remote %s:%d, ",
					inet_ntoa(remotehost_addr->sin_addr),
					ntohs(remotehost_addr->sin_port));
	FAST4ErrorTraceHandle(2, "local %s:%d, udp0_tcp1_sctp2=%d\n",
					inet_ntoa(localhost_addr->sin_addr),
					ntohs(localhost_addr->sin_port),
					udp0_tcp1_sctp2);

	/* Socket created by socket() are still innocent and they are not yet been polluted by a computer hostname or port number.
	 bind() fleshes socket with this values. Server uses bind() to specify the port at which they will be accepting connections from the clients. */
	if ((sock_client = socket(AF_INET,     /* communications domain - select protocol family IPv4 */
					   SOCK_STREAM, /* type - sequenced, reliable, two-way connection-based byte streams.*/
					   (udp0_tcp1_sctp2==2)?IPPROTO_SCTP:IPPROTO_TCP  /* protocol - to be used with the socket (TCP, UDP...) */
					   )) < 0) {
		FAST1ErrorTraceHandle(1, "NetCS_ClientSocketConnect() Couldn't create socket.\n");

		goto NetCS_ClientSocketConnect_FAIL;
	} else {
		FAST2ErrorTraceHandle(2, "NetCS_ClientSocketConnect(client socket %d) Socket created.\n", sock_client);

		if (udp0_tcp1_sctp2==1) {
			int bOptVal = 1;

			if (setsockopt(sock_client,
						   SOL_SOCKET, /* option level - SOL_SOCKET means to set options at socket level, for example, to indicate that an option
										 is to be set on the TCP protocol level, value level is then set to the TCP protocol number. */
						   SO_REUSEADDR,/* option to set - SO_REUSEADDR means to access option value for enable/disable local address reuse,
										 Message "Address already in use" occures if socket has bind() to some address(ip+port), and even
										 socket has been closed, for a while port is still unusable. With this parameter set, address(ip+port)
										 becomes immediately reusable. Reason for this prolonged unusable state is that socket remains in TIME_WAIT
										 state for certain time to be shure that all the data transfer reach destination succesfully. */
						   (char *) &bOptVal, /* set value */
						   sizeof(int)) < 0) {
				FAST2ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) TCP error setsockopt(SO_REUSEADDR)\n", sock_client);

				goto NetCS_ClientSocketConnect_FAIL;
			}

		} else if (udp0_tcp1_sctp2==2) {
			struct sctp_initmsg initmsg;

			memset(&initmsg, 0, sizeof(initmsg));
			initmsg.sinit_num_ostreams  = 5;
			initmsg.sinit_max_instreams = 5; // maximum 5 streams will be available per socket
			initmsg.sinit_max_attempts  = 4;

			if (setsockopt(sock_client,         // socket
						   IPPROTO_SCTP, //   -> level of configuration ... IP
						   SCTP_INITMSG, //       -> parameter selection at configuration level
						   &initmsg,     //          parameter value
						   sizeof(initmsg)) < 0) {
				FAST2ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) SCTP error setsockopt(SCTP_INITMSG)\n", sock_client);

				goto NetCS_ClientSocketConnect_FAIL;
			}

		}
	}

	//if ((remotehost_ip = gethostbyname(remotehost_name)) == NULL)
	//	FAST2ErrorTraceHandle(0, "NetCS_ClientSocketConnect() - Can't determine host IP from host name <%s>!\n", remotehost_name);
	//remotehost_ip_string = (char *) inet_ntoa(*((struct in_addr *) remotehost_ip->h_addr));
	//NetCS_SocketConfigHost(remotehost_ip_string, remotehost_port, &remotehost_addr);

	/* Before calling connect(), if we didn't call explicitly bind() before,
	 * implicit binding on the local socket is taken care by the stack. */
	if (udp0_tcp1_sctp2==2) { // this is explicit binding to select client port to communicate with server

		FAST2ErrorTraceHandle(2, "NetCS_ClientSocketConnect(client socket %d) SCTP explicit bind to local data.\n", sock_client);

		// explicit binding still doesn't work

		/* If the server is not fussy about the client's port number, then don't try and assign it
		yourself in the client, just let connect() pick it for you.
		If, in a client, you use the naive scheme of starting at a fixed port number and
		calling bind() on consecutive values until it works, then you buy yourself a whole lot of trouble:
		The problem is if the server end of your connection does an active close. (E.G. client sends 'QUIT'
		command to server, server responds by closing the connection). That leaves the client end of the
		connection in CLOSED state, and the server end in TIME_WAIT state. So after the client exits,
		there is no trace of the connection on the client end.
		Now run the client again. It will pick the same port number, since as far as it can see, it's free.
		But as soon as it calls connect(), the server finds that you are trying to duplicate an existing
		connection (although one in TIME_WAIT). It is perfectly entitled to refuse to do this, so you get,
		I suspect, ECONNREFUSED from connect(). (Some systems may sometimes allow the connection anyway,
		but you can't rely on it.)
		This problem is especially dangerous because it doesn't show up unless the client and server are on
		different machines. (If they are the same machine, then the client won't pick the same port number
		as before). So you can get bitten well into the development cycle (if you do what I suspect most
		people do, and test client & server on the same box initially).
		Even if your protocol has the client closing first, there are still ways to produce this problem
		(e.g. kill the server).
		 */

		// create local TCP port and bind sock_client to it so that remote ftp server knows on which IP:port to respond
		if (bind(sock_client,
				(struct sockaddr *) localhost_addr,
				 sizeof(struct sockaddr_in)) < 0) {
			FAST4ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) SCTP Failed to bind local data with error %s (%d).\n", sock_client, strerror(errno), errno);

			goto NetCS_ClientSocketConnect_FAIL;
		} else
			FAST2ErrorTraceHandle(2, "NetCS_ClientSocketConnect(client socket %d) SCTP bind to local data.\n", sock_client);

	}

	/* When calling connect(), if we didn't call explicitly bind() before, implicit binding on the local socket is taken care by the stack. */
	if (connect(sock_client,
			    (struct sockaddr *) remotehost_addr, /* attempt to make a connection to another socket specified by &remotehost_addr */
			     sizeof(struct sockaddr_in)) != 0)
	{
		FAST6ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) Connecting to <%s:%d> failed with %s (%d).\n",
						 sock_client, inet_ntoa(remotehost_addr->sin_addr), ntohs(remotehost_addr->sin_port), strerror(errno), errno);

		goto NetCS_ClientSocketConnect_FAIL;
	} else {
		FAST2ErrorTraceHandle(2, "NetCS_ClientSocketConnect(client socket %d) Connected.\n", sock_client);

		if (udp0_tcp1_sctp2==2) { // subscribe to SCTP
			struct sctp_event_subscribe events;
			struct sctp_status status;
			int in;

			memset((void *)&events, 0, sizeof(events));
			events.sctp_data_io_event = 1; // Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg
			if (setsockopt(sock_client,
					   IPPROTO_SCTP,
					   SCTP_EVENTS,
					   (const void *)&events,
					   sizeof(events)) < 0) {
				FAST1ErrorTraceHandle(1, "NetCS_ClientSocketConnect() error setsockopt(SCTP_EVENTS)");
				return 0;
			}

			/* Read and emit the status of the Socket (optional step) */
			in = sizeof(status);
			if (getsockopt(sock_client,
						 IPPROTO_SCTP,
						 SCTP_STATUS,
						 (void *)&status,
						 (socklen_t *)&in ) < 0) {
				FAST2ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) SCTP error setsockopt(SCTP_STATUS).\n", sock_client);

				goto NetCS_ClientSocketConnect_FAIL;
			}

			FAST6ErrorTraceHandle(2, "NetCS_ClientSocketConnect(client socket %d) SCTP assocId=%d, state=%d, in_strms=%d, out_strms=%d\n",
				  sock_client, status.sstat_assoc_id, status.sstat_state, status.sstat_instrms, status.sstat_outstrms );
		}
	}

	return sock_client;

NetCS_ClientSocketConnect_FAIL:

	FAST2ErrorTraceHandle(1, "NetCS_ClientSocketConnect(client socket %d) FAIL state.\n", sock_client);
	if (sock_client !=0)
		NetCS_SocketClose(sock_client);

	return 0;
}


// ***********************************************************************************
// Create Server on port and select between txt printouts and its storage in buffer
// or stream and its storage in paged structure.
// ***********************************************************************************
int NetCS_ServerCreateOnPort(struct sockaddr_in *server_bind_data, int udp0_tcp1_sctp2, int bin0_txt1, int netcs_index) {
	int my_sock_sd_server = 0;
	struct sockaddr_in my_server_addr, my_server_addr_client;
	unsigned short new_port_id;
	int new_port_id_err_counter = 0;
	int error_code = 0;
	struct sctp_initmsg initmsg;

	FAST6ErrorTraceHandle(2, "NetCS_ServerCreateOnPort() Enter with server's %s:%d, udp0_tcp1_sctp2=%d, bin0_txt1=%d, netcs_index=%d\n",
					inet_ntoa(server_bind_data->sin_addr),
					ntohs(server_bind_data->sin_port),
					udp0_tcp1_sctp2,
					bin0_txt1,
					netcs_index);

	if ((my_sock_sd_server = socket(AF_INET,
								   SOCK_STREAM,
								   (udp0_tcp1_sctp2==2)?IPPROTO_SCTP:0)) == -1) {
		FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort() Server Socket error - socket() for %d.\n", udp0_tcp1_sctp2);
		goto NetCS_ServerCreateOnPort_FAIL;
	} else {
		NetCS_DataSet[netcs_index].trn_status.server.socket_id = my_sock_sd_server;
	}

	FAST2ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Server socket created.\n", my_sock_sd_server);

	if (udp0_tcp1_sctp2 == 1) {
		int yes = 1;

		FAST2ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Configuring TCP SO_REUSEADDR.\n", my_sock_sd_server);

		if (setsockopt(my_sock_sd_server,
						SOL_SOCKET,
						SO_REUSEADDR,
						(char*) &yes,
						sizeof(int)) < 0) {
			FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) error setsockopt\n", my_sock_sd_server);
			goto NetCS_ServerCreateOnPort_FAIL;
		}
	}

	new_port_id = server_bind_data->sin_port;
	while (1) {
		my_server_addr.sin_family 		= AF_INET;
		my_server_addr.sin_port 		= new_port_id;
		my_server_addr.sin_addr.s_addr 	= server_bind_data->sin_addr.s_addr; // INADDR_ANY

		if (!NetCS_DataSet[netcs_index].trn_status.reserved) {
			FAST3ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) netcs_index-%d no longer active.\n", my_sock_sd_server, netcs_index);
			goto NetCS_ServerCreateOnPort_FAIL;
		}

		// create local port and bind my_sock_sd_server to it so that remote server knows on which IP:port to send us data
		if (bind(my_sock_sd_server,
				(struct sockaddr *) &my_server_addr,
				 sizeof(my_server_addr)) < 0) {
			new_port_id_err_counter++;
			new_port_id++;
			FAST3ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Trying to bind on my new (another) port ID=%d.\n", my_sock_sd_server, ntohs(new_port_id));
		} else {
			break;
		}

		if (new_port_id_err_counter > 10) {
			FAST6ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) Server Socket error - bind() failed on %s:%d with %s (%d).\n",
					my_sock_sd_server, inet_ntoa(server_bind_data->sin_addr), ntohs(new_port_id), strerror(errno), errno);
			goto NetCS_ServerCreateOnPort_FAIL;
		}
	}

	if (udp0_tcp1_sctp2 == 2) {

		FAST2ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Configuring SCTP SCTP_INITMSG.\n", my_sock_sd_server);

		memset(&initmsg, 0, sizeof(initmsg));

		/* Specify that a maximum of 5 streams will be available per socket */
		initmsg.sinit_num_ostreams = 5;
		initmsg.sinit_max_instreams = 5;
		initmsg.sinit_max_attempts = 4;

		if (setsockopt(my_sock_sd_server,
					   IPPROTO_SCTP,
					   SCTP_INITMSG,
					   &initmsg,
					   sizeof(initmsg)) < 0) {
			FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) SCTP error setsockopt(SCTP_INITMSG)", my_sock_sd_server);
			goto NetCS_ServerCreateOnPort_FAIL;
		}
	}


	if (listen(my_sock_sd_server,
			   5 /* number of allowed waiting connections which are accepted with accept() */
			   ) == -1) {
		FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) Server socket error - listen().\n", my_sock_sd_server);
		goto NetCS_ServerCreateOnPort_FAIL;
	}

	FAST4ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Server listening on %s:%d\n",
			my_sock_sd_server, inet_ntoa(server_bind_data->sin_addr), ntohs(new_port_id));

	if (bin0_txt1 == 0) {
		// create server for collecting binary streams and storing into linked memory pages

		if (error_code = pthread_create(&NetCS_DataSet[netcs_index].trn_status.server.thread.thread_id,
										NULL,
										NetCS_Thread_ServerPortReceiveAndStoreStream,
										(void *) &my_sock_sd_server) != 0) {
			FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) Stream thread failed to create.\n", my_sock_sd_server);
			goto NetCS_ServerCreateOnPort_FAIL;
		}
	} else {
		// create server for collecting text streams

		// store data in .arg struct to pick it up later in thread
		PTHREAD_SEND_OVER(my_sock_sd_server, udp0_tcp1_sctp2, 1, netcs_index);
		if (error_code = pthread_create(&NetCS_DataSet[netcs_index].trn_status.server.thread.thread_id,
										NULL,
										NetCS_Thread_ServerPortReceiveAndStoreBuffer,
										(void *) &NetCS_DataSet[netcs_index].trn_status.server.thread.arg) != 0) {
			FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) Buffer thread failed to create.\n", my_sock_sd_server);
			goto NetCS_ServerCreateOnPort_FAIL;
		} else {
			FAST4ErrorTraceHandle(2, "NetCS_ServerCreateOnPort(server socket %d) Buffer thread created with id %u for netcs_index-%d !\n",
							my_sock_sd_server,
							NetCS_DataSet[netcs_index].trn_status.server.thread.thread_id,
						    netcs_index);
		}

	}

	return ntohs(new_port_id);

NetCS_ServerCreateOnPort_FAIL:

	FAST2ErrorTraceHandle(1, "NetCS_ServerCreateOnPort(server socket %d) FAIL state.\n", my_sock_sd_server);
	if (my_sock_sd_server !=0)
		NetCS_SocketClose(my_sock_sd_server);

	return 0;
}


// ***********************************************************************************
//
// ***********************************************************************************
void *NetCS_Thread_ServerPortReceiveAndStoreBuffer(void *arg) {

	struct NetCS_pthread_data *preceive_in_thread = (struct NetCS_pthread_data *) arg;
	int netcs_index, nbytes = 0;
	int error_code;
	char socket_type[50];
	int my_sock_sd_server_client = 0;
	struct sockaddr_in my_server_addr_client;
	unsigned int TRN_REC_local;

	netcs_index = preceive_in_thread->netcs_index;

	int addrlen = sizeof(my_server_addr_client);

	// if not accepting new client connections (not called from NetCS_ServerCreateOnPort, )
	//  preceive_in_thread->socket_id should contain client socket created before and passed to thread
	if (preceive_in_thread->accept_connections == 0) {my_sock_sd_server_client = preceive_in_thread->socket_id;}
	// otherwise (called from NetCS_ServerCreateOnPort)
	//  preceive_in_thread->socket_id should contain server socket which will accept new client connections

#ifndef FAST
	sprintf(socket_type, "server socket %d, client socket", preceive_in_thread->socket_id);
	FAST6ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s %d) %s (udp0_tcp1_sctp2=%d), netcs_index-%d\n",
					 socket_type,
					 my_sock_sd_server_client,
					 (preceive_in_thread->accept_connections == 1)?"Socket is real server, accepting connections":"Socket is just serving client socket and receiving responses",
					 preceive_in_thread->udp0_tcp1_sctp2,
					 netcs_index);
#endif

	while (1)
	{

		if (!NetCS_DataSet[netcs_index].trn_status.reserved) {
			FAST2ErrorTraceHandle(1, "NetCS_Thread_ServerPortReceiveAndStoreBuffer() netcs_index-%d no longer active.\n", netcs_index);
			goto NetCS_Thread_ServerPortReceiveAndStoreBuffer_FAIL;
		}

		// in this design, one server can accept only one client, for more clients, maybe
		// fork would be needed after socket connection is accepted, and use of FD_SET
		if (preceive_in_thread->accept_connections == 1) {
			if ((nbytes == 0) || /* socket receiving data */
				(nbytes < 0)) {  /* socket connection reseted by peer */
					if ((my_sock_sd_server_client =
						accept(preceive_in_thread->socket_id,
							   (struct sockaddr *) &my_server_addr_client,
							   &addrlen))
							< 0) {
						NetCS_SocketClose(preceive_in_thread->socket_id);
						FAST4ErrorTraceHandle(1, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s) Server socket error accept(): %s (%d).\n", socket_type, strerror(errno), errno);
						return 0;
					} else {
						// fetch client data, IP:port
						NetCS_DataSet[netcs_index].trn_status.client.port_from_NETCS = my_server_addr_client.sin_port;
						// since this is only for real server, accepted data are for client acccessing this server
						strcpy(NetCS_DataSet[netcs_index].trn_status.client.ip, (char *) inet_ntoa(my_server_addr_client.sin_addr));
						NetCS_DataSet[netcs_index].trn_status.client.socket_id = my_sock_sd_server_client;
						FAST6ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s %d) - Server socket accepted connection from %s:%d (port from command = %d).\n",
										socket_type,
										my_sock_sd_server_client,
										NetCS_DataSet[netcs_index].trn_status.client.ip,
										NetCS_DataSet[netcs_index].trn_status.client.port_from_NETCS,
										NetCS_DataSet[netcs_index].trn_status.client.port_from_command);
					}
			}
		}

		NETCS_RECEIVED_GET_WRITE_COUNTER(transport, netcs_index, TRN_REC_local);

		if ((nbytes = NetCS_ServerPortReceiveAndStoreBuffer(
							my_sock_sd_server_client,
							0,
							preceive_in_thread->udp0_tcp1_sctp2,
							1,
							netcs_index,
							TRN_REC_local)) > 0)
		{
			FAST6ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s %d) stored %d bytes in SET-%d> TRN_REC-%d.\n",
								  socket_type, my_sock_sd_server_client, nbytes, netcs_index, TRN_REC_local);
			NETCS_RECEIVED_FILL(transport, netcs_index, TRN_REC_local, nbytes);
			NETCS_RECEIVED_CLR_STATUS(transport, netcs_index, TRN_REC_local, NETCS_WRITELOCK);
			NETCS_RECEIVED_SET_STATUS(transport, netcs_index, TRN_REC_local, NETCS_USERDATAVALID);
			NETCS_USER_WAKEUP(netcs_index);
		} else {
			NETCS_RECEIVED_SET_STATUS(transport, netcs_index, TRN_REC_local, NETCS_BUFFER_AVAILABLE);

			if (preceive_in_thread->accept_connections == 0) {
				close(my_sock_sd_server_client);
				FAST3ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s %d) closed, since i'm client, by.\n",
						socket_type, my_sock_sd_server_client);
				break;
			}
			/* i'm server - leave server on for new connections */
		}

	}

	FAST3ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreBuffer(%s %d) Exit\n",
			socket_type, my_sock_sd_server_client);

	pthread_exit(NULL);

NetCS_Thread_ServerPortReceiveAndStoreBuffer_FAIL:

	FAST1ErrorTraceHandle(1, "NetCS_Thread_ServerPortReceiveAndStoreBuffer() FAIL state.\n");
	if (preceive_in_thread->socket_id != 0)
		NetCS_SocketClose(preceive_in_thread->socket_id);
	if (my_sock_sd_server_client !=0)
		NetCS_SocketClose(my_sock_sd_server_client);

	static int ret = 1;
	pthread_exit(&ret);
}



// ***********************************************************************************
// Simple receive from socket and store in buffer.
//
// ***********************************************************************************
int NetCS_ServerPortReceiveAndStoreBuffer(int my_sock_sd_server_client,
										  int mask_1screen,
										  int udp0_tcp1_sctp2,
										  int i_am_client0_server1, /* in case of SCTP client side receiving data */
										  int netcs_index,
										  unsigned int TRN_REC_local) {
	int nbytes, error_code;
	struct sctp_sndrcvinfo sndrcvinfo;
	int flags = 0; // initialized flags to 0 for sctp_recvmsg() since new builds suddenly started reporting status "resource temporary unavailable"


	FAST5ErrorTraceHandle(2, "NetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) udp0_tcp1_sctp2=%d, waiting data to be received ...\n",
							netcs_index, TRN_REC_local, my_sock_sd_server_client, udp0_tcp1_sctp2);

	if (udp0_tcp1_sctp2 == 1) {
		nbytes = recv(my_sock_sd_server_client,
					  NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data,
					  NETCS_RECEIVED_PRINTOUT_BUFSIZE,
					  0);

	} else if (udp0_tcp1_sctp2 == 2) {
		nbytes = sctp_recvmsg(my_sock_sd_server_client,
							  (void *) NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data,
							  NETCS_RECEIVED_PRINTOUT_BUFSIZE,
							  (struct sockaddr *)NULL,  // from - pointer to an address, filled in with the sender's address
							  0,						// Size of the buffer associated with the from parameter
							  &sndrcvinfo,				// Pointer to an sctp_sndrcvinfo structure, filled in upon the receipt of the message, if SCTP_EVENTS was enabled prior
							  &flags);					// Message flags such as MSG_CTRUNC, MSG_NOTIFICATION, MSG_EOR

		FAST6ErrorTraceHandle(2, "NetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) ... data received (sinfo_flags = 0x%x, sinfo_stream = %d).\n",
								netcs_index, TRN_REC_local, my_sock_sd_server_client, sndrcvinfo.sinfo_flags, sndrcvinfo.sinfo_stream);
		/*
	     struct sctp_sndrcvinfo {
	             u_int16_t sinfo_stream;  		Stream arriving on
	             u_int16_t sinfo_ssn;     		Stream Sequence Number
	             u_int16_t sinfo_flags;   		Flags on the incoming message
	             u_int32_t sinfo_ppid;    		The ppid field
	             u_int32_t sinfo_context; 		context field
	             u_int32_t sinfo_timetolive; 	not used by sctp_recvmsg
	             u_int32_t sinfo_tsn;        	The transport sequence number
	             u_int32_t sinfo_cumtsn;     	The cumulative acknowledgment point
	             sctp_assoc_t sinfo_assoc_id;	The association id of the peer
	     };
	    */

		// sndrcvinfo.sinfo_flags:
		// 	MSG_UNORDERED = 1,  Send/receive message unordered.
		// 	MSG_ADDR_OVER = 2,  Override the primary destination.
		// 	MSG_ABORT     = 4,  Send an ABORT message to the peer.
	}

	if (nbytes <= 0) {
		if (nbytes == 0) {
			if (i_am_client0_server1 == 1)
				FAST4ErrorTraceHandle(2, "NetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) Remote user hung up.\n", netcs_index, TRN_REC_local, my_sock_sd_server_client);
		} else {
			FAST6ErrorTraceHandle(1, "NetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) Info socket receiving data from client - %s (%d).\n", netcs_index, TRN_REC_local, my_sock_sd_server_client, strerror(errno), errno);
			if (errno == 9) // Bad file descriptor.
				exit;
		}
	} else {
		FAST6ErrorTraceHandle(2, "\nNetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) Received %d bytes (sinfo_stream = %d).\n", netcs_index, TRN_REC_local, my_sock_sd_server_client, nbytes, sndrcvinfo.sinfo_stream);
		NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data[nbytes] = '\0';
		NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].stream_no = sndrcvinfo.sinfo_stream; // fill in stream id for purpose of M3UA loop

		if (mask_1screen & 0x1)
			FAST2ErrorTraceHandle(1, "%s", NetCS_DataSet[netcs_index].transport_receive.buffer[TRN_REC_local].data); // printout result
	}


	FAST4ErrorTraceHandle(2, "NetCS_ServerPortReceiveAndStoreBuffer(SET-%d, TRN_REC_local-%d, accepted client socket %d) Data socket receiving FINISHED !\n", netcs_index, TRN_REC_local, my_sock_sd_server_client);

	return nbytes;
}


// ***********************************************************************************
// Send string to remote host port.
//
// ***********************************************************************************
int  NetCS_SCTPClientSendStringToHostPort(char *remotehost_ip,
										  int  remotehost_port,
										  int  localuser_port,
										  int  connect_new_socket_needed,
										  int  connected_socket_id,
										  char (*Cmds)[][MINIBUF],
										  void *binary,
										  int  binary_length_octets,
										  int  sctp_stream_no,
										  int  txt0_bin1)
{
	char  *pCmds;
	int    pCmds_length;
	int    count;
	struct sockaddr_in remotehost_addr;
	struct sockaddr_in localhost_addr;

	NetCS_SocketConfigHost(remotehost_ip, remotehost_port, &remotehost_addr);
	NetCS_SocketConfigHost(NULL, 		  localuser_port,  &localhost_addr);

	FAST3ErrorTraceHandle(2, "NetCS_SCTPClientSendStringToHostPort() Remote %s:%d, ",
			inet_ntoa(remotehost_addr.sin_addr), ntohs(remotehost_addr.sin_port));
	FAST7ErrorTraceHandle(2, "Local %s:%d, connect_new_socket_needed=%d, connected_socket_id=%d, txt0_bin1=%d, length=%d\n",
			inet_ntoa(localhost_addr.sin_addr),  ntohs(localhost_addr.sin_port),
			connect_new_socket_needed, connected_socket_id, txt0_bin1, binary_length_octets);

	if ((txt0_bin1 == 1)&& (binary_length_octets == 0)) {
		FAST1ErrorTraceHandle(1, "NetCS_SCTPClientSendStringToHostPort() Can't send 0 bytes !\n");
		return 0;
	}

	if (connect_new_socket_needed) {// if already connected
		if ((connected_socket_id = NetCS_ClientSocketConnect(&remotehost_addr, &localhost_addr, 2)) == -1) {
			FAST1ErrorTraceHandle(1, "NetCS_SCTPClientSendStringToHostPort() Server Socket error.\n");
			return 0;
		}
	}

	count = 0;
	while (1)
	{
		if (txt0_bin1 == 0) {
			pCmds = (*Cmds)[count];
			FAST4ErrorTraceHandle(2, "SEND(%d)<%s> >> %s", count, remotehost_ip, pCmds);
			pCmds_length = strlen(pCmds);
		} else {
			int   cnt;
			char *working_binary = (char *) binary;
			pCmds = (char *) binary;
			pCmds_length = binary_length_octets;
			FAST2ErrorTraceHandle(2, "SEND(%d bytes) >> ", pCmds_length);
			for(cnt=0;cnt<pCmds_length; cnt++, working_binary++)
				FAST2ErrorTraceHandle(2, "0x%x ", (*(char *)(working_binary)) & 0xff);
			FAST1ErrorTraceHandle(2, "\n");
		}

		/* ppid = 3 for M3UA */

		if (sctp_sendmsg(connected_socket_id,
						 pCmds,
						 pCmds_length,
						 NULL,    		// const struct sockaddr *to - Destination address of the message (sockaddr)
						 0, 			// socklen_t tolen 			 - length of the Destination address
						 LITTLETOBIG(0x3,4), // uint32_t ppid  		 - application-specified payload protocol identifier
						 0, 			// uint32_t flags
						 sctp_stream_no,// uint16_t stream_no		 - message stream number, for the application to send a message
						 0,				// uint32_t timetolive		 - message time to live in milliseconds
						 0) == -1) {	// uint32_t context			 - value returned when an error occurs in sending a message
			/* TIP: Invalid Stream Identifier - SCTP error (RFC - SCTP)  indicates that "message stream number"
			 * used (by me currently) in sctp_sendmsg() as constant is not used on targeted endpoint.
			 *
			 * TIP: file descriptor in this api is "connected_socket_id" variable
			 *
			 * RFC - SCTP: 6.5 Stream Identifier and Stream Sequence Number
			 */
			NetCS_SocketClose(connected_socket_id);
			FAST3ErrorTraceHandle(1, "NetCS_SCTPClientSendStringToHostPort() sctp_sendmsg() error: %s (%d)\n", strerror(errno), errno);
			return 0;
		}

		if (txt0_bin1 == 0) {
		  count ++; // next command
		  if ((*Cmds)[count][0] == '\0')
			break;
		} else
			break; // only one binary stream

	} // end of command loop

	FAST1ErrorTraceHandle(2, "NetCS_SCTPClientSendStringToHostPort() Exited.\n");

	if (connect_new_socket_needed) // connect was not needed, also disconnect it
		NetCS_SocketClose(connected_socket_id);

	return connected_socket_id;
}


// ***********************************************************************************
// Send string to remote host port.
//
// ***********************************************************************************
void NetCS_ClientSendStringToHostPort(char *remotehost_ip, int remotehost_port, char (*Cmds)[][MINIBUF])
{
	char  *pCmds;
	int    my_sock_sd;
	int    count;
	struct sockaddr_in localhost_addr;
	struct sockaddr_in remotehost_addr;

	NetCS_SocketConfigHost(NULL, 		  0, 			   &localhost_addr);
	NetCS_SocketConfigHost(remotehost_ip, remotehost_port, &remotehost_addr);
	if ((my_sock_sd = NetCS_ClientSocketConnect(&remotehost_addr, &localhost_addr, 1)) == -1) {
		FAST1ErrorTraceHandle(1, "NetCS_ClientSendStringToHostPort() Client socket error.\n");
		return;
	}

	count = 0;
	while (1)
	{
		pCmds = (*Cmds)[count];
		FAST4ErrorTraceHandle(2, "SEND(%d)<%s>: %s", count, remotehost_ip, pCmds);

		if (send(my_sock_sd, pCmds, strlen(pCmds), 0) == -1) {
		  NetCS_SocketClose(my_sock_sd);
		  FAST1ErrorTraceHandle(1, "NetCS_ClientSendStringToHostPort() send() error.\n");
		  return;
		}

	  count ++; // next command
	  if ((*Cmds)[count][0] == '\0')
		break;

	} // end of command loop

	FAST1ErrorTraceHandle(2, "NetCS_ClientSendStringToHostPort() Exited.\n");

	NetCS_SocketClose(my_sock_sd);
}


// ***********************************************************************************
// Create thread, receive from socket and store in memory paged structure.
//
// ***********************************************************************************
void *NetCS_Thread_ServerPortReceiveAndStoreStream(void *arg) {
	char tmp_stream_buffer[NETCS_RECEIVED_STREAM_BUFSIZE]; // for tmp streaming
	int my_sock_sd_server = *(int*)arg;
	struct sockaddr_in my_server_addr_client;
	int my_sock_sd_client;
	int addrlen = sizeof(my_server_addr_client);
	long nbytes, nbytes_total = 0;
	int result_select;
	long pageCounter = 0;
	fd_set master_fd_set; // file descriptor list (use it for checking of data flow on stream sockets)

	FAST1ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreStream()\n");

	FD_ZERO(&master_fd_set); // clear fd set

	// all memory pages allocated (with malloc()) for previous stream shall be released here
	if (firstPage != NULL) // clean previous stream data
	{
		nextPage = firstPage;

		while (1) {
			if ((nextPage != NULL) && (nextPage->p_currentPage == NULL))
				FAST1ErrorTraceHandle(2,"NetCS_Thread_ServerPortReceiveAndStoreStream() - WARNING ! Structure assigned but memory page not allocated, file size probably zero !\n");

			FAST3ErrorTraceHandle(2,"... deallocating memory page <offset:size>=<%d:%d>\n", nextPage->p_currentPage, nextPage->size_currentPage);
			free(nextPage->p_currentPage); // release allocated stream memmory
			FAST1ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreStream() releaseall.1\n");
			tmpPage = nextPage->p_nextPage;
			free((NetCS_FileArrayStruct *) nextPage); // release current structure
			nextPage = tmpPage; // goto next structure
			FAST1ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreStream() releaseall.2\n");
			if (nextPage == NULL) // no more allocated pages, exit
				break;
		}
	}
	FAST1ErrorTraceHandle(2, "NetCS_Thread_ServerPortReceiveAndStoreStream() releaseall.exit\n");

	// start from scratch:
	firstPage = (NetCS_FileArrayStruct *) malloc(sizeof(NetCS_FileArrayStruct));
	firstPage->p_currentPage = NULL;
	firstPage->size_currentPage = 0;
	firstPage->p_nextPage = NULL;

	nextPage = NULL;
	lastPage = NULL;

	// accept() blokira dok se na my_sock_sd_server socketu ne pojavi zahtjev za konekcijom (client zove connect())
	// kad dodje nova konekcija accept() kreira novi socket my_sock_sd_client
	if ((my_sock_sd_client = accept(my_sock_sd_server,
			(struct sockaddr *) &my_server_addr_client, &addrlen)) < 0) {
		NetCS_SocketClose(my_sock_sd_server);
		FAST3ErrorTraceHandle(1,"NetCS_Thread_ServerPortReceiveAndStoreStream() - Server Socket error - accept(). Error: %s (%d).\n", strerror(errno), errno);
		return 0;
	} else {
		int bOptVal = 1;

		if (setsockopt(my_sock_sd_client, SOL_SOCKET, SO_REUSEADDR,
				(char *) &bOptVal, sizeof(int)) < 0) {
			FAST1ErrorTraceHandle(1,"NetCS_Thread_ServerPortReceiveAndStoreStream() error setsockopt()");
			return 0;
		}
	}

	// nakon sta se client connect()ira dodamo socket u fd_set radi selecta() nad njim
	FD_SET(my_sock_sd_client, &master_fd_set);

	FAST2ErrorTraceHandle(2, "FTP Server accepted connection on socket <%d>\n", my_sock_sd_client);

	// vadimo podatke sa socketa
	for (;;) {
		sleep(1);
		if ((nbytes = recv(my_sock_sd_client, tmp_stream_buffer,
				NETCS_RECEIVED_STREAM_BUFSIZE, 0)) <= 0) // I've noticed that approx 50000 bytes max passes througs recv()
		{
			if (nbytes == 0) {
				FAST2ErrorTraceHandle(2, "Socket <%d> hung up.\n", my_sock_sd_client);
				FAST2ErrorTraceHandle(2, "... no more data <totsl size>=<%d>.\n", nbytes_total);
				break;
			} else {
				NetCS_SocketClose(my_sock_sd_client);
				FAST3ErrorTraceHandle(1,"NetCS_Thread_ServerPortReceiveAndStoreStream() - Receiving data from client - error %s (%d).\n", strerror(errno), errno);
				return 0;
			}
		} else // nbytes > 0 ... there are data
		{
			nbytes_total = nbytes_total + nbytes;

			if (pageCounter == 0) {
				firstPage->p_currentPage = (char *) malloc(nbytes);
				firstPage->size_currentPage = nbytes;
				firstPage->p_nextPage = NULL;

				memcpy(firstPage->p_currentPage, tmp_stream_buffer, nbytes);

				FAST4ErrorTraceHandle(2,"\nAllocated first memory page <offset:size:total>=<%d:%d:%d>.\n", firstPage->p_currentPage, nbytes, nbytes_total);

				nextPage = firstPage;
				lastPage = firstPage;

				pageCounter++;
			} else {
				nextPage->p_nextPage = (NetCS_FileArrayStruct *) malloc(
						sizeof(NetCS_FileArrayStruct));
				nextPage = nextPage->p_nextPage;

				nextPage->p_currentPage = (char *) malloc(nbytes);
				nextPage->size_currentPage = nbytes;
				nextPage->p_nextPage = NULL;

				memcpy(nextPage->p_currentPage, tmp_stream_buffer, nbytes);

				FAST4ErrorTraceHandle(2,"Allocated next memory page <offset:size:total>=<%d:%d:%d>.\n",nextPage->p_currentPage, nbytes, nbytes_total);

				lastPage = nextPage;

				pageCounter++;
			}

			/* select() checks which of the fd's in set is ready for reading, ready for writing or has an error condition pending.
			 If the specified condition to check is false FOR ALL of the fd's in set, select() blocks up to the specified timeout
			 interval until the specified condition to check is true FOR AT LEAST one of the fd's in set. */
			// FD_SETSIZE - number of sockets in set (not using in here).
			if (FD_ISSET(my_sock_sd_client, &master_fd_set)) // if socket in set (for each case)
			{
				if ((result_select = select(FD_SETSIZE, &master_fd_set, // readfds
						NULL, // writefds
						NULL, // exceptfds
						NULL // timeout
						)) < 0) // scan search_fd_set for reading
				{
					FAST1ErrorTraceHandle(1, "NetCS_Thread_ServerPortReceiveAndStoreStream() - select() error.\n");
					return 0;
				}
				else if (result_select == 0) // no more data to add
				{
					FAST2ErrorTraceHandle(2, "\nNo more data. Received %d bytes.\n", nbytes_total);
					break;
				}
				else // more data waiting
				{
					//FAST1ErrorTraceHandle(1, "#");
					fprintf(stdout, "#");
					continue;
				}
			}
			//printf("BUFFER result:\n%s\n",nextPage->p_currentPage);
		}
	}

	FD_CLR(my_sock_sd_client, &master_fd_set);
	// remove socket from set
	FD_ZERO(&master_fd_set); // clear fd set

	NetCS_SocketClose(my_sock_sd_client);
	NetCS_SocketClose(my_sock_sd_server);

	FAST1ErrorTraceHandle(1, "\n");
	FAST1ErrorTraceHandle(2, "Thread FINISHED, data RECIEVED..\n");

	pthread_exit(NULL);

	return NULL;
}


// ***********************************************************************************
// Proxy between I/O from Ethernet and Serial.
//
// ***********************************************************************************
void Net_EthernetSerialProxy_Select(void)
{
	int   port_to_accept_data;
	long  nbytes, nbytes_added;

	int my_sock_sd_server, my_sock_sd_server_client;
	struct sockaddr_in my_server_addr, my_server_addr_client;
	int  addrlen = sizeof(my_server_addr_client);

	struct termios ttyterm;

	char plain_string[1000];

	// bez addrlen = sizeof(my_server_addr_client) je prijavljiva error na accept:
	// Transport endpoint is not connected
	// !!! ERROR !!! termserv - Server Socket error - accept(). Error: 134.

	char tmp_buffer_read_port[10000];
	char *ptr, c;
	char tmp_buffer_read_serial[1024];

	int count, i, first, status, open_state, tmp;

	FILE *ttyds;

	if ((RemoteUser.station[0] == '\0') || (plain_string[0] == '\0')) {
		FAST1ErrorTraceHandle(1, "\nRemote station name or ip address is missing !\n");
		return;
	}

	port_to_accept_data = RemoteUser.port;

	// *****************************************************************
	// prepare tty
	//

	if ((ttyds = fopen(plain_string, "rw+")) < 0) {
		FAST2ErrorTraceHandle(1, "termserv - fopen() error, can't open %s.\n", plain_string);
		return;
	}

	//if (isatty(ttyds) == 0) {
	// FAST2ErrorTraceHandle(1, "Device <%s> is not tty.\n", plain_string);
	// return 0; }

	setbuf(ttyds, NULL);

	tcgetattr(fileno(ttyds), &ttyterm);

	// Set 8 bit characters, no parity, 1 stop (8N1)
	ttyterm.c_cflag &= ~PARENB;        // Clear previous parity bits values
	ttyterm.c_cflag &= ~CSTOPB;        // Clear previous stop bits values
	ttyterm.c_cflag &= ~CSIZE;         // Clear previous charsize bits values
	ttyterm.c_cflag |= CS8;            // Set for 8 bits.

	ttyterm.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

	tcsetattr(fileno(ttyds), TCSAFLUSH, &ttyterm);

	//
	// *****************************************************************

	ErrorTraceHandle(1, "\n\n-----------------------------------\n");
	ErrorTraceHandle(1,     "---> Terminal Server Split :-) <---\n");
	ErrorTraceHandle(1,     "-----------------------------------\n");

	if ((my_sock_sd_server = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		NetCS_SocketClose(my_sock_sd_server);
		FAST1ErrorTraceHandle(1, "termserv - socket().\n");
		return;
	}

	FAST1ErrorTraceHandle(2, "Socket created.\n");

	bzero((char *)&my_server_addr,sizeof(my_server_addr));
	bzero((char *)&my_server_addr_client,sizeof(my_server_addr_client));
	my_server_addr.sin_family      = AF_INET;
	my_server_addr.sin_port        = htons(port_to_accept_data);
	my_server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(my_server_addr.sin_zero, '\0', 8);

	if (bind(my_sock_sd_server, (struct sockaddr *)&my_server_addr, sizeof(my_server_addr)) < 0)
	{
		FAST2ErrorTraceHandle(2, "termserv - Trying to bind on port ID <%d> failed.\n", port_to_accept_data);
	}

	FAST1ErrorTraceHandle(2, "Socket binded to port.\n");

	if (listen(my_sock_sd_server, 5) == -1) {
		NetCS_SocketClose(my_sock_sd_server);
		FAST1ErrorTraceHandle(1, "termserv - Server Socket error - listen().\n");
		return;
	}

	if ((my_sock_sd_server_client = accept(my_sock_sd_server, (struct sockaddr *) &my_server_addr_client, &addrlen)) < 0) {
		NetCS_SocketClose(my_sock_sd_server);
		FAST3ErrorTraceHandle(1, "termserv - Server Socket error - accept(). Error: %s (%d).\n", strerror(errno), errno);
		return;
	}

	while(1)
	{
		FAST1ErrorTraceHandle(2, "PART FOR READING FROM SOCKET AND SENDING TO SERIAL\n");

		//sleep(1);
		nbytes = recv(my_sock_sd_server_client, tmp_buffer_read_port, 10000, 0);
		strcat(tmp_buffer_read_port, "\r\n");
		nbytes_added = strlen(tmp_buffer_read_port);

		if(nbytes < 0) {
			NetCS_SocketClose(my_sock_sd_server_client);
			NetCS_SocketClose(my_sock_sd_server);
			FAST3ErrorTraceHandle(1, "termserv - Receiving data from client - error %s (%d).\n", strerror(errno), errno);
			return;
		}
		else if (nbytes > 0) // nbytes > 0 ... there are data
		{
			ptr = tmp_buffer_read_port;

			printf("\n>>>");
			while (ptr != (tmp_buffer_read_port + nbytes_added))
			{
				putc(*ptr, stdout);
				putc(*ptr++, ttyds);
			}
			printf("\n");

			FAST3ErrorTraceHandle(1, "inPORToutSERIAL[%d bytes]>>%s\n", nbytes_added, tmp_buffer_read_port);
		}

		FAST1ErrorTraceHandle(2, "PART FOR POOLING FROM SERIAL\n");

		count = 0;
		while((c=getc(ttyds)) != EOF)
			tmp_buffer_read_serial[count++] = c;

		if (count != 0)
			FAST3ErrorTraceHandle(1, "inSERIALoutPORT[%d bytes]>>%s\n", count, tmp_buffer_read_serial);

	} // end of while

	if (fclose(ttyds)) {
		NetCS_SocketClose(my_sock_sd_server);
		NetCS_SocketClose(my_sock_sd_server_client);
		FAST1ErrorTraceHandle(0, "termserv - closing serial failed\n");
		return;
	}

	NetCS_SocketClose(my_sock_sd_server);
	NetCS_SocketClose(my_sock_sd_server_client);

}


// ***********************************************************************************
// Scans all ports on given host name / IP address
//
// ***********************************************************************************
void Net_ScanAllHostPORTs(char *host_name_ip,
						  int host_name_ip_flag,  // name = 1 ; ip = 0
						  int scan_port_offset)
{
  struct servent *sp;
  struct sockaddr_in Host_tmp_Addr; // My_tmp_Addr
  struct hostent *user_ip;
  char  *host_ip, host_ip_store[20];
  int    My_tmp_Sock;
  int    port;

  if (host_name_ip_flag) // host name
	{
		// Get adress from name server
		if (gethostbyname(host_name_ip) == NULL) {
			FAST2ErrorTraceHandle(1, "ScanAllHostPORTs - Cant get host data by name %s", host_name_ip);
			return;
		} else {
			user_ip = gethostbyname(host_name_ip);
			host_ip = (char *) inet_ntoa(*((struct in_addr *)user_ip->h_addr));
			FAST3ErrorTraceHandle(1, "Name: <%s>\nIP:   <%s>\n\n", user_ip->h_name, host_ip);
		}
	} else // host ip
	{
		strcpy(host_ip_store, host_name_ip);
		host_ip = (char *) host_ip_store;
		FAST2ErrorTraceHandle(1, "IP:   <%s>\n\n", host_ip);
	}

	// Create local socket
	if ((My_tmp_Sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		FAST1ErrorTraceHandle(0, "ScanAllHostPORTs - Server Socket error.\n");
		return;
	}

	// My_tmp_Addr->sin_family      = AF_INET;
	// My_tmp_Addr->sin_port        = htons(Port_ID);
	// My_tmp_Addr->sin_addr.s_addr = htonl(INADDR_ANY);
	// if (bind(My_tmp_Sock, (struct sockaddr *)&My_tmp_Addr, sizeof(My_tmp_Addr)) == -1) // bind temporary sock
	// {  FAST2ErrorTraceHandle(1, "Socket error. %s.\n", strerror(errno)); return; }

	FAST2ErrorTraceHandle(1, "Following services on <%s> are supported:\n", host_ip);

	for (port = scan_port_offset; port <= 65536; port++) {
		// Get extern Host_tmp_Addr struct
		NetCS_SocketConfigHost(host_ip, port, &Host_tmp_Addr);
		if (connect(My_tmp_Sock, (struct sockaddr *) &Host_tmp_Addr, sizeof(Host_tmp_Addr)) == 0) {
			if ((sp = getservbyport(port, "tcp")) != NULL)
				{FAST3ErrorTraceHandle(1, "Service on Port %6d (%s)\n", port, sp->s_name);}
			else if ((sp = getservbyport(port, "udp")) != NULL)
				{FAST3ErrorTraceHandle(1, "Service on Port %6d (%s)\n", port, sp->s_name);}
			else
				{FAST2ErrorTraceHandle(1, "Service on Port %6d\n", port);}
			if ((My_tmp_Sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				{FAST1ErrorTraceHandle(0, "ScanAllHostPORTs() - Server Socket error.\n");}
				return;
			}
		} else {
			//FAST4ErrorTraceHandle(2, "Port %d report: %s (%d)\n.", port, strerror(errno), errno);
		}
	}

  NetCS_SocketClose(My_tmp_Sock);
}



// ***********************************************************************************
// Exec cmd on remote host
//
// ***********************************************************************************
void NetPromptHost_rexec_CmdTool(char *host_name, char *sh_string)
{
  struct servent *server_service;
  int sock_stream;
  struct hostent *user_ip;
  char **pnt;

  // in.rexecd(1M) is the server for  the  rexec(3N)  routine
  server_service = getservbyname("exec", "tcp");

  user_ip = gethostbyname(User.station);
  pnt     = &host_name;

  FAST4ErrorTraceHandle(2, "<%s> --> <%s>\n%s\n\n", user_ip->h_name, *pnt, sh_string);
  if ((sock_stream = rexec(pnt, server_service->s_port, User.name, User.passwd, sh_string, 0)) == -1)  {
    FAST2ErrorTraceHandle(0, "Command failed on host <%s> (maybe rexec() port 512 is not enabled).\n", host_name);
    return;
  }

}







#if 0





void Net_EthernetSerialProxyTest_Select(void)
{
	  int   port_to_accept_data;
	  long  nbytes;

	  int my_sock_sd_server, my_sock_sd_server_client;
	  struct sockaddr_in my_server_addr, my_server_addr_client;
	  int  addrlen = sizeof(my_server_addr_client);
	  // bez addrlen = sizeof(my_server_addr_client) je prijavljiva error na accept:
	  // Transport endpoint is not connected
	  // !!! ERROR !!! termserv - Server Socket error - accept(). Error: 134.

	  // isatty(), ttyname()

	  char tmp_buffer_read_port[1024];
	  char tmp_buffer_read_serial[1024];

	  char plain_string[1000];

	  struct pollfd pollfds[1];
	  /*
	    struct pollfd {
	    int fd;             file descriptor
	    short events;       requested events
	    short revents;      returned events
	    }
	  */
	  int count, first, status, open_state;

	  if ((RemoteUser.station[0] == '\0') || (RemoteUser.port == 0) || (plain_string[0] == '\0'))
	    {
	      ErrorTraceHandle(0, "\nMissing data !\n");
	    }

	  port_to_accept_data = RemoteUser.port;

	  if ((pollfds[0].fd = open(plain_string, O_RDWR|O_NONBLOCK)) < 0)
	    {
	      ErrorTraceHandle(0, "termserv - open() error, can't open %s.\n", plain_string);
	    }
	  else
	    {
	      pollfds[0].events  = POLLIN; // | POLLPRI | POLLRDNORM;  // set events to poll
	      pollfds[0].revents = 0;
	    }

	  ErrorTraceHandle(1, "\n\n-----------------------------------\n");
	  ErrorTraceHandle(1,     "---> Terminal Server Split :-) <---\n");
	  ErrorTraceHandle(1,     "-----------------------------------\n");

	  if ((my_sock_sd_server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	    {
	      NetCS_SocketClose(my_sock_sd_server);
	      ErrorTraceHandle(0, "termserv - socket().\n");
	    }

	  ErrorTraceHandle(2, "Socket created.\n");

	  bzero((char *)&my_server_addr,sizeof(my_server_addr));
	  bzero((char *)&my_server_addr_client,sizeof(my_server_addr_client));
	  my_server_addr.sin_family      = AF_INET;
	  my_server_addr.sin_port        = htons(port_to_accept_data);
	  my_server_addr.sin_addr.s_addr = INADDR_ANY;
	  memset(my_server_addr.sin_zero, '\0', 8);

	  if (bind(my_sock_sd_server, (struct sockaddr *)&my_server_addr, sizeof(my_server_addr)) < 0)
	    {
	      ErrorTraceHandle(2, "termserv - Trying to bind on port ID <%d> failed.\n", port_to_accept_data);
	    }

	  ErrorTraceHandle(2, "Socket binded to port.\n");

	  if (listen(my_sock_sd_server, 5) == -1)
	    {
	      NetCS_SocketClose(my_sock_sd_server);
	      ErrorTraceHandle(0, "termserv - Server Socket error - listen().\n");
	    }

	  if ((my_sock_sd_server_client = accept(my_sock_sd_server, (struct sockaddr *) &my_server_addr_client, &addrlen)) < 0)
	    {
	      NetCS_SocketClose(my_sock_sd_server);
	      ErrorTraceHandle(0, "termserv - Server Socket error - accept(). Error: %d.\n", errno);
	    }

	  while(1)
	    {
	      // PART FOR READING FROM SOCKET AND SENDING TO SERIAL
	      sleep(1);
	      if((nbytes = recv(my_sock_sd_server_client, tmp_buffer_read_port, 1024, 0)) <= 0)
		{
		  if(nbytes < 0)
		    {
		      NetCS_SocketClose(my_sock_sd_server_client);
		      NetCS_SocketClose(my_sock_sd_server);
		      ErrorTraceHandle(0, "termserv - Receiving data from client - error. %s.\n", strerror(errno));
		    }
		}
	      else  // nbytes > 0 ... there are data
		{
		  tmp_buffer_read_port[nbytes]='\0';
		  if (write(pollfds[0].fd, tmp_buffer_read_port, nbytes) != nbytes)
		    ErrorTraceHandle(0, "termserv - some data on serial write lost\n");
		  ErrorTraceHandle(1, "PORT>>\n%s", tmp_buffer_read_port);
		}
	      // PART FOR POOLING FROM SERIAL
	      open_state = 1;
	      first = 1;
	      do
		{
		  pollfds[0].revents = 0;
		  if ((status = poll(pollfds, 1, -1)) < 0)
		    {
		      NetCS_SocketClose(my_sock_sd_server_client);
		      NetCS_SocketClose(my_sock_sd_server);
		      ErrorTraceHandle(0, "termserv - poll failed. %s.\n", strerror(errno));
		    }

		  //ErrorTraceHandle(1, "NEW SERIAL>>\n");
		  //ErrorTraceHandle(1, "STATUS (%d,%d)", status,pollfds[0].revents);

		  if (status == 0)
		    open_state = 0;
		  else
		    {
		      switch (pollfds[0].revents) {
		      default: /* default error case */
			NetCS_SocketClose(my_sock_sd_server_client);
			NetCS_SocketClose(my_sock_sd_server);
			ErrorTraceHandle(0, "termserv - error event !");
			break;
		      case POLLIN:
			/*echo incoming data on "other" Stream*/
			while ((count=read(pollfds[0].fd, tmp_buffer_read_serial, 1024)) > 2)
			  {
			    if (first)
			      {
				first = 0;
				ErrorTraceHandle(1, "SERIAL>>\n");
			      }
			    tmp_buffer_read_serial[count] = '\0';
			    ErrorTraceHandle(1, "%s", tmp_buffer_read_serial);

			    if ((!strncmp(tmp_buffer_read_serial, "$ ", 2)) ||
				 (count == 0))
			    {
			      open_state = 0;
			      break;
			    }
			  }
			// write loses data if flow control prevents the transmit at this time
			break;
		      case 0: // no events
			open_state = 0;
			break;
		      }
		    }
		}
	      while(open_state); // end of do

	      nbytes = 0;

	    } // end of while

	  if (close(pollfds[0].fd))
	    {
	      NetCS_SocketClose(my_sock_sd_server);
	      NetCS_SocketClose(my_sock_sd_server_client);
	      ErrorTraceHandle(0, "termserv - closing serial failed\n");
	    }

	  NetCS_SocketClose(my_sock_sd_server);
	  NetCS_SocketClose(my_sock_sd_server_client);

}


void NetPromptHost_daytime(char *host_name)
{
  struct servent *server_service;
  struct hostent *tmp_ip;
  struct sockaddr_in host_addr;
  int    my_sock_sd;
  char  *host_ip;

  tmp_ip  = gethostbyname(host_name);
  host_ip = (char *)inet_ntoa(*((struct in_addr *)tmp_ip->h_addr));
  server_service = getservbyname("daytime", "tcp");

  ErrorTraceHandle(1, "DayTime Server on port %d.\n", htons(server_service->s_port));

  if ((my_sock_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    ErrorTraceHandle(0, "NetPromptHost_daytime() - Server Socket error. %s.\n");
  NetCS_SocketConfigHost(host_ip, ntohs(server_service->s_port), &host_addr);

  if (connect(my_sock_sd, (struct sockaddr *)&host_addr, sizeof(host_addr)) == 0 )
    {
      NetCS_ServerPortReceiveAndStoreBuffer(my_sock_sd, 1, 1, 0, 0, 0);
    }

  NetCS_SocketClose(my_sock_sd);
}

// ***********************************************************************************
//
//  NE RADI !!! neznan kako exec/shell protocol funkcionira ???
//
// ***********************************************************************************
void NetPromptHost_shell_CmdTool_with_cmds_and_get_result(char *host_name)
{
  char Cmds[][MINIBUF] = {
    "setenv DISPLAY vanaheim:0.0;cmdtool -I 'ps -ef' -Ws 1000 200"};
  char buf[255];

  struct servent *server_service;
  struct hostent *tmp_ip;
  struct sockaddr_in host_addr;
  int    my_sock_sd;
  char  *host_ip;

  pid_t  pid;

  int    nbytes;

  // in.rshd is the server for the rsh(1) program (remote shell server)

  if ((my_sock_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - Server Socket error. %s.\n");

  ErrorTraceHandle(2, "Socket <%d> created ...\n", my_sock_sd);

  tmp_ip  = gethostbyname(host_name);
  host_ip = (char *)inet_ntoa(*((struct in_addr *)tmp_ip->h_addr));

  if ((server_service = getservbyname("exec", "tcp")) == NULL)
    {
      NetCS_SocketClose(my_sock_sd);
      ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - getservbyname() error.\n");
    }

  NetCS_SocketConfigHost(host_ip, server_service->s_port, &host_addr);

  if (connect(my_sock_sd, (struct sockaddr *)&host_addr, sizeof(host_addr)) == 0 )
    {
      strcpy(buf, "/bin/ps -ef\0" ); // PORT for returning stderr stream
      if (send(my_sock_sd, buf, strlen(buf), 0) == -1)
	{
	  NetCS_SocketClose(my_sock_sd);
	  ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - send() error.\n");
	}


//	NetSockHandle_MyServerOnPort(6085);


      strcpy(buf, "etkfrbi\0" );
      if (send(my_sock_sd, buf, strlen(buf), 0) == -1)
	{
	  NetCS_SocketClose(my_sock_sd);
	  ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - send() error.\n");
	}

      strcpy(buf, "bija04\0" );
      if (send(my_sock_sd, buf, strlen(buf), 0) == -1)
	{
	  NetCS_SocketClose(my_sock_sd);
	  ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - send() error.\n");
	}

      strcpy(buf, "/bin/ps -ef\0" );
      if (send(my_sock_sd, buf, strlen(buf), 0) == -1)
	{
	  NetCS_SocketClose(my_sock_sd);
	  ErrorTraceHandle(0, "NetPromptHost_shell_CmdTool_with_cmds_and_get_result() - send() error.\n");
	}

      NetCS_ServerPortReceiveAndStoreBuffer(my_sock_sd, 1, 1, 0, 0, 0);

    }
  NetCS_SocketClose(my_sock_sd);
}

#endif
