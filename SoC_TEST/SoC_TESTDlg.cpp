
// SoC_TESTDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SoC_TEST.h"
#include "SoC_TESTDlg.h"
#include "afxdialogex.h"

#include <string>
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

CByteImage gImageBuf;
CByteImage gImageBufGray;
BITMAPINFO gBmpInfo;
CByteImage gScaleSpace[20];
CByteImage gDOG[16];
CByteImage gKeyPoint[8];
#define NUM_CHANNEL 1

void BuildScaleSpace()
{
	double dScale = 2;
	int nOtvIdx;
	int nIdx;
	double dRadius;
	double sqrt2 = sqrt(2);
	for (int i = 0; i < 4; i++)
	{
		nOtvIdx = i * 5;
		int nWStep = gScaleSpace[nOtvIdx].GetWStep();
		int nWidth = gScaleSpace[nOtvIdx].GetWidth();
		int nHeight = gScaleSpace[nOtvIdx].GetHeight();
		
		for (int r = 0; r < nHeight; r++)
		{
			BYTE* pDst = gScaleSpace[nOtvIdx].GetPtr(nHeight - r - 1);
			for (int c = 0; c < nWidth; c++)
			{
				pDst[c] = gImageBufGray.NearestNeighbor(c / dScale, r / dScale);
			}
		}

		dRadius = sqrt2 * 2 / dScale;

		//ShowImage(gScaleSpace[nOtvIdx], std::to_string(nOtvIdx).c_str());

		for (int j = 1; j < 5; j++)
		{
			nIdx = nOtvIdx + j;
			gaussBlur(gScaleSpace[nOtvIdx], gScaleSpace[nIdx], nWidth, nHeight, 2 / dScale * sqrt(2));

			//ShowImage(gScaleSpace[nIdx], std::to_string(nIdx).c_str());
			dRadius *= sqrt2;
		}

		dScale /= 2;
	}

}

//void Norm(CByteImage& m_imageIn, int n)
//{
//	if (m_imageIn.GetChannel() != 1)
//	{
//		AfxMessageBox("회색조 영상을 입력하세요.");
//		return;
//	}
//
//	int nWidth = m_imageIn.GetWidth();
//	int nHeight = m_imageIn.GetHeight();
//
//	CByteImage m_imageOut(nWidth, nHeight, 1);
//	int m_histogram[256];
//	double m_histogramCdf[256];
//
//	memset(m_histogram, 0, 256 * sizeof(int));
//
//	int r, c;
//	for (r = 0; r<nHeight; r++)
//	{
//		BYTE* pIn = m_imageIn.GetPtr(r);
//
//		for (c = 0; c<nWidth; c++)
//		{
//			m_histogram[pIn[c]]++;
//		}
//	}
//
//	double dNormFactor = 255.0 / (nWidth * nHeight);
//
//	for (int i = 0; i<256; i++)
//	{
//		m_histogramCdf[i] = m_histogram[i] * dNormFactor;
//	}
//
//	for (int i = 1; i<256; i++)
//	{
//		m_histogramCdf[i] = m_histogramCdf[i - 1] + m_histogramCdf[i];
//	}
//
//	for (r = 0; r<nHeight; r++)
//	{
//		BYTE* pIn = m_imageIn.GetPtr(r);
//		BYTE* pOut = m_imageOut.GetPtr(r);
//		for (c = 0; c<nWidth; c++)
//		{
//			pOut[c] = (BYTE)(m_histogramCdf[pIn[c]] + 0.5);
//		}
//	}
//
//	ShowImage(m_imageOut, std::to_string(n).c_str());
//}

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
	//	Norm(gDOG[i], i);
	//}
}

