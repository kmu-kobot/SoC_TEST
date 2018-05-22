#include "stdafx.h"
#include "Tracking.h"
#include "MyImageFunc.h"
#include "ImageFrameWndManager.h"
#include <math.h>


Tracking::Tracking(Point leftTop, Point rightBottom, int fs, int j, int f)
{
	this->pastPoint = 0;
	this->localCenter = {
		(rightBottom.x + leftTop.x) / 2,
		(rightBottom.y + leftTop.y) / 2
	};

	this->width = rightBottom.x - leftTop.x;
	this->height = rightBottom.y - leftTop.y;

	this->featureHSize = fs;
	this->featureSSize = j;
	this->featureVSize = f;

	this->isSetFeatureColor = false;
}


Tracking::~Tracking()
{
}


bool Tracking::tracking(CByteImage & originColorImage)
{
	CDoubleImage m_imageHSVAdj = RGB2HSV(originColorImage);
	
	CByteImage m_imageV = m_imageHSVAdj.GetChannelImg(0)*(255.0);//HSV로 바꾼것에서 V만따낸것.
	CByteImage m_imageS = m_imageHSVAdj.GetChannelImg(1)*(255.0);//HSV로 바꾼것에서 S만따낸것.
	CByteImage m_imageH = m_imageHSVAdj.GetChannelImg(2)*(255.0 / 360.0) + 0.5;//HSV로 바꾼것에서 H만따낸것.

	Point move[][2] = {
		{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 1, -1 },{ 0, -1 },{ -1, -1 },{ -1, 0 },{ -1, 1 },{ 0, 0 }
	};

	int repeat = 5;

	int nWidth = m_imageH.GetWidth();//640
	int nHeight = m_imageH.GetHeight();//480

	int rangeX = this->width / 2, rangeY = this->height / 2;

	int color;
	int checkArea[9] = { 1,1,1,1,1,1,1,1,1 };

	int ***yoloVideo = NULL;
	int sizeY = rangeY * 2 + repeat * 2;
	int sizeX = rangeX * 2 + repeat * 2;
	yoloVideo = new int**[sizeY];
	for (int i = 0; i < sizeY; i++) {
		yoloVideo[i] = new int*[sizeX];
		for (int j = 0; j < sizeX; j++) {
			yoloVideo[i][j] = new int[3];
		}
	}

	int startRow = this->localCenter.y - sizeY / 2, startCol = this->localCenter.x - sizeX / 2;
	int endRow = this->localCenter.y + sizeY / 2, endCol = this->localCenter.x + sizeX / 2;

	for (int r = startRow; r < endRow; r++) {
		BYTE *pInH, *pInS, *pInV;
		if (0 <= r && r <= nHeight) {
			pInH = m_imageH.GetPtr(r);
			pInS = m_imageS.GetPtr(r);
			pInV = m_imageV.GetPtr(r);
		}
		else { continue; }

		for (int c = startCol; c < endCol; c++) {
			if (0 <= c && c <= nWidth) {
				yoloVideo[r - startRow][c - startCol][0] = pInH[c];
				yoloVideo[r - startRow][c - startCol][1] = pInS[c];
				yoloVideo[r - startRow][c - startCol][2] = pInV[c];
			}
		}
	}

	int tmpH, tmpS, tmpV, tH, tS, tV, cntFlag;

	for (int i = 0; i < 9; i++) {

		startRow = (1 - move[i]->y) * repeat;
		startCol = (1 + move[i]->x) * repeat;

		endRow = rangeY * 2 + startRow;
		endCol = rangeX * 2 + startCol;

		for (int r = startRow; r < endRow; r++)
		{
			for (int c = startCol; c < endCol; c++)
			{
				tH = yoloVideo[r][c][0];
				tS = yoloVideo[r][c][1];
				tV = yoloVideo[r][c][2];

				if (tH == -1) {
					continue;
				}
				for (int k = 0; k < this->featureHSize; k++)
				{
					tmpH = this->featureH.at(k);
					tmpS = this->featureS.at(k);
					tmpV = this->featureV.at(k);

					cntFlag = 0;

					if (tH - 7 <= tmpH && tmpH <= tH + 7) {
						cntFlag += 1;
					}
					if (tS - 25 <= tmpS && tmpS <= tS + 25) {
						cntFlag += 1;
					}
					if (tV - 35 <= tmpV && tmpV <= tV + 35) {
						cntFlag += 1;
					}
					if (cntFlag == 3) {
						checkArea[i] += 1;
					}
				}
			}
		}
	}

	for (int i = 0; i < sizeY; i++) {
		delete[] yoloVideo[i];
	}

	delete[] yoloVideo;

	int maxIndex = 0, minIndex = 0;
	for (int i = 0; i < 9; i++) {
		if (checkArea[maxIndex] <= checkArea[i]) {
			maxIndex = i;
		}
		if (checkArea[minIndex] >= checkArea[i]) {
			minIndex = i;
		}
	}

	bool rFlag;
	bool test = (checkArea[maxIndex] / (((rangeX == 0) ? 1 : rangeX) * ((rangeY == 0) ? 1 : rangeY) * 4) * 100) <= 5;
	if (test) {
		int s = (pastPoint / checkArea[maxIndex] < 0.3) ? pastPoint / checkArea[maxIndex] * 100 * 2 : 0;

		this->localCenter.x = checkPointX(this->localCenter.x + (move[maxIndex]->x + s) * repeat);
		this->localCenter.y = checkPointY(this->localCenter.y - (move[maxIndex]->y + s) * repeat);
		pastPoint = checkArea[maxIndex];

		int width = 1;
		for (int i = -width; i < width; i++) {
			for (int j = -width; j < width; j++) {
				DrawLine(originColorImage, checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y + rangeY + j), checkPointX(this->localCenter.x + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
				DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + j), checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + j), 0, 255, 0);

				DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y - rangeY + j), checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
				DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + rangeY + j), checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + rangeY + j), 0, 255, 0);

				DrawLine(originColorImage, checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y + rangeY + j), checkPointX(this->localCenter.x - rangeX + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);
				DrawLine(originColorImage, checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y + rangeY + j), checkPointX(this->localCenter.x + rangeX + i), checkPointY(this->localCenter.y - rangeY + j), 0, 255, 0);

			}
		}

		ShowImage(originColorImage, "mmm");

		rFlag = true;
	}
	else {
		rFlag = false;
	}

	return rFlag;
}

