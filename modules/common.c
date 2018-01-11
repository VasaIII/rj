#define FAST

#define RJ_COMMON
#include <modules/common.h>
#undef RJ_COMMON

// ***********************************************************************************
//
// - do a init on start program to read all values in memory
// - do a refresh on some phases in execution to store all new/changed values from memory to file
// ***********************************************************************************
char *InitFileHandle(int Init0_Read1_Write2_Refresh3, char key[], char value[]) {
	static char memory_key_value[100][MINIBUF]; // [number of keys-values][MINIBUF max size]
	static int stack_mem;
	FILE *fptrace;

	static char file_key_value[100][MINIBUF]; // [number of keys-values][MINIBUF max size]
	static int stack_file;

	char *tmp;
	int i, j;

	if (Init0_Read1_Write2_Refresh3 == 0) {
		stack_mem = 0;
		stack_file = 0;

		FAST1ErrorTraceHandle(2, "InitFileHandle() - INITIALIZATION of rj.init\n");

		for (i = 0; i < 100; i++) {
			memory_key_value[i][0] = '\0';
			file_key_value[i][0] = '\0';
		}

		if ((fptrace = fopen(".rj", "a+")) == NULL)
			fptrace = fopen(".rj", "w+");
		else {
			fseek(fptrace, 0, SEEK_SET); // to start

			//The fgets() function reads characters from the  stream  into
			//the array pointed to by s, until n-1 characters are read, or
			//a newline character is read and  transferred  to  s,  or  an
			//end-of-file  condition  is  encountered.  The string is then
			//terminated with a null character.
			while ((tmp = fgets(tmp, MINIBUF * sizeof(char), fptrace)) != NULL) // copy size MINIBUF from file or till end of line to file buffer array
			{
				strcpy(file_key_value[stack_file], tmp);
				FAST2ErrorTraceHandle(2, "InitFileHandle() - in .rj finded <%s> in file\n",
									      file_key_value[stack_file]);
				stack_file++;
				if (stack_file == 100) {
					FAST1ErrorTraceHandle(2, "ERROR !!! InitFileHandle() - max size of .rj stack is 100.\n");
					return NULL;
				}
			}
		}
		fclose(fptrace);
		return "OK";
	}
	if (Init0_Read1_Write2_Refresh3 == 1) {
		FAST2ErrorTraceHandle(2, "InitFileHandle() - READING <%s> ...\n", key);
		for (j = stack_mem; j >= 0; j--)
			if (!strcmp(GetWord(memory_key_value[j], 0, 0, 0), key)) {
				FAST2ErrorTraceHandle(2, "InitFileHandle() - READING latest located in memory <%s> ...\n",
								 memory_key_value[j]);
				return (char *) &memory_key_value[j];
			}
		for (j = stack_file; j >= 0; j--)
			// clear all same keys from file
			if (!strcmp(GetWord(file_key_value[j], 0, 0, 0), key)) {
				FAST2ErrorTraceHandle(2, "InitFileHandle() - READING latest located in file <%s> ...\n",
							     file_key_value[j]);
				return (char *) &file_key_value[j];
			}
	} else if (Init0_Read1_Write2_Refresh3 == 2) {
		sprintf(memory_key_value[stack_mem], "%s SET_TO %s\n", key, value);

		FAST2ErrorTraceHandle(2, "InitFileHandle() - WRITING to memory <%s>\n",
				memory_key_value[stack_mem]);

		stack_mem++;
		if (stack_mem == 100) {
			FAST1ErrorTraceHandle(2, "ERROR !!! InitFileHandle() - max size of .rj stack is 100.\n");
			return NULL;
		}
		return "OK";
	} else if (Init0_Read1_Write2_Refresh3 == 3) {
		stack_file = 0;
		stack_mem = 0;

		FAST1ErrorTraceHandle(2, "InitFileHandle() - REFRESHING .rj file\n");

		// rewrite new .rj file
		fptrace = fopen(".rj", "w+");

		for (i = stack_mem; i >= 0; i--) // start from the latest in memory
		{
			for (j = stack_file; j >= 0; j--)
				// clear all same keys from file
				if (!strcmp(GetWord(memory_key_value[i], 0, 0, 0), GetWord(
						file_key_value[j], 0, 0, 0))) {
					file_key_value[j][0] = '\0';
					break; // there should be no more than one unique key value in file
				}
			for (j = stack_mem; j >= 0; j--)
				// clear all same keys previous defined from memory
				if (j < i) // if j older than i
					if (!strcmp(GetWord(memory_key_value[i], 0, 0, 0), GetWord(
							memory_key_value[j], 0, 0, 0)))
						memory_key_value[j][0] = '\0'; // there can be multiple same definitions in memory
		}

		for (i = 0; i < 100; i++)
			if (memory_key_value[i][0] != '\0')
				fputs(memory_key_value[i], fptrace);
		for (i = 0; i < 100; i++)
			if (file_key_value[i][0] != '\0')
				fputs(file_key_value[i], fptrace);

		fclose(fptrace);
		return "OK";
	}

	return NULL;
}