void FindKeyPoint()
{
	gKeyPoint[0].SetConstValue(0);
	gKeyPoint[1].SetConstValue(0);
	gKeyPoint[2].SetConstValue(0);
	gKeyPoint[3].SetConstValue(0);
	gKeyPoint[4].SetConstValue(0);
	gKeyPoint[5].SetConstValue(0);
	gKeyPoint[6].SetConstValue(0);
	gKeyPoint[7].SetConstValue(0);
	for (int i = 0; i < 4; i++)
	{
		int nWidth = gDOG[i * 5].GetWidth();
		int nHeight = gDOG[i * 5].GetHeight();
		int nWStep = gDOG[i * 5].GetWStep();

		for (int j = 0; j < 2; j++)
		{
			for (int r = 1; r < nHeight - 1; r++)
			{
				BYTE* src1 = gDOG[i * 4 + j].GetPtr(r);
				BYTE* src2 = gDOG[i * 4 + j + 1].GetPtr(r);
				BYTE* src3 = gDOG[i * 4 + j + 2].GetPtr(r);
				BYTE* dst = gKeyPoint[i * 2 + j].GetPtr(r);

				for (int c = 1; c < nWidth - 1; c++)
				{
					if (src2[c] == src1[c] || src2[c] == src3[c])
						continue;
					bool max = src2[c] > src1[c];
					bool max3 = src2[c] > src3[c];
					if (max ^ max3)
						continue;
					
					if (max)
					{
						if (src2[c] > src2[c - 1] && src2[c] > src2[c + 1] &&
							src2[c] > src2[c - 1 - nWStep] && src2[c] > src2[c - nWStep] && src2[c] > src2[c + 1 - nWStep] &&
							src2[c] > src2[c - 1 + nWStep] && src2[c] > src2[c + nWStep] && src2[c] > src2[c + 1 + nWStep] &&
							src2[c] > src1[c - 1] && src2[c] > src1[c + 1] &&
							src2[c] > src3[c - 1] && src2[c] > src3[c + 1] &&
							src2[c] > src1[c - 1 - nWStep] && src2[c] > src1[c - nWStep] && src2[c] > src1[c + 1 - nWStep] &&
							src2[c] > src3[c - 1 - nWStep] && src2[c] > src3[c - nWStep] && src2[c] > src3[c + 1 - nWStep] &&
							src2[c] > src1[c - 1 + nWStep] && src2[c] > src1[c + nWStep] && src2[c] > src1[c + 1 + nWStep] &&
							src2[c] > src3[c - 1 + nWStep] && src2[c] > src3[c + nWStep] && src2[c] > src3[c + 1 + nWStep]
							)
							//dst[c] = src2[c];
							dst[c] = 255;
					}
					else
					{
						if (src2[c] < src2[c - 1] && src2[c] < src2[c + 1] &&
							src2[c] < src2[c - 1 - nWStep] && src2[c] < src2[c - nWStep] && src2[c] < src2[c + 1 - nWStep] &&
							src2[c] < src2[c - 1 + nWStep] && src2[c] < src2[c + nWStep] && src2[c] < src2[c + 1 + nWStep] &&
							src2[c] < src1[c - 1] && src2[c] < src1[c + 1] &&
							src2[c] < src3[c - 1] && src2[c] < src3[c + 1] &&
							src2[c] < src1[c - 1 - nWStep] && src2[c] < src1[c - nWStep] && src2[c] < src1[c + 1 - nWStep] &&
							src2[c] < src3[c - 1 - nWStep] && src2[c] < src3[c - nWStep] && src2[c] < src3[c + 1 - nWStep] &&
							src2[c] < src1[c - 1 + nWStep] && src2[c] < src1[c + nWStep] && src2[c] < src1[c + 1 + nWStep] &&
							src2[c] < src3[c - 1 + nWStep] && src2[c] < src3[c + nWStep] && src2[c] < src3[c + 1 + nWStep]
							)
							//dst[c] = src2[c];
							dst[c] = 255;
					}
				}
			}

			ShowImage(gKeyPoint[i * 2 + j], std::to_string(i * 2 + j).c_str());
		}
	}
}

LRESULT ProcessCamFrame(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	if (gBmpInfo.bmiHeader.biCompression == BI_RGB) // RGB 영상
	{
		memcpy(gImageBuf.GetPtr(), lpVHdr->lpData,
			gBmpInfo.bmiHeader.biHeight*gBmpInfo.bmiHeader.biWidth*3);
		gImageBufGray = RGB2Gray(gImageBuf);
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

	return TRUE;
}


void InitScaleSpace()
{
	int nWidth = gBmpInfo.bmiHeader.biWidth;
	int nHeight = gBmpInfo.bmiHeader.biHeight;
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

void InitKeyPoint()
{
	int nWidth = gBmpInfo.bmiHeader.biWidth;
	int nHeight = gBmpInfo.bmiHeader.biHeight;
	gKeyPoint[0] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gKeyPoint[1] = CByteImage(nWidth * 2, nHeight * 2, NUM_CHANNEL);
	gKeyPoint[2] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gKeyPoint[3] = CByteImage(nWidth, nHeight, NUM_CHANNEL);
	gKeyPoint[4] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gKeyPoint[5] = CByteImage(nWidth / 2, nHeight / 2, NUM_CHANNEL);
	gKeyPoint[6] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
	gKeyPoint[7] = CByteImage(nWidth / 4, nHeight / 4, NUM_CHANNEL);
}

void CSoCTESTDlg::OnBnClickedButtonCamStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_PREVIEW)->SetWindowPos(NULL, 0, 0, 320, 240, SWP_NOZORDER | SWP_NOMOVE);
	m_webCam.StartCam(GetDlgItem(IDC_STATIC_PREVIEW), 320, 240, gBmpInfo);
	//GetDlgItem(IDC_STATIC_RESULT)->SetWindowPos(NULL, 0, 0, );
	gImageBuf = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	InitScaleSpace();
	InitDOG();
	InitKeyPoint();
	m_webCam.SetCallBackOnFrame(ProcessCamFrame);
}


void CSoCTESTDlg::OnBnClickedButtonCamStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_webCam.StopCam();
}