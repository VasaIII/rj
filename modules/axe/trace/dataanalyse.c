
#include "modules/common.h"
#include "modules/axe/trace/trace.h"

//************************************************************************************
// LOCAL FUNCTIONS
//************************************************************************************

bool ExportVariableValuesToStringStorage(int nIndex, int key);

//************************************************************************************
// CONSTANTS
//************************************************************************************

//************************************************************************************
// GLOBALS
//************************************************************************************

//************************************************************************************
// CODE
//************************************************************************************

void debug_text_descr(int nIndex, int key, int Char_KeyVsText_Mismatch,
					  char *printout,
					  int Text_LineIndex_Count,	   int Text_InTextCharCounter,
					  int key_VARIABLE_index, int key_VARIABLE_CharCounter) {
	if (0) {
		char  *pkey_VARIABLE_NAME;
		char  *pkey_VARIABLE_MASK;
		int NoPage1, NoLine1, NoPage2, NoLine2, charlen, charcnt;
		div_t  div_result;
		int    Text_InTextCharCounter1, Text_InTextCharCounter2;
		char   tmptext[MINIBUF];

		div_result	= div(Text_LineIndex_Count, PAGE_LINES_SIZE);
		NoPage1		= div_result.quot;
		NoLine1		= Text_LineIndex_Count - NoPage1 * PAGE_LINES_SIZE;
		div_result	= div((Text_LineIndex_Count+1), PAGE_LINES_SIZE);
		NoPage2		= div_result.quot;
		NoLine2		= (Text_LineIndex_Count+1) - NoPage2 * PAGE_LINES_SIZE;

		Text_InTextCharCounter1 = Project.EditedFile[nIndex].Data.Indexed.Page[NoPage1][NoLine1];
		if ((Text_LineIndex_Count+1) < Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)
			Text_InTextCharCounter2 = Project.EditedFile[nIndex].Data.Indexed.Page[NoPage2][NoLine2];
		else
			Text_InTextCharCounter2 = Text_InTextCharCounter1;

		charlen = Text_InTextCharCounter2 - Text_InTextCharCounter1;

		FAST4ErrorTraceHandle(2, "debug_text_descr() Text chars %d-%d, len=%d !\n",
				      Text_InTextCharCounter1, Text_InTextCharCounter2, charlen);

		pkey_VARIABLE_NAME = (char *) &Project.EditedFile[nIndex].Data.Keys[key].Variable_Name[0];
		pkey_VARIABLE_MASK = (char *) &Project.EditedFile[nIndex].Data.Keys[key].Variable_Mask[0];

		FAST7ErrorTraceHandle(2, "\n%s:1/7 Text line=%d/char=%d, Key line=%d/char=%d, Mismatch=%d",
				printout,
				Text_LineIndex_Count, Text_InTextCharCounter,
				key_VARIABLE_index, key_VARIABLE_CharCounter,
				Char_KeyVsText_Mismatch);

		if (charlen) {
			for(charcnt=0; charcnt<charlen; charcnt ++)
				tmptext[charcnt] = Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter1+charcnt];
			tmptext[charcnt] = '\0';
		FAST3ErrorTraceHandle(2, "\n%s:2/7 Text         <%s>", printout,
				tmptext);
		}
		FAST8ErrorTraceHandle(2, "\n%s:3/7 Text         +1<%d,'%c'> 0<%d,'%c'> -1<%d,'%c'>",
				printout,
				Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter+1], Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter+1],
				Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter],   Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter],
				Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter-1], Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter-1]);
		FAST3ErrorTraceHandle(2, "\n%s:4/7 Key VAR_MASK <%s>",
				 printout,
				 Project.EditedFile[nIndex].Data.Keys[key].Variable_Mask[key_VARIABLE_index]);
		FAST8ErrorTraceHandle(2, "\n%s:5/7 Key VAR_MASK +1<%d,'%c'> 0<%d,'%c'> -1<%d,'%c'>",
				printout,
				*(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter+1), *(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter+1),
				*(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter),   *(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter),
				*(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter-1), *(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter-1));
		FAST3ErrorTraceHandle(2, "\n%s:6/7 Key VAR_NAME <%s>\n",
				 printout,
				 Project.EditedFile[nIndex].Data.Keys[key].Variable_Name[key_VARIABLE_index]);
		FAST8ErrorTraceHandle(2, "\n%s:7/7 Key VAR_MASK +1<%d,'%c'> 0<%d,'%c'> -1<%d,'%c'>",
				printout,
				*(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter+1), *(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter+1),
				*(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter),   *(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter),
				*(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter-1), *(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter-1));
	}
}