// ***********************************************************************************
//
// From string get word (line, position)
//
// ***********************************************************************************
char *GetWord(char *analyse_buffer, int Req0_Array1, int line, int position) // line 0... and position 0...
{
  int current_word_position = 1;
  int current_word_line = 1;
  int current_char_in_word_counter = 0;
  int size_of_string_value = strlen(analyse_buffer);
  char *ch;
  char *first_on_newline_pointer; // to be able to calculate relative offset for word in line
  static char requested_word[1000];

  if (Req0_Array1) {
    current_word_position = 0; // arrays starts from 0...
    current_word_line = 0;     // arrays starts from 0...
  } else {
    current_word_position = 1; // requests starts from 1...
    current_word_line = 1;     // requests starts from 1...
  }

  line = line + current_word_line;
  position = position + current_word_position;

  if (analyse_buffer == NULL)
    FAST1ErrorTraceHandle(0, "GetWord() - No string to analyse !\n");

  ch = analyse_buffer;
  first_on_newline_pointer = ch;

  while (1) {

    // delimiters
    if ((*ch == ' ') || // space
	(*ch == '\t'))  // tab
      {
	requested_word[current_char_in_word_counter] = '\0';
	if (requested_word[0] != '\0') {
	  if (Req0_Array1) {
	    strcpy(GetWord_sObjectMatrics[current_word_line][current_word_position].word, (char *) requested_word);
	    GetWord_sObjectMatrics[current_word_line][current_word_position].offset
	      = (ch - first_on_newline_pointer - strlen(requested_word));

	    FAST5ErrorTraceHandle(4, "(%d,%d(%d)) <%s>\n", current_word_line, current_word_position,
			     lineObj[current_word_line][current_word_position].offset,
			     lineObj[current_word_line][current_word_position].word);

	    current_word_position++;
	  } else {

		FAST4ErrorTraceHandle(4, "(%d,%d) <%s>\n", current_word_line, current_word_position, requested_word);
	    FAST3ErrorTraceHandle(4, "REQUESTED (%d,%d)\n", line, position);

	    if ((current_word_line == line) && (current_word_position == position)) {
	      FAST4ErrorTraceHandle(4, "FINDED (%d,%d) <%s>\n",
			       current_word_line, current_word_position,
			       requested_word);
	      return (char *) requested_word;
	    }
	    current_word_position++;
	    if ((line < current_word_line) || ((line == current_word_line) && (position < current_word_position)))
	      return NULL; // if exceeded requested line/position, exit with NULL
	  }
	}
	current_char_in_word_counter = 0;
      }
    else if ((*ch == '\n') ||
	     (*ch == '\0'))
      {
	requested_word[current_char_in_word_counter] = '\0';

	if (requested_word[0] != '\0') {
	  if (Req0_Array1) {
	    strcpy(GetWord_sObjectMatrics[current_word_line][current_word_position].word, (char *) requested_word);
	    GetWord_sObjectMatrics[current_word_line][current_word_position].offset
	      = (ch - first_on_newline_pointer - strlen(requested_word));

	    FAST5ErrorTraceHandle(4, "(%d,%d(%d)) <%s>\n", current_word_line, current_word_position,
			     lineObj[current_word_line][current_word_position].offset,
			     lineObj[current_word_line][current_word_position].word);

	    current_word_position = 0;
	    current_word_line++;
	  } else {
	    FAST4ErrorTraceHandle(4, "(%d,%d) <%s>\n", current_word_line, current_word_position, requested_word);
	    FAST3ErrorTraceHandle(4, "REQUESTED (%d,%d)\n", line, position);

	    if ((current_word_line == line) && (current_word_position == position)) {
	      FAST4ErrorTraceHandle(4, "FINDED (%d,%d) <%s>\n", current_word_line, current_word_position, requested_word);
	      return (char *) requested_word;
	    }
	    current_word_position = 1;
	    current_word_line++;
	    // there is no so many words in previous line
	    if ((line < current_word_line) || ((line == current_word_line) && (position < current_word_position)))
	      return NULL;
	    if (*ch == '\0')
	      return (char *) requested_word; // there are chars in requested_word[]
	  }
	}
	if (*ch == '\0') // there are NO chars in requested_word[]
	  return NULL;

	current_char_in_word_counter = 0;
	first_on_newline_pointer = ch + 1; // new line starts ...
      }
    else if (*ch == '\r')
      {
    	FAST1ErrorTraceHandle(4, "\\R occured\n");
      }
    else // regular character
      {
	//if (((int)(*ch)>=32) && ((int)(*ch)<=126))
	{
	  requested_word[current_char_in_word_counter] = *ch;
	  requested_word[current_char_in_word_counter + 1] = '\0';
	  if (Req0_Array1) {
	    //lineObj[current_word_line+1][0].word[0]='\n';   // set next line = NULL
	    //lineObj[current_word_line+2][0].word[0]='\0';   // set next line = NULL
	    GetWord_sObjectMatrics[current_word_line][current_word_position + 1].word[0] = '\0'; // set next in row = NULL
	  }
	  // FAST2ErrorTraceHandle(4, "Building word: <%s>\n", requested_word);
	  current_char_in_word_counter++;
	}
	// for (i=0;i<=255;i++)
	//   FAST3ErrorTraceHandle(3, "ASCII(%c)(%d)\n", i,i);
      }
    ch++;
  }
  return NULL;
}

