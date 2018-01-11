
#undef PREDEF
#ifdef FTP
#define PREDEF
#else
#define PREDEF extern
#endif

PREDEF int STOR_BUFSIZE_set;

PREDEF void Download_Folder(char *host_name, char *folder_name);
PREDEF int  Analyse_GlobalBuffer_with_ls_Ra(char (*Cmds)[][MINIBUF]);

PREDEF void FTP_Init(void);
PREDEF void FTP_flushCmds(char *remotehost_ip, char(*Cmds)[][MINIBUF]);
PREDEF void FTP_SelectPutfile(void);
PREDEF void FTP_SelectGetdir(void);
PREDEF void FTP_SelectTest1();
