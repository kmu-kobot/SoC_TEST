
// SoC_TESTDlg.h: 헤더 파일
//

#pragma once

#include "VideoProcessing.h"

enum ThreadWorkingType
{
	THREAD_STOP = 0,
	THREAD_RUNNING = 1,
	THREAD_PAUSE = 2
};

// CSoCTESTDlg 대화 상자
class CSoCTESTDlg : public CDialog
{
// 생성입니다.
public:
	CSoCTESTDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOC_TEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonCamStart();
	afx_msg void OnBnClickedButtonCamStop();
protected:
	CWebCam		m_webCam;

	CByteImage m_imageSample;

	bool m_bFrameCheckStart;
	CWinThread *m_pThread;
	ThreadWorkingType m_eThreadWork;
	int* m_pFrame;
public:
	afx_msg void OnBnClickedButtonSampling();
	afx_msg void OnBnClickedButtonMatching();

	static UINT FrameCheck(LPVOID _method);
};
