//#define WINDOWS
#define UNIX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#ifdef WINDOWS
	#include <io.h>
	#define R_OK    4       /* Test for read permission. Value for 2nd argument of access(). */
	#define W_OK    2       /* Test for write permission. Value for 2nd argument of access().  */
	//#define X_OK    1     /* Test for execute permission. Value for 2nd argument of access(). Windows does not support it.  */
	#define F_OK    0       /* Test for existence. Value for 2nd argument of access(). */
#endif
#ifdef UNIX
	#include <unistd.h>
#endif
#include <sys/timeb.h>
#define MT_NO_INLINE
#define MT_NO_LONGLONG
#include "UtilsOfTensor.h"
#include "sort.h"

/*--------------------
 Routines
--------------------*/
/*---------------------------------
		Misc
-----------------------------------*/
unsigned int _min(unsigned int a, unsigned int b)
{ return (a>=b)?b:a; }
unsigned int _max(unsigned int a, unsigned int b)
{ return (a>=b)?a:b; }
double _min_double(double a, double b)
{ return (a>=b)?b:a; }
double _max_double(double a, double b)
{ return (a>=b)?a:b; }
unsigned int IsNumber_DOUBLE(double x) // This looks like it should always be true,  but it's false if x is a NaN.
{ return (x == x); }
unsigned int IsNumber_FLOAT(float x) // This looks like it should always be true,  but it's false if x is a NaN.
{ return (x == x); }
unsigned int IsFiniteNumber_DOUBLE(double x)
{ return (unsigned int)(x <= DBL_MAX && x >= -DBL_MAX);}
unsigned int IsFiniteNumber_FLOAT(float x)
{ return (unsigned int)(x <= FLT_MAX && x >= -FLT_MAX);}
void exchangeTwoElements_DOUBLE( double v[] )
{
	double tmp;
	tmp=v[1]; v[1]=v[0]; v[0]=tmp;
}
double power_uintExponent(double base, unsigned int exponent)
{
	double Pwr=1;
	div_t div_result;
	while (1) {
		div_result = div( exponent, 2 );
		if (div_result.rem!=0) Pwr *= base;
		exponent = div_result.quot;
		if (exponent==0) return Pwr;
		base *= base;
	}
}
double elapseTime(struct timeb t_start, struct timeb t_end)
{
	double t_elapse;
	t_elapse = (t_end.time-t_start.time) + (double)(t_end.millitm-t_start.millitm)/1000;
	return t_elapse;
}
void errorfile(char *fn, char *operation)
{
	char msg[MAXCHAR];
	sprintf(msg,"Error : Cannot %s file %s !\n",operation,fn);
	fprintf(stderr,msg);
	exit(-1);
}
void erroralloc(char *type, int count)
{
	char msg[MAXCHAR];
	sprintf(msg,"Error : Cannot allocate %d spaces for type %s !\n",count,type);
	fprintf(stderr,msg);
	exit(-1);
}
void fullfile( char* path, char* file )
{
	char tmp[MAXCHAR];
	strcpy( tmp, file );
	strcpy( file, path );
	strcat( file, "/" );
	strcat( file, tmp );
}
BOOL file_exists(const char* filename)
{
	if (access(filename, F_OK) == 0) return TRUE;
	else return FALSE;
}

void append_string_2file( char* file, char* string )
{
	FILE* stream;
	if (string!=NULL) {
		if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
		fprintf( stream, "%s", string );
		fclose( stream );
	}
}
void remove_newlinechars( char* string )
{
	char D=0xD, A=0xA;
	char *p;
	/* DOS, Windows   0D 0A   (\r\n)
	   UNIX           0A      (\n)
	   Machintosh     0D      (\r)*/
	for ( p=string; *p!='\0'; p++ ) {
		if (*p==D) *p='\0';
		if (*p==A) *p='\0';
	}
}
char *trim_whitespace(char *str)
{
	char *end, SPACE=' ';
	// Trim leading space
	while((*str)==SPACE) str++;
	if(*str == '\0') return str; // All spaces?
	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && (*end)==SPACE) end--;
	// Write new null terminator
	*(end+1) = '\0';
	return str;
}
/* Same to traditional C function 'strtok'. The difference is 'strtok' skips
   many successive 'charDelimit', but 'strtok_w' does not skip. Its usage
   is the same as 'strtok' */
char* strtok_w( char *strToken, const char charDelimit )
{
	static char strbackup[MAX_LINE_LENGTH];
	static char* nextbeginPos;
	char *beginPos, *Pos;
	if ( strToken != NULL ) {
		/* Initialize static variables */
		strcpy( strbackup, strToken );
		nextbeginPos = strbackup;
	}
	if ( *nextbeginPos=='\0' ) return NULL;
	beginPos = nextbeginPos;
	for ( Pos=beginPos; *Pos!='\0'; Pos++ )
		if ( *Pos==charDelimit ) { *Pos='\0'; nextbeginPos=Pos+1; return beginPos; }
	/* There is no charDelimit in 'strToken' from 'beginPos' to the end of string */
	nextbeginPos = Pos; return beginPos;
}
/* extract the sub-string which starts from the beginning to the first occurrence of "c" in "str".
   It will change "str". If successful, return the length of this substring; otherwise, return -1. */
int strchr_extract( char *str, char c )
{
	int result=-1;
	char *p;
	p = strchr( str,c );
	if (p!=NULL) { *p='\0'; result=p-str; }
	else result=-1;
	return result;
}
/* count how many occurrences of a char in a string */
int strcnt( char *str, char key )
{
	int n=0;
	char *p;
	for ( p=str; *p!='\0'; p++ )
		if ( *p==key ) n++;
	return n;
}

/*---------------------------------
    Routines for LIST_STRING
-----------------------------------*/
void init_LIST_STRING( unsigned int maxchar, LIST_STRING* l )
{
	l->n = 0;
	l->MAX_CHAR = maxchar;
	l->string = NULL;
}
void create_LIST_STRING(unsigned int n, unsigned int maxchar, LIST_STRING* l)
{
	unsigned int i;

	l->n = n;
	l->MAX_CHAR = maxchar;
	if (n==0) { l->string=NULL; return; }
	l->string = (char**)malloc( n*sizeof(char*) );
	if ( l->string == NULL ) erroralloc("char*",n);
	for ( i=0; i<n; i++) {
		l->string[i] = (char*)malloc( maxchar*sizeof(char) );
		if ( l->string[i] == NULL ) erroralloc("char",maxchar);
	}
	for ( i=0; i<n; i++)
		l->string[i][0] = '\0';
}
void free_LIST_STRING(LIST_STRING* l)
{
	unsigned int i;
	if (l->string!=NULL) {
		for ( i=0; i<l->n; i++ )
				free( l->string[i] );
		free( l->string );
	}
}
unsigned int addstring_LIST_STRING( char* key, LIST_STRING* l )
{
	unsigned int n;
	char **tmp;
	n = l->n + 1;
	tmp = (char**)realloc( l->string, n*sizeof(char*) );
	if ( tmp == NULL ) erroralloc("char*", n);
	l->string = tmp;
	l->string[n-1] = (char*)malloc( l->MAX_CHAR*sizeof(char) );
	if ( l->string[n-1] == NULL ) erroralloc("char",l->MAX_CHAR);
	strcpy( l->string[n-1], key );
	l->n = n;
	return (n-1); /* return index of the 'key' in 'l' */
}
void copy_LIST_STRING(LIST_STRING *dst, LIST_STRING src)
{
	unsigned int i;
	free_LIST_STRING( dst );
	dst->MAX_CHAR = src.MAX_CHAR;
	dst->n = 0;
	dst->string = NULL;
	for (i=0; i<src.n; i++)
		addstring_LIST_STRING( src.string[i], dst );
}
int lookup_LIST_STRING( char* key, LIST_STRING l)
{
	unsigned int i;
	int idx=-1;
	if (l.string!=NULL) {
		for (i=0; i<l.n; i++)
			if (strcmp(key, l.string[i])==0) { idx = i; break; }
	}
	return idx;
}
/* We suppose that the numbers in this string are seperated by spaces and tables characters. */
void read_string2LIST_STRING_sep( char* string, LIST_STRING* l, char* seps, unsigned int maxchar )
{
	char* token;
	init_LIST_STRING( maxchar, l );
	token = strtok( string, seps );
	while( token != NULL ) {
		addstring_LIST_STRING( token, l );
		token = strtok( NULL, seps );
	}
}
/* each line is a string in the file */
void read_LIST_STRING( LIST_STRING* l, char* file, unsigned int maxchar )
{
	FILE* stream;
	int flag;
	char string[MAXCHAR];
	init_LIST_STRING( maxchar, l );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		flag = fscanf( stream, "%s", string );
		if (flag>0) addstring_LIST_STRING(string, l);
	}
	fclose(stream);
}
/* each line has two strings (or columns) in the file. Two strings of each line are
   seperated by delimiters "seps". If each line has only one string (no "seps" found),
   then the second LIST_STRING is assigned empty. */
void read_2COLUMNS_2LIST_STRING( LIST_STRING* l1, LIST_STRING* l2, char* file, char* seps, unsigned int maxchar1, unsigned int maxchar2 )
{
	FILE* stream;
	char line[MAX_LINE_LENGTH], *token;
	init_LIST_STRING( maxchar1, l1 );
	init_LIST_STRING( maxchar2, l2 );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			remove_newlinechars( line );
			if (strlen(line)==0) continue; // if this line is empty, then skip it
			if ((token=strtok(line,seps)) != NULL) addstring_LIST_STRING(token, l1);
			else { fprintf(stderr,"\nread_2COLUMNS_2LIST_STRING ERROR: no tokens are found in the file!\n"); exit(-1); }
			if ((token=strtok(NULL,seps)) != NULL) addstring_LIST_STRING(token, l2);
			else addstring_LIST_STRING("", l2);
		}
	}
	fclose(stream);
}
/* two lines in the file. each line is a LIST_STRING, seperated by "seps" */
void read_2LINES_2LIST_STRING( LIST_STRING* l1, LIST_STRING* l2, char* file, char* seps, unsigned int maxchar )
{
	FILE* stream;
	char line[MAX_LINE_LENGTH];
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
		remove_newlinechars( line );
		read_string2LIST_STRING_sep( line, l1, seps, maxchar );
	} else {
		fprintf( stderr, "\t""%s"" should have two lines of concepts\n", file );
		exit( -1 );
	}
	if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
		remove_newlinechars( line );
		read_string2LIST_STRING_sep( line, l2, seps, maxchar );
	} else {
		fprintf( stderr, "\t""%s"" should have two lines of concepts\n", file );
		exit( -1 );
	}
	fclose(stream);
}
void write_LIST_STRING( LIST_STRING l, char* file )
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (l.string !=NULL) {
		for (i=0; i<l.n; i++)
			fprintf( stream, "%s\n", l.string[i] );
	}
	fclose(stream);
}
/* each list of strings is a line; strings are separate by the string "$sep" */
void write_2LIST_STRING_2LINE( LIST_STRING l1,  LIST_STRING l2, char *seps, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (l1.string!=NULL && l2.string!=NULL) {
		for (i=0; i<l1.n-1; i++)
			fprintf( stream, "%s%s", l1.string[i], seps );
		fprintf( stream, "%s\n", l1.string[i] );
		for (i=0; i<l2.n-1; i++)
			fprintf( stream, "%s%s", l2.string[i], seps );
		fprintf( stream, "%s\n", l2.string[i] );
	}
	fclose(stream);
}
/* get the indexes of selected strings 'selectedStrings' in all strings 'allStrings'.
   return the number of continuous strings which have indexes (or found in allStrings). */
unsigned int getIndex_Of_LIST_STRING( LIST_STRING allStrings, LIST_STRING selectedStrings, VEC_UINT* selectedIndexes )
{
	unsigned int i;
	int index;
	init_VEC_UINT( selectedIndexes );
	for ( i=0; i<selectedStrings.n; i++ ) {
		index = lookup_LIST_STRING( selectedStrings.string[i], allStrings );
		if ( index != -1 ) addnumber_VEC_UINT( index, selectedIndexes );
		else return i+1;
	}
	return selectedIndexes->n;
}

