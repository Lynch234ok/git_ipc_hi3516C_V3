
/* defines for, or consts and inline functions for C++ */

/* global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>	/* Only needed for the test function */

/* local includes */
#include "inifile.h"

/* Global Variables */

//struct ENTRY *Entry = NULL;
//struct ENTRY *CurEntry = NULL;
//char Result[255] =
//{""};
//FILE *IniFile;

//#define Entry thiz->entry
//#define CurEntry thiz->cur_entry
//#define Result thiz->result
//#define IniFile thiz->inifile

/* Private functions declarations */
void AddpKey (struct ENTRY * Entry, INI_CSTR_t pKey, INI_CSTR_t Value);
void FreeMem (void *Ptr);
void FreeAllMem (lpINI_PARSER thiz);
bool FindpKey (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, EFIND * List);
bool AddSectionAndpKey (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, INI_CSTR_t Value);
struct ENTRY *MakeNewEntry (lpINI_PARSER thiz);

void delblank(char *lstr)
{
  int i;
  char tmpstr[512];
  for(i=strlen(lstr)-1;i>=0;i--)
    if(*(lstr+i)==' ')
      *(lstr+i)='\0';
    else
      break;
  for(i=0;i<strlen(lstr);i++)
    if(*(lstr+i)!=' ')
      break;
  memset(tmpstr,0,512);
  strcpy(tmpstr,(char *)(lstr+i));
  memset(lstr,0,strlen(lstr));
  strcpy(lstr,tmpstr);
}


#ifdef DONT_HAVE_STRUPR
/* DONT_HAVE_STRUPR is set when INI_REMOVE_CR is defined */
void strupr( char *str )
{
    // We dont check the ptr because the original also dont do it.
	while (*str != 0)
    {
        if ( islower( *str ) )
        {
		     *str = toupper( *str );
        }
        str++;
	}
}
#endif
#if 0
static char asc2xdigit(char asc)
{
	if(asc >= '0' && asc <= '9'){
		return asc - '0';
	}else if(asc >= 'a' && asc <= 'z'){
		return asc - 'a' + 0xa;
	}else if(asc >= 'A' && asc <= 'Z'){
		return asc - 'A' + 0xa;
	}
	return 0;
}
#endif
static char xdigit2asc(char xdigi)
{
	if(xdigi >= 0 && xdigi <= 9){
		return xdigi + '0';
	}else if(xdigi >= 0x0a && xdigi <= 0x0f){
		return xdigi + 'a';
	}
	return 0;
}



/*=========================================================================
 *
 * OpenIniFile
 * -------------------------------------------------------------------------
 * Job : Opens an ini file or creates a new one if the requested file
 *         doesnt exists.
 * Att : Be sure to call CloseIniFile to free all mem allocated during
 *         operation!
 *
 *========================================================================*/
static lpINI_PARSER create_inifile();

