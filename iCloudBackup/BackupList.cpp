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

	Create(GetDesktopWindow(), _T("BackupListWnd"), UI_WNDSTYLE_FRAME, 0);  //������ָ��Ϊ���棬��ֹ���ظ����ں�������ͼ����ʧ
	SetIcon(IDI_ICLOUD);
	CenterWindow();
	ShowModal();
}

//��ȡ�˺�����
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
			if (m_nRunThreadCount != 0)  //�Ƿ��������߳������У��ر�
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
			OnBtnDownLoad(msg);  //����

		}
		else if (wcsncmp(msg.pSender->GetName(), L"cancel_download", 15) == 0)
		{
			//�Ƿ�ȡ������
			int index = msg.pSender->GetTag();
			CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
			pCancelDlWnd->Init(&m_PaintManager, index, 1);
		}
		else if (wcsncmp(msg.pSender->GetName(), L"delete", 6) == 0)
		{
			//ɾ��
			int index = msg.pSender->GetTag();
			CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
			pCancelDlWnd->Init(&m_PaintManager, index, 2);

		}
		else if (msg.pSender == m_pRescanBtn)
		{
			//����ɨ��
			Clear();
			Sleep(500);

			UINT threadID;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, GetBackupThread, this, 0, &threadID);
			CloseHandle(hThread);
		}
		else if (msg.pSender == m_pBacklogBtn)
		{
			//���ص�¼���棬������������صı���ѡ����������ж��Ƿ�ɾ��
			m_bUnBackLogin = false;
			if (m_nRunThreadCount != 0)
			{
				CCancelDownloadDlg* pCancelDlWnd = new CCancelDownloadDlg(_T("canceldownload.xml"));
				pCancelDlWnd->Init(&m_PaintManager, -1, 3);

				if (!m_bUnBackLogin)  //ɾ���������ص��ļ�
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

//���ذ�ť
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

		//������
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

//�����ļ��߳�
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

//ˢ�½�����
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



//��ʱˢ�½�����
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

			if (iter->GetStopWriteFlag())  //���ر�ȡ��
			{
				pControl->SetValue(0);   //��������0
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


//ˢ���б����
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
						pControl->SetText(L"������");
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

//����б�������أ�����ͼ��
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
		pControl->SetText(L"������");
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

//ȡ�����ء�ɾ������
LRESULT CBackupListWnd::OnCancelDownload(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int list_index = lParam;
	if (list_index == -1)   //�Ƿ��ɷ��ص�¼���水ť�������Ϣ
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
		pControl->SetText(L"����");
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
					//ȡ���������صı��ݣ�ɾ������ɵ��ļ�
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

//��ȡ�б���ȡ���ص�ַ�߳�
UINT CBackupListWnd::GetBackupThread(LPVOID pParam)
{
	CBackupListWnd* pThis = (CBackupListWnd *)pParam;

	pThis->m_pRescanBtn->SetEnabled(false);

	pThis->m_plistStatusLabel->SetText(L"������ԭ�򣬻�ȡiCloud�����ļ���Ϣ��ҪһЩʱ�䣬��ȴ�...");

	string backupList_json;
	pThis->ProcessBackupList(backupList_json);

	pThis->AnalyzeBackupList(backupList_json);

	pThis->GetBackupDownloadPath();

	::PostMessage(pThis->GetHWND(), UWM_BACKUPLIST_UPDATE, 0, 0);
	pThis->m_plistStatusLabel->SetText(L"iCloud�����ļ�");

	pThis->m_pRescanBtn->SetEnabled(true);

	_endthreadex(0);
	return 0;
}

//�����ȡ���б��json�ļ�
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
	string strPostData = EncryptionAES(str);	//AES����

	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(LIST_URL, csPostData, strResponse);  //http POST ��ȡ

	string backupList_json = DecryptionAES(strResponse);	//����

	Json::Reader reader;
	Json::Value value;

	//����Json����
	if (reader.parse(backupList_json, value))
	{
		if (!value["msg"].isNull())  //���ڻ�ȡ�����б���
		{
			string msg = value["msg"].asString();
			if (msg == "No devices")
			{
				return -1;
			}
			else if (msg == "loading backup list")  //���ڻ�ȡ
			{
				string query_data_json;
				while (1)
				{
					query_data_json = QueryStatus("getBackupList");   //��ѯ�б�״̬
					if (reader.parse(query_data_json, value))
					{
						if (value["msg"].isNull())  //�ѳɹ���ȡ
						{
							backupList_json = query_data_json;

							DEBUG_PRINT(L"��ȡ�б�ɹ���");

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

//�����б�����������
int CBackupListWnd::AnalyzeBackupList(string data_json)
{
	Json::Reader reader;
	Json::Value value;

	if (reader.parse(data_json, value))   //����json����
	{
		int uid = value["uid"].asInt();

		Json::Value json_data = value["data"];

		int index = 0;
		for (UINT i = 0; i < json_data.size(); i++)   //�����豸
		{
			string deviceInfo = json_data[i]["deviceInfo"].asString();  //�豸��Ϣ
			Json::Value json_dev = json_data[i][deviceInfo];   //�豸json

			int pos = deviceInfo.find("iPhone");
			string device_name = deviceInfo.substr(pos, 7);

			for (UINT j = 0; j < json_dev.size(); j++)   //���������ļ�
			{
				string device_id = json_dev[j]["deviceId"].asString();
				int index = json_dev[j]["index"].asInt();   //�����ļ�index

				string version = json_dev[j]["version"].asString();
				char version_full[64] = { 0 };
				sprintf_s(version_full, sizeof(version_full), "IOS%s", version.c_str());

				DWORD createTime = json_dev[j]["createTime"].asDouble();
				string createDate = timetodate(createTime);
				string file_size = json_dev[j]["size"].asString();   //�����ļ���С

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

//��ȡ���ص�ַ
int CBackupListWnd::GetBackupDownloadPath()
{
	string zipSavePath;

	for (auto iter : m_BackupInfoList)
	{
		int uid = iter->uId;
		int index = iter->index;
		string deviceId = iter->deviceId;

		string taskId = DownloadFilePath(uid, index, deviceId);   //�����ļ�

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
				//��ʾ����
				break;
			}
		}

		iter->isFileExist = IsFileExist(zipPath, zipSavePath);
	}

	int pos = zipSavePath.rfind('\\');
	string folder = zipSavePath.substr(0, pos + 1);   //��ȡ�ļ���
	string write_path = folder + "iCloudBackupInfo.json";   //�ļ��洢·��
	string download_json = QueryStatus("downloadFile");   //��ѯ���ر���

	//FILE* pFile = fopen(write_path.c_str(), "wb+");	
	FILE* pFile = NULL;
	fopen_s(&pFile, write_path.c_str(), "wb+");
	fwrite(download_json.c_str(), sizeof(char), download_json.size(), pFile);
	fclose(pFile);

	return 0;
}


// ��ѯ״̬
// ��������ѯ���ͣ������ѯ��ȡ�б�״̬Ϊ��getBackupList�� ����״̬��downloadFile
string CBackupListWnd::QueryStatus(string status)
{
	Sleep(1000);

	CDuiString strW;
	strW.Format(L"%s/%s/%S", m_strUserName.GetData(), m_strPasswd.GetData(), status.c_str());
	wstring wstr = strW.GetData();				//ת��
	string str = wstringTostring(wstr);
	string strPostData = EncryptionAES(str);	//AES����
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(QUERY_URL, csPostData, strResponse);   //http POST  ��ѯ
	string dec_data = DecryptionAES(strResponse);   //AES����

	return dec_data;
}

//�ж�������˺������Ƿ���ȷ
int CBackupListWnd::LoginSuccessOrNot()
{
	CDuiString strW;
	strW.Format(L"%s/%s/%S", m_strUserName.GetData(), m_strPasswd.GetData(), "getBackupList");
	wstring wstr = strW.GetData();				//ת��
	string str = wstringTostring(wstr);
	string strPostData = EncryptionAES(str);	//AES����
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(QUERY_URL, csPostData, strResponse);   //http POST  ��ѯ
	string dec_data = DecryptionAES(strResponse);   //AES����

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


// ���ر����ļ�·��
string CBackupListWnd::DownloadFilePath(int uid, int index, string device_id)
{
	char str[260] = { 0 };
	sprintf_s(str, sizeof(str), "%d/%d/%s", uid, index, device_id.c_str());

	string strPostData = EncryptionAES(string(str));	//AES����
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(DOWNLOAD_FILE_URL, csPostData, strResponse);   //http POST  ��ѯ
	string dec_data = DecryptionAES(strResponse);   //AES����

	Json::Reader reader;
	Json::Value value;
	//����Json����
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

//  ��ȡ����
int CBackupListWnd::GetProgress(string task_id, string& zip_file, string& zip_size)
{
	string strPostData = EncryptionAES(task_id);	//AES����
	char csPostData[260] = { 0 };
	sprintf_s(csPostData, sizeof(csPostData), "parameter=%s", strPostData.c_str());

	string strResponse;
	m_httpClient.Post(PROGRESS_URL, csPostData, strResponse);   //http POST  ��ѯ
	string dec_data = DecryptionAES(strResponse);   //AES����

	Json::Reader reader;
	Json::Value value;
	//����Json����
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

//�ж��ļ��Ƿ����
int CBackupListWnd::IsFileExist(string zipPath, string& zipSavePath)
{
	if (zipPath.size() <= 0)
		return -1;

	if (zipPath.rfind(".zip") == -1)
		return -1;

	string writePath;
	writePath = GetModuleDir() + "\\backup";		//��ȡ��ǰĿ¼

	bool rlt = CheckFolderExist(writePath);
	if (!rlt)
		CreateDirectoryA(writePath.c_str(), NULL);

	int pos = zipPath.rfind('/');
	string file_name = zipPath.substr(pos + 1);   //��ȡ�ļ���
	writePath = writePath + "\\" + file_name;   //�ļ��洢·��

	zipSavePath = writePath;

	ifstream infile(writePath.c_str());    //�ж��Ƿ��Ѿ�����
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

//����ѹ����
int  CBackupListWnd::DownloadZip(string zip_path, int index)
{
	if (zip_path.size() <= 0)
		return -1;

	if (zip_path.rfind(".zip") == -1)
		return -1;

	string write_path;
	write_path = GetModuleDir() + "\\backup";		//��ȡ��ǰĿ¼

	int pos = zip_path.rfind('/');
	string file_name = zip_path.substr(pos + 1);   //��ȡ�ļ���
	write_path = write_path + "\\" + file_name;   //�ļ��洢·��

	CHttpClient* httpGet = new CHttpClient();
	httpGet->SetDownloadZipIndex(index);
	m_httpGetZipFileList.push_back(httpGet);

	httpGet->Get(zip_path, write_path);

	return 0;
}
