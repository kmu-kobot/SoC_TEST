#pragma once

#include "ImageView.h"

// CImageFrameWnd frame

class CImageFrameWnd : public CFrameWnd
{
public:
	CImageFrameWnd();
	//CImageFrameWnd(const CByteImage &image, const char *name = NULL);
	CImageFrameWnd(const CByteImage &image, const char *name = NULL, int x = 0, int y = 0);
	virtual ~CImageFrameWnd();
	CImageView& GetImageView() { return m_view; };

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void PostNcDestroy();

	CImageView	m_view;
};


