#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/timeb.h>
#include "UtilsOfTensor.h"
#include "Tensor_Private.h"
#include "sort.h"
#include "default.h"

/* x and y have been allocated before calling */
// "queryNodeIndex" starts from 1 to nGene.
void TensorAnalysisMultipleNetworks_PowerMethod(TENSOROFNETS t, int queryNodeIndex, int max_nnz, double alpha, unsigned int nIteration,
	VEC_DOUBLE* xInit, VEC_DOUBLE* yInit, VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* xTmp, VEC_DOUBLE* yTmp)
{
	unsigned int i;
	copy_VEC_DOUBLE( x, *xInit );
	copy_VEC_DOUBLE( y, *yInit );
	for ( i=0; i<nIteration; i++ ) {
		/* update x */
		yNet_mul_TENSOROFNETS_mul_xNode( t, *y, *x, xTmp );
//		write_VEC_DOUBLE(*xTmp, "debug.xTmp");
		copy_VEC_DOUBLE( x, *xTmp ); // copy values
		threshold_VEC_DOUBLE(x, max_nnz, alpha, xTmp);
//		write_VEC_DOUBLE(*x, "debug.x_thresh");
		if (queryNodeIndex>=1 && queryNodeIndex<=x->n)
			x->v[queryNodeIndex-1] = max_VEC_DOUBLE( x );
		norm2_VEC_DOUBLE( x );
//		write_VEC_DOUBLE(*x, "debug.x_thresh_norm");
		/* update y */
		xNode_mul_TENSOROFNETS_mul_xNode( t, *x, y );
//		write_VEC_DOUBLE(*y, "debug.y");
		norm2_VEC_DOUBLE( y );
//		write_VEC_DOUBLE(*y, "debug.y_norm");
	}
}

