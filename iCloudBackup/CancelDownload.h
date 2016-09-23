#pragma once

#pragma once
#include "DuiFrameBase.h"
#include "common.h"

class CCancelDownloadDlg : public CDuiFrameBase
{
public:
	CCancelDownloadDlg(LPCTSTR pszXMLName);
	~CCancelDownloadDlg();

	DUI_DECLARE_MESSAGE_MAP()

public:
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);
	virtual void    OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnClick(TNotifyUI& msg);

	void Init(CPaintManagerUI *pOwnerPM, int index, int nTitleFlag);

private:
	CButtonUI*	m_pOkBtn;
	CButtonUI*	m_pCancelBtn;
	CLabelUI*	m_pTitleLabel;
	int			m_iIndex;
	int			m_TitleFlag;

};


