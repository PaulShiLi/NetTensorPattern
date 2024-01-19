#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>
#include "Tensor_Private.h"
#include "default.h"

/*---------------------------------
  Routines for OVERLAPPATTERNCHOOSE
-----------------------------------*/
unsigned int check_OVERLAPPATTERNCHOOSE(char* choose)
{
	unsigned int ok=FALSE;
	if (strcmp(choose, OVERLAPPATTERNCHOOSE_NONZEROS)==0) ok=TRUE;
	if (strcmp(choose, OVERLAPPATTERNCHOOSE_MORE_NETS)==0) ok=TRUE;
	if (strcmp(choose, OVERLAPPATTERNCHOOSE_MORE_GENES)==0) ok=TRUE;
	if (strcmp(choose, OVERLAPPATTERNCHOOSE_BOTH)==0) ok=TRUE;
	return ok;
}

/*---------------------------------
  Routines for MASK_STRATEGY
-----------------------------------*/
enum MASK_STRATEGY checkname_MASK_STRATEGY(char* mask_strategy_name)
{
	enum MASK_STRATEGY mask_code = MASKSTRATEGY_ERROR;
	if (strcmp(mask_strategy_name, MASK_STRATEGY_NAME_EDGES_PATTERN)==0) mask_code=MASKSTRATEGY_EDGES_PATTERN;
	if (strcmp(mask_strategy_name, MASK_STRATEGY_NAME_EDGES_ALLNETS)==0) mask_code=MASKSTRATEGY_EDGES_ALLNETS;
	if (strcmp(mask_strategy_name, MASK_STRATEGY_NAME_GENES)==0) mask_code=MASKSTRATEGY_GENES;
	return mask_code;
}

/*---------------------------------
  Routines for PARMS and PARMS_SETTING
-----------------------------------*/
void init_PARMS( PARMS* p ) { p->sparsity=0; p->alpha=0; p->howtoInit_xy=INIT_XY_BY_ONES; }
void print_PARMS(FILE* stream, PARMS p, char* prefix_String) {
	if (p.howtoInit_xy==INIT_XY_BY_RAND) {
		fprintf( stream, "%s\tsparsity=%d, alpha=%g, howtoInit_xy=random\n", prefix_String, p.sparsity, p.alpha );
	} else if (p.howtoInit_xy==INIT_XY_BY_ONES) {
		fprintf( stream, "%s\tsparsity=%d, alpha=%g, howtoInit_xy=ones\n", prefix_String, p.sparsity, p.alpha );
	} else if (p.howtoInit_xy>=1) {
		fprintf( stream, "%s\tsparsity=%d, alpha=%g, howtoInit_xy=unit vector of a query gene (index=%u)\n", prefix_String, p.sparsity, p.alpha, p.howtoInit_xy );
	} else {
		fprintf( stderr, "Error print_PARMS: howtoInit_xy is not correct.\nExit.\n" );
		exit(-1);
	}
}
void assign_PARMS_valueset1( PARMS* p )
{
	p->sparsity = MAXGENE;
	p->alpha = 2.0;
}
void assign_PARMS_valueset2( PARMS* p )
{
	p->sparsity = MAXGENE;
	p->alpha = 1.5;
}
void assign_PARMS_valueset3( PARMS* p )
{
	p->sparsity = MAXGENE;
	p->alpha = 1.0;
}
unsigned int read_PARMS( PARMS* p, char* filename )
{
	char sep_tab[]   = "\t";
	char line[MAX_LINE_LENGTH], *token1, *token2;
	int count = 0;
	FILE* stream;
	assign_PARMS_valueset1( p );
	if( (stream = fopen( filename, "r" )) == NULL ) { errorfile( filename, "read" ); exit(0); }
	while( !feof( stream ) ) {
	if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL ) {
			if (strlen(line)==0) { fclose(stream); return FALSE; }
			remove_newlinechars( line );
			token1 = strtok( line, sep_tab );
			token2 = strtok( NULL, sep_tab );
			if ( token1==NULL || token2==NULL ) { fprintf( stderr, "PARMS File Format Error!\n" ); fclose(stream); return FALSE; }
			switch (token1[0]) { // do not consider "p.howtoInit_xy", because we always use INIT_XY_BY_ONES in the algorithm implementation.
				case 's': p->sparsity = atoi(token2); count++; break;
				case 'a': p->alpha = atof(token2); count++; break;
			}
		}
	}
	fclose( stream );
	if (count>=1) return TRUE;
	else return FALSE;
}