unsigned int iterativeRun_LocalClusters( TENSOROFNETS t, TENSOROFNETS t_backup, PARMS_SETTING p,
	int nPattern_max, unsigned int minGene, unsigned int minNet,
	double minDensity, unsigned int nIteration, enum MASK_STRATEGY mask_strategy_code,
	const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene,
	VEC_DOUBLE *xInit, VEC_DOUBLE *yInit, VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* x_copy,
	VEC_DOUBLE* y_copy, VEC_DOUBLE* xTmp, VEC_DOUBLE* yTmp, int level, unsigned int mute )
{
	int queryNodeIndex;
	PATTERN pattern1, pattern2, pattern3, pattern_pseudo1; /* a pattern found */
	unsigned int i, nPattern, nPattern_pseudo, terminate, found, found_pseudo, totalEdges, totalEdges_masked, n_continuous_pseudopattern, n_continuous_no_pattern;
	char patternFile[MAXCHAR], pseudoPatternFile[MAXCHAR], logFile[MAXCHAR], densitiesOfPatternFile[MAXCHAR];
	char stringCache[MAXCHAR];
	DENSITIES d_cache; /* densities of top-ranking (by SORTIDX) "maxGene" genes in all nets. */
	VEC_UINT xsorti_cache, ysorti_cache, geneRanks_cache, geneRanks_pseudo; /* caches used for calculating densities. assign ranks (1:maxGene) to top-ranking "maxGene" genes; the rest genes are assigned zeros */
//	VEC_DOUBLE xTmp, yTmp; /* "xTmp" and "yTmp" used as intermediate variables of x and y.*/
	PARMS *parm;
	struct timeb t_start, t_end;
	double t_elapse;
	unsigned int currentEdges_masked1, currentEdges_masked2, currentEdges_masked3;
	
	queryNodeIndex = p.settings[0].howtoInit_xy;

	sprintf( patternFile, "%s.PATTERN", resultFile );
	sprintf( pseudoPatternFile, "%s.PSEUDOPATTERN", resultFile );
	sprintf( logFile, "%s.LOG", resultFile );
	sprintf( densitiesOfPatternFile, "%s.DENSITIES", resultFile );
	/****** initialize caches ******/
	create_DENSITIES( t, maxGene, &d_cache ); // "maxGene" is used here. It will be transfered to other functions by "DENSITIES".
	create_VEC_UINT( t.nGene, &geneRanks_cache );
	create_VEC_UINT( t.nGene, &xsorti_cache );
	create_VEC_UINT( t.nNet, &ysorti_cache );
	create_VEC_UINT( t.nGene, &geneRanks_pseudo );
	// create space for temporary variables
	//create_VEC_DOUBLE( t.nGene, &xTmp );
	//create_VEC_DOUBLE( t.nNet, &yTmp );
	/****** iteratively finding patterns ******/
	init_PATTERN( &pattern1 ); init_PATTERN( &pattern2 ); init_PATTERN( &pattern3 );
	init_PATTERN( &pattern_pseudo1 );
	nPattern=0; nPattern_pseudo=0; n_continuous_pseudopattern=0; n_continuous_no_pattern=0;
	totalEdges = totalEdges_Of_TENSOROFNETS( t ); totalEdges_masked = 0;
	terminate=FALSE;
	ftime(&t_start);
	while ( !terminate && nPattern<nPattern_max ) {
		found_pseudo = FALSE;
		for (i=0; i<N_SETTING && nPattern<nPattern_max; i++) { /* use the i-th p to get patterns */
			parm = &p.settings[i];
			/* use which initialization method */
			if (parm->howtoInit_xy==INIT_XY_BY_ONES) {
				ones_VEC_DOUBLE( xInit );
				ones_VEC_DOUBLE( yInit );
			} else if (parm->howtoInit_xy==INIT_XY_BY_RAND) {
			} else if (parm->howtoInit_xy>=1 && parm->howtoInit_xy<=t.nGene) {
				zeros_VEC_DOUBLE( xInit );
				ones_VEC_DOUBLE( yInit );
				xInit->v[parm->howtoInit_xy-1] = 1;
			} else {
				 fprintf( stderr, "Error LpRelax_byMultipleParameters: parm->howtoInit_xy is not %u, %u, or 1<=num<=%u.\nExit.\n", INIT_XY_BY_ONES, INIT_XY_BY_RAND, t.nGene );
				 exit(-1);
			}
			/* update weight vectors x and y by algorithm */
			TensorAnalysisMultipleNetworks_PowerMethod(t, queryNodeIndex, parm->sparsity, parm->alpha, nIteration, xInit, yInit, x, y, xTmp, yTmp);
			copy_VEC_DOUBLE( x_copy, *x );
			copy_VEC_DOUBLE( y_copy, *y );
			/* get densities of all nets across all top-ranking "maxGene" genes. Please note x and y are sorted after completing this function call */
			get_DENSITIES_Of_AllNets_inTENSOROFNETS( t_backup, *x, *y, &d_cache, &xsorti_cache, &ysorti_cache, &geneRanks_cache );
			/* get a pattern by criterion 1 and 2 */
			found=FALSE; currentEdges_masked1=0; currentEdges_masked2=0;
			if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_MORE_NETS)==0)
				getPattern_byCriterion1( t, minGene, minNet, minDensity, d_cache, &pattern1, mute );
			if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_MORE_GENES)==0)
				getPattern_byCriterion2( t, minGene, minNet, minDensity, d_cache, &pattern2, mute );
			if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_BOTH)==0) {
				getPattern_byCriterion1( t, minGene, minNet, minDensity, d_cache, &pattern1, mute );
				getPattern_byCriterion2( t, minGene, minNet, minDensity, d_cache, &pattern2, mute );
			}
			if (strcmp(overlapPatternChoose,OVERLAPPATTERNCHOOSE_NONZEROS)==0) {
				getPattern_byCriterion3( *x, *y, xsorti_cache, ysorti_cache, E_ZERO, d_cache, &pattern3 );
			}
			if ( pattern1.nGene>0 ) { // get a pattern by Criterion 1
				currentEdges_masked1 = mask_strategy( &t, pattern1, mask_strategy_code, &geneRanks_cache );
				if (currentEdges_masked1==0) {
					// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion1': "
					// 	"no edges of the pattern are masked in the tensor, We will use getPattern_byCriterion3 to mask the pattern.\n");
					// if (level >= 2) {
					// 	append_string_2file( logFile, stringCache );
					// }
					// if (!mute) {
					// 	printf( "%s", stringCache ); fflush( stdout );
					// }
				} else {
					n_continuous_pseudopattern = 0;
					found = TRUE;
					nPattern++;
					totalEdges_masked += currentEdges_masked1;
					write_PATTERN_succinct( pattern1, patternFile );
					if (level >= 3) {
						append_simple_DENSITIES( d_cache, densitiesOfPatternFile, pattern1.nGene );
					}
					ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
					sprintf( stringCache, "\n%u pattern (1) found (#Gene=%u,#Net=%u) "
						"(sparsity=%d,alpha=%g,#Iteration=%u) at %g seconds, with "
						"(#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)", nPattern, pattern1.nGene, pattern1.nNet,
						parm->sparsity, parm->alpha, nIteration, t_elapse, totalEdges_masked, totalEdges,
						(double)totalEdges_masked/(double)totalEdges );
					if (level >= 2) {
						append_string_2file( logFile, stringCache );
					}
					if (!mute) {
						printf( "%s", stringCache ); fflush( stdout );
					}
				}
			}
			if ( pattern2.nGene>0 ) { // get a pattern by Criterion 2
				if ( pattern2.nGene!=pattern1.nGene ) {
					currentEdges_masked2 = mask_strategy( &t, pattern2, mask_strategy_code, &geneRanks_cache );
					if (currentEdges_masked2==0) {
						// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion2': "
						// 	"no edges of the pattern are masked in the tensor, We will use getPattern_byCriterion3 "
						// 	"to mask the pattern.\n");
						// if (level >= 2) {
						// 	append_string_2file( logFile, stringCache );
						// }
						// if (!mute) {
						// 	printf( "%s", stringCache ); fflush( stdout );
						// }
					} else {
						found = TRUE;
						n_continuous_pseudopattern = 0;
						nPattern++;
						totalEdges_masked += currentEdges_masked2;
						write_PATTERN_succinct( pattern2, patternFile );
						if (level >= 3) {
							append_simple_DENSITIES( d_cache, densitiesOfPatternFile, pattern2.nGene );
						}
						ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
						sprintf( stringCache, "\n%u pattern (2) found (#Gene=%u,#Net=%u) (sparsity=%d,alpha=%g,#Iteration=%u) "
							"at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)", nPattern, pattern2.nGene,
							pattern2.nNet, parm->sparsity, parm->alpha, nIteration, t_elapse, totalEdges_masked, totalEdges,
							(double)totalEdges_masked/(double)totalEdges );
						if (level >= 2) {
							append_string_2file( logFile, stringCache );
						}
						if (!mute) {
							printf( "%s", stringCache ); fflush( stdout );
						}
					}
					if (currentEdges_masked1!=0 && currentEdges_masked2==0) { found = TRUE; }
				}
			}
			if ( pattern3.nGene>0 ) { // get a pattern by Criterion 3
				currentEdges_masked3 = mask_strategy( &t, pattern3, mask_strategy_code, &geneRanks_cache );
				if (currentEdges_masked3==0) {
					// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion3': "
					// 	"no edges of the pattern are masked in the tensor.\n");
					// if (level >= 2) {
					// 	append_string_2file( logFile, stringCache );
					// }
					// if (!mute) {
					// 	printf( "%s", stringCache ); fflush( stdout );
					// }
					nPattern--;
				} else {
					n_continuous_pseudopattern = 0;
					found = TRUE;
					nPattern++;
					totalEdges_masked += currentEdges_masked3;
					write_PATTERN_succinct( pattern3, patternFile );
	//				append_2VEC_DOUBLE_SetFormat( *x_copy, *y_copy, xyFile );
					if (level >= 3) {
						append_simple_DENSITIES( d_cache, densitiesOfPatternFile, pattern3.nGene );
					}
					ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
					sprintf( stringCache, "\n%u pattern (3) found (#Gene=%u,#Net=%u) (sparsity=%d,alpha=%g,#Iteration=%u) "
						"at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)", nPattern, pattern3.nGene, pattern3.nNet,
						parm->sparsity, parm->alpha, nIteration, t_elapse, totalEdges_masked, totalEdges, (double)totalEdges_masked/(double)totalEdges );
					if (level >= 2) {
						append_string_2file( logFile, stringCache );
					}
					if (!mute) {
						printf( "%s", stringCache ); fflush( stdout );
					}
				}
			}
			free_PATTERN( &pattern1 ); init_PATTERN( &pattern1 );
			free_PATTERN( &pattern2 ); init_PATTERN( &pattern2 );
			free_PATTERN( &pattern3 ); init_PATTERN( &pattern3 );
			if ( found ) { // start another new round of trying all p values, after finding a pattern
				i--; // keep using this set of parameters in the next iteration
			} else {
				getPattern_byCriterion3( *x, *y, xsorti_cache, ysorti_cache, E_ZERO, d_cache, &pattern_pseudo1 );
				if ( pattern_pseudo1.nGene>0 ) {
					nPattern_pseudo++;
					n_continuous_pseudopattern++;

					write_PATTERN_succinct( pattern_pseudo1, pseudoPatternFile );
					totalEdges_masked += mask_strategy( &t, pattern_pseudo1, mask_strategy_code, &geneRanks_cache );
					ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
					sprintf( stringCache, "\n--- %u-th pseudo-pattern (#Gene=%u,#Net=%u) by criterion 3 (sparsity=%d,alpha=%g,#Iteration=%u) "
						"at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)", nPattern_pseudo, pattern_pseudo1.nGene,
						pattern_pseudo1.nNet, parm->sparsity, parm->alpha, nIteration, t_elapse, totalEdges_masked, totalEdges,
						(double)totalEdges_masked/(double)totalEdges );
					if (level >= 2) {
						append_string_2file( logFile, stringCache );
					}
					if (!mute) {
						printf( "%s", stringCache ); fflush( stdout );
					}
					found_pseudo = TRUE;
					if (n_continuous_pseudopattern>=MAX_CONTINUOUS_PSEUDOPATTERN) { // too many continuous pseudo-patterns are generated until now.
						sprintf( stringCache, "\n\nThere are too many %u continuous pseudo-patterns (out of all %u pseudo-patterns) found so far. \n",
							     n_continuous_pseudopattern, nPattern_pseudo);
						if (level >= 2) {
							append_string_2file( logFile, stringCache );
						}
						if (!mute) {
							printf( "%s", stringCache ); fflush( stdout );
						}
						terminate = TRUE;
						break; // break from FOR loops
					}
					if (fmod((float)n_continuous_pseudopattern,(float)MAX_EXCHANGE_CONTINUOUS_PSEUDOPATTERN)==0) { // it is time to change 'a', because many continuous pseudo-patterns are generated until now.
						sprintf( stringCache, "\n\nThere are %u continuous pseudo-patterns (out of all %u pseudo-patterns) found so far. change the set of parameters now.\n",
							     n_continuous_pseudopattern, nPattern_pseudo);
						if (level >= 2) {
							append_string_2file( logFile, stringCache );
						}
						if (!mute) {
							printf( "%s", stringCache ); fflush( stdout );
						}
					} else {
						i--; // keep using this set of parameters in the next iteration
					}
				} else {
					n_continuous_no_pattern++;
					if (fmod((float)n_continuous_no_pattern,(float)(MAX_EXCHANGE_CONTINUOUS_NO_PATTERN))==0) {
						free_PATTERN( &pattern_pseudo1 ); init_PATTERN( &pattern_pseudo1 ); // clear pseudo-pattern memory
						sprintf( stringCache, "\n\nIn the continuous %d time,s even no pseudo-patterns are found.\n", n_continuous_no_pattern);
						if (level >= 2) {
							append_string_2file( logFile, stringCache );
						}
						if (!mute) {
							printf( "%s", stringCache ); fflush( stdout );
						}
						terminate = TRUE;
						break; // break from FOR loops
					} else {
						// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion3': "
						// "no pseudo-patterns are found in the tensor.");
						// if (level >= 2) {
						// 	append_string_2file( logFile, stringCache );
						// }
						// if (!mute) {
						// 	printf( "%s", stringCache ); fflush( stdout );
						// }
					}
				}
				free_PATTERN( &pattern_pseudo1 ); init_PATTERN( &pattern_pseudo1 ); // clear pseudo-pattern found
			}
		} // end of for (i)
	} // end of while
	sprintf(stringCache, "\nTotal patterns found: %u\n\n", nPattern);
	if (level >= 2) {
		append_string_2file( logFile, stringCache );
	}
	if (!mute) {
		printf( "%s", stringCache ); fflush( stdout );
	}
	/* write down the rest of network tensor for later debugging */
	//write_TENSOROFNETS_GENEIDXBY1(t, PREFIX_DATAFILE, SUFFIX_DATAFILE, "./datasets_final_debug");
	//write_VEC_DOUBLE( *x, "./datasets_final_debug/xsort_debug.txt" );
	//write_VEC_DOUBLE( *y, "./datasets_final_debug/ysort_debug.txt" );
	/****** free space ******/
	free_DENSITIES( &d_cache );
	free_VEC_UINT( &xsorti_cache );
	free_VEC_UINT( &ysorti_cache );
	free_VEC_UINT( &geneRanks_cache );
	return nPattern;
}

