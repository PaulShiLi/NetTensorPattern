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

/*---------------------------------
  Routines for Tong Zhang's problem formulation and algorithm (L_p relaxation)
-----------------------------------*/
void normalize_x( VEC_DOUBLE* x, double p, double a, double b )
{
	unsigned int i;
	double sum1 = sum_exponent_of_VEC_DOUBLE( *x, p );
	double sum2 = sum_uint_exponent_of_VEC_DOUBLE( *x, (unsigned int)b );
	double norm = a*pow(sum1,1/p)+(1-a)*pow(sum2,1/b);
	for (i=0; i<x->n; i++) x->v[i] /= norm;
}
/* "normalize_y" is not efficient. so we use "normalize_y_efficient" instead. */
void normalize_y( VEC_DOUBLE* y, double q )
{
	unsigned int i;
	double sum = sum_uint_exponent_of_VEC_DOUBLE( *y, (unsigned int)q );
	double norm = pow( sum, 1/q );
	for (i=0; i<y->n; i++) y->v[i] /= norm;
}
void normalize_y_efficient( VEC_DOUBLE* y, double norm )
{
	unsigned int i;
	for (i=0; i<y->n; i++) y->v[i] /= norm;
}
/* update x and y by power method, x and y have been allocated space, before calling.
   update x firstly, then update y */
void powerMethod_update_x_first( TENSOROFNETS t, VEC_DOUBLE v, double p, double h, double a, double b,
				  double q, unsigned int nIteration, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
				  VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* xTmp, VEC_DOUBLE* yTmp )
{
	unsigned int i;
	double sum, t_elapse;
	struct timeb t_start, t_end;
	copy_VEC_DOUBLE( x, xInit );
	copy_VEC_DOUBLE( y, yInit );
	for (i=0; i<nIteration; i++) {
		/* update x */
		//ftime(&t_start);
		yNet_mul_TENSOROFNETS_mul_xNode( t, *y, *x, xTmp );
		dotmul_VEC_DOUBLE( x, *xTmp );
		dotdiv_VEC_DOUBLE( x, v );
		dotpow_VEC_DOUBLE( x, 1/h );
		normalize_x( x, p, a, b );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n(%u/%u) update x by %.3g seconds", i+1, nIteration, t_elapse );
		/* update y */
		//ftime(&t_start);
		xNode_mul_TENSOROFNETS_mul_xNode( t, *x, yTmp );
		dotmul_VEC_DOUBLE( y, *yTmp );
		sum = sum_VEC_DOUBLE( *y );
		dotpow_VEC_DOUBLE( y, 1/q );
		normalize_y_efficient( y, pow(sum,1/q) );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n(%u/%u) update y by %.3g seconds", i+1, nIteration, t_elapse );
	}
}
/* update x and y by power method, x and y have been allocated space, before calling.
   update x firstly, then update y.
   modified the order of updating x and y at 12/02/2009 */
