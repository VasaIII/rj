
// since local heading files have these PREDEF parts, include them at beginning or end of another local heading files
#include <modules/system/statistics.h>

#undef PREDEF
#ifdef RJ_COMMON
#define PREDEF
#else
#define PREDEF extern
#endif

#include <stdio.h>
#include <stdlib.h>  // random
#include <stdarg.h>
#include <string.h>  // memset
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>  // read, close
#include <fcntl.h>   // open
#include <poll.h>    // pollfd
#include <time.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

//#include <linux/jiffies.h> // timespec_to_jiffies
//#include <linux/preempt.h>
//#include <linux/spinlock_types.h>
#include <termios.h> // ioctl

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <netdb.h>	 // struct sockaddr_in

#include <unistd.h> // getpass, alarm, getpid

#include <pthread.h>
// If successful, the pthread_create() function returns zero    -> ptherad_exit(NULL)
// Otherwise, an error number is returned to indicate the error -> static int ret = xxx; ptherad_exit(&ret)
#include <sched.h>   // pthread extended configuration

#define boolean  bool
#define BOOL     bool
#define FALSE    false
#define TRUE     true

#define MINIBUF  500	// max file name length is 255
#define MAXIBUF  50000

#ifdef RJ_COMMON
PREDEF int DEBUG_on_ERROR_mode 				= 0;
PREDEF int DEBUG_on_TRACE_mode 				= 0;
PREDEF int errortrace_printout_mode 		= 1;   	// if unable to create trace error file, don't keep trying
PREDEF int info_printout_mode 				= 1;   	// if unable to create trace error file
PREDEF int (*ErrorTraceHandle0dump)(FILE *fptrace) = NULL;	// pointer to function containing dumps on ERROR
#else
PREDEF int DEBUG_on_ERROR_mode;
PREDEF int DEBUG_on_TRACE_mode;
PREDEF int errortrace_printout_mode;
PREDEF int info_printout_mode;
PREDEF int (*ErrorTraceHandle0dump)(FILE *fptrace);
#endif

//#define FAST

#ifdef RJ_COMMON

// ***********************************************************************************
//
// This define is used for trace which slows down execution of code
//#define ErrorTraceHandleTEST
//
// ***********************************************************************************
void ErrorTraceHandle(int Error0_Trace12345, char *format, ...) {
	char msg[MAXIBUF];
	char msg_out[MAXIBUF];
	int i;
	va_list list;
	FILE *fp, *fptrace;

	int TraceActive[6] = { 	1, // 1 = User printout
							1, // 2 = Troubleshoot printout
							1, // 3 = Troubleshoot printout (other)
						 	1, // 4 = Troubleshoot printout (large outputs and other)
						   	1, // 5 = Troubleshoot printout (other - signalling messages)
						   	0 };

	int TraceToFileActive[6] = { 	1, // 1 = User printout
								1, // 2 = Troubleshoot printout
								1, // 3 = Troubleshoot printout (other)
								1, // 4 = Troubleshoot printout (large outputs and other)
								1, // 5 = Troubleshoot printout (other)
								 0 };

	if (Error0_Trace12345 == 0) {
		perror(NULL);

		va_start(list, format);
		vsprintf(msg, format, list);
		strcpy(msg_out, "FAULT ! ");
		strcat(msg_out, msg);
		fprintf(stdout, "%s", msg_out);

		if (DEBUG_on_ERROR_mode) {
			if (errortrace_printout_mode) // if able to create file
			{
				if ((fptrace = fopen("ErrorTrace_File.txt", "a+")) == NULL) {
					fprintf(stdout, "Unable to create trace error/trace file.\n");
					errortrace_printout_mode = 0;
				} else {
					fputs(msg_out, fptrace);
					if (ErrorTraceHandle0dump!=NULL) (*ErrorTraceHandle0dump)(fptrace);

					fclose(fptrace);
				}
			}
		} // DEBUG_on_ERROR_mode

		exit(0);
	} else {
		if (TraceActive[Error0_Trace12345 - 1]) {
			va_start(list, format);
			vsprintf(msg, format, list);
			fprintf(stdout, "%s", msg);
			//fprintf(stdout, "\n");
		}

		if (DEBUG_on_TRACE_mode) {
			if (TraceToFileActive[Error0_Trace12345 - 1]) {
				if (errortrace_printout_mode) // if able to create file
				{
					va_start(list, format);
					vsprintf(msg, format, list);

					if ((fptrace = fopen("ErrorTrace_File.txt", "a+")) == NULL) {
						fprintf(stdout,	"Unable to create trace error/trace file.\n");
						errortrace_printout_mode = 0;
					} else {
						sprintf(msg_out, "TRACE[%d] %s \n", Error0_Trace12345, msg);
						fputs(msg_out,  fptrace);
						fclose(fptrace);
					}
				}
			}
		} // DEBUG_on_TRACE_mode
	}
}
#else
PREDEF void ErrorTraceHandle(int Error0_Trace12345, char *format, ...);
#endif

