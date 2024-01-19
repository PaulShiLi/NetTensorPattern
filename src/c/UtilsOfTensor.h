#ifndef UTILS_OF_TENSOR_H
#define UTILS_OF_TENSOR_H
#include <sys/timeb.h>
/*---------------------------
  Data Types
---------------------------*/
typedef unsigned int BOOL;
#define TRUE 1
#define FALSE 0

/*---------------------------
  Constants
---------------------------*/
#define MAXCHAR 1024
#define MAX_LINE_LENGTH 200000
#define MAXCHAR_ID 64
#define ZERO_ALGORITHM 1e-20
#define ZERO_EPS 1e-10

#define EXCLUDE_EDGES_OF_DIRECT_NEIGHBORS 1
#define INCLUDE_EDGES_OF_DIRECT_NEIGHBORS 0

/*---------------------------
  Data structure
---------------------------*/
typedef struct _list_string {
	unsigned int n;
	unsigned int MAX_CHAR;
	char** string;
} LIST_STRING;

typedef struct _vector_u {
	unsigned int n;
	unsigned int* v;
} VEC_UINT;

typedef struct _vector_f {
	unsigned int n;
	float* v;
} VEC_FLOAT;

typedef struct _vector_d {
	unsigned int n;
	double* v;
} VEC_DOUBLE;

typedef struct _matrix_i {
	unsigned int nrow;
	unsigned int ncol;
	unsigned int** mat;
} MATRIX_UINT;

typedef struct _matrix_f {
	unsigned int nrow;
	unsigned int ncol;
	float** mat;
} MATRIX_FLOAT;

typedef struct _matrix_d {
	unsigned int nrow;
	unsigned int ncol;
	double** mat;
} MATRIX_DOUBLE;

typedef struct _net {
	unsigned int nEdge;
	unsigned int nGene;
	VEC_UINT gene1IDX;
	VEC_UINT gene2IDX;
	VEC_FLOAT w; // edges' weights for weighted networks
} NET;

typedef struct _tensor_of_nets {
	unsigned int nNet;
	unsigned int nGene;
	NET* nets;
	LIST_STRING netsID; // newly added, all its functions need to be updated.
} TENSOROFNETS;

typedef struct _densities_allnets_of_maxgenes { // densities structure of all nets with the top-k ranking genes
	unsigned int maxGene; // number of maximal selected genes, the subnetwork will be formed from them.
	unsigned int nNet; // number of all nets
	VEC_UINT genesIDX; // index list of maximal selected genes in the sequence of ranking from top to low. size "maxGene"
	VEC_FLOAT* sumOfWeights; // sum of edges' weights of top-"k" genes in all nets. size "nNet x maxGene"
	VEC_DOUBLE* densities; // densities top-"k" genes in all nets. size "nNet x maxGene"
} DENSITIES;

/*---------------------------
  Routines
---------------------------*/
unsigned int IsNumber_DOUBLE(double x);
unsigned int IsNumber_FLOAT(float x);
unsigned int IsFiniteNumber_DOUBLE(double x);
unsigned int IsFiniteNumber_FLOAT(float x);
void fullfile( char* path, char* file );
BOOL file_exists(const char* filename);
void append_string_2file( char* file, char* string );
void exchangeTwoElements_DOUBLE( double v[] );
double power_uintExponent(double base, unsigned int exponent);
double elapseTime(struct timeb t_start, struct timeb t_end);
void erroralloc(char *type, int count);
void errorfile(char *fn, char *operation);
void remove_newlinechars( char* string );
char *trim_whitespace(char *str);
char *strtok_w( char *strToken, const char charDelimit );
int strchr_extract( char *str, char c );
int strcnt( char *str, char key );

