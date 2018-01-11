
#define AXE_MMLPARSER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#include <modules/axe/application/bss/bss.h>
#include <modules/axe/application/pstn/pstn.h>
#undef AXE_MMLPARSER



void mml_bsci(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int bsci_index  = mml.active_cmd_par_index;
	int status 		= mml.Cbsci.par[bsci_index].status;

	ErrorTraceHandle(2, "mml_bsci(bsci-%d) status=%d.\n", bsci_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start:
		case stop: {
			MML_IGNORED;
		}
		break;

		case delete: {
			MML_COMMAND_RESET_PAR(bsci, bsci_index);
			MML_EXECUTED;
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(bsci, cmds)) {
					memcpy(tmp_param_mask, mml.Cbsci.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("bsci(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case bsc:  mml_send_string("bsc=%s",    mml.Cbsci.par[cmds].Pbsc);   break;
							case ownsp:mml_send_string("ownsp=2-%s",mml.Cbsci.par[cmds].Pownsp); break;
							case mscsp:mml_send_string("mscsp=2-%s",mml.Cbsci.par[cmds].Pmscsp); break;
							case info: mml_send_string("info=\"%s\"",mml.Cbsci.par[cmds].Pinfo);  break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cbsci.par[cmds].status == started)
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




void mml_btsi(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int btsi_index  = mml.active_cmd_par_index;
	int status 		= mml.Cbtsi.par[btsi_index].status;

	ErrorTraceHandle(2, "mml_btsi(btsi-%d) status=%d.\n", btsi_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start:
		case stop: {
			MML_IGNORED;
		}
		break;

		case delete: {
			MML_COMMAND_RESET_PAR(btsi, btsi_index);
			MML_EXECUTED;
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first, skip_this;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(btsi, cmds)) {
					memcpy(tmp_param_mask, mml.Cbtsi.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("btsi(%d):", cmds);
					first=true; skip_this = false;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first && !skip_this) mml_send_string(", ");
							if (skip_this) skip_this = false;
							switch ((enum Emml_command_parameters) new_enum) {
							case bsc: mml_send_string("bsc=%s", mml.Cbtsi.par[cmds].Pbsc);  break;
							case cell:mml_send_string("cell=%s",mml.Cbtsi.par[cmds].Pcell); break;
							case cgi_mcc: mml_send_string("cgi=%s-%s-%s-%s",
													 mml.Cbtsi.par[cmds].Pcgi_mcc,
													 mml.Cbtsi.par[cmds].Pcgi_mnc,
													 mml.Cbtsi.par[cmds].Pcgi_lac,
													 mml.Cbtsi.par[cmds].Pcgi_ci);
								 skip_this=true;
							break;
							case info:mml_send_string("info=\"%s\"",mml.Cbtsi.par[cmds].Pinfo);  break;
							default:
								skip_this=true;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cbtsi.par[cmds].status == started)
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





void mml_mei(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int mei_index  = mml.active_cmd_par_index;
	int status 		= mml.Cmei.par[mei_index].status;

	ErrorTraceHandle(2, "mml_mei(mei-%d) status=%d.\n", mei_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start:
		case stop: {
			MML_IGNORED;
		}
		break;

		case delete: {
			MML_COMMAND_RESET_PAR(mei, mei_index);
			MML_EXECUTED;
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(mei, cmds)) {
					memcpy(tmp_param_mask, mml.Cmei.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("mei(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case me:  mml_send_string("me=%s",  mml.Cmei.par[cmds].Pme);   break;
							case imsi:mml_send_string("imsi=%s",mml.Cmei.par[cmds].Pimsi); break;
							case cell:mml_send_string("cell=%s",mml.Cmei.par[cmds].Pcell); break;
							case bsc: mml_send_string("bsc=%s", mml.Cmei.par[cmds].Pbsc);  break;
							case info:mml_send_string("info=\"%s\"",mml.Cmei.par[cmds].Pinfo);  break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cmei.par[cmds].status == started)
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




void mml_phonei(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int phonei_index  = mml.active_cmd_par_index;
	int status 		  = mml.Cphonei.par[phonei_index].status;

	ErrorTraceHandle(2, "mml_phonei(phone-%d) status=%d.\n", phonei_index, status);

	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start:
		case stop: {
			MML_IGNORED;
		}
		break;

		case delete: {
			MML_COMMAND_RESET_PAR(phonei, phonei_index);
			MML_EXECUTED;
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(phonei, cmds)) {
					memcpy(tmp_param_mask, mml.Cphonei.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("phonei(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case phone:mml_send_string("phone=%s",  	mml.Cphonei.par[cmds].Pphone);   break;
							case Anum: mml_send_string("anum=%s",		mml.Cphonei.par[cmds].PAnum); break;
							case Bnum: mml_send_string("bnum=%s",		mml.Cphonei.par[cmds].PBnum); break;
							case Bdest:mml_send_string("bdest=%s",		mml.Cphonei.par[cmds].PBdest); break;
							case range:mml_send_string("range=%s",		mml.Cphonei.par[cmds].Prange);  break;
							case info: mml_send_string("info=\"%s\"",	mml.Cphonei.par[cmds].Pinfo);  break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Cphonei.par[cmds].status == started)
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




void mml_call(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int mml_call_index 		= mml.active_cmd_par_index;
	int status 		   		= mml.Ccall.par[mml_call_index].status;
	int trncon_index 		= 0xFFFF;
	int trnloc_index 		= 0xFFFF;
	int m3acon_index 		= 0xFFFF;
	int netcs_index 		= 0xFFFF;
	int m3ua_user_index 	= 0xFFFF;
	int error_code;

	ErrorTraceHandle(2, "mml_call(mml-%d) id=%s, caller=%s, called=%s, type=%s, cps=%s, duration=%s, cic=%s, user=%s\n",
					 mml_call_index,
					 mml.Ccall.par[mml_call_index].Pid,
					 mml.Ccall.par[mml_call_index].Pcaller,
					 mml.Ccall.par[mml_call_index].Pcalled,
					 mml.Ccall.par[mml_call_index].Ptype,
					 mml.Ccall.par[mml_call_index].Pcps,
					 mml.Ccall.par[mml_call_index].Pduration,
					 mml.Ccall.par[mml_call_index].Pcic,
					 mml.Ccall.par[mml_call_index].Puser);


	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{

					if ((!strcmp(mml.Ccall.par[mml_call_index].Ptype, "GSM_LU")) ||
						(!strcmp(mml.Ccall.par[mml_call_index].Ptype, "GSM_CALL")))
					{
						int mml_caller_mei_index 	= 0xFFFF;
						int mml_called_mei_index 	= 0xFFFF;
						int mml_caller_btsi_index 	= 0xFFFF;
						int mml_called_btsi_index 	= 0xFFFF;
						int mml_caller_bsci_index 	= 0xFFFF;
						int mml_called_bsci_index 	= 0xFFFF;

						// ********************************************************************************
						// Parameter check
						// fetch caller
						MML_1KEY_get_index(mei, me, mml.Ccall.par[mml_call_index].Pcaller, mml_caller_mei_index);
						if (mml_caller_mei_index == 0xFFFF) {
							mml_send_line("No command mei given with CALLER <%s>.", mml.Ccall.par[mml_call_index].Pcaller);
							MML_IGNORED;
						} else {
							MML_2KEY_get_index(btsi, cell, mml.Cmei.par[mml_caller_mei_index].Pcell, bsc, mml.Cmei.par[mml_caller_mei_index].Pbsc, mml_caller_btsi_index);
							if (mml_caller_btsi_index == 0xFFFF) {
								mml_send_line("No command btsi given with CALLER cell <%s> and CALLER bsc <%s>.", mml.Cmei.par[mml_caller_mei_index].Pcell, mml.Cmei.par[mml_caller_mei_index].Pbsc);
								MML_IGNORED;
							} else {
								MML_1KEY_get_index(bsci, bsc, mml.Cmei.par[mml_caller_mei_index].Pbsc, mml_caller_bsci_index);
								if (mml_caller_bsci_index == 0xFFFF) {
									mml_send_line("No command bsci given with CALLER bsc <%s>.", mml.Cmei.par[mml_caller_mei_index].Pbsc);
									MML_IGNORED;
								}
							}
						}
						if (!strcmp(mml.Ccall.par[mml_call_index].Ptype, "GSM_CALL")) {
							// fetch called
							MML_1KEY_get_index(mei, me, mml.Ccall.par[mml_call_index].Pcalled, mml_called_mei_index);
							if (mml_called_mei_index == 0xFFFF) {
								mml_send_line("No command mei given with CALLED <%s>\n", mml.Ccall.par[mml_call_index].Pcalled);
								MML_IGNORED;
							} else {
								MML_2KEY_get_index(btsi, cell, mml.Cmei.par[mml_called_mei_index].Pcell, bsc, mml.Cmei.par[mml_called_mei_index].Pbsc, mml_called_btsi_index);
								if (mml_called_btsi_index == 0xFFFF) {
									mml_send_line("No command btsi given with CALLED cell <%s> and CALLED bsc <%s>\n", mml.Cmei.par[mml_called_btsi_index].Pcell, mml.Cmei.par[mml_called_btsi_index].Pbsc);
									MML_IGNORED;
								} else {
									MML_1KEY_get_index(bsci, bsc, mml.Cmei.par[mml_called_mei_index].Pbsc, mml_called_bsci_index);
									if (mml_called_bsci_index == 0xFFFF) {
										mml_send_line("No command bsci given with CALLED bsc <%s>\n", mml.Cmei.par[mml_called_mei_index].Pbsc);
										MML_IGNORED;
									}
								}
							}
						}

						// get m3acon over dest (assume that no mrsi's are defined for same DEST over two different SAIDs)
						MML_1KEY_get_index(m3acon, dest, mml.Cbsci.par[mml_caller_bsci_index].Pmscsp, m3acon_index);
						if ((m3acon_index == 0xFFFF) || (mml.Cm3acon.par[m3acon_index].status != started)) {
							mml_send_line("No command m3acon started with dest <%s>.", mml.Cbsci.par[mml_caller_bsci_index].Pmscsp);
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
						SIGTRAN_USER_INDEX_KEY(m3ua_user_index, netcs_index, sigtran_m3ua, m3acon_index);
						mml_send_line("Call m3ua_user_index=%d, mml_call_index=%d", m3ua_user_index, mml_call_index);

						if (((struct Sm3ua_user *)sigtran_user[m3ua_user_index].pUserAdaptation)->sccp_user_ssn254_bssap.allowed == 0) {
							mml_send_line("Subsystem still not allowed, try again in short time ...");
							MML_IGNORED;
						}

						// ********************************************************************************

						if (((struct Sm3ua_user *)sigtran_user[m3ua_user_index].pUserAdaptation)->sccp_user_ssn254_bssap.state == BSC_undef)
							NETCS_INDEX_RESET_TRANSPORT_DATA(netcs_index);

						memcpy(&bss_bsc_application[mml_call_index].mml_call,    &mml.Ccall.par[mml_call_index],    sizeof(struct Scall_parameters));

						memcpy(&bss_bsc_application[mml_call_index].caller.mml_mei,  &mml.Cmei.par[mml_caller_mei_index],   sizeof(struct Smei_parameters));
						memcpy(&bss_bsc_application[mml_call_index].caller.mml_btsi, &mml.Cbtsi.par[mml_caller_btsi_index], sizeof(struct Sbtsi_parameters));
						memcpy(&bss_bsc_application[mml_call_index].caller.mml_bsci, &mml.Cbsci.par[mml_caller_bsci_index], sizeof(struct Sbsci_parameters));

						memcpy(&bss_bsc_application[mml_call_index].called.mml_mei,  &mml.Cmei.par[mml_called_mei_index],   sizeof(struct Smei_parameters));
						memcpy(&bss_bsc_application[mml_call_index].called.mml_btsi, &mml.Cbtsi.par[mml_called_btsi_index], sizeof(struct Sbtsi_parameters));
						memcpy(&bss_bsc_application[mml_call_index].called.mml_bsci, &mml.Cbsci.par[mml_called_bsci_index], sizeof(struct Sbsci_parameters));

						PTHREAD_SEND_TO_BSS_BSC_APPLICATION(m3ua_user_index, mml_call_index);
						if (error_code = pthread_create(&bss_bsc_application[mml_call_index].thread_id, NULL,
														process_bss_bsc_thread,
														(void *) &bss_bsc_application[mml_call_index].thread_arg) != 0) {
							ErrorTraceHandle(0, "mml_call() process_bss_bsc_thread() Failed to create !.\n\n");
							MML_IGNORED;
						} else
							ErrorTraceHandle(2, "mml_call() process_bss_bsc_thread() Created for mml_call_index=%d\n", mml_call_index);

						mml.Ccall.par[mml_call_index].status = (enum Emml_status) started;
						MML_EXECUTED;

					} /* LU || MO_TO_MT */
					else if (!strcmp(mml.Ccall.par[mml_call_index].Ptype, "PHONE_CALL"))
					{
						int mml_caller_phonei_index 	= 0xFFFF;
						int mml_called_phonei_index 	= 0xFFFF;

						MML_1KEY_get_index(phonei, Anum, mml.Ccall.par[mml_call_index].Pcaller, mml_caller_phonei_index);
						if (mml_caller_phonei_index == 0xFFFF) {
							mml_send_line("No command phonei given with caller <%s>.", mml.Ccall.par[mml_call_index].Pcaller);
							MML_IGNORED;
						}

						// get m3acon over dest (assume that no mrsi's are defined for same DEST over two different SAIDs)
						MML_1KEY_get_index(m3acon, dest, mml.Cphonei.par[mml_caller_phonei_index].PBdest, m3acon_index);
						if ((m3acon_index == 0xFFFF) || (mml.Cm3acon.par[m3acon_index].status != started)) {
							mml_send_line("No command m3acon started with Bnum dest <%s>.", mml.Cphonei.par[mml_caller_phonei_index].PBdest);
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
						SIGTRAN_USER_INDEX_KEY(m3ua_user_index, netcs_index, sigtran_m3ua, m3acon_index);
						mml_send_line("Call m3ua_user_index=%d, mml_call_index=%d", m3ua_user_index, mml_call_index);

						// ********************************************************************************

						memcpy(&pstn_application[mml_call_index].mml_call,          &mml.Ccall.par[mml_call_index],            sizeof(struct Scall_parameters));
						memcpy(&pstn_application[mml_call_index].caller.mml_phonei, &mml.Cphonei.par[mml_caller_phonei_index], sizeof(struct Sphonei_parameters));
						memcpy(&pstn_application[mml_call_index].called.mml_phonei, &mml.Cphonei.par[mml_called_phonei_index], sizeof(struct Sphonei_parameters));

						PTHREAD_SEND_TO_PSTN_APPLICATION(m3ua_user_index, mml_call_index);
						if (error_code = pthread_create(&pstn_application[mml_call_index].thread_id,
														NULL,
														process_pstn_thread,
														(void *) &pstn_application[mml_call_index].thread_arg) != 0) {
							ErrorTraceHandle(0, "mml_call() process_pstn_thread() Failed to create !.\n\n");
							MML_IGNORED;
						} else
							ErrorTraceHandle(2, "mml_call() process_pstn_thread() Created for mml_call_index=%d\n", mml_call_index);

						mml.Ccall.par[mml_call_index].status = (enum Emml_status) started;
						MML_EXECUTED;
					} // PSTN CALL


				}
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action start
		break;

		case stop: {
			// kill application thread
			// release dialog references owned by application thread

			mml.Ccall.par[mml_call_index].status = (enum Emml_status) stopped;
			MML_EXECUTED;
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
				{
					MML_COMMAND_RESET_PAR(call, mml_call_index);
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
				if (MML_PARAM_MASKS_CHECK(call, cmds)) {
					memcpy(tmp_param_mask, mml.Ccall.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("call(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case id: 		mml_send_string("id=%s",  		mml.Ccall.par[cmds].Pid);   	break;
							case caller: 	mml_send_string("caller=%s",	mml.Ccall.par[cmds].Pcaller); 	break;
							case called: 	mml_send_string("called=%s", 	mml.Ccall.par[cmds].Pcalled);  	break;
							case type:		mml_send_string("type=%s",		mml.Ccall.par[cmds].Ptype); 	break;
							case cps: 		mml_send_string("cps=%s", 		mml.Ccall.par[cmds].Pcps);  	break;
							case duration: 	mml_send_string("duration=%s", 	mml.Ccall.par[cmds].Pduration); break;
							case cic: 		mml_send_string("cic=%s", 		mml.Ccall.par[cmds].Pcic); 		break;
							case user:		mml_send_string("user=\"%s\"", 	mml.Ccall.par[cmds].Puser);  	break;
							case info: 		mml_send_string("info=\"%s\"", 	mml.Ccall.par[cmds].Pinfo);  	break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Ccall.par[cmds].status == started)
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





