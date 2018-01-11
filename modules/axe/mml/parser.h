
#undef PREDEF
#ifdef AXE_MMLPARSER
#define PREDEF
#else
#define PREDEF extern
#endif

#define yyerror   	yymmlerror
#define yywrap		yymmlwrap
#define yys			yymmls
#define yyv			yymmlv
#define yyparse     yymmlparse
#define yylex       yymmllex
#define yyerror     yymmlerror
#define yydebug     yymmldebug
#define yynerrs     yymmlnerrs
#define yylval      yymmllval
#define yychar      yymmlchar

// keep mml parsing data only within parser.c to separate already configured ongoing actions
// from deleting and updating of various mml combinations

enum Emml_commands {
	nocmd,
	// debugging and application commands
	mmlcnf, stats,
	// c7 signalling
	ss7loc, ss7con,
	// sctp
	trnloc, trncon,
	// m3ua
	m3acon, m3loop,
	// SCCP
	sccpcf,
	// BSS
	bsci, btsi, mei, call,
	// PSTN
	phonei,

	Eparse_select_struct,
	dummy
};

// Design rules
// - since this enum is used as mask (mml.C##cmd.par[mml.C##cmd.counter].mask), i'm limited to 32 parameters
// - when using print, parameter print starts from 0 so it is good to put key parameter enums before other ones
enum Emml_command_parameters {
	nopar,
	// c7 signalling
	ownsp, sp, sptype, net, si, ni, sls,
	// sctp
	epid, said, lip, lpn, rip, rpn, mode,
	// m3ua
	dest, bmode, trn, trnid,
	trnid1, dest1, ownsp1, trnid2, dest2, ownsp2, dir, cicinc1, cicinc2,
	// sccp
	ssn,
	// bss
	id, bsc, cell,
	mscsp, cgi_mcc, cgi_mnc, cgi_lac, cgi_ci,
	me, imsi, range,
	caller, called, type, cps, duration, cic,
	phone, Anum, Bnum, Bdest,

	info, user,

	Emml_command_parameters_MAX_VALUE
};

enum Emml_status {
	stopped, 	// as soon as created it is stopped, no mml_() function needed to confirm this
				// delete  can only be in stopped status and then there is no need for dependency check
	started 	// if status goes from started to stopped:
				//  * check upper/lower dependencies
				//  * no need to verify all indexes since they should be checked when status becomes started
				//    and dependent indexes cannot be stopped due to dependency check and
				//    in status starte, update of parameters is not allowed, this is ensured in
				//    GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE
};

enum Emml_action {
	noaction,
	start,
	stop,
	delete,
	print
};

#define PARSER_MML_PARAM_MAX_NUMBER_MASK	0xF
#define PARSER_MML_MAX_MASKS				3
#define MML_PARAM_MASKS_CHECK(cmd, cnt)			(mml.C##cmd.par[cnt].mask[0] || \
												 mml.C##cmd.par[cnt].mask[1] || \
												 mml.C##cmd.par[cnt].mask[2])
