
// SoC_TESTDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SoC_TEST.h"
#include "SoC_TESTDlg.h"
#include "afxdialogex.h"

#include <string>
#include <list>
#include "MyImage.h"
#include "ImageFrameWndManager.h"
#include "LoadImageFromFileDialog.h"
#include "MyImageFunc.h"
#include "VideoProcessing.h"

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
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

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

typedef struct Feature
{
	int octave, scale;
	float x, y, value;
	Feature(int o, int s, float x, float y, int v)
		:octave(o), scale(s), x(x), y(y), value(v) {};
} feature_t;

CByteImage gImageBuf;
CByteImage gImageOut;
CByteImage gImageBufGray;
//CByteImage gImageIn;
BITMAPINFO gBmpInfo;
CByteImage gScaleTemp[4];
CByteImage gScaleSpace[20];
CIntImage gDOG[16];
//CByteImage gKeyPoint[8];
std::vector<feature_t> feature;
std::vector<feature_t>::iterator itr;
#define NUM_CHANNEL 1
int gWidth[4];
int gHeight[4];
int gWStep[4];
int gSize[4];

void BuildScaleSpace()
{

	static double dSigma[4] = { 1.6, 1.6*sqrt(2), 3.2, 3.2*sqrt(2) };
	for (int r = 0; r <gHeight[0]; r++)
	{
		BYTE* pDst = gScaleTemp[0].GetPtr(gHeight[0] - r - 1);
		for (int c = 0; c < gWidth[0]; c++)
		{
			pDst[c] = gImageBufGray.CubicConvIntp(c / 2, r / 2);
		}
	}

	//ShowImage(gScaleTemp[0], "1/2 Scale Original Image");
	gaussBlur(gScaleTemp[0], gScaleSpace[0], dSigma[0]);
	//ShowImage(gScaleTemp[0], std::to_string(0).c_str());

	for (int i = 0; i < 4; i++)
	{
		memcpy(gScaleTemp[0].GetPtr(), gScaleSpace[i].GetPtr(), gSize[0]);
		gaussBlur(gScaleTemp[0], gScaleSpace[i + 1], dSigma[i]);
		//ShowImage(gScaleSpace[i], std::to_string(i + 1).c_str());
	}

	for (int i = 1; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int r = 0; r < gHeight[i]; r++)
			{
				BYTE* pDst = gScaleSpace[i * 5 + j].GetPtr(r);
				for (int c = 0; c < gWidth[i]; c++)
				{
					pDst[c] = gScaleSpace[(i - 1) * 5 + j + 2].NearestNeighbor(c*2, r*2);
				}
			}
		}

		for (int j = 2; j < 4; j++)
		{
			memcpy(gScaleTemp[i].GetPtr(), gScaleSpace[i * 5 + j].GetPtr(), gSize[i]);
			gaussBlur(gScaleTemp[i], gScaleSpace[i * 5 + j + 1], dSigma[j]);
		}
	}

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

void Norm(CIntImage& m_imageIn, int n)
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
		int* pIn = m_imageIn.GetPtr(r);

		for (c = 0; c<nWidth; c++)
		{
			m_histogram[pIn[c]+256]++;
		}
	}

	double dNormFactor = 256.0 / (nWidth * nHeight);

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
		int* pIn = m_imageIn.GetPtr(r);
		BYTE* pOut = m_imageOut.GetPtr(r);
		for (c = 0; c<nWidth; c++)
		{
			pOut[c] = (BYTE)(m_histogramCdf[pIn[c]+256]);
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

	for (int i = 0; i < 16; i++)
	{
		int* pSrc = gDOG[i].GetPtr();

		//Norm(gDOG[i], i);
		//ShowImage(gDOG[i], std::to_string(i).c_str());
	}
}