/*---------------------------------
    Routines for VEC_UINT
-----------------------------------*/
void init_VEC_UINT(VEC_UINT* vec)
{
	vec->n = 0;
	vec->v = NULL;
}
void create_VEC_UINT(unsigned int n, VEC_UINT* vec)
{
	unsigned int i;
	vec->n = n;
	vec->v = (unsigned int*)malloc( n*sizeof(unsigned int) );
	if ( vec->v == NULL ) erroralloc("unsigned int",n);
	for ( i=0; i<n; i++ ) vec->v[i] = 0;
}
void createINDEX_VEC_UINT(unsigned int n, VEC_UINT* vec)
{
	unsigned int i;
	vec->n = n;
	vec->v = (unsigned int*)malloc( n*sizeof(unsigned int) );
	if ( vec->v == NULL ) erroralloc("unsigned int",n);
	for ( i=0; i<n; i++ ) vec->v[i] = i;
}
void initINDEX_VEC_UINT(unsigned int n, VEC_UINT* vec)
{
	unsigned int i;
	vec->n = n;
	for ( i=0; i<n; i++ ) vec->v[i] = i;
}
void free_VEC_UINT(VEC_UINT* vec)
{
	if (vec->v!=NULL) free( vec->v );
}
/* suppose dst is allocated space, before calling */
void copy_VEC_UINT(VEC_UINT *dst, VEC_UINT src)
{
	unsigned int i;
	if (src.n != dst->n) {
		fprintf( stderr, "Dimension of two vectors do not agree, copy operation can not be performed!\n" );
		exit( -1 );
	}
	if (src.v==NULL) {
		free_VEC_UINT( dst );
		dst->v = NULL;
		return;
	}
	for ( i=0; i<src.n; i++ ) dst->v[i] = src.v[i];
}
void zeros_VEC_UINT(VEC_UINT* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++) vec->v[i] = 0;
}
unsigned int max_VEC_UINT(VEC_UINT vec)
{
	unsigned int i, max=0;
	if (vec.v==NULL) return max;
	max = vec.v[0];
	for (i=0; i<vec.n; i++)
		if ( vec.v[i] > max ) max = vec.v[i];
	return max;
}
unsigned int min_VEC_UINT(VEC_UINT vec)
{
	unsigned int i, min=UINT_MAX;
	if (vec.v==NULL) return min;
	min = vec.v[0];
	for (i=0; i<vec.n; i++)
		if ( vec.v[i] > min ) min = vec.v[i];
	return min;
}
int lookup_VEC_UINT( unsigned int key, VEC_UINT vec)
{
	unsigned int i;
	int idx=-1;
	if (vec.v!=NULL) {
		for (i=0; i<vec.n; i++)
			if (key==vec.v[i]) { idx = i; break; }
	}
	return idx;
}
void addnumber_VEC_UINT( unsigned int key, VEC_UINT* vec )
{
	unsigned int n, *tmp;
	n = vec->n + 1;
	tmp = (unsigned int*)realloc( vec->v, n*sizeof(unsigned int) );
	if ( tmp == NULL ) erroralloc("unsigned int", n);
	vec->v = tmp;
	vec->v[n-1] = key;
	vec->n = n;
}
/* setdiff(n1,v2) = setdiff([0:(n1-1)], v2) */
void setdiff_VEC_UINT(unsigned int n1, VEC_UINT v2, VEC_UINT* v12)
{
	unsigned int intersect12, i, j;
	VEC_UINT binary;
	init_VEC_UINT( v12 );
	if (n1<=v2.n) return;
	create_VEC_UINT(n1, &binary);
	for (i=0,intersect12=0; i<v2.n; i++)
		if (v2.v[i]<n1) { binary.v[v2.v[i]]=1; intersect12++; }
	if (intersect12==n1) return;
	create_VEC_UINT(n1-intersect12, v12);
	for (i=0, j=0; i<binary.n; i++)
		if (binary.v[i]==0) { v12->v[j]=i; j++; }
}
void union_VEC_UINT(VEC_UINT* dst, VEC_UINT src)
{
	unsigned int i;
	if (src.v==NULL) return;
	if (dst==NULL) {
		dst = (VEC_UINT*)malloc( sizeof(VEC_UINT) );
		create_VEC_UINT( src.n, dst );
		copy_VEC_UINT( dst, src );
		return;
	}
	if (dst->v==NULL) {
		create_VEC_UINT( src.n, dst );
		copy_VEC_UINT( dst, src );
		return;
	}
	for (i=0; i<src.n; i++)
		if (lookup_VEC_UINT(src.v[i],*dst)==-1)
			addnumber_VEC_UINT( src.v[i], dst );
}
void read_VEC_UINT(VEC_UINT* vec, char* file)
{
	FILE* stream;
	unsigned int v;
	int flag;
	init_VEC_UINT( vec );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		flag = fscanf( stream, "%u", &v );
		if (flag>0) addnumber_VEC_UINT(v, vec);
	}
	fclose(stream);
}
void write_VEC_UINT(VEC_UINT vec, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (vec.v != NULL) {
		for (i=0; i<vec.n; i++)
			fprintf( stream, "%u\n", vec.v[i] );
	}
	fclose(stream);
}
void print_VEC_UINT(FILE* stream, VEC_UINT vec)
{
	unsigned int i;
	fprintf( stream, "[" );
	if (vec.v != NULL && vec.n>0) {
		for (i=0; i<vec.n-1; i++)
			fprintf( stream, "%u, ", vec.v[i] );
		fprintf( stream, "%u", vec.v[i] );
	}
	fprintf( stream, "]" );
}
void append_VEC_UINT(VEC_UINT vec, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	if (vec.v != NULL) {
		for (i=0; i<vec.n; i++)
			fprintf( stream, "%u ", vec.v[i] );
		fprintf( stream, "\n" );
	}
	fclose(stream);
}
/* We suppose that the numbers in this string are seperated by spaces and tables characters. */
void read_string2LIST_U_sepTables( char* string, LIST_STRING tokenTable, VEC_UINT* l )
{
	char seps[]   = "\t";
	char* token;
	int idx;
	init_VEC_UINT( l );
	token = strtok( string, seps );
	while( token != NULL ) {
		idx = lookup_LIST_STRING( token, tokenTable );
		if (idx==-1) {
			fprintf( stderr, "Error of string lookup: ""%s"" is not found in table\n" );
			exit( -1 );
		}
		addnumber_VEC_UINT( idx, l );
		token = strtok( NULL, seps );
	}
}
/* We suppose that the numbers in this string are written in the format of "set": they are
   seperated by spaces and comma characters, and started/ended by "[" and "]".
   For example, "[122, 561, 692, 490, 247]" */
void read_string2VEC_UINT_SetFormat( char* string, VEC_UINT* v )
{
	char StringCommaSpaceBrackets[]=", []";
	char* token;
	init_VEC_UINT( v );
	if (string==NULL || strlen(string)==0 ) return;
	token = strtok( string, StringCommaSpaceBrackets );
	while( token != NULL ) {
		addnumber_VEC_UINT( (unsigned int)atoi(token), v );
		token = strtok( NULL, StringCommaSpaceBrackets );
	}
}
/* We suppose that the numbers in this string are
   seperated by spaces.
   For example, "122, 561, 692, 490, 247" */
void read_string2VEC_UINT( char* string, VEC_UINT* v )
{
	char StringCommaSpaceBrackets[]=" ";
	char* token;
	init_VEC_UINT( v );
	if (string==NULL || strlen(string)==0 ) return;
	token = strtok( string, StringCommaSpaceBrackets );
	while( token != NULL ) {
		if (strlen(token)>0)
			addnumber_VEC_UINT( (unsigned int)atoi(token), v );
		token = strtok( NULL, StringCommaSpaceBrackets );
	}
}

/*---------------------------------
    Routines for VEC_FLOAT
-----------------------------------*/
void init_VEC_FLOAT(VEC_FLOAT* vec)
{
	vec->n = 0;
	vec->v = NULL;
}
void addnumber_VEC_FLOAT( float key, VEC_FLOAT* vec )
{
	unsigned int n;
	float *tmp;
	n = vec->n + 1;
	tmp = (float*)realloc( vec->v, n*sizeof(float) );
	if ( tmp == NULL ) erroralloc("float", n);
	vec->v = tmp;
	vec->v[n-1] = key;
	vec->n = n;
}
void free_VEC_FLOAT(VEC_FLOAT* vec)
{
	if (vec->v!=NULL) free( vec->v );
}
void create_VEC_FLOAT(unsigned int n, VEC_FLOAT* vec)
{
	unsigned int i;
	vec->n = n;
	vec->v = (float*)malloc( n*sizeof(float) );
	if ( vec->v == NULL ) erroralloc("float",n);
	for ( i=0; i<n; i++ ) vec->v[i] = 0;
}
/* suppose dst is allocated space, before calling */
void copy_VEC_FLOAT(VEC_FLOAT *dst, VEC_FLOAT src)
{
	unsigned int i;
	if (src.n != dst->n) {
		fprintf( stderr, "Dimension of two vectors do not agree, copy operation can not be performed!\n" );
		exit( -1 );
	}
	if (src.v==NULL) {
		free_VEC_FLOAT( dst );
		dst->v = NULL;
		return;
	}
	for ( i=0; i<src.n; i++ ) dst->v[i] = src.v[i];
}
void zeros_VEC_FLOAT(VEC_FLOAT* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++) vec->v[i] = 0;
}
void write_VEC_FLOAT(VEC_FLOAT x, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	for (i=0; i<x.n; i++) fprintf( stream, " %e", x.v[i] );
	fprintf( stream, "\n");
	fclose(stream);
}
void put_VEC_FLOAT(FILE* stream, VEC_FLOAT x, char* delim)
{
	unsigned int i;
	for (i=0; i<x.n-1; i++) fprintf( stream, "%g%s", x.v[i], delim );
	fprintf( stream, "%g", x.v[i] );
}
/* We suppose that the numbers in this string are written in the format of "set": they are
   seperated by spaces and comma characters, and started/ended by "[" and "]".
   For example, "[2.5, 7.8, 4.6, 13.2, 17.3]" */
void read_string2VEC_FLOAT_SetFormat( char* string, VEC_FLOAT* v )
{
	char StringCommaSpaceBrackets[]=", []";
	char* token;
	init_VEC_FLOAT( v );
	if (string==NULL || strlen(string)==0 ) return;
	token = strtok( string, StringCommaSpaceBrackets );
	while( token != NULL ) {
		addnumber_VEC_FLOAT( (float)atof(token), v );
		token = strtok( NULL, StringCommaSpaceBrackets );
	}
}
/* make list of numbers from the string with format "START_NUM:STEP:END_NUM" or "NUM"*/
int read_string2VEC_FLOAT_MATLABFormat( char* src, VEC_FLOAT* dst )
{
	const float ZERO_FLOAT=0.000001;
	char delim=':';
	char *token1, *token2, *token3;
	float start, step, end, v;
	int nColon=0, length=0;
	init_VEC_FLOAT( dst );
	// scan how many colons
	nColon = strcnt( src, delim );
	if (nColon==1) {
		fprintf( stderr, "Error string2VEC_FLOAT: There should not be only one colons."
			"The correct format is ""START_NUM:STEP:END_NUM"" or ""NUM"".\n" );
		return -1;
	}
	// get each number in string
	token1 = strtok_w( src, delim );
	if (token1==NULL) return -1;
	start = (float)atof( token1 );
	if (nColon==0) {
		length = 1;
		addnumber_VEC_FLOAT( start, dst );
		return length;
	}
	token2 = strtok_w( NULL, delim );
	if (strlen(token2)==0)
		step=0;
	else
		step = (float)atof( token2 );
	token3 = strtok_w( NULL, delim );
	if (token3==NULL || strlen(token3)==0) {
		fprintf( stderr, "Error string2VEC_FLOAT: END_NUM should not be empty.\n" );
		return -1;
	} else
		end = (float)atof( token3 );
	// convert START_NUM:STEP:END_NUM to a vector of floats.
	if ( start<=end && step<0 ) {
		fprintf( stderr, "Error string2VEC_FLOAT: START_NUM should > END_NUM, when STEP<0.\n" );
		return -1;
	}
	if ( start>=end && step>0 ) {
		fprintf( stderr, "Error string2VEC_FLOAT: START_NUM should < END_NUM, when STEP>0.\n" );
		return -1;
	}
	if ( start!=end && step==0 ) {
		fprintf( stderr, "Error string2VEC_FLOAT: START_NUM should = END_NUM, when STEP=0.\n" );
		return -1;
	}
	if (step==0) {
		length = 1;
		addnumber_VEC_FLOAT( start, dst );
	} else if (step>0) {
		v = start; length = 0;
		while ( fabs(v-end)<ZERO_FLOAT || v<end ) {
			addnumber_VEC_FLOAT( v, dst );
			v += step;
			length++;
		}
	} else if (step<0) {
		v = start; length = 0;
		while ( fabs(v-end)<ZERO_FLOAT || v>end ) {
			addnumber_VEC_FLOAT( v, dst );
			v += step;
			length++;
		}
	}
	//put_VEC_FLOAT( stdout, *dst, " "); // for debug
	return length;
}

