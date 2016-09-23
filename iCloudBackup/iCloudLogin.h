#pragma once
#include "DuiFrameBase.h"

class CLoginWnd : public CDuiFrameBase
{
public:
	CLoginWnd(LPCTSTR pszXMLName);
	~CLoginWnd();

	DUI_DECLARE_MESSAGE_MAP()

	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnClick(TNotifyUI& msg);
	virtual CControlUI* CreateControl(LPCTSTR pstrClassName);

public:
	static UINT WINAPI VerifyLoginThread(LPVOID pParam);

private:
	CButtonUI*	m_pCloseBtn;
	CButtonUI*	m_pMinBtn;
	CButtonUI*	m_pLoginBtn;
	CEditUI*	m_pNameEdit;
	CEditUI*	m_pPasswdEdit;
	CLabelUI*	m_pLoginLabel;
};
