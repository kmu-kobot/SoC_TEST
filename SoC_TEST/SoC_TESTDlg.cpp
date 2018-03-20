
// SoC_TESTDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SoC_TEST.h"
#include "SoC_TESTDlg.h"
#include "afxdialogex.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <string>
#include <list>
#include "MyImage.h"
#include "ImageFrameWndManager.h"
#include "LoadImageFromFileDialog.h"
#include "MyImageFunc.h"
#include "VideoProcessing.h"
#include "SIFT.h"

#define CLASS

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSoCTESTDlg 대화 상자



CSoCTESTDlg::CSoCTESTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SOC_TEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSoCTESTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoCTESTDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CAM_START, &CSoCTESTDlg::OnBnClickedButtonCamStart)
	ON_BN_CLICKED(IDC_BUTTON_CAM_STOP, &CSoCTESTDlg::OnBnClickedButtonCamStop)
END_MESSAGE_MAP()


// CSoCTESTDlg 메시지 처리기

BOOL CSoCTESTDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CSoCTESTDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSoCTESTDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) >> 1;
		int y = (rect.Height() - cyIcon + 1) >> 1;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSoCTESTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//typedef struct Feature
//{
//	int octave;
//	int scale; // blured scale
//	float x; // loacl x
//	float y; // loack y
//	float value;
//
//	int nx; // normarized x
//	int ny; // normarized y
//
//	float orientation;
//	int vec[128];
//
//	Feature(int o, int s, float x, float y, float v)
//		:octave(o), scale(s), x(x), y(y), value(v) 
//	{
//	}
//
//	Feature(int o, int s, int x, int y, float v)
//		:octave(o), scale(s), x((float)x), y((float)y), value(v)
//	{
//	}
//
//	Feature(const Feature& key, float ori)
//		:octave(key.octave), scale(key.scale), x(key.x), y(key.y), value(key.value), orientation(ori)
//	{
//	}
//} feature_t;


CByteImage gImageBuf;
CByteImage gImageOut;
CByteImage gImageBufGray;
////CByteImage gImageIn;
BITMAPINFO gBmpInfo;
//CFloatImage gScaleTemp[4];
//CFloatImage gScaleSpace[20];
//CByteImage gScaleOriginal[4];
//CFloatImage gDOG[16];
//CFloatImage gGradientMagnitude[8];
//CFloatImage gGradientOrientation[8];
////CByteImage gKeyPoint[8];
//std::vector<feature_t> feature;
//std::vector<feature_t> feature_sub;
//std::vector<feature_t>::iterator itr;
//#define NUM_CHANNEL 1
//int gWidth[4];
//int gHeight[4];
//int gWStep[4];
//int gSize[4];
//float gSigma[4] = { 1.6f, 1.6f * (float)sqrt(2), 3.2f, 3.2f * (float)sqrt(2) };
//float* gWeight[2];
//int gRadius[2];
//int gWindowSize[2];

CSIFT m_SIFT;


#ifndef CLASS
void BuildScaleSpace()
{



	for (int r = 0; r < gHeight[0]; r++)
	{
		BYTE* pDst = gScaleOriginal[0].GetPtr(gHeight[0] - r - 1);
		for (int c = 0; c < gWidth[0]; c++)
		{
			pDst[c] = gImageBufGray.BiLinearIntp(c >> 1, r >> 1);
		}
	}

	BYTE* pOriginal = gScaleOriginal[0].GetPtr();
	float* pTemp = gScaleTemp[0].GetPtr();

	for (int pos = 0; pos < gSize[0]; pos++)
	{
		pTemp[pos] = (float)pOriginal[pos];
	}

	gaussBlur(gScaleTemp[0], gScaleSpace[0], gSigma[0]);

	for (int i = 0; i < 4; i++)
	{
		memcpy(gScaleTemp[0].GetPtr(), gScaleSpace[i].GetPtr(), gSize[0] * sizeof(float));
		gaussBlur(gScaleTemp[0], gScaleSpace[i + 1], gSigma[i]);
	}

	for (int i = 1; i < 4; i++)
	{
		for (int r = 0; r < gHeight[i]; r++)
		{
			float* pDst = gScaleSpace[i * 5].GetPtr(r);
			float* pSrc = gScaleSpace[i * 5 - 3].GetPtr(2 * r);
			for (int c = 0; c < gWidth[i]; c++)
			{
				pDst[c] = pSrc[2 * c];
			}
		}

		for (int j = 1; j < 5; j++)
		{
			memcpy(gScaleTemp[i].GetPtr(), gScaleSpace[i * 5 + j - 1].GetPtr(), gSize[i] * sizeof(float));
			gaussBlur(gScaleTemp[i], gScaleSpace[i * 5 + j], gSigma[j - 1]);
		}
	}

	//for (int r = 0; r <gHeight[0]; r++)
	//{
	//	BYTE* pDst = gScaleTemp[0].GetPtr(gHeight[0] - r - 1);
	//	for (int c = 0; c < gWidth[0]; c++)
	//	{
	//		pDst[c] = gImageBufGray.CubicConvIntp(c >> 1, r >> 1);
	//	}
	//}

	////ShowImage(gScaleTemp[0], "1/2 Scale Original Image");
	//gaussBlur(gScaleTemp[0], gScaleSpace[0], gSigma[0]);
	/////ShowImage(gScaleTemp[0], std::to_string(0).c_str());

	//for (int i = 0; i < 4; i++)
	//{
	//	memcpy(gScaleTemp[0].GetPtr(), gScaleSpace[i].GetPtr(), gSize[0]);
	//	gaussBlur(gScaleTemp[0], gScaleSpace[i + 1], gSigma[i]);
	//	//ShowImage(gScaleSpace[i], std::to_string(i + 1).c_str());
	//}

	//for (int i = 1; i < 4; i++)
	//{
	//	for (int j = 0; j < 3; j++)
	//	{
	//		for (int r = 0; r < gHeight[i]; r++)
	//		{
	//			BYTE* pDst = gScaleSpace[i * 5 + j].GetPtr(r);
	//			BYTE* pSrc = gScaleSpace[(i - 1) * 5 + j + 2].GetPtr(2 * r);
	//			for (int c = 0; c < gWidth[i]; c++)
	//			{
	//				pDst[c] = pSrc[2 * c];
	//			}
	//		}
	//	}

	//	for (int j = 2; j < 4; j++)
	//	{
	//		memcpy(gScaleTemp[i].GetPtr(), gScaleSpace[i * 5 + j].GetPtr(), gSize[i]);
	//		gaussBlur(gScaleTemp[i], gScaleSpace[i * 5 + j + 1], gSigma[j]);
	//	}
	//}

	//for (int i = 0; i < 20; i++)
	//{
	//	std::string str;
	//	switch (i/5)
	//	{
	//	case 0:
	//		str = std::to_string(0.5);
	//		break;
	//	case 1:
	//		str = std::to_string(1);
	//		break;
	//	case 2:
	//		str = std::to_string(2);
	//		break;
	//	case 3:
	//		str = std::to_string(4);
	//	}
	//	str += " Scale ";
	//	str += std::to_string(i % 5 + 1);
	//	str += " Blured Image";
	//	ShowImage(gScaleSpace[i], str.c_str());
	//}
}

//void BuildScaleSpace()
//{
//	double dScale = 2;
//	int nOtvIdx;
//	int nIdx = 0;
//	double dRadius;
//	static double sqrt2 = sqrt(2);
//	for (int i = 0; i < 4; i++)
//	{
//		int nWStep = gScaleOriginal[i].GetWStep();
//		int nWidth = gScaleOriginal[i].GetWidth();
//		int nHeight = gScaleOriginal[i].GetHeight();
//
//		for (int r = 0; r < nHeight; r++)
//		{
//			BYTE* pDst = gScaleOriginal[i].GetPtr(nHeight - r - 1);
//			for (int c = 0; c < nWidth; c++)
//			{
//				pDst[c] = gImageBufGray.BiLinearIntp(c / dScale, r / dScale);
//			}
//		}
//
//		dRadius = 1.6 * (2 / dScale);
//
//		//ShowImage(gScaleOriginal[i], (std::string("original ")+std::to_string(dScale)).c_str());
//
//		for (int j = 0; j < 5; j++)
//		{
//			BYTE* original = gScaleOriginal[i].GetPtr();
//			BYTE* temp = gScaleTemp[i].GetPtr();
//			memcpy(temp, original, nWStep * nHeight * sizeof(BYTE));
//			//gaussBlur(gScaleTemp[i], gScaleSpace[nIdx], nWidth, nHeight, dRadius);
//			GaussianBlur(gScaleTemp[i], gScaleSpace[nIdx], dRadius);
//
//			//ShowImage(gScaleSpace[nIdx], std::to_string(nIdx).c_str());
//			nIdx++;
//			dRadius *= sqrt2;
//		}
//
//		dScale /= 2;
//	}
//
//}