void init_LIST_STRING( unsigned int maxchar, LIST_STRING* l );
void create_LIST_STRING(unsigned int n, unsigned int maxchar, LIST_STRING* l);
void free_LIST_STRING(LIST_STRING* l);
unsigned int addstring_LIST_STRING( char* key, LIST_STRING* l );
void copy_LIST_STRING(LIST_STRING *dst, LIST_STRING src);
int lookup_LIST_STRING( char* key, LIST_STRING l);
void read_LIST_STRING( LIST_STRING* l, char* file, unsigned int maxchar );
void read_2LINES_2LIST_STRING( LIST_STRING* l1, LIST_STRING* l2, char* file, char* seps, unsigned int maxchar );
void read_2COLUMNS_2LIST_STRING( LIST_STRING* l1, LIST_STRING* l2, char* file, char* seps, unsigned int maxchar1, unsigned int maxchar2 );
void write_LIST_STRING( LIST_STRING l, char* file );
void write_2LIST_STRING_2LINE( LIST_STRING l1,  LIST_STRING l2, char *seps, char* file);
unsigned int getIndex_Of_LIST_STRING( LIST_STRING allStrings, LIST_STRING selectedStrings, VEC_UINT* selectedIndexes );

void init_VEC_UINT(VEC_UINT* vec);
void create_VEC_UINT(unsigned int n, VEC_UINT* vec);
void createINDEX_VEC_UINT(unsigned int n, VEC_UINT* vec);
void initINDEX_VEC_UINT(unsigned int n, VEC_UINT* vec);
void free_VEC_UINT(VEC_UINT* vec);
void copy_VEC_UINT(VEC_UINT *dst, VEC_UINT src);
void zeros_VEC_UINT(VEC_UINT* vec);
unsigned int max_VEC_UINT(VEC_UINT vec);
unsigned int min_VEC_UINT(VEC_UINT vec);
int lookup_VEC_UINT( unsigned int key, VEC_UINT vec);
void addnumber_VEC_UINT( unsigned int key, VEC_UINT* vec );
void setdiff_VEC_UINT(unsigned int n1, VEC_UINT v2, VEC_UINT* v12);
void union_VEC_UINT(VEC_UINT* dst, VEC_UINT src);
void read_VEC_UINT(VEC_UINT* vec, char* file);
void read_string2VEC_UINT_SetFormat( char* string, VEC_UINT* v );
void write_VEC_UINT(VEC_UINT vec, char* file);
void print_VEC_UINT(FILE* stream, VEC_UINT vec);
void append_VEC_UINT(VEC_UINT vec, char* file);
void read_string2LIST_U_sepTables( char* string, LIST_STRING tokenTable, VEC_UINT* l );
void read_string2VEC_UINT_SetFormat( char* string, VEC_UINT* v );
void read_string2VEC_UINT( char* string, VEC_UINT* v );

void init_VEC_FLOAT(VEC_FLOAT* vec);
void create_VEC_FLOAT(unsigned int n, VEC_FLOAT* vec);
void addnumber_VEC_FLOAT( float key, VEC_FLOAT* vec );
void free_VEC_FLOAT(VEC_FLOAT* vec);
void copy_VEC_FLOAT(VEC_FLOAT *dst, VEC_FLOAT src);
void zeros_VEC_FLOAT(VEC_FLOAT* vec);
void write_VEC_FLOAT(VEC_FLOAT x, char* file);
void put_VEC_FLOAT(FILE* stream, VEC_FLOAT x, char* delim);
void read_string2VEC_FLOAT_SetFormat( char* string, VEC_FLOAT* v );
int read_string2VEC_FLOAT_MATLABFormat( char* src, VEC_FLOAT* dst ); // string is with the MATLAB sequence format

