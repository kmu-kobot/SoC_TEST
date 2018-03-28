#include "stdafx.h"
#include "SIFT.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

#include "VideoProcessing.h"
#include "MyImageFunc.h"
#include "ImageFrameWndManager.h"


#define CHANNEL 1
#define OCTAVE_NUM 4
#define LEVEL_NUM_SS 5
#define LEVEL_NUM_DOG 4
#define LEVEL_NUM_LE 2
#define ROOT2 1.4142135623730950488016887242097f
#define _2PI ((float)M_PI * 2.0f)
#define SIGMA 1.6f
#define DES_SIGMA 8.0f
#define DES_SIZE 16
#define DES_RADIUS 7.5f
#define VEC_ORI_NUM 8
#define KEY_THRES 0.8f
#define DIST_THRES 0.8f

#define BOX

CSIFT::CSIFT()
{
	sigma[0] = SIGMA;
	sigma[1] = sigma[0] * ROOT2;
	sigma[2] = sigma[1] * ROOT2;
	sigma[3] = sigma[2] * ROOT2;
	magSigma[0] = sigma[1] * 1.5f;
	magSigma[1] = sigma[2] * 1.5f;
	radius[0] = (int)magSigma[0] * 3;
	radius[1] = (int)magSigma[1] * 3;
	wsize[0] = (radius[0] << 1) + 1;
	wsize[1] = (radius[1] << 1) + 1;
}


CSIFT::~CSIFT()
{
	//delete [] weightMagnitude[0];
	//delete [] weightMagnitude[1];

	//for (int i = 0; i < 8; i++)
	//{
		//delete[] MagMap[i];
		//delete[] OriMap[i];
	//}
	//for (itr = feature.begin(); itr != feature.end(); itr++)
	//{
	//	delete &itr;
	//}
	feature.clear();
	//for (itr = feature_sub.begin(); itr != feature_sub.end(); itr++)
	//{
	//	delete &itr;
	//}
	feature_sub.clear();
}

void CSIFT::BuildCmp(CByteImage& image)
{
	int width = image.GetWidth();
	int height = image.GetHeight();
	int wstep = image.GetWStep();
	m_imageCmp = CByteImage(width, height, image.GetChannel());
	for (int r = 0; r < height; r++)
	{
		memcpy(m_imageCmp.GetPtr(r), image.GetPtr(r), wstep * sizeof(BYTE));
	}
	Init(width, height);
	SIFT(image);
	CopyCmp();
}