void Norm(CFloatImage& m_imageIn, int n)
{
	if (m_imageIn.GetChannel() != 1)
	{
		AfxMessageBox("회색조 영상을 입력하세요.");
		return;
	}

	int nWidth = m_imageIn.GetWidth();
	int nHeight = m_imageIn.GetHeight();

	CByteImage m_imageOut(nWidth, nHeight, 1);
	int m_histogram[512];
	double m_histogramCdf[512];

	memset(m_histogram, 0, 512 * sizeof(int));

	int r, c;
	for (r = 0; r<nHeight; r++)
	{
		float* pIn = m_imageIn.GetPtr(r);

		for (c = 0; c<nWidth; c++)
		{
			m_histogram[(int)pIn[c]+256]++;
		}
	}

	float dNormFactor = 256.0f / (nWidth * nHeight);

	for (int i = 0; i<512; i++)
	{
		m_histogramCdf[i] = m_histogram[i] * dNormFactor;
	}

	for (int i = 1; i<512; i++)
	{
		m_histogramCdf[i] = m_histogramCdf[i - 1] + m_histogramCdf[i];
	}

	for (r = 0; r<nHeight; r++)
	{
		float* pIn = m_imageIn.GetPtr(r);
		BYTE* pOut = m_imageOut.GetPtr(r);
		for (c = 0; c<nWidth; c++)
		{
			pOut[c] = (BYTE)(m_histogramCdf[(int)pIn[c] + 256]);
		}
	}

	ShowImage(m_imageOut, std::to_string(n).c_str());
}

void DiffrenceOfGaussian()
{
	SubImage(gScaleSpace[1], gScaleSpace[0], gDOG[0]);
	SubImage(gScaleSpace[2], gScaleSpace[1], gDOG[1]);
	SubImage(gScaleSpace[3], gScaleSpace[2], gDOG[2]);
	SubImage(gScaleSpace[4], gScaleSpace[3], gDOG[3]);
	SubImage(gScaleSpace[6], gScaleSpace[5], gDOG[4]);
	SubImage(gScaleSpace[7], gScaleSpace[6], gDOG[5]);
	SubImage(gScaleSpace[8], gScaleSpace[7], gDOG[6]);
	SubImage(gScaleSpace[9], gScaleSpace[8], gDOG[7]);
	SubImage(gScaleSpace[11], gScaleSpace[10], gDOG[8]);
	SubImage(gScaleSpace[12], gScaleSpace[11], gDOG[9]);
	SubImage(gScaleSpace[13], gScaleSpace[12], gDOG[10]);
	SubImage(gScaleSpace[14], gScaleSpace[13], gDOG[11]);
	SubImage(gScaleSpace[16], gScaleSpace[15], gDOG[12]);
	SubImage(gScaleSpace[17], gScaleSpace[16], gDOG[13]);
	SubImage(gScaleSpace[18], gScaleSpace[17], gDOG[14]);
	SubImage(gScaleSpace[19], gScaleSpace[18], gDOG[15]);

	//for (int i = 0; i < 16; i++)
	//{
		//float* pSrc = gDOG[i].GetPtr();

		//Norm(gDOG[i], i);
		//ShowImage(gDOG[i], std::to_string(i).c_str());
	//}
}

void FindKeyPoint()
{
	feature.clear();
	feature_sub.clear();

	int x = 0;
	for (int i = 0; i < 4; i++)
	{
		int nWidth = gWidth[i];
		int nHeight = gHeight[i];
		int nWStep = gWStep[i];


		for (int j = 0; j < 2; j++)
		{
			for (int r = 1; r < nHeight - 1; r++)
			{
				float* src0 = gDOG[(i << 2) + j].GetPtr(r);
				float* src1 = gDOG[(i << 2) + j + 1].GetPtr(r);
				float* src2 = gDOG[(i << 2) + j + 2].GetPtr(r);
				//int* dst = gKeyPoint[(i << 1) + j].GetPtr(r);

				for (int c = 1; c < nWidth - 1; c++)
				{
					if (fabs(src1[c]) < 0.9f)
						continue;
					if (src1[c] == src0[c] || src1[c] == src2[c])
						continue;
					bool max = src1[c] > src0[c];
					bool max3 = src1[c] > src2[c];
					if (max ^ max3)
						continue;

					if (max)
					{
						if (src1[c] > src1[c - 1] && src1[c] > src1[c + 1] &&
							src1[c] > src1[c - 1 - nWStep] && src1[c] > src1[c - nWStep] && src1[c] > src1[c + 1 - nWStep] &&
							src1[c] > src1[c - 1 + nWStep] && src1[c] > src1[c + nWStep] && src1[c] > src1[c + 1 + nWStep] &&
							src1[c] > src0[c - 1] && src1[c] > src0[c + 1] &&
							src1[c] > src2[c - 1] && src1[c] > src2[c + 1] &&
							src1[c] > src0[c - 1 - nWStep] && src1[c] > src0[c - nWStep] && src1[c] > src0[c + 1 - nWStep] &&
							src1[c] > src2[c - 1 - nWStep] && src1[c] > src2[c - nWStep] && src1[c] > src2[c + 1 - nWStep] &&
							src1[c] > src0[c - 1 + nWStep] && src1[c] > src0[c + nWStep] && src1[c] > src0[c + 1 + nWStep] &&
							src1[c] > src2[c - 1 + nWStep] && src1[c] > src2[c + nWStep] && src1[c] > src2[c + 1 + nWStep]
							)
							//dst[c] = src1[c];
							//dst[c] = 255;
							feature.push_back(feature_t(i, j + 1, c, r, src1[c]));
					}
					else
					{
						if (src1[c] < src1[c - 1] && src1[c] < src1[c + 1] &&
							src1[c] < src1[c - 1 - nWStep] && src1[c] < src1[c - nWStep] && src1[c] < src1[c + 1 - nWStep] &&
							src1[c] < src1[c - 1 + nWStep] && src1[c] < src1[c + nWStep] && src1[c] < src1[c + 1 + nWStep] &&
							src1[c] < src0[c - 1] && src1[c] < src0[c + 1] &&
							src1[c] < src2[c - 1] && src1[c] < src2[c + 1] &&
							src1[c] < src0[c - 1 - nWStep] && src1[c] < src0[c - nWStep] && src1[c] < src0[c + 1 - nWStep] &&
							src1[c] < src2[c - 1 - nWStep] && src1[c] < src2[c - nWStep] && src1[c] < src2[c + 1 - nWStep] &&
							src1[c] < src0[c - 1 + nWStep] && src1[c] < src0[c + nWStep] && src1[c] < src0[c + 1 + nWStep] &&
							src1[c] < src2[c - 1 + nWStep] && src1[c] < src2[c + nWStep] && src1[c] < src2[c + 1 + nWStep]
							)
							//dst[c] = src1[c];
							//dst[c] = 255;
							feature.push_back(feature_t(i, j + 1, c, r, src1[c]));
					}
				}
			}
			//ShowImage(gKeyPoint[(i << 1) + j], std::to_string((i << 1) + j).c_str(), x, j * (30 + gHeight[i]));
		}
		x += gWidth[i];
	}


	//BYTE* origin = gImageBuf.GetPtr();
	//BYTE* out = gImageOut.GetPtr();
	//int wstep = gImageBuf.GetWStep();

	//for (int i = 0; i < gHeight[1]; i++)
	//{
	//	memcpy(out + i * wstep, origin + (gHeight[1] - 1 - i) * wstep, wstep);
	//}

	//for (itr = feature.begin(); itr != feature.end(); itr++)
	//{
	//	double size = pow(2, itr->octave - 1);
	//	int pos = (itr->y * size * wstep) + (itr->x * size * gImageOut.GetChannel());
	//	out[pos] = 255;
	//	out[pos+1] = 255;
	//	out[pos+2] = 255;
	//}
	//ShowImage(gImageOut, "keyPoints");
}
bool Solve(const float* H, const float* D, float* B)
{
	float det = (H[0] * H[4] * H[8]) + (H[3] * H[7] * H[2]) + (H[6] * H[1] * H[5]) - (H[0] * H[7] * H[5]) - (H[6] * H[4] * H[2]) - (H[3] * H[1] * H[8]); // det=a00*a11*a22 + a10*a21*a02 + a20*a01*a12 - a00*a21*a12 - a20*a11*a02 - a10*a01*a22

	if (!det)
		return false;

	float InverseH[9] = { det * (H[4] * H[8] - H[5] * H[7]), det * (H[2] * H[7] - H[1] * H[8]), det * (H[1] * H[5] - H[2] * H[4]),
		det * (H[5] * H[6] - H[3] * H[8]), det * (H[0] * H[8] - H[2] * H[6]), det * (H[2] * H[3] - H[0] * H[5]),
		det * (H[3] * H[7] - H[4] * H[6]), det * (H[1] * H[6] - H[0] * H[7]), det * (H[0] * H[4] - H[1] * H[3]) };

	B[0] = -(InverseH[0] * D[0] + InverseH[1] * D[1] + InverseH[2] * D[2]);
	B[1] = -(InverseH[3] * D[0] + InverseH[4] * D[1] + InverseH[5] * D[2]);
	B[2] = -(InverseH[6] * D[0] + InverseH[7] * D[1] + InverseH[8] * D[2]);

	return true;
}

