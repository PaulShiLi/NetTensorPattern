#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/timeb.h>
#include "UtilsOfTensor.h"
#include "Tensor_Private.h"
#include "Tensor_LocalCluster.h"
#include "default.h"
#include "Tensor_LpRelax.h"


/*
Global Variables
*/
LIST_STRING selectedNetsID; // ID list of selected networks
LIST_STRING selectedNetsAnnotation; // annotation list of selected networks
VEC_UINT selectedNets_INDEX; // index list of selected networks (start from 0)
TENSOROFNETS t, t_backup; // tensor of networks
VEC_FLOAT rangeWeight; // minimum and maximum weight of edges, that are allowd to be loaded to memory, and used in tensor analysis, when minmaxWeight.n=0, it means use "ALL" edges without considering weight's range
VEC_FLOAT removeHeavyFrequentEdges; // thresholds of weight and frequency of the edges that will not be counted in tensor analysis. It is given like "[0.6,0.4]", meaning "remove" edges whose weight>=0.6, and frequency>=%40
VEC_UINT genes_index_excluded; // The indexes of those genes which are excluded in the network
VEC_UINT binary_vec_genes_index_excluded; // The binary vector of those genes which are excluded in the network. Its length is #gene. When a gene is excluded, its value is 1; otherwise, it is 0.
enum MASK_STRATEGY mask_strategy_code;
unsigned int geneTotalNumber=0;

/*
Required Parameters
*/
int howtoInit_xy; // INUM

// Pointers to required parameters
int *p_howtoInit_xy = &howtoInit_xy; // INUM



/*
Optional Parameters
*/
int nPattern_max; // pNUM
unsigned int nIteration; // iNUM
unsigned int nStage;  // sNUM
unsigned int minGene, minNet; // gNUM, nNUM
unsigned int maxGene; // GNUM
double minDensity; // dNUM
char mask_strategy_name[MAXCHAR]; // SSTRING
char overlapPatternChoose[MAXCHAR]; // OSTRING
unsigned int nEdgesLoad; // ENUM

// Pointers to optional parameters
int *p_nPattern_max = &nPattern_max; // pNUM
unsigned int *p_nIteration = &nIteration; // iNUM
unsigned int *p_nStage = &nStage;  // sNUM
unsigned int *p_minGene = &minGene, *p_minNet = &minNet; // gNUM, nNUM
unsigned int *p_maxGene = &maxGene; // GNUM
double *p_minDensity = &minDensity; // dNUM
unsigned int *p_nEdgesLoad = &nEdgesLoad; // ENUM

/*
Flags
*/
unsigned int resume; // r
unsigned int excludeEdgesOfDirectNeighborGenes; // X
unsigned int local;
unsigned int mute;

// Pointers to flags
unsigned int *p_resume = &resume; // r
// succintOutput = FALSE; // c
unsigned int *p_excludeEdgesOfDirectNeighborGenes = &excludeEdgesOfDirectNeighborGenes; // X
unsigned int *p_local = &local;

/*
File Parameters
*/
char *fn_results[MAXCHAR]; // fSTRING | oSTRING
char *suffixdataFile[MAXCHAR]; // FSTRING
char *fn_selectedDatasetsList[MAXCHAR]; // DSTRING
char *path_nets[MAXCHAR]; // NSTRING
char *path_result[MAXCHAR]; // RSTRING

/*
Non-User Parameters
*/
char rangeWeight_string[MAXCHAR]="", removeHeavyFrequentEdges_string[MAXCHAR]=""; // wSTRING, eSTRING
char fn_parms[MAXCHAR]=""; // PSTRING
char fn_excluded_genes[MAXCHAR]=""; // xSTRING
PARMS_SETTING parms_setting;
int level;