//************************************************************************************
//************************************************************************************
// FUNCTION:	DataAnalyse_HEADER()
//
// PURPOSE:		Analyse and find key words from Printout array in
//				HEADER and VARIABLE part.
//************************************************************************************
//************************************************************************************
bool DataAnalyse_HEADER (int nIndex, int key)
{
	int     Text_LineIndexSearch;				// LineIndexSearch -> Text_LineIndexSearch
	int     Text_ZeroCharIndex;					// nIndexText      -> Text_ZeroCharIndex
	int		Text_MatchStringCharIndexBegin;		// nIndexBegin     -> Text_MatchStringCharIndexBegin
	int     Text_MatchStringCharIndexEnd;		// nIndexEnd       -> Text_MatchStringCharIndexEnd


	char	*pkey_HEADING;            			// pointer to Description file (old name pPRINTOUT)
	char    key_HEADING[MINIBUF];			// pWorkKeyStringHome -> Descr_pKeyString
	int		key_HEADING_index;					// StrIndex           -> Descr_KeyStringLineIndex

	bool	String_DescrText_Match;				// StringFinded		  -> String_DescrText_Match
	bool	String_DescrText_Match_SkipEmptyLines;

	div_t	div_result;
	int		NoPage, NoLine, ColumnCount;
	int		counter;
	char *pString;

	// -----------------------------------------------------------------------------------
	// HEADER
	//
	// Search through file memory (pText) for HEADER key words
	//
	// Searches in way that it finds fist occurance of first string key word. Next key word
	// it searches from that place so that it analyses same printout (first one in file)
	// if there is more than one same printout.
	//
	// Text_LineIndexWork - current line in file pText, it starts from 1 (first line)
	// CharIndexWork - current char position in pText, it starts from 0
	// pText		 - gets first occurance of string starting from Text_pWorking position
	//				   in pText memory
	// -----------------------------------------------------------------------------------

	Text_LineIndexSearch = Project.EditedFile[nIndex].Data.FileAnalisysLineIndex;
	key_HEADING_index = 0;

	String_DescrText_Match   = FALSE;
	String_DescrText_Match_SkipEmptyLines = FALSE;

	// set pkey_HEADING to point on Heading array
	pkey_HEADING = (char *) &Project.EditedFile[nIndex].Data.Keys[key].Heading[0];

	 // this while is to match complete HEADER content for single printout and to
	//  ignore blank useless lines (blanks, newline ...)
	while (TRUE)
	{
		div_result		= div(Text_LineIndexSearch, PAGE_LINES_SIZE);
		NoPage			= div_result.quot;
		NoLine			= Text_LineIndexSearch - NoPage * PAGE_LINES_SIZE;
		ColumnCount		= Project.EditedFile[nIndex].Data.Indexed.Page[NoPage][NoLine];

		FAST3ErrorTraceHandle(3, "DataAnalyse_HEADER() Page %d, Line %d\n",
							NoPage,
							NoLine);

		if (Project.EditedFile[nIndex].Data.pText[Project.EditedFile[nIndex].Data.Indexed.Page[NoPage][NoLine]] == 13)
		{
			Text_LineIndexSearch ++;
			continue; // empty line, ignore
		}
		else
		{
			if (String_DescrText_Match_SkipEmptyLines)
			{
				Project.EditedFile[nIndex].Data.FileAnalisysLineIndex = Text_LineIndexSearch;
				return TRUE; // we manage to match all keys and skip blank lines before VARIABLE
			}
		}

		memcpy(key_HEADING, pkey_HEADING + key_HEADING_index * MINIBUF, MINIBUF);

		FAST3ErrorTraceHandle(3, "DataAnalyse_HEADER() key_HEADING(index-%d) <%s>\n",
							key_HEADING_index, key_HEADING);

		/*FAST2ErrorTraceHandle(3, "DataAnalyse_HEADER() pText            <%s>\n",
							&Project.EditedFile[nIndex].Data.pText[ColumnCount]);
		*/
		// Find Key string comparing whole Key qoutes with whole line in text
		// to avoid substring appearence of key words (e.g. END in **** END OF PRN...)
		// if Key string anywhere in Text line
		if (((pString = (char *)strstr(&Project.EditedFile[nIndex].Data.pText[ColumnCount], key_HEADING)) != NULL) &&
	             // if Key string at 0 position in Text line enter, otherwise ignore to avoid multiple Key  matches in same Text line
		    ((pString == &Project.EditedFile[nIndex].Data.pText[ColumnCount])))
			{
				key_HEADING_index ++; // Next Descr Key line
				Text_LineIndexSearch ++;	 // Next Text line
				String_DescrText_Match = TRUE;
				ErrorTraceHandle(2, "DataAnalyse_HEADER() key_HEADING <%s> located on Page[%d][%d], file line %d\n",
						 key_HEADING, NoPage, NoLine, Text_LineIndexSearch);
			}
		else
			{
				if (String_DescrText_Match) // if already finded some key
					Text_LineIndexSearch ++;	// Next Descr Key search from next Text line
				else
					return FALSE; // This header doesn't match, exit from function
								  // and main loop will try next printout
			}


		if (Project.EditedFile[nIndex].Data.Keys[key].Heading[key_HEADING_index][0] == '\0')
		{
			String_DescrText_Match_SkipEmptyLines = TRUE;
			// allow for this while loop to clean all blank new lines in HEADER part prior to VARIABLE part
		}

		if (Text_LineIndexSearch == Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)
		{
			ErrorTraceHandle(2, "DataAnalyse_HEADER() Didn't manage to match all keys till end of text.\n");
			return FALSE;
		}
	}

	ErrorTraceHandle(3, "DataAnalyse_HEADER() Exit !\n");

	return TRUE;
}