// ***********************************************************************************
//
// Converts a hex ascii character string to an unsigned 32 bit value.
//
// ***********************************************************************************
int HexString2Int(char **s_p, int *result) {
	char *p_p = *s_p, *start_p = *s_p;
	int value = 0;
	int i = 0;
	int j = 1;
	int ok = 0;

	/* This while loop moves the pointer forwards to the last hex
	 * character, and then in the next while loop the pointer will
	 * be moved backwards again and at the same time the characters
	 * will be converted into a hex value.
	 */

	while (*p_p != '\0' && *p_p != '\t' && *p_p != ' ' && *p_p != '\n' && *p_p
			!= '\r') {
		p_p++;
	}
	p_p--;
	while (ok == 0 && p_p >= *s_p && *p_p != '\0' && *p_p != '\t' && *p_p
			!= ' ' && *p_p != '\n' && *p_p != '\r') {
		switch (*p_p) {
		case '0':
			;
			break;
		case '1':
			value += 1 * Pow(16, i);
			break;
		case '2':
			value += 2 * Pow(16, i);
			break;
		case '3':
			value += 3 * Pow(16, i);
			break;
		case '4':
			value += 4 * Pow(16, i);
			break;
		case '5':
			value += 5 * Pow(16, i);
			break;
		case '6':
			value += 6 * Pow(16, i);
			break;
		case '7':
			value += 7 * Pow(16, i);
			break;
		case '8':
			value += 8 * Pow(16, i);
			break;
		case '9':
			value += 9 * Pow(16, i);
			break;
		case 'A':
		case 'a':
			value += 10 * Pow(16, i);
			break;
		case 'B':
		case 'b':
			value += 11 * Pow(16, i);
			break;
		case 'C':
		case 'c':
			value += 12 * Pow(16, i);
			break;
		case 'D':
		case 'd':
			value += 13 * Pow(16, i);
			break;
		case 'E':
		case 'e':
			value += 14 * Pow(16, i);
			break;
		case 'F':
		case 'f':
			value += 15 * Pow(16, i);
			break;
		default:
			ok = 1;
			break;
		}
		i++;
		p_p--;
	}
	p_p++;
	while (*p_p != '\0' && *p_p != '\t' && *p_p != ' ' && *p_p != '\n' && *p_p
			!= '\r') {
		if (j && (*p_p == '0')) {
			i--;
		} else {
			j = 0;
		}
		p_p++;
	}
	*s_p = start_p; // prominija iz *s_p = p_p; jer bi se nakon svakog poziva funkcije pomaka pointer
	*result = value;

	return ok;

} /* HexString2Int */