/*---------------------------------
    Routines for VEC_DOUBLE
-----------------------------------*/
void init_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	vec->n = 0;
	vec->v = NULL;
}
void create_VEC_DOUBLE(unsigned int n, VEC_DOUBLE* vec)
{
	unsigned int i;
	vec->n = n;
	vec->v = (double*)malloc( n*sizeof(double) );
	if ( vec->v == NULL ) erroralloc("double",n);
	for ( i=0; i<n; i++ ) vec->v[i] = 0;
}
void createONES_VEC_DOUBLE(unsigned int n, VEC_DOUBLE* vec)
{
	unsigned int i;
	vec->n = n;
	vec->v = (double*)malloc( n*sizeof(double) );
	if ( vec->v == NULL ) erroralloc("double",n);
	for ( i=0; i<n; i++ ) vec->v[i] = 1;
}
void addnumber_VEC_DOUBLE( double key, VEC_DOUBLE* vec )
{
	unsigned int n;
	double *tmp;
	n = vec->n + 1;
	tmp = (double*)realloc( vec->v, n*sizeof(double) );
	if ( tmp == NULL ) erroralloc("double", n);
	vec->v = tmp;
	vec->v[n-1] = key;
	vec->n = n;
}
void free_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	if (vec->v!=NULL) free( vec->v );
}
/* suppose dst is allocated space, before calling */
void copy_VEC_DOUBLE(VEC_DOUBLE *dst, VEC_DOUBLE src)
{
	unsigned int i;
	if (src.n != dst->n) {
		fprintf( stderr, "Dimension of two vectors do not agree, copy operation can not be performed!\n" );
		exit( -1 );
	}
	if (src.v==NULL) {
		free_VEC_DOUBLE( dst );
		dst->v = NULL;
		return;
	}
	for ( i=0; i<src.n; i++ ) dst->v[i] = src.v[i];
}
void read_VEC_DOUBLE(char* file, VEC_DOUBLE* vec)
{
	FILE* stream;
	double v;
	int flag;
	init_VEC_DOUBLE( vec );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		flag = fscanf( stream, "%lf", &v );
		if (flag>0) addnumber_VEC_DOUBLE(v, vec);
	}
	fclose(stream);
}
void zeros_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++) vec->v[i] = 0;
}
void ones_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++) vec->v[i] = 1;
}
void checkZeros_VEC_DOUBLE( VEC_DOUBLE* vec )
{
	unsigned int i;
	for (i=0; i<vec->n; i++)
		if (vec->v[i]==0) vec->v[i] = ZERO_ALGORITHM;
}
unsigned int isNAN_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++)
		if (!IsNumber_DOUBLE(vec->v[i])) return TRUE;
	return FALSE;
}
unsigned int isINF_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	for (i=0; i<vec->n; i++)
		if (!IsFiniteNumber_DOUBLE(vec->v[i])) return TRUE;
	return FALSE;
}
void dotadd_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, double a)
{
	unsigned int i;
	if (dst->n!=src.n) {
		printf("Error: size of two VEC_DOUBLE do not agree\n");
		exit( -1 );
	}
	for (i=0; i<dst->n; i++) dst->v[i]+=a*src.v[i];
}
/* dst <- dst./src */
void dotdiv_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src)
{
	unsigned int i;
	if (dst->n!=src.n) {
		printf("Error: size of two VEC_DOUBLE do not agree\n");
		exit( -1 );
	}
	for (i=0; i<dst->n; i++) dst->v[i]/=src.v[i];
}
/* dst <- dst.*src */
void dotmul_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src)
{
	unsigned int i;
	if (dst->n!=src.n) {
		printf("Error: size of two VEC_DOUBLE do not agree\n");
		exit( -1 );
	}
	for (i=0; i<dst->n; i++) dst->v[i]*=src.v[i];
}
/* dst <- dst.^p */
void dotpow_VEC_DOUBLE(VEC_DOUBLE* dst, double p)
{
	unsigned int i;
	for (i=0; i<dst->n; i++) dst->v[i]=pow(dst->v[i],p);
}
/* dst <- dst.^p */
void dotpow_uintexponent_VEC_DOUBLE(VEC_DOUBLE* dst, unsigned int p)
{
	unsigned int i;
	for (i=0; i<dst->n; i++) dst->v[i]=power_uintExponent(dst->v[i],p);
}
/* dst <- (dst.^p).*src */
void dotpowmul_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, double p)
{
	unsigned int i;
	if (dst->n!=src.n) {
		printf("Error: size of two VEC_DOUBLE do not agree\n");
		exit( -1 );
	}
	for (i=0; i<dst->n; i++) dst->v[i]=pow(dst->v[i],p)*src.v[i];
}
/* dst <- (dst.^p).*src */
void dotpowmul_uintexponent_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, unsigned int p)
{
	unsigned int i;
	if (dst->n!=src.n) {
		printf("Error: size of two VEC_DOUBLE do not agree\n");
		exit( -1 );
	}
	for (i=0; i<dst->n; i++) dst->v[i]=power_uintExponent(dst->v[i],p)*src.v[i];
}
double sum_VEC_DOUBLE(VEC_DOUBLE vec)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<vec.n; i++) sum+=vec.v[i];
	return sum;
}
double sum_exponent_of_VEC_DOUBLE(VEC_DOUBLE vec, double exponent)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<vec.n; i++) sum+=pow( vec.v[i], exponent );
	return sum;
}
double sum_uint_exponent_of_VEC_DOUBLE(VEC_DOUBLE vec, unsigned int exponent)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<vec.n; i++) sum+=power_uintExponent( vec.v[i], exponent );
	return sum;
}
/* This function does two operations: dst=src.*dst; dst=dst/sum(dst); */
void innerProductAndnorm1_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE* src)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<dst->n; i++) {
		dst->v[i] *= src->v[i];
		sum += dst->v[i];
	}
	if (sum==0) return;
	for (i=0; i<dst->n; i++) dst->v[i] /= sum;
}
void norm1_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<vec->n; i++) sum+=vec->v[i];
	if (sum==0) return;
	for (i=0; i<vec->n; i++) vec->v[i] /=sum;
}
void norm2_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	double sum=0;
	for (i=0; i<vec->n; i++) sum+=vec->v[i]*vec->v[i];
	if (sum==0) return;
	sum = sqrt(sum);
	for (i=0; i<vec->n; i++) vec->v[i] /=sum;
}
double max_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	double max=vec->v[0];
	for (i=0; i<vec->n; i++)
		if (vec->v[i]>max) max=vec->v[i];
	return max;
}
double min_VEC_DOUBLE(VEC_DOUBLE* vec)
{
	unsigned int i;
	double min=vec->v[0];
	for (i=0; i<vec->n; i++)
		if (vec->v[i]<min) min=vec->v[i];
	return min;
}
double avg_VEC_DOUBLE(VEC_DOUBLE vec)
{
	return sum_VEC_DOUBLE(vec)/vec.n;
}
double avg_selected_VEC_DOUBLE(VEC_DOUBLE vec, VEC_UINT sortidx, int topK)
{
	unsigned int i, n;
	double sum=0;
	n = topK;
	if (n>vec.n) n=vec.n;
	for (i=0; i<n; i++)
		sum += vec.v[sortidx.v[i]];
	return sum/n;
}
double std_VEC_DOUBLE(VEC_DOUBLE vec, double avg)
{
	int i;
	double std = 0;
	for (i=0; i<vec.n; i++) std+=(vec.v[i]-avg)*(vec.v[i]-avg);
	return sqrt( std / vec.n );
}
double std_selected_VEC_DOUBLE(VEC_DOUBLE vec, double avg, VEC_UINT sortidx, int topK)
{
	unsigned int i, n;
	double v, sum=0;
	n = topK;
	if (n>vec.n) n=vec.n;
	for (i=0; i<n; i++) {
		v = vec.v[sortidx.v[i]];
		sum += (v-avg)*(v-avg);
	}
	return sqrt( sum / n );
}
void threshold_VEC_DOUBLE(VEC_DOUBLE* vec, int max_nnz, double alpha, VEC_DOUBLE* vec_copy)
{
	int i, sparsity=max_nnz;
	double avg, std, lambda, lambda1;
	VEC_UINT sortidx;
	if (sparsity>=vec->n-1) return;
	if (max_VEC_DOUBLE(vec)==min_VEC_DOUBLE(vec)) return; // all elements are the same, because their std is almost zero
	createINDEX_VEC_UINT(vec->n, &sortidx);
	quicksortidx_dec_VEC_DOUBLE(*vec_copy, &sortidx);
	lambda1 = vec->v[sortidx.v[sparsity]];
	if (lambda1==vec->v[sortidx.v[0]]) { // if all the top-ranking 'thrd' values are the same, assign them to 1
		for (i=0; i<vec->n; i++)
			if (vec->v[i] < lambda1)
				vec->v[i] = 0;
			else
				vec->v[i] = 1;
	} else {
		avg = avg_selected_VEC_DOUBLE( *vec, sortidx, sparsity );
		std = std_selected_VEC_DOUBLE( *vec, avg, sortidx, sparsity );
		lambda = avg - alpha*std;
		if (lambda<0) lambda = avg;
		lambda = _max_double( lambda, lambda1 ); // adjusted lambda
		for (i=0; i<vec->n; i++)
			if (vec->v[i] < lambda)
				vec->v[i] = 0;
	}
	return;
}
/* sigmoid(x,a)=1/(1+exp(-a*x))=0.5+0.5*atan(0.5*a*x)
   this function is used for smoothing values. */
void sigmoid_VEC_DOUBLE(VEC_DOUBLE* vec, double a)
{
	unsigned int i;
	for (i=0; i<vec->n; i++)  vec->v[i] = 1/(1+exp(-a*vec->v[i]));
	//for (i=0; i<vec->n; i++)  vec->v[i] = 0.5+0.5*atan(0.5*a*vec->v[i]);
}
void sinh_VEC_DOUBLE(VEC_DOUBLE* vec, double b)
{
	unsigned int i;
	for (i=0; i<vec->n; i++)  vec->v[i] = sinh(b*vec->v[i]);
}
void write_2VEC_DOUBLE(VEC_DOUBLE x, VEC_DOUBLE y, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	fprintf( stream, "x:\n" );
	for (i=0; i<x.n; i++) fprintf( stream, " %e", x.v[i] );
	fprintf( stream, "\n\ny:\n");
	for (i=0; i<y.n; i++) fprintf( stream, " %e", y.v[i] );
	fprintf( stream, "\n");
	fclose(stream);
}
void append_2VEC_DOUBLE_SetFormat(VEC_DOUBLE x, VEC_DOUBLE y, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	fprintf( stream, "[" );
	if (x.v!=NULL) {
		for (i=0; i<x.n-1; i++) fprintf( stream, "%e, ", x.v[i] );
		fprintf( stream, "%e]\t[", x.v[i] );
	} else
		fprintf( stream, "]\t[" );
	if (y.v!=NULL) {
		for (i=0; i<y.n-1; i++) fprintf( stream, "%e, ", y.v[i] );
		fprintf( stream, "%e]\n", y.v[i] );
	} else
		fprintf( stream, "]\n" );
	fclose(stream);
}
void write_VEC_DOUBLE(VEC_DOUBLE x, char* file)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	for (i=0; i<x.n; i++) fprintf( stream, " %e", x.v[i] );
	fprintf( stream, "\n");
	fclose(stream);
}

/*---------------------------------
    Routines for MATRIX_UINT
-----------------------------------*/
void init_MATRIX_UINT( MATRIX_UINT* v )
{
	v->nrow = 0;
	v->ncol = 0;
	v->mat = NULL;
}
void create_MATRIX_UINT(unsigned int nrow, unsigned int ncol, MATRIX_UINT* v)
{
	unsigned int i,j;
	v->nrow = nrow;
	v->ncol = ncol;
	v->mat = (unsigned int**)malloc( nrow*sizeof(unsigned int*) );
	if ( v->mat == NULL ) erroralloc("unsigned int*",nrow);
	for ( i=0; i<nrow; i++ ) {
		v->mat[i] = (unsigned int*)malloc( ncol*sizeof(unsigned int) );
		if ( v->mat[i] == NULL ) erroralloc("unsigned int",ncol);
		for ( j=0; j<ncol; j++ ) v->mat[i][j] = 0;
	}
}
void free_MATRIX_UINT( MATRIX_UINT* v )
{
	unsigned int i;
	if (v->mat!=NULL) {
		for ( i=0; i<v->nrow; i++ )
			if (v->mat[i]!=NULL) free( v->mat[i] );
		free( v->mat );
	}
}

/*---------------------------------
    Routines for MATRIX_FLOAT
-----------------------------------*/
void init_MATRIX_FLOAT(MATRIX_FLOAT* v)
{
	v->nrow = 0;
	v->ncol = 0;
	v->mat = NULL;
}
void create_MATRIX_FLOAT(unsigned int nrow, unsigned int ncol, MATRIX_FLOAT* v)
{
	unsigned int i,j;
	v->nrow = nrow;
	v->ncol = ncol;
	v->mat = (float**)malloc( nrow*sizeof(float*) );
	if ( v->mat == NULL ) erroralloc("float*",nrow);
	for ( i=0; i<nrow; i++ ) {
		v->mat[i] = (float*)malloc( ncol*sizeof(float) );
		if ( v->mat[i] == NULL ) erroralloc("float",ncol);
		for ( j=0; j<ncol; j++ ) v->mat[i][j] = 0;
	}
}
void create_MATRIX_FLOAT_byDefaultValue(unsigned int nrow, unsigned int ncol, float default_entryValue, float default_diagValue, MATRIX_FLOAT* v)
{
	unsigned int i,j;
	v->nrow = nrow;
	v->ncol = ncol;
	v->mat = (float**)malloc( nrow*sizeof(float*) );
	if ( v->mat == NULL ) erroralloc("float*",nrow);
	for ( i=0; i<nrow; i++ ) {
		v->mat[i] = (float*)malloc( ncol*sizeof(float) );
		if ( v->mat[i] == NULL ) erroralloc("float",ncol);
		for ( j=0; j<ncol; j++ ) v->mat[i][j] = default_entryValue;
		v->mat[i][i] = default_diagValue;
	}
}
MATRIX_FLOAT* create_pointer_MATRIX_FLOAT()
{
	MATRIX_FLOAT* v;
	v = (MATRIX_FLOAT*)malloc( sizeof(MATRIX_FLOAT) );
	if ( v == NULL ) erroralloc("MATRIX_FLOAT",1);
	init_MATRIX_FLOAT( v );
	return v;
}
void free_MATRIX_FLOAT( MATRIX_FLOAT* v )
{
	unsigned int i;
	if (v->mat!=NULL) {
		for ( i=0; i<v->nrow; i++ )
			if (v->mat[i]!=NULL) free( v->mat[i] );
		free( v->mat );
	}
}
void copy_MATRIX_FLOAT( MATRIX_FLOAT src, MATRIX_FLOAT* dst )
{
	unsigned int i, j;
	free_MATRIX_FLOAT( dst );
	init_MATRIX_FLOAT( dst );
	if (src.mat!=NULL) {
		create_MATRIX_FLOAT( src.nrow, src.ncol, dst );
		for (i=0; i<src.nrow; i++)
			for (j=0; j<src.ncol; j++)
				dst->mat[i][j] = src.mat[i][j];
	}
}
/* sigmoid(x,a,b)=1/(1+exp(-a*(x-b)))
   this function is used for enlarging the differences among values in the range [0,1]. */
