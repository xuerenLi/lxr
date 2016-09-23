#include "stdafx.h"
#include "CancelDownload.h"


CCancelDownloadDlg::CCancelDownloadDlg(LPCTSTR pszXMLName)
	:CDuiFrameBase(pszXMLName)
{
}


CCancelDownloadDlg::~CCancelDownloadDlg()
{
}

DUI_BEGIN_MESSAGE_MAP(CCancelDownloadDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


void CCancelDownloadDlg::InitWindow()
{
	m_pOkBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("Okbtn")));
	m_pCancelBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("Cancelbtn")));
	m_pTitleLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("Title")));

	if (m_TitleFlag == 1)
	{
		m_pTitleLabel->SetText(L"�Ƿ�ȡ����ǰ�����ļ����أ�");
	}
	else if (m_TitleFlag == 2)
	{
		m_pTitleLabel->SetText(L"�Ƿ�ɾ�����ر����ļ���");
	}
	else if (m_TitleFlag == 3)
	{
		m_pTitleLabel->SetText(L"�Ƿ�ȡ�����ز����ص�¼���棿");
	}

}


void CCancelDownloadDlg::Init(CPaintManagerUI *pOwnerPM, int index, int nTitleFlag)
{
	if (pOwnerPM == NULL)
		return;

	m_iIndex = index;
	m_TitleFlag = nTitleFlag;
	Create(pOwnerPM->GetPaintWindow(), _T("CancelDownloadWnd"), UI_WNDSTYLE_DIALOG, 0);
	CenterWindow();
	ShowModal();
}


void CCancelDownloadDlg::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}


void CCancelDownloadDlg::Notify(TNotifyUI& msg)
{
	CDuiFrameBase::Notify(msg);
}


LRESULT CCancelDownloadDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg) {

	case WM_CLOSE:
	case WM_DESTROY:
		DestroyWindow(m_hWnd);
		break;

	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	return CDuiFrameBase::HandleMessage(uMsg, wParam, lParam);
}


void CCancelDownloadDlg::OnClick(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pOkBtn)
		{
			if (m_iIndex != -1)
				::SendMessage(GetWindowOwner(m_hWnd), UWM_CANCEL_DOWNLOAD, 0, m_iIndex);

			::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
			return;
		}
		else if (msg.pSender == m_pCancelBtn)
		{
			if(m_iIndex == -1)
				::SendMessage(GetWindowOwner(m_hWnd), UWM_CANCEL_DOWNLOAD, 0, m_iIndex);  //�����ص�¼����

			::SendMessage(m_hWnd, WM_CLOSE, 0, 0);


			return;
		}
	}
}