bool SubPixel(feature_t& key, int nAdjustment = 2)
{
	bool needToAdjust = true;
	int x = (int)(key.x + 0.5f);
	int y = (int)(key.y + 0.5f);
	int s = (int)(key.scale + 0.5f);
	float v = key.value;
	int o = (int)(key.octave);

	while (needToAdjust)
	{
		if (s <= 0 || s >= 3)
			return false;
		if (x <= 0 || x >= gWidth[o] - 1)
			return false;
		if (y <= 0 || y >= gHeight[o] - 1)
			return false;

		float dp = 0.0f; // dot product

		float H[9] = { gDOG[(o << 2) + s - 1].GetAt(x, y) - 2 * gDOG[(o << 2) + s].GetAt(x, y) + gDOG[(o << 2) + s + 1].GetAt(x, y),  // below[y][x] - 2*current[y][x] + above[y][x]
						(gDOG[(o << 2) + s + 1].GetAt(x, y + 1) - gDOG[(o << 2) + s + 1].GetAt(x, y - 1) - (gDOG[(o << 2) + s - 1].GetAt(x, y + 1) - gDOG[(o << 2) + s - 1].GetAt(x, y - 1))) / 4, // 0.25 * (above[y+1][x] - above[y-1][x] - (below[y+1][x]-below[y-1][x]))
						(gDOG[(o << 2) + s + 1].GetAt(x + 1, y) - gDOG[(o << 2) + s + 1].GetAt(x - 1, y) - (gDOG[(o << 2) + s - 1].GetAt(x + 1, y) - gDOG[(o << 2) + s - 1].GetAt(x - 1, y))) / 4, // 0.25 * (above[y][x+1] - above[y][x-1] - (below[y][x+1]-below[y][x-1]))
						(gDOG[(o << 2) + s + 1].GetAt(x, y + 1) - gDOG[(o << 2) + s + 1].GetAt(x, y - 1) - (gDOG[(o << 2) + s - 1].GetAt(x, y + 1) - gDOG[(o << 2) + s - 1].GetAt(x, y - 1))) / 4, // H[1][0] = H[0][1]
						gDOG[(o << 2) + s].GetAt(x, y - 1) - 2 * gDOG[(o << 2) + s].GetAt(x, y) + gDOG[(o << 2) + s].GetAt(x, y + 1),  // current[y - 1][x] - 2*current[y][x] + current[y + 1][x]
						(gDOG[(o << 2) + s].GetAt(x + 1, y + 1) - gDOG[(o << 2) + s].GetAt(x - 1, y + 1) - (gDOG[(o << 2) + s].GetAt(x + 1, y - 1) - gDOG[(o << 2) + s].GetAt(x - 1, y - 1))) / 4, // 0.25 * (current[y+1][x+1] - current[y+1][x-1] - (current[y-1][x+1]-current[y-1][x-1]))
						(gDOG[(o << 2) + s + 1].GetAt(x + 1, y) - gDOG[(o << 2) + s + 1].GetAt(x - 1, y) - (gDOG[(o << 2) + s - 1].GetAt(x + 1, y) - gDOG[(o << 2) + s - 1].GetAt(x - 1, y))) / 4, // H[2][0] = H[0][2]
						(gDOG[(o << 2) + s].GetAt(x + 1, y + 1) - gDOG[(o << 2) + s].GetAt(x - 1, y + 1) - (gDOG[(o << 2) + s].GetAt(x + 1, y - 1) - gDOG[(o << 2) + s].GetAt(x - 1, y - 1))) / 4, // H[2][1] = H[1][2]
						gDOG[(o << 2) + s].GetAt(x - 1, y) - 2 * gDOG[(o << 2) + s].GetAt(x, y) + gDOG[(o << 2) + s].GetAt(x + 1, y) };  // current[y][x-1] - 2*current[y][x] + current[y1][x+1]
		
		
		float yong[6] = { gDOG[(o << 2) + s - 1].GetAt(x, y) , gDOG[(o << 2) + s + 1].GetAt(x, y),
			gDOG[(o << 2) + s].GetAt(x, y + 1) , gDOG[(o << 2) + s].GetAt(x, y - 1),
			gDOG[(o << 2) + s].GetAt(x + 1, y) , gDOG[(o << 2) + s].GetAt(x - 1, y) };


		float D[3] = { gDOG[(o << 2) + s - 1].GetAt(x, y) - gDOG[(o << 2) + s + 1].GetAt(x, y), // below[y][x] - above[y][x]
						gDOG[(o << 2) + s].GetAt(x, y + 1) - gDOG[(o << 2) + s].GetAt(x, y - 1),  // current[y+1][x] - current[y+1][x]
						gDOG[(o << 2) + s].GetAt(x + 1, y) - gDOG[(o << 2) + s].GetAt(x - 1, y) };  // current[y][x+1] - current[y][x-1]


		float B[3];

		Solve(H, D, B);


		dp = B[0] * D[0] + B[1] * D[1] + B[2] * D[2];
		

		if (abs(B[0]) > 0.5f || abs(B[1]) > 0.5f || abs(B[2]) > 0.5f)
		{
			if (nAdjustment == 0)
				return false;

			nAdjustment -= 1;

			double distSq = B[1] * B[1] + B[2] * B[2];

			if (distSq > 2.0)
				return false;

			x = (int)(x + B[2] + 0.5f);
			y = (int)(y + B[1] + 0.5f);
			s = (int)(s + B[0] + 0.5f);

			continue;
		}

		key.x = x + B[2];
		key.y = y + B[1];
		key.scale = (int)(s + B[0] + 0.5f);
		key.value = gDOG[(o << 2) + s].GetAt((int)key.x, (int)key.y) + 0.5f * dp;

		if (fabs(key.value) < 0.03f)
			return false;

		float dxx = gDOG[o * 4 + s].GetAt(x - 1, y) - 2 * gDOG[o * 4 + s].GetAt(x, y) + gDOG[o * 4 + s].GetAt(x + 1, y);
		float dyy = gDOG[o * 4 + s].GetAt(x, y - 1) - 2 * gDOG[o * 4 + s].GetAt(x, y) + gDOG[o * 4 + s].GetAt(x, y + 1);
		float dxy = (gDOG[o * 4 + s].GetAt(x + 1, y + 1) - gDOG[o * 4 + s].GetAt(x - 1, y + 1) - (gDOG[o * 4 + s].GetAt(x + 1, y - 1) - gDOG[o * 4 + s].GetAt(x - 1, y - 1))) / 4;

		float trH = dxx + dyy;
		float detH = dxx * dyy - dxy * dxy;

		float R = trH * trH / detH;
		if (detH <= 0.0f || R >= 12.1f)
		{
			return false;
		}
		return true;
	}
	return false;
}

void AccuratingKey()
{
	for (itr = feature.begin(); itr != feature.end();)
	{
		if (!SubPixel(*itr))
		{
			itr = feature.erase(itr);
		}
		else
			itr++;
	}

	BYTE* origin = gImageBuf.GetPtr();
	BYTE* out = gImageOut.GetPtr();
	int wstep = gImageBuf.GetWStep();

	for (int i = 0; i < gHeight[1]; i++)
	{
		memcpy(out + i * wstep, origin + (gHeight[1] - 1 - i) * wstep, wstep);
	}

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		double size = pow(2, itr->octave - 1);
		int pos = (int)((itr->y * size * wstep) + (itr->x * size * gImageOut.GetChannel()));
		out[pos] = 255;
		out[pos+1] = 255;
		out[pos+2] = 255;
	}
	ShowImage(gImageOut, "keyPoints");
}

