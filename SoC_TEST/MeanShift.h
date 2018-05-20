#pragma once

#include <vector>
#include <map>
#include "MyImage.h"

class MeanShift
{
private:
	int pastPoint = 0;
	Point localCenter;
	int width;
	int height;

	int featureColorSize;
	bool isSetFeatureColor;
	std::vector<int> featureColor;

public:
	MeanShift(Point, Point, int);
	~MeanShift();

	bool tracking(CByteImage& originColorImage);
	void setFeatureColor(CByteImage& m_imageIn);

	int checkPointX(int);
	int checkPointY(int);

	bool getIsSetFeatureColor();
};

