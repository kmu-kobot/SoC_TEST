
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
#define SCALE_SPACE_CHANNEL 1

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
		

		CByteImage tmp(nWidth, nHeight, SCALE_SPACE_CHANNEL);

		for (int r = 0; r < nHeight; r++)
		{
			BYTE* pDst = tmp.GetPtr(nHeight - r - 1);
			for (int c = 0; c < nWidth; c++)
			{
				pDst[c] = gImageBufGray.NearestNeighbor(c / dScale, r / dScale);
			}
		}

		dRadius = sqrt2 * 2 / dScale;

		ShowImage(tmp, std::to_string(nOtvIdx+100).c_str());
		//GaussianBlur(tmp, gScaleSpace[nOtvIdx], dRadius);
		gaussBlur(tmp, gScaleSpace[nOtvIdx], nWidth, nHeight, dRadius);

		ShowImage(gScaleSpace[nOtvIdx], std::to_string(nOtvIdx).c_str());

		for (int j = 1; j < 5; j++)
		{
			dRadius *= sqrt2;
			nIdx = nOtvIdx + j;
			//GaussianBlur(gScaleSpace[nOtvIdx], gScaleSpace[nIdx], 2 / dScale * sqrt(2));
			gaussBlur(gScaleSpace[nOtvIdx], gScaleSpace[nIdx], nWidth, nHeight, 2 / dScale * sqrt(2));

			ShowImage(gScaleSpace[nIdx], std::to_string(nIdx).c_str());
		}

		dScale /= 2;
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

	return TRUE;
}


void InitScaleSpace()
{
	gScaleSpace[0] = CByteImage(gBmpInfo.bmiHeader.biWidth * 2, gBmpInfo.bmiHeader.biHeight * 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[1] = CByteImage(gBmpInfo.bmiHeader.biWidth * 2, gBmpInfo.bmiHeader.biHeight * 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[2] = CByteImage(gBmpInfo.bmiHeader.biWidth * 2, gBmpInfo.bmiHeader.biHeight * 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[3] = CByteImage(gBmpInfo.bmiHeader.biWidth * 2, gBmpInfo.bmiHeader.biHeight * 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[4] = CByteImage(gBmpInfo.bmiHeader.biWidth * 2, gBmpInfo.bmiHeader.biHeight * 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[5] = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, SCALE_SPACE_CHANNEL);
	gScaleSpace[6] = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, SCALE_SPACE_CHANNEL);
	gScaleSpace[7] = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, SCALE_SPACE_CHANNEL);
	gScaleSpace[8] = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, SCALE_SPACE_CHANNEL);
	gScaleSpace[9] = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, SCALE_SPACE_CHANNEL);
	gScaleSpace[10] = CByteImage(gBmpInfo.bmiHeader.biWidth / 2, gBmpInfo.bmiHeader.biHeight / 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[11] = CByteImage(gBmpInfo.bmiHeader.biWidth / 2, gBmpInfo.bmiHeader.biHeight / 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[12] = CByteImage(gBmpInfo.bmiHeader.biWidth / 2, gBmpInfo.bmiHeader.biHeight / 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[13] = CByteImage(gBmpInfo.bmiHeader.biWidth / 2, gBmpInfo.bmiHeader.biHeight / 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[14] = CByteImage(gBmpInfo.bmiHeader.biWidth / 2, gBmpInfo.bmiHeader.biHeight / 2, SCALE_SPACE_CHANNEL);
	gScaleSpace[15] = CByteImage(gBmpInfo.bmiHeader.biWidth / 4, gBmpInfo.bmiHeader.biHeight / 4, SCALE_SPACE_CHANNEL);
	gScaleSpace[16] = CByteImage(gBmpInfo.bmiHeader.biWidth / 4, gBmpInfo.bmiHeader.biHeight / 4, SCALE_SPACE_CHANNEL);
	gScaleSpace[17] = CByteImage(gBmpInfo.bmiHeader.biWidth / 4, gBmpInfo.bmiHeader.biHeight / 4, SCALE_SPACE_CHANNEL);
	gScaleSpace[18] = CByteImage(gBmpInfo.bmiHeader.biWidth / 4, gBmpInfo.bmiHeader.biHeight / 4, SCALE_SPACE_CHANNEL);
	gScaleSpace[19] = CByteImage(gBmpInfo.bmiHeader.biWidth / 4, gBmpInfo.bmiHeader.biHeight / 4, SCALE_SPACE_CHANNEL);
}


void CSoCTESTDlg::OnBnClickedButtonCamStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_STATIC_PREVIEW)->SetWindowPos(NULL, 0, 0, 320, 240, SWP_NOZORDER | SWP_NOMOVE);
	m_webCam.StartCam(GetDlgItem(IDC_STATIC_PREVIEW), 320, 240, gBmpInfo);
	//GetDlgItem(IDC_STATIC_RESULT)->SetWindowPos(NULL, 0, 0, );
	gImageBuf = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	InitScaleSpace();
	m_webCam.SetCallBackOnFrame(ProcessCamFrame);
}


void CSoCTESTDlg::OnBnClickedButtonCamStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_webCam.StopCam();
}