void powerMethod( TENSOROFNETS t, VEC_DOUBLE v, double p, double h, double a, double b,
				  double q, unsigned int nIteration, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
				  VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* xTmp, VEC_DOUBLE* yTmp )
{
	unsigned int i;
	double sum, t_elapse;
	struct timeb t_start, t_end;
	copy_VEC_DOUBLE( x, xInit );
	copy_VEC_DOUBLE( y, yInit );
	for (i=0; i<nIteration; i++) {
		/* update y */
		//ftime(&t_start);
		xNode_mul_TENSOROFNETS_mul_xNode( t, *x, yTmp );
		dotmul_VEC_DOUBLE( y, *yTmp );
		sum = sum_VEC_DOUBLE( *y );
		dotpow_VEC_DOUBLE( y, 1/q );
		normalize_y_efficient( y, pow(sum,1/q) );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n(%u/%u) update y by %.3g seconds", i+1, nIteration, t_elapse );
		/* update x */
		//ftime(&t_start);
		yNet_mul_TENSOROFNETS_mul_xNode( t, *y, *x, xTmp );
		dotmul_VEC_DOUBLE( x, *xTmp );
		dotdiv_VEC_DOUBLE( x, v );
		dotpow_VEC_DOUBLE( x, 1/h );
		normalize_x( x, p, a, b );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n(%u/%u) update x by %.3g seconds", i+1, nIteration, t_elapse );
	}
}
void update_v( VEC_DOUBLE x, double p, double h, double a, double b, VEC_DOUBLE* v )
{
	unsigned int i;
	double sum1 = sum_exponent_of_VEC_DOUBLE( x, p );
	double sum2 = sum_exponent_of_VEC_DOUBLE( x, b );
	for ( i=0; i<v->n; i++ )
		v->v[i] = a*pow(sum1,1/p-1)*pow(x.v[i],p-h)/h + (1-a)*pow(sum2,1/b-1)*pow(x.v[i],b-h)/h;
}
void computeEnergy()
{
}
/* get the solution of x and y. Note that x and y have been allocated space and initialized, before calling */
void TensorAnalysisMultipleNetworks_LpRelax( TENSOROFNETS t, PARMS p, unsigned int nIteration,
											 unsigned int nStage, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
											 VEC_DOUBLE* x, VEC_DOUBLE* y )
{
	VEC_DOUBLE v, xTmp, yTmp; /* "xTmp" and "yTmp" used as intermediate variables of x and y; "v" is used in algorithm */
	unsigned int i;
	double t_elapse;
	struct timeb t_start, t_end;
	createONES_VEC_DOUBLE( t.nGene, &v );
	create_VEC_DOUBLE( t.nGene, &xTmp );
	create_VEC_DOUBLE( t.nNet, &yTmp );
	for ( i=0; i<nStage; i++ ) {
		//printf( "\nStage %u/%u (p=%g,h=%g,q=%g,a=%g,b=%g): ", i+1, nStage, p.p, p.h, p.q, p.a, p.b );

		/* fixing v, get x, y */
		//ftime(&t_start);
		powerMethod( t, v, p.p, p.h, p.a, p.b, p.q, nIteration, xInit, yInit, x, y, &xTmp, &yTmp );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n\tStep 'power method': %.3g seconds", t_elapse );

		/* fixing x and y, get v */
		//ftime(&t_start);
		update_v( *x, p.p, p.h, p.a, p.b, &v );
		//ftime(&t_end);
		//t_elapse = elapseTime( t_start, t_end);
		//printf( "\n\tStep 'update v': %.3g seconds", t_elapse );
		/* compute energy */
		//printf( "\n" );
	}
	free_VEC_DOUBLE( &v );
	free_VEC_DOUBLE( &xTmp );
	free_VEC_DOUBLE( &yTmp );
}