lpINI_PARSER  
OpenIniFile (INI_CSTR_t FileName)
{
	char str[255];
	char *pstr;
	struct ENTRY *pEntry;
	int Len;
	lpINI_PARSER thiz = create_inifile();

	//printf("Open File:%s\n",FileName);

	FreeAllMem (thiz);

	if (FileName == NULL)
	{
		printf("FileName is NULL\n");
		free(thiz);
		return NULL;
	}
	if ((thiz->inifile = fopen (FileName, "r")) == NULL)
	{
		printf("Error open File:%s\n",FileName);
		free(thiz);
		return NULL;
	}

	while (fgets (str, 255, thiz->inifile) != NULL)
	{
		pstr = strchr (str, '\n');
		if (pstr != NULL)
		{
			*pstr = 0;
		}
		pEntry = MakeNewEntry (thiz);
		if (pEntry == NULL)
		{
			printf("Get Entry Fail!\n");
			free(thiz);
			return NULL;
		}

#ifdef INI_REMOVE_CR
      	Len = strlen(str);
		if ( Len > 0 )
		{
        	if ( str[Len-1] == '\r' )
        	{
          		str[Len-1] = '\0';
            }
        }
#endif
		
		delblank(str);
		
		pEntry->Text = (char *) malloc (strlen (str) + 1);
		if (pEntry->Text == NULL)
		{
			printf("Get Text Fail!\n");
			FreeAllMem (thiz);
			free(thiz);
			return NULL;
		}
		//printf("%s\n",str);
		strcpy (pEntry->Text, str);
		pstr = strchr (str, ';');
		if (pstr != NULL)
		{
			*pstr = 0;
		}			/* Cut all comments */
		if ((strstr (str, "[") > 0) && (strstr (str, "]") > 0))	/* Is Section */
		{
			pEntry->Type = tpSECTION;
		}
	    else
		{
			if (strstr (str, "=") > 0)
			{
				pEntry->Type = tpKEYVALUE;
			}
			else
			{
				pEntry->Type = tpCOMMENT;
			}
		}
	    thiz->cur_entry = pEntry;
    }
	fclose (thiz->inifile);
    thiz->inifile = NULL;
  //printf("End Open File!%d\n",true);
	return thiz;
}

/*=========================================================================
 *
 * CloseIniFile
 * -------------------------------------------------------------------------
 * Job : Frees the memory and closes the ini file without any
 *   modifications. If you want to write the file use
 *    WriteIniFile instead.
 * ========================================================================*/
void 
CloseIniFile (lpINI_PARSER thiz)
{
	FreeAllMem (thiz);
	if (thiz->inifile != NULL)
	{
		fclose (thiz->inifile);
		thiz->inifile = NULL;
	}
}

/*=========================================================================
   WriteIniFile
  -------------------------------------------------------------------------
   Job : Writes the iniFile to the disk and close it. Frees all memory
         allocated by WriteIniFile;
 *========================================================================*/
bool 
WriteIniFile (lpINI_PARSER thiz, const char *FileName)
{
	struct ENTRY *pEntry = thiz->entry;
	thiz->inifile = NULL;
	if (thiz->inifile != NULL)
	{
		fclose (thiz->inifile);
	}
	if ((thiz->inifile = fopen (FileName, "wb")) == NULL)
	{
		FreeAllMem (thiz);
		return FALSE;
	}

	while (pEntry != NULL)
	{
		if (pEntry->Type != tpNULL)
		{

//#ifdef INI_REMOVE_CR
//			fprintf (thiz->inifile, "%s\n", pEntry->Text);
//#else
			fprintf (thiz->inifile, "%s\r\n", pEntry->Text);
//#endif
		pEntry = pEntry->pNext;
        }
   	}

	fclose (thiz->inifile);
	thiz->inifile = NULL;
	return TRUE;
}


/*=========================================================================
   Writestring : Writes a string to the ini file
*========================================================================*/
void 
write_text (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, INI_CSTR_t Value)
{
	EFIND List;
	char Str[255];

	if (ArePtrValid (Section, pKey, Value) == FALSE)
	{
		return;
	}
	if (FindpKey (thiz, Section, pKey, &List) == TRUE)
	{
		sprintf (Str, "%s=%s%s", List.KeyText, Value, List.Comment);
		FreeMem (List.pKey->Text);
		List.pKey->Text = (char *) malloc (strlen (Str) + 1);
		strcpy (List.pKey->Text, Str);
	}
	else
	{
		if ((List.pSec != NULL) && (List.pKey == NULL))	/* section exist, pKey not */
		{
			AddpKey (List.pSec, pKey, Value);
		}
		else
		{
			AddSectionAndpKey (thiz, Section, pKey, Value);
		}
	}
}

