#include "stdafx.h"
#include "BackupList.h"
#include "CancelDownload.h"
#include <fstream>

#include "AES_Decrypt.h"
#include "json/json.h"


#ifdef _DEBUG
#   pragma comment(lib, "json/json_lib/json_libmtd.lib")
#else
#   pragma comment(lib, "json/json_lib/json_libmt.lib")
#endif

typedef struct _DOWNLOADFILE_PARAM
{
	CBackupListWnd*	pThis;
	int				index;
}DOWNLOADFILE_PARAM;

CBackupListWnd::CBackupListWnd(LPCTSTR pszXMLName)
	:CDuiFrameBase(pszXMLName)
{
	m_bUpdate = true;
	m_bUnBackLogin = false;
	m_hWndParent = NULL;
	m_nRunThreadCount = 0;
}

CBackupListWnd::~CBackupListWnd()
{
	Clear();
}

DUI_BEGIN_MESSAGE_MAP(CBackupListWnd, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


void CBackupListWnd::Clear()
{
	for (auto iter : m_BackupInfoList)
		delete iter;
	m_BackupInfoList.clear();

	for (auto iter : m_httpGetZipFileList)
		delete iter;
	m_httpGetZipFileList.clear();

}

void CBackupListWnd::InitWindow()
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_wndShadow.Create(m_hWnd);
	m_wndShadow.SetSize(4);
	m_wndShadow.SetPosition(0, 0);

	m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pMinBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
	m_pBackupList = static_cast<CListUI*>(m_PaintManager.FindControl(L"backuplist"));
	m_plistStatusLabel = static_cast<CLabelUI*>(m_PaintManager.FindControl(L"liststatus"));
	m_pRescanBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("rescan")));
	m_pBacklogBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("backlogin")));

	UINT threadID;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, GetBackupThread, this, 0, &threadID);
	CloseHandle(hThread);
}

void CBackupListWnd::Init(CPaintManagerUI *pOwnerPM)
{
	if (pOwnerPM == NULL)
		return;

	m_hWndParent = pOwnerPM->GetPaintWindow();

	Create(GetDesktopWindow(), _T("BackupListWnd"), UI_WNDSTYLE_FRAME, 0);  //父窗口指定为桌面，防止隐藏父窗口后任务了图标消失
	SetIcon(IDI_ICLOUD);
	CenterWindow();
	ShowModal();
}

//获取账号密码
void CBackupListWnd::SetNamePasswd(CDuiString strName, CDuiString strPasswd)
{
	m_strUserName = strName;
	m_strPasswd = strPasswd;
}


void CBackupListWnd::OnFinalMessage(HWND /*hWnd*/)
{
	delete this;
}


void CBackupListWnd::Notify(TNotifyUI& msg)
{
	CDuiFrameBase::Notify(msg);
}


LRESULT CBackupListWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg)
	{
	case UWM_BACKUPLIST_UPDATE:
		lRes = OnBackuplistUpdate(uMsg, wParam, lParam, bHandled);
		break;
	case UWM_BACKUP_DOWNLOAD_FINISH:
		lRes = OnBackuplistDFinish(uMsg, wParam, lParam, bHandled);
		break;
	case UWM_CANCEL_DOWNLOAD:
		lRes = OnCancelDownload(uMsg, wParam, lParam, bHandled);
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		DestroyWindow(m_hWnd);
		break;

	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	//if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CDuiFrameBase::HandleMessage(uMsg, wParam, lParam);

}