/* get the solution of x and y. Note that x and y have been allocated space and initialized, before calling */
unsigned int LpRelax_byMultipleParameters( TENSOROFNETS t, TENSOROFNETS t_backup, PARMS_SETTING p,
				   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity, unsigned int nIteration,
				   unsigned int nStage, enum MASK_STRATEGY mask_strategy_code, const char* overlapPatternChoose,
				   const char* resultFile, unsigned int maxGene, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
				   VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* x_copy, VEC_DOUBLE* y_copy, int level, unsigned int mute )
{
	PATTERN pattern1, pattern2, pattern3, pattern_pseudo1; /* a pattern found */
	unsigned int i, nPattern, nPattern_pseudo, terminate, found, found_pseudo, totalEdges, totalEdges_masked, n_continuous_pseudopattern;
	char patternFile[MAXCHAR], pseudoPatternFile[MAXCHAR], logFile[MAXCHAR], densitiesOfPatternFile[MAXCHAR];
	char stringCache[MAXCHAR];
	DENSITIES d_cache; /* densities of top-ranking (by SORTIDX) "maxGene" genes in all nets. */
	VEC_UINT xsorti_cache, ysorti_cache, geneRanks_cache, geneRanks_pseudo; /* caches used for calculating densities. assign ranks (1:maxGene) to top-ranking "maxGene" genes; the rest genes are assigned zeros */
	PARMS *parm;
	struct timeb t_start, t_end;
	double t_elapse;
	unsigned int currentEdges_masked1, currentEdges_masked2, currentEdges_masked3;

	sprintf( patternFile, "%s.PATTERN", resultFile );
//	sprintf( pseudoPatternFile, "%s.PSEUDOPATTERN", resultFile );
	sprintf( logFile, "%s.LOG", resultFile );
	sprintf( densitiesOfPatternFile, "%s.DENSITIES", resultFile );
	/****** initialize caches ******/
	create_DENSITIES( t, maxGene, &d_cache ); // "maxGene" is used here. It will be transfered to other functions by "DENSITIES".
	create_VEC_UINT( t.nGene, &geneRanks_cache );
	create_VEC_UINT( t.nGene, &xsorti_cache );
	create_VEC_UINT( t.nNet, &ysorti_cache );
	create_VEC_UINT( t.nGene, &geneRanks_pseudo );
	/****** iteratively finding patterns ******/
	init_PATTERN( &pattern1 ); init_PATTERN( &pattern2 ); init_PATTERN( &pattern3 );
	init_PATTERN( &pattern_pseudo1 );
	nPattern=0; nPattern_pseudo=0; n_continuous_pseudopattern=0;
	totalEdges = totalEdges_Of_TENSOROFNETS( t ); totalEdges_masked = 0;
	terminate=FALSE;
	ftime(&t_start);
	while ( !terminate && nPattern<nPattern_max ) {
		found_pseudo = FALSE;
		for (i=0; i<N_SETTING && nPattern<nPattern_max; i++) { /* use the i-th p to get patterns */
			parm = &p.settings[i];
			/* use which initialization method */
			// switch (parm->howtoInit_xy) {
			//  case INIT_XY_BY_RAND:
			// 	break;
			//  case INIT_XY_BY_ONES:
			// 	ones_VEC_DOUBLE( &xInit );
			// 	ones_VEC_DOUBLE( &yInit );
			// 	break;
			//  case INIT_XY_BY_UNIT:
			// 	break;
			//  default:
			// 	 fprintf( stderr, "Error LpRelax_byMultipleParameters: parm->howtoInit_xy is not %u, %u, or %u.\nExit.\n", INIT_XY_BY_ONES, INIT_XY_BY_RAND, INIT_XY_BY_UNIT );
			// 	 exit(-1);
			// }

			int val = parm->howtoInit_xy;
			if (val==INIT_XY_BY_RAND) {
			}
			else if (val==INIT_XY_BY_ONES) {
				ones_VEC_DOUBLE( &xInit );
				ones_VEC_DOUBLE( &yInit );
			}
			else if (val==INIT_XY_BY_UNIT) {
				
			}
			else {
				fprintf( stderr, "Error LpRelax_byMultipleParameters: parm->howtoInit_xy is not %d or %d.\nExit.\n", INIT_XY_BY_ONES, INIT_XY_BY_RAND );
				exit(-1);
			}
			/* update weight vectors x and y by algorithm */
			TensorAnalysisMultipleNetworks_LpRelax( t, *parm, nIteration, nStage, xInit, yInit, x, y );
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
				getPattern_byCriterionLP( *x, *y, xsorti_cache, ysorti_cache, parm->p, parm->q, E_ZERO, d_cache, &pattern3 );
			}
			if ( pattern1.nGene>0 ) { // get a pattern by Criterion 1
				currentEdges_masked1 = mask_strategy( &t, pattern1, mask_strategy_code, &geneRanks_cache );
				if (currentEdges_masked1==0) {
					// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion1': no edges of the pattern are masked in the tensor, We will use getPattern_byCriterion3 to mask the pattern.\n");
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
					sprintf( stringCache, "\n%u pattern (1) found (#Gene=%u,#Net=%u) (p=%g,q=%g,b=%g,h=%g,a=%g,#Iteration=%u,#Stage=%u) at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)",						nPattern, pattern1.nGene, pattern1.nNet, parm->p, parm->q, parm->b, parm->h, parm->a, nIteration, nStage, t_elapse, totalEdges_masked, totalEdges, (double)totalEdges_masked/(double)totalEdges );
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
						// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion2': no edges of the pattern are masked in the tensor, We will use getPattern_byCriterion3 to mask the pattern.\n");
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
						sprintf( stringCache, "\n%u pattern (2) found (#Gene=%u,#Net=%u) (p=%g,q=%g,b=%g,h=%g,a=%g,#Iteration=%u,#Stage=%u) at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)",
								 nPattern, pattern2.nGene, pattern2.nNet, parm->p, parm->q, parm->b, parm->h, parm->a, nIteration, nStage, t_elapse, totalEdges_masked, totalEdges, (double)totalEdges_masked/(double)totalEdges );
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
					// sprintf( stringCache, "\nUnusual pattern obtained from 'getPattern_byCriterion3': no edges of the pattern are masked in the tensor.\n");
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
					sprintf( stringCache, "\n%u pattern (3) found (#Gene=%u,#Net=%u) (p=%g,q=%g,b=%g,h=%g,a=%g,#Iteration=%u,#Stage=%u) at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)",
						nPattern, pattern3.nGene, pattern3.nNet, parm->p, parm->q, parm->b, parm->h, parm->a, nIteration, nStage, t_elapse, totalEdges_masked, totalEdges, (double)totalEdges_masked/(double)totalEdges );
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
				getPattern_byCriterionLP( *x, *y, xsorti_cache, ysorti_cache, parm->p, parm->q, E_ZERO, d_cache, &pattern_pseudo1 );
				if ( pattern_pseudo1.nGene>0 ) {
					nPattern_pseudo++;
					n_continuous_pseudopattern++;
//					write_PATTERN_succinct( pattern_pseudo1, pseudoPatternFile );
					totalEdges_masked += mask_strategy( &t, pattern_pseudo1, mask_strategy_code, &geneRanks_cache );
					ftime(&t_end); t_elapse = elapseTime( t_start, t_end);
					sprintf( stringCache, "\n--- %u-th pseudo-pattern (#Gene=%u,#Net=%u) by criterion 3 (p=%g,q=%g,b=%g,h=%g,a=%g,#Iteration=%u,#Stage=%u) at %g seconds, with (#Edge_Masked=%u,#TotalEdge=%u,percent=%lf)",
						     nPattern_pseudo, pattern_pseudo1.nGene, pattern_pseudo1.nNet, parm->p, parm->q, parm->b, parm->h, parm->a, nIteration, nStage, t_elapse, totalEdges_masked, totalEdges, (double)totalEdges_masked/(double)totalEdges );
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
						/* write down the rest of network tensor for later debugging */
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

void LpRelax_byMultipleRounds( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p,
				   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity,
				   unsigned int nIteration, unsigned int nStage, enum MASK_STRATEGY mask_strategy_code,
				   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene,
				   VEC_DOUBLE xInit, VEC_DOUBLE yInit, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute )
{
	char logFile[MAXCHAR], patternFile[MAXCHAR], removedEdgesFile[MAXCHAR], stringCache[MAXCHAR];
	VEC_DOUBLE x, y, x_copy, y_copy; /* x: gene weights; y: net weights */
	unsigned int nGene, nNet, nPattern_old, kPattern, nPattern, nRound, nRemovedEdges;
	struct timeb t_start, t_end;
	double t_elapse;

	sprintf( logFile, "%s.LOG", resultFile );
	sprintf( patternFile, "%s.PATTERN", resultFile );
	sprintf( removedEdgesFile, "%s.RE", resultFile );
	nGene = t->nGene;
	nNet = t->nNet;
	/****** initialize weight vectors ******/
	create_VEC_DOUBLE( t->nGene, &x );
	create_VEC_DOUBLE( t->nNet, &y );
	create_VEC_DOUBLE( t->nGene, &x_copy );
	create_VEC_DOUBLE( t->nNet, &y_copy );
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
		nPattern += LpRelax_byMultipleParameters( *t, t_backup, p, nPattern_max, minGene, minNet, minDensity,
			nIteration, nStage, mask_strategy_code, overlapPatternChoose, resultFile, maxGene,
			xInit, yInit, &x, &y, &x_copy, &y_copy, level, mute );
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
}

void LpRelax_byMultipleStrategies( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p, int nPattern_max, unsigned int minGene,
				   unsigned int minNet, double minDensity, unsigned int nIteration, unsigned int nStage, enum MASK_STRATEGY mask_strategy_code,
				   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene, unsigned int resume, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute )
{
	VEC_DOUBLE xInit, yInit;
	char logFile[MAXCHAR], patternFile[MAXCHAR], stringCache[MAXCHAR];
	sprintf( logFile, "%s.LOG", resultFile );
	sprintf( patternFile, "%s.PATTERN", resultFile );
	createONES_VEC_DOUBLE( t->nGene, &xInit );
	createONES_VEC_DOUBLE( t->nNet, &yInit );

	sprintf( stringCache, "/////////////////////////////////////////////////////////////////////////////////\n//////Search Strategy: all genes and networks; Mask Strategy: %d; overlapPatternChoose: %s //////\n/////////////////////////////////////////////////////////////////////////////////\n\n",mask_strategy_code,overlapPatternChoose);
	if (level >= 2) {
		append_string_2file( logFile, stringCache );
	}
	if (!mute) {
		printf( "%s", stringCache ); fflush( stdout );
	}
	if (!resume) mask_genesWeights_from_patternFile( &xInit, patternFile, mute ); // This should be run, if it is a new run
	LpRelax_byMultipleRounds( t, t_backup, p, nPattern_max, minGene, minNet, minDensity, nIteration, nStage,
		                      mask_strategy_code, overlapPatternChoose, resultFile, maxGene, xInit, yInit,
							  removeHeavyFrequentEdges, level, mute );
	free_VEC_DOUBLE( &xInit );
	free_VEC_DOUBLE( &yInit );
}