void CSIFT::SIFT(CByteImage& imageIn)
{
#ifndef STATIC_SIZE
	int w = imageIn.GetWidth();
	int h = imageIn.GetHeight();
	m_imageInGray = CByteImage(w, h);
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

	for (int i = 0; i < OCTAVE_NUM; i++)
	{
		int octave = i >> 2;
		ScaleTemp[i] = CFloatImage(width[i], height[i], CHANNEL);
		for (int j = 0; j < LEVEL_NUM_SS; j++)
		{
			ScaleSpace[octave + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
		for (int j = 0; j < LEVEL_NUM_DOG; j++)
		{
			DOG[octave + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
	}

	for (int i = 0; i < LEVEL_NUM_LE; i++)
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
#endif
	m_imageIn = imageIn;
	m_imageInGray = RGB2Gray(m_imageIn);

	//ShowImage(m_imageIn, "imageIn");
	//ShowImage(m_imageInGray, "imageInGray");

	BuildScaleSpace();
	BuildDOG();
	FindKeyPoint();
	//AccurateKey();
	BuildGradient();
	AssignOrientation();
 	DescriptKey();
	ShowKeyPoint();
}

#ifdef STATIC_SIZE
void CSIFT::Init(int w, int h)
{
	m_imageIn = CByteImage(w, h, 3);

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
	imageInX2 = CByteImage(width[0], height[0], CHANNEL);

	for (int i = 0; i < OCTAVE_NUM; i++)
	{
		ScaleTemp[i] = CFloatImage(width[i], height[i], CHANNEL);
		for (int j = 0; j < LEVEL_NUM_SS; j++)
		{
			ScaleSpace[i * LEVEL_NUM_SS + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
		for (int j = 0; j < LEVEL_NUM_DOG; j++)
		{
			DOG[i * LEVEL_NUM_DOG + j] = CFloatImage(width[i], height[i], CHANNEL);
		}
	}

	for (int i = 0; i < 4; i++)
	{
		MagMap[i * LEVEL_NUM_LE] = new float[height[i] * wstep[i]];
		MagMap[i * LEVEL_NUM_LE + 1] = new float[height[i] * wstep[i]];
		OriMap[i * LEVEL_NUM_LE] = new float[height[i] * wstep[i]];
		OriMap[i * LEVEL_NUM_LE + 1] = new float[height[i] * wstep[i]];

	}

	for (int i = 0; i < LEVEL_NUM_LE; i++)
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
			weightDescript[r * DES_SIZE + c] = center[r] * expf(-(c - DES_RADIUS) * (c - DES_RADIUS) / (2 * DES_SIGMA * DES_SIGMA));
		}
	}
}
#endif


void CSIFT::BuildScaleSpace()
{
	for (int r = 0; r < height[0]; r++)
	{
		BYTE* pDst = imageInX2.GetPtr(height[0] - r - 1);
		for (int c = 0; c < width[0]; c++)
		{
			pDst[c] = m_imageInGray.BiLinearIntp(c >> 1, r >> 1);
		}
	}

	BYTE* pOriginal = imageInX2.GetPtr();
	float* pTemp = ScaleTemp[0].GetPtr();

	for (int pos = 0; pos < size[0]; pos++)
	{
		pTemp[pos] = (float)pOriginal[pos];
	}

#ifdef BOX
	gaussBlur(ScaleTemp[0], ScaleSpace[0], sigma[0]);
#else
	GaussianBlur(ScaleTemp[0], ScaleSpace[0], sigma[0]);
#endif
	for (int i = 0; i < LEVEL_NUM_SS - 1; i++)
	{
		memcpy(ScaleTemp[0].GetPtr(), ScaleSpace[i].GetPtr(), size[0] * sizeof(float));
#ifdef BOX
		gaussBlur(ScaleTemp[0], ScaleSpace[i + 1], sigma[i]);
#else
		GaussianBlur(ScaleTemp[0], ScaleSpace[i + 1], sigma[i]);
#endif
	}

	for (int i = 1; i < OCTAVE_NUM; i++)
	{
		for (int r = 0; r < height[i]; r++)
		{
			float* pDst = ScaleSpace[i * LEVEL_NUM_SS].GetPtr(r);
			float* pSrc = ScaleSpace[i * LEVEL_NUM_SS - 3].GetPtr(2 * r);
			for (int c = 0; c < width[i]; c++)
			{
				pDst[c] = pSrc[2 * c];
			}
		}

		for (int j = 1; j < LEVEL_NUM_SS; j++)
		{
			memcpy(ScaleTemp[i].GetPtr(), ScaleSpace[i * LEVEL_NUM_SS + j - 1].GetPtr(), size[i] * sizeof(float));
#ifdef BOX
			gaussBlur(ScaleTemp[i], ScaleSpace[i * LEVEL_NUM_SS + j], sigma[j - 1]);
#else
			GaussianBlur(ScaleTemp[i], ScaleSpace[i * LEVEL_NUM_SS + j], sigma[j - 1]);
#endif
		}
	}
}

void CSIFT::BuildDOG()
{
	SubImage(ScaleSpace[1], ScaleSpace[0], DOG[0]);
	SubImage(ScaleSpace[2], ScaleSpace[1], DOG[1]);
	SubImage(ScaleSpace[3], ScaleSpace[2], DOG[2]);
	SubImage(ScaleSpace[4], ScaleSpace[3], DOG[3]);
	SubImage(ScaleSpace[6], ScaleSpace[5], DOG[4]);
	SubImage(ScaleSpace[7], ScaleSpace[6], DOG[5]);
	SubImage(ScaleSpace[8], ScaleSpace[7], DOG[6]);
	SubImage(ScaleSpace[9], ScaleSpace[8], DOG[7]);
	SubImage(ScaleSpace[11], ScaleSpace[10], DOG[8]);
	SubImage(ScaleSpace[12], ScaleSpace[11], DOG[9]);
	SubImage(ScaleSpace[13], ScaleSpace[12], DOG[10]);
	SubImage(ScaleSpace[14], ScaleSpace[13], DOG[11]);
	SubImage(ScaleSpace[16], ScaleSpace[15], DOG[12]);
	SubImage(ScaleSpace[17], ScaleSpace[16], DOG[13]);
	SubImage(ScaleSpace[18], ScaleSpace[17], DOG[14]);
	SubImage(ScaleSpace[19], ScaleSpace[18], DOG[15]);
}

void CSIFT::FindKeyPoint()
{
	feature.clear();
	feature_sub.clear();

	int x = 0;
	for (int i = 0; i < 4; i++)
	{
		int nWidth = width[i];
		int nHeight = height[i];
		int nWStep = wstep[i];


		for (int j = 0; j < 2; j++)
		{
			for (int r = 1; r < nHeight - 1; r++)
			{
				float* src0 = DOG[(i << 2) + j].GetPtr(r);
				float* src1 = DOG[(i << 2) + j + 1].GetPtr(r);
				float* src2 = DOG[(i << 2) + j + 2].GetPtr(r);

				for (int c = 1; c < nWidth - 1; c++)
				{
					if (fabs(src1[c]) < KEY_THRES)
						continue;
						
						if (src1[c] > src1[c - 1] && src1[c] > src1[c + 1] &&
							src1[c] > src1[c - 1 - nWStep] && src1[c] > src1[c - nWStep] && src1[c] > src1[c + 1 - nWStep] &&
							src1[c] > src1[c - 1 + nWStep] && src1[c] > src1[c + nWStep] && src1[c] > src1[c + 1 + nWStep] &&
							src1[c] > src0[c - 1] && src1[c] > src0[c] && src1[c] > src0[c + 1] &&
							src1[c] > src2[c - 1] && src1[c] > src2[c] && src1[c] > src2[c + 1] &&
							src1[c] > src0[c - 1 - nWStep] && src1[c] > src0[c - nWStep] && src1[c] > src0[c + 1 - nWStep] &&
							src1[c] > src2[c - 1 - nWStep] && src1[c] > src2[c - nWStep] && src1[c] > src2[c + 1 - nWStep] &&
							src1[c] > src0[c - 1 + nWStep] && src1[c] > src0[c + nWStep] && src1[c] > src0[c + 1 + nWStep] &&
							src1[c] > src2[c - 1 + nWStep] && src1[c] > src2[c + nWStep] && src1[c] > src2[c + 1 + nWStep]
							)
							feature.push_back(feature_t(i, j + 1, c, r, src1[c]));
						else if (src1[c] < src1[c - 1] && src1[c] < src1[c + 1] &&
							src1[c] < src1[c - 1 - nWStep] && src1[c] < src1[c - nWStep] && src1[c] < src1[c + 1 - nWStep] &&
							src1[c] < src1[c - 1 + nWStep] && src1[c] < src1[c + nWStep] && src1[c] < src1[c + 1 + nWStep] &&
							src1[c] < src0[c - 1] && src1[c] < src0[c] && src1[c] < src0[c + 1] &&
							src1[c] < src2[c - 1] && src1[c] < src2[c] && src1[c] < src2[c + 1] &&
							src1[c] < src0[c - 1 - nWStep] && src1[c] < src0[c - nWStep] && src1[c] < src0[c + 1 - nWStep] &&
							src1[c] < src2[c - 1 - nWStep] && src1[c] < src2[c - nWStep] && src1[c] < src2[c + 1 - nWStep] &&
							src1[c] < src0[c - 1 + nWStep] && src1[c] < src0[c + nWStep] && src1[c] < src0[c + 1 + nWStep] &&
							src1[c] < src2[c - 1 + nWStep] && src1[c] < src2[c + nWStep] && src1[c] < src2[c + 1 + nWStep]
							)
							feature.push_back(feature_t(i, j + 1, c, r, src1[c]));
				}
			}
		}
		x += width[i];
	}
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

bool CSIFT::SubPixel(feature_t& key, int nAdjustment = 2)
{
	bool needToAdjust = true;
	int x = (int)(key.x + 0.5f);
	int y = (int)(key.y + 0.5f);
	int l = (int)(key.level + 0.5f);
	float v = key.value;
	int o = (int)(key.octave);

	while (needToAdjust)
	{
		if (l <= 0 || l >= 3)
			return false;
		if (x <= 0 || x >= width[o] - 1)
			return false;
		if (y <= 0 || y >= height[o] - 1)
			return false;

		float dp = 0.0f; // dot product

		float H[9] = { DOG[(o << 2) + l - 1].GetAt(x, y) - 2 * DOG[(o << 2) + l].GetAt(x, y) + DOG[(o << 2) + l + 1].GetAt(x, y),  // below[y][x] - 2*current[y][x] + above[y][x]
			(DOG[(o << 2) + l + 1].GetAt(x, y + 1) - DOG[(o << 2) + l + 1].GetAt(x, y - 1) - (DOG[(o << 2) + l - 1].GetAt(x, y + 1) - DOG[(o << 2) + l - 1].GetAt(x, y - 1))) / 4, // 0.25 * (above[y+1][x] - above[y-1][x] - (below[y+1][x]-below[y-1][x]))
			(DOG[(o << 2) + l + 1].GetAt(x + 1, y) - DOG[(o << 2) + l + 1].GetAt(x - 1, y) - (DOG[(o << 2) + l - 1].GetAt(x + 1, y) - DOG[(o << 2) + l - 1].GetAt(x - 1, y))) / 4, // 0.25 * (above[y][x+1] - above[y][x-1] - (below[y][x+1]-below[y][x-1]))
			(DOG[(o << 2) + l + 1].GetAt(x, y + 1) - DOG[(o << 2) + l + 1].GetAt(x, y - 1) - (DOG[(o << 2) + l - 1].GetAt(x, y + 1) - DOG[(o << 2) + l - 1].GetAt(x, y - 1))) / 4, // H[1][0] = H[0][1]
			DOG[(o << 2) + l].GetAt(x, y - 1) - 2 * DOG[(o << 2) + l].GetAt(x, y) + DOG[(o << 2) + l].GetAt(x, y + 1),  // current[y - 1][x] - 2*current[y][x] + current[y + 1][x]
			(DOG[(o << 2) + l].GetAt(x + 1, y + 1) - DOG[(o << 2) + l].GetAt(x - 1, y + 1) - (DOG[(o << 2) + l].GetAt(x + 1, y - 1) - DOG[(o << 2) + l].GetAt(x - 1, y - 1))) / 4, // 0.25 * (current[y+1][x+1] - current[y+1][x-1] - (current[y-1][x+1]-current[y-1][x-1]))
			(DOG[(o << 2) + l + 1].GetAt(x + 1, y) - DOG[(o << 2) + l + 1].GetAt(x - 1, y) - (DOG[(o << 2) + l - 1].GetAt(x + 1, y) - DOG[(o << 2) + l - 1].GetAt(x - 1, y))) / 4, // H[2][0] = H[0][2]
			(DOG[(o << 2) + l].GetAt(x + 1, y + 1) - DOG[(o << 2) + l].GetAt(x - 1, y + 1) - (DOG[(o << 2) + l].GetAt(x + 1, y - 1) - DOG[(o << 2) + l].GetAt(x - 1, y - 1))) / 4, // H[2][1] = H[1][2]
			DOG[(o << 2) + l].GetAt(x - 1, y) - 2 * DOG[(o << 2) + l].GetAt(x, y) + DOG[(o << 2) + l].GetAt(x + 1, y) };  // current[y][x-1] - 2*current[y][x] + current[y1][x+1]


		float yong[6] = { DOG[(o << 2) + l - 1].GetAt(x, y) , DOG[(o << 2) + l + 1].GetAt(x, y),
			DOG[(o << 2) + l].GetAt(x, y + 1) , DOG[(o << 2) + l].GetAt(x, y - 1),
			DOG[(o << 2) + l].GetAt(x + 1, y) , DOG[(o << 2) + l].GetAt(x - 1, y) };


		float D[3] = { DOG[(o << 2) + l - 1].GetAt(x, y) - DOG[(o << 2) + l + 1].GetAt(x, y), // below[y][x] - above[y][x]
			DOG[(o << 2) + l].GetAt(x, y + 1) - DOG[(o << 2) + l].GetAt(x, y - 1),  // current[y+1][x] - current[y+1][x]
			DOG[(o << 2) + l].GetAt(x + 1, y) - DOG[(o << 2) + l].GetAt(x - 1, y) };  // current[y][x+1] - current[y][x-1]


		float B[3];

		Solve(H, D, B);


		dp = B[0] * D[0] + B[1] * D[1] + B[2] * D[2];


		if (abs(B[0]) > 0.5f || abs(B[1]) > 0.5f || abs(B[2]) > 0.5f)
		{
			if (nAdjustment == 0)
				return false;

			nAdjustment -= 1;

			double distSq = B[1] * B[1] + B[2] * B[2];

			if (distSq > 2.0)
				return false;

			x = (int)(x + B[2] + 0.5f);
			y = (int)(y + B[1] + 0.5f);
			l = (int)(l + B[0] + 0.5f);

			continue;
		}

		key.x = x + B[2];
		key.y = y + B[1];
		key.level = (int)(l + B[0] + 0.5f);
		key.value = DOG[(o << 2) + l].GetAt((int)key.x, (int)key.y) + 0.5f * dp;

		if (fabs(key.value) < 0.03f)
			return false;

		float dxx = DOG[o * 4 + l].GetAt(x - 1, y) - 2 * DOG[o * 4 + l].GetAt(x, y) + DOG[o * 4 + l].GetAt(x + 1, y);
		float dyy = DOG[o * 4 + l].GetAt(x, y - 1) - 2 * DOG[o * 4 + l].GetAt(x, y) + DOG[o * 4 + l].GetAt(x, y + 1);
		float dxy = (DOG[o * 4 + l].GetAt(x + 1, y + 1) - DOG[o * 4 + l].GetAt(x - 1, y + 1) - (DOG[o * 4 + l].GetAt(x + 1, y - 1) - DOG[o * 4 + l].GetAt(x - 1, y - 1))) / 4;

		float trH = dxx + dyy;
		float detH = dxx * dyy - dxy * dxy;

		float R = trH * trH / detH;
		if (detH <= 0.0f || R >= 12.1f)
		{
			return false;
		}
		return true;
	}
	return false;
}

void CSIFT::AccurateKey()
{
	for (itr = feature.begin(); itr != feature.end();)
	{
		if (!SubPixel(*itr))
		{
			itr = feature.erase(itr);
		}
		else
			itr++;
	}
}

void CSIFT::BuildGradient()
{
	int SSIdx, GrdIdx;
	float dx, dy;
	float* pSrc;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			SSIdx = i * 5 + j + 1;
			GrdIdx = (i << 1) + j;
			pSrc = ScaleSpace[SSIdx].GetPtr();

			for (int r = 1; r < height[i] - 1; r++)
			{
				pSrc = ScaleSpace[SSIdx].GetPtr(r);
				for (int c = 1; c < width[i] - 1; c++)
				{
					dx = pSrc[c + 1] - pSrc[c - 1];
					dy = pSrc[c + wstep[i]] - pSrc[c - wstep[i]];
					MagMap[GrdIdx][r * wstep[i] + c] = sqrtf(dx * dx + dy * dy);
					OriMap[GrdIdx][r * wstep[i] + c] = atan2(dy, dx);
				}
			}


			//top out line
			pSrc = ScaleSpace[SSIdx].GetPtr();
			for (int c = 1; c < width[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c + wstep[i]] - pSrc[c]);
				MagMap[GrdIdx][c] = sqrtf(dx * dx + dy * dy);
				OriMap[GrdIdx][c] = atan2(dy, dx);
			}

			//bottom out line
			pSrc = ScaleSpace[SSIdx].GetPtr(height[i] - 1);
			for (int c = 1; c < width[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c] - pSrc[c - wstep[i]]);
				MagMap[GrdIdx][(height[i]-1) * wstep[i] + c] = sqrtf(dx * dx + dy * dy); // no sqrt
				OriMap[GrdIdx][(height[i] - 1) * wstep[i] + c] = atan2(dy, dx);
			}

			//left out line
			pSrc = ScaleSpace[SSIdx].GetPtr();
			for (int r = 1; r < height[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * wstep[i] + 1] - pSrc[r * wstep[i]]);
				dy = pSrc[(r + 1) * wstep[i]] - pSrc[(r - 1) * wstep[i]];
				MagMap[GrdIdx][r * wstep[i]] = sqrtf(dx * dx + dy * dy); // no sqrt
				OriMap[GrdIdx][r * wstep[i]] = atan2(dy, dx);
			}

			//right out line
			pSrc = ScaleSpace[SSIdx].GetPtr(0, width[i] - 1);
			for (int r = 1; r < height[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * wstep[i]] - pSrc[r * wstep[i] - 1]);
				dy = pSrc[(r + 1) * wstep[i]] - pSrc[(r - 1) * wstep[i]];
				MagMap[GrdIdx][r * wstep[i] + width[i]-1] = sqrtf(dx * dx + dy * dy); // no sqrt
				OriMap[GrdIdx][r * wstep[i] + width[i]-1] = atan2(dy, dx);
			}


			//vertics
			pSrc = ScaleSpace[SSIdx].GetPtr();
			int cLast = width[i] - 1;
			int rLast = height[i] - 1;

			dx = 2.0f * (pSrc[1] - pSrc[0]);
			dy = 2.0f * (pSrc[wstep[i]] - pSrc[0]);
			MagMap[GrdIdx][0] = sqrtf(dx * dx + dy * dy); // no sqrt
			OriMap[GrdIdx][0] = atan2(dy, dx);

			dx = 2.0f * (pSrc[cLast] - pSrc[cLast - 1]);
			dy = 2.0f * (pSrc[wstep[i] + cLast] - pSrc[cLast]);
			MagMap[GrdIdx][cLast] = sqrtf(dx * dx + dy * dy); // no sqrt
			OriMap[GrdIdx][cLast] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * wstep[i] + 1] - pSrc[rLast * wstep[i]]);
			dy = 2.0f * (pSrc[rLast * wstep[i]] - pSrc[(rLast - 1) * wstep[i]]);
			MagMap[GrdIdx][rLast * wstep[i]] = sqrtf(dx * dx + dy * dy); // no sqrt
			OriMap[GrdIdx][rLast * wstep[i]] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * wstep[i] + cLast] - pSrc[rLast * wstep[i] + cLast - 1]);
			dy = 2.0f * (pSrc[rLast * wstep[i] + cLast] - pSrc[(rLast - 1) * wstep[i] + cLast]);
			MagMap[GrdIdx][rLast * wstep[i] + cLast] = sqrtf(dx * dx + dy * dy); // no sqrt
			OriMap[GrdIdx][rLast * wstep[i] + cLast] = atan2(dy, dx);
		}
	}
}