void CBackupListWnd::OnClick(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pCloseBtn)
		{
			if (m_nRunThreadCount != 0)  //是否还有下载线程在运行，关闭
			{
				for (auto iter : m_httpGetZipFileList)
				{
					if (iter->GetDownloadZipIndex() != -1)
						::SendMessage(m_hWnd, UWM_CANCEL_DOWNLOAD, 0, iter->GetDownloadZipIndex());
				}
			}

			PostQuitMessage(0);
			return;
		}
		else if (msg.pSender == m_pMinBtn)
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return;
		}
		else if (wcsncmp(msg.pSender->GetName(), L"download", 8) == 0)
		{
			OnBtnDownLoad(msg);  //下载

		}
		else if (wcsncmp(msg.pSender->GetName(), L"cancel_download", 15) == 0)
		{
			//是否取消下载
			int index = msg.pSender->GetTag();
			CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
			pCancelDlWnd->Init(&m_PaintManager, index, 1);
		}
		else if (wcsncmp(msg.pSender->GetName(), L"delete", 6) == 0)
		{
			//删除
			int index = msg.pSender->GetTag();
			CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
			pCancelDlWnd->Init(&m_PaintManager, index, 2);

		}
		else if (msg.pSender == m_pRescanBtn)
		{
			//重新扫描
			Clear();
			Sleep(500);

			UINT threadID;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, GetBackupThread, this, 0, &threadID);
			CloseHandle(hThread);
		}
		else if (msg.pSender == m_pBacklogBtn)
		{
			//返回登录界面，如果有正在下载的备份选项，跳出界面判断是否删除
			m_bUnBackLogin = false;
			if (m_nRunThreadCount != 0)
			{
				CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
				pCancelDlWnd->Init(&m_PaintManager, -1, 3);

				if (!m_bUnBackLogin)  //删除正在下载的文件
				{
					for (auto iter : m_httpGetZipFileList)
					{
						if (iter->GetDownloadZipIndex() != -1)
							::SendMessage(m_hWnd, UWM_CANCEL_DOWNLOAD, 0, iter->GetDownloadZipIndex());
						Sleep(100);
					}
				}
				else
					return;
			}
			::ShowWindow(m_hWndParent, SW_SHOW);
			::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		}
	}
}

//下载按钮
void CBackupListWnd::OnBtnDownLoad(TNotifyUI& msg)
{
	int index = msg.pSender->GetTag();
	int nCount = m_pBackupList->GetCount();
	if (index<0 || index >= nCount)
		return;

	CContainerUI* pLayout = static_cast<CContainerUI*>(m_pBackupList->GetItemAt(index));
	if (pLayout)
	{
		CControlUI* pControl = pLayout->GetItemAt(4);
		if (pControl)
			pControl->SetVisible(false);

		//进度条
		pControl = pLayout->GetItemAt(6);
		if (pControl)
		{
			pControl->SetVisible(true);

			DOWNLOADFILE_PARAM* pDownload_param;
			pDownload_param = new DOWNLOADFILE_PARAM();        
			ZeroMemory(pDownload_param, sizeof(DOWNLOADFILE_PARAM));
			pDownload_param->pThis = this;
			pDownload_param->index = index;


			UINT threadID1, threadID2;
			HANDLE hWorkThread = (HANDLE)_beginthreadex(NULL, 0, DownloadFileThread, pDownload_param, 0, &threadID1);
			HANDLE hWndThread = (HANDLE)_beginthreadex(NULL, 0, RefreshWndThread, pDownload_param, 0, &threadID2);

			CloseHandle(hWorkThread);
			CloseHandle(hWndThread);

			m_nRunThreadCount++;

			DEBUG_PRINT(L"m_nRunThreadCount_start = %d", m_nRunThreadCount);
		}

		pControl = pLayout->GetItemAt(7);
		if (pControl)
			pControl->SetVisible(true);
	}
}

//下载文件线程
UINT CBackupListWnd::DownloadFileThread(LPVOID pParam)
{
	DOWNLOADFILE_PARAM* pData = (DOWNLOADFILE_PARAM *)pParam;
	CBackupListWnd* pThis = pData->pThis;
	UINT index = pData->index;

	if (index < pThis->m_BackupInfoList.size())
	{
		string zipPath = pThis->m_BackupInfoList[index]->zipPath;

		pThis->DownloadZip(zipPath, index);
	}

	return 0;
}

//刷新进度条
UINT CBackupListWnd::RefreshWndThread(LPVOID pParam)
{
	DOWNLOADFILE_PARAM* pData = (DOWNLOADFILE_PARAM *)pParam;
	CBackupListWnd* pThis = pData->pThis;
	UINT index = pData->index;

	pThis->m_pRescanBtn->SetEnabled(false);

	if (index < pThis->m_BackupInfoList.size())
	{
		Sleep(1000);
		pThis->RefreshProgress(index);
	}

	//DEBUG_PRINT(L"m_nRunThreadCount_refresh = %d", pThis->m_nRunThreadCount);

	if (pThis->m_nRunThreadCount == 0)
		pThis->m_pRescanBtn->SetEnabled(true);

	return 0;
}