void localCluster_byMultipleRounds( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p,
                   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity,
                   unsigned int nIteration, enum MASK_STRATEGY mask_strategy_code,
                   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene,
                   unsigned int resume, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute )
{
    char logFile[MAXCHAR], patternFile[MAXCHAR], removedEdgesFile[MAXCHAR], stringCache[MAXCHAR];
    VEC_DOUBLE x, y, xInit, yInit, x_copy, y_copy, xTmp, yTmp; /* x: gene weights; y: net weights */
    unsigned int nGene, nNet, nPattern_old, kPattern, nPattern, nRound, nRemovedEdges;
    struct timeb t_start, t_end;
    double t_elapse;

    sprintf( logFile, "%s.LOG", resultFile );
    sprintf( patternFile, "%s.PATTERN", resultFile );
    sprintf( removedEdgesFile, "%s.RE", resultFile );
    nGene = t->nGene;
    nNet = t->nNet;

	sprintf( stringCache, "/////////////////////////////////////////////////////////////////////////////////\n"
		"//////Search Strategy: all genes and networks; Mask Strategy: %d; overlapPatternChoose: %s //////\n"
		"/////////////////////////////////////////////////////////////////////////////////\n\n",mask_strategy_code,overlapPatternChoose);
    if (level >= 2) {
        append_string_2file( logFile, stringCache );
    }
    if (!mute) {
		printf( "%s", stringCache ); fflush( stdout );
	}
    
    /****** initialize weight vectors ******/
    create_VEC_DOUBLE( t->nGene, &x );
    create_VEC_DOUBLE( t->nNet, &y );
    create_VEC_DOUBLE( t->nGene, &x_copy );
    create_VEC_DOUBLE( t->nNet, &y_copy );
	create_VEC_DOUBLE( t->nGene, &xInit );
    create_VEC_DOUBLE( t->nNet, &yInit );
	create_VEC_DOUBLE( t->nGene, &xTmp );
    create_VEC_DOUBLE( t->nNet, &yTmp );
    ftime(&t_start);
    nPattern_old=UINT_MAX; nPattern=0; nRound=0;
    while (nPattern!=nPattern_old && nPattern<nPattern_max) {
        nRound++;

        sprintf( stringCache, "\n######## %u round to discover patterns ########\n", nRound );
        if (level >= 2) {
            append_string_2file( logFile, stringCache );
        }
        if (!mute) {
			printf( "%s", stringCache ); fflush( stdout );
		}

        // Step 1: remove those heavy and frequent edges before performing tensor analysis
        if (removeHeavyFrequentEdges.v!=NULL) {
            if (nRound==1)
                nRemovedEdges = removeHeavyFrequentEdges_Of_TENSOROFNETS( t, removeHeavyFrequentEdges.v[0], removeHeavyFrequentEdges.v[1], removedEdgesFile, TRUE );
            else
                nRemovedEdges = removeHeavyFrequentEdges_Of_TENSOROFNETS( t, removeHeavyFrequentEdges.v[0], removeHeavyFrequentEdges.v[1], removedEdgesFile, FALSE );
            sprintf( stringCache, "\nRemoved %u edges which are heavy (>=%g) and frequent (>=%g networks)\n", nRemovedEdges, removeHeavyFrequentEdges.v[0], removeHeavyFrequentEdges.v[1] );
            if (level >= 2) {
                append_string_2file( logFile, stringCache );
            }
            if (!mute) {
				printf( "%s", stringCache ); fflush( stdout );
			}
        }
        // Step 2: mask existing patterns
        kPattern = mask_Patterns_from_File( t, mask_strategy_code, patternFile, mute );
        sprintf( stringCache, "\nIn networks tensor, mask %u existing patterns from pattern file '%s'\n", kPattern, patternFile );
        if (level >= 2) {
            append_string_2file( logFile, stringCache );
        }
        if (!mute) {
			printf( "%s", stringCache ); fflush( stdout );
		}
        nPattern_old = nPattern;
        // Step 3: discover new patterns
		nPattern += iterativeRun_LocalClusters( *t, t_backup, p, nPattern_max, minGene, minNet, minDensity, nIteration,
			mask_strategy_code, overlapPatternChoose, resultFile, maxGene, &xInit, &yInit, &x, &y, &x_copy, &y_copy,
			&xTmp, &yTmp, level, mute );
        free_TENSOROFNETS( t );
        init_TENSOROFNETS( t );
        if (nPattern==nPattern_old) break;
        if (nPattern>=nPattern_max) break;
        // Step 4: load original tensor of networks
        copy_TENSOROFNETS( t, t_backup );
        ftime(&t_end); t_elapse = elapseTime( t_start, t_end);

        sprintf( stringCache, "# #TotalPattern=%u found until now. #Pattern=%u found this round. %g seconds taken.\n\n",
            nPattern, nPattern-nPattern_old, t_elapse );
        if (level >= 2) {
            append_string_2file( logFile, stringCache );
        }
        if (!mute) {
			printf( "%s", stringCache ); fflush( stdout );
		}
    }
    ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
    sprintf( stringCache, "########################\n\n#TotalPattern=%u found by %u rounds. %g seconds taken.\n",
             nPattern, nRound, t_elapse );
    if (level >= 2) {
        append_string_2file( logFile, stringCache );
    }
    if (!mute) {
		printf( "%s", stringCache ); fflush( stdout );
	}
    free_VEC_DOUBLE( &x );
    free_VEC_DOUBLE( &y );
    free_VEC_DOUBLE( &x_copy );
    free_VEC_DOUBLE( &y_copy );
	free_VEC_DOUBLE( &xInit );
    free_VEC_DOUBLE( &yInit );
	free_VEC_DOUBLE( &xTmp );
    free_VEC_DOUBLE( &yTmp );
}