//=========================================================================
//   write_text : Writes a binary to the ini file
//=========================================================================
void
write_binary(lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, INI_CBIN_t bin, size_t bin_len)
{
	int i = 0;
	size_t text_len = 2 * bin_len + 1;
	char *text = alloca(text_len);
	char *ptr_text = text;
	char *ptr_bin = (char*)bin;

	for(i = 0; i < bin_len; ++i){
		char const asc_msb = xdigit2asc(ptr_bin[i] >> 4);
		char const asc_lsb = xdigit2asc(ptr_bin[i] & 0xf);
		*ptr_text++ = asc_msb;
		*ptr_text++ = asc_lsb;
	}
	*ptr_text = '\0';
	write_text(thiz, section, key, text);
}

/*=========================================================================
   write_bool : Writes a boolean to the ini file
*========================================================================*/
void 
write_bool (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, bool Value)
{
	char Val[2] = {'0', 0};
	if (Value != 0)
	{
		Val[0] = '1';
	}
	write_text (thiz, Section, pKey, Val);
}

/*=========================================================================
   write_int : Writes an integer to the ini file
*========================================================================*/
void 
write_int (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, int Value)
{
	char Val[12];			/* 32bit maximum + sign + \0 */
    sprintf (Val, "%d", Value);
    write_text (thiz, Section, pKey, Val);
}

/*=========================================================================
   write_double : Writes a double to the ini file
*========================================================================*/
void 
write_double (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, double Value)
{
	char Val[32];			/* DDDDDDDDDDDDDDD+E308\0 */
	sprintf (Val, "%1.10lE", Value);
	write_text (thiz, Section, pKey, Val);
}


//=========================================================================
//   read_text : Reads a string from the ini file
//=========================================================================
INI_CSTR_t
read_text (lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, INI_CSTR_t def_val, INI_STR_t buf, size_t stack_len)
{
	EFIND List;
	if(!ArePtrValid (section, key, def_val)){
		printf("ArePtrValid Fail!\n");
		return def_val;
	}
	if(FindpKey(thiz, section, key, &List)){
		if(NULL != buf){
			bzero(buf, stack_len);		/* TODO: Empty buffer */
			snprintf(buf, stack_len, "%s", List.ValText);
			return buf;
		}
    }
	return def_val;
}

//=========================================================================
//   read_binary : Reads a binary data from the ini file
//=========================================================================
ssize_t
read_binary(lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, INI_BIN_t buf, size_t stack_len)
{
	int i = 0;
	size_t const text_len = stack_len * 2 + 1;
	char *text_buf = alloca(text_len);
	char *ptr_buf = (char*)buf;
	ssize_t bin_len = -1;
	if(text_buf == read_text(thiz, section, key, NULL, text_buf, text_len)){
		for(i = 0; i < strlen(text_buf) - 1; ++i){
			char asc_msb = text_buf[i * 2];
			char asc_lsb = text_buf[i * 2 + 1];
			if(isxdigit(asc_msb) && isxdigit(asc_lsb)){
				unsigned char const byte1 = (asc_msb << 4) + asc_lsb;
				ptr_buf[bin_len++] = byte1;
			}
			
		}
	}
	return bin_len;
}


//=========================================================================
//   read_bool : Reads a boolean from the ini file
//=========================================================================
bool 
read_bool (lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, bool def_val)
{
	char buf[32] = {""};
	INI_CSTR_t cstr = read_text (thiz, section, key, def_val ? "yes" : "no", buf, sizeof(buf));
	if(0 == strcmp("yes", cstr) || 0 == strcmp("1", cstr)){
		// Only 1 or yes allowed for true
		return true;
	}
	return false;
}

//=========================================================================
//   read_int : Reads a integer from the ini file
//=========================================================================
int 
read_int (lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, int def_val)
{
	char val[32] = {""};
	char buf[32] = {""};
	sprintf(val, "%d", def_val);
	return (atoi(read_text (thiz, section, key, val, buf, sizeof(buf))));
}