void init_PARMS_SETTING( PARMS_SETTING* p )
{
	p->n_setting = 0;
	p->settings = NULL;
}
void create_PARMS_SETTING( unsigned int n_setting, PARMS_SETTING* p )
{
	unsigned int i;
	if (n_setting==0) { init_PARMS_SETTING(p); return; }
	p->n_setting = n_setting;
	p->settings = (PARMS*)malloc( n_setting*sizeof(PARMS) );
	if ( p->settings == NULL ) erroralloc("PARMS",n_setting);
	for (i=0; i<n_setting; i++) init_PARMS( &p->settings[i] );
}
void free_PARMS_SETTING( PARMS_SETTING* p )
{
	if (p->settings!=NULL) free( p->settings );
}
void print_PARMS_SETTING(FILE* stream, PARMS_SETTING p, char* prefix_String)
{
	unsigned int i;
	char string[MAXCHAR];
	fprintf( stream, "%sPredefined %u sets of parameters:\n", prefix_String, p.n_setting);
	for (i=0; i<p.n_setting; i++) {
		sprintf( string, "%s\t(%u):", prefix_String, i+1 );
		print_PARMS(stream, p.settings[i], string);
	}
}

/*---------------------------------
  Routines for PATTERN
-----------------------------------*/
void init_PATTERN( PATTERN* p )
{
	p->nGene = 0;
	p->nNet = 0;
	init_VEC_UINT( &p->genesIDX );
	init_VEC_UINT( &p->netsIDX );
	init_VEC_FLOAT( &p->sumOfWeights );
	init_VEC_DOUBLE( &p->densities );
}
void create_PATTERN( unsigned int nGene, unsigned int nNet, PATTERN* p)
{
	p->nGene = nGene;
	p->nNet = nNet;
	create_VEC_UINT( nGene, &p->genesIDX );
	create_VEC_UINT( nNet, &p->netsIDX );
	create_VEC_FLOAT( nNet, &p->sumOfWeights );
	create_VEC_DOUBLE( nNet, &p->densities );
}
void free_PATTERN( PATTERN* p )
{
	free_VEC_UINT( &p->genesIDX );
	free_VEC_UINT( &p->netsIDX );
	free_VEC_FLOAT( &p->sumOfWeights );
	free_VEC_DOUBLE( &p->densities );
}
double minDensity_inPattern( PATTERN p )
{
	unsigned int i;
	double minD=1;
	for (i=0; i<p.nNet; i++)
		if (p.densities.v[i]<minD) minD = p.densities.v[i];
	return minD;
}
int recordPattern( DENSITIES d, unsigned int nGene, unsigned int nNetdense, double minDensity, PATTERN* p, unsigned int mute )
{
	unsigned int i, iGene, iNetdense;
	if (nGene>d.maxGene) {
		if (!mute) {
			printf("In function 'recordPattern', nGene(%u)>d.maxGene(%u)\n", nGene, d.maxGene);
		}
		return -1;
	}
	create_PATTERN( nGene, nNetdense, p );
	/* fill field ".genesIDX" */
	for (i=0; i<p->nGene; i++) p->genesIDX.v[i] = d.genesIDX.v[i];
	iGene = nGene-1;
	/* fill field ".netsIDX" and ".densities" */
	for (i=0, iNetdense=0; i<d.nNet; i++)
		if (d.densities[i].v[iGene]>=minDensity) {
			p->netsIDX.v[iNetdense] = i;
			p->sumOfWeights.v[iNetdense] = d.sumOfWeights[i].v[iGene];
			p->densities.v[iNetdense] = d.densities[i].v[iGene];
			iNetdense++;
		}
	if (iNetdense!=nNetdense) {
		if (!mute) {
			printf("In function 'recordPattern', iNetdense(%u)!=nNetdense(%u)\n", iNetdense, nNetdense);
		}
	return -1;
	}
	return 1;
}
int recordPattern_bySelectedGenesNetworks( DENSITIES d, unsigned int nGene_Of_Pattern, unsigned int nNet_Of_Pattern,
										   VEC_UINT geneRanks, VEC_UINT netRanks, PATTERN* p )
{
	unsigned int i, netIDX;
	create_PATTERN( nGene_Of_Pattern, nNet_Of_Pattern, p );
	/* fill field ".genesIDX" */
	for (i=0; i<p->nGene; i++) p->genesIDX.v[i] = geneRanks.v[i];
	/* fill field ".netsIDX" and ".densities" */
	for (i=0; i<p->nNet; i++) {
		netIDX = netRanks.v[i];
		p->netsIDX.v[i] = netIDX;
		p->densities.v[i] = d.densities[netIDX].v[nGene_Of_Pattern-1];
		p->sumOfWeights.v[i] = d.sumOfWeights[netIDX].v[nGene_Of_Pattern-1];
	}
	return 1;
}
void write_genes_Of_PATTERN( PATTERN p, char* file )
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	for (i=0; i<(p.genesIDX.n-1); i++) fprintf( stream, "%u\t", p.genesIDX.v[i]+1 );
	fprintf( stream, "%u\n", p.genesIDX.v[i]+1);
	fclose( stream );
}
void write_datasets_Of_PATTERN( PATTERN p, char* file )
{
	FILE* stream;
	unsigned int i;
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	for (i=0; i<(p.netsIDX.n-1); i++) fprintf( stream, "%u\t", p.netsIDX.v[i]+1 );
	fprintf( stream, "%u\n", p.netsIDX.v[i]+1);
	fclose( stream );
}
void write_PATTERN_succinct( PATTERN p, char* file )
{
	FILE* stream;
	unsigned int i;
	double minD;
	minD = minDensity_inPattern( p );
	if( (stream = fopen( file, "a" )) == NULL ) { errorfile( file, "append" ); exit(0); }
	fprintf( stream, "[" );
	for (i=0; i<(p.genesIDX.n-1); i++) fprintf( stream, "%u, ", p.genesIDX.v[i]+1 );
	fprintf( stream, "%u]\t%u\t%.2g\t%u\t[]\t[]\t", p.genesIDX.v[i]+1, p.nGene, minD, p.nNet);
	for(i=0; i<(p.nNet-1); i++) fprintf( stream, "%g:%u:%.2g ", p.sumOfWeights.v[i], p.netsIDX.v[i]+1, p.densities.v[i]);
	fprintf( stream, "%g:%u:%.2g\n", p.sumOfWeights.v[i], p.netsIDX.v[i]+1, p.densities.v[i]);
	fclose( stream );
}
void read_PATTERN_succinct_fromString( char* string_pattern, PATTERN *p )
{
	char CharTable='\t', StringSpace[]=" ", CharColon=':';
	char *token, *lastItem, *subtoken;
	unsigned int i;
	init_PATTERN( p );
	if (string_pattern==NULL || strlen(string_pattern)==0) return;
	remove_newlinechars( string_pattern );
	// read genes
	token = strtok_w( string_pattern, CharTable );
	if ( token==NULL ) { fprintf( stderr, "PATTERN Format Error: set of genes in the pattern does not exist!\n" ); return; }
	read_string2VEC_UINT_SetFormat( token, &p->genesIDX );
	for (i=0; i<p->genesIDX.n; i++) p->genesIDX.v[i]--; // gene IDX starts from 1 in pattern string, but starts from 0 in this program memory.
	// read number of genes
	token = strtok_w( NULL, CharTable );
	if ( token==NULL ) { fprintf( stderr, "PATTERN Format Error: number of genes in the pattern does not exist!\n" ); free_VEC_UINT(&p->genesIDX); return; }
	if ( p->genesIDX.n != (unsigned int)atoi(token) )
		fprintf( stderr, "PATTERN Format Warning: number of genes in the pattern does not agree!\n" );
	p->nGene = p->genesIDX.n;
	// skip min density
	strtok_w( NULL, CharTable );
	// read number of networks
	token = strtok_w( NULL, CharTable );
	if ( token==NULL ) { fprintf( stderr, "PATTERN Format Error: number of networks in the pattern does not exist!\n" ); free_VEC_UINT(&p->genesIDX); return; }
	p->nNet = (unsigned int)atoi(token);
	// skip []
	strtok_w( NULL, CharTable );
	// skip []
	strtok_w( NULL, CharTable );
	// read "sumOfWeights", IDX_of_network, Densities for each network
	lastItem = strtok_w( NULL, CharTable );
	token = strtok( lastItem, StringSpace );
	while ( token!=NULL ) {
		subtoken = strtok_w( token, CharColon ); // read #Edges
		if (subtoken==NULL) {
			fprintf( stderr, "PATTERN Format Error: #Edges, IDX_of_network and Densities for each network in the pattern does not exist!\n" );
			free_VEC_UINT(&p->genesIDX); free_VEC_UINT(&p->netsIDX); free_VEC_FLOAT(&p->sumOfWeights); free_VEC_DOUBLE(&p->densities); return;
		}
		addnumber_VEC_FLOAT( (float)atof(subtoken), &p->sumOfWeights );
		subtoken = strtok_w( NULL, CharColon ); // read IDX_of_network
		if (subtoken==NULL) {
			fprintf( stderr, "PATTERN Format Error: #Edges, IDX_of_network and Densities for each network in the pattern does not exist!\n" );
			free_VEC_UINT(&p->genesIDX); free_VEC_UINT(&p->netsIDX); free_VEC_FLOAT(&p->sumOfWeights); free_VEC_DOUBLE(&p->densities); return;
		}
		addnumber_VEC_UINT( (unsigned int)atoi(subtoken), &p->netsIDX );
		subtoken = strtok_w( NULL, CharColon ); // read Density of network
		if (subtoken==NULL) {
			fprintf( stderr, "PATTERN Format Error: #Edges, IDX_of_network and Densities for each network in the pattern does not exist!\n" );
			free_VEC_UINT(&p->genesIDX); free_VEC_UINT(&p->netsIDX); free_VEC_FLOAT(&p->sumOfWeights); free_VEC_DOUBLE(&p->densities); return;
		}
		addnumber_VEC_DOUBLE( (double)atof(subtoken), &p->densities );
		token = strtok( NULL, StringSpace ); // read the next token seperated by space
	}
	for (i=0; i<p->netsIDX.n; i++) p->netsIDX.v[i]--; // network IDX starts from 1 in pattern string, but starts from 0 in this program memory.
	if ( p->nNet!=p->netsIDX.n ) {
		fprintf( stderr, "PATTERN Format Error: number of network in the pattern does not agree!\n" );
		fprintf( stderr, "\n#net=%d != %d\n", p->nNet, p->netsIDX.n);
		free_VEC_UINT(&p->genesIDX); free_VEC_UINT(&p->netsIDX); free_VEC_FLOAT(&p->sumOfWeights); free_VEC_DOUBLE(&p->densities);
	}
}