void CSIFT::JudgeOrientation(feature_t& key)
{
	float hist[36];
	memset(hist, 0, 36 * sizeof(float));

	int window = key.level - 1;
	int imageIdx = key.octave * 2 + window;
	int kx = (int)(key.x);
	int ky = (int)(key.y);
	int o = (int)(key.octave);
	if (kx > radius[window] && kx < width[o] - radius[window] && ky > radius[window] && ky < height[o] - radius[window])
	{
		for (int r = 0; r < wsize[window]; r++)
		{
			for (int c = 0; c < wsize[window]; c++)
			{
				int x = kx - radius[window] + c;
				int y = ky - radius[window] + r;
				int ori = (int)((OriMap[imageIdx][y * wstep[o] + x] * 180.0f) / M_PI);
				if (ori < 0) ori += 360;
				
				ori = ori / 10;
				
				hist[ori] += MagMap[imageIdx][y * wstep[o] + x] * weightMagnitude[window][r * wsize[window] + c];
			}
		}
	}
	else
	{
		for (int r = 0; r < wsize[window]; r++)
		{
			for (int c = 0; c < wsize[window]; c++)
			{
				int x = max(0, min(kx - radius[window] + c, width[o] - 1));
				int y = max(0, min(ky - radius[window] + r, height[o] - 1));
				int ori = (int)((OriMap[imageIdx][y * wstep[o] + x] * 180.0f) / M_PI);
				if (ori < 0) ori += 360;

				ori = ori / 10;

				hist[ori] += MagMap[imageIdx][y * wstep[o] + x] * weightMagnitude[window][r * wsize[window] + c];
			}
		}
	}

	float max = FLT_MIN;
	int theta = 0;

	for (int i = 0; i < 36; i++)
	{
		if (hist[i] > max)
		{
			max = hist[i];
			theta = i;
		}
	}

	key.orientation = theta * _2PI / 36.0f;

	for (int i = 0; i < 36; i++)
	{
		if (hist[i] > hist[theta] * 0.64f && i != theta)
		{
			feature_sub.push_back(feature_t(key, i * _2PI / 36.0f));
		}
	}
}