// ***********************************************************************************
//
// Returns base^exp
//
// ***********************************************************************************
int Pow(const int base, const int exp) {
	int result = 1;
	int i;

	for (i = 0; i < exp; i++) {
		result = base * result;
	}

	return result;

} /* Pow */

// ***********************************************************************************
// Print in binary
//
// ***********************************************************************************
void Int2BinaryString(int no) {
	int powerof2 = 1;
	int count = 0, count2, i, j;
	char binary_out[200];
	char binary_out_inv[200];

	while (powerof2 * 2 <= no)
		// Find largest power of 2
		powerof2 = powerof2 * 2; // less than or equal to no

	do {
		if (no >= powerof2) {
			binary_out[count] = '1'; // Print a 1 in this position
			no = no - powerof2;
		} else
			binary_out[count] = '0'; // Print a 0 in this position

		count++;
		powerof2 = powerof2 / 2;
	} while (powerof2 > 0);
	binary_out[count] = '\0';

	count2 = 0;
	j = 0;
	for (i = count - 1; i >= 0; i--) {
		binary_out_inv[count2] = binary_out[i];
		j++;
		count2++;
		if (j == 4) {
			j = 0;
			binary_out_inv[count2] = ' ';
			count2++;
		}
	}
	binary_out_inv[count2] = '\0';

	count = 0;
	for (i = count2 - 1; i >= 0; i--) {
		binary_out[count] = binary_out_inv[i];
		count++;
	}
	binary_out[count] = '\0';

	fprintf(stdout, "%s", binary_out);
}


// ***********************************************************************************
//
// Copyies whole regular file content to memory which allocates also here.
//
// ***********************************************************************************