void init_VEC_DOUBLE(VEC_DOUBLE* vec);
void create_VEC_DOUBLE(unsigned int n, VEC_DOUBLE* vec);
void createONES_VEC_DOUBLE(unsigned int n, VEC_DOUBLE* vec);
void free_VEC_DOUBLE(VEC_DOUBLE* vec);
void copy_VEC_DOUBLE(VEC_DOUBLE *dst, VEC_DOUBLE src);
void addnumber_VEC_DOUBLE( double key, VEC_DOUBLE* vec );
void zeros_VEC_DOUBLE(VEC_DOUBLE* vec);
void ones_VEC_DOUBLE(VEC_DOUBLE* vec);
void checkZeros_VEC_DOUBLE(VEC_DOUBLE* vec);
unsigned int isNAN_VEC_DOUBLE(VEC_DOUBLE* vec);
unsigned int isINF_VEC_DOUBLE(VEC_DOUBLE* vec);
void dotadd_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, double a); /* dst <- dst+a*src */
void dotdiv_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src); /* dst <- dst./src */
void dotmul_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src); /* dst <- dst.*src */
void dotpow_VEC_DOUBLE(VEC_DOUBLE* dst, double p); /* dst <- dst.^p */
void dotpow_uintexponent_VEC_DOUBLE(VEC_DOUBLE* dst, unsigned int p); /* dst <- dst.^p */
void dotpowmul_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, double p); /* dst <- (dst.^p).*src */
void dotpowmul_uintexponent_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE src, unsigned int p); /* dst <- (dst.^p).*src */
double sum_VEC_DOUBLE(VEC_DOUBLE vec);
double sum_exponent_of_VEC_DOUBLE(VEC_DOUBLE vec, double exponent);
double sum_uint_exponent_of_VEC_DOUBLE(VEC_DOUBLE vec, unsigned int exponent);
void innerProductAndnorm1_VEC_DOUBLE(VEC_DOUBLE* dst, VEC_DOUBLE* src);
void norm1_VEC_DOUBLE(VEC_DOUBLE* vec);
void norm2_VEC_DOUBLE(VEC_DOUBLE* vec);
double max_VEC_DOUBLE(VEC_DOUBLE* vec);
double min_VEC_DOUBLE(VEC_DOUBLE* vec);
double avg_VEC_DOUBLE(VEC_DOUBLE vec);
double avg_selected_VEC_DOUBLE(VEC_DOUBLE vec, VEC_UINT sortidx, int topK);
double std_VEC_DOUBLE(VEC_DOUBLE vec, double avg);
double std_selected_VEC_DOUBLE(VEC_DOUBLE vec, double avg, VEC_UINT sortidx, int topK);
void threshold_VEC_DOUBLE(VEC_DOUBLE* vec, int max_nnz, double alpha, VEC_DOUBLE* vec_copy);
void sigmoid_VEC_DOUBLE(VEC_DOUBLE* vec, double a);
void sinh_VEC_DOUBLE(VEC_DOUBLE* vec, double b);
void read_VEC_DOUBLE(char* file, VEC_DOUBLE* vec);
void write_VEC_DOUBLE(VEC_DOUBLE x, char* file);
void write_2VEC_DOUBLE(VEC_DOUBLE x, VEC_DOUBLE y, char* file);
void append_2VEC_DOUBLE_SetFormat(VEC_DOUBLE x, VEC_DOUBLE y, char* file);

void init_MATRIX_UINT( MATRIX_UINT* v );
void create_MATRIX_UINT(unsigned int nrow, unsigned int ncol, MATRIX_UINT* v);
void free_MATRIX_UINT( MATRIX_UINT* v );