void sigmoid_SYM_MATRIX_FLOAT(MATRIX_FLOAT* sym, double a, double b)
{
	unsigned int i, j;
	if (sym->nrow!=sym->ncol) {
		fprintf( stderr, "Error sigmoid_SYM_MATRIX_FLOAT: #rows (%u) should be equal to #columns (%u) in a symmetric matrix\n", sym->nrow, sym->ncol );
		exit(-1);
	}
	for (i=0; i<sym->nrow; i++)
		for (j=i+1; j<sym->ncol; j++)
			if (sym->mat[i][j]!=1 && sym->mat[i][j]!=0) {
				sym->mat[i][j] = 1/(1+exp(-a*(sym->mat[i][j]-b)));
				sym->mat[j][i] = sym->mat[i][j];
			}
}
/* do not consider the diagonal values */
float max_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym)
{
	unsigned int i, j;
	float max;
	if (sym.nrow!=sym.ncol) {
		fprintf( stderr, "Error max_SYM_MATRIX_FLOAT: #rows (%u) should be equal to #columns (%u) in a symmetric matrix\n", sym.nrow, sym.ncol );
		exit(-1);
	}
	if (sym.nrow==1) { max=sym.mat[0][0]; return max; }
	 max=sym.mat[0][1];
	for (i=0; i<sym.nrow; i++)
		for (j=i+1; j<sym.ncol; j++)
			if (max<sym.mat[i][j]) max=sym.mat[i][j];
	return max;
}
/* do not consider the diagonal values */
float min_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym)
{
	unsigned int i, j;
	float min;
	if (sym.nrow!=sym.ncol) {
		fprintf( stderr, "Error min_SYM_MATRIX_FLOAT: #rows (%u) should be equal to #columns (%u) in a symmetric matrix\n", sym.nrow, sym.ncol );
		exit(-1);
	}
	if (sym.nrow==1) { min=sym.mat[0][0]; return min; }
	 min=sym.mat[0][1];
	for (i=0; i<sym.nrow; i++)
		for (j=i+1; j<sym.ncol; j++)
			if (min>sym.mat[i][j]) min=sym.mat[i][j];
	return min;
}
/* scale the range of the matrix from [min,max] to [0,1] */
void scale_SYM_MATRIX_FLOAT(MATRIX_FLOAT *sym)
{
	float min, max, range, v;
	unsigned int i, j;
	if (sym->mat==NULL) return;
	min = min_SYM_MATRIX_FLOAT(*sym);
	max = max_SYM_MATRIX_FLOAT(*sym);
	range = max - min;
	if (range==0) return;
	for (i=0; i<sym->nrow; i++)
		for (j=i+1; j<sym->ncol; j++) {
			v = (sym->mat[i][j]-min) / range;
			sym->mat[i][j] = v;
			sym->mat[j][i] = v;
		}
}
/* calculate the average of entries in selected rows/columns.
   'sym':                input - symmetric matrix
   'selectedRows_index': input - index list of selected rows/columns, (index should start from 0);
   'count_diagvalues':   input - TRUE means to use diagonal values in average calculation, otherwise FALSE indicats not to use;
   'avg_vec':            output - averages for submatrix of the top-k selected rows/columns (k=1,2,...,selectedRows_index.n).
   
   Before calling, make sure 'avg_vec' is supposed to have no space allocated yet. */
void average_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym, VEC_UINT selectedRows_index, BOOL count_diagvalues, VEC_FLOAT* avg_vec)
{
	unsigned int i, j, r, c;
	if (sym.nrow<selectedRows_index.n) {
		fprintf( stderr, "Error average_SYM_MATRIX_FLOAT: size of 'selectedRows_index' should be <= size of 'sym'\n" );
		exit(-1);
	}
	init_VEC_FLOAT( avg_vec );
	create_VEC_FLOAT( selectedRows_index.n, avg_vec ); // avg_vec is already initialized to zero in 'create_VEC_FLOAT'
	if (count_diagvalues) {
		for (i=0; i<selectedRows_index.n; i++) {
			r = selectedRows_index.v[i];
			for (j=0; j<i+1; j++) {
				c = selectedRows_index.v[j];
				avg_vec->v[i] += sym.mat[r][c];
			}
		}
		for (i=0; i<selectedRows_index.n-1; i++) avg_vec->v[i+1] +=avg_vec->v[i];
		for (i=0; i<selectedRows_index.n; i++) avg_vec->v[i] /= (i+1)*(i+2)/2;
	} else {
		for (i=0; i<selectedRows_index.n; i++) {
			r = selectedRows_index.v[i];
			for (j=0; j<i; j++) {
				c = selectedRows_index.v[j];
				avg_vec->v[i] += sym.mat[r][c];
			}
		}
		for (i=0; i<selectedRows_index.n-1; i++) avg_vec->v[i+1] +=avg_vec->v[i];
		for (i=1; i<selectedRows_index.n; i++) avg_vec->v[i] /= i*(i+1)/2;
	}
}
/* before calling, "net" is already allocated space and have values. We read "file" and add values to corresponding entries. */
void read_and_accumulate_SYM_MATRIX_FLOAT_IndexBy1(char* file, MATRIX_FLOAT* net)
{
	FILE* stream;
	int idx1, idx2;
	unsigned int n=net->nrow;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if ( idx1>=1 && idx1<=n ) {
					token2 = strtok( NULL, sep_tab );
					if (token2 != NULL) {
						idx2 = atoi( token2 );
						if ( idx2>=1  && idx2<=n ) {
							token3 = strtok( NULL, sep_tab );
							if ( token3 != NULL ) {
								weight = (float)atof(token3);
								net->mat[idx1-1][idx2-1] += weight;
								net->mat[idx2-1][idx1-1] += weight;
							}
						}
					}
				}
			}
		}
	}
	fclose(stream);
}
/* before calling, "net" is already allocated space and have values. We read "file" and add values to corresponding entries. */
void read_and_accumulate_SYM_MATRIX_FLOAT_IndexBy0(char* file, MATRIX_FLOAT* net)
{
	FILE* stream;
	int idx1, idx2;
	unsigned int n=net->nrow;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if ( idx1>=0 && idx1<n ) {
					token2 = strtok( NULL, sep_tab );
					if (token2 != NULL) {
						idx2 = atoi( token2 );
						if ( idx2>=0  && idx2<n ) {
							token3 = strtok( NULL, sep_tab );
							if ( token3 != NULL ) {
								weight = (float)atof(token3);
								net->mat[idx1][idx2] += weight;
								net->mat[idx2][idx1] += weight;
							}
						}
					}
				}
			}
		}
	}
	fclose(stream);
}
void read_SYM_MATRIX_FLOAT_IndexBy1(char* file, unsigned int n, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net)
{
	FILE* stream;
	int idx1, idx2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_MATRIX_FLOAT( net );
	// scan the file to get the number of rows/columns 'n'
	/*
	n = 0;
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if (n<idx1) n = idx1;
				token2 = strtok( NULL, sep_tab );
				if (token2 != NULL) {
					idx2 = atoi( token2 );
					if (n<idx2) n = idx2;
				}
			}
		}
	}
	fclose(stream);
	*/
	if (n==0) {
		fprintf( stderr, "Error read_SYM_MATRIX_FLOAT_IndexBy1: # of rows/columns should be >= 1\n" );
		exit(-1);
	}
	// scan the file again to read matrix data to memory
	create_MATRIX_FLOAT_byDefaultValue( n, n, default_entryValue, default_diagValue, net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if ( idx1>=1  && idx1<=n ) {
					token2 = strtok( NULL, sep_tab );
					if (token2 != NULL) {
						idx2 = atoi( token2 );
						if ( idx2>=1  && idx2<=n ) {
							token3 = strtok( NULL, sep_tab );
							if ( token3 != NULL ) {
								weight = (float)atof(token3);
								net->mat[idx1-1][idx2-1] = weight;
								net->mat[idx2-1][idx1-1] = weight;
							}
						}
					}
				}
			}
		}
	}
	fclose(stream);
}
void read_SYM_MATRIX_FLOAT_IndexBy0(char* file, unsigned int n, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net)
{
	FILE* stream;
	int idx1, idx2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_MATRIX_FLOAT( net );
	// scan the file to get the number of rows/columns 'n'
	/*
	n = -1;
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if (n<idx1) n = idx1;
				token2 = strtok( NULL, sep_tab );
				if (token2 != NULL) {
					idx2 = atoi( token2 );
					if (n<idx2) n = idx2;
				}
			}
		}
	}
	fclose(stream);
	n = n+1;
	*/
	if (n==0) {
		fprintf( stderr, "Error read_SYM_MATRIX_FLOAT_IndexBy0: # of rows/columns should be >= 1\n" );
		exit(-1);
	}
	// scan the file again to read matrix data to memory
	create_MATRIX_FLOAT_byDefaultValue( n, n, default_entryValue, default_diagValue, net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if (token1 != NULL) {
				idx1 = atoi( token1 );
				if ( idx1>=0 && idx1<n ) {
					token2 = strtok( NULL, sep_tab );
					if (token2 != NULL) {
						idx2 = atoi( token2 );
						if ( idx2>=0 && idx1<n ) {
							token3 = strtok( NULL, sep_tab );
							if ( token3 != NULL ) {
								weight = (float)atof(token3);
								net->mat[idx1][idx2] = weight;
								net->mat[idx2][idx1] = weight;
							}
						}
					}
				}
			}
		}
	}
	fclose(stream);
}

void read_SYM_MATRIX_FLOAT_byID(char* file, LIST_STRING list_ids, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net)
{
	FILE* stream;
	unsigned int n=list_ids.n, idx1, idx2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_MATRIX_FLOAT( net );
	create_MATRIX_FLOAT_byDefaultValue( n, n, default_entryValue, default_diagValue, net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			idx1 = lookup_LIST_STRING( token1, list_ids );
			if ( idx1 != -1 ) {
				token2 = strtok( NULL, sep_tab );
				idx2 = lookup_LIST_STRING( token2, list_ids );
				if ( idx2 != -1 ) {
					token3 = strtok( NULL, sep_tab );
					if ( token3 != NULL ) {
						weight = (float)atof(token3);
						net->mat[idx1][idx2] = weight;
						net->mat[idx2][idx1] = weight;
					}
				}
			}
		}
	}
	fclose(stream);
}
/* When 'format'==0, print edges with index starting from 0; When 'format'==1, print edges with index starting from 1; when 'format'==-1, print matrix */
void write_SYM_MATRIX_FLOAT(char* file, MATRIX_FLOAT net, int format)
{
	FILE* stream;
	unsigned int i,j,n;
	if (format!=-1 && format!=0 && format!=1) {
		fprintf( stderr, "Error write_SYM_MATRIX_FLOAT: print format should be -1 (matrix), 0 (edges whose nodes indexed from 0), or 1 (edges whose nodes indexed from 1)\n" );
		exit(-1);
	}
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (net.mat!=NULL) {
		if ( net.nrow != net.ncol ) {
			fprintf( stderr, "Error write_SYM_MATRIX_FLOAT: matrix is not symmetric (%u x %u)\n", net.nrow, net.ncol );
			exit(-1);
		}
		n = net.nrow;
		if ( format==0 ) { /* ==0: print only non-zero weighted edges whose nodes are indexed from 0 */
			for (i=0; i<n; i++)
				for (j=i+1; j<n; j++)
					if (net.mat[i][j]!=0)
						fprintf(stream, "%u\t%u\t%.6g\n", i, j, net.mat[i][j]);
		}
		if ( format==1 ) { /* ==1: print only non-zero weighted edges whose nodes are indexed from 0 */
			for (i=0; i<n; i++)
				for (j=i+1; j<n; j++)
					if (net.mat[i][j]!=0)
						fprintf(stream, "%u\t%u\t%.6g\n", i+1, j+1, net.mat[i][j]);
		}
		if ( format==-1 ) { /* ==-1: print matrix */
			for (i=0; i<n; i++) {
				for (j=0; j<n-1; j++)
					fprintf(stream,"%.6g\t", net.mat[i][j]);
				fprintf(stream,"%.6g\n", net.mat[i][n-1]);
			}
		}
	}
	fclose(stream);
}
/* When 'format'!=-1, print edges; when 'format'==-1, print matrix */
void write_SYM_MATRIX_FLOAT_byID(char* file, MATRIX_FLOAT net, LIST_STRING list_ids, int format)
{
	FILE* stream;
	unsigned int i,j,n;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (net.mat!=NULL) {
		if ( net.nrow != net.ncol ) {
			fprintf( stderr, "Error write_SYM_MATRIX_FLOAT_byID: matrix is not symmetric (%u x %u)\n", net.nrow, net.ncol );
			exit(-1);
		}
		if ( net.nrow != list_ids.n ) {
			fprintf( stderr, "Error write_SYM_MATRIX_FLOAT_byID: matrix's dimension (%u,%u) does not agree with that of id list (%u)\n", net.nrow, net.ncol, list_ids.n );
			exit(-1);
		}
		n = net.nrow;
		if ( format!=-1 ) { /* !=-1: print only non-zero weighted edges */
			for (i=0; i<n; i++)
				for (j=i+1; j<n; j++)
					if (net.mat[i][j]!=0)
						fprintf(stream, "%s\t%s\t%.6g\n", list_ids.string[i], list_ids.string[j], net.mat[i][j]);
		} else { /* ==-1: print matrix */
			for (i=0; i<n; i++) fprintf(stream,"\t%s", list_ids.string[i]);
			fprintf(stream,"\n");
			for (i=0; i<n; i++) {
				fprintf(stream,"%s", list_ids.string[i]);
				for (j=0; j<n; j++)
					fprintf(stream,"\t%.6g", net.mat[i][j]);
				fprintf(stream,"\n");
			}
		}
	}
	fclose(stream);
}
void write_MATRIX_FLOAT_byID(char* file, MATRIX_FLOAT mat, VEC_UINT rowIDs, LIST_STRING columnIDs, char* preString_rowID, char* preString_columnID)
{
	FILE* stream;
	unsigned int i,j;
	if (mat.nrow!=rowIDs.n) {
		fprintf( stderr, "Error write_MATRIX_FLOAT_byID: matrix's row number (%u) is not equal to number of rowIDs (%u)\n", mat.nrow, rowIDs.n );
		exit(-1);
	}
	if (mat.ncol!=columnIDs.n) {
		fprintf( stderr, "Error write_MATRIX_FLOAT_byID: matrix's column number (%u) is not equal to number of columnIDs (%u)\n", mat.ncol, columnIDs.n );
		exit(-1);
	}
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (mat.mat!=NULL) {
		for (j=0; j<columnIDs.n; j++) fprintf(stream,"\t%s%s", preString_columnID, columnIDs.string[j]);
		fprintf(stream,"\n");
		for (i=0; i<mat.nrow; i++) {
			fprintf(stream,"%s%u", preString_rowID, rowIDs.v[i]);
			for (j=0; j<mat.ncol; j++)
				fprintf(stream,"\t%.6g", mat.mat[i][j]);
			fprintf(stream,"\n");
		}
	}
	fclose(stream);
}
void write_MATRIX_FLOAT_byUINTID(char* file, MATRIX_FLOAT mat, VEC_UINT rowIDs, VEC_UINT columnIDs, char* preString_rowID, char* preString_columnID)
{
	FILE* stream;
	unsigned int i,j;
	if (mat.nrow!=rowIDs.n) {
		fprintf( stderr, "Error write_MATRIX_FLOAT_byID: matrix's row number (%u) is not equal to number of rowIDs (%u)\n", mat.nrow, rowIDs.n );
		exit(-1);
	}
	if (mat.ncol!=columnIDs.n) {
		fprintf( stderr, "Error write_MATRIX_FLOAT_byID: matrix's column number (%u) is not equal to number of columnIDs (%u)\n", mat.ncol, columnIDs.n );
		exit(-1);
	}
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (mat.mat!=NULL) {
		for (j=0; j<columnIDs.n; j++) fprintf(stream,"\t%s%u", preString_columnID, columnIDs.v[j]);
		fprintf(stream,"\n");
		for (i=0; i<mat.nrow; i++) {
			fprintf(stream,"%s%u", preString_rowID, rowIDs.v[i]);
			for (j=0; j<mat.ncol; j++)
				fprintf(stream,"\t%.6g", mat.mat[i][j]);
			fprintf(stream,"\n");
		}
	}
	fclose(stream);
}

