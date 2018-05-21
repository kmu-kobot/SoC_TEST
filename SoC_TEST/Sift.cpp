#include "stdafx.h"
#include "Sift.h"


CSift::CSift()
{
	sqrt2 = sqrtf(2);

	m_sigma[0] = SIGMA;
	for (int i = 1; i < NUM_DOG_LEVEL; ++i)
	{
		m_sigma[i] = m_sigma[i - 1] * sqrt2;
	}

#ifndef BOXBLUR
	for (int i = 0; i < NUM_DOG_LEVEL; ++i)
	{
		maskRadius[i] = m_sigma[i] * 6;
		maskSize[i] = (maskRadius[i] << 1) + 1;
	}
	initGauss();
#endif

	for (int i = 0; i < NUM_GRADIENT_LEVEL; ++i)
	{
		mag_Sigma[i] = COEF_MAG_SIGMA * m_sigma[i + 1];
		mag_Radius[i] = 6 * mag_Sigma[i];
		mag_Size[i] = (mag_Radius[i] << 1) + 1;
		mag_Weight[i] = new float[mag_Size[i] * mag_Size[i]];
	}

	initMagWeight();
	initDesWeight();
}


CSift::~CSift()
{
#ifndef BOXBLUR
	for (int i = 0; i < 4; ++i)
	{
		delete[] gaussBlurMask[i];
	}
#endif
	for (int i = 0; i < NUM_GRADIENT; ++i)
	{
		delete[] magMap[i];
		delete[] oriMap[i];
	}
	
	for (int i = 0; i < NUM_GRADIENT_LEVEL; ++i)
	{
		delete[] mag_Weight[i];
	}
}

void CSift::buildSample(CByteImage imageIn, int i)
{
	m_imageCmp[i] = imageIn;
	init(m_imageCmp[i]);
	detectFeature(m_imageCmp[i], std::to_string(i).c_str());
	copyCmp(i);
	sample[i].dimension = 128;
	sample[i].buildKDTree(feature_sample[i], sample[i].root, 0, feature_sample[i].size() - 1, 0);
}

void CSift::init(CByteImage imageIn)
{
	m_imageIn = imageIn;
	m_imageInGray = imageIn.GetChannel() == 1 ? imageIn : RGB2Gray(imageIn);
	m_width[0] = m_imageInGray.GetWidth() << 1;
	m_height[0] = m_imageInGray.GetHeight() << 1;
	for (int i = 1; i < NUM_OCTAVE; ++i)
	{
		m_width[i] = m_width[i - 1] >> 1;
		m_height[i] = m_height[i - 1] >> 1;
	}

	int idx_ScaleSpace, idx_DOG;
	for (int i = 0; i < NUM_OCTAVE; ++i)
	{
		idx_ScaleSpace = i * NUM_SCALE_SPACE_LEVEL;
		for (int j = 0; j < NUM_SCALE_SPACE_LEVEL; ++j)
		{
			ScaleSpace[idx_ScaleSpace + j] = CFloatImage(m_width[i], m_height[i]);
		}

		idx_DOG = i * NUM_DOG_LEVEL;
		for (int j = 0; j < NUM_DOG_LEVEL; ++j)
		{
			DOG[idx_DOG + j] = CFloatImage(m_width[i], m_height[i]); 
		}

		ScaleTemp[i] = CFloatImage(m_width[i], m_height[i]);
		m_wstep[i] = ScaleTemp[i].GetWStep();
		m_size[i] = m_height[i] * m_wstep[i];
	}

	int idx_Gradient;
	for (int i = 0; i < NUM_OCTAVE; ++i)
	{
		idx_Gradient = i * NUM_GRADIENT_LEVEL;
		for(int j = 0; j < NUM_GRADIENT_LEVEL; ++j)
		{
			magMap[idx_Gradient + j] = new float[m_height[i] * m_wstep[i]];
			oriMap[idx_Gradient + j] = new float[m_height[i] * m_wstep[i]];
		}
	}
}

#ifndef BOXBLUR
void CSift::initGauss()
{
	float sum, _2sigmaSquare, factor;
	for (int i = 0; i < NUM_DOG_LEVEL; ++i)
	{
		gaussBlurMask[i] = new float[maskSize[i]];
		sum = 0;
		_2sigmaSquare = 2.0f * m_sigma[i] * m_sigma[i];
		factor = (m_sigma[i] * sqrt2 * sqrt(M_PI));
		for (int n = -maskRadius[i]; n < maskRadius[i] + 1; ++n)
		{
			sum += gaussBlurMask[i][n + maskRadius[i]] = 
				expf(-(n*n) / _2sigmaSquare) / factor;
		}
	}
}
#endif

void CSift::initMagWeight()
{
	float dx, dy, dxSquare, dySquare, _2SigmaSquare, factor;
	for (int i = 0; i < NUM_GRADIENT_LEVEL; ++i)
	{
		_2SigmaSquare = 2 * mag_Sigma[i] * mag_Sigma[i];
		for (int r = 0; r < mag_Size[i]; ++r)
		{
			dy = r - mag_Radius[i];
			dySquare = dy * dy;
			for (int c = 0; c < mag_Size[i]; ++c)
			{
				dx = c - mag_Radius[i];
				dxSquare = dx * dx;
				mag_Weight[i][r * mag_Size[i] + c] = expf(-(dxSquare + dySquare) / _2SigmaSquare);
			}
		}
	}
}