void init_MATRIX_FLOAT(MATRIX_FLOAT* v);
void create_MATRIX_FLOAT(unsigned int nrow, unsigned int ncol, MATRIX_FLOAT* v);
void create_MATRIX_FLOAT_byDefaultValue(unsigned int nrow, unsigned int ncol, float default_entryValue, float default_diagValue, MATRIX_FLOAT* v);
MATRIX_FLOAT* create_pointer_MATRIX_FLOAT();
void free_MATRIX_FLOAT( MATRIX_FLOAT* v );
void copy_MATRIX_FLOAT( MATRIX_FLOAT src, MATRIX_FLOAT* dst );
void sigmoid_SYM_MATRIX_FLOAT(MATRIX_FLOAT* sym, double a, double b);
float max_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym);
float min_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym);
void scale_SYM_MATRIX_FLOAT(MATRIX_FLOAT *sym);
void average_SYM_MATRIX_FLOAT(MATRIX_FLOAT sym, VEC_UINT selectedRows_index, BOOL count_diagvalues, VEC_FLOAT* avg_vec); /*Before calling, make sure 'avg_vec' is supposed to have no space allocated yet.*/
void read_and_accumulate_SYM_MATRIX_FLOAT_IndexBy1(char* file, MATRIX_FLOAT* net);
void read_and_accumulate_SYM_MATRIX_FLOAT_IndexBy0(char* file, MATRIX_FLOAT* net);
void read_SYM_MATRIX_FLOAT_IndexBy1(char* file, unsigned int n, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net);
void read_SYM_MATRIX_FLOAT_IndexBy0(char* file, unsigned int n, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net);
void read_SYM_MATRIX_FLOAT_byID(char* file, LIST_STRING list_ids, float default_entryValue, float default_diagValue, MATRIX_FLOAT* net);
void write_SYM_MATRIX_FLOAT(char* file, MATRIX_FLOAT net, int format); /* When 'format'==0, print edges with index starting from 0; When 'format'==1, print edges with index starting from 1; when 'format'==-1, print matrix */
void write_SYM_MATRIX_FLOAT_byID(char* file, MATRIX_FLOAT net, LIST_STRING list_ids, int format); /* When 'format'!=-1, print edges; when 'format'==-1, print matrix */
void write_MATRIX_FLOAT_byID(char* file, MATRIX_FLOAT mat, VEC_UINT rowIDs, LIST_STRING columnIDs, char* preString_rowID, char* preString_columnID);
void write_MATRIX_FLOAT_byUINTID(char* file, MATRIX_FLOAT mat, VEC_UINT rowIDs, VEC_UINT columnIDs, char* preString_rowID, char* preString_columnID);

void init_MATRIX_DOUBLE(MATRIX_DOUBLE* v);
void create_MATRIX_DOUBLE(unsigned int nrow, unsigned int ncol, MATRIX_DOUBLE* v);
MATRIX_DOUBLE* create_pointer_MATRIX_DOUBLE();
void free_MATRIX_DOUBLE( MATRIX_DOUBLE* v );
void elementwise_pow_MATRIX_DOUBLE(MATRIX_DOUBLE src, double p, MATRIX_DOUBLE* dst);
void outer_powsum_VEC_DOUBLE2MATRIX_DOUBLE(VEC_DOUBLE src, double p, MATRIX_DOUBLE* dst, VEC_DOUBLE tmp);
void outer_unitpowsum_VEC_DOUBLE2MATRIX_DOUBLE(VEC_DOUBLE src, unsigned int p, MATRIX_DOUBLE* dst, VEC_DOUBLE tmp); // this is more efficient
void write_MATRIX_DOUBLE(MATRIX_DOUBLE mat, char* file, unsigned int format); /* When 'format'!=0, print non-zero entries; when 'format'==0, print matrix */

void init_NET(NET* net);
NET* create_pointer_NET();
void free_NET(NET* net);
void copy_NET(NET* dst, NET src);
unsigned int zerosEdges_Of_NET_bySelectedGenes(NET* net, VEC_UINT geneRanks);
unsigned int zerosAllEdges_Of_NET_BetweenSelectedGenes(NET* net, VEC_UINT geneRanks);
unsigned int zerosAllEdges_Of_NET_AdjacentToSelectedGenes(NET* net, VEC_UINT geneRanks);
void read_NET_GENEIDXBY0(char* file, NET* net, unsigned int nGene);
void read_NET_GENEIDXBY1(char* file, NET* net, unsigned int nGene);
void read_NET_GENEID(char* file, NET* net, LIST_STRING list_geneids);
void read_nEdges_Of_NET_GENEIDXBY0(char* file, NET* net, unsigned int nGene, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted);
void read_nEdges_Of_NET_GENEIDXBY1(char* file, NET* net, unsigned int nGene, unsigned int nEdges, VEC_FLOAT rangeWeight, unsigned int loadUnweighted);
void read_nEdges_Of_NET_GENEIDXBY1_UnweightedNetwork(char* file, NET* net, unsigned int nGene, unsigned int nEdges);
void read_Of_NET_muleGraphFormat(char* file, NET* net);
void read_Of_NET_muleGraphFormat_withExcludedGenes(char* file, VEC_UINT excluded_genes, unsigned int excludeEdgesOfDirectNeighborGenes, NET* net);
void write_NET_GENEIDXBY0(char* file, NET net);
void write_NET_GENEIDXBY1(char* file, NET net);
void write_NET_GENEID(char* file, NET net, LIST_STRING list_geneids);

