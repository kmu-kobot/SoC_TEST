#include "stdafx.h"
#include "SIFT.h"
#include <math.h>
#include <string>

#define USE_MATH_DEFINES

#define CHANNEL 1
#define ROOT2 1.4142135623730950488016887242097f
#define _2PI (float)M_PI * 2.0f
#define SIGMA 1.6
#define DES_SIGMA 8
#define DES_SIZE 16
#define DES_RADIUS 7.5f

CSIFT::CSIFT()
{
	sigma[0] = SIGMA;
	sigma[1] = sigma[0] * ROOT2;
	sigma[2] = sigma[1] * ROOT2;
	sigma[3] = sigma[2] * ROOT2;
	magSigma[0] = sigma[1] * 1.5;
	magSigma[1] = sigma[2] * 1.5;
	radius[0] = magSigma[0] * 3;
	radius[1] = magSigma[1] * 3;
	wsize[0] = (radius[0] >> 1) + 1;
	wsize[1] = (radius[1] >> 1) + 1;
}


CSIFT::~CSIFT()
{
}

void CSIFT::SIFT()
{

}

void CSIFT::Init(CByteImage& imageIn, int w, int h)
{
	m_imageIn = CByteImage(imageIn);
	width[0] = w << 1;
	width[1] = w;
	width[2] = w >> 1;
	width[3] = w >> 2;
	height[0] = h << 1;
	height[1] = h;
	height[2] = h >> 1;
	height[3] = h >> 2;
	wstep[0] = ((width[0] * CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
	wstep[1] = ((width[1] * CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
	wstep[2] = ((width[2] * CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
	wstep[3] = ((width[3] * CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
	size[0] = wstep[0] * height[0];
	size[1] = wstep[1] * height[1];
	size[2] = wstep[2] * height[2];
	size[3] = wstep[3] * height[3];

	for (int i = 0; i < 4; i++)
	{
		int octave = i >> 2;
		ScaleTemp[i] = CFloatImage(width[i], height[i], CHANNEL);
		for (int j = 0; j < 5; j++)
		{
			ScaleSpace[octave + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
		for (int j = 0; j < 4; j++)
		{
			DOG[octave + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
	}

	for (int i = 0; i < 2; i++)
	{
		weightMagnitude[i] = new float[wsize[i] * wsize[i]];
		for (int r = 0; r < wsize[i]; r++)
		{
			for (int c = 0; c < wsize[i]; c++)
			{
				weightMagnitude[i][r * wsize[i] + c] = expf(-(((r - radius[i]) * (r - radius[i]) + (c - radius[i]) * (c - radius[i])) / (2 * magSigma[i] * magSigma[i])));
			}
		}
	}

	float center[DES_SIZE];
	for (int r = 0; r < DES_SIZE; r++)
	{
		center[r] = expf(-(r - DES_RADIUS) * (r - DES_RADIUS) / (2 * DES_SIGMA * DES_SIGMA));
		for (int c = 0; c < DES_SIZE; c++)
		{
			weightDescript[r * DES_SIZE + c] = center[r] * expf(-(c - DES_RADIUS) * (c * DES_RADIUS) / (2 * DES_SIGMA * DES_SIGMA));
		}
	}
}

void CSIFT::BuildScaleSpace()
{

}

void CSIFT::BuildDOG()
{

}

void CSIFT::FindKeyPoint()
{

}

bool CSIFT::SubPixel(feature_t& key, int nAdjustment = 2)
{

}

void CSIFT::AccurategKey()
{

}

void CSIFT::BuildGradient()
{

}

void CSIFT::AssignOrientation()
{

}

void CSIFT::DescriptKey()
{

}


bool Solve(const float* H, const float* D, float* B)
{
	float det = (H[0] * H[4] * H[8]) + (H[3] * H[7] * H[2]) + (H[6] * H[1] * H[5]) - (H[0] * H[7] * H[5]) - (H[6] * H[4] * H[2]) - (H[3] * H[1] * H[8]); // det=a00*a11*a22 + a10*a21*a02 + a20*a01*a12 - a00*a21*a12 - a20*a11*a02 - a10*a01*a22

	if (!det)
		return false;

	float InverseH[9] = { det * (H[4] * H[8] - H[5] * H[7]), det * (H[2] * H[7] - H[1] * H[8]), det * (H[1] * H[5] - H[2] * H[4]),
		det * (H[5] * H[6] - H[3] * H[8]), det * (H[0] * H[8] - H[2] * H[6]), det * (H[2] * H[3] - H[0] * H[5]),
		det * (H[3] * H[7] - H[4] * H[6]), det * (H[1] * H[6] - H[0] * H[7]), det * (H[0] * H[4] - H[1] * H[3]) };

	B[0] = -(InverseH[0] * D[0] + InverseH[1] * D[1] + InverseH[2] * D[2]);
	B[1] = -(InverseH[3] * D[0] + InverseH[4] * D[1] + InverseH[5] * D[2]);
	B[2] = -(InverseH[6] * D[0] + InverseH[7] * D[1] + InverseH[8] * D[2]);

	return true;
}