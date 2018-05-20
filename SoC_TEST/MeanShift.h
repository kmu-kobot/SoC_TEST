#pragma once

#include <vector>
#include <map>
#include "MyImage.h"

class MeanShift
{
private:
	int pastPoint = 0;
	Point localCenter;

	int featureColorSize;
	bool isSetFeatureColor;
	std::vector<int> featureColor;

public:
	MeanShift(Point, int);
	~MeanShift();

	void tracking(CByteImage& originColorImage);
	void setFeatureColor(CByteImage& m_imageIn, Point start, Point end);

	int checkPointX(int);
	int checkPointY(int);

	bool getIsSetFeatureColor();
};

