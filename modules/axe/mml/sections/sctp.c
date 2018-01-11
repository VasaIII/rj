
#define AXE_MMLPARSER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#undef AXE_MMLPARSER

/*
 *
 *
 * 		***********************************  TRNLOC *****************************************
 *
 *
 */

void mml_trnloc(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int trnloc_index  	= mml.active_cmd_par_index;
	int status 		 	= mml.Ctrnloc.par[trnloc_index].status;
	int trncon_index 	= 0xFFFF; // dummy
	int m3acon_index 	= 0xFFFF; // dummy
	int netcs_index 	= 0xFFFF;

	ErrorTraceHandle(2, "mml_trnloc(trnloc-%d) status=%d.\n", trnloc_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{
					unsigned int locportnum, tmp_port;
					struct sockaddr_in localserver_addr;

					if (!strncmp(mml.Ctrnloc.par[trnloc_index].Pmode, "server", 6))	{

						// ********************************************************************************
						if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "udp")) 		{NETCS_INDEX_KEY(netcs_index, udp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "tcp")) 	{NETCS_INDEX_KEY(netcs_index, tcp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else 															{NETCS_INDEX_KEY(netcs_index, sctp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						if (netcs_index!=0xFFFF) { // not allowed parameter update for trncon
							mml_send_line("Transport layer already active.");
							MML_IGNORED;
						}
						if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "udp")) 		{NETCS_INDEX_NEW(netcs_index, udp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "tcp")) 	{NETCS_INDEX_NEW(netcs_index, tcp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else 															{NETCS_INDEX_NEW(netcs_index, sctp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						// ********************************************************************************

						ErrorTraceHandle(2, "mml_trnloc() Server mode\n");
						NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 	= 2;
						strcpy(NetCS_DataSet[netcs_index].trn_status.server.ip,   			mml.Ctrnloc.par[trnloc_index].Plip);
						NetCS_DataSet[netcs_index].trn_status.server.port 					= atoi(mml.Ctrnloc.par[trnloc_index].Plpn);
						// get locportnum from trnloc
						locportnum = NetCS_DataSet[netcs_index].trn_status.server.port;

						ErrorTraceHandle(2, "mml_trnloc() netcs_index-%d: trnloc-%d\n",
								netcs_index,
								trnloc_index/*((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trnloc_index*/);
					} else {
						ErrorTraceHandle(2, "mml_trnloc() netcs_index-%d: trnloc-%d in client mode, actions are performed in mml_trncon().\n\n",
								netcs_index,
								trnloc_index);
					}

					// server mode
					if (NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 2) {
						ErrorTraceHandle(2, "mml_trncon() Server on localhost port %d.\n", locportnum);
						mml_send_line("Server on localhost port %d.", locportnum);

						NetCS_SocketConfigHost(NetCS_DataSet[netcs_index].trn_status.server.ip, locportnum, &localserver_addr);
						if ((tmp_port = NetCS_ServerCreateOnPort(&localserver_addr, NetCS_DataSet[netcs_index].trn_status.udp0_tcp1_sctp2, 1, netcs_index)) == 0) {
							ErrorTraceHandle(1, "mml_trncon() Unable to create server.\n");
							mml_send_line("Unable to create server.");

							NetCS_reset_userset(netcs_index);
							MML_IGNORED;
						}
						if (tmp_port!=locportnum) {
							ErrorTraceHandle(1, "mml_trncon() Unable to create server on localhost port %d.\n", locportnum);
							mml_send_line("Unable to create server on localhost port %d.", locportnum);

							NetCS_reset_userset(netcs_index);
							MML_IGNORED;
						}

						ErrorTraceHandle(2, "mml_trncon() Connection established.");
						mml_send_line("Connection established.");
					}

					mml.Ctrnloc.par[trnloc_index].status = (enum Emml_status) started;
					MML_EXECUTED;
				}
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action start
		break;

		case stop: {
			switch(status) {
				case started:
				{
					// check for one level up dependencies (in case of client trnloc) -> trncon index over epid
					MML_1KEY_get_index(trncon, epid, mml.Ctrnloc.par[trnloc_index].Pepid, trncon_index);
					if ((trncon_index != 0xFFFF) && (mml.Ctrncon.par[trncon_index].status != stopped)) {
						mml_send_line("There exists dependency to trncon with epid <%s>.", mml.Ctrnloc.par[trnloc_index].Pepid);
						MML_IGNORED;
					}

					// check for one level up dependencies (in case of server trnloc) -> m3acon index over said
					MML_1KEY_get_index(m3acon, trnid, mml.Ctrncon.par[trncon_index].Psaid, m3acon_index);
					if ((m3acon_index != 0xFFFF) && (mml.Cm3acon.par[m3acon_index].status != stopped)) {
						mml_send_line("There exists dependency to M3UA m3acon with trnid <%s>.", mml.Ctrncon.par[trncon_index].Psaid);
						MML_IGNORED;
					}

					mml.Ctrnloc.par[trnloc_index].status = (enum Emml_status) stopped;
					MML_EXECUTED;
				}
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
				{
					MML_COMMAND_RESET_PAR(trnloc, trnloc_index);
					MML_EXECUTED;
				}
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(trnloc, cmds)) {
					memcpy(tmp_param_mask, mml.Ctrnloc.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("trnloc(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case epid: mml_send_string("epid=%s", mml.Ctrnloc.par[cmds].Pepid); break;
							case lip:  mml_send_string("lip=%s",  mml.Ctrnloc.par[cmds].Plip); break;
							case lpn:  mml_send_string("lpn=%s",  mml.Ctrnloc.par[cmds].Plpn); break;
							case user: mml_send_string("user=%s", mml.Ctrnloc.par[cmds].Puser); break;
							case mode: mml_send_string("mode=%s", mml.Ctrnloc.par[cmds].Pmode); break;
							case trn:  mml_send_string("trn=%s",  mml.Ctrnloc.par[cmds].Ptrn); break;
							case info: mml_send_string("info=\"%s\"", mml.Ctrnloc.par[cmds].Pinfo); break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Ctrnloc.par[cmds].status == started)
						mml_send_line("; ---> STARTED");
					else
						mml_send_line(";");
				}
			}
			MML_EXECUTED;
		} // action print
		break;

		default:
			MML_IGNORED;
	} // switch action

}

/*
 *
 *
 * 		***********************************  TRNCON *****************************************
 *
 *
 */

void mml_trncon(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int trncon_index 	= mml.active_cmd_par_index;
	int status 	    	= mml.Ctrncon.par[trncon_index].status;
	int trnloc_index 	= 0xFFFF;
	int m3acon_index 	= 0xFFFF;
	int netcs_index 	= 0xFFFF;

	ErrorTraceHandle(2, "mml_trncon(trncon-%d) status=%d.\n", trncon_index, status);

	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{
					struct sockaddr_in localclient_addr;
					struct sockaddr_in remoteserver_addr;

					// ********************************************************************************
					// Parameter check
					// get trnloc over epid
					MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index].Pepid, trnloc_index);
					if ((trnloc_index == 0xFFFF) || (mml.Ctrnloc.par[trnloc_index].status != started)) {
						mml_send_line("No command trnloc started with trncon epid <%s>.", mml.Ctrncon.par[trncon_index].Pepid);
						MML_IGNORED;
					}
					// ********************************************************************************

					if (!strncmp(mml.Ctrnloc.par[trnloc_index].Pmode, "client", 6)) {

						// ********************************************************************************
						if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "udp")) 		{NETCS_INDEX_KEY(netcs_index, udp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "tcp")) 	{NETCS_INDEX_KEY(netcs_index, tcp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else 															{NETCS_INDEX_KEY(netcs_index, sctp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						if (netcs_index!=0xFFFF) { // not allowed parameter update for trncon
							mml_send_line("Transport layer already active.");
							MML_IGNORED;
						}
						if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "udp")) 		{NETCS_INDEX_NEW(netcs_index, udp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "tcp")) 	{NETCS_INDEX_NEW(netcs_index, tcp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						else 															{NETCS_INDEX_NEW(netcs_index, sctp, client, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
						// ********************************************************************************

						ErrorTraceHandle(2, "mml_trncon() Client mode\n");
						NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 = 1;
						strcpy(NetCS_DataSet[netcs_index].trn_status.client.ip,   mml.Ctrnloc.par[trnloc_index].Plip);
						NetCS_DataSet[netcs_index].trn_status.client.port_from_command = atoi(mml.Ctrnloc.par[trnloc_index].Plpn);
						NetCS_DataSet[netcs_index].trn_status.client.port_from_NETCS = 0; // not configured if i'm client
						strcpy(NetCS_DataSet[netcs_index].trn_status.server.ip,   mml.Ctrncon.par[trncon_index].Prip);
						NetCS_DataSet[netcs_index].trn_status.server.port = atoi(mml.Ctrncon.par[trncon_index].Prpn);
					} else {
						ErrorTraceHandle(1, "mml_trncon() Only client mode supported.\n");
						mml_send_line("Only client mode supported for remote Transport configuration (not mode <%s>).", mml.Ctrnloc.par[trnloc_index].Pmode);

						NetCS_reset_userset(netcs_index);
						MML_IGNORED;
					}

					ErrorTraceHandle(2, "mml_trncon() netcs_index-%d: trnloc-%d, trncon-%d\n",
							netcs_index,
							trnloc_index/*((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trnloc_index*/,
							trncon_index/*((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trncon_index*/);
					ErrorTraceHandle(2, "mml_trncon() matching trnloc-%d uses lip:lpn %s:%s\n",
							trnloc_index, mml.Ctrnloc.par[trnloc_index].Plip, mml.Ctrnloc.par[trnloc_index].Plpn);
					ErrorTraceHandle(2, "mml_trncon() matching netcs_index-%d uses client IP:port=%s:%d and server IP:port=%s:%d\n",
							netcs_index,
							NetCS_DataSet[netcs_index].trn_status.client.ip, NetCS_DataSet[netcs_index].trn_status.client.port_from_command,
							NetCS_DataSet[netcs_index].trn_status.server.ip, NetCS_DataSet[netcs_index].trn_status.server.port);

					// SCTP client mode
					if (NetCS_DataSet[netcs_index].trn_status.mode_undef0_client1_server2 == 1) {
						int error_code = 0;
						char tmp[MINIBUF];

						NetCS_SocketConfigHost(NetCS_DataSet[netcs_index].trn_status.client.ip, NetCS_DataSet[netcs_index].trn_status.client.port_from_command, &localclient_addr);
						NetCS_SocketConfigHost(NetCS_DataSet[netcs_index].trn_status.server.ip, NetCS_DataSet[netcs_index].trn_status.server.port, &remoteserver_addr);

						sprintf(tmp, "SCTP client from %s:%d to SCTP server ",
								inet_ntoa(localclient_addr.sin_addr),
								ntohs(localclient_addr.sin_port));
						ErrorTraceHandle(2, "mml_trncon() %s", tmp);
						mml_send_string(tmp);
						sprintf(tmp, "%s:%d.",
								inet_ntoa(remoteserver_addr.sin_addr),
								ntohs(remoteserver_addr.sin_port));
						ErrorTraceHandle(2, "%s\n", tmp);
						mml_send_line(tmp);

						if ((NetCS_DataSet[netcs_index].trn_status.client.socket_id = NetCS_ClientSocketConnect(&remoteserver_addr, &localclient_addr, 2)) <= 0) {
							ErrorTraceHandle(1, "mml_trncon() Client connect error.\n");
							mml_send_line("Client connect error.");

							NetCS_reset_userset(netcs_index);
							MML_IGNORED;
						}
						PTHREAD_SEND_OVER(NetCS_DataSet[netcs_index].trn_status.client.socket_id, 2, 0, netcs_index);
						if (error_code = pthread_create(&NetCS_DataSet[netcs_index].trn_status.server.thread.thread_id, NULL,
														NetCS_Thread_ServerPortReceiveAndStoreBuffer,
														(void *) &NetCS_DataSet[netcs_index].trn_status.server.thread.arg) != 0) {
							NetCS_SocketClose(NetCS_DataSet[netcs_index].trn_status.client.socket_id);
							ErrorTraceHandle(1, "mml_trncon() Failed to create client thread for accepting server data.\n");
							mml_send_line("Failed to create client thread for accepting server data.");

							NetCS_reset_userset(netcs_index);
							MML_IGNORED;
						}

						ErrorTraceHandle(2, "mml_trncon() Connection established.");
						mml_send_line("Connection established.");
					}

					mml.Ctrncon.par[trncon_index].status = (enum Emml_status) started;
					MML_EXECUTED;
				}
				break; // stopped

				default:
					MML_IGNORED;
			} // switch status

		} // action start
		break;

		case stop: {
			switch(status) {
				case started:
				{
					// ********************************************************************************
					// Parameter check
					MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index].Pepid, trnloc_index);
					if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "udp")) 		{NETCS_INDEX_KEY(netcs_index, udp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
					else if (!strcmp(mml.Ctrnloc.par[trnloc_index].Ptrn, "tcp")) 	{NETCS_INDEX_KEY(netcs_index, tcp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}
					else 															{NETCS_INDEX_KEY(netcs_index, sctp, server, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);}

					// check for one level up dependencies -> m3acon index over said
					MML_1KEY_get_index(m3acon, trnid, mml.Ctrncon.par[trncon_index].Psaid, m3acon_index);
					if ((m3acon_index != 0xFFFF) && (mml.Cm3acon.par[m3acon_index].status != stopped)) {
						mml_send_line("There exists dependency to M3UA m3acon with said <%s>.", mml.Ctrncon.par[trncon_index].Psaid);
						MML_IGNORED;
					}
					// ********************************************************************************

					NetCS_reset_userset(netcs_index);

					mml.Ctrncon.par[trncon_index].status = (enum Emml_status) stopped;
					MML_EXECUTED;
				}
				break;

				default:
					MML_IGNORED;
			}
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
					MML_COMMAND_RESET_PAR(trncon, trncon_index);
					MML_EXECUTED;
				break;
				default:
					MML_IGNORED;
			}
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(trncon, cmds)) {
					memcpy(tmp_param_mask, mml.Ctrncon.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("trncon(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case epid: mml_send_string("epid=%s", mml.Ctrncon.par[cmds].Pepid); break;
							case said: mml_send_string("said=%s", mml.Ctrncon.par[cmds].Psaid); break;
							case rip:  mml_send_string("rip=%s",  mml.Ctrncon.par[cmds].Prip); break;
							case rpn:  mml_send_string("rpn=%s",  mml.Ctrncon.par[cmds].Prpn); break;
							case info: mml_send_string("info=\"%s\"", mml.Ctrncon.par[cmds].Pinfo); break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Ctrncon.par[cmds].status == started)
						mml_send_line("; ---> STARTED");
					else
						mml_send_line(";");
				}
			}
			MML_EXECUTED;
		} // action print
		break;

		default:
			MML_IGNORED;

	} // switch action

}