//定时刷新进度条
int CBackupListWnd::RefreshProgress(int index)
{
	HANDLE hTimeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CContainerUI* pLayout = static_cast<CContainerUI*>(m_pBackupList->GetItemAt(index));
	if (!pLayout)
		return -1;
	CProgressUI* pControl = (CProgressUI*)pLayout->GetItemAt(6);
	if (!pControl)
		return -1;

	for (auto iter : m_httpGetZipFileList)
	{
		if (index == iter->GetDownloadZipIndex())
		{
		QueryCyc:  

			if (iter->GetStopWriteFlag())  //下载被取消
			{
				pControl->SetValue(0);   //进度条置0
				CloseHandle(hTimeEvent);
				hTimeEvent = NULL;
				m_nRunThreadCount--;
				return 0;
			}

			pControl->SetValue(iter->GetProgress());

			WaitForSingleObject(hTimeEvent, 500);

			if (iter->GetProgress() == 100)
			{
				pControl->SetValue(100);
				pControl->SetVisible(false);

				CloseHandle(hTimeEvent);
				hTimeEvent = NULL;

				m_nRunThreadCount--;
				CControlUI* pControl2 = pLayout->GetItemAt(7);
				if (pControl2)
					pControl2->SetVisible(false);

				::PostMessage(GetHWND(), UWM_BACKUP_DOWNLOAD_FINISH, 0, index);

				return 0;
			}
			else
				goto QueryCyc;
		}
	}

	return 0;
}


//刷新列表界面
LRESULT CBackupListWnd::OnBackuplistUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int index = 0;
	m_pBackupList->RemoveAll();
	if (m_pBackupList)
	{
		CDialogBuilder builder;
		CListContainerElementUI* pItem = static_cast<CListContainerElementUI*>(builder.Create(L"list_item.xml", 0));
		if (pItem)
		{
			CDuiString strText;
			for (auto iter : m_BackupInfoList)
			{
				string device_name = iter->deviceName;
				string version = iter->version;
				string file_size = iter->size;
				string create_date= iter->createDate;

				if (pItem == NULL)
					pItem = static_cast<CListContainerElementUI*>(builder.Create());
				CControlUI* pControl = pItem->GetItemAt(0);
				if (pControl)
				{
					strText.Format(L"%S", device_name.c_str());
					pControl->SetText(strText);

					if (iter->isFileExist == 1)
					{
						pControl->SetBkImage(L"file='ico_down.png' dest='10,17,16,23'");
					}
					else
					{
						pControl->SetBkImage(L"file='ico_ndown.png' dest='10,17,16,23'");
					}
				}
				pControl = pItem->GetItemAt(1);
				if (pControl)
				{
					strText.Format(L"%S", version.c_str());
					pControl->SetText(strText);
				}
				pControl = pItem->GetItemAt(2);
				if (pControl)
				{
					strText.Format(L"%S", file_size.c_str());
					pControl->SetText(strText);
				}
				pControl = pItem->GetItemAt(3);
				if (pControl)
				{
					strText.Format(L"%S", create_date.c_str());
					pControl->SetText(strText);
				}

				pControl = pItem->GetItemAt(4);
				if (pControl)
				{
					if (iter->isFileExist == 1)
					{
						pControl->SetText(L"已下载");
						pControl->SetBkImage(L"file='btn_down_ok.png' dest='20,5,50,35'");
						pControl->SetEnabled(false);
					}
					else
					{
						pControl->SetBkImage(L"file='btn_down.png' dest='20,5,50,35'");
					}
					pControl->SetTag(index);

				}
				pControl = pItem->GetItemAt(5);
				if (pControl)
				{
					if (iter->isFileExist == 1)
					{
						pControl->SetBkImage(L"file='btn_delete.png' dest='20,5,50,35'");
					}
					else
					{
						pControl->SetBkImage(L"file='btn_delete_un.png' dest='20,5,50,35'");
						pControl->SetEnabled(false);
					}
					pControl->SetTag(index);
				}
				pControl = pItem->GetItemAt(6);
				if (pControl)
				{
					pControl->SetVisible(false);
				}
				pControl = pItem->GetItemAt(7);
				if (pControl)
				{
					pControl->SetVisible(false);
					pControl->SetTag(index);
				}

				m_pBackupList->Add(pItem);
				pItem = NULL;

				index++;
			}
		}
	}

	return 0;
}

