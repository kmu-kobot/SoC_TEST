#include "stdafx.h"
#include "Feature.h"

float _CalcSIFTSqDist(const feature_t& k1, const feature_t& k2)
{
const float *pk1, *pk2;

pk1 = k1.vec;
pk2 = k2.vec;

float dif;
float distsq = 0;
for (int i = 0; i<128; i++)
{
	dif = *pk1++ - *pk2++;
	distsq += dif * dif;
}
return distsq;
}
