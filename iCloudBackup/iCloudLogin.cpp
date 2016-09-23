

#include "stdafx.h"
#include "ICloudLogin.h"
#include "BackupList.h"

CLoginWnd::CLoginWnd(LPCTSTR pszXMLName) :CDuiFrameBase(pszXMLName)
{
}
CLoginWnd::~CLoginWnd()
{
}

DUI_BEGIN_MESSAGE_MAP(CLoginWnd, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

void CLoginWnd::InitWindow()
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_wndShadow.Create(m_hWnd);
	m_wndShadow.SetSize(4);
	m_wndShadow.SetPosition(0, 0);

	m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
	m_pLoginBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("loginbtn")));
	m_pNameEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("nameedit")));
	m_pPasswdEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("passwdedit")));

	m_pLoginLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("loginsuccess")));

	//测试
	m_pNameEdit->SetText(USERNAME);
	m_pPasswdEdit->SetText(PASSWORD);
}

CControlUI* CLoginWnd::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (!strXML.IsEmpty())
	{
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}

	return NULL;
}



void CLoginWnd::Notify(TNotifyUI& msg)
{
	CDuiFrameBase::Notify(msg);
}


LRESULT CLoginWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = CDuiFrameBase::HandleMessage(uMsg, wParam, lParam);

	switch (uMsg)
	{
	case UWM_CREATE_BACKUPLIST:
	{
		CBackupListWnd* pBLWnd = new CBackupListWnd(_T("backuplist.xml"));
		pBLWnd->SetNamePasswd(m_pNameEdit->GetText(), m_pPasswdEdit->GetText());

		ShowWindow(SW_HIDE);
		pBLWnd->Init(&m_PaintManager);
		break;
	}

	default:
		break;
	}

	return lRes;
}

void CLoginWnd::OnClick(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pCloseBtn)
		{
			PostQuitMessage(0);
			return;
		}
		else if (msg.pSender == m_pMinBtn)
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if (msg.pSender == m_pLoginBtn)
		{
			UINT threadID;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, VerifyLoginThread, this, 0, &threadID);
			CloseHandle(hThread);

			return;
		}
	}
}

//验证账号密码
UINT CLoginWnd::VerifyLoginThread(LPVOID pParam)
{
	CLoginWnd* pThis = (CLoginWnd *)pParam;

	CBackupListWnd pBLWnd(_T("backuplist.xml"));
	pBLWnd.SetNamePasswd(pThis->m_pNameEdit->GetText(), pThis->m_pPasswdEdit->GetText());

	int rlt = pBLWnd.LoginSuccessOrNot();

	if (rlt != 0)
	{
		pThis->m_pLoginLabel->SetText(L"*您输入的账号密码不正确，请重新输入！");
		return -1;
	}
	else
	{
		pThis->m_pLoginLabel->SetText(L"");

		pThis->SendMessage(UWM_CREATE_BACKUPLIST, 0, 0);
	}

	_endthreadex(0);
	return 0;
}