void BuildGradient()
{
	int SSIdx, GrdIdx, wstep;
	float dx, dy;
	float* pSrc;
	float* pMag;
	float* pOri;
	for (int i = 0; i < 4; i++)
	{
		wstep = gWStep[i];
		for (int j = 0; j < 2; j++)
		{
			SSIdx = i * 5 + j + 1;
			GrdIdx = (i << 1) + j;
			pSrc = gScaleSpace[SSIdx].GetPtr();

			for (int r = 1; r < gHeight[i] - 1; r++)
			{
				pSrc = gScaleSpace[SSIdx].GetPtr(r);
				pMag = gGradientMagnitude[GrdIdx].GetPtr(r);
				pOri = gGradientOrientation[GrdIdx].GetPtr(r);
				for (int c = 1; c < gWidth[i] - 1; c++)
				{
					dx = pSrc[c + 1] - pSrc[c - 1];
					dy = pSrc[c + wstep] - pSrc[c - wstep];
					pMag[c] = dx * dx + dy * dy; // no sqrt
					pOri[c] = atan2(dy, dx);
				}
			}


			//top out line
			pSrc = gScaleSpace[SSIdx].GetPtr();
			pMag = gGradientMagnitude[GrdIdx].GetPtr();
			pOri = gGradientOrientation[GrdIdx].GetPtr();
			for (int c = 1; c < gWidth[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c + wstep] - pSrc[c]);
				pMag[c] = dx * dx + dy * dy; // no sqrt
				pOri[c] = atan2(dy, dx);
			}

			//bottom out line
			pSrc = gScaleSpace[SSIdx].GetPtr(gHeight[i] - 1);
			pMag = gGradientMagnitude[GrdIdx].GetPtr(gHeight[i] - 1);
			pOri = gGradientOrientation[GrdIdx].GetPtr(gHeight[i] - 1);
			for (int c = 1; c < gWidth[i] - 1; c++)
			{
				dx = pSrc[c + 1] - pSrc[c - 1];
				dy = 2.0f * (pSrc[c] - pSrc[c - wstep]);
				pMag[c] = dx * dx + dy * dy; // no sqrt
				pOri[c] = atan2(dy, dx);
			}

			//left out line
			pSrc = gScaleSpace[SSIdx].GetPtr();
			pMag = gGradientMagnitude[GrdIdx].GetPtr();
			pOri = gGradientOrientation[GrdIdx].GetPtr();
			for (int r = 1; r < gHeight[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * wstep + 1] - pSrc[r * wstep]);
				dy = pSrc[(r + 1) * wstep] - pSrc[(r - 1) * wstep];
				pMag[r * wstep] = dx * dx + dy * dy; // no sqrt
				pOri[r * wstep] = atan2(dy, dx);
			}

			//right out line
			pSrc = gScaleSpace[SSIdx].GetPtr(0, gWidth[i] - 1);
			pMag = gGradientMagnitude[GrdIdx].GetPtr(0, gWidth[i] - 1);
			pOri = gGradientOrientation[GrdIdx].GetPtr(0, gWidth[i] - 1);
			for (int r = 1; r < gHeight[i] - 1; r++)
			{
				dx = 2.0f * (pSrc[r * wstep] - pSrc[r * wstep - 1]);
				dy = pSrc[(r + 1) * wstep] - pSrc[(r - 1) * wstep];
				pMag[r * wstep] = dx * dx + dy * dy; // no sqrt
				pOri[r * wstep] = atan2(dy, dx);
			}


			//vertics
			pSrc = gScaleSpace[SSIdx].GetPtr();
			pMag = gGradientMagnitude[GrdIdx].GetPtr();
			pOri = gGradientOrientation[GrdIdx].GetPtr();
			int cLast = gWidth[i] - 1;
			int rLast = gHeight[i] - 1;

			dx = 2.0f * (pSrc[1] - pSrc[0]);
			dy = 2.0f * (pSrc[wstep] - pSrc[0]);
			pMag[0] = dx * dx + dy * dy; // no sqrt
			pOri[0] = atan2(dy, dx);

			dx = 2.0f * (pSrc[cLast] - pSrc[cLast - 1]);
			dy = 2.0f * (pSrc[wstep + cLast] - pSrc[cLast]);
			pMag[cLast] = dx * dx + dy * dy; // no sqrt
			pOri[cLast] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * wstep + 1] - pSrc[rLast * wstep]);
			dy = 2.0f * (pSrc[rLast * wstep] - pSrc[(rLast - 1) * wstep]);
			pMag[rLast * wstep] = dx * dx + dy * dy; // no sqrt
			pOri[rLast * wstep] = atan2(dy, dx);

			dx = 2.0f * (pSrc[rLast * wstep + cLast] - pSrc[rLast * wstep + cLast - 1]);
			dy = 2.0f * (pSrc[rLast * wstep + cLast] - pSrc[(rLast - 1) * wstep + cLast]);
			pMag[rLast * wstep + cLast] = dx * dx + dy * dy; // no sqrt
			pOri[rLast * wstep + cLast] = atan2(dy, dx);

			//ShowImage(gGradientMagnitude[GrdIdx], std::to_string(GrdIdx).c_str());
		}
	}
}

#define _2PI (float)(M_PI * 2.0)
void JudgeOrientation(feature_t& key)
{
	float hist[36];
	memset(hist, 0, 36 * sizeof(float));

	int window = key.scale - 1;
	int imageIdx = key.octave * 2 + window;
	int kx = (int)(key.x);
	int ky = (int)(key.y);
	int o = (int)(key.octave);
	if (kx > gRadius[window] && kx < gWidth[o] - gRadius[window] && ky > gRadius[window] && ky < gHeight[o] - gRadius[window])
	{
		for (int r = 0; r < gWindowSize[window]; r++)
		{
			for (int c = 0; c < gWindowSize[window]; c++)
			{
				int x = kx - gRadius[window] + c;
				int y = ky - gRadius[window] + r;
				int deg = (int)(gGradientOrientation[imageIdx].GetAt(x, y) * 36.0f / _2PI);
				if (deg < 0) deg += 36;
				float mag = gGradientMagnitude[imageIdx].GetAt(x, y);
				hist[deg] += mag * gWeight[window][r * gWindowSize[window] + c];
			}
		}
	}
	else
	{
		for (int r = 0; r < gWindowSize[window]; r++)
		{
			for (int c = 0; c < gWindowSize[window]; c++)
			{
				int x = max(0, min(kx - gRadius[window] + c, gWidth[o] - 1));
				int y = max(0, min(ky - gRadius[window] + r, gHeight[o] - 1));
				int ori = (int)(gGradientOrientation[imageIdx].GetAt(x, y) * 36.0f / _2PI);
				if (ori < 0) ori += 36;
				float mag = gGradientMagnitude[imageIdx].GetAt(x, y);
				hist[ori] += mag * gWeight[window][r * gWindowSize[window] + c];
			}
		}
	}

	float max = FLT_MIN;
	int theta = 0;

	for (int i = 0; i < 36; i++)
	{
		if (hist[i] > max)
		{
			max = hist[i];
			theta = i;
		}
	}

	key.orientation = theta * _2PI / 36.0f;

	for (int i = 0; i < 36; i++)
	{
		if (hist[i] > hist[theta] * 0.64f && i != theta)
		{
			feature_sub.push_back(feature_t(key, i * _2PI / 36.0f));
		}
	}
}

void AssignOrientation()
{
	BuildGradient();
	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		JudgeOrientation(*itr);
	}
	
	feature.insert(feature.end(), feature_sub.begin(), feature_sub.end());
}


