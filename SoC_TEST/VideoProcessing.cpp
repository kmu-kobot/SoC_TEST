#include "stdafx.h"
#include "VideoProcessing.h"

#include <math.h>
#include "MyImage.h"

///////////////////////////////////////////////////////////////////////////////
// 카메라 캡쳐 클래스 
///////////////////////////////////////////////////////////////////////////////
CWebCam::CWebCam() :
	m_hCam(NULL)
{

}

CWebCam::~CWebCam()
{
	if (m_hCam)
		StopCam();
}

bool CWebCam::StartCam(CWnd* wnd, int nWidth, int nHeight, BITMAPINFO& bmpInfo)
{
	// 캡쳐 영상을 출력할 윈도우의 크기 및 위치 조절
	m_hCam = capCreateCaptureWindow("Cam Capture", WS_CHILD|WS_VISIBLE,
									0, 0, nWidth, nHeight, wnd->m_hWnd, NULL);

	if (!capDriverConnect(m_hCam, 0))
	{
		AfxMessageBox("연결된 카메라를 찾을 수 없습니다.");
		return false;
	}

//	capDlgVideoFormat(m_hCam);
	capGetVideoFormat(m_hCam, &bmpInfo, sizeof(BITMAPINFO));

	// 캡쳐 영상의 크기를 희망 값으로 설정
	bmpInfo.bmiHeader.biWidth		= nWidth;
	bmpInfo.bmiHeader.biHeight		= nHeight;
	bmpInfo.bmiHeader.biSizeImage	= nWidth*nHeight*bmpInfo.bmiHeader.biBitCount/8;
	if (!capSetVideoFormat(m_hCam, &bmpInfo, sizeof(BITMAPINFO)))
	{
		// 설정에 실패하면 원래 값으로 되돌림
		capGetVideoFormat(m_hCam, &bmpInfo, sizeof(BITMAPINFO));
	}

	// 캡쳐 영상이 24비트 RGB가 될 수 있도록 설정
	bmpInfo.bmiHeader.biBitCount	= 24;
	bmpInfo.bmiHeader.biCompression	= BI_RGB;
	if (!capSetVideoFormat(m_hCam, &bmpInfo, sizeof(BITMAPINFO)))
	{
		// 설정에 실패하면 원래 값으로 되돌림
		capGetVideoFormat(m_hCam, &bmpInfo, sizeof(BITMAPINFO));
	}

	capOverlay	  (m_hCam,TRUE);
	capPreviewRate(m_hCam, 30);
	capPreview	  (m_hCam, TRUE);
	
	return true;
}


void CWebCam::StopCam()
{
	capCaptureAbort(m_hCam);
	capDriverDisconnect(m_hCam);
	m_hCam = NULL;
}


BOOL CWebCam::SetCallBackOnFrame(LRESULT (*fpProc)(HWND, LPVIDEOHDR))
{
	return capSetCallbackOnFrame(m_hCam, fpProc);
}


void YUY2ToRGB24(int nWidth, int nHeight, const BYTE* pYUY2, BYTE* pRGB24)
{
	int nSize = nHeight * (nWidth/2);

	int idxSrc = 0;
	int idxDst = 0;

	BYTE y1, u, y2, v;
	BYTE r, g, b;

	for (int i=0 ; i<nSize ; i++)
	{
		y1 = pYUY2[idxSrc  ];
		u  = pYUY2[idxSrc+1];
		y2 = pYUY2[idxSrc+2];
		v  = pYUY2[idxSrc+3];

		// 첫 번째 픽셀
		b = CLIP((76284*(y1-16) + 132252*(u-128)) >> 16);
		g = CLIP((76284*(y1-16) - 53281*(v-128) -  25625*(u-128) ) >> 16);
		r = CLIP((76284*(y1-16) + 104595*(v-128)) >> 16);

		pRGB24[idxDst  ] = b;
		pRGB24[idxDst+1] = g;
		pRGB24[idxDst+2] = r;

		idxDst += 3;

		// 두 번째 픽셀
		b = CLIP((76284*(y2-16) + 132252*(u-128)) >> 16);
		g = CLIP((76284*(y2-16) -  53281*(v-128) -  25625*(u-128)) >> 16);
		r = CLIP((76284*(y2-16) + 104595*(v-128)) >> 16);

		pRGB24[idxDst  ] = b;
		pRGB24[idxDst+1] = g;
		pRGB24[idxDst+2] = r;

		idxDst += 3;
		idxSrc += 4;
	}
}