void Tracking::setFeatureH(CByteImage & m_imageIn)
{
	this->isSetFeatureColor = true;
	this->videoHeight = m_imageIn.GetHeight();
	this->videoWidth = m_imageIn.GetWidth();

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

			for (int error = 0; error < 3; error++) {
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
				if ((int) rColor.size() <= this->featureHSize) {
					rColor.push_back((*iterResult).second.at(i));
				}
				else {
					this->featureH = rColor;
					this->featureHSize = rColor.size();
					return;
				}
			}
		}
	}

	this->featureH = rColor;
	this->featureHSize = rColor.size();
	return;
}

void Tracking::setFeatureS(CByteImage & m_imageIn)
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

	CByteImage m_imageS = m_imageHSVAdj.GetChannelImg(1)*(255.0);//HSV로 바꾼것에서 S만따낸것.

	long value;

	for (int r = start.x; r < end.x; r += 2) {
		BYTE *pIn = m_imageS.GetPtr(r);
		for (int c = start.y; c < end.y; c += 2) {
			value = pIn[c];

			chk = false;

			for (int error = 0; error < 20; error++) {
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
				if ((int)rColor.size() <= this->featureSSize) {
					rColor.push_back((*iterResult).second.at(i));
				}
				else {
					this->featureS = rColor;
					this->featureSSize = rColor.size();
					return;
				}
			}
		}
	}

	this->featureS = rColor;
	this->featureSSize = rColor.size();
	return;
}


void Tracking::setFeatureV(CByteImage & m_imageIn)
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

	CByteImage m_imageV = m_imageHSVAdj.GetChannelImg(0)*(255.0);//HSV로 바꾼것에서 V만따낸것.

	long value;

	for (int r = start.x; r < end.x; r += 2) {
		BYTE *pIn = m_imageV.GetPtr(r);
		for (int c = start.y; c < end.y; c += 2) {
			value = pIn[c];

			chk = false;

			for (int error = 0; error < 30; error++) {
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
				if ((int)rColor.size() <= this->featureVSize) {
					rColor.push_back((*iterResult).second.at(i));
				}
				else {
					this->featureV = rColor;
					this->featureVSize = rColor.size();
					return;
				}
			}
		}
	}

	this->featureV = rColor;
	this->featureVSize = rColor.size();
	return;
}

int Tracking::checkPointX(int p)
{
	if (3 <= p && p < this->videoWidth - 3) {
		return p;
	}
	else if (3 > p) {
		return 3;
	}
	else {
		return this->videoWidth - 4;
	}
}

int Tracking::checkPointY(int p)
{
	if (3 <= p && p < this->videoHeight - 3) {
		return p;
	}
	else if (3 > p) {
		return 3;
	}
	else {
		return this->videoHeight - 4;
	}
}

bool Tracking::getIsSetFeatureColor()
{
	return this->isSetFeatureColor;
}