void CSift::initDesWeight()
{
	float dx, dy, dxSquare, dySquare, _2SigmaSquare, factor;
		_2SigmaSquare = 2 * DES_SIGMA * DES_SIGMA;
		for (int r = 0; r < DES_SIZE; ++r)
		{
			dy = r - DES_RADIUS;
			dySquare = dy * dy;
			for (int c = 0; c < DES_SIZE; ++c)
			{
				dx = c - DES_RADIUS;
				dxSquare = dx * dx;
				des_Weight[r * DES_SIZE + c] = expf(-(dxSquare + dySquare) / _2SigmaSquare);
			}
		}
}

void CSift::detectFeature(CByteImage imageIn, const char* name)
{
	m_imageIn = imageIn;
	m_imageInGray = imageIn.GetChannel() == 1 ? imageIn : RGB2Gray(imageIn);
	buildScaleSpace();
	//showScaleSpace();
	buildDOG();
	//showDOG();
	buildFeature();
	accurateKey();
	showFeature(name);
	buildGradient();
	assignOrientation();
	descriptKey();
}

void CSift::buildScaleSpace()
{
	image2xB2F(m_imageInGray, ScaleTemp[0]);
	gaussBlur(ScaleTemp[0], ScaleSpace[0], 0);
	for (int i = 0; i < NUM_SCALE_SPACE_LEVEL - 1; ++i)
	{
		memcpy(ScaleTemp[0].GetPtr(), ScaleSpace[i].GetPtr(), m_size[0] * sizeof(float));
		gaussBlur(ScaleTemp[0], ScaleSpace[i + 1], i);
	}
	
	int idx_Octave;
	for (int i = 1; i < NUM_OCTAVE; ++i)
	{
		idx_Octave = i * NUM_SCALE_SPACE_LEVEL;
		image2xDownSampleF2F(ScaleSpace[(i - 1) * NUM_SCALE_SPACE_LEVEL + IDX_DOWNSAMPLING], ScaleSpace[i * NUM_SCALE_SPACE_LEVEL]);
		for (int j = 0; j < NUM_SCALE_SPACE_LEVEL - 1; ++j)
		{
			memcpy(ScaleTemp[i].GetPtr(), ScaleSpace[idx_Octave + j].GetPtr(), m_size[i] * sizeof(float));
			gaussBlur(ScaleTemp[i], ScaleSpace[idx_Octave + j + 1], j);
		}
	}
}

void CSift::showScaleSpace()
{
	for (int i = 0; i < NUM_SCALE_SPACE; ++i)
	{
		ShowImage(ScaleSpace[i], std::to_string(i).c_str());
	}
}

void CSift::buildDOG()
{
	int idx_SS_Octave, idx_DOG_Octave;
	for (int i = 0; i < NUM_OCTAVE; ++i)
	{
		idx_SS_Octave = i * NUM_SCALE_SPACE_LEVEL;
		idx_DOG_Octave = i * NUM_DOG_LEVEL;
		for (int j = 0; j < NUM_DOG_LEVEL; ++j)
		{
			SubImage(ScaleSpace[idx_SS_Octave + j + 1], ScaleSpace[idx_SS_Octave + j], DOG[idx_DOG_Octave + j]);
		}
	}
}

void CSift::showDOG()
{
	for (int i = 0; i < NUM_DOG; ++i)
	{
		ShowImage(DOG[i], std::to_string(i).c_str());
	}
}

void CSift::buildFeature()
{
	static float *top, *mid, *bot;
	feature.clear();
	feature_sub.clear();
	for (int i = 0; i < NUM_OCTAVE; ++i)
	{
		for (int j = 1; j < NUM_DOG_LEVEL - 1; ++j)
		{

			for (int r = 1; r < m_height[i] - 1; ++r)
			{
				top = DOG[i * NUM_DOG_LEVEL + j + 1].GetPtr(r);
				mid = DOG[i * NUM_DOG_LEVEL + j	   ].GetPtr(r);
				bot = DOG[i * NUM_DOG_LEVEL + j - 1].GetPtr(r);
				for (int c = 1; c < m_width[i] - 1; ++c)
				{
					float valN = mid[c] + LOCAL_THRES;
					float valP = mid[c] - LOCAL_THRES;
					if (valP > mid[c - 1] && valP > mid[c + 1] &&
						valP > mid[c - 1 - m_wstep[i]] && valP > mid[c - m_wstep[i]] && valP > mid[c + 1 - m_wstep[i]] &&
						valP > mid[c - 1 + m_wstep[i]] && valP > mid[c + m_wstep[i]] && valP > mid[c + 1 + m_wstep[i]] &&
						valP > bot[c - 1] && valP > bot[c] && valP > bot[c + 1] &&
						valP > top[c - 1] && valP > top[c] && valP > top[c + 1] &&
						valP > bot[c - 1 - m_wstep[i]] && valP > bot[c - m_wstep[i]] && valP > bot[c + 1 - m_wstep[i]] &&
						valP > top[c - 1 - m_wstep[i]] && valP > top[c - m_wstep[i]] && valP > top[c + 1 - m_wstep[i]] &&
						valP > bot[c - 1 + m_wstep[i]] && valP > bot[c + m_wstep[i]] && valP > bot[c + 1 + m_wstep[i]] &&
						valP > top[c - 1 + m_wstep[i]] && valP > top[c + m_wstep[i]] && valP > top[c + 1 + m_wstep[i]]
						)
						feature.push_back(feature_t(i, j + 1, c, r, mid[c]));
					else if (valN < mid[c - 1] && valN < mid[c + 1] &&
						valN < mid[c - 1 - m_wstep[i]] && valN < mid[c - m_wstep[i]] && valN < mid[c + 1 - m_wstep[i]] &&
						valN < mid[c - 1 + m_wstep[i]] && valN < mid[c + m_wstep[i]] && valN < mid[c + 1 + m_wstep[i]] &&
						valN < bot[c - 1] && valN < bot[c] && valN < bot[c + 1] &&
						valN < top[c - 1] && valN < top[c] && valN < top[c + 1] &&
						valN < bot[c - 1 - m_wstep[i]] && valN < bot[c - m_wstep[i]] && valN < bot[c + 1 - m_wstep[i]] &&
						valN < top[c - 1 - m_wstep[i]] && valN < top[c - m_wstep[i]] && valN < top[c + 1 - m_wstep[i]] &&
						valN < bot[c - 1 + m_wstep[i]] && valN < bot[c + m_wstep[i]] && valN < bot[c + 1 + m_wstep[i]] &&
						valN < top[c - 1 + m_wstep[i]] && valN < top[c + m_wstep[i]] && valN < top[c + 1 + m_wstep[i]]
						)
						feature.push_back(feature_t(i, j + 1, c, r, mid[c]));
				}
			}
		}
	}
}