/*---------------------------------
    Routines for MATRIX_DOUBLE
-----------------------------------*/
void init_MATRIX_DOUBLE(MATRIX_DOUBLE* v)
{
	v->nrow = 0;
	v->ncol = 0;
	v->mat = NULL;
}
void create_MATRIX_DOUBLE(unsigned int nrow, unsigned int ncol, MATRIX_DOUBLE* v)
{
	unsigned int i,j;
	v->nrow = nrow;
	v->ncol = ncol;
	v->mat = (double**)malloc( nrow*sizeof(double*) );
	if ( v->mat == NULL ) erroralloc("double*",nrow);
	for ( i=0; i<nrow; i++ ) {
		v->mat[i] = (double*)malloc( ncol*sizeof(double) );
		if ( v->mat[i] == NULL ) erroralloc("double",ncol);
		for ( j=0; j<ncol; j++ ) v->mat[i][j] = 0;
	}
}
MATRIX_DOUBLE* create_pointer_MATRIX_DOUBLE()
{
	MATRIX_DOUBLE* v;
	v = (MATRIX_DOUBLE*)malloc( sizeof(MATRIX_DOUBLE) );
	if ( v == NULL ) erroralloc("MATRIX_DOUBLE",1);
	init_MATRIX_DOUBLE( v );
	return v;
}
void free_MATRIX_DOUBLE( MATRIX_DOUBLE* v )
{
	unsigned int i;
	if (v->mat!=NULL) {
		for ( i=0; i<v->nrow; i++ )
			if (v->mat[i]!=NULL) free( v->mat[i] );
		free( v->mat );
	}
}
/* "dst" has been allocated space by using "create_MATRIX_DOUBLE", before calling. */
void elementwise_pow_MATRIX_DOUBLE(MATRIX_DOUBLE src, double p, MATRIX_DOUBLE* dst)
{
	unsigned int i, j;
	if ( dst==NULL ) {
		fprintf( stderr, "Error : dst matrix is allocated space yet (pointer is NULL)\n" );
		exit(-1);
	}
	if ( dst->mat==NULL ) {
		fprintf( stderr, "Error : dst matrix is allocated space yet (the field '.mat' is NULL)\n" );
		exit(-1);
	}
	if ( (dst->nrow != src.nrow) || (dst->ncol != src.ncol) ) {
		fprintf( stderr, "Error : src's dimension (%u,%u) does not agree with that of dst (%u,%u)\n", src.nrow, src.ncol, dst->nrow, dst->ncol );
		exit(-1);
	}
	for (i=0; i<src.nrow; i++)
		for (j=0; j<src.ncol; j++)
			dst->mat[i][j] = pow( src.mat[i][j], p );
}
/* "dst" and "tmp" has been allocated space by using "create_MATRIX_DOUBLE" and "create_VEC_DOUBLE", before calling. */
void outer_powsum_VEC_DOUBLE2MATRIX_DOUBLE(VEC_DOUBLE src, double p, MATRIX_DOUBLE* dst, VEC_DOUBLE tmp)
{
	unsigned int i, j, n;
	if ( dst==NULL ) {
		fprintf( stderr, "Error : matrix is allocated space yet (pointer is NULL)\n" );
		exit(-1);
	}
	if ( dst->mat==NULL ) {
		fprintf( stderr, "Error : matrix is allocated space yet (the field '.mat' is NULL)\n" );
		exit(-1);
	}
	if ( dst->nrow != dst->ncol ) {
		fprintf( stderr, "Error : matrix is not symmetric (%u x %u)\n", dst->nrow, dst->ncol );
		exit(-1);
	}
	if ( dst->nrow != src.n ) {
		fprintf( stderr, "Error : matrix's dimension (%u,%u) does not agree with that of id list (%u)\n", dst->nrow, dst->ncol, src.n );
		exit(-1);
	}
	if ( tmp.n != src.n ) {
		fprintf( stderr, "Error : src's dimension (%u) does not agree with that of tmp vector (%u)\n", src.n, tmp.n );
		exit(-1);
	}
	n = src.n;
	for (i=0; i<n; i++) tmp.v[i] = pow(src.v[i],p);
	for (i=0; i<n; i++)
		for (j=0; j<n; j++)
			dst->mat[i][j] = tmp.v[i] + tmp.v[j];
}
/* "dst" and "tmp" has been allocated space by using "create_MATRIX_DOUBLE" and "create_VEC_DOUBLE", before calling. */
void outer_unitpowsum_VEC_DOUBLE2MATRIX_DOUBLE(VEC_DOUBLE src, unsigned int p, MATRIX_DOUBLE* dst, VEC_DOUBLE tmp)
{
	unsigned int i, j, n;
	if ( dst==NULL ) {
		fprintf( stderr, "Error : matrix is allocated space yet (pointer is NULL)\n" );
		exit(-1);
	}
	if ( dst->mat==NULL ) {
		fprintf( stderr, "Error : matrix is allocated space yet (the field '.mat' is NULL)\n" );
		exit(-1);
	}
	if ( dst->nrow != dst->ncol ) {
		fprintf( stderr, "Error : matrix is not symmetric (%u x %u)\n", dst->nrow, dst->ncol );
		exit(-1);
	}
	if ( dst->nrow != src.n ) {
		fprintf( stderr, "Error : matrix's dimension (%u,%u) does not agree with that of id list (%u)\n", dst->nrow, dst->ncol, src.n );
		exit(-1);
	}
	if ( tmp.n != src.n ) {
		fprintf( stderr, "Error : src's dimension (%u) does not agree with that of tmp vector (%u)\n", src.n, tmp.n );
		exit(-1);
	}
	n = src.n;
	for (i=0; i<n; i++) tmp.v[i] = power_uintExponent(src.v[i],p);
	for (i=0; i<n; i++)
		for (j=0; j<n; j++)
			dst->mat[i][j] = tmp.v[i] + tmp.v[j];
}
/* When 'format'!=0, print non-zero entries; when 'format'==0, print matrix */
void write_MATRIX_DOUBLE(MATRIX_DOUBLE mat, char* file, unsigned int format)
{
	FILE* stream;
	unsigned int i,j;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (mat.mat!=NULL) {
		if ( format ) { /* 1: print only non-zero weighted edges */
			for (i=0; i<mat.nrow; i++)
				for (j=0; j<mat.ncol; j++)
					if (mat.mat[i][j]!=0)
						fprintf(stream, "%u\t%u\t%g\n", i+1, j+1, mat.mat[i][j]);
		} else { /* 0: print matrix */
			for (i=0; i<mat.nrow; i++) {
				for (j=0; j<mat.ncol-1; j++)
					fprintf(stream,"%g\t", mat.mat[i][j]);
				fprintf(stream,"%g\n",mat.mat[i][mat.ncol-1]);
			}
		}
	}
	fclose(stream);
}

/*---------------------------------
    Routines for NET
-----------------------------------*/
void init_NET(NET* net)
{
	net->nEdge = 0;
	net->nGene = 0;
	init_VEC_UINT( &net->gene1IDX );
	init_VEC_UINT( &net->gene2IDX );
	init_VEC_FLOAT( &net->w );
}
NET* create_pointer_NET()
{
	NET* v;
	v = (NET*)malloc( sizeof(NET) );
	if ( v == NULL ) erroralloc("NET",1);
	init_NET( v );
	return v;
}
void free_NET(NET* net)
{
	free_VEC_UINT( &net->gene1IDX );
	free_VEC_UINT( &net->gene2IDX );
	free_VEC_FLOAT( &net->w );
}
unsigned int zerosAllEdges_Of_NET_BetweenSelectedGenes(NET* net, VEC_UINT geneRanks)
{
	unsigned int nEdges_Masked, i, gene1IDX, gene2IDX, rank1, rank2;
	nEdges_Masked = 0;
	for (i=0; i<net->nEdge; i++) {
		gene1IDX = net->gene1IDX.v[i];
		gene2IDX = net->gene2IDX.v[i];
		if ((gene1IDX || gene2IDX)!=0) { // when both geneIDX are zero, this edge is disabled.
			rank1 = geneRanks.v[gene1IDX];
			rank2 = geneRanks.v[gene2IDX];
			if ( (rank1 && rank2)!=0 ) { // "both genes" are selected in the subnetwork and with rank information (>=1)
				// disable this edge
				net->gene1IDX.v[i] = 0;
				net->gene2IDX.v[i] = 0;
				nEdges_Masked++;
			}
		}
	}
	return nEdges_Masked;
}
unsigned int zerosAllEdges_Of_NET_AdjacentToSelectedGenes(NET* net, VEC_UINT geneRanks)
{
	unsigned int nEdges_Masked, i, gene1IDX, gene2IDX, rank1, rank2;
	nEdges_Masked = 0;
	for (i=0; i<net->nEdge; i++) {
		gene1IDX = net->gene1IDX.v[i];
		gene2IDX = net->gene2IDX.v[i];
		if ((gene1IDX || gene2IDX)!=0) { // when both geneIDX are zero, this edge is disabled.
			rank1 = geneRanks.v[gene1IDX];
			rank2 = geneRanks.v[gene2IDX];
			if ( (rank1 || rank2)!=0 ) { // "either gene" is selected in the subnetwork and with rank information (>=1)
				// disable this edge
				net->gene1IDX.v[i] = 0;
				net->gene2IDX.v[i] = 0;
				nEdges_Masked++;
			}
		}
	}
	return nEdges_Masked;
}
void copy_NET(NET* dst, NET src)
{
	free_NET( dst );
	init_NET( dst );
	dst->nGene = src.nGene;
	dst->nEdge = src.nEdge;
	create_VEC_UINT( src.gene1IDX.n, &dst->gene1IDX );
	copy_VEC_UINT( &dst->gene1IDX, src.gene1IDX );
	create_VEC_UINT( src.gene2IDX.n, &dst->gene2IDX );
	copy_VEC_UINT( &dst->gene2IDX, src.gene2IDX );
	create_VEC_FLOAT( src.w.n, &dst->w );
	copy_VEC_FLOAT( &dst->w, src.w );
}
void read_NET_GENEID(char* file, NET* net, LIST_STRING list_geneids)
{
	FILE* stream;
	unsigned int nEdge=0, idxGene1, idxGene2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			idxGene1 = lookup_LIST_STRING( token1, list_geneids );
			if ( idxGene1 != -1 ) {
				token2 = strtok( NULL, sep_tab );
				idxGene2 = lookup_LIST_STRING( token2, list_geneids );
				if ( idxGene2 != -1 ) {
					token3 = strtok( NULL, sep_tab );
					if ( token3 != NULL ) {
						weight = (float)atof(token3);
						nEdge++;
						addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
						addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
						addnumber_VEC_FLOAT( weight, &net->w );
					}
				}
			}
		}
	}
	fclose(stream);
	net->nEdge = nEdge;
	net->nGene = list_geneids.n;
}
void read_NET_GENEIDXBY1(char* file, NET* net, unsigned int nGene)
{
	FILE* stream;
	unsigned int nEdge=0, idxGene1, idxGene2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1)-1;
			token2 = strtok( NULL, sep_tab );
			if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2)-1;
			token3 = strtok( NULL, sep_tab );
			if ( token3 != NULL ) weight = (float)atof(token3);
			if ( token1!=NULL && token2!=NULL && token3!=NULL && idxGene1<nGene && idxGene2<nGene ) {
				nEdge++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				addnumber_VEC_FLOAT( weight, &net->w );
			}
		}
	}
	fclose(stream);
	net->nEdge = nEdge;
	net->nGene = nGene;
}
void read_NET_GENEIDXBY0(char* file, NET* net, unsigned int nGene)
{
	FILE* stream;
	unsigned int nEdge=0, idxGene1, idxGene2;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	float weight;
	char sep_tab[]   = "\t";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1);
			token2 = strtok( NULL, sep_tab );
			if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2);
			token3 = strtok( NULL, sep_tab );
			if ( token3 != NULL ) weight = (float)atof(token3);
			if ( token1!=NULL && token2!=NULL && token3!=NULL && idxGene1<nGene && idxGene2<nGene ) {
				nEdge++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				addnumber_VEC_FLOAT( weight, &net->w );
			}
		}
	}
	fclose(stream);
	net->nEdge = nEdge;
	net->nGene = nGene;
}
void read_nEdges_Of_NET_GENEIDXBY0(char* file, NET* net, unsigned int nGene, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted)
{
	FILE* stream;
	unsigned int nEdge=0, idxGene1, idxGene2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) && nEdge<nEdges ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1);
			token2 = strtok( NULL, sep_tab );
			if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2);
			token3 = strtok( NULL, sep_tab );
			if ( token3 != NULL ) weight = (float)atof(token3);
			if ( token1!=NULL && token2!=NULL && token3!=NULL && idxGene1<nGene && idxGene2<nGene ) {
				if (rangeWeight.v!=NULL)
					if (weight>rangeWeight.v[1] || weight<rangeWeight.v[0]) continue;
				nEdge++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				if (loadUnweighted)
					addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
				else
					addnumber_VEC_FLOAT( weight, &net->w );
			}
		}
	}
	fclose(stream);
	net->nEdge = nEdge;
	net->nGene = nGene;
}
// read only the first two columns (nodes indexes)
void read_nEdges_Of_NET_GENEIDXBY0_UnweightedNetwork(char* file, NET* net, unsigned int nGene, unsigned int nEdges)
{
    FILE* stream;
    unsigned int nEdge=0, idxGene1, idxGene2;
    float weight;
    char line[MAX_LINE_LENGTH], *token1, *token2;
    char sep_tab[]   = "\t";
    init_NET( net );
    if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
    while( !feof( stream ) && nEdge<nEdges ) {
        if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
            token1 = strtok( line, sep_tab );
            if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1);
            token2 = strtok( NULL, sep_tab );
            if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2);
            if ( token1!=NULL && token2!=NULL && idxGene1<nGene && idxGene2<nGene ) {
                nEdge++;
                addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
                addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
                addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
            }
        }
    }
    fclose(stream);
    net->nEdge = nEdge;
    net->nGene = nGene;
}
void read_nEdges_Of_NET_GENEIDXBY1(char* file, NET* net, unsigned int nGene, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted)
{
	FILE* stream;
	unsigned int nEdge=0, idxGene1, idxGene2;
	float weight;
	char line[MAX_LINE_LENGTH], *token1, *token2, *token3;
	char sep_tab[]   = "\t";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	while( !feof( stream ) && nEdge<nEdges ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
			token1 = strtok( line, sep_tab );
			if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1)-1;
			token2 = strtok( NULL, sep_tab );
			if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2)-1;
			token3 = strtok( NULL, sep_tab );
			if ( token3 != NULL ) weight = (float)atof(token3);
			if ( token1!=NULL && token2!=NULL && token3!=NULL && idxGene1<nGene && idxGene2<nGene ) {
				if (rangeWeight.v!=NULL)
					if (weight>rangeWeight.v[1] || weight<rangeWeight.v[0]) continue;
				nEdge++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				if (loadUnweighted)
					addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
				else
					addnumber_VEC_FLOAT( weight, &net->w );
			}
		}
	}
	fclose(stream);
	net->nEdge = nEdge;
	net->nGene = nGene;
}
// read only the first two columns (nodes indexes)
void read_nEdges_Of_NET_GENEIDXBY1_UnweightedNetwork(char* file, NET* net, unsigned int nGene, unsigned int nEdges)
{
    FILE* stream;
    unsigned int nEdge=0, idxGene1, idxGene2;
    float weight;
    char line[MAX_LINE_LENGTH], *token1, *token2;
    char sep_tab[]   = "\t";
    init_NET( net );
    if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
    while( !feof( stream ) && nEdge<nEdges ) {
        if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL) {
            token1 = strtok( line, sep_tab );
            if ( token1 != NULL ) idxGene1 = (unsigned int)atoi(token1)-1;
            token2 = strtok( NULL, sep_tab );
            if ( token2 != NULL ) idxGene2 = (unsigned int)atoi(token2)-1;
            if ( token1!=NULL && token2!=NULL && idxGene1<nGene && idxGene2<nGene ) {
                nEdge++;
                addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
                addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
                addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
            }
        }
    }
    fclose(stream);
    net->nEdge = nEdge;
    net->nGene = nGene;
}

