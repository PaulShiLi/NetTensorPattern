#ifndef TENSOR_DYNAMICALSYSTEM_H
#define TENSOR_DYNAMICALSYSTEM_H

void localCluster_byMultipleRounds( TENSOROFNETS* t, TENSOROFNETS t_backup, PARMS_SETTING p,
                   int nPattern_max, unsigned int minGene, unsigned int minNet, double minDensity,
                   unsigned int nIteration, enum MASK_STRATEGY mask_strategy_code,
                   const char* overlapPatternChoose, const char* resultFile, unsigned int maxGene,
                   unsigned int resume, VEC_FLOAT removeHeavyFrequentEdges, int level, unsigned int mute );
#endif
