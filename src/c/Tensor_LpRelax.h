#include "UtilsOfTensor.h"
#include "Tensor_Private.h"

#ifndef TENSOR_LPRELAX_H
#define TENSOR_LPRELAX_H

void normalize_x( VEC_DOUBLE* x, double p, double a, double b );

void normalize_y_efficient( VEC_DOUBLE* y, double norm );

void TensorAnalysisMultipleNetworks_LpRelax( TENSOROFNETS t, PARMS p, unsigned int nIteration,
											 unsigned int nStage, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
											 VEC_DOUBLE* x, VEC_DOUBLE* y );

unsigned int LpRelax_byMultipleParameters( TENSOROFNETS t, TENSOROFNETS t_backup, PARMS_SETTING p,
				   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity, unsigned int nIteration,
				   unsigned int nStage, enum MASK_STRATEGY mask_strategy_code, const char* overlapPatternChoose,
				   const char* resultFile, unsigned int maxGene, VEC_DOUBLE xInit, VEC_DOUBLE yInit,
				   VEC_DOUBLE* x, VEC_DOUBLE* y, VEC_DOUBLE* x_copy, VEC_DOUBLE* y_copy, int level, unsigned int mute );


void LpRelax_byMultipleRounds( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p,
				   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity,
				   unsigned int nIteration, unsigned int nStage, enum MASK_STRATEGY mask_strategy_code,
				   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene,
				   VEC_DOUBLE xInit, VEC_DOUBLE yInit, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute );

void LpRelax_byMultipleStrategies( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p, int nPattern_max, unsigned int minGene,
				   unsigned int minNet, double minDensity, unsigned int nIteration, unsigned int nStage, enum MASK_STRATEGY mask_strategy_code,
				   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene, unsigned int resume, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute );

#endif