/*
mule graph format:
  A graph is specified in a text file as follows:
  - The first line contains some identifier for the graph
  - The second line contains two integers, first being the number of
    nodes in the graph, second being the number of edges.
  - Each subsequent two lines correspond to a single node of the graph,
    where
    * The first line contains some identifier for this node
    * The second line contains the indexes of all nodes that share an
      edge for this node. If the graph is undirected, the edge needs to
      be listed only once based on some ordering of node identifiers.
      Note that indexing of nodes starts from 0.
*/
void read_Of_NET_muleGraphFormat(char* file, NET* net)
{
	FILE* stream;
	unsigned int nGene, nEdge, nEdge_count=0, idxGene1, idxGene2, i;
	VEC_UINT v;
	char line[MAX_LINE_LENGTH], *newline, *token1, *token2, *token3;
	char sep_tab[]   = " ";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	fgets( line, MAX_LINE_LENGTH, stream ); // 1st line is this graph name
	fgets( line, MAX_LINE_LENGTH, stream ); // 2nd line is #gene and #edge
	token1 = strtok( line, sep_tab );
	if ( token1 != NULL ) nGene = (unsigned int)atoi(token1);
	token2 = strtok( NULL, sep_tab );
	if ( token2 != NULL ) nEdge = (unsigned int)atoi(token2);
	for (idxGene1=0; idxGene1<nGene; idxGene1++) {
		fgets( line, MAX_LINE_LENGTH, stream ); // this line is node label, skip it.
		fgets( line, MAX_LINE_LENGTH, stream ); remove_newlinechars(line); newline = trim_whitespace(line); // this line is a list of indexes of neighoured nodes. index starts from 0.
		if (strlen(newline)==0) continue; // this indicates this gene doesn't have neighbour genes. So skip this gene.
		init_VEC_UINT( &v );
		read_string2VEC_UINT( newline, &v );
		for (i=0; i<v.n; i++) {
			idxGene2 = v.v[i];
			if ( idxGene2<nGene ) {
				nEdge_count++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
			}
		}
		free_VEC_UINT( &v );
	}
	fclose(stream);
	if (nEdge != nEdge_count) {
		printf("\nWarning: %s: #supposed_edge=%u, #actual_edge=%u", file, nEdge, nEdge_count);
		fprintf(stderr, "\nWarning: %s: #supposed_edge=%u, #actual_edge=%u", file, nEdge, nEdge_count);
	}
	net->nEdge = nEdge;
	net->nGene = nGene;
}
// "excluded_genes" is a binary vector where 1 means this gene is excluded, 0 means this gene is used for analysis.
void read_Of_NET_muleGraphFormat_withExcludedGenes(char* file, VEC_UINT excluded_genes,
	unsigned int excludeEdgesOfDirectNeighborGenes, NET* net)
{
	FILE* stream;
	unsigned int nGene, nEdge, nEdge_count=0, idxGene1, idxGene2, i;
	VEC_UINT v;
	char line[MAX_LINE_LENGTH], *newline, *token1, *token2, *token3;
	char sep_tab[]   = " ";
	init_NET( net );
	if( (stream = fopen( file, "r" )) == NULL ) { errorfile( file, "read" ); exit(0); }
	fgets( line, MAX_LINE_LENGTH, stream ); // 1st line is this graph name
	fgets( line, MAX_LINE_LENGTH, stream ); // 2nd line is #gene and #edge
	token1 = strtok( line, sep_tab );
	if ( token1 != NULL ) nGene = (unsigned int)atoi(token1);
	token2 = strtok( NULL, sep_tab );
	if ( token2 != NULL ) nEdge = (unsigned int)atoi(token2);
	for (idxGene1=0; idxGene1<nGene; idxGene1++) {
		fgets( line, MAX_LINE_LENGTH, stream ); // this line is node label, skip it.
		fgets( line, MAX_LINE_LENGTH, stream ); remove_newlinechars(line); newline = trim_whitespace(line); // this line is a list of indexes of neighoured nodes. index starts from 0.
		if (strlen(newline)==0) continue; // this indicates this gene doesn't have neighbour genes. So skip this gene.
		if (excluded_genes.v[idxGene1]==1) continue; // this gene "idxGene1" is excluded from network analysis. So don't load it to the network.
		init_VEC_UINT( &v );
		read_string2VEC_UINT( newline, &v );
		for (i=0; i<v.n; i++) {
			idxGene2 = v.v[i];
			if ( idxGene2<nGene ) {
				if (excluded_genes.v[idxGene2]==1) continue; // this gene "idxGene2" is excluded from network analysis. So don't load it to the network.
				if (excludeEdgesOfDirectNeighborGenes==EXCLUDE_EDGES_OF_DIRECT_NEIGHBORS) { // remove direct neighbors if two genes' indexes are adjacent.
					if ( ((idxGene1+1)==idxGene2) || ((idxGene2+1)==idxGene1) ) continue;
				}
				nEdge_count++;
				addnumber_VEC_UINT( idxGene1, &net->gene1IDX );
				addnumber_VEC_UINT( idxGene2, &net->gene2IDX );
				addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
			}
		}
		free_VEC_UINT( &v );
	}
	fclose(stream);
	if (nEdge < nEdge_count) { // it's supposed that nEdge_count == nEdge; if there are excluded genes, then nEdge should be nEdge >= nEdge_count.
		printf("\nWarning: %s: #supposed_edge=%u, #actual_edge=%u\n", file, nEdge, nEdge_count);
		fprintf(stderr, "Warning: %s: #supposed_edge=%u, #actual_edge=%u\n", file, nEdge, nEdge_count);
	}
	net->nEdge = nEdge_count;
	net->nGene = nGene;
}
void write_NET_GENEIDXBY0(char* file, NET net)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (net.gene1IDX.v!=NULL && net.gene2IDX.v!=NULL) {
		for (i=0; i<net.nEdge; i++)
			if ((net.gene1IDX.v[i] || net.gene2IDX.v[i])!=0) // when both geneIDX are zero, this edge is disabled.
				fprintf(stream, "%u\t%u\t%g\n", net.gene1IDX.v[i], net.gene2IDX.v[i], net.w.v[i] );
	}
	fclose(stream);
}
void write_NET_GENEIDXBY1(char* file, NET net)
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (net.gene1IDX.v!=NULL && net.gene2IDX.v!=NULL) {
		for (i=0; i<net.nEdge; i++)
			if ((net.gene1IDX.v[i]!=0 || net.gene2IDX.v[i])!=0) // when both geneIDX are zero, this edge is disabled.
				fprintf(stream, "%u\t%u\t%g\n", net.gene1IDX.v[i]+1, net.gene2IDX.v[i]+1, net.w.v[i] );
	}
	fclose(stream);
}
void write_NET_GENEID(char* file, NET net, LIST_STRING list_geneids)
{
	FILE* stream;
	unsigned int i, gene1IDX, gene2IDX, nGene;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	if (net.gene1IDX.v!=NULL && net.gene2IDX.v!=NULL) {
		nGene = list_geneids.n;
		for (i=0; i<net.nEdge; i++) {
			gene1IDX = net.gene1IDX.v[i];
			gene2IDX = net.gene2IDX.v[i];
			if ( ((gene1IDX||gene2IDX)!=0) && (gene1IDX<nGene && gene2IDX<nGene) ) // when both geneIDX are zero, this edge is disabled.
				fprintf(stream, "%s\t%s\t%g\n", list_geneids.string[gene1IDX], list_geneids.string[gene2IDX], net.w.v[i] );
		}
	}
	fclose(stream);
}

/*---------------------------------
    Routines for TENSOROFNETS
-----------------------------------*/
void init_TENSOROFNETS(TENSOROFNETS* t)
{
	t->nNet = 0;
	t->nGene = 0;
	t->nets = NULL;
	init_LIST_STRING( MAXCHAR_ID, &t->netsID );
}
void create_multiple_TENSOROFNETS(unsigned int nTensor, TENSOROFNETS** t)
{
	unsigned int i;
	if (nTensor==0) {*t=NULL; return;}
	*t = (TENSOROFNETS*)malloc( nTensor*sizeof(TENSOROFNETS) );
	if ( *t == NULL ) erroralloc("TENSOROFNETS",nTensor);
	for (i=0; i<nTensor; i++) {
		init_TENSOROFNETS( &((*t)[i]) );
	}
}
void free_TENSOROFNETS(TENSOROFNETS* t)
{
	unsigned int i;
	if (t->nets!=NULL) {
		for (i=0; i<t->nNet; i++)
			free_NET( &t->nets[i] );
		free( t->nets );
	}
	free_LIST_STRING( &t->netsID );
}
void addNULLNET_2_TENSOROFNETS(TENSOROFNETS* t)
{
	unsigned int nNet;
	NET *tmp;
	if (t==NULL) nNet=1;
	else nNet = t->nNet + 1;
	tmp = (NET*)realloc( t->nets, nNet*sizeof(NET) );
	if ( tmp == NULL ) erroralloc("NET", nNet);
	t->nNet = nNet;
	t->nets = tmp;
	init_NET( &t->nets[nNet-1] );
}
/* Before call, make sure t->nets==NULL, and t->netsIDX, t->nNet and t->nGene are already set values */
void batchCreateNULLNET_2_TENSOROFNETS(TENSOROFNETS* t)
{
	unsigned int i;
	if (t->nets!=NULL) {
		fprintf(stderr, "batchCreate NULL NETs error: already exist nets\n");
		exit( -1 );
	}
	t->nets = (NET*)malloc( t->nNet*sizeof(NET) );
	for (i=0; i<t->nNet; i++) init_NET( &t->nets[i] );
}
void copy_TENSOROFNETS(TENSOROFNETS* dst, TENSOROFNETS src)
{
	unsigned int i;
	free_TENSOROFNETS( dst );
	init_TENSOROFNETS( dst );
	dst->nGene = src.nGene;
	dst->nNet = src.nNet;
	copy_LIST_STRING( &dst->netsID, src.netsID );
	batchCreateNULLNET_2_TENSOROFNETS( dst );
	for (i=0; i<dst->nNet; i++)
		copy_NET( &dst->nets[i], src.nets[i] );
}
unsigned int totalEdges_Of_TENSOROFNETS(TENSOROFNETS t)
{
	unsigned int totalEdges, i, j, gene1IDX, gene2IDX;
	NET* net;
	totalEdges = 0;
	for (j=0; j<t.nNet; j++) {
		net = &t.nets[j];
		for (i=0; i<net->nEdge; i++) {
			gene1IDX = net->gene1IDX.v[i];
			gene2IDX = net->gene2IDX.v[i];
			if ((gene1IDX || gene2IDX)!=0) totalEdges++; // when both geneIDX are zero, this edge is disabled.
		}
	}
	return totalEdges;
}

