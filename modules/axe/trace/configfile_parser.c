
#include <modules/common.h>
#include <modules/nethelper.h>
#include "modules/axe/trace/trace.h"
#include <modules/axe/trace/configfile_parser.h>
#include <modules/axe/trace/configfile_parser.y.tab.h>

bool yyFillKeyStruct(int eprojectTraceEnum, char *parsestring1, char *parsestring2)
{
	bool preformed; // locate or create key with name yyActiveBracketUnknownString
	int i, key;

	for (i=0; i<MAX_NO_OF_FILES; i++)
	{
		// find project TraceConfigPrefix parameter that matches yyActiveBracketUnknownString
		if ((Project.EditedFile[i].TriggerProjectInfo.TraceConfigPrefix[0] != '\0') &&
			(strstr(yyActiveBracketUnknownString, Project.EditedFile[i].TriggerProjectInfo.TraceConfigPrefix) != NULL))
		{
			ErrorTraceHandle(3, "yyFillKeyStruct() yyActiveBracketUnknownString<%s>, TraceConfigPrefix<%s>, nIndex<%d>\n", yyActiveBracketUnknownString, Project.EditedFile[i].TriggerProjectInfo.TraceConfigPrefix, i);

			preformed = false;
			for (key=0; key<KEY_MAX; key++)
			{
				if (Project.EditedFile[i].Data.Keys[key].Name[0] == '\0') {
					// key not found and we passed last created key, create new key
					strcpy(Project.EditedFile[i].Data.Keys[key].Name, yyActiveBracketUnknownString);
					ErrorTraceHandle(3, "yyFillKeyStruct(ACTION) ON NEW KEY <%s,%d>\n", Project.EditedFile[i].Data.Keys[key].Name,key);
					preformed = true;
					break;
				}
				if (!strcmp(Project.EditedFile[i].Data.Keys[key].Name, yyActiveBracketUnknownString)) {
					// key previously created and now located
					ErrorTraceHandle(3, "yyFillKeyStruct(ACTION) ON EXISTING KEY <%s,%d>\n", Project.EditedFile[i].Data.Keys[key].Name,key);
					preformed = true;
					break;
				}
			}

			if (preformed) {
				switch (eprojectTraceEnum) {
				case eHEADING:
				{
					int headingpart;
					for (headingpart=0; headingpart<KEY_HEADINGPART_MAX; headingpart++)
					{
						if (Project.EditedFile[i].Data.Keys[key].Heading[headingpart][0] == '\0') {
							// Heading part in key not found and we passed last created key, add new HEADING key
							strcpy(Project.EditedFile[i].Data.Keys[key].Heading[headingpart], parsestring1);
							ErrorTraceHandle(3, "yyFillKeyStruct(ACTION) ADDED NEW HEADING CONTENT <%s,%d> in KEY <%s,%d>\n",
											Project.EditedFile[i].Data.Keys[key].Heading[headingpart],headingpart,
											Project.EditedFile[i].Data.Keys[key].Name, key);
							break;
						}
						if (!strcmp(Project.EditedFile[i].Data.Keys[key].Heading[headingpart], parsestring1)) {
							// same key content already created and located
							ErrorTraceHandle(3, "yyFillKeyStruct(INFO) SAME HEADING CONTENT <%s,%d> ALREADY in KEY <%s,%d>\n",
											Project.EditedFile[i].Data.Keys[key].Heading[headingpart],headingpart,
											Project.EditedFile[i].Data.Keys[key].Name, key);
							//break;
						}
					}
				}
				break;
				case eVARIABLE_MASK:
				{
					int variablepart;
					for (variablepart=0; variablepart<KEY_VARIABLEPART_MAX; variablepart++)
					{
						if (Project.EditedFile[i].Data.Keys[key].Variable_Mask[variablepart][0] == '\0') {
							// Variable part in key not found and we passed last created key, add new VARIABLE key
							strcpy(Project.EditedFile[i].Data.Keys[key].Variable_Mask[variablepart], parsestring1);
							ErrorTraceHandle(3, "yyFillKeyStruct(ACTION) ADDED NEW VARIABLE_MASKCONTENT <%s,%d> in KEY <%s,%d>\n",
											Project.EditedFile[i].Data.Keys[key].Variable_Mask[variablepart],variablepart,
											Project.EditedFile[i].Data.Keys[key].Name, key);
							break;
						}
						if (!strcmp(Project.EditedFile[i].Data.Keys[key].Variable_Mask[variablepart], parsestring1)) {
							// same key content already created and located
							ErrorTraceHandle(3, "yyFillKeyStruct(INFO) SAME VARIABLE_MASK CONTENT <%s,%d> ALREADY in KEY <%s,%d>\n",
											Project.EditedFile[i].Data.Keys[key].Variable_Mask[variablepart],variablepart,
											Project.EditedFile[i].Data.Keys[key].Name, key);
							//break;
						}
					}
				}
				break;
				case eVARIABLE_NAME:
				{
					int variablepart;
					for (variablepart=0; variablepart<KEY_VARIABLEPART_MAX; variablepart++)
					{
						if (Project.EditedFile[i].Data.Keys[key].Variable_Name[variablepart][0] == '\0') {
							// Variable part in key not found and we passed last created key, add new VARIABLE key
							strcpy(Project.EditedFile[i].Data.Keys[key].Variable_Name[variablepart], parsestring1);
							ErrorTraceHandle(3, "yyFillKeyStruct(ACTION) ADDED NEW VARIABLE_NAME CONTENT <%s,%d> in KEY <%s,%d>\n",
											Project.EditedFile[i].Data.Keys[key].Variable_Name[variablepart],variablepart,
											Project.EditedFile[i].Data.Keys[key].Name, key);
							break;
						}
						if (!strcmp(Project.EditedFile[i].Data.Keys[key].Variable_Name[variablepart], parsestring1)) {
							// same key content already created and located
							ErrorTraceHandle(3, "yyFillKeyStruct(INFO) SAME VARIABLE_NAME CONTENT <%s,%d> ALREADY in KEY <%s,%d>\n",
									Project.EditedFile[i].Data.Keys[key].Variable_Name[variablepart],variablepart,
									Project.EditedFile[i].Data.Keys[key].Name, key);
							//break;
						}
					}
				}
				break;
				case eVARIABLE:
				{
					char varname=parsestring1[0];
					if ((varname >= 'A') && (varname <= ('A'+ VARIABLE_MAX - 1))) {
						strcpy(Project.EditedFile[i].Data.Keys[key].Variable_Title[varname-'A'], parsestring2);
						ErrorTraceHandle(2, "yyFillKeyStruct(ACTION) ADDED VARIABLE TITLE CONTENT <%s> on VARIABLE_NAME <%c> in KEY <%s,%d>\n",
										 Project.EditedFile[i].Data.Keys[key].Variable_Title[varname-'A'],
										 varname,
										 Project.EditedFile[i].Data.Keys[key].Name, key);
					} else
						ErrorTraceHandle(2, "yyFillKeyStruct(INFO) IGNORED VARIABLE TITLE CONTENT <%s> on VARIABLE_NAME <%s> in KEY <%s,%d>\n",
										parsestring2,
										parsestring1,
										Project.EditedFile[i].Data.Keys[key].Name, key);
				}
				break;
				case eTIMESLICE:
				{
				    Project.EditedFile[i].Data.Keys[key].TimeSlice = atoi(parsestring1);
				    ErrorTraceHandle(2, "yyFillKeyStruct(ACTION) ADDED TIMESLICE CONTENT <%d> in KEY <%s,%d>\n",
						     Project.EditedFile[i].Data.Keys[key].TimeSlice,
						     Project.EditedFile[i].Data.Keys[key].Name, key);
				}
				break;
				case eCONVERTBASE:
				{
				    Project.EditedFile[i].Data.Keys[key].ConvertBase = atoi(parsestring1);
				    ErrorTraceHandle(2, "yyFillKeyStruct(ACTION) ADDED CONVERTBASE CONTENT <%d> in KEY <%s,%d>\n",
						     Project.EditedFile[i].Data.Keys[key].ConvertBase,
						     Project.EditedFile[i].Data.Keys[key].Name, key);
				}
				break;
				case eINDEXTYPE:
				{
					if (strstr(parsestring1, "LOCAL"))
						Project.EditedFile[i].Data.Keys[key].IndexType = 0;
					else if (strstr(parsestring1, "GLOBAL"))
						Project.EditedFile[i].Data.Keys[key].IndexType = 1;

				    ErrorTraceHandle(2, "yyFillKeyStruct(ACTION) ADDED INDEXTYPE CONTENT <%d, %s> in KEY <%s,%d>\n",
						     Project.EditedFile[i].Data.Keys[key].IndexType, parsestring1,
						     Project.EditedFile[i].Data.Keys[key].Name, key);
				}
				break;
				default:;
				}
			}
		}
	} // end of for(int i=0; i<MAX_NO_OF_FILES; i++)

	return preformed;
}



