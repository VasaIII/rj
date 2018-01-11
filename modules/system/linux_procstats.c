#include <signal.h>

#include <dirent.h>

#include <termcap.h> // in debian for temcap.h do: apt-get install ncurses-dev
#include <termios.h>

#include <unistd.h> // _SC_NPROCESSORS_CONF

#include <modules/common.h>
#include <modules/system/linux_sysinfo.h>


// Some functions and structures are taken from initial versions of
// top.c, readproc.c, readproc.h
// http://procps.cvs.sourceforge.net/procps/procps/?pathrev=MAIN&bcsi_scan_7D494D5FE7434A74=baOfqxInAkIAc+naIXSFEgsAAABjHf4E


	/* The original terminal attributes. */
static struct termios Savetty;
	/* The new terminal attributes. */
static struct termios Rawtty;
	/* Cached termcap entries. */
static char *cm, *cl, *top_clrtobot, *top_clrtoeol, *ho, *md, *me, *mr;
	/* Current window size.  Note that it is legal to set Display_procs
	   larger than can fit; if the window is later resized, all will be ok.
	   In other words: Display_procs is the specified max number of
	   processes to display (zero for infinite), and Maxlines is the actual
	   number. */
static int Lines, Cols, Maxlines, Display_procs;
	/* Maximum length to display of the command line of a process. */
static unsigned Maxcmd;

char *cl_string;


typedef struct proc_s {
    char
	/* Linux 2.1.7x and up have more signals. This handles 88. */
	signal[24],	/* mask of pending signals */
	blocked[24],	/* mask of blocked signals */
	sigignore[24],	/* mask of ignored signals */
	sigcatch[24];	/* mask of caught  signals */

    long
	cutime,		/* cumulative utime of process and reaped children */
	cstime,		/* cumulative stime of process and reaped children */
	priority,	/* kernel scheduling priority */
	timeout,	/* ? */
	nice,		/* standard unix nice level of process */
	rss,		/* resident set size from /proc/#/stat (pages) */
	it_real_value,	/* ? */
    /* the next 7 members come from /proc/#/statm */
	size,		/* total # of pages of memory */
	resident,	/* number of resident set (non-swapped) pages (4k) */
	share,		/* number of pages of shared (mmap'd) memory */
	trs,		/* text resident set size */
	lrs,		/* shared-lib resident set size */
	drs,		/* data resident set size */
	dt;		/* dirty pages */

    unsigned long
	/* FIXME: are these longs? Maybe when the alpha does PCI bounce buffers */
	vm_size,        /* same as vsize in kb */
	vm_lock,        /* locked pages in kb */
	vm_rss,         /* same as rss in kb */
	vm_data,        /* data size */
	vm_stack,       /* stack size */
	vm_exe,         /* executable size */
	vm_lib,         /* library size (all pages, not just used ones) */
	vsize,		/* number of pages of virtual memory ... */
	rss_rlim,	/* resident set size limit? */
	flags,		/* kernel flags for the process */
	min_flt,	/* number of minor page faults since process start */
	maj_flt,	/* number of major page faults since process start */
	cmin_flt,	/* cumulative min_flt of process and child processes */
	cmaj_flt,	/* cumulative maj_flt of process and child processes */
	nswap,		/* ? */
	cnswap,		/* cumulative nswap ? */
	utime,		/* user-mode CPU time accumulated by process */
	stime,		/* kernel-mode CPU time accumulated by process */
	start_code,	/* address of beginning of code segment */
	end_code,	/* address of end of code segment */
	start_stack,	/* address of the bottom of stack for the process */
	kstk_esp,	/* kernel stack pointer */
	kstk_eip,	/* kernel instruction pointer */
	start_time,	/* start time of process -- seconds since 1-1-70 */
	wchan;		/* address of kernel wait channel proc is sleeping in */

    struct proc_s *l,	/* ptrs for building arbitrary linked structs */
                  *r;	/* (i.e. singly/doubly-linked lists and trees */

    char
	**environ,	/* environment string vector (/proc/#/environ) */
	**cmdline;	/* command line string vector (/proc/#/cmdline) */

    char
	/* Be compatible: Digital allows 16 and NT allows 14 ??? */
    	ruser[16],	/* real user name */
    	euser[16],	/* effective user name */
    	suser[16],	/* saved user name */
    	fuser[16],	/* filesystem user name */
    	rgroup[16],	/* real group name */
    	egroup[16],	/* effective group name */
    	sgroup[16],	/* saved group name */
    	fgroup[16],	/* filesystem group name */
    	cmd[16];	/* basename of executable file in call to exec(2) */

    int
        ruid, rgid,     /* real      */
        euid, egid,     /* effective */
        suid, sgid,     /* saved     */
        fuid, fgid,     /* fs (used for file access only) */
    	pid,		/* process id */
    	ppid,		/* pid of parent process */
	pgrp,		/* process group id */
	session,	/* session id */
	tty,		/* full device number of controlling terminal */
	tpgid,		/* terminal process group id */
	exit_signal,	/* might not be SIGCHLD */
	processor;      /* current (or most recent?) CPU */

    unsigned
        pcpu;           /* %CPU usage (is not filled in by readproc!!!) */

    char
    	state;		/* single-char code for process state (S=sleeping) */
} proc_t;

