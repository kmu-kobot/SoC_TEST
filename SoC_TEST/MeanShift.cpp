#include "stdafx.h"
#include "MeanShift.h"
#include "MyImageFunc.h"
#include "ImageFrameWndManager.h"
#include <math.h>


MeanShift::MeanShift(Point leftTop, Point rightBottom, int fs)
{
	this->pastPoint = 0;
	this->localCenter = {
		((rightBottom.x + leftTop.x) + 320) / 2,
		((rightBottom.y + leftTop.y) + 240) / 2
	};

	this->width = rightBottom.x - leftTop.x;
	this->height = rightBottom.y - leftTop.y;

	this->featureColorSize = fs;
	this->isSetFeatureColor = false;
}


MeanShift::~MeanShift()
{
}


bool MeanShift::tracking(CByteImage & originColorImage)
{
	CDoubleImage m_imageHSVAdj = RGB2HSV(originColorImage);
	CByteImage imageIn = (m_imageHSVAdj.GetChannelImg(2)*(255.0 / 360.0) + 0.5);//HSV로 바꾼것에서 H만따낸것.
	CByteImage TestImage = imageIn;

	Point move[][2] = {
		{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 },{ -1, 0 },{ -1, 1 }
	};

	int repeat = 5;

	int nWidth = imageIn.GetWidth();//640
	int nHeight = imageIn.GetHeight();//480

	int rangeX = this->width / 2, rangeY = this->height / 2;

	int color;
	int checkArea[8] = { 1,1,1,1,1,1,1,1 };

	int **yoloVideo = new int*[rangeY * 2 + repeat * 2];
	for (int i = 0; i < rangeY * 2 + repeat * 2; i++) {
		yoloVideo[i] = new int[rangeX * 2 + repeat * 2];
		memset(yoloVideo[i], -1, sizeof(int)*(rangeX * 2 + repeat * 2));
	}

	int startRow = this->localCenter.y - rangeY - repeat, startCol = this->localCenter.x - rangeX - repeat;
	int endRow = this->localCenter.y + rangeY + repeat, endCol = this->localCenter.x + rangeX + repeat;

	for (int r = startRow; r < endRow; r++) {
		BYTE *pIn;
		if (0 <= r && r <= nHeight) { pIn = TestImage.GetPtr(r); }
		else { continue; }

		for (int c = startCol; c < endCol; c++) {
			if (0 <= c && c <= nWidth) {
				color = pIn[c];
				yoloVideo[r - startRow][c - startCol] = color;
			}
		}

	}

	int tmpColor, t;

	for (int i = 0; i < 8; i++) {

		startRow = (1 - move[i]->y) * repeat;
		startCol = (1 + move[i]->x) * repeat;

		endRow = rangeY * 2 + startRow;
		endCol = rangeX * 2 + startCol;

		for (int r = startRow; r < endRow; r+=2)
		{
			for (int c = startCol; c < endCol; c+=2)
			{
				t = yoloVideo[r][c];
				if (t == -1) {
					continue;
				}
				for (int k = 0; k < this->featureColorSize; k++)
				{
					tmpColor = this->featureColor.at(k);
					if (t - 10 <= tmpColor && tmpColor <= t + 10) {
						checkArea[i] += 1;
					}
				}
			}
		}
	}

	for (int i = 0; i < rangeY * 2 + repeat * 2; i++) {
		delete[] yoloVideo[i];
	}

	delete[] yoloVideo;

	int maxIndex = 0, sum = 0;
	for (int i = 0; i < 8; i++) {
		sum += checkArea[i];
		if (checkArea[maxIndex] < checkArea[i]) {
			maxIndex = i;
		}
	}

	int s = (pastPoint / checkArea[maxIndex] < 0.3) ? pastPoint / checkArea[maxIndex] * 100 * 2 : 0;

	bool isCheckFrequency = true; // !(0.75 <= checkArea[maxIndex] / ((rangeX * 2)*(rangeY * 2) * arrayLength) && checkArea[maxIndex] / ((rangeX * 2)*(rangeY * 2)*arrayLength) <= 1.2);

	if (isCheckFrequency && sum > (this->width * this->height)*0.7) {
		this->localCenter.x = checkPointX(this->localCenter.x + (move[maxIndex]->x + s) * repeat);
		this->localCenter.y = checkPointY(this->localCenter.y - (move[maxIndex]->y + s) * repeat);
	}
	else {
		return false;
	}

	pastPoint = checkArea[maxIndex];

	// sprintf_s(hsvValue, "[DIR] => %d, %d\t\n", maxIndex, checkArea[maxIndex]);
	// OutputDebugString(hsvValue);
	
	int width = 1;
	for (int i = -width; i < width; i++) {
		for (int j = -width; j < width; j++) {
			DrawLine(originColorImage, checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y + rangeY + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
			DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + j), 0, 255, 0);

			DrawLine(originColorImage, checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y + rangeY + j), 0, 255, 0);
			DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y + rangeY + j), 0, 255, 0);
			DrawLine(originColorImage, checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
			DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
		}
	}

	ShowImage(originColorImage, "mmm");

	return true;
}