/* Criterion 1 of a pattern: density>=minDensity and the maximum number of nets.
   So this criterion prefers a small-size gene group with large frequency.
   a PATTERN (variable p) will be created and returned.
   suppose "d_cache" already carries all density info of all subnetwork
   formed by top-ranking genes.
   
   The following variables are caches, which are allocated space before calling
   "d_cache", "xsorti_cache", "ysorti_cache", "geneRanks_cache"
   These caches will be frequently used when calling this function.
*/
void getPattern_byCriterion1( TENSOROFNETS t, unsigned int minGene, unsigned int minNet,
							 double minDensity, DENSITIES d, PATTERN *p, unsigned int mute )
{
	unsigned int nGene, maxGene, nGene_Of_Pattern, nNet_Of_Pattern, maxNet_Of_Pattern;
	int result;
	maxGene = d.maxGene; // calculate densities of subnetworks formed by the top-'maxGene' ranking genes
	init_PATTERN( p );
	/* check maximal top-ranking genes which is a pattern */
	nGene_Of_Pattern=0;
	maxNet_Of_Pattern=0;
	for (nGene=minGene; nGene<=maxGene; nGene++) {
		nNet_Of_Pattern = getNumberOfDenseNets( d, nGene, minDensity, mute );
		if (nNet_Of_Pattern>=maxNet_Of_Pattern) {
			nGene_Of_Pattern=nGene;
			maxNet_Of_Pattern = nNet_Of_Pattern;
		}
	}
	nNet_Of_Pattern = maxNet_Of_Pattern;
	if (nNet_Of_Pattern<minNet) return; // no pattern is found, return empty pattern.
	/* record this pattern */
	//printf("\t\tCriterion 1: nGene_Of_Pattern=%u, nNet=%u\n",nGene_Of_Pattern,nNet_Of_Pattern);
	result = recordPattern( d, nGene_Of_Pattern, nNet_Of_Pattern, minDensity, p, mute );
	if (result==-1) { fprintf(stderr, "Error: getPattern_byCriterion1 - unable to record the pattern\n"); exit(-1); }
}
/* Criterion 2 of a pattern: density>=minDensity and the maximum number of genes with at least 'minNet' nets.
   So this criterion prefers a large-size gene group with small frequency (at least 'minNet').
   a PATTERN (variable p) will be created and returned.
   suppose "d" already carries all density info of all subnetwork
   formed by top-ranking genes.
*/
void getPattern_byCriterion2( TENSOROFNETS t, unsigned int minGene, unsigned int minNet,
							  double minDensity, DENSITIES d, PATTERN *p, unsigned int mute )
{
	unsigned int nGene, nNet, maxGene, nGene_Of_Pattern, nNet_Of_Pattern;
	int result;
	maxGene = d.maxGene; // calculate densities of subnetworks formed by the top-'maxGene' ranking genes
	init_PATTERN( p );
	/* check maximal top-ranking genes which is a pattern */
	nGene_Of_Pattern=0; nNet_Of_Pattern=0;
	for (nGene=minGene; nGene<=maxGene; nGene++) {
		nNet = getNumberOfDenseNets( d, nGene, minDensity, mute );
		if (nNet>=minNet) {
			nGene_Of_Pattern = nGene;
			nNet_Of_Pattern = nNet;
		}
	}
	if (nGene_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	/* record this pattern */
	//printf("\t\tCriterion 2: nGene_Of_Pattern=%u, nNet=%u\n",nGene_Of_Pattern,nNet_Of_Pattern);
	result = recordPattern( d, nGene_Of_Pattern, nNet_Of_Pattern, minDensity, p, mute );
	if (result==-1) { fprintf(stderr, "Error: getPattern_byCriterion2 - unable to record the pattern\n"); exit(-1); }
}
/* Criterion 3 of a pattern: select those genes and networks whose px or py power of weights are greater than 0 (measured by >=zeroThreshold).
   So this criterion uses only the information from genes and networks weights.
   a PATTERN (variable p) will be created and returned.
   suppose "d" already carries all density info of all subnetwork
   "xsort" and "ysort" are already sorted by decreasing order, and "xsorti" and "ysorti" are their indexes of genes
   and networks.
   "px" and "py" are the exponents used in the tensor method as L_{px} and L_{py}.
   "zeroThreshold" is a real value very close to zero. It is used to measure if a number is zero or non-zero.
*/
void getPattern_byCriterion3( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double zeroThreshold, DENSITIES d, PATTERN *p )
{
	unsigned int iGene, iNet, nGene_Of_Pattern, nNet_Of_Pattern;
	int result;
	init_PATTERN( p );
	for (iGene=0, nGene_Of_Pattern=0; iGene<xsort.n; iGene++) {
		if (xsort.v[iGene]<zeroThreshold) break;
		nGene_Of_Pattern++;
	}
	if (nGene_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	if (nGene_Of_Pattern>MAXGENE) nGene_Of_Pattern=MAXGENE; // MAXGENE is the maximum genes a pattern should contain. Defined in "parameter.h"
	for (iNet=0, nNet_Of_Pattern=0; iNet<ysort.n; iNet++) {
		if (ysort.v[iNet]<zeroThreshold) break;
		nNet_Of_Pattern++;
	}
	if (nNet_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	result = recordPattern_bySelectedGenesNetworks( d, nGene_Of_Pattern, nNet_Of_Pattern, xsorti, ysorti, p );
	if (result==-1) { fprintf(stderr, "Error: getPattern_byCriterion3 - unable to record the pattern\n"); exit(-1); }
}
/* Criterion 4 of a pattern: select those genes and networks whose weights are greater than 0 (measured by >=zeroThreshold).
   So this criterion uses only the information from genes and networks weights.
   a PATTERN (variable p) will be created and returned.
   suppose "d" already carries all density info of all subnetwork
   "xsort" and "ysort" are already sorted by decreasing order, and "xsorti" and "ysorti" are their indexes of genes
   and networks.
   "zeroThreshold" is a real value very close to zero. It is used to measure if a number is zero or non-zero.
*/
void getPattern_byCriterion4( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double zeroThreshold, DENSITIES d, PATTERN *p )
{
	unsigned int iGene, iNet, nGene_Of_Pattern, nNet_Of_Pattern;
	int result;
	init_PATTERN( p );
	for (iGene=0, nGene_Of_Pattern=0; iGene<xsort.n; iGene++) {
		if (xsort.v[iGene]<zeroThreshold) break;
		nGene_Of_Pattern++;
	}
	if (nGene_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	if (nGene_Of_Pattern>MAXGENE) nGene_Of_Pattern=MAXGENE; // MAXGENE is the maximum genes a pattern should contain. Defined in "parameter.h"
	for (iNet=0, nNet_Of_Pattern=0; iNet<ysort.n; iNet++) {
		if (ysort.v[iNet]<zeroThreshold) break;
		nNet_Of_Pattern++;
	}
	if (nNet_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	result = recordPattern_bySelectedGenesNetworks( d, nGene_Of_Pattern, nNet_Of_Pattern, xsorti, ysorti, p );
	if (result==-1) { fprintf(stderr, "Error: getPattern_byCriterion4 - unable to record the pattern\n"); exit(-1); }
}

void getPattern_byCriterionLP( VEC_DOUBLE xsort, VEC_DOUBLE ysort, VEC_UINT xsorti, VEC_UINT ysorti,
							  double px, double py, double zeroThreshold, DENSITIES d, PATTERN *p )
{
	unsigned int iGene, iNet, nGene_Of_Pattern, nNet_Of_Pattern;
	int result;
	init_PATTERN( p );
	for (iGene=0, nGene_Of_Pattern=0; iGene<xsort.n; iGene++) {
		if (pow(xsort.v[iGene],px)<zeroThreshold) break;
		nGene_Of_Pattern++;
	}
	if (nGene_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	if (nGene_Of_Pattern>MAXGENE) nGene_Of_Pattern=MAXGENE; // MAXGENE is the maximum genes a pattern should contain. Defined in "parameter.h"
	for (iNet=0, nNet_Of_Pattern=0; iNet<ysort.n; iNet++) {
		if (pow(ysort.v[iNet],py)<zeroThreshold) break;
		nNet_Of_Pattern++;
	}
	if (nNet_Of_Pattern==0) return; // no pattern is found, return empty pattern.
	result = recordPattern_bySelectedGenesNetworks( d, nGene_Of_Pattern, nNet_Of_Pattern, xsorti, ysorti, p );
	if (result==-1) { fprintf(stderr, "Error: getPattern_byCriterion3 - unable to record the pattern\n"); exit(-1); }
}


/* Masking Pattern Strategy 1: Zero all dense subnetworks formed by top-ranking selected genes and networks in the pattern.
   Masking Pattern Strategy 2: Zero all edges adjacent to top-ranking selected genes in the pattern, and across all networks*/
unsigned int mask_strategy( TENSOROFNETS* t, PATTERN p, enum MASK_STRATEGY strategy_code, VEC_UINT* geneRanks_cache )
{
	unsigned int nEdges_Masked, i;
	/* initialize "geneRanks_cache" by assigning ranks to top-ranking selected genes in the pattern*/
	zeros_VEC_UINT( geneRanks_cache );
	for (i=0; i<p.nGene; i++) geneRanks_cache->v[p.genesIDX.v[i]] = i+1; // assign ranks (1:p.nGene) to top-ranking "p.nGene" genes
	/* zero all dense subnetworks formed by top-ranking selected genes in the pattern. */
	nEdges_Masked = 0;
	switch (strategy_code) {
		case MASKSTRATEGY_EDGES_PATTERN:
			for ( i=0; i<p.nNet; i++ )
				nEdges_Masked += zerosAllEdges_Of_NET_BetweenSelectedGenes( &t->nets[p.netsIDX.v[i]], *geneRanks_cache );
			break;
		case MASKSTRATEGY_EDGES_ALLNETS:
			for ( i=0; i<t->nNet; i++ )
				nEdges_Masked += zerosAllEdges_Of_NET_BetweenSelectedGenes( &t->nets[i], *geneRanks_cache );
			break;
		case MASKSTRATEGY_GENES:
			for ( i=0; i<t->nNet; i++ )
				nEdges_Masked += zerosAllEdges_Of_NET_AdjacentToSelectedGenes( &t->nets[i], *geneRanks_cache );
			break;
		default:
			fprintf( stderr, "\nError: strategy is not properly assigned!\n" );
	}
	return nEdges_Masked;
}
/* Masking weights of genes contained in the patter: zeros weights of the genes which are contained in the pattern.
   Before calling, suppose x has been allocated and initialized. */
unsigned int mask_genesWeights( VEC_DOUBLE* x, PATTERN p )
{
	unsigned int nGenes_Masked=0, gIDX, i;
	for ( i=0; i<p.nGene; i++ ) {
		gIDX = p.genesIDX.v[i];
		if ( x->v[gIDX]!=0 ){
			x->v[gIDX] = 0;
			nGenes_Masked++;
		}
	}
	return nGenes_Masked;
}
/* Masking all patterns in the pattern file by assigned strategy. */
unsigned int mask_Patterns_from_File( TENSOROFNETS* t, enum MASK_STRATEGY strategy, char* patternFile, unsigned int mute )
{
	VEC_UINT geneRanks_cache;
	PATTERN p_cache;
	unsigned int nEdges_Masked, nPattern;
	FILE *stream;
	char *line;
	if( (stream = fopen( patternFile, "r" )) == NULL ) {
		fprintf( stderr, "\nWarning: No pattern file found. Creating file '%s'.\n", patternFile );
		return 0;
	}
	/****** initialize caches ******/
	line = (char*)malloc(MAX_LINE_LENGTH*sizeof(char));
	create_VEC_UINT( t->nGene, &geneRanks_cache );
	init_PATTERN( &p_cache );
	/* read following lines */
	nPattern=0; nEdges_Masked=0;
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL ) {
			nPattern++;
			read_PATTERN_succinct_fromString( line, &p_cache );
			nEdges_Masked += mask_strategy( t, p_cache, strategy, &geneRanks_cache );
			free_PATTERN( &p_cache ); init_PATTERN( &p_cache );
		}
	}
	fclose( stream );
	free( line );
	free_VEC_UINT( &geneRanks_cache );
	if (!mute) {
		printf( "\nmasked #Patterns=%u of the pattern file with #Edges=%u\n", nPattern, nEdges_Masked );
	}
	return nPattern;
}
unsigned int mask_genesWeights_from_patternFile( VEC_DOUBLE* x, char* patternFile, unsigned int mute )
{
	PATTERN p_cache;
	unsigned int nGenes_Masked, nPattern;
	FILE *stream;
	char line[MAX_LINE_LENGTH];
	if( (stream = fopen( patternFile, "r" )) == NULL ) {
		fprintf( stderr, "\nWarning: No pattern file found. Creating file '%s'.\n", patternFile );
		return 0;
	}
	/* read following lines */
	nPattern=0; nGenes_Masked=0;
	init_PATTERN( &p_cache );
	while( !feof( stream ) ) {
		if( fgets( line, MAX_LINE_LENGTH, stream ) != NULL ) {
			nPattern++;
			read_PATTERN_succinct_fromString( line, &p_cache );
			nGenes_Masked += mask_genesWeights( x, p_cache );
			free_PATTERN( &p_cache ); init_PATTERN( &p_cache );
		}
	}
	fclose( stream );
	if (!mute) {
		printf( "\nmasked weights of #Gene=%u in #Patterns=%u of the pattern file\n", nGenes_Masked, nPattern );
	}
	return nGenes_Masked;
}
