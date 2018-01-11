
#include <sys/time.h>

#define TRACE_PROJECT
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/trace/trace.h"
#include "modules/axe/trace/dataanalyse.h"
#include "modules/axe/trace/configfile_parser.h"
#include "modules/axe/trace/configfile_parser.y.tab.h"
#undef TRACE_PROJECT

void Trace_ProjectReset(void)
{
  int nIndex;
  int i, j;

  Project.ID    = 0;
  Project.State = 0; // no state

  Global10msIndexStorage = 0;

  for (nIndex = 0; nIndex < MAX_NO_OF_FILES; nIndex ++)
  {
	  Project.EditedFile[nIndex].TriggerProjectInfo.TraceConfigPrefix[0] = '\0';
	  Project.EditedFile[nIndex].TriggerProjectInfo.LocalFileNameDir[0]  = '\0';
	  Project.EditedFile[nIndex].TriggerProjectInfo.OutputFileNameDir[0] = '\0';
	  Project.EditedFile[nIndex].TriggerProjectInfo.DataFromSocket[0]    = '\0';
	  Project.EditedFile[nIndex].TriggerProjectInfo.pDataFromSocket	     = NULL;

	  for (i=0; i<KEY_MAX; i++) {
		  // RESET Keys struct
		  Project.EditedFile[nIndex].Data.Keys[i].Name[0]   	 = '\0'; // reset key name
		  Project.EditedFile[nIndex].Data.Keys[i].TimeSlice 	 = 0;
		  Project.EditedFile[nIndex].Data.Keys[i].ConvertBase 	 = 0;
		  Project.EditedFile[nIndex].Data.Keys[i].IndexType 	 = 0;

		  for (j=0; j<KEY_HEADINGPART_MAX; j++)
			  Project.EditedFile[nIndex].Data.Keys[i].Heading[j][0] = '\0'; // reset all Heading key strings
		  for (j=0; j<KEY_VARIABLEPART_MAX; j++)
			  Project.EditedFile[nIndex].Data.Keys[i].Variable_Mask[j][0] = '\0'; // reset all Variable key strings
		  for (j=0; j<KEY_VARIABLEPART_MAX; j++) {
			  Project.EditedFile[nIndex].Data.Keys[i].Variable_Name[j][0]  = '\0'; // reset all Variable key strings
			  Project.EditedFile[nIndex].Data.Keys[i].Variable_Title[j][0] = '\0';
		  }
		  }
  }

  Project.EditedFile[nIndex].Data.FileName[0] = '\0';

}

void Trace_ProjectDataAnalisysReset() {
  int nIndex;
  int i, j;

  for (nIndex = 0; nIndex < MAX_NO_OF_FILES; nIndex ++)
  {
	  for (i=0; i<VARIABLE_MAX; i++) {
		  // RESET Variable struct
		  Project.EditedFile[nIndex].Data.Variable[i].Storage_CellSize 	= 0;
		  Project.EditedFile[nIndex].Data.Variable[i].Storage_CellsNumber = 0;
		  if (Project.EditedFile[nIndex].Data.Variable[i].Storage != NULL) {
			  free(Project.EditedFile[nIndex].Data.Variable[i].Storage);
			  Project.EditedFile[nIndex].Data.Variable[i].Storage = NULL;
		  }
	  }

	  // RESET struct TmpVariable in ExportVariableValuesToStringStorage()

	  // File2PagedBuffer() call clean
	  Project.EditedFile[nIndex].Data.Info.SizeInBytes = 0;
	  if (Project.EditedFile[nIndex].Data.pText != NULL)
		  free(Project.EditedFile[nIndex].Data.pText);

	  Project.EditedFile[nIndex].Data.FileAnalisysLineIndex = 0;

	  // RESET struct Indexed
	  Project.EditedFile[nIndex].Data.Indexed.NoPages = 0;
	  Project.EditedFile[nIndex].Data.Indexed.NoTotalLines = 0;
  }

}