void CSift::showFeature(const char* name)
{
	static int width, height;
	m_imageOut = m_imageIn;
	width = m_imageIn.GetWidth();
	height = m_imageIn.GetHeight();
	for (itr = feature.begin(); itr != feature.end(); ++itr)
	{
		for (int r = -FEATURE_RADIUS; r < FEATURE_RADIUS + 1; ++r)
		{
			for (int c = -FEATURE_RADIUS; c < FEATURE_RADIUS + 1; ++c)
			{
				m_imageOut.GetAt(IN_IMG(itr->nx + c, 0, width - 1), IN_IMG(itr->ny + r, 0, height - 1), 0) = 255;
				m_imageOut.GetAt(IN_IMG(itr->nx + c, 0, width - 1), IN_IMG(itr->ny + r, 0, height - 1), 1) = 255;
				m_imageOut.GetAt(IN_IMG(itr->nx + c, 0, width - 1), IN_IMG(itr->ny + r, 0, height - 1), 2) = 255;
			}
		}
	}
	ShowImage(m_imageOut, name);
}

bool solve(const float* H, const float* D, float* B)
{
	enum
	{
		a, b, c,
		d, e, f,
		g, h, i
	};
	float det = (H[a] * H[e] * H[i]) + (H[b] * H[f] * H[g]) + (H[c] * H[d] * H[h])
		- (H[c] * H[e] * H[g]) - (H[a] * H[f] * H[i]) - (H[b] * H[d] * H[i]); // det=a00*a11*a22 + a10*a21*a02 + a20*a01*a12 - a00*a21*a12 - a20*a11*a02 - a10*a01*a22

	if (!det)
		return false;

	float InverseH[9] = { det * (H[e] * H[i] - H[f] * H[h]), det * (H[c] * H[h] - H[b] * H[i]), det * (H[b] * H[f] - H[c] * H[e]),
		det * (H[f] * H[g] - H[d] * H[i]), det * (H[a] * H[i] - H[c] * H[g]), det * (H[c] * H[d] - H[a] * H[f]),
		det * (H[d] * H[h] - H[e] * H[g]), det * (H[b] * H[g] - H[a] * H[h]), det * (H[a] * H[e] - H[b] * H[d]) };

	B[0] = -(InverseH[a] * D[0] + InverseH[b] * D[1] + InverseH[c] * D[2]);
	B[1] = -(InverseH[d] * D[0] + InverseH[e] * D[1] + InverseH[f] * D[2]);
	B[2] = -(InverseH[g] * D[0] + InverseH[h] * D[1] + InverseH[i] * D[2]);

	return true;
}

