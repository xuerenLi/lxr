#pragma once

#include "DuiFrameBase.h"
#include "common.h"
#include "HttpClient.h"


class CBackupListWnd : public CDuiFrameBase
{
public:
	CBackupListWnd(LPCTSTR pszXMLName);

	virtual ~CBackupListWnd();

	DUI_DECLARE_MESSAGE_MAP()

public:
	void Init(CPaintManagerUI *pOwnerPM);
	virtual void    OnFinalMessage(HWND hWnd);
	virtual void	InitWindow();
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void    Notify(TNotifyUI& msg);

	virtual void OnClick(TNotifyUI& msg);
	LRESULT OnBackuplistUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBackuplistDFinish(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCancelDownload(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);



public:
	void SetNamePasswd(CDuiString strName, CDuiString strPasswd);
	static UINT WINAPI GetBackupThread(LPVOID pParam);
	static UINT WINAPI DownloadFileThread(LPVOID pParam);
	static UINT WINAPI RefreshWndThread(LPVOID pParam);
	int ProcessBackupList(string& data_json);
	int AnalyzeBackupList(string data_json);
	string QueryStatus(string status);
	int	LoginSuccessOrNot();
	string DownloadFilePath(int uid, int index, string device_id);
	int GetProgress(string task_id, string& zip_file, string& zip_size);
	int DownloadZip(string zip_path, int index);

	int GetBackupDownloadPath();
	int IsFileExist(string zipPath, string& zipSavePath);

	void OnBtnDownLoad(TNotifyUI& msg);
	void Clear();
	int RefreshProgress(int index);

private:
	CButtonUI*	m_pCloseBtn;
	CButtonUI*	m_pMinBtn;
	CListUI*	m_pBackupList;
	CLabelUI*	m_plistStatusLabel;
	CButtonUI*	m_pRescanBtn;
	CButtonUI*	m_pBacklogBtn;

public:
	bool		m_bUpdate;			//是否强制更新
	bool		m_bUnBackLogin;		//是否取消返回到登录界面
	CDuiString	m_strUserName;		
	CDuiString	m_strPasswd;
	CHttpClient	m_httpClient;
	HWND		m_hWndParent;
	int			m_nRunThreadCount;   //线程计数
	vector<BACKUP_INFO *> m_BackupInfoList;		//备份文件列表信息
	vector<CHttpClient *> m_httpGetZipFileList;	//下载zip文件对象数组

};
