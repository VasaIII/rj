
#include <arpa/inet.h>
#include <pthread.h>

#define AXE_MMLPARSER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#undef AXE_MMLPARSER

/*
 *
 *
 * 		***********************************  M3ACON *****************************************
 *
 *
 */


/* m3acon configuration model
 *  - m3acon contains dest parameter which is matched to ss7con to find out its ownsp
 *    thus each dest should have only one ownsp
 *  - SI (Service Indication) from matched ss7con will be source of information for m3ua user adaptation
 *    and enum eSigtran_m3ua_UserAdaptation_Supported will be stored there (UserAdaptation_ServiceIndicator)
 *    		- since m3ua-isup is one-to-one connection, m3ua adaptation may contain pointer to isup cic data
 *    		- ??? for sccp it needs to be checked wheather m3ua-sccp is one-to-one and to have it in same way as for isup
*/
void mml_m3acon(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int m3acon_index = mml.active_cmd_par_index;
	int status      = mml.Cm3acon.par[m3acon_index].status;
	int ss7con_index = 0xFFFF;
	int trncon_index = 0xFFFF;
	int trnloc_index = 0xFFFF;
	int sccpcf_index = 0xFFFF;
	int netcs_index = 0xFFFF;
	int sigtran_user_index = 0xFFFF;
	int error_code;

	ErrorTraceHandle(2, "mml_m3acon(m3acon-%d) status=%d.\n", m3acon_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{
					// ********************************************************************************
					// Parameter check
					// get ss7con index over dest - only one ownsp should be connected to same dest (sp)
					MML_1KEY_get_index(ss7con, sp, mml.Cm3acon.par[m3acon_index].Pdest, ss7con_index);
					if (ss7con_index == 0xFFFF) {
						mml_send_line("No command ss7con given with destination sp <%s>.", mml.Cm3acon.par[m3acon_index].Pdest);
						MML_IGNORED;
					}

					if (!strcmp(mml.Cm3acon.par[m3acon_index].Pbmode, "client")) {
						// if client than trncon can be defined for trnloc and trncon_index is allocated
						// get trncon over said (in case trncon, trnid represents said)
						MML_1KEY_get_index(trncon, said, mml.Cm3acon.par[m3acon_index].Ptrnid, trncon_index);
						if ((trncon_index == 0xFFFF) || (mml.Ctrncon.par[trncon_index].status != started)) {
							mml_send_line("No command trncon started with said <%s>.", mml.Cm3acon.par[m3acon_index].Ptrnid);
							MML_IGNORED;
						}

						// get trnloc over trncon epid
						MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index].Pepid, trnloc_index);
						if ((trnloc_index == 0xFFFF) || (mml.Ctrnloc.par[trnloc_index].status != started)) {
							mml_send_line("No command trnloc started with epid <%s>.", mml.Ctrncon.par[trncon_index].Pepid);
							MML_IGNORED;
						}

					} else {
						// if server than trncon can NOT be defined for trnloc and trncon_index is NOT allocated (it is dummy #FFFF)
						// get trnloc over epid (in case trnloc, trnid represents epid)
						MML_1KEY_get_index(trnloc, epid, mml.Cm3acon.par[m3acon_index].Ptrnid, trnloc_index);
						if ((trnloc_index == 0xFFFF) || (mml.Ctrnloc.par[trnloc_index].status != started)) {
							mml_send_line("No command trnloc started with epid <%s>.", mml.Cm3acon.par[m3acon_index].Ptrnid);
							MML_IGNORED;
						}
					}

					NETCS_INDEX_KEY(netcs_index, sctp, unknown, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);
					if (netcs_index == 0xFFFF) {
						mml_send_line("Transport layer network configuration is not active for this m3acon.");
						MML_IGNORED;
					}

					SIGTRAN_USER_INDEX_KEY(sigtran_user_index, netcs_index, sigtran_m3ua, m3acon_index);
					if (sigtran_user_index!=0xFFFF) { // not allowed parameter update for m3acon if m3ua already active
						mml_send_line("M3UA layer already active.");
						MML_IGNORED;
					}
					// ********************************************************************************

					SIGTRAN_USER_INDEX_NEW(sigtran_user_index, netcs_index, sigtran_m3ua, m3acon_index);

					ErrorTraceHandle(2, "mml_m3acon() netcs_index-%d/sigtran_user_index-%d: m3ua, m3acon-%d, trncon-%d, trnloc-%d\n",
									 netcs_index,
									 sigtran_user_index,
									 ((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->mml_m3acon_index,
									 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trncon_index,
									 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trnloc_index);

					if (!strcmp(mml.Cm3acon.par[m3acon_index].Pbmode, "server"))
						sigtran_user[sigtran_user_index].mode_undef0_client1_server2 = 2;
					else if (!strcmp(mml.Cm3acon.par[m3acon_index].Pbmode, "client"))
						sigtran_user[sigtran_user_index].mode_undef0_client1_server2 = 1;

					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vDPC 	= atoi(mml.Cm3acon.par[m3acon_index].Pdest);
					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vOPC 	= atoi(mml.Css7con.par[ss7con_index].Pownsp);
					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vSI  	= atoi(mml.Css7con.par[ss7con_index].Psi);
					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vNI  	= atoi(mml.Css7con.par[ss7con_index].Pni);
					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vMP  	= 0;
					((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vSLS 	= atoi(mml.Css7con.par[ss7con_index].Psls);

					ErrorTraceHandle(2, "mml_m3acon() DPC=%d, OPC=%d, SI=%d, NI=%d, MP=%d, SLS=%d\n",
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vDPC,
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vOPC,
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vSI,
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vNI,
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vMP,
									((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->Protocol_Data_Configured.vSLS);

					// create message parsing thread
					PTHREAD_SEND_OVER_SIGTRAN(sigtran_user_index, m3ua_HEADER, netcs_index);
					if (error_code = pthread_create(&sigtran_user[sigtran_user_index].netcs_user_thread_id,
													NULL,
													process_sigtran_thread,
													(void *) &sigtran_user[sigtran_user_index].netcs_user_thread_arg) != 0) {
						ErrorTraceHandle(1, "mml_m3acon() process_sigtran() thread failed to create !.\n\n");

						sigtran_USER_INDEX_RESET(sigtran_user_index);
						MML_IGNORED;
					}

					if (sigtran_user[sigtran_user_index].mode_undef0_client1_server2 == 1) { /*client mode*/

						ErrorTraceHandle(2, "mml_m3acon() Client user, initiate handshake !\n");

						ErrorTraceHandle(2, "mml_m3acon() %d %d\n",
								sigtran_user[sigtran_user_index].client.vASPUPACK_received,
								sigtran_user[sigtran_user_index].client.vASPUP_sent);

						sigtran_user[sigtran_user_index].client.waiting_handshake = true;
						sigtran_user[sigtran_user_index].client.vASPUP_time_sent = time(NULL);


						while(1) {

							if (sigtran_user[sigtran_user_index].client.waiting_handshake) {

								if (sigtran_user[sigtran_user_index].client.vASPUPACK_received)
									sigtran_user[sigtran_user_index].client.waiting_handshake = false;
								else if (!sigtran_user[sigtran_user_index].client.vASPUP_sent) {

									SIGTRAN__ASP_Up_ASPUP(netcs_index, sigtran_user_index);

									sigtran_user[sigtran_user_index].client.vASPUP_sent = true;
									sigtran_user[sigtran_user_index].client.vASPUP_time_sent = time(NULL);
									SEND_MSG_TRACE("", ASP_Up_ASPUP);
									mml_send_line("Pinging peer (%s:%s) M3UA.", mml.Ctrncon.par[trncon_index].Prip, mml.Ctrncon.par[trncon_index].Prpn);
								}
							}

							while (sigtran_user[sigtran_user_index].client.waiting_handshake) {
								time_t passed_time = time(NULL)-sigtran_user[sigtran_user_index].client.vASPUP_time_sent;

								if ((passed_time!=0) && ((passed_time)%5 == 0)) { // if after 5 seconds ack did not arrive, reset aspup
									if (!sigtran_user[sigtran_user_index].client.vASPUPACK_received) {
										sigtran_user[sigtran_user_index].client.vASPUP_sent=false;
										ErrorTraceHandle(1, "5 sec elapsed, reset ASPUP and try again (state=%s) ...\n",
												(sigtran_user[sigtran_user_index].client.vASPUP_sent)?"TRUE":"FALSE");
										break;
									}
								}
							} // wait until buffer filled and released

							// message will be received and this flag set in process_sigtran() thread
							if (sigtran_user[sigtran_user_index].client.waiting_handshake == 0)
								break;
						} // while()

					} else {
						ErrorTraceHandle(2, "mml_m3acon() Server user (%d-%s), no handhake !\n", sigtran_user[sigtran_user_index].mode_undef0_client1_server2, mml.Cm3acon.par[m3acon_index].Pbmode);
					}

					mml.Cm3acon.par[m3acon_index].status = (enum Emml_status) started;
					MML_EXECUTED;
				}
				break; // status stopped
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
					if (!strcmp(mml.Cm3acon.par[m3acon_index].Pbmode, "client")) {
						// if client than trncon can be defined for trnloc and trncon_index is allocated
						MML_1KEY_get_index(trncon, said, mml.Cm3acon.par[m3acon_index].Ptrnid, trncon_index);
						MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index].Psaid, trnloc_index);
					} else {
						// if server than trncon can NOT be defined for trnloc and trncon_index is NOT allocated (it is dummy #FFFF)
						// get trnloc over epid
						MML_1KEY_get_index(trnloc, epid, mml.Cm3acon.par[m3acon_index].Ptrnid, trnloc_index);
					}

					NETCS_INDEX_KEY(netcs_index, sctp, unknown, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);
					SIGTRAN_USER_INDEX_KEY(sigtran_user_index, netcs_index, sigtran_m3ua, m3acon_index);

					// check for one level up dependencies -> sccp (sccpcf)
					MML_1KEY_get_index(sccpcf, sp, mml.Cm3acon.par[m3acon_index].Pdest, sccpcf_index);
					if ((sccpcf_index != 0xFFFF) && (mml.Csccpcf.par[sccpcf_index].status != stopped)) {
						mml_send_line("There exists dependency to sccp sccpcf with sp <%s>.", mml.Cm3acon.par[m3acon_index].Pdest);
						MML_IGNORED;
					}
					// ********************************************************************************

					sigtran_user[sigtran_user_index].both.shuttingdown = 1;
					NETCS_USER_WAKEUP(netcs_index); \

					mml.Cm3acon.par[m3acon_index].status = (enum Emml_status) stopped;
					// reset of sigtran id and free of sigtran user alloc is preformed within process_sigtran_thread beffore exit

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
					MML_COMMAND_RESET_PAR(m3acon, m3acon_index);
					MML_EXECUTED;
				}
				break; // satus stopped
				default:
					MML_IGNORED;
			} // switch status
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;
			ErrorTraceHandle(2, "mml_m3acon()\n");

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(m3acon, cmds)) {
					memcpy(tmp_param_mask, mml.Cm3acon.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("m3acon(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case trnid: mml_send_string("trnid=%s",  mml.Cm3acon.par[cmds].Ptrnid);   break;
							case dest:  mml_send_string("dest=2-%s",mml.Cm3acon.par[cmds].Pdest);   break;
							case bmode: mml_send_string("bmode=%s", mml.Cm3acon.par[cmds].Pbmode);  break;
							case info:  mml_send_string("info=\"%s\"",  mml.Cm3acon.par[cmds].Pinfo);   break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cm3acon.par[cmds].status == started)
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




void mml_m3loop(void) {
	int m3loop_index = mml.active_cmd_par_index;
	int status 		 = mml.Cm3loop.par[m3loop_index].status;
	int netcs_index1 = 0xFFFF;
	int netcs_index2 = 0xFFFF;
	int sigtran_user_index1 	= 0xFFFF;
	int sigtran_user_index2 	= 0xFFFF;

	ErrorTraceHandle(2, "mml_m3loop(m3loop-%d) status=%d.\n", m3loop_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{

					{
						int ss7con_index1 		= 0xFFFF;
						int m3acon_index1 		= 0xFFFF;
						int trncon_index1 		= 0xFFFF;
						int trnloc_index1 		= 0xFFFF;

						// ********************************************************************************
						// Parameter check
						// get ss7con index over ownsp (ss7con due to fact that ownsp should be connected to remote sp, not yust defined with ss7loc)
						MML_1KEY_get_index(ss7con, ownsp, mml.Cm3loop.par[m3loop_index].Pownsp1, ss7con_index1);
						if (ss7con_index1 == 0xFFFF) {
							mml_send_line("No command ss7con given with ownsp <%s>\n", mml.Cm3loop.par[m3loop_index].Pownsp1);
							MML_IGNORED;
						}

						// get m3acon index over dest-said
						MML_2KEY_get_index(m3acon, dest, mml.Cm3loop.par[m3loop_index].Pdest1, trnid, mml.Cm3loop.par[m3loop_index].Ptrnid1, m3acon_index1);
						if ((m3acon_index1 == 0xFFFF) || (mml.Cm3acon.par[m3acon_index1].status != started)) {
							mml_send_line("No command m3acon started with trnid <%s> and dest <%s>.", mml.Cm3loop.par[m3loop_index].Ptrnid1, mml.Cm3loop.par[m3loop_index].Pdest1);
							MML_IGNORED;
						}

						// get trncon over said
						MML_1KEY_get_index(trncon, said, mml.Cm3acon.par[m3acon_index1].Ptrnid, trncon_index1);
						if ((trncon_index1 == 0xFFFF) || (mml.Ctrncon.par[trncon_index1].status != started)) {
							mml_send_line("No command trncon started with said <%s>.", mml.Cm3acon.par[m3acon_index1].Ptrnid);
							MML_IGNORED;
						}

						// get trnloc over epid from trncon
						MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index1].Pepid, trnloc_index1);
						if ((trnloc_index1 == 0xFFFF) || (mml.Ctrnloc.par[trnloc_index1].status != started)) {
							mml_send_line("No command trnloc started with epid <%s>.", mml.Ctrncon.par[trncon_index1].Pepid);
							MML_IGNORED;
						}
						// ********************************************************************************

						NETCS_INDEX_KEY(netcs_index1, sctp, unknown, netcs_sigtran, trnloc, trnloc_index1, trncon, trncon_index1);
						if (netcs_index1 == 0xFFFF) {
							mml_send_line("mml_m3loop(FIRST) Transport layer network configuration is not started for this m3acon.");
							MML_IGNORED;
						}

						SIGTRAN_USER_INDEX_KEY(sigtran_user_index1, netcs_index1, sigtran_m3ua, m3acon_index1);
						if (sigtran_user_index1==0xFFFF) {
							mml_send_line("mml_m3loop(FIRST) M3UA layer already active.");
							MML_IGNORED;
						}

						ErrorTraceHandle(2, "mml_m3loop(FIRST) netcs_index1-%d/sigtran_user_index1-%d: m3ua, m3acon-%d, trncon-%d, trnloc-%d\n",
										 netcs_index1,
										 sigtran_user_index1,
										 ((struct Sm3ua_user *) sigtran_user[sigtran_user_index1].pUserAdaptation)->mml_m3acon_index,
										 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index1].puser_data)->mml_trncon_index,
										 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index1].puser_data)->mml_trnloc_index);
					}

					{
						int ss7con_index2 		= 0xFFFF;
						int m3acon_index2 		= 0xFFFF;
						int trncon_index2 		= 0xFFFF;
						int trnloc_index2 		= 0xFFFF;

						// ********************************************************************************
						// Parameter check
						// get ss7con index over ownsp (ss7con due to fact that ownsp should be connected to remote sp, not yust defined with ss7loc)
						MML_1KEY_get_index(ss7con, ownsp, mml.Cm3loop.par[m3loop_index].Pownsp2, ss7con_index2);
						if (ss7con_index2 == 0xFFFF) {
							mml_send_line("No command ss7con given with ownsp <%s>\n", mml.Cm3loop.par[m3loop_index].Pownsp2);
							MML_IGNORED;
						}

						// get m3acon index over dest-said
						MML_2KEY_get_index(m3acon, dest, mml.Cm3loop.par[m3loop_index].Pdest2, trnid, mml.Cm3loop.par[m3loop_index].Ptrnid2, m3acon_index2);
						if ((m3acon_index2 == 0xFFFF) || (mml.Cm3acon.par[m3acon_index2].status != started)) {
							mml_send_line("No command m3acon started with trnid2 <%s> and dest <%s>.", mml.Cm3loop.par[m3loop_index].Ptrnid2, mml.Cm3loop.par[m3loop_index].Pdest2);
							MML_IGNORED;
						}

						// get trncon over said
						MML_1KEY_get_index(trncon, said, mml.Cm3acon.par[m3acon_index2].Ptrnid, trncon_index2);
						if ((trncon_index2 == 0xFFFF) || (mml.Ctrncon.par[trncon_index2].status != started)) {
							mml_send_line("No command trncon started with said <%s>.", mml.Cm3acon.par[m3acon_index2].Ptrnid);
							MML_IGNORED;
						}

						// get trnloc over epid from trncon
						MML_1KEY_get_index(trnloc, epid, mml.Ctrncon.par[trncon_index2].Pepid, trnloc_index2);
						if ((trnloc_index2 == 0xFFFF) || (mml.Ctrnloc.par[trnloc_index2].status != started)) {
							mml_send_line("No command trnloc started with epid <%s>.", mml.Ctrncon.par[trncon_index2].Pepid);
							MML_IGNORED;
						}
						// ********************************************************************************

						NETCS_INDEX_KEY(netcs_index2, sctp, unknown, netcs_sigtran, trnloc, trnloc_index2, trncon, trncon_index2);
						if (netcs_index2 == 0xFFFF) {
							mml_send_line("mml_m3loop(SECOND) Transport layer network configuration is not active for this m3acon.");
							MML_IGNORED;
						}
						SIGTRAN_USER_INDEX_KEY(sigtran_user_index2, netcs_index2, sigtran_m3ua, m3acon_index2);
						if (sigtran_user_index1==0xFFFF) {
							mml_send_line("mml_m3loop(SECOND) M3UA layer already active.");
							MML_IGNORED;
						}

						ErrorTraceHandle(2, "mml_m3loop(SECOND) netcs_index2-%d/sigtran_user_index2-%d: m3ua, m3acon-%d, trncon-%d, trnloc-%d\n",
										 netcs_index2,
										 sigtran_user_index2,
										 ((struct Sm3ua_user *) sigtran_user[sigtran_user_index2].pUserAdaptation)->mml_m3acon_index,
										 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index2].puser_data)->mml_trncon_index,
										 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index2].puser_data)->mml_trnloc_index);
					}

					if        (!strcmp(mml.Cm3loop.par[m3loop_index].Pdir, "sp1sp2")) {
						M3UA_USER_LOOP(netcs_index1,
									   sigtran_user_index1,
									   mml.Cm3loop.par[m3loop_index].Pownsp1,
									   netcs_index2,
									   sigtran_user_index2,
									   mml.Cm3loop.par[m3loop_index].Pownsp2,
									   atoi(mml.Cm3loop.par[m3loop_index].Pcicinc1));
					} else if (!strcmp(mml.Cm3loop.par[m3loop_index].Pdir, "sp2sp1")) {
						M3UA_USER_LOOP(netcs_index2,
									   sigtran_user_index2,
									   mml.Cm3loop.par[m3loop_index].Pownsp2,
									   netcs_index1,
									   sigtran_user_index1,
									   mml.Cm3loop.par[m3loop_index].Pownsp1,
									   -atoi(mml.Cm3loop.par[m3loop_index].Pcicinc2));
					} else if (!strcmp(mml.Cm3loop.par[m3loop_index].Pdir, "bothway")) {
						M3UA_USER_LOOP(netcs_index1,
									   sigtran_user_index1,
									   mml.Cm3loop.par[m3loop_index].Pownsp1,
									   netcs_index2,
									   sigtran_user_index2,
									   mml.Cm3loop.par[m3loop_index].Pownsp2,
									   atoi(mml.Cm3loop.par[m3loop_index].Pcicinc1));
						M3UA_USER_LOOP(netcs_index2,
									   sigtran_user_index2,
									   mml.Cm3loop.par[m3loop_index].Pownsp2,
									   netcs_index1,
									   sigtran_user_index1,
									   mml.Cm3loop.par[m3loop_index].Pownsp1,
									   -atoi(mml.Cm3loop.par[m3loop_index].Pcicinc2));
					} else {
						mml_send_line("Only sp1-sp2, sp2-sp1 and bothway supported for <dir> parameter, %s unsupported.", mml.Cm3loop.par[m3loop_index].Pdir);
						MML_IGNORED;
					}

					mml.Cm3loop.par[m3loop_index].status = (enum Emml_status) started;
				}
				break; // status stopped
				default:
					MML_IGNORED;
			} // switch status
		} // action start
		break;


		case stop: {
			switch(status) {
				case started:
				{
					// There is no upper level dependencies for m3ua loop

					mml.Cm3loop.par[m3loop_index].status = (enum Emml_status) stopped;
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
					// There is no upper level dependencies for m3ua loop

					M3UA_USER_LOOP_RESET(m3loop_index);
					MML_COMMAND_RESET_PAR(m3loop, m3loop_index);
					MML_EXECUTED;
				}
				break; // satus stopped
				default:
					MML_IGNORED;
			} // switch status
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;
			ErrorTraceHandle(2, "mml_m3loop()\n");

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(m3loop, cmds)) {
					memcpy(tmp_param_mask, mml.Cm3loop.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("m3loop(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case ownsp1: 	mml_send_string("ownsp1=2-%s", 	mml.Cm3loop.par[cmds].Pownsp1);  break;
							case dest1:  	mml_send_string("dest1=2-%s",	mml.Cm3loop.par[cmds].Pdest1);   break;
							case trnid1:  	mml_send_string("trnid1=%s",  	mml.Cm3loop.par[cmds].Ptrnid1);   break;
							case ownsp2: 	mml_send_string("ownsp2=2-%s", 	mml.Cm3loop.par[cmds].Pownsp2);  break;
							case dest2:  	mml_send_string("dest2=2-%s",	mml.Cm3loop.par[cmds].Pdest2);   break;
							case trnid2:  	mml_send_string("trnid2=%s",  	mml.Cm3loop.par[cmds].Ptrnid2);   break;
							case dir:  		mml_send_string("dir=\"%s\"",  	mml.Cm3loop.par[cmds].Pdir);   	 break;
							case cicinc1:	mml_send_string("cicinc1=%s",	mml.Cm3loop.par[cmds].Pcicinc1);break;
							case cicinc2:	mml_send_string("cicinc2=%s",	mml.Cm3loop.par[cmds].Pcicinc2);break;
							case info:  	mml_send_string("info=\"%s\"",  mml.Cm3loop.par[cmds].Pinfo);    break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cm3loop.par[cmds].status == started)
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