//************************************************************************************
//************************************************************************************
// FUNCTION:	DataAnalyse_HEADER()
//
// PURPOSE:		Analyse and find key words from Printout array in
//				HEADER and VARIABLE part.
//************************************************************************************
//************************************************************************************
bool DataAnalyse_VARIABLE(int nIndex, int key)
{
	int		Text_LineIndex_Count;
	int		Text_InTextCharCounter;

	char	*pkey_VARIABLE_MASK;
	char	*pkey_VARIABLE_NAME;
	int		key_VARIABLE_index; 		// same index   for VARIABLE_NAME and VARIABLE_MASK
	int		key_VARIABLE_CharCounter;	// same counter for VARIABLE_NAME and VARIABLE_MASK

	enum {
		INSIDE_VAR_MISMATCH = 1,
		INSIDE_VAR_BLANK = 2,
		OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT = 4,
		OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH = 8,
		OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT = 16
	};
	int		Char_KeyVsText_Mismatch;

	char	TextCH;
	char	key_VARIABLE_MASK_Char; // may be ' ' or '*', in case of ' ', match key_VARIABLE_NAME_Char char, otherwise match key_VARIABLE_NAME_Char variable
	char	key_VARIABLE_NAME_Char;

	int		NoPage, NoLine;
	char	itoaBuff[MINIBUF];
	div_t	div_result;
	bool	test = FALSE;
	int		Counter, CounterNew;

	FAST3ErrorTraceHandle(2, "DataAnalyse_VARIABLE() nIndex %d, key %d\n",
						nIndex,
						key);

	// -----------------------------------------------------------------------------------
	// VARIABLE
	//
	// Search through file memory (pText) for VARIABLE words and get their values and
	// set text property for analysed values.
	//
	// It searches one row by one row in a way that row is analysed with each Variable
	// Key string available so that if in one Variable Key String appears Char Mismatch
	// it tries to find other Variable Key String. If all Variable Key Strings are mismatched
	// it skips the row.
	// -----------------------------------------------------------------------------------

	{
		// -----------------------------------------------------------------------------------
		// Reset all TmpVariable.TotalRows that use as New Value counters for each Variable.
		// Reset all TmpVariable.TotalPages that uses as total pages counter for each Variable.
		// -----------------------------------------------------------------------------------
		for (Counter = 0; Counter < VARIABLE_MAX; Counter ++)
		{
			Project.EditedFile[nIndex].Data.TmpVariable[Counter].TotalRows = 0;
			Project.EditedFile[nIndex].Data.TmpVariable[Counter].TotalPages = 0;
		}

		// set pkey_VARIABLE_NAME to point on Variable array
		pkey_VARIABLE_MASK = (char *) &Project.EditedFile[nIndex].Data.Keys[key].Variable_Mask[0];
		pkey_VARIABLE_NAME = (char *) &Project.EditedFile[nIndex].Data.Keys[key].Variable_Name[0];

		Text_LineIndex_Count = Project.EditedFile[nIndex].Data.FileAnalisysLineIndex;

		// -----------------------------------------------------------------------------------
		// In this WHILE scan each Text Line in VARIABLE part with first rule from
		// Project.EditedFile[nIndex].Data.Keys[key].Variable that matches
		// -----------------------------------------------------------------------------------
		while (TRUE)
		{
			if (Text_LineIndex_Count >= Project.EditedFile[nIndex].Data.Indexed.NoTotalLines)
				break;

			// -----------------------------------------------------------------------------------
			// NoPage		Text_LineIndex_Count position in page index.
			// NoLine		Text_LineIndex_Count position in line index.
			// key_VARIABLE_index - line offset from Variable array index
			// -----------------------------------------------------------------------------------
			div_result	= div(Text_LineIndex_Count, PAGE_LINES_SIZE);
			NoPage		= div_result.quot;
			NoLine		= Text_LineIndex_Count - NoPage * PAGE_LINES_SIZE;
			key_VARIABLE_index = 0;

			// -----------------------------------------------------------------------------------
			// In this while sequence it analyses Text line with first Desc line in VARIABLE part,
			// if there is more than one, for one key_VARIABLE_index index
			// -----------------------------------------------------------------------------------
			while (TRUE)
			{
				int	 VARS_LineIndexToName[VARIABLE_MAX], VARS_LineIndexCnt;
				char VARS_CharsActive[VARIABLE_MAX];
				char VARS_CharsData[VARIABLE_MAX][VARIABLE_MAX_LENGTH];
				int	 VARS_CharsColumnIndex[VARIABLE_MAX][VARIABLE_MAX_LENGTH];
				int	 VAR_Counter, VAR_Pages, VAR_Name;
				int  TmpLargest;

				// -----------------------------------------------------------------------------------
				// key_VARIABLE_CharCounter		counter used for one row from begin until end (0..) for key_VARIABLE_NAME_Char
				// Text_InTextCharCounter		counter used for one row from begin until end (0..) for TextCH
				// Char_KeyVsText_Mismatch		indicates if TextCH mismatched with key_VARIABLE_NAME_Char in this row.
				// VAR_CharsActive[]char counter index for each variable detected in this row.
				//					It is an array because if there are more variables in file
				//					then there is no need to allocate memory for each one but
				//					only for ones that appeared in this row. This array is tested
				//					also when new page should be allocated to avoid more same allocations
				//					in same row (allocated only for first char of same variable if
				//					stepping to new page). It is tested and when end of row comes
				//					so that TmpVariable.TotalRows counter increase only for variables
				//					that appeared in this row.
				// -----------------------------------------------------------------------------------

NEWKEY_NEWTEXT:
				key_VARIABLE_CharCounter	= 0; //NO_INFO_CHARS;
				Text_InTextCharCounter		= Project.EditedFile[nIndex].Data.Indexed.Page[NoPage][NoLine];
				Char_KeyVsText_Mismatch		= 0;
				for (Counter = 0; Counter < VARIABLE_MAX; Counter ++)
				{
					VARS_CharsActive[Counter] = 0;
					VARS_CharsData[Counter][0] = 0;
					VARS_LineIndexToName[Counter] = 0xFF;
				}
				VARS_LineIndexCnt = 0;
				// -----------------------------------------------------------------------------------
				// Analyse one row from first to last char in it with rule from printout
				// -----------------------------------------------------------------------------------
				while (TRUE)
				{
					// -----------------------------------------------------------------------------------
					// TextCH	Char in edit document.
					// key_VARIABLE_NAME_Char	Char in Key string.
					// -----------------------------------------------------------------------------------
					TextCH	= Project.EditedFile[nIndex].Data.pText[Text_InTextCharCounter];
					key_VARIABLE_MASK_Char	= *(pkey_VARIABLE_MASK + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter);
					key_VARIABLE_NAME_Char	= *(pkey_VARIABLE_NAME + key_VARIABLE_index * MINIBUF + key_VARIABLE_CharCounter);

// - ENDING OF KEY STRING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// - ENDING OF KEY STRING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// - ENDING OF KEY STRING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
					if (key_VARIABLE_MASK_Char == 0)
					{
						if (!((TextCH == 13) /* new line */ || (TextCH == 10) /* line feed */)) {
							// no excuse for this, description ended, and text did't end
							FAST1ErrorTraceHandle(2, "VARIABLE() OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT 1.\n");
							Char_KeyVsText_Mismatch |= OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT;
						} else {
PROCESS_ACTIVE_CHARS:
							// VARIABLE_MAX represents max number of variables that could be in one row (same variables or different ones)
							for (Counter = 0; Counter < VARIABLE_MAX; Counter ++)  // Counter is actually VARS_LineIndexCnt
							{
								if (VARS_CharsActive[Counter] != 0)
								{
									VAR_Name = VARS_LineIndexToName[Counter];
									Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows ++;

									VAR_Counter	= Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows;   // Counter 1..
									VAR_Pages	= Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalPages;  // Counter 1..
									// Set Variable counter in range 1..VARIABLE_PAGE_ROWS
									if (VAR_Pages > 0)
									{
										if ((VAR_Counter - (VARIABLE_PAGE_ROWS * VAR_Pages)) > 0)
											VAR_Counter	= VAR_Counter - (VARIABLE_PAGE_ROWS * VAR_Pages);
										else
											VAR_Counter	= VAR_Counter - (VARIABLE_PAGE_ROWS * (VAR_Pages - 1));
									}
									// -----------------------------------------------------------------------------------
									// Checks if Variable encountered Max Page Rows counter, if Yes then
									// allocate new Page for this Variable and work in new Page for this
									// Variable.
									//
									// Conditions are:
									//		(VAR_Counter == 1)	allocate new page only if VAR_Counter for this variable
									//							overrided value VARIABLE_PAGE_ROWS and therefore reseted
									//							to value 1 (in new page is to be allocated).
									// -----------------------------------------------------------------------------------
									if (VAR_Counter == 1)
									{
										Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalPages ++;
										VAR_Pages = Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalPages;

										// Allocate new Page for this Variable of VARIABLE_PAGE_ROWS x VARIABLE_MAX_LENGTH
										Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].Page[VAR_Pages - 1] =
											(pTmpVariablePageString *) malloc(sizeof(pTmpVariablePageString));

										if (Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].Page[VAR_Pages - 1] == NULL)
											ErrorTraceHandle(0, "Error Allocating Memory Page with index <%d> for Variable <%c>.\n",
															// nešta èudno inet_ntoa((VAR_Pages - 1), itoaBuff, 10),
														    (VAR_Pages - 1),
															(VAR_Name + 'A'));

										// initialize to NULL strings for each Variable on page
										for (CounterNew = 0; CounterNew < VARIABLE_PAGE_ROWS; CounterNew ++)
											Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].Page[VAR_Pages - 1]->
												pString[CounterNew][0] = 0;
									}

									// Set new char over previous variable NULL char.
									strcpy(Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].Page[VAR_Pages - 1]->
									pString[VAR_Counter - 1], VARS_CharsData[Counter]);

									// Calculate largest string value for that variable
									TmpLargest = VARS_CharsActive[Counter] + 1;
									if (TmpLargest > Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].LargestString)
										Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].LargestString = TmpLargest;
								}
							}
							goto STOP_PROCESS_THIS_TEXT_LINE; // break to next Text line search, this Text line is finished
						}