#define MML_PARAM_MASKS_EQUAL(cmd, cnt1, cnt2)	((mml.C##cmd.par[cnt1].mask[0]==mml.C##cmd.par[cnt2].mask[0]) && \
												 (mml.C##cmd.par[cnt1].mask[1]==mml.C##cmd.par[cnt2].mask[1]) && \
												 (mml.C##cmd.par[cnt1].mask[2]==mml.C##cmd.par[cnt2].mask[2]))
#define MML_COMMAND_RESET_PAR(cmd, cnt)			{memset(&mml.C##cmd.par[cnt], 0, sizeof(mml.C##cmd.par[cnt]));}
#define MML_COMMAND_RESET_CMD(cmd)				{memset(&mml.C##cmd, 0, sizeof(mml.C##cmd));}

#define MML_EXECUTED 	{mml_send_line("Command executed\r\n"); return;}
#define MML_IGNORED 	{mml_send_line("Command ignored\r\n"); return;}


// MML Design Rules:
// - let the command deletion be connected to one upper level so that each stabile part like netcs and sigtran have always defined lower part
//   and that deletion can go only from top
// - update of same command key parameters can in some commands be usefull (like "call" with "start" and "stop") and in some
//   unusefull without deleting and restarting (like "trncon" with different "lip")
// - do not use mml values outside parser.c, in that case if someone changes command, it will not affect ongoing traffic, only delete should affect it
// - mml's are named with prefix C and P since preprocessor concatenation directives can'n join
//   something like mml.##en.counter, but they can join mml.C##en.counter

#define DEFAULT_ADDITIONS \
		char Pinfo[100]; \
		char Pdummy[1]; \
		unsigned int mask[PARSER_MML_MAX_MASKS]; \
		unsigned char status; // enum Emml_action


typedef struct {

	struct Smmlcnf {
		struct Smmlcnf_parameters {
			char Plip[100];
			char Plpn[100];
			char Ptrn[10];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
		pthread_t thread_id;
	} Cmmlcnf;

	struct Sstats {
		struct Sstats_parameters {
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cstats;



	struct Sss7loc {
		struct Sss7loc_parameters {
			char Pownsp[100];
			char Psptype[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Css7loc;

	struct Sss7con {
		struct Sss7con_parameters {
			char Pownsp[100];
			char Psp[100];
			char Pnet[100];
			char Psi[100];
			char Pni[100];
			char Psls[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Css7con;




	struct Strnloc {
		struct Strnloc_parameters {
			char Pepid[100];
			char Plip[100];
			char Plpn[100];
			char Pmode[100];
			char Puser[100];
			char Ptrn[10];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Ctrnloc;

	struct Strncon {
		struct Strncon_parameters {
			char Psaid[100];
			char Pepid[100];
			char Prip[100];
			char Prpn[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Ctrncon;




	struct Sm3acon {
		struct Sm3acon_parameters {
			char Ptrnid[100];
			char Pdest[100];
			char Pbmode[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cm3acon;

	struct Sm3loop {
		struct Sm3loop_parameters {
			char Pownsp1[100];
			char Pdest1[100];
			char Ptrnid1[100];
			char Pownsp2[100];
			char Pdest2[100];
			char Ptrnid2[100];
			char Pdir[100];
			char Pcicinc1[100];
			char Pcicinc2[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cm3loop;





	struct Ssccpcf {
		struct Ssccpcf_parameters {
			char Psp[100];
			char Pssn[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Csccpcf;

	struct Sbsci {
		struct Sbsci_parameters {
			char Pownsp[100];
			char Pmscsp[100];
			char Pbsc[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cbsci;

	struct Sbtsi {
		struct Sbtsi_parameters {
			char Pbsc[100];
			char Pcell[100];
			char Pcgi_mcc[100];
			char Pcgi_mnc[100];
			char Pcgi_lac[100];
			char Pcgi_ci[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cbtsi;

	struct Smei {
		struct Smei_parameters {
			char Pme[100];
			char Pimsi[100];
			char Prange[100];
			char Pbsc[100];
			char Pcell[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cmei;

	struct Sphonei {
		struct Sphonei_parameters {
			char Pphone[100];
			char PAnum[100];
			char PBnum[100];
			char PBdest[100];
			char Prange[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Cphonei;

	struct Scall {
		struct Scall_parameters {
			char Pid[100];
			char Pcaller[100];
			char Pcalled[100];
			char Ptype[100];
			char Pcps[100];
			char Pduration[100];
			char Pcic[100];
			char Puser[100];
			DEFAULT_ADDITIONS
		} par[PARSER_MML_PARAM_MAX_NUMBER_MASK+1];
		int counter;
	} Ccall;

	int active_cmd;					// comdand that is currently parsing
	int active_cmd_par_index;		// after all parameters are parsed, this contains command parameter set that is determined as one for updating or new one
	int active_cmd_par_action;		// action to be preformed on active command, which should result in changing of command parameter set status
	bool execution_based_on_cmd_id;
	unsigned int execution_based_on_cmd_id_value;
} Smml_data;
PREDEF Smml_data mml;

PREDEF void mml_init_data(void);
PREDEF void *mml_thread_parse(void *arg);
struct pthread_data_mml { // pthread can send only one arg in function
	int netcs_index;
};
PREDEF struct pthread_data_mml send_over_mml;
#define PTHREAD_SEND_OVER_MML(val_netcs_index) \
	send_over_mml.netcs_index = val_netcs_index;

#define GENERIC_CASE_CMD_START_RESET_PARAM(cmd) \
		case cmd: { \
			memset(&mml.C##cmd.par[mml.C##cmd.counter], 0, sizeof(mml.C##cmd.par[mml.C##cmd.counter])); \
		} break;

#define MML_1KEY_get_index(cmd, key1, str1, key_index) \
	{ int count; key_index = 0xFFFF; \
		if (strlen(str1)!=0) \
		for (count=0; count<PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) { \
			  FAST6ErrorTraceHandle(3, "MML_1KEY_get_index: %s->%s: value <%s> with %s[%d]\n", #cmd, #key1, str1, mml.C##cmd.par[count].P##key1, count); \
			  if (MML_PARAM_MASKS_CHECK(cmd, count)) \
				  if (!strncmp(str1, mml.C##cmd.par[count].P##key1, strlen(str1))) \
				  { key_index=count; \
					FAST4ErrorTraceHandle(3, "MML_1KEY_get_index: %s-%d->%s match !\n", #cmd, key_index, #key1); \
					break; } \
		} \
		if (key_index == 0xFFFF) \
			FAST4ErrorTraceHandle(2, "MML_1KEY_get_index: There is no %s that has %s with <%s>!\n", #cmd, #key1, str1); \
	}

#define MML_2KEY_get_index(cmd, key1, str1, key2, str2, key_index) \
	{ int count; key_index = 0xFFFF; \
	if ((strlen(str1)!=0) && (strlen(str2)!=0)) \
	for (count=0; count<=PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) { \
		  FAST10ErrorTraceHandle(3, "MML_2KEY_get_index: %s-%d->%s,%s <<%s> & <%s>> with <<%s> & <%s>>-%d\n", #cmd, count, #key1, #key2, str1, str2, mml.C##cmd.par[count].P##key1, mml.C##cmd.par[count].P##key2, count); \
		  if (MML_PARAM_MASKS_CHECK(cmd, count)) \
			  if ((!strncmp(str1, mml.C##cmd.par[count].P##key1, strlen(str1))) && \
				  (!strncmp(str2, mml.C##cmd.par[count].P##key2, strlen(str2)))) \
			  { key_index=count; \
			    FAST5ErrorTraceHandle(3, "MML_2KEY_get_index: %s-%d->%s,%s match !\n", #cmd, key_index, #key1, #key2); \
				break; } } }





#define GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE(cmd) \
	if (key_index==0xFFFF) { /*new command data*/\
		mml.active_cmd_par_index = mml.C##cmd.counter; /* choose this new index */\
		FAST3ErrorTraceHandle(3, "GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE: No mml %s with same key located, taken new one %d.\n", #cmd, mml.active_cmd_par_index); \
	} else { /*already defined command data*/\
		int new_enum;\
		if (mml.C##cmd.par[key_index].status == stopped) {\
			for (new_enum=0;new_enum<((enum Emml_command_parameters) Emml_command_parameters_MAX_VALUE);new_enum++) {\
				if (mml.C##cmd.par[mml.C##cmd.counter].mask[new_enum/32]&1) { \
					generic_string_parameters(key_index, new_enum, (char *) NULL); /*update only new parameters, other leave as they were*/\
				} \
				mml.C##cmd.par[mml.C##cmd.counter].mask[new_enum/32]>>=1; /*latest counter data is whatsoever not used, so we can shift this mask*/\
			}\
		} else {\
			FAST3ErrorTraceHandle(3, "GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE: mml %s with same key located at %d, parameter update not allowed if status is not stopped.\n", #cmd, mml.active_cmd_par_index); \
		} \
		MML_COMMAND_RESET_PAR(cmd, mml.C##cmd.counter); /* reset new index which is not needed any more */ \
		mml.active_cmd_par_index = key_index; \
		FAST4ErrorTraceHandle(3, "GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE: mml %s with same key located at %d with status %d\n", #cmd, mml.active_cmd_par_index, mml.C##cmd.par[key_index].status); \
	} \
	count=0; key_index=0xFFFF; \
	while (count!=PARSER_MML_PARAM_MAX_NUMBER_MASK) {if(!MML_PARAM_MASKS_CHECK(cmd, count)) {key_index=count; break;} count++; } \
	mml.C##cmd.counter = key_index; \
	FAST4ErrorTraceHandle(3, "GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE: mml %s index %d, next free index is %d\n", #cmd, mml.active_cmd_par_index, mml.C##cmd.counter); \


#define GENERIC_CASE_CMD_END_ALIGN_DATA_TO_1KEY_INDEX(cmd, key1) \
		case cmd: { \
			int count, key_index = 0xFFFF; \
			for (count=0; count<PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) { \
				if (MML_PARAM_MASKS_CHECK(cmd, count) && (count!=mml.C##cmd.counter)) \
				  if (!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key1, \
							   mml.C##cmd.par[count].P##key1, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key1))) \
							   { key_index=count; break;} \
			} \
			GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE(cmd); \
		} \
		break;

#define GENERIC_CASE_CMD_END_ALIGN_DATA_TO_2KEY_INDEX(cmd, key1, key2) \
		case cmd: { \
			int count, key_index = 0xFFFF; \
			for (count=0; count<PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) { \
				if (MML_PARAM_MASKS_CHECK(cmd, count) && (count!=mml.C##cmd.counter)) \
				  if ((!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key1, \
							    mml.C##cmd.par[count].P##key1, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key1))) && \
					  (!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key2, \
							    mml.C##cmd.par[count].P##key2, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key2)))) \
							    { key_index=count; break;} \
			} \
			GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE(cmd); \
		} \
		break;

#define GENERIC_CASE_CMD_END_ALIGN_DATA_TO_3KEY_INDEX(cmd, key1, key2, key3) \
		case cmd: { \
			int count, key_index = 0xFFFF; \
			for (count=0; count<PARSER_MML_PARAM_MAX_NUMBER_MASK; count++) { \
				if (MML_PARAM_MASKS_CHECK(cmd, count) && (count!=mml.C##cmd.counter)) \
				  if ((!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key1, \
							    mml.C##cmd.par[count].P##key1, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key1))) && \
					  (!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key2, \
								mml.C##cmd.par[count].P##key2, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key2))) && \
					  (!strncmp(mml.C##cmd.par[mml.C##cmd.counter].P##key3, \
								mml.C##cmd.par[count].P##key3, strlen(mml.C##cmd.par[mml.C##cmd.counter].P##key3)))) \
								{ key_index=count; break;} \
			} \
			GENERIC_CASE_CMD_END_ALIGN_DATA_TO_KEY_INDEX_CONCLUDE(cmd); \
		} \
		break;

#define GENERIC_CASE_PARAM1_SELECT( cmd, par1) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM2_SELECT( cmd, par1, par2) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM3_SELECT( cmd, par1, par2, par3) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, dummy,dummy,dummy,dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM4_SELECT( cmd, par1, par2, par3, par4) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, dummy,dummy,dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM5_SELECT( cmd, par1, par2, par3, par4, par5) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, dummy,dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM6_SELECT( cmd, par1, par2, par3, par4, par5, par6) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, par6, dummy,dummy,dummy,dummy)
#define GENERIC_CASE_PARAM7_SELECT( cmd, par1, par2, par3, par4, par5, par6, par7) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, par6, par7, dummy,dummy,dummy)
#define GENERIC_CASE_PARAM8_SELECT( cmd, par1, par2, par3, par4, par5, par6, par7, par8) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, par6, par7, par8, dummy,dummy)
#define GENERIC_CASE_PARAM9_SELECT( cmd, par1, par2, par3, par4, par5, par6, par7, par8, par9) \
		GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, par6, par7, par8, par9, dummy)
#define GENERIC_CASE_PARAM10_SELECT(cmd, par1, par2, par3, par4, par5, par6, par7, par8, par9, par10) \
		case cmd: { /* single enum_param is updated in this case */ \
			/*str_value==NULL -> case when creating new parameter data where new parsing command is creating,
			 *str_value!=NULL -> case when updating only modified parameter data if new command ended up as already existing  */ \
			if (str_value!=NULL) mml.C##cmd.par[mml.C##cmd.counter].mask[(enum_param/32)] |= (1<<(enum_param%32)); \
				else mml.C##cmd.par[key_index].mask[(enum_param/32)] |= (1<<(enum_param%32)); \
			if ((enum Emml_command_parameters) enum_param == par1) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par1, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par1, mml.C##cmd.par[mml.C##cmd.counter].P##par1); break;}\
			if ((enum Emml_command_parameters) enum_param == par2) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par2, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par2, mml.C##cmd.par[mml.C##cmd.counter].P##par2); break;}\
			if ((enum Emml_command_parameters) enum_param == par3) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par3, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par3, mml.C##cmd.par[mml.C##cmd.counter].P##par3); break;}\
			if ((enum Emml_command_parameters) enum_param == par4) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par4, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par4, mml.C##cmd.par[mml.C##cmd.counter].P##par4); break;}\
			if ((enum Emml_command_parameters) enum_param == par5) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par5, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par5, mml.C##cmd.par[mml.C##cmd.counter].P##par5); break;}\
			if ((enum Emml_command_parameters) enum_param == par6) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par6, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par6, mml.C##cmd.par[mml.C##cmd.counter].P##par6); break;}\
			if ((enum Emml_command_parameters) enum_param == par7) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par7, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par7, mml.C##cmd.par[mml.C##cmd.counter].P##par7); break;}\
			if ((enum Emml_command_parameters) enum_param == par8) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par8, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par8, mml.C##cmd.par[mml.C##cmd.counter].P##par8); break;}\
			if ((enum Emml_command_parameters) enum_param == par9) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par9, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par9, mml.C##cmd.par[mml.C##cmd.counter].P##par9); break;}\
			if ((enum Emml_command_parameters) enum_param == par10) {\
				if (str_value!=NULL) strcpy(mml.C##cmd.par[mml.C##cmd.counter].P##par10, str_value); \
					else strcpy(mml.C##cmd.par[key_index].P##par10, mml.C##cmd.par[mml.C##cmd.counter].P##par10); break;}\
		} break;


PREDEF int yyinput_analyse_stop0_file1_buffer2;
PREDEF int yyinput_analyse_buffer_set;
PREDEF int myyyinput(char *buf, int max_size);
PREDEF int yymmlparse (void);
PREDEF int yymmllex (void);

PREDEF bool mml_send_string(char *format, ...);
PREDEF bool mml_send_line(char *format, ...);
PREDEF void mml_init_data(void);
PREDEF bool mml_help();
PREDEF bool mml_dump();
PREDEF void mml_mmlcnf(void);
PREDEF bool mml_stats(FILE *fptrace);

PREDEF void NETCS_netcs_mml_udp_USER_INDEX_INIT(int id, int dummy1, int dummy2);
PREDEF void NETCS_netcs_mml_tcp_USER_INDEX_INIT(int id, int dummy1, int dummy2);
PREDEF void NETCS_netcs_mml_sctp_USER_INDEX_INIT(int id, int dummy1, int dummy2);
PREDEF bool NETCS_netcs_mml_udp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_netcs_mml_tcp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);
PREDEF bool NETCS_netcs_mml_sctp_USER_INDEX_CMP(int id, int key_mml_trnloc_index, int key_dummy);