void CSIFT::AssignOrientation()
{
	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		JudgeOrientation(*itr);
	}

	feature.insert(feature.end(), feature_sub.begin(), feature_sub.end());
}

void CSIFT::DescriptKey()
{
	static float hist[8];

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		int o = itr->octave; // key octave
		int l = itr->level; // key level
		int kx = (int)itr->x; // key x
		int ky = (int)itr->y; // key y
		float kori = itr->orientation; // key orientation
		int idx = (o >> 1) + l - 1; // index of gradient array
		int pos = 0; // index of 128 dimension vector

		if (kx > 6 && kx < width[o] - 8 && ky > 6 && ky < height[o] - 8)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					int bigRow = i * 4;
					int bigCol = j * 4;
					memset(hist, 0, 8 * sizeof(float));
					for (int r = 0; r < 4; r++)
					{
						int posY = (ky - 7 + bigRow + r) * wstep[o];
						for (int c = 0; c < 4; c++)
						{
							int posX = idx + kx - 7 + bigCol + c;
							int ori = (int)((OriMap[idx][posY + posX] - kori) * 180 / M_PI);
							if (ori < 0) ori += 360;

							ori = ori / 45;

							hist[ori] += MagMap[idx][posY + posX] * weightDescript[(bigRow + r) * DES_SIZE + bigCol + c];
						}
					}
					float max = FLT_MIN;
					for (int k = 0; k < 8; k++)
					{
						if (hist[k] > max)
							max = hist[k];
					}
					for (int k = 0; k < 8; k++)
					{
						itr->vec[pos++] = hist[k]/* / (5.0f * max)*/;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					int bigRow = i * 4;
					int bigCol = j * 4;
					memset(hist, 0, 8 * sizeof(float));
					for (int r = 0; r < 4; r++)
					{
						int posY = min(0, max(ky - 7 + bigRow + r, height[o] - 1));
						for (int c = 0; c < 4; c++)
						{
							int posX = min(0, max(kx - 7 + bigCol + c, width[i] - 1));
							int ori = (int)((OriMap[idx][posY + posX] - kori) * 180 / M_PI);
							if (ori < 0) ori += 360;

							ori = ori / 45;

							hist[ori] += MagMap[idx][posY + posX] * weightDescript[(bigRow + r) * DES_SIZE + bigCol + c];
						}
					}
					float max = FLT_MIN;
					for (int k = 0; k < 8; k++)
					{
						if (hist[k] > max)
							max = hist[k];
					}
					for (int k = 0; k < 8; k++)
					{
						itr->vec[pos++] = hist[k]/* / (5.0f * max)*/;
					}
				}
			}
		}
	}
}

