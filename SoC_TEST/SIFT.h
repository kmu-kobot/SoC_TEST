#pragma once
#define STATIC_SIZE

#include <vector>

#include "MyImage.h"

typedef struct Feature
{
	int octave;				//	image ocvate.
	int level;				//	blured level.
	float x;				//	local x.
	float y;				//	local y.
	float value;			//	pixel value.

	int nx;					//	normaized x;
	int ny;					//	normaized y;

	float orientation;		//	feature's orientation.
	float vec[128];			//	key's 128 dimension vector.

	Feature* nearest;

	float minDist1;
	float minDist2;

	Feature()
	{}

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
	}
	
	Feature(const Feature& key, float ori)
		:octave(key.octave), level(key.level), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(ori), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}

	Feature(const Feature& key)
		:octave(key.octave), level(key.level), x(key.x), y(key.y), value(key.value), nx(key.nx), ny(key.ny), orientation(key.orientation), minDist1(FLT_MAX), minDist2(FLT_MAX)
	{}
} feature_t;

class CSIFT
{
public:
	CSIFT();
	~CSIFT();
	void SIFT(CByteImage& imageIn);
	void Init(int width, int height);	//	initialize image size, sigma, etc..
	void BuildCmp(CByteImage& image);
	void KeyMatching();			//	match key points.
private:
	void BuildScaleSpace();		//	build scale space. it has 4 octave and 5 level.
	void BuildDOG();			//	build diffrence of gaussian. it has 4 octave and 4 level.
	void FindKeyPoint();		//	find key points in dog. key points are local max, local min value.
	bool SubPixel(feature_t& key, int nAdjustment);			//	find key point's real number position.
	void AccurateKey();			//	accurate key points.
	void BuildGradient();		//	build gradient map. magnitude and orientation. it have 4 octave and 2 level.
	void JudgeOrientation(feature_t& key);	//	judge orientation of key point.
	void AssignOrientation();	//	assign key point's orientation.
	void DescriptKey();			//	descript key point's 128 dimension vector.
	void ShowKeyPoint();
	void CopyCmp();

	CByteImage m_imageIn;		//	input image
	CByteImage m_imageInGray;	//	gray imput image
	CByteImage imageInX2;		//	input image double size.
	CByteImage m_imageOut;
	CByteImage m_imageCmp;
	CFloatImage ScaleSpace[20];	//	4 octave 5 level scale space.
	CFloatImage ScaleTemp[4];	//	each 4 octave's image buffer
	CFloatImage DOG[16];		//	4 octave 4 level diffrence of gaussian.
	float* MagMap[8];			//	4 octave 2 level magnitude map. use each octave's 1, 2 level.
	float* OriMap[8];			//	4 octave 2 level orientation map. use each octave's 1, 2 level.

	float* weightMagnitude[2];	//	gaussian weight function mask. each sigma are 1.5 * level 1's sigma, 1.5 * level 2's sigma.
	float weightDescript[256];	//	gaussian weight function mask. sigma is 8, 16 * 16 size.

	int width[4];				//	each octave's width;
	int height[4];				//	each octave's height;
	int wstep[4];				//	each octave's wstep;
	int size[4];				//	each octave's image size;
	float sigma[4];				//	each level's sigma; it dosen't need level 0's sigma
	float magSigma[2];			//	each sigma of magnitude weight kernel.
	int radius[2];				//	each level weight mask's radius.
	int wsize[2];				//	each level weight mask's window size.

	std::vector<feature_t> feature;			//	main key point vector.
	std::vector<feature_t> feature_sub;		//	sub key point vector. if any key point has another orientation, push back in this vector.
	std::vector<feature_t>::iterator itr;	//	key point vector's iterator.
	std::vector<feature_t>::iterator itr2;	//	
	std::vector<feature_t>::iterator itr3;	//	
	std::vector<feature_t>::iterator itr4;	//	
	std::vector<feature_t> feature_cmp;
};