void assign_default_params(
) {
	initDefaultVariables();
	minGene = MINGENE_DEFAULT;
	minDensity = MIN_DENSITY_DEFAULT;
	minNet = MINNET_DEFAULT;
	strcpy(mask_strategy_name, MASK_STRATEGY_NAME_DEFAULT);
	strcpy(overlapPatternChoose, OVERLAPPATTERNCHOOSE_DEFAULT);
	strcpy(fn_selectedDatasetsList, "");
	howtoInit_xy = INIT_XY_BY_ONES;

	strcpy(fn_results, PREFIX_RESULTFILE_DEFAULT);
	strcpy(suffixdataFile, SUFFIX_DATAFILE);
	strcpy(path_nets, PATH_NETS_DEFAULT);
	strcpy(path_result, PATH_RESULT_DEFAULT);

	nPattern_max = MAXPATTERN_DEFAULT;
	nIteration = NITERATION_DEFAULT;
	nStage = NSTAGE_DEFAULT;
	maxGene = MAXGENE;
	nEdgesLoad = NEDGES_LOAD_DEFAULT;
	resume = RESUME_LASTRUN_DEFAULT;
	excludeEdgesOfDirectNeighborGenes = DEFAULT_INCLUDE_EDGES;
	local = TRUE;
	mute = FALSE;
	level = 3;

	howtoInit_xy = 0;
	geneTotalNumber = 50;
	strcpy(fn_selectedDatasetsList, "./test_data/smallScale50x20/datasets/selectedDatasets.list");
	strcpy(path_nets, "./test_data/smallScale50x20/datasets");
	strcpy(path_result, "./test_data/runSmallScale50x20/results");
	minGene = 4;
	minNet = 4;
	minDensity = 0.6;
	nIteration = 20;
	strcpy(suffixdataFile, ".sig");
}

int verify_args(
	unsigned int isLocal,
	unsigned int geneTotalNumber,
	unsigned int iStage,
	unsigned int *p_minGene,
	double *p_minDensity,
	unsigned int *p_minNet,
	char mask_strategy_name[MAXCHAR],
	char overlapPatternChoose[MAXCHAR],
	char fn_selectedDatasetsList[MAXCHAR],
	char rangeWeight_string[MAXCHAR],
	char removeHeavyFrequentEdges_string[MAXCHAR],
	int howtoInit_xy,
	int level
) {
	if (isLocal == TRUE && iStage != NSTAGE_DEFAULT) {
		printf("Warning: nStage will not be used in local mode\n");
	}
	// Conditions
	if (geneTotalNumber == 0) {
		printf("Error: geneTotalNumber is 0\n");
		return FALSE;
	}
	// Print mingene
	if (*p_minGene < MINGENE_DEFAULT) {
		printf("Error: minGene {%d} is less than %d\n", *p_minGene, MINGENE_DEFAULT);
		return FALSE;
	}
	if (*p_minDensity > 1 || *p_minDensity <= 0) {
		printf("Error: minDensity {%f} is not in (0,1]\n", *p_minDensity);
		return FALSE;
	}
	if (*p_minNet < MINNET_DEFAULT) {
		printf("Error: minNet {%d} is less than %d\n", *p_minNet, MINNET_DEFAULT);
		return FALSE;
	}
	
	mask_strategy_code = checkname_MASK_STRATEGY(mask_strategy_name);
	if (mask_strategy_code == MASKSTRATEGY_ERROR) {
		printf("Error: mask_strategy_name {%s} is not valid\n", mask_strategy_name);
		return FALSE;
	}
	if (!check_OVERLAPPATTERNCHOOSE(overlapPatternChoose)) {
		printf("Error: overlapPatternChoose {%s} is not valid\n", overlapPatternChoose);
		return FALSE;
	}
	if (strlen(fn_selectedDatasetsList) == 0) {
		printf("Error: fn_selectedDatasetsList is empty\n");
		return FALSE;
	}
	if (strcmp(rangeWeight_string, RANGE_WEIGHT_ALL) == 0 || strlen(rangeWeight_string) == 0) {
		init_VEC_FLOAT( &rangeWeight );
	}
	else {
		read_string2VEC_FLOAT_SetFormat(rangeWeight_string, &rangeWeight);
		if (rangeWeight.n != 2) {
			printf("Error: rangeWeight {%d} does not equal to 2\n", rangeWeight.n);
			return FALSE;
		}
		else if (rangeWeight.v[0] > rangeWeight.v[1]) {
			printf("Error: rangeWeight.v[0] {%d} is greater than rangeWeight.v[1]\n", rangeWeight.v[0], rangeWeight.v[1]);
			return FALSE;
		}
	}
	if (strcmp(removeHeavyFrequentEdges_string, "") == 0 || strlen(removeHeavyFrequentEdges_string)==0) {
		init_VEC_FLOAT( &removeHeavyFrequentEdges );
	}
	else {
		read_string2VEC_FLOAT_SetFormat(removeHeavyFrequentEdges_string, &removeHeavyFrequentEdges);
		if (removeHeavyFrequentEdges.n != 2) {
			printf("Error: removeHeavyFrequentEdges {%d} does not equal to 2\n", removeHeavyFrequentEdges.n);
			return FALSE;
		}
		else if (removeHeavyFrequentEdges.v[0] < 0 || removeHeavyFrequentEdges.v[0] > 1 || removeHeavyFrequentEdges.v[1] < 0 || removeHeavyFrequentEdges.v[1] > 1) {
			printf("Error: removeHeavyFrequentEdges is not valid\n");
			return FALSE;
		}
	}

	if (howtoInit_xy > geneTotalNumber) {
		printf("Error: howtoInit_xy {%d} is greater than geneTotalNumber {%d}\n", howtoInit_xy, geneTotalNumber);
		return FALSE;
	}
	if (howtoInit_xy != INIT_XY_BY_ONES && howtoInit_xy != INIT_XY_BY_RAND && howtoInit_xy != INIT_XY_BY_UNIT && isLocal == FALSE) {
		printf("Error: howtoInit_xy {%d} is not a valid option\n", howtoInit_xy);
		return FALSE;
	}

	if (useGivenResultFilename && strcmp(fn_results, PREFIX_RESULTFILE_DEFAULT) == 0) {
		printf("Error: fn_results is empty\n");
		return FALSE;
	}
	if (level < 1 || level > 4) {
		printf("Error: level {%d} is not valid\n", level);
		return FALSE;
	}
	return TRUE;
}