void FindKeyPoint()
{
	feature.clear();

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
				int* src0 = gDOG[i * 4 + j].GetPtr(r);
				int* src1 = gDOG[i * 4 + j + 1].GetPtr(r);
				int* src2 = gDOG[i * 4 + j + 2].GetPtr(r);
				//int* dst = gKeyPoint[i * 2 + j].GetPtr(r);

				for (int c = 1; c < nWidth - 1; c++)
				{
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
			//ShowImage(gKeyPoint[i * 2 + j], std::to_string(i * 2 + j).c_str(), x, j * (30 + gHeight[i]));
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

	B[0] = -1 * (InverseH[0] * D[0] + InverseH[1] * D[1] * InverseH[2] * D[2]);
	B[1] = -1 * (InverseH[3] * D[0] + InverseH[4] * D[1] * InverseH[5] * D[2]);
	B[2] = -1 * (InverseH[6] * D[0] + InverseH[7] * D[1] * InverseH[8] * D[2]);

	return true;
}

bool SubPixel(feature_t& key, int nAdjustment = 2)
{
	bool needToAdjust = true;
	int x = key.x;
	int y = key.y;
	int s = key.scale;
	int v = key.value;
	int o = key.octave;

	while (needToAdjust)
	{
		if (s <= 0 || s >= 3)
			return false;
		if (x <= 0 || x >= gWidth[o] - 1)
			return false;
		if (y <= 0 || y >= gHeight[o] - 1)
			return false;

		float dp = 0.0; // dot product

		float H[9] = { gDOG[o * 4 + s - 1].GetAt(x, y) - 2 * gDOG[o * 4 + s].GetAt(x, y) + gDOG[o * 4 + s + 1].GetAt(x, y),  // below[y][x] - 2*current[y][x] + above[y][x]
						0.25 * (gDOG[o * 4 + s + 1].GetAt(x, y + 1) - gDOG[o * 4 + s + 1].GetAt(x, y - 1) - (gDOG[o * 4 + s - 1].GetAt(x, y + 1) - gDOG[o * 4 + s - 1].GetAt(x, y - 1))), // 0.25 * (above[y+1][x] - above[y-1][x] - (below[y+1][x]-below[y-1][x]))
						0.25 * (gDOG[o * 4 + s + 1].GetAt(x + 1, y) - gDOG[o * 4 + s + 1].GetAt(x - 1, y) - (gDOG[o * 4 + s - 1].GetAt(x + 1, y) - gDOG[o * 4 + s - 1].GetAt(x - 1, y))), // 0.25 * (above[y][x+1] - above[y][x-1] - (below[y][x+1]-below[y][x-1]))
						0.25 * (gDOG[o * 4 + s + 1].GetAt(x, y + 1) - gDOG[o * 4 + s + 1].GetAt(x, y - 1) - (gDOG[o * 4 + s - 1].GetAt(x, y + 1) - gDOG[o * 4 + s - 1].GetAt(x, y - 1))), // H[1][0] = H[0][1]
						gDOG[o * 4 + s].GetAt(x, y - 1) - 2 * gDOG[o * 4 + s].GetAt(x, y) + gDOG[o * 4 + s].GetAt(x, y + 1),  // current[y - 1][x] - 2*current[y][x] + current[y + 1][x]
						0.25 * (gDOG[o * 4 + s].GetAt(x + 1, y + 1) - gDOG[o * 4 + s].GetAt(x - 1, y + 1) - (gDOG[o * 4 + s].GetAt(x + 1, y - 1) - gDOG[o * 4 + s].GetAt(x - 1, y - 1))), // 0.25 * (current[y+1][x+1] - current[y+1][x-1] - (current[y-1][x+1]-current[y-1][x-1]))
						0.25 * (gDOG[o * 4 + s + 1].GetAt(x + 1, y) - gDOG[o * 4 + s + 1].GetAt(x - 1, y) - (gDOG[o * 4 + s - 1].GetAt(x + 1, y) - gDOG[o * 4 + s - 1].GetAt(x - 1, y))), // H[2][0] = H[0][2]
						0.25 * (gDOG[o * 4 + s].GetAt(x + 1, y + 1) - gDOG[o * 4 + s].GetAt(x - 1, y + 1) - (gDOG[o * 4 + s].GetAt(x + 1, y - 1) - gDOG[o * 4 + s].GetAt(x - 1, y - 1))), // H[2][1] = H[1][2]
						gDOG[o * 4 + s].GetAt(x - 1, y) - 2 * gDOG[o * 4 + s].GetAt(x, y) + gDOG[o * 4 + s].GetAt(x + 1, y) };  // current[y][x-1] - 2*current[y][x] + current[y1][x+1]
		
		
		float yong[6] = { gDOG[o * 4 + s - 1].GetAt(x, y) , gDOG[o * 4 + s + 1].GetAt(x, y),
			gDOG[o * 4 + s].GetAt(x, y + 1) , gDOG[o * 4 + s].GetAt(x, y - 1),
			gDOG[o * 4 + s].GetAt(x + 1, y) , gDOG[o * 4 + s].GetAt(x - 1, y) };


		float D[3] = { gDOG[o * 4 + s - 1].GetAt(x, y) - gDOG[o * 4 + s + 1].GetAt(x, y), // below[y][x] - above[y][x]
						gDOG[o * 4 + s].GetAt(x, y + 1) - gDOG[o * 4 + s].GetAt(x, y - 1),  // current[y+1][x] - current[y+1][x]
						gDOG[o * 4 + s].GetAt(x + 1, y) - gDOG[o * 4 + s].GetAt(x - 1, y) };  // current[y][x+1] - current[y][x-1]


		float B[3];

		Solve(H, D, B);

		dp = B[0] * D[0] + B[1] * D[1] + B[2] * D[2];

		if (abs(B[0]) > 0.5 || abs(B[1]) > 0.5 || abs(B[2]) > 0.5)
		{
			if (nAdjustment == 0)
				return false;

			nAdjustment -= 1;

			double distSq = B[1] * B[1] + B[2] * B[2];

			if (distSq > 2.0)
				return false;

			x = (int)(x + B[2] + 0.5);
			y = (int)(y + B[1] + 0.5);
			s = (int)(s + B[0] + 0.5);

			continue;
		}

		key.x = x + B[2];
		key.y = y + B[1];
		key.scale = s + B[0] + 0.5;
		key.value = gDOG[o * 4 + s].GetAt((int)key.x, (int)key.y) + 0.5 * dp;

		if (key.value < 0.03)
			return false;

		return true;
	}
	return false;
}

bool isCorner(feature_t& key)
{
	return true;
}

void EliminateLowContrast()
{
	std::vector<feature_t>::iterator tmp;
	for (itr = feature.begin(); itr != feature.end();)
	{
		if (!SubPixel(*itr) || fabs((*itr).value) < 0.03 || !isCorner(*itr))
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
		int pos = (itr->y * size * wstep) + (itr->x * size * gImageOut.GetChannel());
		out[pos] = 255;
		out[pos+1] = 255;
		out[pos+2] = 255;
	}
	ShowImage(gImageOut, "keyPoints");
}

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

	BuildScaleSpace();
	DiffrenceOfGaussian();
	FindKeyPoint();
	EliminateLowContrast();

	return TRUE;
}