//=========================================================================
//   read_double : Reads a double from the ini file
//=========================================================================
double 
read_float(lpINI_PARSER thiz, INI_CSTR_t section, INI_CSTR_t key, double def_val)
{
	char val[64] = {""};
	char buf[64] = {""};
	double ret;
	sprintf (val, "%1.10lE", def_val);
	sscanf(read_text (thiz, section, key, val, buf, sizeof(buf)), "%lE", &ret);
	return ret;
}



/*=========================================================================
   delete_key : Deletes a pKey from the ini file.
*========================================================================*/

bool delete_key (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey)
{
    EFIND         List;
    struct ENTRY *pPrev;
    struct ENTRY *pNext;

    if (FindpKey (thiz, Section, pKey, &List) == TRUE)
    {
        pPrev = List.pKey->pPrev;
        pNext = List.pKey->pNext;
        if (pPrev)
        {
            pPrev->pNext=pNext;
        }
        if (pNext)
        {
            pNext->pPrev=pPrev;
        }
        FreeMem (List.pKey->Text);
        FreeMem (List.pKey);
        return TRUE;
    }
    return FALSE;
}



/* Here we start with our helper functions */
/*=========================================================================
   FreeMem : Frees a pointer. It is set to NULL by Free AllMem
*========================================================================*/
void 
FreeMem (void *Ptr)
{
	if (Ptr != NULL)
	{
		free (Ptr);
	}
}

/*=========================================================================
   FreeAllMem : Frees all allocated memory and set the pointer to NULL.
             	Thats IMO one of the most important issues relating
             	to pointers :

             	A pointer is valid or NULL.
*========================================================================*/
void 
FreeAllMem (lpINI_PARSER thiz)
{
	struct ENTRY *pEntry;
	struct ENTRY *pNextEntry;
	pEntry = thiz->entry;
	while (1)
	{
		if (pEntry == NULL)
		{
			break;
		}
		pNextEntry = pEntry->pNext;
		FreeMem (pEntry->Text);	/* Frees the pointer if not NULL */
		FreeMem (pEntry);
		pEntry = pNextEntry;
	}
	thiz->entry = NULL;
	thiz->cur_entry = NULL;
}

/*=========================================================================
   FindSection : Searches the chained list for a section. The section
                 must be given without the brackets!
   Return Value: NULL at an error or a pointer to the ENTRY structure
                 if succeed.
*========================================================================*/
struct ENTRY *
FindSection (lpINI_PARSER thiz, INI_CSTR_t Section)
{
	char Sec[130];
	char iSec[130];
	struct ENTRY *pEntry;
	if(Section != NULL){
		sprintf (Sec, "[%s]", Section);
	}
	strupr (Sec);//小写转大写
	pEntry = thiz->entry;		/* Get a pointer to the first Entry */
	while (pEntry != NULL)
    {
		if (pEntry->Type == tpSECTION)
		{
			strcpy (iSec, pEntry->Text);
			strupr (iSec);
			if (strcmp (Sec, iSec) == 0)
			{
				return pEntry;
			}
		}
		pEntry = pEntry->pNext;
    }
	return NULL;
}