//************************************************************************************
//************************************************************************************
// FUNCTION:	Trace_ProjectLocalFileOperation()
//
// PURPOSE:	Handle result
//************************************************************************************
//************************************************************************************
bool Trace_ProjectOperation(int file0_mem1) {
  int	nIndex;
  char 	szPrintout[MINIBUF];

  for (nIndex = 0; nIndex < MAX_NO_OF_FILES; nIndex ++) {

	  if (file0_mem1 == 0) {
		if (Project.EditedFile[nIndex].TriggerProjectInfo.LocalFileNameDir[0] != '\0') {
			//printf ("&Project.EditedFile[nIndex].Data.Indexed.Page[0][0]=0x%x\n", &Project.EditedFile[nIndex].Data.Indexed.Page[0][0]);
			//printf ("&Project.EditedFile[nIndex].Data.Indexed.Page[1][0]=0x%x\n", &Project.EditedFile[nIndex].Data.Indexed.Page[1][0]);

			ErrorTraceHandle(3, "Trace_ProjectOperation(File) !\n");

			// File2PagedBuffer() is located in rj common code
			if ((Project.EditedFile[nIndex].Data.pText = (char *)File2PagedBuffer(
												 NULL,
												 Project.EditedFile[nIndex].TriggerProjectInfo.LocalFileNameDir,
												 &Project.EditedFile[nIndex].Data.Info.SizeInBytes,
												 &Project.EditedFile[nIndex].Data.Indexed.Page[0][0],
												 PAGE_PAGES_SIZE, PAGE_LINES_SIZE,
												 &Project.EditedFile[nIndex].Data.Indexed.NoPages,
												 &Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)) == NULL) {
				ErrorTraceHandle(0, "Trace_ProjectOperation(File) LocalFileNameDir<%s> NoPages<%d> NoTotalLines<%d> failed !\n",
						Project.EditedFile[nIndex].TriggerProjectInfo.LocalFileNameDir,
						Project.EditedFile[nIndex].Data.Indexed.NoPages,
						Project.EditedFile[nIndex].Data.Indexed.NoTotalLines);
			}

			return Trace_ProjectOperation_DataAnalyse(nIndex);
		}
	  } else {
		if (Project.EditedFile[nIndex].TriggerProjectInfo.DataFromSocket != NULL) {
			//printf ("&Project.EditedFile[nIndex].Data.Indexed.Page[0][0]=0x%x\n", &Project.EditedFile[nIndex].Data.Indexed.Page[0][0]);
			//printf ("&Project.EditedFile[nIndex].Data.Indexed.Page[1][0]=0x%x\n", &Project.EditedFile[nIndex].Data.Indexed.Page[1][0]);

			ErrorTraceHandle(3, "Trace_ProjectOperation(Buffer) !\n");

			// File2PagedBuffer() is located in rj common code
			if ((Project.EditedFile[nIndex].Data.pText = (char *)Buffer2PagedBuffer(
												 Project.EditedFile[nIndex].TriggerProjectInfo.DataFromSocket,
												 Project.EditedFile[nIndex].TriggerProjectInfo.DataFromSocket_SizeInBytes,
												 &Project.EditedFile[nIndex].Data.Info.SizeInBytes,
												 &Project.EditedFile[nIndex].Data.Indexed.Page[0][0],
												 PAGE_PAGES_SIZE, PAGE_LINES_SIZE,
												 &Project.EditedFile[nIndex].Data.Indexed.NoPages,
												 &Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)) == NULL) {
				ErrorTraceHandle(0, "Trace_ProjectOperation(Buffer) DataFromSocket<%s> NoPages<%d> NoTotalLines<%d> failed !\n",
						Project.EditedFile[nIndex].TriggerProjectInfo.DataFromSocket,
						Project.EditedFile[nIndex].Data.Indexed.NoPages,
						Project.EditedFile[nIndex].Data.Indexed.NoTotalLines);
			}

			return Trace_ProjectOperation_DataAnalyse(nIndex);
		}
	  }
  }

  return TRUE;
}