bool CSift::subPixelize(feature_t& key, int nAdjustment = 2)
{
	int x, y, l, o;
	float v;
	x = (int)(key.x + 0.5f);
	y = (int)(key.y + 0.5f);
	l = key.level;
	o = key.octave;
	v = key.value;

	while (1)
	{
		if (l < 1 || l > NUM_GRADIENT_LEVEL)
			return false;
		if (x < 1 || x > m_width[o] - 2)
			return false;
		if (y < 1 || y > m_height[o] - 2)
			return false;

		CFloatImage& top = DOG[o * NUM_DOG_LEVEL + l + 1];
		CFloatImage& mid = DOG[o * NUM_DOG_LEVEL + l    ];
		CFloatImage& bot = DOG[o * NUM_DOG_LEVEL + l - 1];

		float dp; // dot product

		float D[3] = 
		{ 
			mid.GetAt(x + 1, y) - mid.GetAt(x - 1, y),	// current[y][x+1] - current[y][x-1]
			mid.GetAt(x, y + 1) - mid.GetAt(x, y - 1),  // current[y+1][x] - current[y+1][x]
			top.GetAt(x, y)     - bot.GetAt(x, y)		// below[y][x] - above[y][x]
		};  

		/*	dxx, dxy, dxs,
			dyx, dyy, dys,
			dsx, dsy, dss	*/
		float H[9] =
		{
			(mid.GetAt(x - 1, y) + mid.GetAt(x + 1, y)) / 2.0f - mid.GetAt(x, y),
			((mid.GetAt(x + 1, y + 1) - mid.GetAt(x - 1, y + 1)) - (mid.GetAt(x + 1, y - 1) - mid.GetAt(x - 1, y - 1))) / 4.0f,
			((top.GetAt(x + 1, y) - top.GetAt(x - 1, y)) - (bot.GetAt(x + 1, y) - bot.GetAt(x - 1, y))) / 4.0f,

			((mid.GetAt(x + 1, y + 1) - mid.GetAt(x + 1, y - 1)) - (mid.GetAt(x - 1, y + 1) - mid.GetAt(x - 1, y - 1))) / 4.0f,
			(mid.GetAt(x, y - 1) + mid.GetAt(x, y + 1)) / 2.0f - mid.GetAt(x, y),
			((top.GetAt(x, y + 1) - top.GetAt(x, y - 1)) - (bot.GetAt(x, y + 1) - bot.GetAt(x, y - 1))) / 4.0f,

			((top.GetAt(x + 1, y) - bot.GetAt(x + 1, y)) - (top.GetAt(x - 1, y) - bot.GetAt(x - 1, y))) / 4.0f,
			((top.GetAt(x, y + 1) - bot.GetAt(x, y - 1)) - (top.GetAt(x, y + 1) - bot.GetAt(x, y - 1))) / 4.0f,
			(bot.GetAt(x, y) + top.GetAt(x, y)) / 2.0f - mid.GetAt(x, y)
		};

		float B[3];

		solve(H, D, B);


		dp = B[0] * D[0] + B[1] * D[1] + B[2] * D[2];


		if (abs(B[0]) > 0.5f || abs(B[1]) > 0.5f || abs(B[2]) > 0.5f)
		{
			if (nAdjustment == 0)
				return false;

			nAdjustment -= 1;

			double distSq = B[0] * B[0] + B[1] * B[1];

			if (distSq > 2.0)
				return false;

			x = (int)(x + B[0] + 0.5f);
			y = (int)(y + B[1] + 0.5f);
			l = (int)(l + B[2] + 0.5f);

			continue;
		}

		key.x = (float)x + B[0];
		key.y = (float)y + B[1];
		key.nx = (int)(key.x * pow(2.0f, o - 1) + 0.5f);
		key.ny = (int)(key.y * pow(2.0f, o - 1) + 0.5f);
		key.nscale = (float)l + B[2];
		key.scale = key.nscale * (float)(1 << o);
		key.level = (int)(key.nscale + 0.5f);
		key.value = mid.GetAt((int)key.x, (int)key.y) + 0.5f * dp;

		if (key.level < 1 || l > NUM_GRADIENT_LEVEL)
			return false;
		if (key.x < 1 || key.x > m_width[o] - 2)
			return false;
		if (key.y < 1 || key.y > m_height[o] - 2)
			return false;

		if (fabs(key.value) < CONSTRAINT_THRES)
			return false;

		float trH = H[0] + H[4];
		float detH = H[0] * H[4] - H[1] * H[1];

		float r = trH * trH / detH;
		if (detH <= 0.0f || r >= CORNER_THRES)
		{
			return false;
		}
		return true;
	}
	return false;
}

void CSift::accurateKey()
{
	for (itr = feature.begin(); itr != feature.end();)
	{
		if (!subPixelize(*itr))
		{
			itr = feature.erase(itr);
		}
		else
			itr++;
	}
}

//not checked
void CSift::buildGradient()
{
	int idx_ScaleSpace, idx_Gradient, posY;
	float dx, dy;
	float *pSrc, *pOri, *pMag;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			idx_ScaleSpace = i * NUM_SCALE_SPACE_LEVEL + j + 1;
			idx_Gradient = i * NUM_GRADIENT_LEVEL + j;
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr();
			pOri = oriMap[idx_Gradient];
			pMag = magMap[idx_Gradient];

			for (int r = 1; r < m_height[i] - 1; r++)
			{
				pSrc = ScaleSpace[idx_ScaleSpace].GetPtr(r);
				posY = r * m_wstep[i];
				for (int c = 1; c < m_width[i] - 1; c++)
				{
					dx = pSrc[c + 1] - pSrc[c - 1];
					dy = pSrc[c + m_wstep[i]] - pSrc[c - m_wstep[i]];
					pMag[posY + c] = sqrtf(dx * dx + dy * dy);
					pOri[posY + c] = atan2(dy, dx);
				}
			}


			//top out line
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr();
			for (int c = 1; c < m_width[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c + m_wstep[i]] - pSrc[c]);
				pMag[c] = sqrtf(dx * dx + dy * dy);
				pOri[c] = atan2(dy, dx);
			}

			//bottom out line
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr(m_height[i] - 1);
			for (int c = 1; c < m_width[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c] - pSrc[c - m_wstep[i]]);
				pMag[(m_height[i] - 1) * m_wstep[i] + c] = sqrtf(dx * dx + dy * dy); // no sqrt
				pOri[(m_height[i] - 1) * m_wstep[i] + c] = atan2(dy, dx);
			}

			//left out line
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr();
			for (int r = 1; r < m_height[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * m_wstep[i] + 1] - pSrc[r * m_wstep[i]]);
				dy = pSrc[(r + 1) * m_wstep[i]] - pSrc[(r - 1) * m_wstep[i]];
				pMag[r * m_wstep[i]] = sqrtf(dx * dx + dy * dy); // no sqrt
				pOri[r * m_wstep[i]] = atan2(dy, dx);
			}

			//right out line
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr(0, m_width[i] - 1);
			for (int r = 1; r < m_height[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * m_wstep[i]] - pSrc[r * m_wstep[i] - 1]);
				dy = pSrc[(r + 1) * m_wstep[i]] - pSrc[(r - 1) * m_wstep[i]];
				pMag[r * m_wstep[i] + m_width[i] - 1] = sqrtf(dx * dx + dy * dy); // no sqrt
				pOri[r * m_wstep[i] + m_width[i] - 1] = atan2(dy, dx);
			}


			//vertics
			pSrc = ScaleSpace[idx_ScaleSpace].GetPtr();
			int cLast = m_width[i] - 1;
			int rLast = m_height[i] - 1;

			dx = 2.0f * (pSrc[1] - pSrc[0]);
			dy = 2.0f * (pSrc[m_wstep[i]] - pSrc[0]);
			pMag[0] = sqrtf(dx * dx + dy * dy); // no sqrt
			pOri[0] = atan2(dy, dx);

			dx = 2.0f * (pSrc[cLast] - pSrc[cLast - 1]);
			dy = 2.0f * (pSrc[m_wstep[i] + cLast] - pSrc[cLast]);
			pMag[cLast] = sqrtf(dx * dx + dy * dy); // no sqrt
			pOri[cLast] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * m_wstep[i] + 1] - pSrc[rLast * m_wstep[i]]);
			dy = 2.0f * (pSrc[rLast * m_wstep[i]] - pSrc[(rLast - 1) * m_wstep[i]]);
			pMag[rLast * m_wstep[i]] = sqrtf(dx * dx + dy * dy); // no sqrt
			pOri[rLast * m_wstep[i]] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * m_wstep[i] + cLast] - pSrc[rLast * m_wstep[i] + cLast - 1]);
			dy = 2.0f * (pSrc[rLast * m_wstep[i] + cLast] - pSrc[(rLast - 1) * m_wstep[i] + cLast]);
			pMag[rLast * m_wstep[i] + cLast] = sqrtf(dx * dx + dy * dy); // no sqrt
			pOri[rLast * m_wstep[i] + cLast] = atan2(dy, dx);
		}
	}
}