void printmsg(FILE* stream, PARMS_SETTING p)
{
	fprintf( stream, "\nTensor Computation of Multiple Networks for Frequent Heavy Subgraphs Discovery\n"  );
	fprintf( stream, "\nUsage: localNetworkTensorPattern [options] ...\n"
		             "Options:\n"
					 "\t-h or --help      :show this message and exit\n"
					 "\t-pNUM or --maxPattern=NUM  :number of patterns to be discovered, default(unlimited: %d)\n"
					 "\t-iNUM or --nIteration=NUM  :number of iterations, default(%u)\n"
					 "\t-sNUM or --nStage=NUM      :number of stages, default(%u)\n"
					 "\t-gNUM or --minGene=NUM     :minimum number of genes, default(%u)\n"
					 "\t-GNUM or --maxGene=NUM     :maximum number of genes, default(%u)\n"
					 "\t-nNUM or --minNet=NUM      :minimum number of networks, default(%u)\n"
					 "\t-dNUM or --minDensity=NUM  :minimum density (0<d<=1)  :default(%g)\n"
					 "\t-SSTRING or --maskStrategy=STRING  :%s,%s or %s (default: %s)\n"
					 "\t-OSTRING or --overlapPatternChoose=STRING  :%s,%s, %s or %s (default: %s)\n"
					 "\t-fSTRING or --prefixResultFilename=STRING  :default(%s)\n"
					 "\t-FSTRING or --suffixdataFile=STRING        :default(%s)\n"
					 "\t-DSTRING or --selectedDatasetsListFile=FILE  :required(put in the networks path)\n"
					 "\t-NSTRING or --networksPath=PATH :default(%s)\n"
					 "\t-RSTRING or --resultPath=PATH   :default(%s)\n"
					 "\t-ENUM or --nEdgesLoad=NUM   :max #edges loaded for each network, default(%u)\n"
					 "\t-r or --resume              :resume the last run\n"
					 "\t-c or --succintOutput       :output only PATTERN file"
					 "\t-u or --loadUnweighted      :load networks in the unweighted manner.\n\n"
					 "\t-wSTRING or --rangeWeight=STRING : minimum and maximum weight of edges that can be loaded to memory, default (""%s"")\n"
					 "\t                             could be like ""[0.5,0.8]""\n"
					 "\t-eSTRING or --removeHeavyFrequentEdges=STRING : threshold of weight and frequency of the edge that will not be\n"
					 "\t                                          counted in tensor analysis. If this option is not set, then do\n"
					 "\t                                          NOT remove heavy-frequency-edges; otherwise remove them before\n"
					 "\t                                          tensor analysis. It is given like ""[0.6,0.4]"", meaning ""remove""\n"
					 "\t                                          edges whose weight>=0.6, and frequency>=\%40, default (""%s"")\n"
					 "\t-INUM or --Init_xy=NUM : %u, use random values to initialize vectors x and y;\n"
					 "\t                         %u, use ones to initialize x and y;\n"
					 "\t                         num, where #gene>= num >=1 use the query gene's index to make the unit vector to initialize x;\n"
					 "\t                         default (""%u"")\n"
					 "\t-PSTRING or --ParmsFile=STRING  : parameters file. If not setting, use default parameters.\n"
					 "\t-oSTRING or --ResultFile=STRING : result filename(no path included). If not setting, use default filename(with params).\n"
					 "\t-xSTRING or --excludeGenesIndexesFile=STRING : filename(path must be included).\n"
					 "\t-X or --excludeEdgesOfDirectNeighborGenes    : if two genes are adjacent in genome (their indexes are of difference 1), remove the edge connecting them.\n",
		    MAXPATTERN_DEFAULT,NITERATION_DEFAULT,NSTAGE_DEFAULT,MINGENE_DEFAULT,MAXGENE,MINNET_DEFAULT,MIN_DENSITY_DEFAULT,
		    MASK_STRATEGY_NAME_EDGES_PATTERN,MASK_STRATEGY_NAME_EDGES_ALLNETS,MASK_STRATEGY_NAME_GENES,MASK_STRATEGY_NAME_DEFAULT,
			OVERLAPPATTERNCHOOSE_MORE_NETS,OVERLAPPATTERNCHOOSE_MORE_GENES,OVERLAPPATTERNCHOOSE_BOTH,OVERLAPPATTERNCHOOSE_NONZEROS,OVERLAPPATTERNCHOOSE_DEFAULT,
			PREFIX_RESULTFILE_DEFAULT,SUFFIX_DATAFILE,PATH_NETS_DEFAULT,PATH_RESULT_DEFAULT,NEDGES_LOAD_DEFAULT,RANGE_WEIGHT_DEFAULT,REMOVE_HEAVYFREQUENTEDGES_DEFAULT,
			INIT_XY_BY_RAND, INIT_XY_BY_ONES, INIT_XY_DEFAULT);
	fprintf( stream, "Algorithm predefined parameters, files and directories\n" );
	print_PARMS_SETTING(stream, p, "\t");
	fprintf( stream, "\tmaxGene is %u", MAXGENE );
	fprintf( stream, "\n\tdatasets Path - %s", PATH_NETS_DEFAULT);
	fprintf( stream, "\n\tdatasets filenames prefix and suffix - '%s' and '%s'", PREFIX_DATAFILE, SUFFIX_DATAFILE);
	fprintf( stream, "\n\tresults Path - %s\n\n", PATH_RESULT_DEFAULT);}

