
#================================================================ 
# Linux: 
# Linux suportira deklaraciju varijabli usred koda, Solaris ne
# Solaris:
# Use "alias make /usr/local/apstools/13.1_Solaris/lib/cse/make/make", 
# other may report some Makefile errors
#================================================================ 

OSTYPE = $(shell lsb_release -si)
ARCH   = $(shell uname -m | sed 's/x86_//;s/i[3-6]86/32/')
VER    = $(shell lsb_release -sr)

CC=gcc
GET=co

# Switches
CPP = NO

# Arguments used
BUILD_OBJ  = 
BUILD_SRC  = 
BUILD_FLAG = 
BUILD_INC  = 
RJ_VERSION = v13

RJ_HOME   = /vagrant/rj
# RJ_HOME = /home3/etkfrbi/bin/design/REMOTEjob_$(RJ_VERSION)
# RJ_HOME = /HOME/BETEETK/etkfrbi/rj
# RJ_HOME = /home/frbi/workspace/rj

LEX_COMPILE = FLEX_BISON
#LEX_COMPILE = LEX_YACC
# Bison is the GNU implementation/extension of Yacc, Flex is the successor of Lex
# In either case, it's fine (and recommended) to use bison / flex.


#================================================================ 
# For Threads
ifeq "$(OSTYPE)" "solaris"
	# For MultiThreaded applications (INF01) libpthread -> POSIX threads, libthread -> SOLARIS threads
	BUILD_FLAG := $(BUILD_FLAG) -lthread -lpthread -D_REENTRAN
else
	# On Linux to check version of PTHREAD: getconf GNU_LIBPTHREAD_VERSION
	BUILD_FLAG := $(BUILD_FLAG) -lpthread -D_REENTRANT
endif
#================================================================ 

#================================================================ 
# For RiscWatch:
ifeq "$(CPP)" "YES"
	# UAB: BUILD_OBJ = $(BUILD_OBJ) /opt/hde/RISCWatch/5.1/rwpi/rwpic.o
	BUILD_OBJ := $(BUILD_OBJ) /home4/etkcello/DDE/RiscWatch/5.1/rwpi/rwpic.o
	BUILD_SRC := $(BUILD_SRCS) modules/cpp/riscwatch.c modules/cpp/ultramapper.c
endif
#================================================================

#================================================================ 
# For terminal library on linux
BUILD_FLAG := $(BUILD_FLAG) -ltermcap
# in debian for temcap.h do: apt-get install ncurses-dev
#================================================================ 

#================================================================ 
# For sctp linux:
BUILD_FLAG := $(BUILD_FLAG) -lsctp
#================================================================ 

BUILD_SRC := $(BUILD_SRC) \
modules/axe/signalling.c \
modules/axe/mml/parser.c \
modules/axe/mml/parser.lex.yy.c \
modules/axe/mml/parser.y.tab.c \
modules/axe/mml/sections/sctp.c \
modules/axe/mml/sections/m3ua.c \
modules/axe/mml/sections/c7.c \
modules/axe/mml/sections/mobile.c \
modules/axe/tgen.c \
modules/axe/oscillo.c \
modules/axe/trace/trace.c \
modules/axe/trace/dataanalyse.c \
modules/axe/trace/configfile_parser.c \
modules/axe/trace/configfile_parser.lex.yy.c \
modules/axe/trace/configfile_parser.y.tab.c \
modules/axe/application/bss/bss_bsc.c \
modules/axe/application/ims/voip.c \
modules/axe/application/pstn/pstn.c \
modules/protocols/mobile/radio.c \
modules/protocols/mobile/sccp_bssap.c \
modules/protocols/mobile/sccp.c \
modules/protocols/ims/sip.c \
modules/protocols/trunk/isup.c \
modules/protocols/sigtran_m3ua.c \
modules/protocols/sigtran.c \
modules/protocols/ftp.c \
modules/common.c \
modules/nethelper.c


#================================================================ 
# Common for envirnments
# Solaris environment for CPP
ifeq "$(CPP)" "YES"
	# for Solaris without vendor-supplied libraries (INF02)
	#DIRS=-L/usr/openwin/lib -L/usr/lib -L/usr/dt/lib -L/usr/local/lib
	#INCL=-I/usr/openwin/include
	#LIBS=-lrpcsvc
	BUILD_FLAG := $(BUILD_FLAG) -lsocket -lnsl -D _SOLARIS_ -D SPLIT
else
	ifeq "$(OSTYPE)" "suse"
		# Suse environment for MSS I&V
		BUILD_FLAG := $(BUILD_FLAG) -lc -L/usr/lib64 -D _LINUX_ -D _SUSE_
		BUILD_SRC  := $(BUILD_SRC) \
				 	  modules/system/linux_sysinfo.c
	else 
		# Debian local PC 
		# Ubuntu virtual 
		BUILD_FLAG := $(BUILD_FLAG) -L/usr/lib -D _LINUX_
		BUILD_SRC  := $(BUILD_SRC) \
					  modules/system/linux_procstats.c \
					  modules/system/linux_sysinfo.c
	endif
