#pragma once
#include "MyImage.h"
#include "MyImageFunc.h"
#include "ImageFrameWndManager.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

#define BOXBLUR
#define VECTOR_NORM
#define NUM_OCTAVE 4
#define NUM_SCALE_SPACE_LEVEL 5
#define NUM_SCALE_SPACE (NUM_SCALE_SPACE_LEVEL * NUM_OCTAVE)
#define IDX_DOWNSAMPLING 2
#define NUM_DOG_LEVEL (NUM_SCALE_SPACE_LEVEL - 1)
#define NUM_DOG (NUM_DOG_LEVEL * NUM_OCTAVE)
#define SIGMA 1.6f
#define LOCAL_THRES 0.0f
#define CONSTRAINT_THRES 0.03f
#define R_THRES 10.0f
#define CORNER_THRES ((R_THRES + 1.0f) * (R_THRES + 1.0f) / R_THRES)
#define NUM_GRADIENT_LEVEL (NUM_DOG_LEVEL - 2)
#define NUM_GRADIENT (NUM_GRADIENT_LEVEL * NUM_OCTAVE)
#define FEATURE_RADIUS 1
#define _2PI (M_PI * 2.0)
#define COEF_MAG_SIGMA 1.5f
#define ORIENT_THRES 0.8f
#define DES_SIZE 16
#define DES_RADIUS ((float)DES_SIZE / 2.0f)
#define DES_SIGMA (DES_SIZE / 2)
#define DIST_THRES 0.8f

#ifndef __FEATURE__
#define __FEATURE__
typedef struct Feature
{
	int octave;
	int level;
	float scale, nscale, x, y, value;

	int nx, ny;

	float orientation;
	float vec[128];

	Feature* nearest;

	float minDist1;
	float minDist2;

	Feature() {}

	Feature(int o, int l, float x, float y, float v)
		:octave(o), level(l), x(x), y(y), value(v), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{
		if (!octave)
		{
			nx = (int)x >> 1;
			ny = (int)y >> 1;
		}
		else
		{
			nx = (int)x << (octave - 1);
			ny = (int)y << (octave - 1);
		}
		nscale = (float)l;
		scale = nscale * (float)(1 << o);
	}

	Feature(int o, int l, int x, int y, float v)
		:octave(o), level(l), x((float)x), y((float)y), value(v), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{
		if (!octave)
		{
			nx = (int)x >> 1;
			ny = (int)y >> 1;
		}
		else
		{
			nx = (int)x << (octave - 1);
			ny = (int)y << (octave - 1);
		}
		nscale = (float)l;
		scale = nscale * (float)(1 << o);
	}

	Feature(const Feature& key, float ori)
		:octave(key.octave), level(key.level), scale(key.scale), nscale(key.nscale), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(ori), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}

	Feature(const Feature& key)
		:octave(key.octave), level(key.level), scale(key.scale), nscale(key.nscale), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(key.orientation), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}
}feature_t;
#endif

class CSift
{
public:
	CSift();
	~CSift();
	void init(CByteImage imageIn);
	void detectFeature(CByteImage imageIn);

	void buildSample(CByteImage imageCmp);
	void keyMatching();

protected:
#ifndef BOXBLUR
	void initGauss();
#endif
	void initMagWeight();
	void initDesWeight();
	void buildScaleSpace();
	void showScaleSpace();
	void buildDOG();
	void showDOG();
	void buildFeature();
	void showFeature();
	bool subPixelize(feature_t& key, int nAdjustment);
	void accurateKey();
	void buildGradient();
	void judgeOrientation(feature_t& key);
	void assignOrientation();
	void descriptKey();
	void image2xB2F(CByteImage& src, CFloatImage& dst);
	void image2xDownSampleF2F(CFloatImage& src, CFloatImage& dst);
	void gaussBlur(CFloatImage& src, CFloatImage& dst, int level);

	void copyCmp();

protected:
	CByteImage m_imageIn;
	CByteImage m_imageInGray;
	CByteImage m_imageCmp;
	CByteImage m_imageOut;
	CFloatImage ScaleTemp[NUM_OCTAVE];
	CFloatImage ScaleSpace[NUM_SCALE_SPACE];
	CFloatImage DOG[NUM_DOG];

	int m_width[NUM_OCTAVE];
	int m_height[NUM_OCTAVE];
	int m_wstep[NUM_OCTAVE];
	int m_size[NUM_OCTAVE];

	float m_sigma[NUM_DOG_LEVEL];
	float sqrt2;

#ifndef BOXBLUR
	int maskRadius[NUM_DOG_LEVEL];
	int maskSize[NUM_DOG_LEVEL];
	float* gaussBlurMask[NUM_DOG_LEVEL];
#endif

	float* magMap[NUM_GRADIENT];
	float* oriMap[NUM_GRADIENT];

	float mag_Sigma[NUM_GRADIENT_LEVEL];
	int mag_Radius[NUM_GRADIENT_LEVEL];
	int mag_Size[NUM_GRADIENT_LEVEL];
	float* mag_Weight[NUM_GRADIENT_LEVEL];

	float des_Weight[DES_SIZE * DES_SIZE];

	std::vector<feature_t> feature;
	std::vector<feature_t> feature_sub;
	std::vector<feature_t> feature_sample;
	std::vector<feature_t>::iterator itr;
	std::vector<feature_t>::iterator itr2;
};