/*=========================================================================
   FindpKey     : Searches the chained list for a pKey under a given section
   Return Value: NULL at an error or a pointer to the ENTRY structure
                 if succeed.
*========================================================================*/
bool 
FindpKey (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, EFIND * List)
{
	char Search[130];
	char Found[130];
	char Text[255];
	char *pText;
	struct ENTRY *pEntry;
	List->pSec = NULL;
	List->pKey = NULL;
	pEntry = FindSection (thiz, Section);
	if (pEntry == NULL)
	{
		return FALSE;
    }
	List->pSec = pEntry;
	List->KeyText[0] = 0;
	List->ValText[0] = 0;
	List->Comment[0] = 0;
	pEntry = pEntry->pNext;
	if (pEntry == NULL)
	{
		return FALSE;
    }
	if(NULL != pKey){
  		sprintf (Search, "%s", pKey);
	}
  	//strupr (Search);
  	while (pEntry != NULL)
    {
      	if ((pEntry->Type == tpSECTION) ||	/* Stop after next section or EOF */
	  	(pEntry->Type == tpNULL))
		{
	  		return FALSE;
		}
      	if (pEntry->Type == tpKEYVALUE)
		{
			strcpy (Text, pEntry->Text);
			pText = strchr (Text, ';');
			if (pText != NULL)
			{
				strcpy (List->Comment, Text);
				*pText = 0;
			}
			pText = strchr (Text, '=');
	  		if (pText != NULL)
			{
				*pText = 0;
				strcpy (List->KeyText, Text);
				strcpy (Found, Text);
				*pText = '=';
				//strupr (Found);
				delblank(Found);
				/* printf ("%s,%s\n", Search, Found); */
				if (strcmp (Found, Search) == 0)
				{
					strcpy (List->ValText, pText + 1);
					delblank(List->ValText);
					List->pKey = pEntry;
					return TRUE;
				}
			}
		}
      	pEntry = pEntry->pNext;
    }
  	return FALSE;
}

/*=========================================================================
   AddItem  : Adds an item (pKey or section) to the chaines list
*========================================================================*/
bool 
AddItem (lpINI_PARSER thiz, char Type, INI_CSTR_t Text)
{
	struct ENTRY *pEntry = MakeNewEntry (thiz);
	if (pEntry == NULL)
	{
		return FALSE;
    }
	pEntry->Type = Type;
	pEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pEntry->Text == NULL)
    {
		free (pEntry);
		return FALSE;
    }
	strcpy (pEntry->Text, Text);
	pEntry->pNext = NULL;
	if (thiz->cur_entry != NULL)
    {
		thiz->cur_entry->pNext = pEntry;
    }
	thiz->cur_entry = pEntry;
	return TRUE;
}

/*=========================================================================
   AddItemAt : Adds an item at a selected position. This means, that the
               chained list will be broken at the selected position and
               that the new item will be Inserted.
               Before : A.Next = &B
               After  : A.Next = &NewItem, NewItem.Next = &B
*========================================================================*/
bool 
AddItemAt (struct ENTRY * EntryAt, char Mode, INI_CSTR_t Text)
{
	struct ENTRY *pNewEntry;
	if (EntryAt == NULL)
    {
		return FALSE;
    }
	pNewEntry = (struct ENTRY *) malloc (sizeof (ENTRY));
	if (pNewEntry == NULL)
    {
		return FALSE;
    }
	pNewEntry->Text = (char *) malloc (strlen (Text) + 1);
	if (pNewEntry->Text == NULL)
    {
		free (pNewEntry);
		return FALSE;
    }
	strcpy (pNewEntry->Text, Text);
	if (EntryAt->pNext == NULL)	/* No following nodes. */
    {
		EntryAt->pNext = pNewEntry;
		pNewEntry->pNext = NULL;
    }
  else
    {
		pNewEntry->pNext = EntryAt->pNext;
		EntryAt->pNext = pNewEntry;
    }
	pNewEntry->pPrev = EntryAt;
	pNewEntry->Type = Mode;
	return TRUE;
}

/*=========================================================================
   AddSectionAndpKey  : Adds a section and then a pKey to the chained list
*========================================================================*/
bool 
AddSectionAndpKey (lpINI_PARSER thiz, INI_CSTR_t Section, INI_CSTR_t pKey, INI_CSTR_t Value)
{
	char Text[255];
	if(NULL != Section){
		sprintf (Text, "[%s]", Section);
	}
	if (AddItem (thiz, tpSECTION, Text) == FALSE)
    {
		return FALSE;
    }
	sprintf (Text, "%s=%s", pKey, Value);
	return AddItem (thiz, tpKEYVALUE, Text);
}

