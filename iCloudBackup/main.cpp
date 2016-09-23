#include "stdafx.h"
#include "ICloudLogin.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));   // 设置资源的默认路径（此处设置为和exe在同一目录）

	CWndShadow::Initialize(hInstance);

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;

	CLoginWnd* pFrame = new CLoginWnd(_T("login.xml"));
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, _T("iCloudLogin"), UI_WNDSTYLE_FRAME, 0);

	pFrame->SetIcon(IDI_ICLOUD);

	pFrame->CenterWindow();
	pFrame->ShowModal();

	delete pFrame;

	::CoUninitialize();
	return 0;
}