void DescriptingKey()
{
	static float mask[16][16] =
	{ 0.415237f, 0.46323f, 0.508759f, 0.550099f, 0.585578f, 0.61368f, 0.633161f, 0.643131f, 0.643131f, 0.633161f, 0.61368f, 0.585578f, 0.550099f, 0.508759f, 0.46323f, 0.415237f,
		0.46323f, 0.516771f, 0.567561f, 0.61368f, 0.653259f, 0.68461f, 0.706342f, 0.717465f, 0.717465f, 0.706342f, 0.68461f, 0.653259f, 0.61368f, 0.567561f, 0.516771f, 0.46323f,
		0.508759f, 0.567561f, 0.623344f, 0.673996f, 0.717465f, 0.751897f, 0.775765f, 0.787981f, 0.787981f, 0.775765f, 0.751897f, 0.717465f, 0.673996f, 0.623344f, 0.567561f, 0.508759f,
		0.550099f, 0.61368f, 0.673996f, 0.728763f, 0.775765f, 0.812994f, 0.838801f, 0.852011f, 0.852011f, 0.838801f, 0.812994f, 0.775765f, 0.728763f, 0.673996f, 0.61368f, 0.550099f,
		0.585578f, 0.653259f, 0.717465f, 0.775765f, 0.825797f, 0.865428f, 0.8929f, 0.906961f, 0.906961f, 0.8929f, 0.865428f, 0.825797f, 0.775765f, 0.717465f, 0.653259f, 0.585578f,
		0.61368f, 0.68461f, 0.751897f, 0.812994f, 0.865428f, 0.906961f, 0.935751f, 0.950487f, 0.950487f, 0.935751f, 0.906961f, 0.865428f, 0.812994f, 0.751897f, 0.68461f, 0.61368f,
		0.633161f, 0.706342f, 0.775765f, 0.838801f, 0.8929f, 0.935751f, 0.965455f, 0.980658f, 0.980658f, 0.965455f, 0.935751f, 0.8929f, 0.838801f, 0.775765f, 0.706342f, 0.633161f,
		0.643131f, 0.717465f, 0.787981f, 0.852011f, 0.906961f, 0.950487f, 0.980658f, 0.996101f, 0.996101f, 0.980658f, 0.950487f, 0.906961f, 0.852011f, 0.787981f, 0.717465f, 0.643131f,
		0.643131f, 0.717465f, 0.787981f, 0.852011f, 0.906961f, 0.950487f, 0.980658f, 0.996101f, 0.996101f, 0.980658f, 0.950487f, 0.906961f, 0.852011f, 0.787981f, 0.717465f, 0.643131f,
		0.633161f, 0.706342f, 0.775765f, 0.838801f, 0.8929f, 0.935751f, 0.965455f, 0.980658f, 0.980658f, 0.965455f, 0.935751f, 0.8929f, 0.838801f, 0.775765f, 0.706342f, 0.633161f,
		0.61368f, 0.68461f, 0.751897f, 0.812994f, 0.865428f, 0.906961f, 0.935751f, 0.950487f, 0.950487f, 0.935751f, 0.906961f, 0.865428f, 0.812994f, 0.751897f, 0.68461f, 0.61368f,
		0.585578f, 0.653259f, 0.717465f, 0.775765f, 0.825797f, 0.865428f, 0.8929f, 0.906961f, 0.906961f, 0.8929f, 0.865428f, 0.825797f, 0.775765f, 0.717465f, 0.653259f, 0.585578f,
		0.550099f, 0.61368f, 0.673996f, 0.728763f, 0.775765f, 0.812994f, 0.838801f, 0.852011f, 0.852011f, 0.838801f, 0.812994f, 0.775765f, 0.728763f, 0.673996f, 0.61368f, 0.550099f,
		0.508759f, 0.567561f, 0.623344f, 0.673996f, 0.717465f, 0.751897f, 0.775765f, 0.787981f, 0.787981f, 0.775765f, 0.751897f, 0.717465f, 0.673996f, 0.623344f, 0.567561f, 0.508759f,
		0.46323f, 0.516771f, 0.567561f, 0.61368f, 0.653259f, 0.68461f, 0.706342f, 0.717465f, 0.717465f, 0.706342f, 0.68461f, 0.653259f, 0.61368f, 0.567561f, 0.516771f, 0.46323f,
		0.415237f, 0.46323f, 0.508759f, 0.550099f, 0.585578f, 0.61368f, 0.633161f, 0.643131f, 0.643131f, 0.633161f, 0.61368f, 0.585578f, 0.550099f, 0.508759f, 0.46323f, 0.415237f };

	static float hist[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	for (itr = feature.begin(); itr != feature.end(); itr++)
	{
		int o = itr->octave; // key octave
		int s = itr->scale; // key scale
		int kx = (int)itr->x; // key x
		int ky = (int)itr->y; // key y
		float kori = itr->orientation; // key orientation
		int idx = (o >> 1) + s - 1; // index of gradient array
		int pos = 0; // index of 128 dimension vector

		if (kx > 6 && kx < gWidth[o] - 8 && ky > 6 && ky < gHeight[o] - 8)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					int bigRow = i * 4;
					int bigCol = j * 4;
					for (int r = 0; r < 4; r++)
					{
						float* pMag = gGradientMagnitude[idx].GetPtr(ky - 7 + bigRow + r);
						float* pOri = gGradientOrientation[idx].GetPtr(ky - 7 + bigRow + r);
						for (int c = 0; c < 4; c++)
						{
							int ori = (int)((pOri[kx - 7 + bigCol + c] - kori) * 8.0f / _2PI);
							float mag = pMag[kx - 7 + bigCol + c] * mask[bigRow + r][bigCol + c];
							hist[ori] += mag;
						}
					}
					for (int k = 0; k < 8; k++)
					{
						itr->vec[pos++] = (int)hist[k];
					}
					memset(hist, 0, 8 * sizeof(int));
				}
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					int bigRow = i * 4;
					int bigCol = j * 4;
					for (int r = 0; r < 4; r++)
					{
						float* pMag = gGradientMagnitude[idx].GetPtr(min(0, max(ky - 7 + bigRow + r, gHeight[o] - 1)));
						float* pOri = gGradientOrientation[idx].GetPtr(min(0, max(ky - 7 + bigRow + r, gHeight[o] - 1)));
						for (int c = 0; c < 4; c++)
						{
							int ori = (int)((pOri[min(0, max(kx - 7 + bigCol + c, gWidth[i] - 1))] - kori) * 8.0f / _2PI);
							float mag = pMag[min(0, max(kx - 7 + bigCol + c, gWidth[i] - 1))] * mask[bigRow + r][bigCol + c];
							hist[ori] += mag;
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
						itr->vec[pos++] = (int)(hist[k] / (5.0f * max));
					}
					memset(hist, 0, 8 * sizeof(int));
				}
			}
		}
	}
}

#endif

LRESULT ProcessCamFrame(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	if (gBmpInfo.bmiHeader.biCompression == BI_RGB) // RGB 영상
	{
		memcpy(gImageBuf.GetPtr(), lpVHdr->lpData,
			gBmpInfo.bmiHeader.biHeight*gBmpInfo.bmiHeader.biWidth * 3);
		gImageBufGray = RGB2Gray(gImageBuf);
		//memcpy(gImageIn.GetPtr(), gImageIn.GetPtr(),
		//	gImageIn.GetHeight()*gImageIn.GetWStep() * gImageIn.GetChannel());
		//gImageBufGray = RGB2Gray(gImageIn);
	}
	else if (gBmpInfo.bmiHeader.biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) // 16비트 영상
	{
		YUY2ToRGB24(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight,
			lpVHdr->lpData, gImageBuf.GetPtr());
		ShowImage(gImageBuf, "YUY2");
	}
	else
	{
		return FALSE;
	}

	//BuildScaleSpace();
	//DiffrenceOfGaussian();
	//FindKeyPoint();
	//AccuratingKey();
	//AssignOrientation();
	//DescriptingKey();
	//feature_sub;

	m_SIFT.SIFT(gImageBuf);
	

	return TRUE;
}