//完成列表项的下载，更换图标
LRESULT CBackupListWnd::OnBackuplistDFinish(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int index = lParam;

	CContainerUI* pLayout = static_cast<CContainerUI*>(m_pBackupList->GetItemAt(index));
	if (!pLayout)
		return -1;

	CControlUI*  pControl = pLayout->GetItemAt(0);
	if (pControl)
		pControl->SetBkImage(L"file='ico_down.png' dest='10,17,16,23'");

	pControl = pLayout->GetItemAt(4);
	if (pControl)
	{
		pControl->SetBkImage(L"file='btn_down_ok.png' dest='20,5,50,35'");
		pControl->SetText(L"已下载");
		pControl->SetEnabled(false);
		pControl->SetVisible(true);
	}

	pControl = pLayout->GetItemAt(5);
	if (pControl)
	{
		pControl->SetBkImage(L"file='btn_delete.png' dest='20,5,50,35'");
		pControl->SetEnabled(true);
		pControl->SetVisible(true);
	}

	return 0;
}

//取消下载、删除备份
LRESULT CBackupListWnd::OnCancelDownload(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int list_index = lParam;
	if (list_index == -1)   //是否由返回登录界面按钮引起的消息
	{
		m_bUnBackLogin = true;
		return 0;
	}

	CContainerUI* pLayout = static_cast<CContainerUI*>(m_pBackupList->GetItemAt(list_index));
	if (!pLayout)
		return -1;

	CControlUI*  pControl = pLayout->GetItemAt(0);
	if (pControl)
		pControl->SetBkImage(L"file='ico_ndown.png' dest='10,17,16,23'");

	pControl = pLayout->GetItemAt(6);
	if (pControl)
		pControl->SetVisible(false);
	pControl = pLayout->GetItemAt(7);
	if (pControl)
		pControl->SetVisible(false);


	pControl = pLayout->GetItemAt(4);
	if (pControl)
	{
		pControl->SetBkImage(L"file='btn_down.png' dest='20,5,50,35'");
		pControl->SetText(L"下载");
		pControl->SetEnabled(true);
		pControl->SetVisible(true);
	}

	pControl = pLayout->GetItemAt(5);
	if (pControl)
	{
		pControl->SetBkImage(L"file='btn_delete_un.png' dest='20,5,50,35'");
		pControl->SetEnabled(false);
		pControl->SetVisible(true);
	}

	int index = 0;

	for (auto iter : m_BackupInfoList)
	{
		if (index == list_index)
		{
			string zipSavePath;
			int rlt = IsFileExist(iter->zipPath, zipSavePath);
			//if (rlt)
			{
				int nErr = DeleteFileA(zipSavePath.c_str());

				if (nErr == 0)
				{
					//取消正在下载的备份，删除不完成的文件
					for (auto iter_http : m_httpGetZipFileList)
					{
						if (iter_http->GetDownloadZipIndex() == index)
						{
							iter_http->SetStopWriteFile(true);
							Sleep(200);
							DeleteFileA(zipSavePath.c_str());
							break;
						}
					}
				}
			}
			break;
		}
		index++;
	}

	DEBUG_PRINT(L"m_nRunThreadCount_cancel = %d", m_nRunThreadCount);

	if (m_nRunThreadCount == 0)
		m_pRescanBtn->SetEnabled(true);

	return 0;
}

//获取列表、获取下载地址线程
UINT CBackupListWnd::GetBackupThread(LPVOID pParam)
{
	CBackupListWnd* pThis = (CBackupListWnd *)pParam;

	pThis->m_pRescanBtn->SetEnabled(false);

	pThis->m_plistStatusLabel->SetText(L"因网络原因，获取iCloud备份文件信息需要一些时间，请等待...");

	string backupList_json;
	pThis->ProcessBackupList(backupList_json);

	pThis->AnalyzeBackupList(backupList_json);

	pThis->GetBackupDownloadPath();

	::PostMessage(pThis->GetHWND(), UWM_BACKUPLIST_UPDATE, 0, 0);
	pThis->m_plistStatusLabel->SetText(L"iCloud备份文件");

	pThis->m_pRescanBtn->SetEnabled(true);

	_endthreadex(0);
	return 0;
}

