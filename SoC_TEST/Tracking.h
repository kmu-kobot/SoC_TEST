#pragma once

#include <vector>
#include <map>
#include "MyImage.h"

class Tracking
{
private:
	int pastPoint = 0;
	Point localCenter;

	int width;
	int height;

	int videoWidth;
	int videoHeight;

	int featureHSize;
	int featureSSize;
	int featureVSize;

	bool isSetFeatureColor;

	std::vector<int> featureH;
	std::vector<int> featureS;
	std::vector<int> featureV;

public:
	Tracking(Point, Point, int, int, int);
	~Tracking();

	bool tracking(CByteImage& originColorImage);

	void setFeatureH(CByteImage& m_imageIn);
	void setFeatureS(CByteImage& m_imageIn);
	void setFeatureV(CByteImage& m_imageIn);

	int checkPointX(int);
	int checkPointY(int);

	bool getIsSetFeatureColor();
};

