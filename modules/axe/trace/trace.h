
#undef PREDEF
#ifdef TRACE_PROJECT
#define PREDEF
#else
#define PREDEF extern
#endif

//************************************************************************************
// CONSTANTS
//************************************************************************************

#define	CURRENT_NO_OF_PROJECTS	4		// Current number of defined projects
#define	PROJECT_TELIMPULSE	1
#define	PROJECT_CCM		2
#define	PROJECT_CUSTOM		3
#define	PROJECT_TGEN		4

#define	PAGE_LINES_SIZE		100	// Size of page in lines
#define	PAGE_PAGES_SIZE		1000	// Maximum number of pages

// VARIABLE_MAX is used both for max number of variables in row and for max number of variables that could exist in system
#define	VARIABLE_MAX		30
#define	VARIABLE_MAX_PAGES	1000	// Maximum number of pages
#define	VARIABLE_PAGE_ROWS	100	// Size of array in rows
#define	VARIABLE_MAX_LENGTH	20	// Maximum length of variable in row

#define	MAX_NO_OF_FILES		5      	// Max number of opened FILES

// Maximum number of keys, that is "name{}"
#define	KEY_MAX				50
#define	KEY_HEADINGPART_MAX		10
#define	KEY_VARIABLEPART_MAX		50

//************************************************************************************
// VARIABLE DECLARATION
//************************************************************************************


typedef struct		// POINTERS TO ANALYSED VARIABLE ARRAY
{
	char			pString[VARIABLE_PAGE_ROWS][VARIABLE_MAX_LENGTH];		// Variable values in string array
} pTmpVariablePageString;

typedef struct		// FILES INFORMATION
{
	char		*pText;	                    // pointer to the first byte of the allocated Text memory block

	struct		// pText INFORMATION
	{
		int	SizeInBytes;
	} Info;

	int		FileAnalisysLineIndex;

	char	FileName[MINIBUF];	       			// Name of analysed file, used only to name export Variable Files

	//--------------------------------------------------------------------------------------
	// PAGE/LINE pointers to pText memory
	//--------------------------------------------------------------------------------------
	struct		// pText INFORMATION
	{
		int		NoPages;
		int		NoTotalLines;
		int     Page[PAGE_PAGES_SIZE][PAGE_LINES_SIZE];	// Array Pointers to all rows in pText memmory
	} Indexed;

	//--------------------------------------------------------------------------------------
	// Array Pointers to String Array of Variables
	// Variables are stored during Analyse phase in pages, and each page is
	// allocated when needed for each variable
	//--------------------------------------------------------------------------------------
	struct
	{
		pTmpVariablePageString	*Page[VARIABLE_MAX_PAGES];
		int		TotalRows;
		int		TotalPages;
		int		LargestString;		// Use for calculation CellSize[VARIABLE_MAX] later
	} TmpVariable[VARIABLE_MAX];

	//--------------------------------------------------------------------------------------
	// Memory space for variables. After Analyse phase and storing into TmpVariable[]
	// pages and lines, data are stored in whole linear space that is more adequate for
	// transfering pointer over application. TmpVariable[] memory is then released
	// and unuseable.
	//--------------------------------------------------------------------------------------
	struct
	{
		char   *Storage;
		int		Storage_CellSize;
		int		Storage_CellsNumber;
		char    KeyIndex;
	} Variable[VARIABLE_MAX];

	struct
	{
		// Key name for all variables in {} (e.g. CCMVAR_ELAPSEDTIME)
		char	Name[MINIBUF];
		int		TimeSlice;
		int		ConvertBase; // 10-Dec, 16-Hex
		int     IndexType;	 // 0-Local, 1-Global
		// Max VARIABLE_MAX variables with max KEY_HEADINGPART_MAX heading strings of max BUFSIZE characters
		char	Heading[KEY_HEADINGPART_MAX][MINIBUF];
		// Max VARIABLE_MAX variables with max KEY_VARIABLEPART_MAX variable strings of max BUFSIZE characters
		char	Variable_Mask[KEY_VARIABLEPART_MAX][MINIBUF];
		char	Variable_Name[KEY_VARIABLEPART_MAX][MINIBUF];
		char	Variable_Title[KEY_VARIABLEPART_MAX][MINIBUF];

		// KEY example (CCMVAR_ELAPSEDTIME):
		// CCMVAR_ELAPSEDTIME {
		// HEADER = "   CCM1     VAR H'039"				... key heading part
		// HEADER = "   xxxxx"							... key heading part
		// VARIABLE = "********:......) *'AAAAAAAAA: *'AAAAAAAAA: *'AAAAAAAAA: *'AAAAAAAAA:" ... key variable part
		// VARIABLE = "********:......)"				... key variable part
		// SIZE     = 100 | 10
		// }

	} Keys[KEY_MAX];

} ParsedProjectData;

int Global10msIndexStorage;


typedef struct
{
  int	ID;			// Project Choosed
  int	State;		// Current Project state (0-no state)

  struct	// FILE INFORMATION
  {
	  struct sTriggerProjectInfo {
		  char TraceConfigFileName[MINIBUF];	// name of config file with parsing instructions
	      char TraceConfigPrefix[MINIBUF];		// prefix for instructions in config file
	      char LocalFileNameDir[MINIBUF];		// file to be parsed
	      char OutputFileNameDir[MINIBUF];		// file name + location of output parsed data
	      char *pDataFromSocket;			// data from socket to be parsed
	      char DataFromSocket[MAXIBUF];		// data from socket to be parsed
	      int  DataFromSocket_SizeInBytes;
	  } TriggerProjectInfo;
	  ParsedProjectData Data;
  } EditedFile[MAX_NO_OF_FILES];

} ProjectInformation;

#ifdef TRACE_PROJECT
int	NoOfProjectsDefined = CURRENT_NO_OF_PROJECTS;			// No of defined projects
#else
PREDEF int NoOfProjectsDefined;
#endif

PREDEF ProjectInformation	Project;

PREDEF void Trace_ProjectReset(void);
PREDEF void Trace_ProjectDataAnalisysReset();
PREDEF bool Trace_ProjectOperation(int file0_mem1);
PREDEF bool Trace_ProjectOperation_DataAnalyse(int nIndex);
PREDEF bool Trace_DataAnalyse_LoadTextFileToPagedMem(int nIndex);
PREDEF boolean Trace_StoreToTableInFile (int filemode_uglyfast0_niceslow1);