char *File2PagedBuffer(
			 char *directory,
		     char *file,
		     int  *num_read,
		     int  *Page,
		     int   maxp, int maxl,
		     int  *PageCount,
		     int  *TotalLines)
{
  static char filename[80];
  int file_size;
  FILE *fp;
  char *buffp;

  if (directory != NULL)
    sprintf(filename, "%s/%s", directory, file);
  else
    strcpy(filename, file);


  if ((fp = fopen(filename, "r")) == NULL) {
    FAST2ErrorTraceHandle(0, "File2PagedBuffer() Can't open file <%s> !\n", filename);
  }
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if ((buffp = (char *) malloc(file_size)) == NULL) {
    FAST2ErrorTraceHandle(0, "File2PagedBuffer() Can't malloc mem for file size of %d bytes !\n", file_size);
    return NULL;
  }

  if ((*num_read = fread(buffp, 1, file_size, fp)) <= 0) {
    FAST3ErrorTraceHandle(1, "File2PagedBuffer() fread failed (tried to read %d bytes, readed %d) !\n", file_size, num_read);
    return NULL;
 } else {
    buffp[*num_read] = 0;
    FAST3ErrorTraceHandle(3, "File2PagedBuffer() Copyied from file %d bytes in memory 0x%x!\n", *num_read, buffp);
  }

  {
    int		LineCount = 0;
    int		CharCount = 0;
    *PageCount	= 0;
    *TotalLines	= 0;

    Page[*PageCount*maxl+LineCount] = (int)(&buffp[CharCount] - &buffp[0]);

    FAST2ErrorTraceHandle(3, "File2PagedBuffer() Page[0]=&buffp[0]=0x%x\n", Page[0]);

    while (buffp[CharCount] != 0)
      {
		if (buffp[CharCount] == 13)
		  {
			LineCount	= LineCount + 1;
			CharCount	= CharCount  + 2;

			*TotalLines	= *TotalLines + 1;

			if (LineCount == maxl)
			  {
				LineCount = 0;
				*PageCount = *PageCount + 1;

				Page[*PageCount*maxl+LineCount] = (int) (&buffp[CharCount] - &buffp[0]);
			  }
			else
			  {
				Page[*PageCount*maxl+LineCount] = (int) (&buffp[CharCount] - &buffp[0]);
			  }
		  }
		else
	  CharCount = CharCount +1;
      }

    FAST4ErrorTraceHandle(3, "File2PagedBuffer() num_read=%d, PageCount=%d, TotalLines=%d\n", *num_read, *PageCount, *TotalLines);
    FAST2ErrorTraceHandle(3, "File2PagedBuffer() &buffp[CharCount-1]=0x%x\n", &buffp[CharCount-1]);
  }

  fclose(fp);

  return buffp;
}

// ***********************************************************************************
//
// Copyies whole regular file content to memory which allocates also here.
//
// ***********************************************************************************

char *Buffer2PagedBuffer(
			 char *inbuffer,
		     int   inbuffer_size,
		     int  *num_read,
		     int  *Page,
		     int   maxp, int maxl,
		     int  *PageCount,
		     int  *TotalLines)
{
  FILE *fp;
  char *buffp;

  if ((buffp = (char *) malloc(inbuffer_size)) == NULL) {
    FAST2ErrorTraceHandle(0, "Buffer2PagedBuffer() Can't malloc mem for file size of %d bytes !\n", inbuffer_size);
    return NULL;
  }

  memcpy(buffp, inbuffer, inbuffer_size);
  buffp[inbuffer_size] = 0;
  *num_read = inbuffer_size;

  {
    int		LineCount = 0;
    int		CharCount = 0;
    *PageCount	= 0;
    *TotalLines	= 0;

    Page[*PageCount*maxl+LineCount] = (int)(&buffp[CharCount] - &buffp[0]);

    FAST2ErrorTraceHandle(3, "Buffer2PagedBuffer() Page[0]=&buffp[0]=0x%x\n", &buffp[0]);


    while (buffp[CharCount] != 0)
      {
		if ((buffp[CharCount] == 13 /*cr - carriage return*/) ||
			(buffp[CharCount] == 10 /*nl - new line */))
		  {
			LineCount	= LineCount + 1;
			if (buffp[CharCount] == 13)
				CharCount	= CharCount  + 2;
			else
				CharCount	= CharCount  + 1;

			*TotalLines	= *TotalLines + 1;

			if (LineCount == maxl)
			  {
				LineCount = 0;
				*PageCount = *PageCount + 1;

				Page[*PageCount*maxl+LineCount] = (int) (&buffp[CharCount] - &buffp[0]);
			  }
			else
			  {
				Page[*PageCount*maxl+LineCount] = (int) (&buffp[CharCount] - &buffp[0]);
			  }
		  }
		else
	  CharCount = CharCount +1;
      }

    FAST5ErrorTraceHandle(3, "Buffer2PagedBuffer() num_read=%d, PageCount=%d, TotalLines=%d, CharCount=%d\n", *num_read, *PageCount, *TotalLines, CharCount);
    FAST2ErrorTraceHandle(3, "Buffer2PagedBuffer() &buffp[CharCount-1]=0x%x\n", &buffp[CharCount-1]);
  }

  return buffp;
}