DIR*	procfs;

static void stat2proc(char* S, proc_t* P) {
	int num;
	char* tmp = strrchr(S, ')'); /* split into "PID (cmd" and "<rest>" */
	*tmp = '\0'; /* replace trailing ')' with NUL */
	/* fill in default values for older kernels */
	P->exit_signal = SIGCHLD;
	P->processor = 0;
	/* parse these two strings separately, skipping the leading "(". */
	memset(P->cmd, 0, sizeof P->cmd); /* clear even though *P xcalloc'd ?! */
	sscanf(S, "%d (%15c", &P->pid, P->cmd); /* comm[16] in kernel */
	num = sscanf(tmp + 2, /* skip space after ')' too */
	"%c "
		"%d %d %d %d %d "
		"%lu %lu %lu %lu %lu %lu %lu "
		"%ld %ld %ld %ld %ld %ld "
		"%lu %lu "
		"%ld "
		"%lu %lu %lu %lu %lu %lu "
		"%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
		"%lu %lu %lu "
		"%d %d", &P->state, &P->ppid, &P->pgrp, &P->session, &P->tty,
			&P->tpgid, &P->flags, &P->min_flt, &P->cmin_flt, &P->maj_flt,
			&P->cmaj_flt, &P->utime, &P->stime, &P->cutime, &P->cstime,
			&P->priority, &P->nice, &P->timeout, &P->it_real_value,
			&P->start_time, &P->vsize, &P->rss, &P->rss_rlim, &P->start_code,
			&P->end_code, &P->start_stack, &P->kstk_esp, &P->kstk_eip,
			/*     P->signal, P->blocked, P->sigignore, P->sigcatch,   *//* can't use */
			&P->wchan, &P->nswap, &P->cnswap,
			/* -- Linux 2.0.35 ends here -- */
			&P->exit_signal, &P->processor /* 2.2.1 ends with "exit_signal" */
	/* -- Linux 2.2.8 and 2.3.47 end here -- */
	);

	/* fprintf(stderr, "stat2proc converted %d fields.\n",num); */
	if (P->tty == 0)
		P->tty = -1; /* the old notty val, update elsewhere bef. moving to 0 */
}

#define term_buffer 0
#define BUFFADDR 0

void init_terminal_data() {
	char *termtype = getenv("TERM");
	int success;

	int height;
	int width;
	int auto_wrap;

	char PC;   /* For tputs.  */
	char *BC;  /* For tgoto.  */
	char *UP;

	char *temp;

	if (termtype == 0)
		printf("Specify a terminal type with `setenv TERM <yourtype>'.\n");

	success = tgetent(term_buffer, termtype);
	if (success < 0)
		printf("Could not access the termcap data base.\n");
	else if (success == 0)
		printf("Terminal type `%s' is not defined.\n", termtype);

	/* Extract information we will use.  */
	cl_string = tgetstr("cl", BUFFADDR);
	height = tgetnum("li");
	width = tgetnum("co");

	/* Extract information that termcap functions use.  */
	temp = tgetstr("pc", BUFFADDR);
	PC = temp ? *temp : 0;
	BC = tgetstr("le", BUFFADDR);
	UP = tgetstr("up", BUFFADDR);
}