void DrawLine(CByteImage& canvas, int x1, int y1, int x2, int y2, BYTE R, BYTE G, BYTE B)
{
	ASSERT(canvas.GetChannel() == 3);

	int xs, ys, xe, ye;
	if (x1 == x2) // 수직선
	{
		if (y1 < y2) { ys = y1; ye = y2; }
		else { ys = y2; ye = y1; }
		for (int r = ys; r <= ye; r++)
		{
			canvas.GetAt(x1, r, 0) = B;
			canvas.GetAt(x1, r, 1) = G;
			canvas.GetAt(x1, r, 2) = R;
		}
		return;
	}

	double a = (double)(y2 - y1) / (x2 - x1); // 기울기
	int nHeight = canvas.GetHeight();

	if ((a>-1) && (a<1)) // 가로축에 가까움
	{
		if (x1 < x2) { xs = x1; xe = x2; ys = y1; ye = y2; }
		else { xs = x2; xe = x1; ys = y2; ye = y1; }
		for (int c = xs; c <= xe; c++)
		{
			int r = (int)(a*(c - xs) + ys + 0.5);
			if (r<0 || r >= nHeight)
				continue;
			canvas.GetAt(c, r, 0) = B;
			canvas.GetAt(c, r, 1) = G;
			canvas.GetAt(c, r, 2) = R;
		}
	}
	else // 세로축에 가까움
	{
		double invA = 1.0 / a;
		if (y1 < y2) { ys = y1; ye = y2; xs = x1; xe = x2; }
		else { ys = y2; ye = y1; xs = x2; xe = x1; }
		for (int r = ys; r <= ye; r++)
		{
			int c = (int)(invA*(r - ys) + xs + 0.5);
			if (r<0 || r >= nHeight)
				continue;
			canvas.GetAt(c, r, 0) = B;
			canvas.GetAt(c, r, 1) = G;
			canvas.GetAt(c, r, 2) = R;
		}
	}
}