// - ENDING OF KEY STRING <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// - ENDING OF KEY STRING <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// - ENDING OF KEY STRING <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					}
// - If VARIABLE_MASK is not indicating variable in VARIABLE_NAME ---------------------
					else if (key_VARIABLE_MASK_Char != '*') {
// - SKIP asterix chars ---------------------------------------------------------------
						if (key_VARIABLE_NAME_Char == '*')
						{
							if ((TextCH == 13) /* new line */ || (TextCH == 10) /* line feed */) {
								FAST1ErrorTraceHandle(2, "VARIABLE() OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT 1.\n");
								Char_KeyVsText_Mismatch |= OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT;
							} else {
								if (VARS_CharsActive[VARS_LineIndexCnt] != 0)
									VARS_LineIndexCnt ++; // set new variable ongoing
							}
// - Everything else that is not a VARIABLE or to SKIP ------------------------
						} else if (key_VARIABLE_NAME_Char != TextCH) {
							if ((TextCH == 13) /* new line */ || (TextCH == 10) /* line feed */) {
								FAST1ErrorTraceHandle(2, "VARIABLE() OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT 3.\n");
								Char_KeyVsText_Mismatch |= OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT;
							} else {
								FAST1ErrorTraceHandle(2, "VARIABLE() OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH.\n");
								Char_KeyVsText_Mismatch |= OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH;
								//debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
							}
						} else {
							if (VARS_CharsActive[VARS_LineIndexCnt] != 0)
								VARS_LineIndexCnt ++; // set new variable in line ongoing
						}

					} else { // key_VARIABLE_MASK_Char == '*'

// - Analyse VARIABLES from 'A' ... ----------------------------------------------------
						if ((key_VARIABLE_NAME_Char >= 'A') && (key_VARIABLE_NAME_Char<=('A'+ VARIABLE_MAX - 1)))	// Variable Char
						{
							if ((TextCH == 13) /* new line */ || (TextCH == 10) /* line feed */) {
								FAST1ErrorTraceHandle(2, "VARIABLE() OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT 2.\n");
								Char_KeyVsText_Mismatch |= OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT;
							} else if (((TextCH >= '0') && (TextCH <= '9')) ||
									   ((TextCH >= 'A') && (TextCH <= 'F')) ||
									   ((TextCH >= 'a') && (TextCH <= 'f')))   // If value of TextCH within searched range
							{
								// -----------------------------------------------------------------------------------
								// VAR_Name		Variable Index (eg 0 -> 'A', 1 -> 'B' ...)
								// VAR_Counter	Variable counter in one page for this variable.
								// VAR_Pages	Variable number of pages.
								// VAR_Chars	If Variable appearred, set that it is active in this row.
								// -----------------------------------------------------------------------------------
								VAR_Name = key_VARIABLE_NAME_Char - 'A'; // name of ongoing variable
								if ((VARS_CharsActive[VARS_LineIndexCnt] != 0) &&
									(VARS_LineIndexToName[VARS_LineIndexCnt] != VAR_Name))
									VARS_LineIndexCnt ++; // set new variable ongoing

								VARS_LineIndexToName[VARS_LineIndexCnt] = VAR_Name;

								// Set new char over previous variable NULL char.
								VARS_CharsData[VARS_LineIndexCnt][VARS_CharsActive[VARS_LineIndexCnt]] = TextCH;
								// Set NULL char on the end every time.
								VARS_CharsData[VARS_LineIndexCnt][VARS_CharsActive[VARS_LineIndexCnt]+1] = 0;

								// Store Text_InTextCharCounter of every Variable char finded, and on the end of
								// analysed row, if no Chars mismatched, set chars properties (italic, colour, bold).
								VARS_CharsColumnIndex[VARS_LineIndexCnt][VARS_CharsActive[VARS_LineIndexCnt]] = Text_InTextCharCounter;

								VARS_CharsActive[VARS_LineIndexCnt] ++;
							}
							// If TextCH is not in searched Variable Name range 'A'... ('A'+VARIABLE_MAX)
							else
							{
								if (TextCH == ' ') // if same variable has space like "H'0000 0000" ignore it, otherwise mismatch
									Char_KeyVsText_Mismatch |= INSIDE_VAR_BLANK;
								else
									Char_KeyVsText_Mismatch |= INSIDE_VAR_MISMATCH;
							}
						} else {
							if (VARS_CharsActive[VARS_LineIndexCnt] != 0)
								VARS_LineIndexCnt ++; // set new variable in line ongoing
						}
					}