//************************************************************************************
//************************************************************************************
// FUNCTION:	Trace_ProjectOperation_MemFiolAnalyse()
//
// COMMENTS:
//************************************************************************************
//************************************************************************************
bool Trace_ProjectOperation_DataAnalyse(int nIndex) // Index of created file, from Appl side reference to file
{
	int  key;
	char status[MINIBUF];
	int  SearchHeaderCounter = 0;
	int  SearchHeaderStarValue = 0;
	struct timeval HDR_start_time, HDR_end_time;
	struct timeval VAR_start_time, VAR_end_time;

	// * open Text file to edit/analyse and copy its content in memory,
	// * fill structure for page/line pointers to that file in memory to speed up searching

	gettimeofday(&HDR_start_time, NULL);
	ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() Analysing header parts started at %d microseconds... \n",
			 HDR_start_time.tv_usec);

	// Text line counter (SearchHeaderCounter)
	while (TRUE)
	{
		// Variable index from config printout file
		key = 0;

		// for each Text line counter (SearchHeaderCounter), pass through all key HEADER printouts,
		// if it matches, continue analysing for VARIABLE printouts
		while (Project.EditedFile[nIndex].Data.Keys[key].Name[0] != '\0')
		{
			FAST4ErrorTraceHandle(3, "Trace_ProjectOperation_DataAnalyse() Syntax selected nIndex-%d, key-%d, Name<%s>",
								  nIndex, key, Project.EditedFile[nIndex].Data.Keys[key].Name);
			FAST3ErrorTraceHandle(3, ", searching for header part: file %d, line %5d\n",
								  nIndex,
								  Project.EditedFile[nIndex].Data.FileAnalisysLineIndex);

			// - determine HEADER constant strings
			if (DataAnalyse_HEADER(nIndex, key))
			{
			    gettimeofday(&VAR_start_time, NULL);
				ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() Analysing variable part for <%s> from line %d started at %d microseconds... \n",
						 Project.EditedFile[nIndex].Data.Keys[key].Name,
						 Project.EditedFile[nIndex].Data.FileAnalisysLineIndex,
						 VAR_start_time.tv_usec);

				SearchHeaderCounter = 0; // header detected, reset counter

				// * extract veriables from edit file according to printout file rules
				if (!DataAnalyse_VARIABLE(nIndex, key)) {
				  ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() DataAnalyse_VARIABLE failed even there is match in HEADER for <%s> index.\n", Project.EditedFile[nIndex].Data.Keys[key].Name);
				  return FALSE;
				}

				SearchHeaderStarValue = Project.EditedFile[nIndex].Data.FileAnalisysLineIndex;
			        gettimeofday(&VAR_end_time, NULL);
				ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() Ended at %d microseconds, duration %d.%d miliseconds.\n",
						 VAR_end_time.tv_usec,
						 (VAR_end_time.tv_usec-VAR_start_time.tv_usec)/1000,
						 (VAR_end_time.tv_usec-VAR_start_time.tv_usec)%1000);

				// part of printout sucessfully analysed, just continue
			}
			else
				key ++;
		}

		if (SearchHeaderCounter >= 500) {

		  ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() Search rules degrade performances: started at %d, now at %d).\n",
				   SearchHeaderStarValue, Project.EditedFile[nIndex].Data.FileAnalisysLineIndex);

		  SearchHeaderCounter=0;
		  /*
		  // for debugging purposes
		  while (true) {
		    char c;
		    printf ("Do you want to proceed ? (y/n)\n");
		    c=getc(stdin);
		    if (c=='y')
		      break;
		    else if (c=='n')
		      return FALSE;
		  }
		  */
		}
		SearchHeaderCounter ++;

		if ((Project.EditedFile[nIndex].Data.FileAnalisysLineIndex ++) == Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)
			break;
	}

	gettimeofday(&HDR_end_time, NULL);
	ErrorTraceHandle(2, "Trace_ProjectOperation_DataAnalyse() Analysing header parts ended at %d microseconds, duration %d.%d miliseconds. \n",
			 HDR_end_time.tv_usec,
			 (HDR_end_time.tv_usec-HDR_start_time.tv_usec)/1000,
			 (HDR_end_time.tv_usec-HDR_start_time.tv_usec)%1000);


	return TRUE;
}