/* suppose t->netsIDX is already assigned and other
   fields of t are initialized, before call*/
void read_TENSOROFNETS_GENEIDXBY0( TENSOROFNETS* t, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("%u,",i+1); fflush( stdout );
		}
		read_NET_GENEIDXBY0(netFile, &t->nets[i], t->nGene);
	}
	if (!mute) {
		printf("\n");
	}
}
void read_TENSOROFNETS_GENEIDXBY1(TENSOROFNETS* t, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute)
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("%u,",i+1); fflush( stdout );
		}
		read_NET_GENEIDXBY1(netFile, &t->nets[i], t->nGene);
	}
	if (!mute) {
		printf("\n");
	}
}
void read_nEdges_Of_TENSOROFNETS_GENEIDXBY0( TENSOROFNETS* t, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_nEdges_Of_NET_GENEIDXBY0(netFile, &t->nets[i], t->nGene, nEdges, rangeWeight, loadUnweighted); // it could be 'read_NET_GENEIDXBY0' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void read_nEdges_Of_TENSOROFNETS_GENEIDXBY0_UnweightedNetwork( TENSOROFNETS* t, unsigned int nEdges, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_nEdges_Of_NET_GENEIDXBY0_UnweightedNetwork(netFile, &t->nets[i], t->nGene, nEdges); // it could be 'read_NET_GENEIDXBY1' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void read_nEdges_Of_TENSOROFNETS_GENEIDXBY1( TENSOROFNETS* t, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_nEdges_Of_NET_GENEIDXBY1(netFile, &t->nets[i], t->nGene, nEdges, rangeWeight, loadUnweighted); // it could be 'read_NET_GENEIDXBY1' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void read_nEdges_Of_TENSOROFNETS_GENEIDXBY1_UnweightedNetwork( TENSOROFNETS* t, unsigned int nEdges, LIST_STRING netsID, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_nEdges_Of_NET_GENEIDXBY1_UnweightedNetwork(netFile, &t->nets[i], t->nGene, nEdges); // it could be 'read_NET_GENEIDXBY1' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void read_TENSOROFNETS_muleGraphFormat( TENSOROFNETS* t, LIST_STRING netsID, char* prefixfile,
	char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_Of_NET_muleGraphFormat(netFile, &t->nets[i]); // it could be 'read_NET_GENEIDXBY1' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void read_TENSOROFNETS_muleGraphFormat_withExcludedGenes( TENSOROFNETS* t, LIST_STRING netsID,
	VEC_UINT excluded_genes, unsigned int excludeEdgesOfDirectNeighborGenes, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute )
{
	unsigned int i;
	char netFile[MAXCHAR];
	batchCreateNULLNET_2_TENSOROFNETS( t );
	for (i=0; i<t->nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, netsID.string[i], suffixfile);
		if (!mute) {
			printf("(%s,",t->netsID.string[i]); fflush( stdout );
		}
		read_Of_NET_muleGraphFormat_withExcludedGenes(netFile, excluded_genes, excludeEdgesOfDirectNeighborGenes, &t->nets[i]); // it could be 'read_NET_GENEIDXBY1' as well
		if (!mute) {
			printf("%u),",t->nets[i].nEdge); fflush( stdout );
		}
	}
	if (!mute) {
		printf("\n");
	}
}
void write_TENSOROFNETS_GENEIDXBY0(TENSOROFNETS t, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute)
{
	unsigned int i;
	char netFile[MAXCHAR];
	for (i=0; i<t.nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, t.netsID.string[i], suffixfile);
		//printf("\n\t%s",netFile); fflush( stdout );
		write_NET_GENEIDXBY0(netFile, t.nets[i]);
	}
	if (!mute) {
		printf("\n");
	}
}
void write_TENSOROFNETS_GENEIDXBY1(TENSOROFNETS t, char* prefixfile, char* suffixfile, char* netspath, unsigned int mute)
{
	unsigned int i;
	char netFile[MAXCHAR];
	for (i=0; i<t.nNet; i++) {
		sprintf(netFile, "%s/%s%s%s", netspath, prefixfile, t.netsID.string[i], suffixfile);
		//printf("\n\t%s",netFile); fflush( stdout );
		write_NET_GENEIDXBY1(netFile, t.nets[i]);
	}
	if (!mute) {
		printf("\n");
	}
}
void load_TENSOROFNETS_GENEIDXBY0( unsigned int nGene, LIST_STRING netsID, TENSOROFNETS* t,
					               char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_TENSOROFNETS_GENEIDXBY0( t, netsID, prefixFile, suffixFile, netsPath, mute );
}
void load_TENSOROFNETS_GENEIDXBY1( unsigned int nGene, LIST_STRING netsID, TENSOROFNETS* t,
					               char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_TENSOROFNETS_GENEIDXBY1( t, netsID, prefixFile, suffixFile, netsPath, mute );
}
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY0( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 VEC_FLOAT rangeWeight, unsigned int loadUnweighted, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_nEdges_Of_TENSOROFNETS_GENEIDXBY0( t, nEdges, rangeWeight, loadUnweighted, netsID, prefixFile, suffixFile, netsPath, mute );
}
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY0_UnweightedNetwork( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_nEdges_Of_TENSOROFNETS_GENEIDXBY0_UnweightedNetwork( t, nEdges, netsID, prefixFile, suffixFile, netsPath, mute );
}
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY1( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 VEC_FLOAT rangeWeight, unsigned int loadUnweighted, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_nEdges_Of_TENSOROFNETS_GENEIDXBY1( t, nEdges, rangeWeight, loadUnweighted, netsID, prefixFile, suffixFile, netsPath, mute );
}
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY1_UnweightedNetwork( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nGene = nGene;
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_nEdges_Of_TENSOROFNETS_GENEIDXBY1_UnweightedNetwork( t, nEdges, netsID, prefixFile, suffixFile, netsPath, mute );
}
void fill_diagonal_ones_Of_TENSOROFNETS( TENSOROFNETS* t )
{
	unsigned int i, j;
	NET *net;
	for (i=0; i<t->nNet; i++) {
		net = &t->nets[i];
		for (j=0; j<t->nGene; j++) {
			addnumber_VEC_UINT( j, &net->gene1IDX );
        	addnumber_VEC_UINT( j, &net->gene2IDX );
        	addnumber_VEC_FLOAT( 1.0, &net->w ); // unweighted edge whose weight is 1.0
			net->nEdge++;
		}
	}
}
void load_TENSOROFNETS_muleGraphFormat( LIST_STRING netsID, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_TENSOROFNETS_muleGraphFormat( t, netsID, prefixFile, suffixFile, netsPath, mute );
	t->nGene = t->nets[0].nGene;
}
void load_TENSOROFNETS_muleGraphFormat_withExcludedGenes( LIST_STRING netsID, VEC_UINT excluded_genes, unsigned int excludeEdgesOfDirectNeighborGenes,
	TENSOROFNETS* t, char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute)
{
	unsigned int nNet=netsID.n;
	init_TENSOROFNETS( t );
	t->nNet = nNet;
	copy_LIST_STRING( &t->netsID, netsID );
	read_TENSOROFNETS_muleGraphFormat_withExcludedGenes( t, netsID, excluded_genes, excludeEdgesOfDirectNeighborGenes, prefixFile, suffixFile, netsPath, mute );
	t->nGene = t->nets[0].nGene;
}
void print_nEdges_Of_TENSOROFNETS( TENSOROFNETS t, FILE* stream )
{
	unsigned int i;
	fprintf(stream,"\n#Edges loaded in the tensor of %u networks:\n", t.nNet );
	for (i=0; i<t.nNet; i++)
		fprintf(stream,"(%s,%u)",t.netsID.string[i],t.nets[i].nEdge);
	fprintf(stream,"\n\n");
}

/*---------------------------------
  Tensor Routines for a single TENSOROFNETS
-----------------------------------*/
/* y = x*T*x, each element of x corresponds to a node of the net */
void xNode_mul_TENSOROFNETS_mul_xNode(TENSOROFNETS t, VEC_DOUBLE x, VEC_DOUBLE* y)
{
	unsigned int i, j, gene1IDX, gene2IDX;
	float weight;
	double sum;
	for (i=0; i<t.nNet; i++) {
		sum = 0;
		for (j=0; j<t.nets[i].nEdge; j++) {
			gene1IDX = t.nets[i].gene1IDX.v[j];
			gene2IDX = t.nets[i].gene2IDX.v[j];
			weight = t.nets[i].w.v[j];
			if ((gene1IDX || gene2IDX)!=0) // when both geneIDX are zero, this edge is disabled.
				sum += weight * x.v[gene1IDX] * x.v[gene2IDX];
				if (gene1IDX != gene2IDX)
					sum += weight * x.v[gene2IDX] * x.v[gene1IDX];
		}
		y->v[i] = sum;
//		y->v[i] = 2*sum; /* nodeA->nodeB, and nodeB->A, so double sum of edges' weights */
	}
}
/* z = y*T*x, each element of y corresponds to a net; each element of x corresponds to a node of the net */
void yNet_mul_TENSOROFNETS_mul_xNode(TENSOROFNETS t, VEC_DOUBLE y, VEC_DOUBLE x, VEC_DOUBLE* z)
{
	unsigned int i, j, gene1IDX, gene2IDX;
	float weight;
	double vy;
	zeros_VEC_DOUBLE( z );
	for (i=0; i<t.nNet; i++) {
		vy = y.v[i];
		for (j=0; j<t.nets[i].nEdge; j++) {
			gene1IDX = t.nets[i].gene1IDX.v[j];
			gene2IDX = t.nets[i].gene2IDX.v[j];
			weight = t.nets[i].w.v[j];
			if ((gene1IDX || gene2IDX)!=0) { // when both geneIDX are zero, this edge is disabled.
				z->v[gene1IDX] += weight * vy * x.v[gene2IDX];
				if (gene1IDX != gene2IDX)
					z->v[gene2IDX] += weight * vy * x.v[gene1IDX];
			}
		}
	}
}