// - Check for mismatches ------------------------
					if (Char_KeyVsText_Mismatch) {
						// Char IS mismatched outside VAR, check if there is at least one var stored correctly
						if (Char_KeyVsText_Mismatch & OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT)
						{
							int cnt, any_active = 0;

							//debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() inside action OUTSIDE_VAR_MORE_KEY_BUT_NOMORE_TEXT", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);

							for (cnt=0; cnt<VARS_LineIndexCnt; cnt++)
								if (VARS_CharsActive[cnt] != 0)
									// if any variable of detected ones in row is active, if VARS_LineIndexCnt=0 then
									// ignore this since we can't be shure that this var is compleeted
									{any_active=1; break;}
							FAST2ErrorTraceHandle(2, "VARIABLE() inside action any value found = %d.\n",
												 any_active);
							if (any_active != 0)
								goto PROCESS_ACTIVE_CHARS;
							else { // if there is no completed variables, advance this
								   // state to OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH and break since this line didn't
								   // produce any variable value
								Char_KeyVsText_Mismatch |= OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH;
								goto STOP_PROCESS_THIS_TEXT_LINE;
							}
						} else if ((Char_KeyVsText_Mismatch & OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH) ||
								   (Char_KeyVsText_Mismatch & OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT) ||
								   (Char_KeyVsText_Mismatch & INSIDE_VAR_MISMATCH)) {
							// Char IS mismatched inside VAR, exit WHILE and continue with checking of
							// current Descr keys and start over for new descr keys
							if (Char_KeyVsText_Mismatch & OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH) debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() inside action NOK OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
							if (Char_KeyVsText_Mismatch & OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT)	  debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() inside action NOK OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
							if (Char_KeyVsText_Mismatch & INSIDE_VAR_MISMATCH)				      debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() inside action NOK INSIDE_VAR_MISMATCH", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
							goto STOP_PROCESS_THIS_TEXT_LINE;
						}
					}
					key_VARIABLE_CharCounter = key_VARIABLE_CharCounter + 1; // Icrease key_VARIABLE_CharCounter counter used for key_VARIABLE_NAME_Char
					Text_InTextCharCounter   = Text_InTextCharCounter  + 1;	 // Icrease Text_InLineCharCounter counter used for TextCH
				}