boolean Trace_StoreToTableInFile (int filemode_uglyfast0_niceslow1)
{
	char  ExportVarFileName[MINIBUF];
	FILE *hExportVarFile=NULL;
	bool  appenedfile = false;

	int	Counter10ms		= 0;
	int	Counter100ms    = 0;
	int	Counter1000ms	= 0;

	int	  StorageCellSize;
	char *StorageAddress;
	int	  maxStorageSize10ms   = 0;
	int   maxStorageSize100ms  = 0;
	int   maxStorageSize1000ms = 0;

	int	VAR_Name;
	int firstvariable;
	int VariablesLoaded = 0;
	int TimeSlice, ConvertBase, IndexType;

	char LineBuf[MINIBUF];
	char LineBuf100ms[VARIABLE_MAX][MINIBUF];
	char LineBuf1000ms[VARIABLE_MAX][MINIBUF];
	char tmpBuf[MINIBUF], tmpBuf2[MINIBUF], *pTmp;

	char  Conv_Str1[MINIBUF], Conv_Str2[MINIBUF];
	long  Conv_dec;
	char *Conv_endpStr, *Conv_pStr;


	for (VAR_Name = 0; VAR_Name < VARIABLE_MAX; VAR_Name ++)
	{
	  if ((Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber > 0) && (VAR_Name>VariablesLoaded))
	    VariablesLoaded = VAR_Name;

		switch (Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].TimeSlice) {
		case 10:
			if (Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber > maxStorageSize10ms)
				maxStorageSize10ms = Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber;
			break;
		case 100:
			if (Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber > maxStorageSize100ms)
				maxStorageSize100ms = Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber;
			break;
		case 1000:
			if (Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber > maxStorageSize1000ms)
				maxStorageSize1000ms = Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber;
			break;
		}
	}
	ErrorTraceHandle(2, "Trace_StoreToTableInFile() max VAR_Name value is %d, maxStorageSize 10ms=%d, 100ms=%d, 1000ms=%d\n",
			VariablesLoaded, maxStorageSize10ms, maxStorageSize100ms, maxStorageSize1000ms);
	if (maxStorageSize1000ms*10>maxStorageSize100ms)
		maxStorageSize100ms = maxStorageSize1000ms*10;
	if (maxStorageSize100ms*10>maxStorageSize10ms)
		maxStorageSize10ms = maxStorageSize100ms*10;
	ErrorTraceHandle(2, "Trace_StoreToTableInFile() max 10ms counter = %d\n",
			VariablesLoaded, maxStorageSize10ms);

	strcpy(ExportVarFileName, Project.EditedFile[0].TriggerProjectInfo.LocalFileNameDir);
	strcat(ExportVarFileName, "_Variables.log");
	if ((hExportVarFile = fopen(ExportVarFileName,"r+")) == NULL)
	{
		if ((hExportVarFile = fopen(ExportVarFileName,"w+")) == NULL)
		{
		  ErrorTraceHandle(2, "Trace_StoreToTableInFile() Can not create new file <%s>.\n", ExportVarFileName);
		  return FALSE;
		}
	} else {
		if ((hExportVarFile = fopen(ExportVarFileName,"a+")) == NULL) {
		  ErrorTraceHandle(2, "Trace_StoreToTableInFile() Can not append file <%s>.\n", ExportVarFileName);
		  return FALSE;
		} else
			appenedfile = true;
	}

	if (!appenedfile) {
		if (filemode_uglyfast0_niceslow1 == 1)
			strcpy(LineBuf, "   INDEX      ");
		else
			strcpy(LineBuf, "INDEX ");

		for (VAR_Name = 0; VAR_Name <= VariablesLoaded; VAR_Name ++)
		{
			StorageCellSize = Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellSize;
			if (Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber > 0)
			{
				if (Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].Variable_Title[VAR_Name][0] != '\0') {
					strcpy(tmpBuf, Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].Variable_Title[VAR_Name]);
					strcat(LineBuf, tmpBuf);
				} else { // copy only after first "_" if name of key
					strcpy(tmpBuf, Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].Name);
					strcpy(tmpBuf2, "_");

					pTmp = (char *)strstr((char *)tmpBuf, (char *)tmpBuf2);
					if (pTmp == NULL)
						strcat(LineBuf, tmpBuf);
					else
						strcat(LineBuf, ++pTmp);
				}
				strncat(LineBuf, "           ", 1);

			}
		}
		fprintf(hExportVarFile, "\n%s\n", LineBuf);
	}

	for (VAR_Name = 0; VAR_Name <= VariablesLoaded; VAR_Name ++) {
		ErrorTraceHandle(2, "Trace_StoreToTableInFile() VAR_Name=%s(%d), Storage=0x%x, Storage_CellsNumber=%d, Storage_CellSize=%d, TimeSlice=%d, ConvertBase=%d, IndexType=%d, Variable_Title=%s\n",
				Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].Name, VAR_Name,
				Project.EditedFile[0].Data.Variable[VAR_Name].Storage,
				Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber,
				Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellSize,
				Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].TimeSlice,
				Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].ConvertBase,
				Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].IndexType,
				Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].Variable_Title[VAR_Name]);
	}

	IndexType  = Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VariablesLoaded].KeyIndex].IndexType;

	for (Counter10ms = (IndexType==0)?0:Global10msIndexStorage;
		 Counter10ms < ((IndexType==0)?maxStorageSize10ms:(Global10msIndexStorage+maxStorageSize10ms));
		 Counter10ms ++)
	{

		FAST4ErrorTraceHandle(4, "Trace_StoreToTableInFile(1000) Counter1000ms=%d, Counter100ms=%d, Counter10ms=%d\n",
							  Counter1000ms,
							  Counter100ms,
							  Counter10ms);

	    firstvariable = 1;
		for (VAR_Name = 0; VAR_Name <= VariablesLoaded; VAR_Name ++) {

			if (firstvariable == 1) {
				strcpy(LineBuf, "");
				firstvariable = 0;
			}

			TimeSlice 		= Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].TimeSlice;
			ConvertBase 	= Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].ConvertBase;
			StorageCellSize = Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellSize;


			switch (TimeSlice) {
			case 1000: // 1000ms variable
			{
				if ((Counter10ms % 100) == 0)
				{
					if (Counter1000ms < Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber) {
						StorageAddress  = Project.EditedFile[0].Data.Variable[VAR_Name].Storage + Counter1000ms * StorageCellSize;

						FAST3ErrorTraceHandle(4, "Trace_StoreToTableInFile(1000) Var_%d[%s]\n",
											  VAR_Name,
											  StorageAddress);

						strcpy(Conv_Str2, StorageAddress);
						Conv_pStr = Conv_Str2;
						Conv_dec  = strtol(Conv_pStr, &Conv_endpStr, ConvertBase);

						FAST3ErrorTraceHandle(4, "Trace_StoreToTableInFile(1000) Conv_Str2=%s, Conv_dec=%d\n",
											  Conv_Str2,
											  Conv_dec);
						if (filemode_uglyfast0_niceslow1 == 1)
							sprintf(Conv_Str1, "%7d", (int) Conv_dec);
						else
							sprintf(Conv_Str1, " %d", (int) Conv_dec);

						strcpy(LineBuf1000ms[VAR_Name], Conv_Str1);
					}
					else
						LineBuf1000ms[VAR_Name][0]='\0';

				}
				if (LineBuf1000ms[VAR_Name][0]!='\0')
					strcat(LineBuf, LineBuf1000ms[VAR_Name]);
				else {
					if (filemode_uglyfast0_niceslow1 == 1)
						strncat(LineBuf, "              ", 7);
				}
			}
			break;
			case 100: // 100ms variable
			{
				if ((Counter10ms % 10) == 0)
				{
					if (Counter100ms < Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber) {
						StorageAddress  = Project.EditedFile[0].Data.Variable[VAR_Name].Storage + Counter100ms * StorageCellSize;

						FAST3ErrorTraceHandle(4, "Trace_StoreToTableInFile(100) Var_%d[%s]\n",
											  VAR_Name,
											  StorageAddress);

						strcpy(Conv_Str2, StorageAddress);
						Conv_pStr = Conv_Str2;
						Conv_dec  = strtol(Conv_pStr, &Conv_endpStr, ConvertBase);

						FAST3ErrorTraceHandle(4, "Trace_StoreToTableInFile(100) Conv_Str2=%s, Conv_dec=%d\n",
											  Conv_Str2,
											  Conv_dec);
						if (filemode_uglyfast0_niceslow1 == 1)
							sprintf(Conv_Str1, "%7d", (int) Conv_dec);
						else
							sprintf(Conv_Str1, " %d", (int) Conv_dec);
						strcpy(LineBuf100ms[VAR_Name], Conv_Str1);
					}
					else
						LineBuf100ms[VAR_Name][0]='\0';

				}
				if (LineBuf100ms[VAR_Name][0]!='\0')
					strcat(LineBuf, LineBuf100ms[VAR_Name]);
				else {
					if (filemode_uglyfast0_niceslow1 == 1)
						strncat(LineBuf, "              ", 7);
				}
			}
			break;
			case 10: // 10ms variable
			{
				if (Counter10ms < Project.EditedFile[0].Data.Variable[VAR_Name].Storage_CellsNumber) {
					StorageAddress  = Project.EditedFile[0].Data.Variable[VAR_Name].Storage + Counter10ms * StorageCellSize;

					FAST6ErrorTraceHandle(4, "Trace_StoreToTableInFile(10) Var_%d[%s], Counter1000ms=%d, Counter100ms=%d, Counter10ms=%d\n",
										  VAR_Name,
										  StorageAddress,
										  Counter1000ms,
										  Counter100ms,
										  Counter10ms);

					strcpy(Conv_Str2, StorageAddress);
					Conv_pStr = Conv_Str2;
					Conv_dec  = strtol(Conv_pStr, &Conv_endpStr, ConvertBase);

					FAST3ErrorTraceHandle(4, "Trace_StoreToTableInFile(10) Conv_Str2=%s, Conv_dec=%d\n",
										  Conv_Str2,
										  Conv_dec);

					if (filemode_uglyfast0_niceslow1 == 1)
						sprintf(Conv_Str1, "%7d", (int) Conv_dec);
					else
						sprintf(Conv_Str1, " %d", (int) Conv_dec);
					strcat(LineBuf, Conv_Str1);
				}
				else {
					if (filemode_uglyfast0_niceslow1 == 1)
						strncat(LineBuf, "              ", 7);
				}
			}
			break;

			default:
			{
				FAST3ErrorTraceHandle(3, "Trace_StoreToTableInFile() Var_%d unsupported TimeSlice=%d\n",
						VAR_Name, Project.EditedFile[0].Data.Keys[Project.EditedFile[0].Data.Variable[VAR_Name].KeyIndex].TimeSlice);
			}
			}

			if (VAR_Name == VariablesLoaded) { // increase counters on last 100ms variable
				if ((Counter10ms % 10) == 0)
					 Counter100ms ++;
				if ((Counter10ms % 100) == 0)
					 Counter1000ms ++;
			}
		}

		if (filemode_uglyfast0_niceslow1 == 1)
			fprintf(hExportVarFile, "   %5d%s\n", Counter10ms, LineBuf);
		else
			fprintf(hExportVarFile, "%d%s\n", Counter10ms, LineBuf);

		FAST3ErrorTraceHandle(2, "FILE>>>%5d%s<<<\n",
								Counter10ms,
								LineBuf);

	}

	Global10msIndexStorage = Counter10ms;

	fclose(hExportVarFile);
	return TRUE;
}