void init_TENSOROFNETS(TENSOROFNETS* t);
void create_multiple_TENSOROFNETS(unsigned int nTensor, TENSOROFNETS** t);
void free_TENSOROFNETS(TENSOROFNETS* t);
void batchCreateNULLNET_2_TENSOROFNETS(TENSOROFNETS* t);
void copy_TENSOROFNETS(TENSOROFNETS* dst, TENSOROFNETS src);
void load_TENSOROFNETS_GENEIDXBY0( unsigned int nGene, LIST_STRING netsID, TENSOROFNETS* t,
					               char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute);
void load_TENSOROFNETS_GENEIDXBY1( unsigned int nGene, LIST_STRING netsID, TENSOROFNETS* t,
					               char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute);
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY0( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 VEC_FLOAT rangeWeight, unsigned int loadUnweighted, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute);
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY0_UnweightedNetwork( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute);
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY1( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 VEC_FLOAT rangeWeight, unsigned int loadUnweighted, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute);
void load_nEdges_Of_TENSOROFNETS_GENEIDXBY1_UnweightedNetwork( unsigned int nGene, LIST_STRING netsID, unsigned int nEdges,
											 TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute);
void fill_diagonal_ones_Of_TENSOROFNETS( TENSOROFNETS* t );
void load_TENSOROFNETS_muleGraphFormat( LIST_STRING netsID, TENSOROFNETS* t, char* prefixFile,
											 char* suffixFile, char* netsPath, unsigned int mute);
void load_TENSOROFNETS_muleGraphFormat_withExcludedGenes( LIST_STRING netsID, VEC_UINT excluded_genes, unsigned int excludeEdgesOfDirectNeighborGenes,
											 TENSOROFNETS* t, char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute);
void write_TENSOROFNETS_GENEIDXBY0(TENSOROFNETS t, char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute);
void write_TENSOROFNETS_GENEIDXBY1(TENSOROFNETS t, char* prefixFile, char* suffixFile, char* netsPath, unsigned int mute);
void print_nEdges_Of_TENSOROFNETS( TENSOROFNETS t, FILE* stream );
unsigned int totalEdges_Of_TENSOROFNETS(TENSOROFNETS t);
void xNode_mul_TENSOROFNETS_mul_xNode(TENSOROFNETS t, VEC_DOUBLE x, VEC_DOUBLE* y);
void yNet_mul_TENSOROFNETS_mul_xNode(TENSOROFNETS t, VEC_DOUBLE y, VEC_DOUBLE x, VEC_DOUBLE* z);

void init_DENSITIES( DENSITIES* d );
void create_DENSITIES( TENSOROFNETS t, unsigned int maxGene, DENSITIES* d );
void free_DENSITIES( DENSITIES* d );
void write_DENSITIES( DENSITIES d, char* file );
void append_DENSITIES( DENSITIES d, char* file, unsigned int maxGene );
void append_simple_DENSITIES( DENSITIES d, char* file, unsigned int maxGene );
unsigned int getNumberOfDenseNets( DENSITIES d, unsigned int nGene, double minDensity, unsigned int mute );
void get_DENSITIES_Of_AllNets_inTENSOROFNETS( TENSOROFNETS t, VEC_DOUBLE x, VEC_DOUBLE y,
											  DENSITIES* d, VEC_UINT* xsorti_cache,
											  VEC_UINT* ysorti_cache, VEC_UINT* geneRanks_cache);
unsigned int removeHeavyFrequentEdges_Of_TENSOROFNETS(TENSOROFNETS* t, float threshold_weight, float threshold_frequency, char* removedEdgesFile, BOOL force_write );

void get_sumOfWeights_Of_NET_bySelectedGenes(NET net, VEC_UINT geneRanks, VEC_FLOAT* sumOfWeights);
void get_sumOfEdges_Of_unweightedNET_bySelectedGenes(NET net, VEC_UINT geneRanks, float weight_cutoff, VEC_FLOAT* sumOfWeights);

#endif