// ***********************************************************************************
//
// Copies whole regular file content to memory which allocates also here.
//
// ***********************************************************************************

int File2Buffer(char *directory,
		char *file,
		char *return_buffer)
{
  static char filename[80];
  int num_read, file_size;
  FILE *fp;

  if (directory != NULL)
    sprintf(filename, "%s/%s", directory, file);
  else
    strcpy(filename, file);

  if ((fp = fopen(filename, "r")) == NULL) {
    FAST2ErrorTraceHandle(0, "main() Can't open file <%s> !\n", filename);
  }
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if ((return_buffer = (char *) malloc(file_size)) == NULL) {
    FAST2ErrorTraceHandle(0, "main() Can't malloc mem for file size of %d bytes !\n", file_size);
  }

  if ((num_read = fread(return_buffer, 1, file_size, fp)) <= 0) {
    FAST3ErrorTraceHandle(2, "File2Buffer() fread failed (tried to read %d bytes, readed %d) !\n", file_size, num_read);
    num_read = -1;
  } else
    return_buffer[num_read] = 0;

  fclose(fp);

  return num_read;
}



// ***********************************************************************************
//
// Copies file content to already allocated memory.
//
// ***********************************************************************************
int PrimitiveFile2Buffer(char *directory,
						 char *file,
						 char *return_buffer,
						 int   buffer_size)
{
  static char filename[80];
  int fd;
  ssize_t num_read = 0;

  if (directory != NULL)
    sprintf(filename, "%s/%s", directory, file);
  else
    strcpy(filename, file);

  if ((fd = open(filename, O_RDONLY, 0)) == -1) {
    FAST2ErrorTraceHandle(1, "PrimitiveFile2Buffer() open failed for <%s>!\n", filename);
    return -1;
  }

  if ((num_read=read(fd, return_buffer, buffer_size - 1)) <= 0) {
    FAST2ErrorTraceHandle(1, "PrimitiveFile2Buffer() read failed (tried to read %d bytes) !\n", buffer_size);
    num_read = -1;
  } else {
    return_buffer[(int)num_read] = 0;
    FAST3ErrorTraceHandle(3, "PrimitiveFile2Buffer() size=%d (max=%d)!\n", num_read, buffer_size);
  }

  close(fd);

  return (int) num_read;
}


// ***********************************************************************************
//
// Finds the current time (in miliseconds) and calculates the time
// elapsed since the last update. This is essential for computing
// percent CPU usage.
//
// ***********************************************************************************
float GetElapsedTimeSinceLastCallUS(void) {
	struct timeval t;
	static struct timeval oldtime;
	struct timezone timez;
	float elapsed_time;

	gettimeofday(&t, &timez);
	elapsed_time = (t.tv_sec - oldtime.tv_sec) + (float) (t.tv_usec
			- oldtime.tv_usec) / 1000000.0;
	oldtime.tv_sec = t.tv_sec;
	oldtime.tv_usec = t.tv_usec;
	return (elapsed_time);
}

short int    littletobig2(short int little    /* short int    = 2 bytes */) {
	return (((little & 0x00ff) << 8) |
		    ((little & 0xff00) >> 8));
}

unsigned int littletobig4(unsigned int little /* unsigned int = 4 bytes */) {
	return (((little & 0x000000ff) << 24) |
		    ((little & 0x0000ff00) << 8)  |
		    ((little & 0x00ff0000) >> 8)  |
		    ((little & 0xff000000) >> 24));
}

/*
double       littletobig8(double little ... prijavljuje pogresku na binarne operacije nad double ...) {
	return (((little & 0x00000000000000ff) << 56)  |
		    ((little & 0x000000000000ff00) << 40)  |
		    ((little & 0x0000000000ff0000) << 24)  |
		    ((little & 0x00000000ff000000) << 8)   |
		    ((little & 0x000000ff00000000) >> 8)   |
		    ((little & 0x0000ff0000000000) >> 24)  |
		    ((little & 0x00ff000000000000) >> 40)  |
		    ((little & 0xff00000000000000) >> 56));
}
*/