void MeanShift::setFeatureColor(CByteImage & m_imageIn)
{
	Point start = {
		this->localCenter.x - (this->width / 2),
		this->localCenter.y - (this->height / 2)
	};

	Point end = {
		this->localCenter.x + (this->width / 2),
		this->localCenter.y + (this->height / 2)
	};

	std::map<long, int> m;
	std::map<int, std::vector<int>> result;
	bool chk = false;

	CDoubleImage m_imageHSVAdj = RGB2HSV(m_imageIn);

	DrawLine(m_imageIn, start.x, start.y, end.x, end.y, 255, 0, 0);
	DrawLine(m_imageIn, end.x, start.y, start.x, end.y, 255, 0, 0);
	ShowImage(m_imageIn, "target");

	CByteImage m_imageH = (m_imageHSVAdj.GetChannelImg(2)*(255.0 / 360.0) + 0.5);//HSV로 바꾼것에서 H만따낸것.

	long value;

	for (int r = start.x; r < end.x; r += 2) {
		BYTE *pIn = m_imageH.GetPtr(r);
		for (int c = start.y; c < end.y; c += 2) {
			value = pIn[c];

			chk = false;

			for (int error = 0; error < 5; error++) {
				if (m.find(value + error) == m.end()) {
					m[value + error]++;
					chk = true;
					continue;
				}
				else if (m.find(value - error) == m.end()) {
					m[value + error]++;
					chk = true;
					continue;
				}
			}
			if (!chk) m[value] = 1;
		}
	}

	int currentMax = 0;
	int argMax = 0;

	std::map<long, int>::iterator iter;
	for (iter = m.begin(); iter != m.end(); ++iter) {

		if (result.find((*iter).second) == result.end()) {
			result[(*iter).second].push_back((*iter).first);
		}
		else {
			result.insert({ (*iter).second, std::vector<int>((*iter).first) });
		}

		if ((*iter).second >= currentMax) {
			argMax = (*iter).first;
			currentMax = (*iter).second;
		}
	}

	std::map<int, std::vector<int>>::iterator iterResult;
	std::vector<int>::iterator iterVector;

	std::vector<int> rColor;
	for (iterResult = result.begin(); iterResult != result.end(); ++iterResult) {
		for (int i = (*iterResult).second.size() - 1; i >= 0; i--) {
			if ((*iterResult).second.at(i) != 0) {
				if (rColor.size() <= this->featureColorSize) {
					rColor.push_back((*iterResult).second.at(i));
				}
				else {
					this->featureColor = rColor;
					this->isSetFeatureColor = true;
					return;
				}
			}
		}
	}

	this->featureColor = rColor;
	this->isSetFeatureColor = true;
	return;
}

int MeanShift::checkPointX(int p)
{
	if (5 <= p && p <= 638) {
		return p;
	}
	else if (5 > p) {
		return 5;
	}
	else {
		return 637;
	}
}

int MeanShift::checkPointY(int p)
{
	if (3 <= p && p <= 478) {
		return p;
	}
	else if (3 > p) {
		return 3;
	}
	else {
		return 477;
	}
}

bool MeanShift::getIsSetFeatureColor()
{
	return this->isSetFeatureColor;
}