//#define evne(x) (x % 2 == 0 ? x + 1 : x)

//void GaussianBlur(CByteImage& src, CByteImage& dst, double radius)
//{
//	int nWidth = src.GetWidth();
//	int nHeight = src.GetHeight();
//	int nChnnl = src.GetChannel();
//	int nWStep = src.GetWStep();
//	BYTE* pSrc = src.GetPtr();
//	BYTE* pDst = dst.GetPtr();
//
//	// 1차원 가우스 마스크 생성
//	int nHalf = max((radius * 6 - 1) / 2, 1);
//	nHalf = evne(nHalf);
//
//	int nMeanSize = nHalf * 2 + 1;
//
//	double* m_bufGss = new double[evne(nHalf * 2) * 2 + 1];
//
//	for (int n = 0; n <= nHalf; n++)
//	{
//		m_bufGss[nHalf - n] = m_bufGss[nHalf + n]
//			= exp(-n * n / (2 * radius*radius));
//	}
//
//	int r, c, l;
//	CDoubleImage tmpConv(nWidth, nHeight, nChnnl);
//	double* pTmp = tmpConv.GetPtr();
//
//	// 가로 방향 회선
//	for (r = 0; r<nHeight; r++) // 행 이동
//	{
//		for (c = 0; c<nWidth; c++) // 열 이동
//		{
//			for (l = 0; l<nChnnl; l++) // 채널 이동
//			{
//				double dSum = 0; // (마스크*픽셀) 값의 합
//				double dGss = 0; // 마스크 값의 합
//				for (int n = -nHalf; n <= nHalf; n++)
//				{
//					int px = c + n;
//
//					if (px >= 0 && px<nWidth)
//					{
//						dSum += (pSrc[nWStep * r + px + l] * m_bufGss[nHalf + n]);
//						dGss += m_bufGss[nHalf + n];
//					}
//				}
//				pTmp[nWStep * r + c + l] = dSum / dGss;
//			} // 채널 이동 끝
//		} // 열 이동 끝
//	} // 행 이동 끝
//
//	  // 세로 방향 회선
//	for (r = 0; r<nHeight; r++) // 행 이동
//	{
//		for (c = 0; c<nWidth; c++) // 열 이동
//		{
//			for (l = 0; l<nChnnl; l++) // 채널 이동
//			{
//				double dSum = 0; // 픽셀 값의 합
//				double dGss = 0; // 마스크 값의 합
//				for (int n = -nHalf; n <= nHalf; n++)
//				{
//					int py = r + n;
//
//					if (py >= 0 && py<nHeight)
//					{
//						int absN = abs(n);
//						dSum += pTmp[nWStep*py + nChnnl * c + l] * m_bufGss[nHalf + n];
//						dGss += m_bufGss[nHalf + n];
//					}
//				}
//				pDst[nWStep*r + nChnnl * c + l] = (BYTE)(dSum / dGss);
//			} // 채널 이동 끝
//		} // 열 이동 끝
//	} // 행 이동 끝
//}
//
//