//处理获取的列表的json文件
int CBackupListWnd::ProcessBackupList(string& data_json)
{
	m_bUpdate = 0;

	CDuiString strW;
	if (m_bUpdate)
		strW.Format(L"%s/%s/true", m_strUserName.GetData(), m_strPasswd.GetData());
	else
		strW.Format(L"%s/%s/false", m_strUserName.GetData(), m_strPasswd.GetData());

	wstring wstr = strW.GetData();
	string str = wstringTostring(wstr);
	string strPostData = EncryptionAES(str);	//AES加密

	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(LIST_URL, csPostData, strResponse);  //http POST 获取

	string backupList_json = DecryptionAES(strResponse);	//解密

	Json::Reader reader;
	Json::Value value;

	//解析Json数据
	if (reader.parse(backupList_json, value))
	{
		if (!value["msg"].isNull())  //正在获取备份列表中
		{
			string msg = value["msg"].asString();
			if (msg == "No devices")
			{
				return -1;
			}
			else if (msg == "loading backup list")  //正在获取
			{
				string query_data_json;
				while (1)
				{
					query_data_json = QueryStatus("getBackupList");   //查询列表状态
					if (reader.parse(query_data_json, value))
					{
						if (value["msg"].isNull())  //已成功获取
						{
							backupList_json = query_data_json;

							DEBUG_PRINT(L"获取列表成功！");

							break;
						}
						else
						{
							string state = value["state"].asString();
							if (state == "err")
							{
								return -2;
							}
						}
					}
				}
			}
		}
	}

	data_json = backupList_json;

	return 0;
}

//分析列表，保存至链表
int CBackupListWnd::AnalyzeBackupList(string data_json)
{
	Json::Reader reader;
	Json::Value value;

	if (reader.parse(data_json, value))   //解析json数据
	{
		int uid = value["uid"].asInt();

		Json::Value json_data = value["data"];

		int index = 0;
		for (UINT i = 0; i < json_data.size(); i++)   //遍历设备
		{
			string deviceInfo = json_data[i]["deviceInfo"].asString();  //设备信息
			Json::Value json_dev = json_data[i][deviceInfo];   //设备json

			int pos = deviceInfo.find("iPhone");
			string device_name = deviceInfo.substr(pos, 7);

			for (UINT j = 0; j < json_dev.size(); j++)   //遍历备份文件
			{
				string device_id = json_dev[j]["deviceId"].asString();
				int index = json_dev[j]["index"].asInt();   //备份文件index

				string version = json_dev[j]["version"].asString();
				char version_full[64] = { 0 };
				sprintf_s(version_full, sizeof(version_full), "IOS%s", version.c_str());

				DWORD createTime = json_dev[j]["createTime"].asDouble();
				string createDate = timetodate(createTime);
				string file_size = json_dev[j]["size"].asString();   //备份文件大小

				PBACKUP_INFO pBackupInfo;
				pBackupInfo = new BACKUP_INFO();
				ZeroMemory(pBackupInfo, sizeof(BACKUP_INFO));
				pBackupInfo->uId = uid;
				pBackupInfo->index = index;
				pBackupInfo->deviceName = device_name;
				pBackupInfo->deviceId = device_id;
				pBackupInfo->createDate = createDate;
				pBackupInfo->size = file_size;
				pBackupInfo->version = version_full;

				m_BackupInfoList.push_back(pBackupInfo);

				index++;
			}
		}
	}

	return 0;
}

//获取下载地址
int CBackupListWnd::GetBackupDownloadPath()
{
	string zipSavePath;

	for (auto iter : m_BackupInfoList)
	{
		int uid = iter->uId;
		int index = iter->index;
		string deviceId = iter->deviceId;

		string taskId = DownloadFilePath(uid, index, deviceId);   //下载文件

		string zipPath;
		string zipSize;

		while (1)
		{
			int progress = GetProgress(taskId, zipPath, zipSize);

			DEBUG_PRINT(L"taskId = %S, progress = %d , zipPath = %S", taskId.c_str(), progress, zipPath.c_str());

			if (progress >= 0 && progress < 100)
			{
				Sleep(1000);
				continue;
			}
			else if (progress == 100)
			{
				iter->zipPath = zipPath;
				iter->zipSize = zipSize;
				break;
			}
			else
			{
				//提示错误
				break;
			}
		}

		iter->isFileExist = IsFileExist(zipPath, zipSavePath);
	}

	int pos = zipSavePath.rfind('\\');
	string folder = zipSavePath.substr(0, pos + 1);   //获取文件夹
	string write_path = folder + "iCloudBackupInfo.json";   //文件存储路径
	string download_json = QueryStatus("downloadFile");   //查询下载备份

	//FILE* pFile = fopen(write_path.c_str(), "wb+");	
	FILE* pFile = NULL;
	fopen_s(&pFile, write_path.c_str(), "wb+");
	fwrite(download_json.c_str(), sizeof(char), download_json.size(), pFile);
	fclose(pFile);

	return 0;
}


