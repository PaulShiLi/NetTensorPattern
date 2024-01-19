#include <stdio.h>
#include <string.h>
#include "UtilsOfTensor.h"

/*-- DATA OPTIONS --*/
#define SIMULATION_DATA
// #define REAL_DATA


const int INIT_XY_BY_ONES = 0; // ones vector is a vector, all of whose elements are ONE
const int INIT_XY_BY_RAND = -1; // random vector
const int INIT_XY_BY_UNIT = -2; // unit vector is a vector in which there is only one element is ONE, the others are all ZERO
const int INIT_XY_DEFAULT = INIT_XY_BY_ONES;
const int NPATTERN_UNLIMITED = -1;
const int MAXPATTERN_DEFAULT = NPATTERN_UNLIMITED;

const unsigned int NITERATION_DEFAULT = 20;
const unsigned int NSTAGE_DEFAULT = 20;
const unsigned int MINGENE_DEFAULT = 3;
const unsigned int MINNET_DEFAULT = 3;
const unsigned int MAXGENE = 50; // we assume the maximal size of a pattern is less than 50

const double MIN_DENSITY_DEFAULT = 0.6;

const char MASK_STRATEGY_NAME_EDGES_PATTERN[MAXCHAR] = "EDGES_PATTERN"; // NAME OF MASK STRATEGY
const char MASK_STRATEGY_NAME_EDGES_ALLNETS[MAXCHAR] = "EDGES_ALLNETS"; // NAME OF MASK STRATEGY
const char MASK_STRATEGY_NAME_GENES[MAXCHAR] = "GENES"; // NAME OF MASK STRATEGY
char MASK_STRATEGY_NAME_DEFAULT[MAXCHAR]; // default MASK STRATEGY

const char OVERLAPPATTERNCHOOSE_NONZEROS[MAXCHAR] = "PATTERN_WITH_NONZEROS_XY";
const char OVERLAPPATTERNCHOOSE_MORE_NETS[MAXCHAR] = "PATTERN_WITH_MORE_NETS";
const char OVERLAPPATTERNCHOOSE_MORE_GENES[MAXCHAR] = "PATTERN_WITH_MORE_GENES";
const char OVERLAPPATTERNCHOOSE_BOTH[MAXCHAR] = "PATTERN_WITH_BOTH";
char OVERLAPPATTERNCHOOSE_DEFAULT[MAXCHAR];

const unsigned int NEDGES_LOAD_DEFAULT = 1000000;
unsigned int loadUnweighted = FALSE;
const unsigned int RESUME_LASTRUN_DEFAULT = FALSE;
const unsigned int DEFAULT_INCLUDE_EDGES = TRUE;

const char PREFIX_RESULTFILE_DEFAULT[MAXCHAR] = "";
const char PATH_RESULT_DEFAULT[MAXCHAR] = "./results";
const char PATH_DEBUG_DATASETS[MAXCHAR] = "./debug/debug_datasets";
const char PATH_NETS_DEFAULT[MAXCHAR] = "./datasets";

#ifdef SIMULATION_DATA
    const char SUFFIX_DATAFILE[MAXCHAR] = ".sig";
#endif

#ifdef REAL_DATA
    const char SUFFIX_DATAFILE[MAXCHAR] = ".net";
#endif

unsigned int useGivenResultFilename = FALSE; /* default is to use result filename defined by this code. If users provide result filename, then use --ResultFile=FILE to give filename*/
int muleInput=FALSE; // correspond to the option "--muleInput" or "-M"
int muleOutput=FALSE; // correspond to the option "--muleOutput" or "-m"

int initDefaultVariables() {
    strcpy(MASK_STRATEGY_NAME_DEFAULT, MASK_STRATEGY_NAME_EDGES_PATTERN);
    strcpy(OVERLAPPATTERNCHOOSE_DEFAULT, OVERLAPPATTERNCHOOSE_BOTH);
    // strcpy(OVERLAPPATTERNCHOOSE_DEFAULT, OVERLAPPATTERNCHOOSE_NONZEROS);

    return 0;
}