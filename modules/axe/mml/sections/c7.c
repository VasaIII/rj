#define AXE_MMLPARSER
#include <modules/common.h>
#include <modules/nethelper.h>
#include <modules/axe/mml/parser.h>
#include <modules/protocols/sigtran.h>
#undef AXE_MMLPARSER


void mml_ss7loc(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int ss7loc_index = mml.active_cmd_par_index;
	int status 		= mml.Css7loc.par[ss7loc_index].status;
	int trncon_index = 0xFFFF;
	int trnloc_index = 0xFFFF;
	int m3acon_index = 0xFFFF;
	int sigtran_user_index = 0xFFFF;
	int netcs_index = 0xFFFF;

	ErrorTraceHandle(2, "mml_ss7loc(ss7loc-%d)\n", ss7loc_index);


	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{
					mml.Css7loc.par[ss7loc_index].status = (enum Emml_status) started;
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
					mml.Css7loc.par[ss7loc_index].status = (enum Emml_status) stopped;
					MML_EXECUTED;
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
					MML_COMMAND_RESET_PAR(ss7loc, ss7loc_index);
					MML_EXECUTED;
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
				if (MML_PARAM_MASKS_CHECK(ss7loc, cmds)) {
					memcpy(tmp_param_mask, mml.Css7loc.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("ss7loc(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case ownsp: mml_send_string("ownsp=2-%s",	mml.Css7loc.par[cmds].Pownsp);  	break;
							case sptype:mml_send_string("sptype=%s",	mml.Css7loc.par[cmds].Psptype);  break;
							case info:	mml_send_string("info=\"%s\"",	mml.Css7loc.par[cmds].Pinfo);	break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Css7loc.par[cmds].status == started)
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


// ss7con
//  * currently it is not allowed to create ss7con with different ownsp's toward same destination
//    this is important due fact that m3acon contains only destination parameter and will confuse if
//    there is more ownsp's toward same destinations
void mml_ss7con(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int ss7con_index = mml.active_cmd_par_index;
	int status 		= mml.Css7con.par[ss7con_index].status;
	int trncon_index = 0xFFFF;
	int trnloc_index = 0xFFFF;
	int m3acon_index = 0xFFFF;
	int sigtran_user_index = 0xFFFF;
	int netcs_index = 0xFFFF;

	ErrorTraceHandle(2, "mml_ss7con(ss7con-%d)\n", ss7con_index);


	// deduce new action upon current status
	switch(mml.active_cmd_par_action) {
		case noaction:
		break;

		case start: {
			switch(status) {
				case stopped:
				{
					mml.Css7con.par[ss7con_index].status = (enum Emml_status) started;
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
					mml.Css7con.par[ss7con_index].status = (enum Emml_status) stopped;
					MML_EXECUTED;
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
					MML_COMMAND_RESET_PAR(ss7con, ss7con_index);
					MML_EXECUTED;
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action delete
		break;

		case print: {
			unsigned int cmds, new_enum, tmp_param_mask[PARSER_MML_MAX_MASKS];
			bool first;

/* 	ProtSpec - M3UA Protocol Specification, RevA.pdf
	Service Indicator (SI) - value is used within the SIO to identify the M3UA user.
			Service indicator codes are allocated as follows
			bits 	4 3 2 1
					0 0 0 0	SSNM messages
					0 0 0 1 SSNM regular messages
					0 0 1 0 SSNM special messages
					0 0 1 1 SCCP
					0 1 0 0 TUP
					0 1 0 1 N-ISUP (5)
					0 1 1 0 DUP call and circuit related messages
					0 1 1 1 DUP facility registration and cancellation messages
					1 0 0 0 MTP testing user part
					1 0 0 1 B-ISUP
					1 0 1 0 Satellite ISUP
					1 0 1 1 Reserved
					1 1 0 0 AAL type 2 Signalling
					1 1 0 1 BICC (13)
					1 1 1 0 GCP
					1 1 1 1 Reserved

	Signalling Link Selection (SLS) - 8 bit The Signalling Link Selection field contains the SLS
			bits from the routing label of the original SS7 message justified to the least
			significant bit and in Network Byte Order. Unused bits are coded ‘0’.
			See also chapter ’MTP-SIO, MTP-label, M3UA-User Protocol Data’.
*/
			for (cmds=0;cmds<PARSER_MML_PARAM_MAX_NUMBER_MASK;cmds++) {
				if (MML_PARAM_MASKS_CHECK(ss7con, cmds)) {
					memcpy(tmp_param_mask, mml.Css7con.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("ss7con(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case ownsp: mml_send_string("ownsp=2-%s",	mml.Css7con.par[cmds].Pownsp);  	break;
							case sp:  	mml_send_string("sp=2-%s",		mml.Css7con.par[cmds].Psp);  	break;
							case net: 	mml_send_string("net=%s", 		mml.Css7con.par[cmds].Pnet); 	break;
							case si: 	mml_send_string("si=%s", 		mml.Css7con.par[cmds].Psi); 		break;
							case ni: 	mml_send_string("ni=%s", 		mml.Css7con.par[cmds].Pni); 		break;
							case sls: 	mml_send_string("sls=%s", 		mml.Css7con.par[cmds].Psls); 	break;
							case info:	mml_send_string("info=\"%s\"",	mml.Css7con.par[cmds].Pinfo);	break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Css7con.par[cmds].status == started)
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


void mml_sccpcf(void) { // mml.active_cmd_par_index should be valid key index before entering this function
	int sccpcf_index = mml.active_cmd_par_index;
	int status 		= mml.Csccpcf.par[sccpcf_index].status;
	int trncon_index = 0xFFFF;
	int trnloc_index = 0xFFFF;
	int m3acon_index = 0xFFFF;
	int sigtran_user_index = 0xFFFF;
	int netcs_index = 0xFFFF;

	ErrorTraceHandle(2, "mml_sccpcf(ss7con-%d)\n", sccpcf_index);


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
					// get m3acon over dest (assume that no mrsi's are defined for same DEST over two different SAIDs)
					MML_1KEY_get_index(m3acon, dest, mml.Csccpcf.par[sccpcf_index].Psp, m3acon_index);
					if ((m3acon_index == 0xFFFF) || (mml.Cm3acon.par[m3acon_index].status != started)) {
						mml_send_line("No command m3acon started with sp <%s>.", mml.Csccpcf.par[sccpcf_index].Psp);
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
					// ********************************************************************************

					NETCS_INDEX_KEY(netcs_index, sctp, unknown, netcs_sigtran, trnloc, trnloc_index, trncon, trncon_index);
					SIGTRAN_USER_INDEX_KEY(sigtran_user_index, netcs_index, sigtran_m3ua, m3acon_index);

					ErrorTraceHandle(2, "mml_sccpcf() sccpcf-%d, netcs_index-%d/sigtran_user_index-%d: m3acon-%d, trncon-%d, trnloc-%d\n",
									 sccpcf_index,
									 netcs_index,
									 sigtran_user_index,
									 ((struct Sm3ua_user *) sigtran_user[sigtran_user_index].pUserAdaptation)->mml_m3acon_index,
									 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trncon_index,
									 ((struct SSigtran_userof_NetCS_DataSet *) NetCS_DataSet[netcs_index].puser_data)->mml_trnloc_index);

					mml.Csccpcf.par[sccpcf_index].status = (enum Emml_status) started;
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
					mml.Csccpcf.par[sccpcf_index].status = (enum Emml_status) stopped;
					MML_EXECUTED;
				break;
				default:
					MML_IGNORED;
			} // switch status
		} // action stop
		break;

		case delete: {
			switch(status) {
				case stopped:
					MML_COMMAND_RESET_PAR(sccpcf, sccpcf_index);
					MML_EXECUTED;
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
				if (MML_PARAM_MASKS_CHECK(sccpcf, cmds)) {
					memcpy(tmp_param_mask, mml.Csccpcf.par[cmds].mask, sizeof(tmp_param_mask));
					mml_send_string("sccpcf(%d):", cmds);
					first=true;
					for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {
						if (tmp_param_mask[new_enum/32]&1) {
							if (!first) mml_send_string(", ");
							switch ((enum Emml_command_parameters) new_enum) {
							case sp:  mml_send_string("sp=2-%s",mml.Csccpcf.par[cmds].Psp);  break;
							case ssn: mml_send_string("ssn=%s", mml.Csccpcf.par[cmds].Pssn); break;
							case info:mml_send_string("info=\"%s\"",mml.Csccpcf.par[cmds].Pinfo);break;
							}
							if (first) first=false;
						}
						tmp_param_mask[new_enum/32]>>=1;
					}
					if (mml.Csccpcf.par[cmds].status == started)
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

