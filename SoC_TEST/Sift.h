#pragma once
#include "MyImage.h"
#include "MyImageFunc.h"
#include "ImageFrameWndManager.h"
#include "Feature.h"
#include "KDTree.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

#define BOXBLUR
#define VECTOR_NORM
#define NUM_SAMPLE 1
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
#define NUM_BIN 36
#define NUM_BIN_F (float)NUM_BIN
#define ORIENT_THRES 0.8f
#define DES_SIZE 16
#define DES_RADIUS ((float)DES_SIZE / 2.0f)
#define DES_SIGMA (DES_SIZE / 2)
#define DIST_THRES (0.6f * 0.6f)
#define KDTREE


class CSift
{
public:
	CSift();
	~CSift();
	void init(CByteImage imageIn);
	void detectFeature(CByteImage imageIn);

	void buildSample(CByteImage imageCmp, int i);
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

	void copyCmp(int i);

protected:
	CByteImage m_imageIn;
	CByteImage m_imageInGray;
	CByteImage m_imageCmp[NUM_SAMPLE];
	CByteImage m_imageOutH;
	CByteImage m_imageOutV;
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
	std::vector<feature_t> feature_sample[NUM_SAMPLE];
	std::vector<feature_t>::iterator itr;
	std::vector<feature_t>::iterator itr2;
	CKDTree sample[NUM_SAMPLE];
};