/*=========================================================================
   AddpKey  : Adds a pKey to the chained list
*========================================================================*/
void 
AddpKey (struct ENTRY *SecEntry, INI_CSTR_t pKey, INI_CSTR_t Value)
{
	char Text[255];
	sprintf (Text, "%s=%s", pKey, Value);
	AddItemAt (SecEntry, tpKEYVALUE, Text);
}

/*=========================================================================
   MakeNewEntry  : Allocates the memory for a new entry. This is only
                   the new empty structure, that must be filled from
                   function like AddItem etc.
   Info          : This is only a internal function. You dont have to call
                   it from outside.
*==========================================================================*/
struct ENTRY *
MakeNewEntry (lpINI_PARSER thiz)
{
	struct ENTRY *pEntry;
	pEntry = (struct ENTRY *) malloc (sizeof (ENTRY));
	if (pEntry == NULL)
    {
		FreeAllMem (thiz);
		return NULL;
    }
	if (thiz->entry == NULL)
    {
		thiz->entry = pEntry;
    }
	pEntry->Type = tpNULL;
	pEntry->pPrev = thiz->cur_entry;
	pEntry->pNext = NULL;
	pEntry->Text = NULL;
	if (thiz->cur_entry != NULL)
    {
		thiz->cur_entry->pNext = pEntry;
    }
	return pEntry;
}

static lpINI_PARSER create_inifile()
{
	lpINI_PARSER inifile = calloc(sizeof(stINI_PARSER), 1);
	
	inifile->write_text = write_text;
	inifile->write_binary = write_binary;
	inifile->write_bool = write_bool;
	inifile->write_int = write_int;
	inifile->write_double = write_double;

	inifile->read_text = read_text;
	inifile->read_binary = read_binary;
	inifile->read_bool = read_bool;
	inifile->read_int = read_int;
	inifile->read_float = read_float;

	inifile->delete_key = delete_key;

	inifile->find_section = FindSection;

	return inifile;
}

/*#define INIFILE_TEST_THIS_FILE*/
#ifdef INIFILE_TEST_THIS_FILE
#define INIFILE_TEST_READ_AND_WRITE
int main (void)
{
	printf ("Hello World\n");
	OpenIniFile ("Test.Ini");
#ifdef INIFILE_TEST_READ_AND_WRITE
	write_text  ("Test", "Name", "Value");
	write_text  ("Test", "Name", "OverWrittenValue");
	write_text  ("Test", "Port", "COM1");
	write_text  ("Test", "User", "James Brown jr.");
	write_text  ("Configuration", "eDriver", "MBM2.VXD");
	write_text  ("Configuration", "Wrap", "LPT.VXD");
	write_int 	 ("IO-Port", "Com", 2);
	write_bool 	 ("IO-Port", "IsValid", 0);
	write_double  ("TheMoney", "TheMoney", 67892.00241);
	write_int     ("Test"    , "ToDelete", 1234);
	WriteIniFile ("Test.Ini");
	printf ("Key ToDelete created. Check ini file. Any key to continue");
	while (!kbhit());
	OpenIniFile  ("Test.ini");
	delete_key    ("Test"	  , "ToDelete");
	WriteIniFile ("Test.ini");
#endif
	printf ("[Test] Name = %s\n", read_text ("Test", "Name", "NotFound"));
	printf ("[Test] Port = %s\n", read_text ("Test", "Port", "NotFound"));
	printf ("[Test] User = %s\n", read_text ("Test", "User", "NotFound"));
	printf ("[Configuration] eDriver = %s\n", read_text ("Configuration", "eDriver", "NotFound"));
	printf ("[Configuration] Wrap = %s\n", read_text ("Configuration", "Wrap", "NotFound"));
	printf ("[IO-Port] Com = %d\n", read_int ("IO-Port", "Com", 0));
	printf ("[IO-Port] IsValid = %d\n", read_bool ("IO-Port", "IsValid", 0));
	printf ("[TheMoney] TheMoney = %1.10lf\n", read_double ("TheMoney", "TheMoney", 111));
	CloseIniFile ();
	return 0;
}
#endif