STOP_PROCESS_THIS_TEXT_LINE:
				if ((Char_KeyVsText_Mismatch & OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH) ||
					(Char_KeyVsText_Mismatch & OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT) ||
					(Char_KeyVsText_Mismatch & INSIDE_VAR_MISMATCH))
				{
					if (Char_KeyVsText_Mismatch & OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH) debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() outside action NOK OUTSIDE_VAR_MORE_KEY_BUT_TEXT_MISMATCH", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
					if (Char_KeyVsText_Mismatch & OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT)	  debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() outside action NOK OUTSIDE_VAR_NOMORE_KEY_BUT_MORETEXT", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
					if (Char_KeyVsText_Mismatch & INSIDE_VAR_MISMATCH)				      debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() outside action NOK INSIDE_VAR_MISMATCH", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);

					// try to see if there is more Descr lines for matching this Text line
					key_VARIABLE_index 		 ++;
					// ... and if there is no more Variable Key strings left
					if (Project.EditedFile[nIndex].Data.Keys[key].Variable_Name[key_VARIABLE_index][0] == '\0') {
						FAST1ErrorTraceHandle(2, "VARIABLE() No more keys, goto FINISH.\n");
						goto FINISH; // detected unmatcheable line, return to main
						//break; // just continue to next Text line, this one is hopeless
					}
					goto NEWKEY_NEWTEXT;

				} else {
					debug_text_descr(nIndex, key, Char_KeyVsText_Mismatch, "VARIABLE() outside action OK", Text_LineIndex_Count, Text_InTextCharCounter, key_VARIABLE_index, key_VARIABLE_CharCounter);
					goto NEXT_TEXT_LINE; // it's ok, let's go to next line
				}
			}

