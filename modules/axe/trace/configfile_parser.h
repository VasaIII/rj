
#undef PREDEF
#ifdef YACC
#define PREDEF
#else
#define PREDEF extern
#endif

#define yyerror   	yytraceerror
#define yydebug		yytracedebug
#define yywrap		yytracewrap
#define yys			yytraces
#define yyv			yytracev
#define	yylval		yytracelval

extern FILE *yyin;

enum 	projectDataConfigEnum
{
	eUSERNAME,
	ePASSWORD,
	eLOCALFILENAMEDIR,
	eTRACEPREFIXNAME,
	eTRACECONFIGFILENAME
};

enum 	projectTraceEnum
{
	eHEADING,
	eVARIABLE_MASK,
	eVARIABLE_NAME,
	eTIMESLICE,
	eCONVERTBASE,
	eINDEXTYPE,
	eVARIABLE
};

PREDEF bool yyFillKeyStruct(int eprojectTraceEnum, char *parsestring1, char *parsestring2);

PREDEF int  yyProjectDataConfigMask;
PREDEF char yyActiveBracketUnknownString[MINIBUF];
PREDEF int  yynIndex;
PREDEF char yyparseFileName[MINIBUF];
PREDEF int  yyNetCS_set;

PREDEF int  yyinput_analyse_stop0_file1_buffer2;