/*---------------------------
  Routines for Densities Computation
---------------------------*/
void init_DENSITIES( DENSITIES* d )
{
	d->maxGene = 0;
	d->nNet = 0;
	init_VEC_UINT( &d->genesIDX );
	d->sumOfWeights = NULL;
	d->densities = NULL;
}
/* create and assign the fixed fields ".nGene", ".nNet", ".netsIDX" and ".netsIDX" */
void create_DENSITIES( TENSOROFNETS t, unsigned int maxGene, DENSITIES* d )
{
	unsigned int i;
	d->maxGene = maxGene;
	d->nNet = t.nNet;
	create_VEC_UINT( maxGene, &d->genesIDX ); // initialized to zeros in "create_VEC_UINT". They will be assigned later in function "getPattern_byCriterion"
	d->sumOfWeights = (VEC_FLOAT*)malloc( d->nNet*sizeof(VEC_FLOAT) );
	if ( d->sumOfWeights == NULL ) erroralloc("VEC_FLOAT",d->nNet);
	for (i=0; i<d->nNet; i++ ) create_VEC_FLOAT( maxGene, &d->sumOfWeights[i] ); // initialized to zeros in "create_VEC_FLOAT"
	d->densities = (VEC_DOUBLE*)malloc( d->nNet*sizeof(VEC_DOUBLE) );
	if ( d->densities == NULL ) erroralloc( "VEC_DOUBLE", d->nNet );
	for (i=0; i<d->nNet; i++ ) create_VEC_DOUBLE( maxGene, &d->densities[i] ); // initialized to zeros in "create_VEC_DOUBLE"
}
void free_DENSITIES( DENSITIES* d )
{
	unsigned int i;
	free_VEC_UINT( &d->genesIDX );
	if (d->sumOfWeights!=NULL) {
		for (i=0; i<d->nNet; i++)
			free_VEC_FLOAT( &d->sumOfWeights[i] );
		free( d->sumOfWeights );
	}
	if (d->densities!=NULL) {
		for (i=0; i<d->nNet; i++)
			free_VEC_DOUBLE( &d->densities[i] );
		free( d->densities );
	}
}
void write_DENSITIES( DENSITIES d, char* file )
{
	FILE* stream;
	unsigned int i, j;
	if( (stream = fopen( file, "w" )) == NULL ) { errorfile( file, "write" ); exit(0); }
	fprintf( stream, "DENSITIES structure\n" );
	fprintf( stream, ".maxGene=%u\n.nNet=%u\n", d.maxGene, d.nNet );
	fprintf( stream, ".genesIDX(%u)=[", d.genesIDX.n );
	for (i=0; i<d.genesIDX.n; i++) fprintf( stream, " %u", d.genesIDX.v[i]+1 );
	fprintf( stream, "]\n");
	fprintf( stream, ".sumOfWeights(%u x %u)=\n", d.nNet, d.maxGene );
	for (i=0; i<d.nNet; i++) {
		fprintf( stream, "\tnet %u (%u)=[", i+1, d.sumOfWeights[i].n );
		for (j=0; j<d.sumOfWeights[i].n; j++) fprintf( stream, " %.3f", d.sumOfWeights[i].v[j] );
		fprintf( stream, "]\n" );
	}
	fprintf( stream, ".densities(%u x %u)=\n", d.nNet, d.maxGene );
	for (i=0; i<d.nNet; i++) {
		fprintf( stream, "\tnet %u (%u)=[", i+1, d.densities[i].n );
		for (j=0; j<d.densities[i].n; j++) fprintf( stream, " %.6g", d.densities[i].v[j] );
		fprintf( stream, "]\n" );
	}
	fclose( stream );
}
void append_DENSITIES( DENSITIES d, char* file, unsigned int maxGene )
{
	FILE* stream;
	unsigned int i, j;
	if (maxGene>d.maxGene) maxGene=d.maxGene;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	for (j=1; j<maxGene; j++)
		for (i=0; i<d.nNet; i++)
			if ((j==(maxGene-1)) && (i==(d.nNet-1))) // the last item to write
				fprintf( stream, "(%u,%u,%.3g)\n", j+1, i+1, d.densities[i].v[j] ); // (iGene,iNet,density)
			else
				fprintf( stream, "(%u,%u,%.3g)\t", j+1, i+1, d.densities[i].v[j] ); // (iGene,iNet,density)
	fclose( stream );
}
// For a module with 5 genes and 3 networks, format is as below,
// 2-5,3 0.7 0.7 0.7 0.8 0.8 0.8 0.9 0.9 0.9 1.0 1.0 1.0
// This example means:
// d(gene-2,net-1)=0.7, d(gene-2,net-2)=0.7, d(gene-2,net-3)=0.7
// d(gene-3,net-1)=0.8, d(gene-3,net-2)=0.8, d(gene-3,net-3)=0.8
// d(gene-4,net-1)=0.9, d(gene-4,net-2)=0.9, d(gene-4,net-3)=0.9
// d(gene-5,net-1)=1.0, d(gene-5,net-2)=1.0, d(gene-5,net-3)=1.0
void append_simple_DENSITIES( DENSITIES d, char* file, unsigned int maxGene )
{
	FILE* stream;
	unsigned int i, j;
	if (maxGene>d.maxGene) maxGene=d.maxGene;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	fprintf( stream, "2-%u,%u\t", maxGene, d.nNet );
	for (j=1; j<maxGene; j++)
		for (i=0; i<d.nNet; i++)
			if ((j==(maxGene-1)) && (i==(d.nNet-1))) // the last item to write
				fprintf( stream, "%.3g\n", d.densities[i].v[j] ); // (iGene,iNet,density)
			else
				fprintf( stream, "%.3g\t", d.densities[i].v[j] ); // (iGene,iNet,density)
	fclose( stream );
}
void zeros_sumOfWeightsAndDensities_Of_DENSITIES( DENSITIES* d )
{
	unsigned int i;
	for (i=0; i<d->nNet; i++) {
		zeros_VEC_FLOAT( &d->sumOfWeights[i] );
		zeros_VEC_DOUBLE( &d->densities[i] );
	}
}
/* Core function to calculate weights' sum of subnetworks formed by
   top-ranking genes. It sums the weights of the edges in these sub-
   networks, which are used to calculate their heaviness.

   "sumOfWeights" is already allocated with space of "maxGene" FLOAT, and
   is initialized to be zeros. It should meet "sumOfWeights->n==maxGene".
   The number of edges in these subnetworks will be returned and
   saved to this variable.

   "maxGene" genes are selected to calculate their subnetworks'
   number of edges (which is further used for computing densities).

   "geneRanks" is a UINT vector with ranking information of selected
   genes. Its size is "#allGenes". Only selected genes (its number is
   "maxGene") have non-zeros values in "geneRanks" and the rest of genes
   have zero values in "geneRanks". These non-zero values are the ranking
   values (starting from 1, to "maxGene") of selecte genes. */
void get_sumOfWeights_Of_NET_bySelectedGenes(NET net, VEC_UINT geneRanks, VEC_FLOAT* sumOfWeights)
{
	unsigned int i, gene1IDX, gene2IDX, rank1, rank2;
	float weight;
	for (i=0; i<net.nEdge; i++) {
		gene1IDX = net.gene1IDX.v[i];
		gene2IDX = net.gene2IDX.v[i];
		if (gene1IDX==gene2IDX) continue;
		weight = net.w.v[i];
		if ((gene1IDX || gene2IDX)!=0) { // when both geneIDX are zero, this edge is disabled.
			rank1 = geneRanks.v[gene1IDX];
			rank2 = geneRanks.v[gene2IDX];
			if ( (rank1 && rank2)!=0 ) { // both genes are selected and with rank information (>=1)
				sumOfWeights->v[_max(rank1,rank2)-1] += weight;
			}
		}
	}
	for (i=0; i<(sumOfWeights->n-1); i++) sumOfWeights->v[i+1] += sumOfWeights->v[i];
}
/* Core function to calculate edges' sum of subnetworks formed by
   top-ranking genes. It counts the number of edges with weights>=cutoff in these sub-
   networks, which are used to calculate their densities.

   "sumOfWeights" is already allocated with space of "maxGene" FLOAT, and
   is initialized to be zeros. It should meet "sumOfWeights->n==maxGene".
   The number of edges in these subnetworks will be returned and
   saved to this variable.

   "maxGene" genes are selected to calculate their subnetworks'
   number of edges (which is further used for computing densities).

   "geneRanks" is a UINT vector with ranking information of selected
   genes. Its size is "#allGenes". Only selected genes (its number is
   "maxGene") have non-zeros values in "geneRanks" and the rest of genes
   have zero values in "geneRanks". These non-zero values are the ranking
   values (starting from 1, to "maxGene") of selecte genes. */
void get_sumOfEdges_Of_unweightedNET_bySelectedGenes(NET net, VEC_UINT geneRanks, float weight_cutoff, VEC_FLOAT* sumOfWeights)
{
	unsigned int i, gene1IDX, gene2IDX, rank1, rank2;
	float weight;
	for (i=0; i<net.nEdge; i++) {
		gene1IDX = net.gene1IDX.v[i];
		gene2IDX = net.gene2IDX.v[i];
		weight = net.w.v[i];
		if ((gene1IDX || gene2IDX)!=0) { // when both geneIDX are zero, this edge is disabled.
			rank1 = geneRanks.v[gene1IDX];
			rank2 = geneRanks.v[gene2IDX];
			if ( (rank1 && rank2)!=0 ) { // both genes are selected and with rank information (>=1)
				if (weight>=weight_cutoff)
					sumOfWeights->v[_max(rank1,rank2)-1]++;
			}
		}
	}
	for (i=0; i<(sumOfWeights->n-1); i++) sumOfWeights->v[i+1] += sumOfWeights->v[i];
}
void assign_geneIDX_Of_DENSITIES_by_XSORTI(VEC_UINT xsorti, DENSITIES* d)
{
	unsigned int i;
	for (i=0; i<d->maxGene; i++) d->genesIDX.v[i] = xsorti.v[i];
}
/* "geneRanks_cache" is a cache which is allocated spaces before calling.

   "d" is the densities which is allocated spaces before calling. And its fields
   "nGene=maxGene", "nNet", "genesIDX" are already assigned values,
   before calling. Genes ranking information is assigned to the field ".genesIDX"
   of DENSITIES. A TENSOROFNETS determines these fields, and so their assignments
   are put outside of this function. */
void get_DENSITIES_Of_AllNets_inTENSOROFNETS( TENSOROFNETS t, VEC_DOUBLE x, VEC_DOUBLE y,
											  DENSITIES* d, VEC_UINT* xsorti_cache,
											  VEC_UINT* ysorti_cache, VEC_UINT* geneRanks_cache)
{
	unsigned int i, j, maxGene, nNode;
	maxGene = d->maxGene;

	/* sort weights of genes and nets: x and y */
	initINDEX_VEC_UINT( t.nGene, xsorti_cache );
	quicksortidx_dec_VEC_DOUBLE( x, xsorti_cache );
	initINDEX_VEC_UINT( t.nNet, ysorti_cache );
	quicksortidx_dec_VEC_DOUBLE( y, ysorti_cache );

	/* get densities of subnetworks  formed by the top-'maxGene' ranking genes */
	assign_geneIDX_Of_DENSITIES_by_XSORTI( *xsorti_cache, d ); // assign ranks of top-ranking "maxGene" genes to "DENSITIES.genesIDX".

	/* initialize "geneRanks_cache" by assigning ranks to top-ranking "maxGene" genes */
	zeros_VEC_UINT( geneRanks_cache );
	for (i=0; i<maxGene; i++) geneRanks_cache->v[d->genesIDX.v[i]] = i+1; // assign ranks (1:maxGene) to top-ranking "maxGene" genes
	/* initialize "d_cache.densities" and "d_cache.sumOfWeights" for all nets */
	zeros_sumOfWeightsAndDensities_Of_DENSITIES( d );
	/* calculate and fill "sumOfWeights"
	   traverse each net in the tensor to get their #edges of subnetworks formed by top-ranking genes. */
	for (i=0; i<t.nNet; i++)
		get_sumOfWeights_Of_NET_bySelectedGenes( t.nets[i], *geneRanks_cache, &d->sumOfWeights[i] );
	/* calculate and fill densities by using "sumOfWeights" obtained above */
	for (i=0; i<maxGene; i++) { // visit subnetwork formed by top-ranking "i+1" genes
		nNode = i+1;
		if (nNode==1) continue; // a subnetwork with only one gene does not have density (default to be zero)
		for (j=0; j<d->nNet; j++) d->densities[j].v[i] = (double)d->sumOfWeights[j].v[i]*2/(nNode*nNode-nNode);
	}
}
unsigned int getNumberOfDenseNets( DENSITIES d, unsigned int nGene, double minDensity, unsigned int mute )
{
	unsigned int i, iGene=nGene-1, nNetdense=0;
	if (nGene>d.maxGene) {
		if (!mute) {
			printf( "In function 'getNumberOfDenseNets', nGene(%u)>d.maxGene(%u)\n", nGene, d.maxGene ); fflush( stdout );
		}
		return nNetdense;
	}
	for (i=0; i<d.nNet; i++)
		if (d.densities[i].v[iGene]>=minDensity) nNetdense++;
	return nNetdense;
}


unsigned int removeHeavyFrequentEdges_Of_TENSOROFNETS(TENSOROFNETS* t, float threshold_weight, float threshold_frequency, char* removedEdgesFile, BOOL force_write )
{
	MATRIX_UINT frequencies;
	unsigned int i, j, gene1IDX, gene2IDX, sumEdges;
	BOOL fileExist;
	FILE* stream;
	char sep[]="\t";
	float weight;
	unsigned int threshold_freq=(unsigned int)(threshold_frequency*t->nNet);
	if (threshold_freq==0) {
		fprintf( stderr, "Error (removeHeavyFrequentEdges_Of_TENSOROFNETS): threshold_frequency=%g is too low, such that (%g x %u)==0\nExit.\n", threshold_frequency, threshold_frequency, t->nNet );
		exit(-1);
	}
	// first scan to get how frequent each heavy edge is
	create_MATRIX_UINT( t->nGene, t->nGene, &frequencies );
	for (i=0; i<t->nNet; i++) {
		for (j=0; j<t->nets[i].nEdge; j++) {
			gene1IDX = t->nets[i].gene1IDX.v[j];
			gene2IDX = t->nets[i].gene2IDX.v[j];
			weight = t->nets[i].w.v[j];
			if ((gene1IDX || gene2IDX)!=0) // when both geneIDX are zero, this edge is disabled.
				if (weight>=threshold_weight) {
					frequencies.mat[gene1IDX][gene2IDX]++;
					frequencies.mat[gene2IDX][gene1IDX]++;
				}
		}
	}
	// second scan to remove those heavy frequent edges
	sumEdges=0;
	//fileExist = file_exists( removedEdgesFile );
	if (!force_write) {
		for (i=0; i<t->nNet; i++) {
			for (j=0; j<t->nets[i].nEdge; j++) {
				gene1IDX = t->nets[i].gene1IDX.v[j];
				gene2IDX = t->nets[i].gene2IDX.v[j];
				weight = t->nets[i].w.v[j];
				if ((gene1IDX || gene2IDX)!=0) // when both geneIDX are zero, this edge is disabled.
					if (frequencies.mat[gene1IDX][gene2IDX]>=threshold_freq) { // disable this heavy frequent edge
						t->nets[i].gene1IDX.v[j] = 0;
						t->nets[i].gene2IDX.v[j] = 0;
						sumEdges++;
					}
			}
		}
	} else {
		if( (stream = fopen( removedEdgesFile, "w" )) == NULL ) { errorfile( removedEdgesFile, "write" ); exit(-1); }
		for (i=0; i<t->nNet; i++) {
			for (j=0; j<t->nets[i].nEdge; j++) {
				gene1IDX = t->nets[i].gene1IDX.v[j];
				gene2IDX = t->nets[i].gene2IDX.v[j];
				weight = t->nets[i].w.v[j];
				if ((gene1IDX || gene2IDX)!=0) // when both geneIDX are zero, this edge is disabled.
					if (frequencies.mat[gene1IDX][gene2IDX]>=threshold_freq) { // disable this heavy frequent edge
						t->nets[i].gene1IDX.v[j] = 0;
						t->nets[i].gene2IDX.v[j] = 0;
						sumEdges++;
						fprintf( stream, "%u%s%u%s%g%s%s\n", gene1IDX+1, sep, gene2IDX+1, sep, weight, sep, t->netsID.string[i] ); // format: gene1_index \t gene2_index \t weight \t network_ID
					}
			}
		}
		fclose(stream);
	}
	free_MATRIX_UINT( &frequencies );
	return sumEdges;
}