#ifdef FAST
#define FAST1ErrorTraceHandle(Error0_Trace12345, d1) ;
#define FAST2ErrorTraceHandle(Error0_Trace12345, d1, d2) ;
#define FAST3ErrorTraceHandle(Error0_Trace12345, d1, d2, d3) ;
#define FAST4ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4) ;
#define FAST5ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5) ;
#define FAST6ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6) ;
#define FAST7ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7) ;
#define FAST8ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8) ;
#define FAST9ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9) ;
#define FAST10ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9,d10) ;
#else
#define FAST1ErrorTraceHandle(Error0_Trace12345, d1) ErrorTraceHandle(Error0_Trace12345, d1);
#define FAST2ErrorTraceHandle(Error0_Trace12345, d1, d2) ErrorTraceHandle(Error0_Trace12345, d1, d2);
#define FAST3ErrorTraceHandle(Error0_Trace12345, d1, d2, d3) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3);
#define FAST4ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4);
#define FAST5ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5);
#define FAST6ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6);
#define FAST7ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7);
#define FAST8ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8);
#define FAST9ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9);
#define FAST10ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9,d10) ErrorTraceHandle(Error0_Trace12345, d1, d2, d3, d4, d5, d6, d7, d8, d9,d10);
#endif

#ifndef _SOLARIS_
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#endif

PREDEF int  HexString2Int (char **s_p, int *result);
PREDEF void Int2BinaryString(int no);
PREDEF int  Pow (const int base, const int exp);

PREDEF int PrimitiveFile2Buffer(char *directory,    // INPUT:  directory to be readed, if NULL only filename will be considered
				char *file,          				// INPUT:  file to be readed
				char *return_buffer, 				// OUTPUT: pointer to already allocated MEMORY where file will be written
				int   buffer_size);  				// INPUT:  size of content to be readed

// OUTPUT: file size
PREDEF int File2Buffer(char *directory,      // INPUT:  directory to be readed, if NULL only filename will be considered
		       char *file,                   // INPUT:  file to be readed
		       char *buffp);                 // OUTPUT: pointer where this function will store and allocate MEMORY for file to be written

PREDEF char *File2PagedBuffer(
				char *directory,         // INPUT:  directory to be readed, if NULL only filename will be considered
			    char *file,              // INPUT:  file to be readed
			    int  *num_read,          // OUTPUT: pointer where this function will store and allocate MEMORY for file to be written
			    int  *Page,              // INPUT:  pointer to int[maxp][maxl] array where function will store pointers to pages and lines in buffp MEMORY
			    int   maxp, int maxl,    // INPUT:  max number of pages and lines in array
			    int  *PageCount,         // OUTPUT: number of pages created
			    int  *TotalLines);       // OUTPUT: total number of lines

PREDEF char *Buffer2PagedBuffer(
			 char *inbuffer,
		     int   inbuffer_size,
		     int  *num_read,
		     int  *Page,
		     int   maxp, int maxl,
		     int  *PageCount,
		     int  *TotalLines);

PREDEF float GetElapsedTimeSinceLastCallUS(void);

PREDEF short int    littletobig2(short int little    /* short int    = 2 bytes */);
PREDEF unsigned int littletobig4(unsigned int little /* unsigned int = 4 bytes */);

#define LITTLETOBIG(val,nobytes) littletobig##nobytes(val)

// ----------------------------------------------------------------------------------------
// TEXT BUFFER ANALYSIS (used by FTP)
//
// INPUT
PREDEF char *GetWord(char *analyse_buffer,
		     int   Req0_Array1, // either request single (Req0_Array1=0) word from matrics location (line, position)
		                        // or select to receive complete analysed matrics pointer (Req0_Array1=1).
		     int   line, int position);

// RESULT of analysis from GetWord() function call
#define MAX_LINE_LS             20000
PREDEF struct GetWord_ObjectMatrics {
  char word[255];            // word in matrics
  char offset;               // 0..255, offset in line
}GetWord_sObjectMatrics[MAX_LINE_LS][15];   // [number of strings][number of elements in row (max=10)]
                             // -rw-r--r--   1 etkfrbi  cellocc    33659 Jul  7 12:08 umeqmtservice.c
// ----------------------------------------------------------------------------------------