void CSift::judgeOrientation(feature_t& key)
{
	int size, radius, o, kx, ky, x, y, posY, ori, idx_Map, width, height, wstep;
	float hist[NUM_BIN], factor, dx, dy, *pMag, *pOri;

	o = key.octave;
	kx = (int)(key.x + 0.5f);
	ky = (int)(key.y + 0.5f);
	radius = (int)(3.0f * key.nscale);
	size = (radius << 1) | 1;
	factor = 2.0f * key.nscale * key.nscale;
	idx_Map = key.octave * NUM_GRADIENT_LEVEL + key.level - 1;
	pOri = oriMap[idx_Map];
	pMag = magMap[idx_Map];
	width = m_width[o];
	height = m_height[o];
	wstep = m_wstep[o];

	memset(hist, 0, NUM_BIN * sizeof(float));

	for (int r = -radius; r <= radius; ++r)
	{
		y = IN_IMG(ky + r, 0, height - 1);
		dy = y - key.y;
		posY = y * wstep;
		for (int c = -radius; c <= radius; ++c)
		{
			x = IN_IMG(kx + c, 0, width - 1);
			dx = x - key.x;
			ori = (int)(pOri[posY + x] * NUM_BIN_F / _2PI);
			if (ori < 0) ori += NUM_BIN;
			hist[ori] = pMag[posY + x] * expf(-(dx*dx + dy * dy) / factor);
		}
	}

	float max = hist[0];
	int theta = 0;
	for (int i = 1; i < NUM_BIN; ++i)
	{
		if (max < hist[i])
		{
			max = hist[i];
		}
	}

	key.orientation = (float)theta / NUM_BIN_F * _2PI;

	for (int i = 0; i < NUM_BIN; ++i)
	{
		if (max * ORIENT_THRES < hist[i] && i != theta)
		{
			feature_sub.push_back(feature_t(key, (float)i / NUM_BIN_F * _2PI));
		}
	}
}

void CSift::assignOrientation()
{
	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		judgeOrientation(*itr);
	}

	feature.insert(feature.end(), feature_sub.begin(), feature_sub.end());
}

