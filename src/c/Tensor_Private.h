#ifndef TENSOR_PRIVATE_H
#define TENSOR_PRIVATE_H

#include "UtilsOfTensor.h"
#include <string.h>
/*----------------------------------------------
----------------------------------------------
 ------    Parameters of Tensor Method ------
----------------------------------------------
----------------------------------------------*/

/*-- DATA OPTIONS --*/
//#define SIMULATION_DATA
#define REAL_DATA

/*-- METHOD OPTIONS --*/
//#define DEBUG

/*-- CONSTANTS FOR DATA --*/
#define FILE_GENESID "gene_ids"
#define PREFIX_DATAFILE ""
#define DELIMTERS_SELECTEDDATAFILE "\t"
#ifdef SIMULATION_DATA
	#define INPUT_TENSOROFNETS_GENEIDXBY0
	#define OUTPUT_TENSOROFNETS_GENEIDXBY0
#endif
#ifdef REAL_DATA
	#define INPUT_TENSOROFNETS_GENEIDXBY1
	#define OUTPUT_TENSOROFNETS_GENEIDXBY1
#endif
#define RANGE_WEIGHT_ALL "ALL"
#define RANGE_WEIGHT_DEFAULT RANGE_WEIGHT_ALL
#define REMOVE_HEAVYFREQUENTEDGES_NONE "NONE"
#define REMOVE_HEAVYFREQUENTEDGES_DEFAULT REMOVE_HEAVYFREQUENTEDGES_NONE
#define FREQUENCY_OF_REMOVE_HEAVYFREQUENTEDGES_DEFAULT 0.4

/*-- CONSTANTS FOR RESULTS --*/
#define N_SETTING 3
#define N_SETTING_LP 2
#define E_ZERO 1e-8
#define E_ZERO_LP 1e-5
#define MAX_EXCHANGE_CONTINUOUS_PSEUDOPATTERN 5
#define MAX_CONTINUOUS_PSEUDOPATTERN 20
#define MAX_EXCHANGE_CONTINUOUS_NO_PATTERN N_SETTING+1


/*----------------------------------------------
----------------------------------------------
  -----------  Data structure -----------
----------------------------------------------
----------------------------------------------*/
typedef struct _parms { /* a set of parameters */
	int sparsity; /* parameter "sparsity" (maximal number of non-zero elements in the vector) of problem formulation */
	double alpha; /* parameter "alpha" (with different values) of problem formulation */
	int howtoInit_xy; // determine if we use randomized values to initialize vectors x and y: 1 indicates use rand, 0 indicates use vector ones.
	double p; /* parameter "p" (with different values) of problem formulation */
	double q; /* parameter "q" (with different values) of problem formulation */
	double a; /* parameters of problem formulation */
	double b; /* parameter "b" (with different values) of problem formulation */
	double h; /* parameter "h" (with different values) of problem formulation */
} PARMS;

typedef struct _parms_setting { /* multiple sets of parameters*/
		unsigned int n_setting;
		PARMS* settings;
} PARMS_SETTING;

typedef struct _pattern {
	unsigned int nGene; // number of selected genes, the subnetwork whom they form in those "netsIDX" are dense
	unsigned int nNet; // number of selected nets
	VEC_UINT genesIDX; // idx of selected genes whose subnetworks in "netsIDX" are dense (above threshold). they are in the sequence of ranking from top to low.
	VEC_UINT netsIDX; // idx of selected nets
	VEC_FLOAT sumOfWeights; // "sumOfWeights" of "netsIDX"
	VEC_DOUBLE densities; // densities of "netsIDX"
} PATTERN;

enum MASK_STRATEGY { // mask strategies are implemented in function "mask_strategy" of file "TensorUtils.c"
	MASKSTRATEGY_ERROR = 0,
	MASKSTRATEGY_EDGES_PATTERN, // "EDGES_PATTERN": masking edges in the genes and networks contained in the pattern
	MASKSTRATEGY_EDGES_ALLNETS, // "EDGES_ALLNETS": masking edges which connect genes contained in the pattern, across all networks
	MASKSTRATEGY_GENES // "GENES": masking edges which are adjacent to the genes contained in the pattern, across all networks
};


/*----------------------------------------------
----------------------------------------------
     -----------  Routines -----------
----------------------------------------------
----------------------------------------------*/
unsigned int check_OVERLAPPATTERNCHOOSE(char* choose);
enum MASK_STRATEGY checkname_MASK_STRATEGY(char* mask_strategy_name);

void init_PARMS( PARMS* p );
void print_PARMS(FILE* stream, PARMS p, char* prefix_String);
unsigned int read_PARMS( PARMS* p, char* filename );
void assign_PARMS_valueset1( PARMS* p );
void assign_PARMS_valueset2( PARMS* p );
void assign_PARMS_valueset3( PARMS* p );
void init_PARMS_SETTING( PARMS_SETTING* p );
void create_PARMS_SETTING( unsigned int n_setting, PARMS_SETTING* p );
void free_PARMS_SETTING( PARMS_SETTING* p );
void print_PARMS_SETTING(FILE* stream, PARMS_SETTING p, char* prefix_String);

void init_PATTERN( PATTERN* p );
void create_PATTERN( unsigned int nGene, unsigned int nNet, PATTERN* p);
void free_PATTERN( PATTERN* p );
double minDensity_inPattern( PATTERN p );
void write_PATTERN_succinct( PATTERN p, char* file );
void write_genes_Of_PATTERN( PATTERN p, char* file );
void write_datasets_Of_PATTERN( PATTERN p, char* file );
void read_PATTERN_succinct_fromString( char* string_pattern, PATTERN *p );
void getPattern_byCriterion1( TENSOROFNETS t, unsigned int minGene, unsigned int minNet,
							 double minDensity, DENSITIES d, PATTERN *p, unsigned int mute );
void getPattern_byCriterion2( TENSOROFNETS t, unsigned int minGene, unsigned int minNet,
							  double minDensity, DENSITIES d, PATTERN *p, unsigned int mute );
void getPattern_byCriterion3( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double zeroThreshold, DENSITIES d, PATTERN *p );
void getPattern_byCriterion4( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double zeroThreshold, DENSITIES d, PATTERN *p );
void getPattern_byCriterionLP( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double px, double py, double zeroThreshold, DENSITIES d, PATTERN *p );
unsigned int mask_strategy( TENSOROFNETS* t, PATTERN p, enum MASK_STRATEGY strategy_code, VEC_UINT* geneRanks_cache );
unsigned int mask_genesWeights( VEC_DOUBLE* x, PATTERN p );
unsigned int mask_Patterns_from_File( TENSOROFNETS* t, enum MASK_STRATEGY strategy, char* patternFile, unsigned int mute );
unsigned int mask_genesWeights_from_patternFile( VEC_DOUBLE* x, char* patternFile, unsigned int mute );

#endif