void print_current_time( FILE* stream )
{
	time_t curtime;
	struct tm *loctime;
	curtime = time (NULL); /* Get the current time.  */
	loctime = localtime (&curtime); /* Convert it to local time representation.  */
	fputs (asctime (loctime), stream); /* Print out the date and time in the standard format.  */
}

void printparms(FILE* stream, PARMS_SETTING p)
{
	fprintf( stream, "#-----------------------------\n"  );
	fprintf( stream, "# Tensor Computation of Multiple Networks for Frequent Local Cluster Discovery\n"  );
	fprintf( stream, "#------- Input Files -------\n"  );
	fprintf( stream, "# selectedDatasetsListFile: %s\n", fn_selectedDatasetsList );
	fprintf( stream, "# #Genes: %u\n", geneTotalNumber );
	fprintf( stream, "# #Networks: %u\n", selectedNetsID.n );
	if (p.settings[0].howtoInit_xy>=1)
		fprintf( stream, "# Query_gene_index: %d (index starts from 1)\n", p.settings[0].howtoInit_xy );
	if (strlen(fn_excluded_genes)>0)
		fprintf( stream, "# excludeGenesIndexesFile: %s\n", fn_excluded_genes );
	if (excludeEdgesOfDirectNeighborGenes==EXCLUDE_EDGES_OF_DIRECT_NEIGHBORS)
		fprintf( stream, "# exclude edges whose two genes are direct neighbors in genome sequence (their indexes are of difference 1)." );
	fprintf( stream, "# #max_Load_Edges_each_network: %u\n", nEdgesLoad );
	if (loadUnweighted)
		fprintf( stream, "# #loadUnweighted = TRUE (load networks in the unweighted manner)\n" );
	if (rangeWeight.v==NULL) fprintf( stream, "# #range_of_loaded_edges: %s\n", RANGE_WEIGHT_ALL );
	else fprintf( stream, "# #range_of_loaded_edges: [%g,%g]\n", rangeWeight.v[0], rangeWeight.v[1] );
	if (removeHeavyFrequentEdges.v==NULL) fprintf( stream, "# removeHeavyFrequentEdges: %s\n", REMOVE_HEAVYFREQUENTEDGES_NONE );
	else fprintf( stream, "# removeHeavyFrequentEdges: [%g, %g]\n", removeHeavyFrequentEdges.v[0], removeHeavyFrequentEdges.v[1] );
	fprintf( stream, "#----- Input Parameters ------\n"  );
	print_PARMS_SETTING(stream, p, "# ");
	fprintf( stream, "# #MAX_EXCHANGE_CONTINUOUS_PSEUDOPATTERN: %u\n", MAX_EXCHANGE_CONTINUOUS_PSEUDOPATTERN );
	fprintf( stream, "# #MAX_CONTINUOUS_PSEUDOPATTERN: %u\n", MAX_CONTINUOUS_PSEUDOPATTERN );
	fprintf( stream, "# #E_ZERO: %g\n", E_ZERO );
	if (nPattern_max==NPATTERN_UNLIMITED)
		fprintf( stream, "# #Pattern= unlimited (#patterns to be discovered)\n" );
	else
		fprintf( stream, "# #Pattern= %d (#patterns to be discovered)\n", nPattern_max );
	fprintf( stream, "# #Iteration= %u\n", nIteration );
	fprintf( stream, "# maskStrategy= %s\n", mask_strategy_name );
	fprintf( stream, "# overlapPatternChoose= %s\n", overlapPatternChoose );
	if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_NONZEROS)!=0) {
		fprintf( stream, "# minGene= %u (in pattern)\n", minGene );
		fprintf( stream, "# minNet= %u (in pattern)\n", minNet );
		fprintf( stream, "# minDensity= %g\n", minDensity );
	} else {
		fprintf( stream, "   indicating no ""minGene"",""minNet"",""minDensity"" are needed\n" );
	}
	fprintf( stream, "# maxGene= %u (in pattern)\n", maxGene );
	if (howtoInit_xy==INIT_XY_BY_RAND) {
		fprintf( stream, "# howtoInit_xy= random\n" );
	} else if (howtoInit_xy==INIT_XY_BY_ONES) {
		fprintf( stream, "# howtoInit_xy= ones\n" );
	} else if (howtoInit_xy>=1 && howtoInit_xy<=geneTotalNumber) {
		fprintf( stream, "# howtoInit_xy= unit vector of a given query gene (index=%d)\n", howtoInit_xy );
	} else {
		fprintf( stderr, "Error: howtoInit_xy is not %u, %u, or 1<=num<=%u.\nExit.\n", INIT_XY_BY_ONES, INIT_XY_BY_RAND, geneTotalNumber );
		exit( -1 );
	}
	fprintf( stream, "#------- Output File --------\n"  );
	fprintf( stream, "# prefix of resultFile: %s\n", fn_results );
	if (level>=2) fprintf( stream, "# level= %d (level of output)\n", level );
	fprintf( stream, "#-----------------------------\n\n"  );
}