void CSift::descriptKey()
{
	float hist[8];

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		int o = itr->octave; // key octave
		int l = itr->level; // key level
		int kx = (int)itr->x; // key x
		int ky = (int)itr->y; // key y
		float kori = itr->orientation; // key orientation
		int idx = (o >> 1) + l - 1; // index of gradient array
		int pos = 0; // index of 128 dimension vector

		float *pOri, *pMag;
		pOri = oriMap[idx];
		pMag = magMap[idx];

		//if (7.0f / 4.0f * M_PI < kori || kori <= 1.0f / 4.0f * M_PI)
		{
			for (int i = 0; i < 4; i++)
			{
				int bigRow = i * 4;
				for (int j = 0; j < 4; j++)
				{
					int bigCol = j * 4;
					memset(hist, 0, 8 * sizeof(float));
					for (int r = 0; r < 4; r++)
					{
						int posY = IN_IMG(ky - 7 + bigRow + r, 0, m_height[o] - 1) * m_wstep[o];
						for (int c = 0; c < 4; c++)
						{
							int posX = IN_IMG(kx - 7 + bigCol + c, 0, m_width[o] - 1);
							int ori = (int)((pOri[posY + posX] - kori) * 8.0f / _2PI);
							while (ori < 0) ori += 8;

							hist[ori] += pMag[posY + posX] * des_Weight[(bigRow + r) * DES_SIZE + bigCol + c];
						}
					}
					float max = FLT_MIN;
					for (int k = 0; k < 8; k++)
					{
						if (hist[k] > max)
							max = hist[k];
					}
					for (int k = 0; k < 8; k++)
					{
#ifdef VECTOR_NORM
						itr->vec[pos++] = hist[k] / (5.0f * max);
#else
						itr->vec[pos++] = hist[k];
#endif
					}
				}
			}
		}
//		else if (1.0f / 4.0f * M_PI < kori && kori <= 3.0f / 4.0f * M_PI)
//		{
//			for (int i = 0; i < 4; i++)
//			{
//				int bigCol = i * 4;
//				for (int j = 3; j > -1; j--)
//				{
//					int bigRow = j * 4;
//					memset(hist, 0, 8 * sizeof(float));
//					for (int r = 0; r < 4; r++)
//					{
//						int posY = IN_IMG(ky - 7 + bigRow + r, 0, m_height[o] - 1) * m_wstep[o];
//						for (int c = 0; c < 4; c++)
//						{
//							int posX = IN_IMG(kx - 7 + bigCol + c, 0, m_width[o] - 1);
//							int ori = (int)((pOri[posY + posX] - kori) * 8.0f / _2PI);
//							while (ori < 0) ori += 8;
//
//							hist[ori] += pMag[posY + posX] * des_Weight[(bigRow + r) * DES_SIZE + bigCol + c];
//						}
//					}
//					float max = FLT_MIN;
//					for (int k = 0; k < 8; k++)
//					{
//						if (hist[k] > max)
//							max = hist[k];
//					}
//					for (int k = 0; k < 8; k++)
//					{
//#ifdef VECTOR_NORM
//						itr->vec[pos++] = hist[k] / (5.0f * max);
//#else
//						itr->vec[pos++] = hist[k];
//#endif
//					}
//				}
//			}
//		}
//		else if (3.0f / 4.0f * M_PI < kori && kori <= 5.0f / 4.0f * M_PI)
//		{
//			for (int i = 3; i > -1; i--)
//			{
//				int bigRow = i * 4;
//				for (int j = 3; j > -1; j--)
//				{
//					int bigCol = j * 4;
//					memset(hist, 0, 8 * sizeof(float));
//					for (int r = 0; r < 4; r++)
//					{
//						int posY = IN_IMG(ky - 7 + bigRow + r, 0, m_height[o] - 1) * m_wstep[o];
//						for (int c = 0; c < 4; c++)
//						{
//							int posX = IN_IMG(kx - 7 + bigCol + c, 0, m_width[o] - 1);
//							int ori = (int)((pOri[posY + posX] - kori) * 8.0f / _2PI);
//							while (ori < 0) ori += 8;
//
//							hist[ori] += pMag[posY + posX] * des_Weight[(bigRow + r) * DES_SIZE + bigCol + c];
//						}
//					}
//					float max = FLT_MIN;
//					for (int k = 0; k < 8; k++)
//					{
//						if (hist[k] > max)
//							max = hist[k];
//					}
//					for (int k = 0; k < 8; k++)
//					{
//#ifdef VECTOR_NORM
//						itr->vec[pos++] = hist[k] / (5.0f * max);
//#else
//						itr->vec[pos++] = hist[k];
//#endif
//					}
//				}
//			}
//		}
//		else if (5.0f / 4.0f * M_PI < kori && kori <= 7.0f / 4.0f * M_PI)
//		{
//			for (int i = 3; i > -1; i--)
//			{
//				int bigCol = i * 4;
//				for (int j = 0; j < 4; j++)
//				{
//					int bigRow = j * 4;
//					memset(hist, 0, 8 * sizeof(float));
//					for (int r = 0; r < 4; r++)
//					{
//						int posY = IN_IMG(ky - 7 + bigRow + r, 0, m_height[o] - 1) * m_wstep[o];
//						for (int c = 0; c < 4; c++)
//						{
//							int posX = IN_IMG(kx - 7 + bigCol + c, 0, m_width[o] - 1);
//							int ori = (int)((pOri[posY + posX] - kori) * 8.0f / _2PI);
//							while (ori < 0) ori += 8;
//
//							hist[ori] += pMag[posY + posX] * des_Weight[(bigRow + r) * DES_SIZE + bigCol + c];
//						}
//					}
//					float max = FLT_MIN;
//					for (int k = 0; k < 8; k++)
//					{
//						if (hist[k] > max)
//							max = hist[k];
//					}
//					for (int k = 0; k < 8; k++)
//					{
//#ifdef VECTOR_NORM
//						itr->vec[pos++] = hist[k] / (5.0f * max);
//#else
//						itr->vec[pos++] = hist[k];
//#endif
//					}
//				}
//			}
//		}
	}
}
//not checked