NEXT_TEXT_LINE:
			// -----------------------------------------------------------------------------------
			// This is the end of analyse of one Text line. Increase Text_LineIndex_Count counter
			// -----------------------------------------------------------------------------------
			Text_LineIndex_Count ++;

			//if ((Text_LineIndex_Count % 10)==0)
			{
				//sprintf(status, "Analysing variable part: %5d", Text_LineIndex_Count);
				//SendMessage(hStatusBar, SB_SETTEXT, 0, status);
			}

		}

FINISH:
		Project.EditedFile[nIndex].Data.FileAnalisysLineIndex = Text_LineIndex_Count;

	}

	if (!ExportVariableValuesToStringStorage(nIndex, key))
		return FALSE;

	return TRUE;
}




//************************************************************************************
//************************************************************************************
// FUNCTION:	ExportVariableValuesToStringStorage()
//
// PURPOSE:		Store analysed values from array in file.
//************************************************************************************
//************************************************************************************
bool ExportVariableValuesToStringStorage(int nIndex, int key)
{
	int				VAR_Counter, VAR_Pages, VAR_Name, TotRowCount;
	bool			END_SEARCH;
	long			AreaSize;
	char			*pStorageAddress, *pStorageSource;
	int				StorageCount;
	int				StorageCellSize;

	ErrorTraceHandle(3, "ExportVariableValuesToStringStorage().\n");

	for (VAR_Name = 0; VAR_Name < VARIABLE_MAX; VAR_Name ++)
	{
		END_SEARCH		= FALSE;
		TotRowCount		= 0;
		VAR_Pages		= 0;

		if (Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows > 0)
		{
			StorageCount	= 0;
			StorageCellSize = Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].LargestString;

			// ----------------- Variable[VAR_Name].Storage_CellSize -------------------
			// If there was already stored values in this variable, e.g. if there were more printouts of
			// same variable, concatenate new tmp data to old storage
			if (Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize != 0) {
				// also cell size of previous one should not be smaller than new one
				if (Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize != StorageCellSize) {
					if (StorageCellSize > Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize) {
						ErrorTraceHandle(1, "ExportVariableValuesToStringStorage() Can not concatenate new larger size data to old smaller in mem storage, ignore this data.\n");
						return TRUE;
					} else
						ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() New data cell size (%d) equilized with old ones (%d).\n",
								StorageCellSize,
								Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize);

				} else
					ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() New data (%d values) concatenated to old ones (%d values) in mem storage.\n",
							Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows,
							Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber);
			}
			Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize = StorageCellSize;

			// ----------------- Variable[VAR_Name].Storage -------------------
			AreaSize = Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows *		// Total number of variables
					   Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].LargestString *	// Largest size of variable string
					   sizeof(char);

			// if previous storage is not empty
			if (Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage != NULL) {
				unsigned int oldAreaSize = Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellSize *
										   Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber *
										   sizeof(char);
				char *tmp_Storage = (char *) malloc (oldAreaSize+AreaSize);
				char *tmp_cpyPtr, tmp_cell[100];
				if (tmp_Storage == NULL) {
					ErrorTraceHandle(1, "ExportVariableValuesToStringStorage() Can not Allocate extended memory.\n");
					return FALSE;
				} else
					ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() Allocated extended (old%d+new%d=%d) bytes of memory starting from 0x%x.\n",
							oldAreaSize, AreaSize, oldAreaSize+AreaSize, tmp_Storage);

				memset(tmp_Storage, 0, oldAreaSize+AreaSize);
				tmp_cpyPtr = memcpy(tmp_Storage, Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage, oldAreaSize);
				if (tmp_cpyPtr != NULL) {
					memcpy(tmp_cell, (char *)(tmp_cpyPtr+(Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber-1)*StorageCellSize), StorageCellSize);
					ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() Last copy on 0x%x=%s.\n", tmp_cpyPtr, tmp_cell);
				}
				free(Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage);
				Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage = tmp_Storage;
			} else {
				if ((Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage = (char *) malloc (AreaSize)) == NULL) {
					ErrorTraceHandle(1, "ExportVariableValuesToStringStorage() Can not Allocate memory.\n");
					return FALSE;
				}
				memset(Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage, 0, AreaSize);
			}

			// ----------------- Variable[VAR_Name].Storage_CellsNumber -------------------
			// if previous storage is not empty
			if (Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber != 0) {
				StorageCount = Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber;
				ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() Old Storage_CellsNumber=%d.\n",
						Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber);
			}


			while (TRUE)
			{
				VAR_Pages ++;

				for (VAR_Counter = 0; VAR_Counter < VARIABLE_PAGE_ROWS; VAR_Counter ++)
				{
					TotRowCount ++;

					if (TotRowCount > Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].TotalRows) {
						END_SEARCH = TRUE;
						break;
					}

					pStorageAddress	= Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage +
									  StorageCount * StorageCellSize;
					pStorageSource	= Project.EditedFile[nIndex].Data.TmpVariable[VAR_Name].Page[VAR_Pages - 1]->pString[VAR_Counter];

					FAST7ErrorTraceHandle(2, "ExportVariableValuesToStringStorage() Var_%d[count-%d, addr-0x%x] = <%s>(page-%d,counter-%d)\n",
									VAR_Name,
									StorageCount,
									pStorageAddress,
									pStorageSource,
									VAR_Pages - 1,
									VAR_Counter);

					memcpy(pStorageAddress, pStorageSource, strlen(pStorageSource));

					StorageCount ++;
				}

				if (END_SEARCH)
				{
					Project.EditedFile[nIndex].Data.Variable[VAR_Name].Storage_CellsNumber = StorageCount;
					Project.EditedFile[nIndex].Data.Variable[VAR_Name].KeyIndex = key;
					break;
				}
			}
		}
	}
	// Release PAGE and LINES memory
	{
		int Counter, CounterA;

		// TmpVariable[]
		for (Counter = 0; Counter < VARIABLE_MAX; Counter ++)
			for (CounterA = 0;
				 CounterA <= Project.EditedFile[nIndex].Data.TmpVariable[Counter].TotalPages - 1;
				 CounterA ++)
				if (Project.EditedFile[nIndex].Data.TmpVariable[Counter].Page[CounterA] != NULL)
				{
					free(Project.EditedFile[nIndex].Data.TmpVariable[Counter].Page[CounterA]);
					Project.EditedFile[nIndex].Data.TmpVariable[Counter].Page[CounterA] = NULL;
				}

		for (Counter = 0; Counter < VARIABLE_MAX; Counter ++)
		{
			Project.EditedFile[nIndex].Data.TmpVariable[Counter].TotalPages = 0;
			Project.EditedFile[nIndex].Data.TmpVariable[Counter].TotalRows = 0;
		}
	}

	return TRUE;
}