float _CalcSIFTSqDist(const feature_t& k1, const feature_t& k2)
{
	const float *pk1, *pk2;

	pk1 = k1.vec;
	pk2 = k2.vec;

	float dif;
	float distsq = 0;
	for (int i = 0; i<128; i++)
	{
		dif = pk1[i] - pk2[i];
		distsq += dif * dif;
	}
	return distsq;
}


void CSIFT::KeyMatching()
{
	m_imageOut = CByteImage(width[1] + m_imageCmp.GetWidth(), max(height[1], m_imageCmp.GetHeight()), 3);
	int heightCmp = m_imageCmp.GetHeight();
	int wstep = m_imageIn.GetWStep();
	for (int r = 0; r < heightCmp; r++)
	{
		memcpy(m_imageOut.GetPtr(r), m_imageIn.GetPtr(height[1] - r - 1), wstep * sizeof(BYTE));
		memcpy(m_imageOut.GetPtr(r, width[1] * 3), m_imageCmp.GetPtr(r), m_imageCmp.GetWStep() * sizeof(BYTE));
	}
	for (int r = heightCmp; r < height[1]; r++)
	{
		memcpy(m_imageOut.GetPtr(r), m_imageIn.GetPtr(height[1] - r - 1), wstep * sizeof(BYTE));
	}

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		for (itr2 = feature_cmp.begin(); itr2 != feature_cmp.end(); itr2++)
		{
			float distSq = _CalcSIFTSqDist(*itr, *itr2);

			if (distSq < itr->minDist1) // 가장 가까운 특징점 갱신
			{
				itr->minDist2 = itr->minDist1;
				itr->minDist1 = distSq;
				itr->nearest = &(*itr2);
			}
			else if (distSq < itr->minDist2) // 두번째로 가까운 특징점 갱신
			{
				itr->minDist2 = distSq;
			}
		}
	}
	for (itr = feature_cmp.begin(); itr != feature_cmp.end(); itr++)
	{
		for (itr2 = feature.begin(); itr2 != feature.end(); itr2++)
		{
			float distSq = _CalcSIFTSqDist(*itr, *itr2);

			if (distSq < itr->minDist1) // 가장 가까운 특징점 갱신
			{
				itr->minDist2 = itr->minDist1;
				itr->minDist1 = distSq;
				itr->nearest = &(*itr2);
			}
			else if (distSq < itr->minDist2) // 두번째로 가까운 특징점 갱신
			{
				itr->minDist2 = distSq;
			}
		}
	}
	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		feature_t* nearkey = itr->nearest;
		if (&(*itr) == nearkey->nearest && itr->minDist1 < itr->minDist2 * DIST_THRES && nearkey->minDist1 < nearkey->minDist2 * DIST_THRES) // 유효한 대응 관계 판단
		{
			DrawLine(m_imageOut, itr->nx, itr->ny,
				nearkey->nx + width[1], nearkey->ny, 255, 0, 0);
		}
	}
	ShowImage(m_imageOut, "result");

}

void CSIFT::ShowKeyPoint()
{
	CByteImage imageOut = CByteImage(width[1], height[1], 3);
	BYTE* origin = m_imageIn.GetPtr();
	BYTE* out = imageOut.GetPtr();
	int wstep = m_imageIn.GetWStep();

	for (int i = 0; i < height[1]; i++)
	{
		memcpy(out + i * wstep, origin + (height[1] - 1 - i) * wstep, wstep);
	}

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		double size = pow(2, itr->octave - 1);
		int pos = (int)((itr->y * size * wstep) + (itr->x * size * imageOut.GetChannel()));
		out[pos] = 255;
		out[pos + 1] = 255;
		out[pos + 2] = 255;
	}
	ShowImage(imageOut, "keyPoints");
}

void CSIFT::CopyCmp()
{
	feature_cmp.clear();
	feature_cmp.resize(feature.size());
	std::copy(feature.begin(), feature.end(), feature_cmp.begin());
}