bool CSift::keyMatching()
{
	float max = FLT_MIN;
	float matchingRatio;
	feature_t* nearest;
	if (matched < 0 || matched >= NUM_SAMPLE)
	{
		matched = 0;
	}

	for(int i = 0; i < NUM_SAMPLE; ++i)
	{
		feature_matched[i].clear();
		for (itr = feature_sample[i].begin(); itr != feature_sample[i].end(); itr++)
		{
			itr->minDist1 = itr->minDist2 = FLT_MAX;

#ifndef KDTREE
			for (itr2 = feature.begin(); itr2 != feature.end(); itr2++)
			{
				float distSq = _CalcSIFTSqDist(*itr, *itr2);

				if (distSq < itr->minDist1)
				{
					itr->minDist2 = itr->minDist1;
					itr->minDist1 = distSq;
					itr->nearest = &(*itr2);
				}
				else if (distSq < itr->minDist2)
				{
					itr->minDist2 = distSq;
				}
			}
		}
#else
		}
		for (itr = feature.begin(); itr != feature.end(); ++itr)
		{
			itr->minDist1 = itr->minDist2 = FLT_MAX;

			sample[i].NNSearch(*itr, sample[i].root, itr->nearest, itr->minDist1, itr->minDist2);
		}
#endif

		for (itr = feature_sample[i].begin(); itr != feature_sample[i].end(); ++itr)
		{
			if (itr->minDist1 < itr->minDist2 * DIST_THRES && itr->nearest->nearest == &*itr)
			{
				feature_matched[i].push_back(*itr);
			}
		}

		matchingRatio = (float)feature_matched[i].size() / (float)feature_sample[i].size();
		if (max < matchingRatio)
		{
			max = matchingRatio;
			matched = i;
		}
	}

	if (max < MATCHING_THRES)
	{
		return FALSE;
	}

	CByteImage& imageIn = m_imageIn.GetChannel() == 3 ? m_imageIn : Gray2RGB(m_imageIn);
	CByteImage& imageCmp = m_imageCmp[matched].GetChannel() == 3 ? m_imageCmp[matched] : Gray2RGB(m_imageCmp[matched]);

	int wstep = imageIn.GetWStep();
	int width_Cmp = imageCmp.GetWidth();
	int height_Cmp = imageCmp.GetHeight();
	int wstep_Cmp = imageCmp.GetWStep();
	int min_Height = min(m_height[1], height_Cmp);
	int min_Width = min(m_width[1], width_Cmp);
	m_imageOutH = CByteImage(m_width[1] + width_Cmp, max(m_height[1], height_Cmp), 3);
	m_imageOutV = CByteImage(max(m_width[1], width_Cmp), m_height[1] + height_Cmp, 3);

	//H
	for (int r = 0; r < min_Height; r++)
	{
		memcpy(m_imageOutH.GetPtr(r), imageCmp.GetPtr(r), wstep_Cmp * sizeof(BYTE));
		memcpy(m_imageOutH.GetPtr(r, width_Cmp), imageIn.GetPtr(r), wstep * sizeof(BYTE));
	}
	for (int r = min_Height; r < height_Cmp; ++r)
	{
		memcpy(m_imageOutH.GetPtr(r), imageCmp.GetPtr(r), wstep_Cmp * sizeof(BYTE));
	}
	for (int r = min_Height; r < m_height[1]; r++)
	{
		memcpy(m_imageOutH.GetPtr(r, width_Cmp), imageIn.GetPtr(r), wstep * sizeof(BYTE));
	}

	//V
	for (int r = 0; r < m_height[1]; ++r)
	{
		memcpy(m_imageOutV.GetPtr(r + height_Cmp), imageIn.GetPtr(r), wstep * sizeof(BYTE));
	}
	for (int r = 0; r < height_Cmp; ++r)
	{
		memcpy(m_imageOutV.GetPtr(r), imageCmp.GetPtr(r), wstep_Cmp * sizeof(BYTE));
	}

	for (itr = feature_matched[matched].begin(); itr != feature_matched[matched].end(); itr++)
	{
		nearest = itr->nearest;
			
		DrawLine(m_imageOutH, itr->nx, itr->ny,
			nearest->nx + width_Cmp, nearest->ny, 255, 0, 0);

		DrawLine(m_imageOutV, itr->nx, itr->ny,
			nearest->nx, nearest->ny + height_Cmp, 255, 0, 0);
	}
	ShowImage(m_imageOutH, "H");
	ShowImage(m_imageOutV, "V");

	return TRUE;
}

void CSift::copyCmp(int i)
{
	feature_sample[i].clear();
	feature_sample[i].resize(feature.size());
	std::copy(feature.begin(), feature.end(), feature_sample[i].begin());
}

void CSift::getObjectArea(Point& leftTop, Point& rightBottom)
{
	std::unordered_map<__int64, CntPoint> hist;
	int x, y;
	float theta, sinTheta, cosTheta;
	__int64 key;
	for (itr = feature_matched[matched].begin(); itr != feature_matched[matched].end(); ++itr)
	{
		theta = itr->nearest->orientation - itr->orientation;
		sinTheta = sinf(theta);
		cosTheta = cosf(theta);
		x = itr->nearest->nx - cosTheta * itr->nx + sinTheta * itr->ny;
		y = itr->nearest->ny + sinTheta * itr->nx - cosTheta * itr->ny;

		key = x << 30;
		key |= y >> 2;
		if (hist.find(key) == hist.end())
		{
			hist[key] = CntPoint{ x, y, 1 };
		}
		else
		{
			hist[key].x += x;
			hist[key].y += y;
			++hist[key].cnt;
		}
	}

	int max = INT_MIN;
	for (std::unordered_map<__int64, CntPoint>::iterator it = hist.begin(); it != hist.end(); ++it)
	{
		if (max < it->second.cnt)
		{
			key = it->first;
			max = it->second.cnt;
		}
	}

	leftTop.x = IN_IMG(hist[key].x / hist[key].cnt, 0, m_imageIn.GetWidth() - 1);
	leftTop.y = IN_IMG(hist[key].y / hist[key].cnt, 0, m_imageIn.GetHeight() - 1);

	rightBottom.x = IN_IMG(leftTop.x + m_imageCmp[matched].GetWidth() - 1, 0, m_imageIn.GetWidth() - 1);
	rightBottom.y = IN_IMG(leftTop.y + m_imageCmp[matched].GetHeight() - 1, 0, m_imageIn.GetHeight() - 1);

	DrawLine(m_imageOut, leftTop.x, leftTop.y,
		rightBottom.x, leftTop.y, 255, 0, 0);
	DrawLine(m_imageOut, leftTop.x, leftTop.y,
		leftTop.x, rightBottom.y, 255, 0, 0);
	DrawLine(m_imageOut, rightBottom.x, leftTop.y,
		rightBottom.x, rightBottom.y, 255, 0, 0);
	DrawLine(m_imageOut, leftTop.x, rightBottom.y,
		rightBottom.x, rightBottom.y, 255, 0, 0);

	ShowImage(m_imageOut, "area");

}

