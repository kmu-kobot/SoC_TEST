#pragma once

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
	int vec[128];			//	key's 128 dimension vector.

	Feature(int o, int l, float x, float y, float v)
		:octave(o), level(l), x(x), y(y), value(v) {};

	Feature(int o, int l, int x, int y, float v)
		:octave(o), level(l), x((float)x), y((float)y), value(v)
	{}
	
	Feature(const Feature& key, float ori)
		:octave(key.octave), level(key.level), x(key.x), y(key.y), value(key.value), orientation(ori)
	{}
} feature_t;

class CSIFT
{
public:
	CSIFT();
	~CSIFT();
	void SIFT();
	void Init(CByteImage& imageIn, int width, int height);	//	initialize image size, sigma, etc..
private:
	void BuildScaleSpace();		//	build scale space. it has 4 octave and 5 level.
	void BuildDOG();			//	build diffrence of gaussian. it has 4 octave and 4 level.
	void FindKeyPoint();		//	find key points in dog. key points are local max, local min value.
	bool SubPixel(feature_t& key, int nAdjustment);			//	find key point's real number position.
	void AccurategKey();		//	accurate key points.
	void BuildGradient();		//	build gradient map. magnitude and orientation. it have 4 octave and 2 level.
	void AssignOrientation();	//	assign key point's orientation.
	void DescriptKey();			//	descript key point's 128 dimension vector.

	CByteImage m_imageIn;		//	imput image
	CFloatImage ScaleSpace[20];	//	4 octave 5 level scale space.
	CFloatImage ScaleTemp[4];	//	each 4 octave's image buffer
	CFloatImage DOG[16];		//	4 octave 4 level diffrence of gaussian.
	float** MagMap[8];			//	4 octave 2 level magnitude map. use each octave's 1, 2 level.
	float** OriMap[8];			//	4 octave 2 level orientation map. use each octave's 1, 2 level.

	float* weightMagnitude[2];	//	gaussian weight function mask. each sigma are 1.5 * level 1's sigma, 1.5 * level 2's sigma.
	float* weightDescript;		//	gaussian weight function mask. sigma is 8, 16 * 16 size.

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
};