endif
#================================================================ 


BUILD_INC= \
-I$(RJ_HOME) \
-I$(RJ_HOME)/modules \
-I$(RJ_HOME)/modules/system \
-I$(RJ_HOME)/modules/axe \
-I$(RJ_HOME)/modules/axe/application/bss \
-I$(RJ_HOME)/modules/axe/application/pstn \
-I$(RJ_HOME)/modules/axe/mml \
-I$(RJ_HOME)/modules/axe/trace \
-I$(RJ_HOME)/modules/cpp \
-I$(RJ_HOME)/modules/protocols \
-I$(RJ_HOME)/modules/protocols/mobile \
-I$(RJ_HOME)/modules/protocols/trunk \
-I$(RJ_HOME)/modules/protocols/ims



ifeq "$(LEX_COMPILE)" "LEX_YACC"

# "remotejob" must be differnet than "rj", otherwise it will mixup order of building files
remotejob: ymml.tab.o lexmml.yy.o ytrace.tab.o lextrace.yy.o rj.o 
	
# yacc flags:
#     -v provides adddiotional file y.output with human readeable description of the parser 
#     -b file_prefix
#     -d produces y.tab.h file (using it for yyinput)
#     -p symbol_prefix .. Change the prefix prepended to yacc-generated symbols to the
#        string denoted by symbol_prefix. The default prefix is the string yy.
ymml.tab.o:modules/axe/mml/parser_y
	yacc -b ymml -p yymml -d modules/axe/mml/parser_y;
	mv ymml.tab.c modules/axe/mml/parser.y.tab.c
	mv ymml.tab.h modules/axe/mml/parser.y.tab.h
lexmml.yy.o:modules/axe/mml/parser_l
	flex -Pyymml -t modules/axe/mml/parser_l>lexmml.yy.c;
	mv lexmml.yy.c modules/axe/mml/parser.lex.yy.c
	
ytrace.tab.o:modules/axe/trace/configfile_parser_y
	yacc -b ytrace -p yytrace -v -d modules/axe/trace/configfile_parser_y;
	mv ytrace.tab.c modules/axe/trace/configfile_parser.y.tab.c
	mv ytrace.tab.h modules/axe/trace/configfile_parser.y.tab.h
lextrace.yy.o:modules/axe/trace/configfile_parser_l
	flex -Pyytrace -t modules/axe/trace/configfile_parser_l>lextrace.yy.c;
	mv lextrace.yy.c modules/axe/trace/configfile_parser.lex.yy.c
	
# to build debug version ad -g after $(CC)
	
# To make an executable
rj.o: rj_$(RJ_VERSION).c
	$(CC) $< $(BUILD_SRC) $(BUILD_FLAG) $(BUILD_OBJ) $(BUILD_INC) -o rj 
	cp rj ./bin
	cp sigtran.conf* ./bin
	cp oscillo.conf* ./bin
	cp tgen.trigger ./bin
	cp tgen.conf* ./bin	
	
else

# "remotejob" must be differnet than "rj", otherwise it will mixup order of building files
remotejob: ymml.tab.o lexmml.yy.o ytrace.tab.o lextrace.yy.o rj.o 
	
# yacc flags:
#     -v provides adddiotional file y.output with human readeable description of the parser 
#     -b file_prefix
#     -d produces y.tab.h file (using it for yyinput)
#     -p symbol_prefix .. Change the prefix prepended to yacc-generated symbols to the
#        string denoted by symbol_prefix. The default prefix is the string yy.
ymml.tab.o:modules/axe/mml/parser_y
	bison -b ymml -p yymml -d modules/axe/mml/parser_y;
	mv ymml.tab.c modules/axe/mml/parser.y.tab.c
	mv ymml.tab.h modules/axe/mml/parser.y.tab.h
lexmml.yy.o:modules/axe/mml/parser_l
	flex -Pyymml -t modules/axe/mml/parser_l>lexmml.yy.c;
	mv lexmml.yy.c modules/axe/mml/parser.lex.yy.c
	
ytrace.tab.o:modules/axe/trace/configfile_parser_y
	bison -b ytrace -p yytrace -v -d modules/axe/trace/configfile_parser_y;
	mv ytrace.tab.c modules/axe/trace/configfile_parser.y.tab.c
	mv ytrace.tab.h modules/axe/trace/configfile_parser.y.tab.h
lextrace.yy.o:modules/axe/trace/configfile_parser_l
	flex -Pyytrace -t modules/axe/trace/configfile_parser_l>lextrace.yy.c;
	mv lextrace.yy.c modules/axe/trace/configfile_parser.lex.yy.c
	
# to build debug version ad -g after $(CC)
	
# To make an executable
rj.o: rj_$(RJ_VERSION).c
	$(CC) $< $(BUILD_SRC) $(BUILD_FLAG) $(BUILD_OBJ) $(BUILD_INC) -o rj 
	mv rj ./bin
	cp sigtran.conf* ./bin
	cp oscillo.conf* ./bin
	cp tgen.trigger ./bin
	cp tgen.conf* ./bin	

endif





	