void init_global_variables()
{
	init_PARMS_SETTING( &parms_setting );
	init_LIST_STRING( MAXCHAR_ID, &selectedNetsID );
	init_LIST_STRING( MAXCHAR, &selectedNetsAnnotation );
	init_VEC_UINT( &selectedNets_INDEX );
	init_TENSOROFNETS( &t );
	init_TENSOROFNETS( &t_backup );
	init_VEC_FLOAT( &rangeWeight );
	init_VEC_FLOAT( &removeHeavyFrequentEdges );
	init_VEC_UINT( &genes_index_excluded );
	init_VEC_UINT( &binary_vec_genes_index_excluded );
}

void free_global_variables()
{
	free_PARMS_SETTING( &parms_setting );
	free_LIST_STRING( &selectedNetsID );
	free_LIST_STRING( &selectedNetsAnnotation );
	free_TENSOROFNETS( &t );
	free_TENSOROFNETS( &t_backup );
	free_VEC_FLOAT( &rangeWeight );
	free_VEC_FLOAT( &removeHeavyFrequentEdges );
	free_VEC_UINT( &genes_index_excluded );
	free_VEC_UINT( &binary_vec_genes_index_excluded );
}

int main(int argc, char *argv[])
{
	char parms[MAXCHAR], logFile[MAXCHAR], tmpString[MAXCHAR];
	FILE* stream;
	unsigned int nRemovedEdges, ret, i;

	assign_default_params();
	init_global_variables();

	verify_args(local, geneTotalNumber, nStage, p_minGene, p_minDensity, p_minNet, mask_strategy_name, overlapPatternChoose, fn_selectedDatasetsList, rangeWeight_string, removeHeavyFrequentEdges_string, howtoInit_xy, level);

	create_PARMS_SETTING( N_SETTING, &parms_setting );
	if (strlen(fn_parms)==0) { // use default parameters
		assign_PARMS_valueset1( &parms_setting.settings[0] );
		assign_PARMS_valueset2( &parms_setting.settings[1] );
		assign_PARMS_valueset3( &parms_setting.settings[2] );
	} else { // Two sets of parameters use the same values of parameters defined in the file.
		ret = read_PARMS( &parms_setting.settings[0], fn_parms );
		if (ret==FALSE) { fprintf(stderr, "Terminate accidently.\n"); exit(-1); }
		read_PARMS( &parms_setting.settings[1], fn_parms );
	}

	parms_setting.settings[0].howtoInit_xy = howtoInit_xy;
	parms_setting.settings[1].howtoInit_xy = howtoInit_xy;
	parms_setting.settings[2].howtoInit_xy = howtoInit_xy;

	if ((howtoInit_xy>=1)) { // It's a query gene and local cluster finder.
		sprintf( parms, "Gene%d", howtoInit_xy );
	} else {
		sprintf( parms, "" );
	}

	if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_NONZEROS)!=0) {
		sprintf( parms, "%snEdgeLoad%uminGene%uminNet%uminD%g", parms, nEdgesLoad, minGene, minNet, minDensity );
	} else {
		sprintf( parms, "%snEdgeLoad%u", parms, nEdgesLoad );
	}
	if (removeHeavyFrequentEdges.v!=NULL)
		sprintf( parms, "%sFHS%g~%g", parms, removeHeavyFrequentEdges.v[0], removeHeavyFrequentEdges.v[1] );
	if (rangeWeight.v!=NULL)
		sprintf( parms, "%srangeW%g~%g", parms, rangeWeight.v[0], rangeWeight.v[1] );

	/* get full files with path */
	fullfile( path_result, fn_results );

	if (!useGivenResultFilename)
		strcat( fn_results, parms );

	read_2COLUMNS_2LIST_STRING( &selectedNetsID, &selectedNetsAnnotation, fn_selectedDatasetsList,DELIMTERS_SELECTEDDATAFILE, MAXCHAR_ID, MAXCHAR );
	if (selectedNetsID.n==0) { fprintf(stderr,"\nError: No networks are provided in '%s', exit\n",fn_selectedDatasetsList); exit(-1);}
	
	/* print parms used */
	if (!mute) {
		if ( ! resume ) { /* start a new run */
			printparms( stdout, parms_setting );
		} else { /* resume the last run */
			printparms( stdout, parms_setting );
		}
	}

	#ifdef DEBUG
		if (debug_nGene<geneTotalNumber) geneTotalNumber = debug_nGene;
		printf("\n--------debug (use first %u genes and %u nets)-------\n",geneTotalNumber,selectedNetsID.n);
	#endif

	if (strlen(fn_excluded_genes)>0) {
		read_VEC_UINT( &genes_index_excluded, fn_excluded_genes ); // Note that gene index starts from 1.
		create_VEC_UINT( geneTotalNumber, &binary_vec_genes_index_excluded ); // it is initialized as ZEROs
		for (i=0; i<genes_index_excluded.n; i++) {
			binary_vec_genes_index_excluded.v[genes_index_excluded.v[i]-1] = 1;
		}
		if (!mute) {
			printf("#Genes_to_be_excluded: %u\n", genes_index_excluded.n);
		}
	}

	#ifdef INPUT_TENSOROFNETS_GENEIDXBY0
		load_nEdges_Of_TENSOROFNETS_GENEIDXBY0_UnweightedNetwork( geneTotalNumber, selectedNetsID, nEdgesLoad, &t, PREFIX_DATAFILE, suffixdataFile, path_nets);
			fill_diagonal_ones_Of_TENSOROFNETS( &t );
	#endif
	#ifdef INPUT_TENSOROFNETS_GENEIDXBY1
		load_nEdges_Of_TENSOROFNETS_GENEIDXBY1_UnweightedNetwork( geneTotalNumber, selectedNetsID, nEdgesLoad, &t, PREFIX_DATAFILE, suffixdataFile, path_nets, mute);
			fill_diagonal_ones_Of_TENSOROFNETS( &t );
	#endif

	/* print edges load information to log file */
	if (level >= 2) {
		sprintf( logFile, "%s.LOG", fn_results );
		if( (stream = fopen( logFile, "a" )) == NULL ) { errorfile( logFile, "append" ); exit(0); }
		if (!mute) {
			print_nEdges_Of_TENSOROFNETS( t, stream );
		}
		fclose( stream );
	}

	if (maxGene>t.nGene) maxGene = t.nGene; // maxGene is originally assigned as MAXGENE=200
	if (minGene>=t.nGene || minGene>maxGene) {
		fprintf(stderr,"\nError: minGene(%u) should be <#AllGene(%u) or <=MAXGENE(default %u)\n", minGene, t.nGene, maxGene);
		if (!mute) {
			printmsg(stderr, parms_setting);
		}
		free_global_variables();
		return 0;
	}

	copy_TENSOROFNETS( &t_backup, t );

	if (local==TRUE) {
		printf("Local Cluster\n");
        localCluster_byMultipleRounds( &t, t_backup, parms_setting, nPattern_max, minGene, minNet, minDensity,
            nIteration, mask_strategy_code, overlapPatternChoose, fn_results, maxGene, resume, removeHeavyFrequentEdges, level, mute );
    }
    else {
		printf("Global Cluster\n");
        LpRelax_byMultipleStrategies( &t, t_backup, parms_setting, nPattern_max, minGene, minNet, minDensity,
                nIteration, nStage, mask_strategy_code, overlapPatternChoose, fn_results, maxGene, resume, removeHeavyFrequentEdges, level, mute );
    }

	if (!mute) {
		printf("Free space\n");
	}
	
	free_global_variables();
	if (!mute) {
		printf("Finish.\n");
	}
	return 0;
}
