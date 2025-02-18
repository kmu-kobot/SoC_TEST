
// SoC_TESTDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SoC_TEST.h"
#include "SoC_TESTDlg.h"
#include "afxdialogex.h"

#include "MyImage.h"
#include "ImageFrameWndManager.h"
#include "LoadImageFromFileDialog.h"
#include "MyImageFunc.h"
#include "VideoProcessing.h"
#include "Sift.h"
#include "Tracking.h"
#include <string>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WIDTH 640
#define HEIGHT 480
#define VIDEO


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
	ON_BN_CLICKED(IDC_BUTTON_SAMPLING, &CSoCTESTDlg::OnBnClickedButtonSampling)
	ON_BN_CLICKED(IDC_BUTTON_MATCHING, &CSoCTESTDlg::OnBnClickedButtonMatching)
END_MESSAGE_MAP()


// CSoCTESTDlg 메시지 처리기
int frame;

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
	m_pFrame = &frame;


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
CByteImage gImageInv;
CByteImage gImageInvDown;
BITMAPINFO gBmpInfo;
CSift gSift;
Tracking *tracking;

bool successTracking = false;
bool isCheckedTracking = false;
bool isCheckedMatching = false;
bool isMatched;
bool doSift = true;
unsigned int frameCnt = 0;
Point leftTop, rightBottom;

UINT CSoCTESTDlg::FrameCheck(LPVOID _method)
{
	CSoCTESTDlg *pDlg = (CSoCTESTDlg *)_method;
	std::string str;

	while (pDlg->m_eThreadWork == THREAD_RUNNING)
	{
		str = std::string("Frame: ") + std::to_string(*pDlg->m_pFrame);
		pDlg->SetDlgItemText(IDC_STATIC_FRAME, str.c_str());
		*pDlg->m_pFrame = 0;
		Sleep(1000);
	}
	return 0;
}


LRESULT ProcessCamFrame(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	if (gBmpInfo.bmiHeader.biCompression == BI_RGB) // RGB 영상
	{
		memcpy(gImageBuf.GetPtr(), lpVHdr->lpData,
			gBmpInfo.bmiHeader.biHeight*gBmpInfo.bmiHeader.biWidth*3);
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

	int width = gImageBuf.GetWidth();
	int hWidth = gImageInvDown.GetWidth();
	int qWidth = hWidth >> 1;
	int height = gImageBuf.GetHeight();
	int hHeight = gImageInvDown.GetHeight();
	int qHeight = hHeight >> 1;
	int wstep = gImageBuf.GetWStep();
	int hWstep = gImageInvDown.GetWStep();
	int channel = gImageInvDown.GetChannel();

	for (int r = 0; r < height; ++r)
	{
		for (int c = 0; c < width; ++c)
		{
			memcpy(gImageInv.GetPtr(r), gImageBuf.GetPtr(height - r - 1), wstep * sizeof(BYTE));
		}
	}

	for (int r = 0; r < hHeight; ++r)
	{
		for (int c = 0; c < hWidth; ++c)
		{
			memcpy(gImageInvDown.GetPtr(r), gImageInv.GetPtr(r + qHeight) + qWidth * channel, hWstep * sizeof(BYTE));
		}
	}

	doSift |= (++frameCnt & 32) >> 5;
	frameCnt &= 31;

	if (doSift)
	{
		gSift.detectFeature(gImageInvDown, "input Down");

		if (isCheckedMatching)
		{
			isMatched = gSift.keyMatching();

			if (isMatched)
			{
				doSift = false;
				gSift.getObjectArea(leftTop, rightBottom);
				tracking = new Tracking(
					{ leftTop.x + (WIDTH >> 2), leftTop.y + (HEIGHT >> 2) },
					{ rightBottom.x + (WIDTH >> 2), rightBottom.y + (HEIGHT >> 2) },
					3, 3, 3
				);
			}
		}
	}
	else if (isCheckedMatching && !successTracking)
	{
		gSift.detectFeature(gImageInvDown, "input Down");
		isMatched = gSift.keyMatching();

		if (!isMatched )
		{
			++frame;
			std::this_thread::yield();
			return TRUE;
		}

		gSift.getObjectArea(leftTop, rightBottom);
		tracking = new Tracking(
			{ leftTop.x + (WIDTH >> 2), leftTop.y + (HEIGHT >> 2)},
			{ rightBottom.x + (WIDTH >> 2), rightBottom.y + (HEIGHT >> 2) },
			3, 3, 3
		);
	}

	if (isCheckedTracking && tracking != NULL) {
		if (!tracking->getIsSetFeatureColor()) {
			tracking->setFeatureH(gImageInv);
			tracking->setFeatureS(gImageInv);
			tracking->setFeatureV(gImageInv);
		}
		successTracking = tracking->tracking(gImageInv);
		if (!successTracking) {
			delete tracking;
		}
	}

	++frame;
	std::this_thread::yield();
	return TRUE;
}


void CSoCTESTDlg::OnBnClickedButtonCamStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
#ifdef VIDEO
	GetDlgItem(IDC_STATIC_PREVIEW)->SetWindowPos(NULL, 0, 0, WIDTH, HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
	m_webCam.StartCam(GetDlgItem(IDC_STATIC_PREVIEW), WIDTH, HEIGHT, gBmpInfo);
	//GetDlgItem(IDC_STATIC_RESULT)->SetWindowPos(NULL, 0, 0, );
	gImageBuf = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	gImageInv = CByteImage(gBmpInfo.bmiHeader.biWidth, gBmpInfo.bmiHeader.biHeight, 3);
	gImageInvDown = CByteImage(gBmpInfo.bmiHeader.biWidth >> 1, gBmpInfo.bmiHeader.biHeight >> 1, 3);
	m_webCam.SetCallBackOnFrame(ProcessCamFrame);
	gSift.init(gImageInvDown);
#else
	gImageBuf = LoadImageFromDialog();
	gSift.init(gImageBuf);
	gSift.detectFeature(gImageBuf, "input Feature");
#endif

	if (m_pThread == NULL)
	{
		m_pThread = AfxBeginThread(FrameCheck, this);

		if (m_pThread == NULL)
		{
			AfxMessageBox("Error");
		}

		m_pThread->m_bAutoDelete = FALSE;
		m_eThreadWork = THREAD_RUNNING;
	}
	else
	{
		if (m_eThreadWork == THREAD_PAUSE)
		{
			m_pThread->ResumeThread();
			m_eThreadWork = THREAD_RUNNING;
		}
	}
}


void CSoCTESTDlg::OnBnClickedButtonCamStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	isCheckedMatching = false;
	m_webCam.StopCam();
	CloseAllImages();

	if (m_pThread == NULL)
	{
		AfxMessageBox("Thread Not Start");
	}
	else
	{
		m_pThread->SuspendThread();

		DWORD dwResult;
		::GetExitCodeThread(m_pThread->m_hThread, &dwResult);

		delete m_pThread;
		m_pThread = NULL;

		m_eThreadWork = THREAD_STOP;
	}
}

void CSoCTESTDlg::OnBnClickedButtonSampling()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	for (int i = 0; i < NUM_SAMPLE; ++i)
	{
		gImageBuf = LoadImageFromDialog();
		gSift.buildSample(gImageBuf, i);
	}
	this->OnBnClickedButtonCamStart();
}


void CSoCTESTDlg::OnBnClickedButtonMatching()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	isCheckedMatching = true;
	gSift.keyMatching();
	isCheckedTracking = true;
}