void Procstat_SelectProcStats (int Pid)
{
	proc_t *p = NULL;
	static struct dirent *ent; /* dirent handle */
	char tmpBuf[500];
	char sPid[20];
	char sPid_name[50] = " dummy";
	char statBuf[512];
	off_t procfs_offset_start;
	int pid_locked = 0;

	init_terminal_data();

	sprintf(sPid, "%d", Pid);

	if ((procfs = opendir("/proc")) == NULL)
	{
		ErrorTraceHandle(0, "Procstat_SelectProcStats() - opendir.\n");
	}

	procfs_offset_start = telldir(procfs);

	while(1)
	{
		struct timespec ts;
		float elapsTime;

		ts.tv_sec = 3;
	    ts.tv_nsec = 0; //10*1000000;
	    nanosleep(&ts, NULL);

	    elapsTime = GetElapsedTimeSinceLastCallUS();
	    ErrorTraceHandle(1, "%sProcess %s (PID=%d) data: measurement interval [%f], ",
	    		cl_string, &sPid_name[1], Pid, elapsTime);

	    seekdir(procfs, procfs_offset_start);

		while (ent = readdir(procfs))
		{
			if (!(*ent->d_name < '0' || *ent->d_name> '9'))
			{
				if (!ent || !ent->d_name)
				{
					ErrorTraceHandle(0, ".\n");
				}
				else if (!strcmp(sPid, ent->d_name))
				{
					int target_process_ticks_diff;
					int target_process_ticks_new;
					static int target_process_ticks_old = 0;
					int sleeping = 0, stopped = 0, zombie = 0, running = 0;
					double system_ticks, user_ticks, nice_ticks, idle_ticks;
					int n = 0;
					static char *top_clrtoeol;

					pid_locked = 1;
					// Read process stat file into buffer
					sprintf(tmpBuf, "/proc/%s", sPid);
					PrimitiveFile2Buffer(tmpBuf, "stat", statBuf, sizeof(statBuf));
					//printf("Targeted /proc/%s\n", ent->d_name);
					//printf("Content <%s>\n", statBuf);

					// Interpret string buffer to proc_t structure variables
					p = malloc(sizeof *p);
					stat2proc(statBuf, p);
					//printf("Content utime=%d, stime=%d\n", p->utime, p->stime);

					// Calculate delta ticks from previous calculation
					target_process_ticks_new = p->utime + p->stime;
					target_process_ticks_diff = target_process_ticks_new - target_process_ticks_old;
					target_process_ticks_old = target_process_ticks_new;
					//printf("Total %d ticks\n", target_process_ticks_diff);

					/*
					 *   From cpu_load.c:
						 1 HZ      =   100 ticks
						 5 HZ      =   500 ticks
						 1 tick    =    10 milliseconds
						 500 ticks =  5000 milliseconds (or 5 seconds)

						 10ms=1tick -> 1sec=100ticks
					 */

					// Adjust delta ticks according to clock and time elapsed
					p->pcpu = (target_process_ticks_diff * 10 * 100 / sysconf(_SC_CLK_TCK)) / elapsTime;
					//printf("Hertz      %d\n", sysconf(_SC_CLK_TCK));

					strcpy(sPid_name, GetWord(statBuf, 0, 0, 1));
					printf("CPU usage %# 4.1f%%\n",
								(float) p->pcpu/10);

					//printf("%d processes: %d sleeping, %d running, %d zombie, %d stopped\n", n, sleeping, running, zombie, stopped);
					four_cpu_numbers(&user_ticks, &nice_ticks, &system_ticks, &idle_ticks);
					printf("System CPU states:"
								" %# 5.1f%% user, %# 5.1f%% system,"
								" %# 5.1f%% nice, %# 5.1f%% idle\n", user_ticks, system_ticks,
								nice_ticks, idle_ticks);


				}
//				else
//				ErrorTraceHandle(1, "Other /proc/%s\n", ent->d_name);
			}
		}
	}

	closedir(procfs);
}