// 查询状态
// 参数：查询类型；例如查询获取列表状态为：getBackupList、 下载状态：downloadFile
string CBackupListWnd::QueryStatus(string status)
{
	Sleep(1000);

	CDuiString strW;
	strW.Format(L"%s/%s/%S", m_strUserName.GetData(), m_strPasswd.GetData(), status.c_str());
	wstring wstr = strW.GetData();				//转换
	string str = wstringTostring(wstr);
	string strPostData = EncryptionAES(str);	//AES加密
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(QUERY_URL, csPostData, strResponse);   //http POST  查询
	string dec_data = DecryptionAES(strResponse);   //AES解密

	return dec_data;
}

//判断输入的账号密码是否正确
int CBackupListWnd::LoginSuccessOrNot()
{
	CDuiString strW;
	strW.Format(L"%s/%s/%S", m_strUserName.GetData(), m_strPasswd.GetData(), "getBackupList");
	wstring wstr = strW.GetData();				//转换
	string str = wstringTostring(wstr);
	string strPostData = EncryptionAES(str);	//AES加密
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(QUERY_URL, csPostData, strResponse);   //http POST  查询
	string dec_data = DecryptionAES(strResponse);   //AES解密

	Json::Reader reader;
	Json::Value value;

	if (reader.parse(dec_data, value))
	{
		string state = value["state"].asString();
		if (state == "err")
			return -1;
	}
	else
		return -2;

	return 0;
}


// 下载备份文件路径
string CBackupListWnd::DownloadFilePath(int uid, int index, string device_id)
{
	char str[260] = { 0 };
	sprintf_s(str, sizeof(str), "%d/%d/%s", uid, index, device_id.c_str());

	string strPostData = EncryptionAES(string(str));	//AES加密
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(DOWNLOAD_FILE_URL, csPostData, strResponse);   //http POST  查询
	string dec_data = DecryptionAES(strResponse);   //AES解密

	Json::Reader reader;
	Json::Value value;
	//解析Json数据
	if (reader.parse(dec_data, value))
	{
		if (!value["progress"].isNull())
		{
			int progress = value["progress"].asInt();
			string state = value["state"].asString();
			string task_id = value["taskId"].asString();

			return task_id;
		}
	}

	return NULL;
}

//  获取进度
int CBackupListWnd::GetProgress(string task_id, string& zip_file, string& zip_size)
{
	string strPostData = EncryptionAES(task_id);	//AES加密
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(PROGRESS_URL, csPostData, strResponse);   //http POST  查询
	string dec_data = DecryptionAES(strResponse);   //AES解密

	Json::Reader reader;
	Json::Value value;
	//解析Json数据
	if (reader.parse(dec_data, value))
	{
		if (!value["state"].isNull())
		{
			string progress_state = value["state"].asString();
			if (progress_state == "complete")
			{
				zip_file = value["zip_path"].asString();
				zip_size = value["zip_size"].asString();
				return 100;
			}
			else if (progress_state == "loading")
			{
				int progress = value["progress"].asInt();
				return progress;
			}
			else if (progress_state == "err")
			{
				return -1;
			}
		}
	}

	return -1;
}

//判断文件是否存在
int CBackupListWnd::IsFileExist(string zipPath, string& zipSavePath)
{
	if (zipPath.size() <= 0)
		return -1;

	if (zipPath.rfind(".zip") == -1)
		return -1;

	string writePath;
	writePath = GetModuleDir() + "\\backup";		//获取当前目录

	bool rlt = CheckFolderExist(writePath);
	if (!rlt)
		CreateDirectoryA(writePath.c_str(), NULL);

	int pos = zipPath.rfind('/');
	string file_name = zipPath.substr(pos + 1);   //获取文件名
	writePath = writePath + "\\" + file_name;   //文件存储路径

	zipSavePath = writePath;

	ifstream infile(writePath.c_str());    //判断是否已经存在
	if (infile)
	{
		infile.close();
		return 1;
	}
	else
	{
		infile.close();
		return 0;
	}


}

//下载压缩包
int  CBackupListWnd::DownloadZip(string zip_path, int index)
{
	if (zip_path.size() <= 0)
		return -1;

	if (zip_path.rfind(".zip") == -1)
		return -1;

	string write_path;
	write_path = GetModuleDir() + "\\backup";		//获取当前目录

	int pos = zip_path.rfind('/');
	string file_name = zip_path.substr(pos + 1);   //获取文件名
	write_path = write_path + "\\" + file_name;   //文件存储路径

	CHttpClient* httpGet = new CHttpClient();
	httpGet->SetDownloadZipIndex(index);
	m_httpGetZipFileList.push_back(httpGet);

	httpGet->Get(zip_path, write_path);

	return 0;
}