void InitScaleSpace()
{
	int nWidth = gBmpInfo.bmiHeader.biWidth;
	int nHeight = gBmpInfo.bmiHeader.biHeight;
	gScaleTemp[0] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleTemp[1] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleTemp[2] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleTemp[3] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gScaleSpace[0] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleSpace[1] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleSpace[2] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleSpace[3] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleSpace[4] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gScaleSpace[5] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleSpace[6] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleSpace[7] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleSpace[8] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleSpace[9] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gScaleSpace[10] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleSpace[11] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleSpace[12] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleSpace[13] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleSpace[14] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gScaleSpace[15] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gScaleSpace[16] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gScaleSpace[17] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gScaleSpace[18] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gScaleSpace[19] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);

	for(int i = 0; i < 4; i++)
	{
		gWidth[i] = gScaleTemp[i].GetWidth();
		gHeight[i] = gScaleTemp[i].GetHeight();
		gWStep[i] = gScaleTemp[i].GetWStep();
		gSize[i] = gHeight[i] * gWStep[i];
	};
}


void InitDOG()
{
	int nWidth = gBmpInfo.bmiHeader.biWidth;
	int nHeight = gBmpInfo.bmiHeader.biHeight;
	gDOG[0] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gDOG[1] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gDOG[2] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gDOG[3] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gDOG[4] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gDOG[5] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gDOG[6] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gDOG[7] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gDOG[8] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gDOG[9] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gDOG[10] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gDOG[11] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gDOG[12] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gDOG[13] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gDOG[14] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gDOG[15] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
}


void CSoCTESTDlg::OnBnClickedButtonCamStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_PREVIEW)->SetWindowPos(NULL, 0, 0, 320, 240, SWP_NOZORDER | SWP_NOMOVE);
	m_webCam.StartCam(GetDlgItem(IDC_STATIC_PREVIEW), 320, 240, gBmpInfo);
	//gImageIn = LoadImageFromDialog();
	//GetDlgItem(IDC_STATIC_RESULT)->SetWindowPos(NULL, 0, 0, );
	gImageBuf = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	gImageOut = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	InitScaleSpace();
	InitDOG();
	m_webCam.SetCallBackOnFrame(ProcessCamFrame);
}


void CSoCTESTDlg::OnBnClickedButtonCamStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_webCam.StopCam();
}