//void gaussBlur(CByteImage& src, CByteImage& dst, int width, int height, double radius)
//{
//	int* bxs = boxesForGauss(radius, 3);
//	boxBlur(src, dst, width, height, (bxs[0] - 1) / 2);
//	boxBlur(dst, src, width, height, (bxs[1] - 1) / 2);
//	boxBlur(src, dst, width, height, (bxs[2] - 1) / 2);
//}
//
//void boxBlur(CByteImage& src, CByteImage& dst, int width, int height, double radius)
//{
//	int sizeSrc = height * src.GetWStep();
//	BYTE* pSrc = src.GetPtr();
//	BYTE* pDst = dst.GetPtr();
//
//	for (int i = 0; i<sizeSrc; i++)
//		pDst[i] = pSrc[i];
//	
//	boxBlurH(dst, src, width, height, radius);
//	boxBlurT(src, dst, width, height, radius);
//}
//
//void boxBlurH(CByteImage& src, CByteImage& dst, int width, int height, double radius)
//{
//	double iarr = 1 / (radius + radius + 1);
//	int nWStep = src.GetWStep();
//
//	BYTE* pSrc = src.GetPtr();
//	BYTE* pDst = dst.GetPtr();
//
//	for (int i = 0; i<height; i++) {
//
//		int ti = i * nWStep;
//		int li = ti;
//		int ri = ti + radius;
//
//		int fv = pSrc[ti];
//		int lv = pSrc[ti + width - 1];
//		double val = (radius + 1)*fv;
//
//		for (int j = 0; j<radius; j++)
//			val += pSrc[ti + j];
//
//		for (int j = 0; j <= radius; j++)
//		{
//			val += pSrc[ri++] - fv;
//			pDst[ti++] = round(val*iarr);
//		}
//
//		for (int j = radius + 1; j<width - radius; j++)
//		{ 
//			val += pSrc[ri++] - pSrc[li++];
//			pDst[ti++] = round(val*iarr);
//		}
//
//		for (int j = width - radius; j<width; j++) 
//		{ 
//			val += lv - pSrc[li++]; 
//			pDst[ti++] = round(val*iarr);
//		}
//	}
//}
//
//void boxBlurT(CByteImage& src, CByteImage& dst, int width, int height, double radius)
//{
//	double iarr = 1 / (radius + radius + 1);
//	int nWStep = src.GetWStep();
//	BYTE* pSrc = src.GetPtr();
//	BYTE* pDst = dst.GetPtr();
//
//	for (int i = 0; i<width; i++) {
//
//		int ti = i;
//		int li = ti;
//		int ri = ti + radius * nWStep;
//		int fv = pSrc[ti];
//		int lv = pSrc[ti + nWStep * (height - 1)];
//		double val = (radius + 1)*fv;
//
//		for (int j = 0; j<radius; j++)
//			val += pSrc[ti + j * nWStep];
//
//		for (int j = 0; j <= radius; j++)
//		{ 
//			val += pSrc[ri] - fv;
//			pDst[ti] = round(val*iarr);
//			ri += nWStep; ti += nWStep;
//		}
//
//		for (int j = radius + 1; j<height - radius; j++) 
//		{
//			val += pSrc[ri] - pSrc[li];
//			pDst[ti] = round(val*iarr); 
//			li += nWStep; ri += nWStep; ti += nWStep;
//		}
//
//		for (int j = height - radius; j<height; j++) 
//		{ 
//			val += lv - pSrc[li];
//			pDst[ti] = round(val*iarr); 
//			li += nWStep; ti += nWStep;
//		}
//	}
//}

//new
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

void gaussBlur(CFloatImage& src, CFloatImage& dst, float sigma)
{
	int* bxs = boxesForGauss(sigma, 3);
	int nWidth = src.GetWidth();
	int nHeight = src.GetHeight();
	int nWStep = src.GetWStep();
	float* pSrc = src.GetPtr();
	float* pDst = dst.GetPtr();
	boxBlur(pSrc, pDst, nWidth, nHeight, nWStep, (bxs[0] - 1) / 2);
	boxBlur(pDst, pSrc, nWidth, nHeight, nWStep, (bxs[1] - 1) / 2);
	boxBlur(pSrc, pDst, nWidth, nHeight, nWStep, (bxs[2] - 1) / 2);
}

void boxBlur(float* src, float* dst, int width, int height, int wstep, float radius)
{
	memcpy(dst, src, wstep*height*sizeof(float));
	boxBlurH(dst, src, width, height, wstep, radius);
	boxBlurT(src, dst, width, height, wstep, radius);
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
			dst[ti++] = val*iarr;
		}

		for (int j = radius + 1; j<width - radius; j++)
		{
			val += src[ri++] - src[li++];
			dst[ti++] = val*iarr;
		}

		for (int j = width - radius; j<width; j++)
		{
			val += lv - src[li++];
			dst[ti++] = val*iarr;
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
			dst[ti] = val*iarr;
			ri += wstep; ti += wstep;
		}

		for (int j = radius + 1; j<height - radius; j++)
		{
			val += src[ri] - src[li];
			dst[ti] = val*iarr;
			li += wstep; ri += wstep; ti += wstep;
		}

		for (int j = height - radius; j<height; j++)
		{
			val += lv - src[li];
			dst[ti] = val*iarr;
			li += wstep; ti += wstep;
		}
	}
}