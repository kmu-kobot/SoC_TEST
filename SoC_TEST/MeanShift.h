#pragma once

#include <vector>
#include <map>
#include "MyImage.h"

struct Point {
	int x;
	int y;
};

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

	void DrawLine(CByteImage& input, int x1, int y1, int x2, int y2, BYTE R, BYTE G, BYTE B);

};