void CSift::image2xB2F(CByteImage& src, CFloatImage& dst)
{
	int width, height;
	float* pDst;
	width = dst.GetWidth();
	height = dst.GetHeight();

	for (int r = 0; r < height; ++r)
	{
		pDst = dst.GetPtr(r);
		for (int c = 0; c < width; ++c)
		{
			pDst[c] = src.CubicConvIntpD(c / 2.0f, r / 2.0f);
		}
	}
}

void CSift::image2xDownSampleF2F(CFloatImage& src, CFloatImage& dst)
{
	int width, height;
	float* pSrc, *pDst;
	width = dst.GetWidth();
	height = dst.GetHeight();
	for (int r = 0; r < height; ++r)
	{
		pSrc = src.GetPtr(r << 1);
		pDst = dst.GetPtr(r);
		for (int c = 0; c < width; ++c)
		{
			pDst[c] = pSrc[c << 1];
		}
	}
}

#ifndef BOXBLUR
void CSift::gaussBlur(CFloatImage& src, CFloatImage& dst, int level)
{
	int width, height, wstep;
	float* pSrc, * pDst, sum;
	width = src.GetWidth();
	height = src.GetHeight();
	wstep = src.GetWStep();
	dst.SetConstValue(0);
	for (int r = 0; r < height; ++r)
	{
		pSrc = src.GetPtr(r);
		pDst = dst.GetPtr(r);
		for (int c = 0; c < width; ++c)
		{
			sum = 0;
			for (int n = 0; n < maskSize[level]; ++n)
			{
				sum += gaussBlurMask[level][n] * pSrc[min(max(0, c + n - maskRadius[level]), width - 1)];
			}
			pDst[c] = sum;
		}
	}

	pDst = dst.GetPtr();
	for (int r = 0; r < height; ++r)
	{
		for (int c = 0; c < width; ++c)
		{
			sum = 0;
			for (int n = 0; n < maskSize[level]; ++n)
			{
				sum += gaussBlurMask[level][n] * pDst[min(max(0, r + n - maskRadius[level]), height - 1) * wstep + c];
			}
			pDst[r * wstep + c] = sum;
		}
	}
}


#else
// BOX BLUR

int* boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
	float widthIdeal = sqrt((12 * sigma*sigma / n) + 1);  // Ideal averaging filter width 

	int widthLow = floor(widthIdeal);
	if (widthLow % 2 == 0) widthLow--;

	int widthUp = widthLow + 2;

	float mIdeal = (12 * sigma*sigma - n * widthLow*widthLow - 4 * n*widthLow - 3 * n) / (-4 * widthLow - 4);
	int m = round(mIdeal);
	// var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );

	int* sizes = new int[n];
	for (int i = 0; i<n; i++) sizes[i] = (i<m ? widthLow : widthUp);
	return sizes;
}

void boxBlurH(float* src, float* dst, int width, int height, int wstep, float radius)
{
	float iarr = 1 / (radius + radius + 1);

	for (int i = 0; i<height; i++) {

		int ti = i * wstep;
		int li = ti;
		int ri = ti + radius;

		float fv = src[ti];
		float lv = src[ti + width - 1];
		float val = (radius + 1)*fv;

		for (int j = 0; j<radius; j++)
			val += src[ti + j];

		for (int j = 0; j <= radius; j++)
		{
			val += src[ri++] - fv;
			dst[ti++] = val * iarr;
		}

		for (int j = radius + 1; j<width - radius; j++)
		{
			val += src[ri++] - src[li++];
			dst[ti++] = val * iarr;
		}

		for (int j = width - radius; j<width; j++)
		{
			val += lv - src[li++];
			dst[ti++] = val * iarr;
		}
	}
}

void boxBlurT(float* src, float* dst, int width, int height, int wstep, float radius)
{
	float iarr = 1 / (radius + radius + 1);

	for (int i = 0; i<width; i++) {

		int ti = i;
		int li = ti;
		int ri = ti + radius * wstep;
		float fv = src[ti];
		float lv = src[ti + wstep * (height - 1)];
		float val = (radius + 1)*fv;

		for (int j = 0; j<radius; j++)
			val += src[ti + j * wstep];

		for (int j = 0; j <= radius; j++)
		{
			val += src[ri] - fv;
			dst[ti] = val * iarr;
			ri += wstep; ti += wstep;
		}

		for (int j = radius + 1; j<height - radius; j++)
		{
			val += src[ri] - src[li];
			dst[ti] = val * iarr;
			li += wstep; ri += wstep; ti += wstep;
		}

		for (int j = height - radius; j<height; j++)
		{
			val += lv - src[li];
			dst[ti] = val * iarr;
			li += wstep; ti += wstep;
		}
	}
}

void boxBlur(float* src, float* dst, int width, int height, int wstep, float radius)
{
	memcpy(dst, src, wstep*height * sizeof(float));
	boxBlurH(dst, src, width, height, wstep, radius);
	boxBlurT(src, dst, width, height, wstep, radius);
}

void CSift::gaussBlur(CFloatImage& src, CFloatImage& dst, int i)
{
	int width, height, wstep, *bxs;
	float *pSrc, *pDst;
	width = src.GetWidth();
	height = src.GetHeight();
	wstep = src.GetWStep();
	dst.SetConstValue(0);
	bxs = boxesForGauss(m_sigma[i], 3);
	pSrc = src.GetPtr();
	pDst = dst.GetPtr();
	boxBlur(pSrc, pDst, width, height, wstep, (bxs[0] - 1) / 2);
	boxBlur(pDst, pSrc, width, height, wstep, (bxs[1] - 1) / 2);
	boxBlur(pSrc, pDst, width, height, wstep, (bxs[2] - 1) / 2);
}
#endif