#ifndef CLASS
void InitScaleSpace()
{
	gWidth[0] = gBmpInfo.bmiHeader.biWidth << 1;
	gHeight[0] = gBmpInfo.bmiHeader.biHeight << 1;
	gWStep[0] = ((gWidth[0]*NUM_CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
	gSize[0] = gHeight[0] * gWStep[0];
	for(int i = 1; i < 4; i++)
	{
		gWidth[i] = gWidth[i - 1] >> 1;
		gHeight[i] = gHeight[i - 1] >> 1;
		gWStep[i] = ((gWidth[i] * NUM_CHANNEL * sizeof(float) + 3)&~3) / sizeof(float);
		gSize[i] = gHeight[i] * gWStep[i];
	};
	gScaleOriginal[0] = CByteImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleOriginal[1] = CByteImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleOriginal[2] = CByteImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleOriginal[3] = CByteImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleTemp[0] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleTemp[1] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleTemp[2] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleTemp[3] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleSpace[0] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleSpace[1] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleSpace[2] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleSpace[3] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleSpace[4] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gScaleSpace[5] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleSpace[6] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleSpace[7] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleSpace[8] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleSpace[9] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gScaleSpace[10] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleSpace[11] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleSpace[12] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleSpace[13] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleSpace[14] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gScaleSpace[15] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleSpace[16] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleSpace[17] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleSpace[18] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gScaleSpace[19] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);

}


void InitDOG()
{
	gDOG[0] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gDOG[1] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gDOG[2] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gDOG[3] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gDOG[4] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gDOG[5] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gDOG[6] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gDOG[7] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gDOG[8] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gDOG[9] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gDOG[10] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gDOG[11] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gDOG[12] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gDOG[13] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gDOG[14] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gDOG[15] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
}


void InitGradient()
{
	gGradientMagnitude[0] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gGradientMagnitude[1] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gGradientMagnitude[2] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gGradientMagnitude[3] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gGradientMagnitude[4] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gGradientMagnitude[5] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gGradientMagnitude[6] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gGradientMagnitude[7] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gGradientOrientation[0] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gGradientOrientation[1] = CFloatImage(gWidth[0], gHeight[0], NUM_CHANNEL);
	gGradientOrientation[2] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gGradientOrientation[3] = CFloatImage(gWidth[1], gHeight[1], NUM_CHANNEL);
	gGradientOrientation[4] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gGradientOrientation[5] = CFloatImage(gWidth[2], gHeight[2], NUM_CHANNEL);
	gGradientOrientation[6] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
	gGradientOrientation[7] = CFloatImage(gWidth[3], gHeight[3], NUM_CHANNEL);
}

void InitWeight()
{
	gRadius[0] = (int)(3 * gSigma[1]);
	gRadius[1] = (int)(3 * gSigma[2]);
	gWindowSize[0] = (int)(gRadius[0] * 2 + 1);
	gWindowSize[1] = (int)(gRadius[1] * 2 + 1);
	static float weight1[441] = { 2.88513e-08f, 1.50124e-07f, 6.56653e-07f, 2.41448e-06f, 7.46298e-06f, 1.93912e-05f, 4.23542e-05f, 7.77661e-05f, 0.000120029f, 0.000155734f, 0.000169857f, 0.000155734f, 0.000120029f, 7.77661e-05f, 4.23542e-05f, 1.93912e-05f, 7.46298e-06f, 2.41448e-06f, 6.56653e-07f, 1.50124e-07f, 2.88513e-08f,
		1.50124e-07f, 7.8115e-07f, 3.4168e-06f, 1.25634e-05f, 3.88326e-05f, 0.000100899f, 0.000220384f, 0.000404645f, 0.000624555f, 0.000810341f, 0.000883827f, 0.000810341f, 0.000624555f, 0.000404645f, 0.000220384f, 0.000100899f, 3.88326e-05f, 1.25634e-05f, 3.4168e-06f, 7.8115e-07f, 1.50124e-07f,
		6.56653e-07f, 3.4168e-06f, 1.49453e-05f, 5.49532e-05f, 0.000169857f, 0.00044134f, 0.000963976f, 0.00176995f, 0.00273185f, 0.00354449f, 0.00386592f, 0.00354449f, 0.00273185f, 0.00176995f, 0.000963976f, 0.00044134f, 0.000169857f, 5.49532e-05f, 1.49453e-05f, 3.4168e-06f, 6.56653e-07f,
		2.41448e-06f, 1.25634e-05f, 5.49532e-05f, 0.00020206f, 0.000624555f, 0.00162279f, 0.00354449f, 0.00650801f, 0.0100449f, 0.0130329f, 0.0142148f, 0.0130329f, 0.0100449f, 0.00650801f, 0.00354449f, 0.00162279f, 0.000624555f, 0.00020206f, 5.49532e-05f, 1.25634e-05f, 2.41448e-06f,
		7.46298e-06f, 3.88326e-05f, 0.000169857f, 0.000624555f, 0.00193046f, 0.00501592f, 0.0109558f, 0.0201158f, 0.031048f, 0.0402838f, 0.0439369f, 0.0402838f, 0.031048f, 0.0201158f, 0.0109558f, 0.00501592f, 0.00193046f, 0.000624555f, 0.000169857f, 3.88326e-05f, 7.46298e-06f,
		1.93912e-05f, 0.000100899f, 0.00044134f, 0.00162279f, 0.00501592f, 0.0130329f, 0.0284665f, 0.0522671f, 0.0806722f, 0.10467f, 0.114162f, 0.10467f, 0.0806722f, 0.0522671f, 0.0284665f, 0.0130329f, 0.00501592f, 0.00162279f, 0.00044134f, 0.000100899f, 1.93912e-05f,
		4.23542e-05f, 0.000220384f, 0.000963976f, 0.00354449f, 0.0109558f, 0.0284665f, 0.0621765f, 0.114162f, 0.176204f, 0.22862f, 0.249352f, 0.22862f, 0.176204f, 0.114162f, 0.0621765f, 0.0284665f, 0.0109558f, 0.00354449f, 0.000963976f, 0.000220384f, 4.23542e-05f,
		7.77661e-05f, 0.000404645f, 0.00176995f, 0.00650801f, 0.0201158f, 0.0522671f, 0.114162f, 0.209611f, 0.323527f, 0.419767f, 0.457833f, 0.419767f, 0.323527f, 0.209611f, 0.114162f, 0.0522671f, 0.0201158f, 0.00650801f, 0.00176995f, 0.000404645f, 7.77661e-05f,
		0.000120029f, 0.000624555f, 0.00273185f, 0.0100449f, 0.031048f, 0.0806722f, 0.176204f, 0.323527f, 0.499352f, 0.647894f, 0.706648f, 0.647894f, 0.499352f, 0.323527f, 0.176204f, 0.0806722f, 0.031048f, 0.0100449f, 0.00273185f, 0.000624555f, 0.000120029f,
		0.000155734f, 0.000810341f, 0.00354449f, 0.0130329f, 0.0402838f, 0.10467f, 0.22862f, 0.419767f, 0.647894f, 0.840624f, 0.916855f, 0.840624f, 0.647894f, 0.419767f, 0.22862f, 0.10467f, 0.0402838f, 0.0130329f, 0.00354449f, 0.000810341f, 0.000155734f,
		0.000169857f, 0.000883827f, 0.00386592f, 0.0142148f, 0.0439369f, 0.114162f, 0.249352f, 0.457833f, 0.706648f, 0.916855f, 1.0f, 0.916855f, 0.706648f, 0.457833f, 0.249352f, 0.114162f, 0.0439369f, 0.0142148f, 0.00386592f, 0.000883827f, 0.000169857f,
		0.000155734f, 0.000810341f, 0.00354449f, 0.0130329f, 0.0402838f, 0.10467f, 0.22862f, 0.419767f, 0.647894f, 0.840624f, 0.916855f, 0.840624f, 0.647894f, 0.419767f, 0.22862f, 0.10467f, 0.0402838f, 0.0130329f, 0.00354449f, 0.000810341f, 0.000155734f,
		0.000120029f, 0.000624555f, 0.00273185f, 0.0100449f, 0.031048f, 0.0806722f, 0.176204f, 0.323527f, 0.499352f, 0.647894f, 0.706648f, 0.647894f, 0.499352f, 0.323527f, 0.176204f, 0.0806722f, 0.031048f, 0.0100449f, 0.00273185f, 0.000624555f, 0.000120029f,
		7.77661e-05f, 0.000404645f, 0.00176995f, 0.00650801f, 0.0201158f, 0.0522671f, 0.114162f, 0.209611f, 0.323527f, 0.419767f, 0.457833f, 0.419767f, 0.323527f, 0.209611f, 0.114162f, 0.0522671f, 0.0201158f, 0.00650801f, 0.00176995f, 0.000404645f, 7.77661e-05f,
		4.23542e-05f, 0.000220384f, 0.000963976f, 0.00354449f, 0.0109558f, 0.0284665f, 0.0621765f, 0.114162f, 0.176204f, 0.22862f, 0.249352f, 0.22862f, 0.176204f, 0.114162f, 0.0621765f, 0.0284665f, 0.0109558f, 0.00354449f, 0.000963976f, 0.000220384f, 4.23542e-05f,
		1.93912e-05f, 0.000100899f, 0.00044134f, 0.00162279f, 0.00501592f, 0.0130329f, 0.0284665f, 0.0522671f, 0.0806722f, 0.10467f, 0.114162f, 0.10467f, 0.0806722f, 0.0522671f, 0.0284665f, 0.0130329f, 0.00501592f, 0.00162279f, 0.00044134f, 0.000100899f, 1.93912e-05f,
		7.46298e-06f, 3.88326e-05f, 0.000169857f, 0.000624555f, 0.00193046f, 0.00501592f, 0.0109558f, 0.0201158f, 0.031048f, 0.0402838f, 0.0439369f, 0.0402838f, 0.031048f, 0.0201158f, 0.0109558f, 0.00501592f, 0.00193046f, 0.000624555f, 0.000169857f, 3.88326e-05f, 7.46298e-06f,
		2.41448e-06f, 1.25634e-05f, 5.49532e-05f, 0.00020206f, 0.000624555f, 0.00162279f, 0.00354449f, 0.00650801f, 0.0100449f, 0.0130329f, 0.0142148f, 0.0130329f, 0.0100449f, 0.00650801f, 0.00354449f, 0.00162279f, 0.000624555f, 0.00020206f, 5.49532e-05f, 1.25634e-05f, 2.41448e-06f,
		6.56653e-07f, 3.4168e-06f, 1.49453e-05f, 5.49532e-05f, 0.000169857f, 0.00044134f, 0.000963976f, 0.00176995f, 0.00273185f, 0.00354449f, 0.00386592f, 0.00354449f, 0.00273185f, 0.00176995f, 0.000963976f, 0.00044134f, 0.000169857f, 5.49532e-05f, 1.49453e-05f, 3.4168e-06f, 6.56653e-07f,
		1.50124e-07f, 7.8115e-07f, 3.4168e-06f, 1.25634e-05f, 3.88326e-05f, 0.000100899f, 0.000220384f, 0.000404645f, 0.000624555f, 0.000810341f, 0.000883827f, 0.000810341f, 0.000624555f, 0.000404645f, 0.000220384f, 0.000100899f, 3.88326e-05f, 1.25634e-05f, 3.4168e-06f, 7.8115e-07f, 1.50124e-07f,
		2.88513e-08f, 1.50124e-07f, 6.56653e-07f, 2.41448e-06f, 7.46298e-06f, 1.93912e-05f, 4.23542e-05f, 7.77661e-05f, 0.000120029f, 0.000155734f, 0.000169857f, 0.000155734f, 0.000120029f, 7.77661e-05f, 4.23542e-05f, 1.93912e-05f, 7.46298e-06f, 2.41448e-06f, 6.56653e-07f, 1.50124e-07f, 2.88513e-08f };
	gWeight[0] = weight1;
	static float weight2[841] = { 4.08284e-08f, 1.31796e-07f, 3.90068e-07f, 1.05848e-06f, 2.63344e-06f, 6.0071e-06f, 1.25634e-05f, 2.40908e-05f, 4.23542e-05f, 6.82718e-05f, 0.000100899f, 0.000136721f, 0.000169857f, 0.000193478f, 0.00020206f, 0.000193478f, 0.000169857f, 0.000136721f, 0.000100899f, 6.82718e-05f, 4.23542e-05f, 2.40908e-05f, 1.25634e-05f, 6.0071e-06f, 2.63344e-06f, 1.05848e-06f, 3.90068e-07f, 1.31796e-07f, 4.08284e-08f,
		1.31796e-07f, 4.25441e-07f, 1.25916e-06f, 3.4168e-06f, 8.50083e-06f, 1.93912e-05f, 4.05552e-05f, 7.77661e-05f, 0.000136721f, 0.000220384f, 0.000325707f, 0.00044134f, 0.000548304f, 0.000624555f, 0.000652259f, 0.000624555f, 0.000548304f, 0.00044134f, 0.000325707f, 0.000220384f, 0.000136721f, 7.77661e-05f, 4.05552e-05f, 1.93912e-05f, 8.50083e-06f, 3.4168e-06f, 1.25916e-06f, 4.25441e-07f, 1.31796e-07f,
		3.90068e-07f, 1.25916e-06f, 3.72666e-06f, 1.01125e-05f, 2.51594e-05f, 5.73909e-05f, 0.000120029f, 0.00023016f, 0.000404645f, 0.000652259f, 0.000963976f, 0.00130621f, 0.00162279f, 0.00184846f, 0.00193046f, 0.00184846f, 0.00162279f, 0.00130621f, 0.000963976f, 0.000652259f, 0.000404645f, 0.00023016f, 0.000120029f, 5.73909e-05f, 2.51594e-05f, 1.01125e-05f, 3.72666e-06f, 1.25916e-06f, 3.90068e-07f,
		1.05848e-06f, 3.4168e-06f, 1.01125e-05f, 2.7441e-05f, 6.82718e-05f, 0.000155734f, 0.000325707f, 0.000624555f, 0.00109803f, 0.00176995f, 0.00261581f, 0.00354449f, 0.00440354f, 0.00501592f, 0.00523842f, 0.00501592f, 0.00440354f, 0.00354449f, 0.00261581f, 0.00176995f, 0.00109803f, 0.000624555f, 0.000325707f, 0.000155734f, 6.82718e-05f, 2.7441e-05f, 1.01125e-05f, 3.4168e-06f, 1.05848e-06f,
		2.63344e-06f, 8.50083e-06f, 2.51594e-05f, 6.82718e-05f, 0.000169857f, 0.000387458f, 0.000810341f, 0.00155386f, 0.00273185f, 0.00440354f, 0.00650801f, 0.00881851f, 0.0109558f, 0.0124793f, 0.0130329f, 0.0124793f, 0.0109558f, 0.00881851f, 0.00650801f, 0.00440354f, 0.00273185f, 0.00155386f, 0.000810341f, 0.000387458f, 0.000169857f, 6.82718e-05f, 2.51594e-05f, 8.50083e-06f, 2.63344e-06f,
		6.0071e-06f, 1.93912e-05f, 5.73909e-05f, 0.000155734f, 0.000387458f, 0.000883827f, 0.00184846f, 0.00354449f, 0.00623158f, 0.0100449f, 0.0148453f, 0.0201158f, 0.0249911f, 0.0284665f, 0.0297292f, 0.0284665f, 0.0249911f, 0.0201158f, 0.0148453f, 0.0100449f, 0.00623158f, 0.00354449f, 0.00184846f, 0.000883827f, 0.000387458f, 0.000155734f, 5.73909e-05f, 1.93912e-05f, 6.0071e-06f,
		1.25634e-05f, 4.05552e-05f, 0.000120029f, 0.000325707f, 0.000810341f, 0.00184846f, 0.00386592f, 0.00741305f, 0.0130329f, 0.0210081f, 0.031048f, 0.0420707f, 0.0522671f, 0.0595356f, 0.0621765f, 0.0595356f, 0.0522671f, 0.0420707f, 0.031048f, 0.0210081f, 0.0130329f, 0.00741305f, 0.00386592f, 0.00184846f, 0.000810341f, 0.000325707f, 0.000120029f, 4.05552e-05f, 1.25634e-05f,
		2.40908e-05f, 7.77661e-05f, 0.00023016f, 0.000624555f, 0.00155386f, 0.00354449f, 0.00741305f, 0.0142148f, 0.0249911f, 0.0402838f, 0.0595356f, 0.0806722f, 0.100224f, 0.114162f, 0.119226f, 0.114162f, 0.100224f, 0.0806722f, 0.0595356f, 0.0402838f, 0.0249911f, 0.0142148f, 0.00741305f, 0.00354449f, 0.00155386f, 0.000624555f, 0.00023016f, 7.77661e-05f, 2.40908e-05f,
		4.23542e-05f, 0.000136721f, 0.000404645f, 0.00109803f, 0.00273185f, 0.00623158f, 0.0130329f, 0.0249911f, 0.0439369f, 0.0708231f, 0.10467f, 0.14183f, 0.176204f, 0.200708f, 0.209611f, 0.200708f, 0.176204f, 0.14183f, 0.10467f, 0.0708231f, 0.0439369f, 0.0249911f, 0.0130329f, 0.00623158f, 0.00273185f, 0.00109803f, 0.000404645f, 0.000136721f, 4.23542e-05f,
		6.82718e-05f, 0.000220384f, 0.000652259f, 0.00176995f, 0.00440354f, 0.0100449f, 0.0210081f, 0.0402838f, 0.0708231f, 0.114162f, 0.16872f, 0.22862f, 0.284029f, 0.323527f, 0.337878f, 0.323527f, 0.284029f, 0.22862f, 0.16872f, 0.114162f, 0.0708231f, 0.0402838f, 0.0210081f, 0.0100449f, 0.00440354f, 0.00176995f, 0.000652259f, 0.000220384f, 6.82718e-05f,
		0.000100899f, 0.000325707f, 0.000963976f, 0.00261581f, 0.00650801f, 0.0148453f, 0.031048f, 0.0595356f, 0.10467f, 0.16872f, 0.249352f, 0.337878f, 0.419767f, 0.478142f, 0.499352f, 0.478142f, 0.419767f, 0.337878f, 0.249352f, 0.16872f, 0.10467f, 0.0595356f, 0.031048f, 0.0148453f, 0.00650801f, 0.00261581f, 0.000963976f, 0.000325707f, 0.000100899f,
		0.000136721f, 0.00044134f, 0.00130621f, 0.00354449f, 0.00881851f, 0.0201158f, 0.0420707f, 0.0806722f, 0.14183f, 0.22862f, 0.337878f, 0.457833f, 0.568794f, 0.647894f, 0.676634f, 0.647894f, 0.568794f, 0.457833f, 0.337878f, 0.22862f, 0.14183f, 0.0806722f, 0.0420707f, 0.0201158f, 0.00881851f, 0.00354449f, 0.00130621f, 0.00044134f, 0.000136721f,
		0.000169857f, 0.000548304f, 0.00162279f, 0.00440354f, 0.0109558f, 0.0249911f, 0.0522671f, 0.100224f, 0.176204f, 0.284029f, 0.419767f, 0.568794f, 0.706648f, 0.804919f, 0.840624f, 0.804919f, 0.706648f, 0.568794f, 0.419767f, 0.284029f, 0.176204f, 0.100224f, 0.0522671f, 0.0249911f, 0.0109558f, 0.00440354f, 0.00162279f, 0.000548304f, 0.000169857f,
		0.000193478f, 0.000624555f, 0.00184846f, 0.00501592f, 0.0124793f, 0.0284665f, 0.0595356f, 0.114162f, 0.200708f, 0.323527f, 0.478142f, 0.647894f, 0.804919f, 0.916855f, 0.957526f, 0.916855f, 0.804919f, 0.647894f, 0.478142f, 0.323527f, 0.200708f, 0.114162f, 0.0595356f, 0.0284665f, 0.0124793f, 0.00501592f, 0.00184846f, 0.000624555f, 0.000193478f,
		0.00020206f, 0.000652259f, 0.00193046f, 0.00523842f, 0.0130329f, 0.0297292f, 0.0621765f, 0.119226f, 0.209611f, 0.337878f, 0.499352f, 0.676634f, 0.840624f, 0.957526f, 1.0f, 0.957526f, 0.840624f, 0.676634f, 0.499352f, 0.337878f, 0.209611f, 0.119226f, 0.0621765f, 0.0297292f, 0.0130329f, 0.00523842f, 0.00193046f, 0.000652259f, 0.00020206f,
		0.000193478f, 0.000624555f, 0.00184846f, 0.00501592f, 0.0124793f, 0.0284665f, 0.0595356f, 0.114162f, 0.200708f, 0.323527f, 0.478142f, 0.647894f, 0.804919f, 0.916855f, 0.957526f, 0.916855f, 0.804919f, 0.647894f, 0.478142f, 0.323527f, 0.200708f, 0.114162f, 0.0595356f, 0.0284665f, 0.0124793f, 0.00501592f, 0.00184846f, 0.000624555f, 0.000193478f,
		0.000169857f, 0.000548304f, 0.00162279f, 0.00440354f, 0.0109558f, 0.0249911f, 0.0522671f, 0.100224f, 0.176204f, 0.284029f, 0.419767f, 0.568794f, 0.706648f, 0.804919f, 0.840624f, 0.804919f, 0.706648f, 0.568794f, 0.419767f, 0.284029f, 0.176204f, 0.100224f, 0.0522671f, 0.0249911f, 0.0109558f, 0.00440354f, 0.00162279f, 0.000548304f, 0.000169857f,
		0.000136721f, 0.00044134f, 0.00130621f, 0.00354449f, 0.00881851f, 0.0201158f, 0.0420707f, 0.0806722f, 0.14183f, 0.22862f, 0.337878f, 0.457833f, 0.568794f, 0.647894f, 0.676634f, 0.647894f, 0.568794f, 0.457833f, 0.337878f, 0.22862f, 0.14183f, 0.0806722f, 0.0420707f, 0.0201158f, 0.00881851f, 0.00354449f, 0.00130621f, 0.00044134f, 0.000136721f,
		0.000100899f, 0.000325707f, 0.000963976f, 0.00261581f, 0.00650801f, 0.0148453f, 0.031048f, 0.0595356f, 0.10467f, 0.16872f, 0.249352f, 0.337878f, 0.419767f, 0.478142f, 0.499352f, 0.478142f, 0.419767f, 0.337878f, 0.249352f, 0.16872f, 0.10467f, 0.0595356f, 0.031048f, 0.0148453f, 0.00650801f, 0.00261581f, 0.000963976f, 0.000325707f, 0.000100899f,
		6.82718e-05f, 0.000220384f, 0.000652259f, 0.00176995f, 0.00440354f, 0.0100449f, 0.0210081f, 0.0402838f, 0.0708231f, 0.114162f, 0.16872f, 0.22862f, 0.284029f, 0.323527f, 0.337878f, 0.323527f, 0.284029f, 0.22862f, 0.16872f, 0.114162f, 0.0708231f, 0.0402838f, 0.0210081f, 0.0100449f, 0.00440354f, 0.00176995f, 0.000652259f, 0.000220384f, 6.82718e-05f,
		4.23542e-05f, 0.000136721f, 0.000404645f, 0.00109803f, 0.00273185f, 0.00623158f, 0.0130329f, 0.0249911f, 0.0439369f, 0.0708231f, 0.10467f, 0.14183f, 0.176204f, 0.200708f, 0.209611f, 0.200708f, 0.176204f, 0.14183f, 0.10467f, 0.0708231f, 0.0439369f, 0.0249911f, 0.0130329f, 0.00623158f, 0.00273185f, 0.00109803f, 0.000404645f, 0.000136721f, 4.23542e-05f,
		2.40908e-05f, 7.77661e-05f, 0.00023016f, 0.000624555f, 0.00155386f, 0.00354449f, 0.00741305f, 0.0142148f, 0.0249911f, 0.0402838f, 0.0595356f, 0.0806722f, 0.100224f, 0.114162f, 0.119226f, 0.114162f, 0.100224f, 0.0806722f, 0.0595356f, 0.0402838f, 0.0249911f, 0.0142148f, 0.00741305f, 0.00354449f, 0.00155386f, 0.000624555f, 0.00023016f, 7.77661e-05f, 2.40908e-05f,
		1.25634e-05f, 4.05552e-05f, 0.000120029f, 0.000325707f, 0.000810341f, 0.00184846f, 0.00386592f, 0.00741305f, 0.0130329f, 0.0210081f, 0.031048f, 0.0420707f, 0.0522671f, 0.0595356f, 0.0621765f, 0.0595356f, 0.0522671f, 0.0420707f, 0.031048f, 0.0210081f, 0.0130329f, 0.00741305f, 0.00386592f, 0.00184846f, 0.000810341f, 0.000325707f, 0.000120029f, 4.05552e-05f, 1.25634e-05f,
		6.0071e-06f, 1.93912e-05f, 5.73909e-05f, 0.000155734f, 0.000387458f, 0.000883827f, 0.00184846f, 0.00354449f, 0.00623158f, 0.0100449f, 0.0148453f, 0.0201158f, 0.0249911f, 0.0284665f, 0.0297292f, 0.0284665f, 0.0249911f, 0.0201158f, 0.0148453f, 0.0100449f, 0.00623158f, 0.00354449f, 0.00184846f, 0.000883827f, 0.000387458f, 0.000155734f, 5.73909e-05f, 1.93912e-05f, 6.0071e-06f,
		2.63344e-06f, 8.50083e-06f, 2.51594e-05f, 6.82718e-05f, 0.000169857f, 0.000387458f, 0.000810341f, 0.00155386f, 0.00273185f, 0.00440354f, 0.00650801f, 0.00881851f, 0.0109558f, 0.0124793f, 0.0130329f, 0.0124793f, 0.0109558f, 0.00881851f, 0.00650801f, 0.00440354f, 0.00273185f, 0.00155386f, 0.000810341f, 0.000387458f, 0.000169857f, 6.82718e-05f, 2.51594e-05f, 8.50083e-06f, 2.63344e-06f,
		1.05848e-06f, 3.4168e-06f, 1.01125e-05f, 2.7441e-05f, 6.82718e-05f, 0.000155734f, 0.000325707f, 0.000624555f, 0.00109803f, 0.00176995f, 0.00261581f, 0.00354449f, 0.00440354f, 0.00501592f, 0.00523842f, 0.00501592f, 0.00440354f, 0.00354449f, 0.00261581f, 0.00176995f, 0.00109803f, 0.000624555f, 0.000325707f, 0.000155734f, 6.82718e-05f, 2.7441e-05f, 1.01125e-05f, 3.4168e-06f, 1.05848e-06f,
		3.90068e-07f, 1.25916e-06f, 3.72666e-06f, 1.01125e-05f, 2.51594e-05f, 5.73909e-05f, 0.000120029f, 0.00023016f, 0.000404645f, 0.000652259f, 0.000963976f, 0.00130621f, 0.00162279f, 0.00184846f, 0.00193046f, 0.00184846f, 0.00162279f, 0.00130621f, 0.000963976f, 0.000652259f, 0.000404645f, 0.00023016f, 0.000120029f, 5.73909e-05f, 2.51594e-05f, 1.01125e-05f, 3.72666e-06f, 1.25916e-06f, 3.90068e-07f,
		1.31796e-07f, 4.25441e-07f, 1.25916e-06f, 3.4168e-06f, 8.50083e-06f, 1.93912e-05f, 4.05552e-05f, 7.77661e-05f, 0.000136721f, 0.000220384f, 0.000325707f, 0.00044134f, 0.000548304f, 0.000624555f, 0.000652259f, 0.000624555f, 0.000548304f, 0.00044134f, 0.000325707f, 0.000220384f, 0.000136721f, 7.77661e-05f, 4.05552e-05f, 1.93912e-05f, 8.50083e-06f, 3.4168e-06f, 1.25916e-06f, 4.25441e-07f, 1.31796e-07f,
		4.08284e-08f, 1.31796e-07f, 3.90068e-07f, 1.05848e-06f, 2.63344e-06f, 6.0071e-06f, 1.25634e-05f, 2.40908e-05f, 4.23542e-05f, 6.82718e-05f, 0.000100899f, 0.000136721f, 0.000169857f, 0.000193478f, 0.00020206f, 0.000193478f, 0.000169857f, 0.000136721f, 0.000100899f, 6.82718e-05f, 4.23542e-05f, 2.40908e-05f, 1.25634e-05f, 6.0071e-06f, 2.63344e-06f, 1.05848e-06f, 3.90068e-07f, 1.31796e-07f, 4.08284e-08f };
	
	gWeight[1] = weight2;
}
#endif


void CSoCTESTDlg::OnBnClickedButtonCamStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_PREVIEW)->SetWindowPos(NULL, 0, 0, 320, 240, SWP_NOZORDER | SWP_NOMOVE);
	m_webCam.StartCam(GetDlgItem(IDC_STATIC_PREVIEW), 320, 240, gBmpInfo);
	//gImageIn = LoadImageFromDialog();
	//GetDlgItem(IDC_STATIC_RESULT)->SetWindowPos(NULL, 0, 0, );
	gImageBuf = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	gImageOut = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	m_SIFT = CSIFT();
	m_SIFT.Init(320, 240);
	//InitScaleSpace();
	//InitDOG();
	//InitGradient();
	//InitWeight();
	m_webCam.SetCallBackOnFrame(ProcessCamFrame);
}


void CSoCTESTDlg::OnBnClickedButtonCamStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_webCam.StopCam();
}
