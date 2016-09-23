#pragma once


#include <string> 
using namespace std;

#define UWM_CREATE_BACKUPLIST			(WM_USER + 0x100)
#define UWM_BACKUPLIST_UPDATE			(WM_USER + 0x101)
#define	UWM_BACKUP_DOWNLOAD_FINISH		(WM_USER + 0x102)
#define UWM_CANCEL_DOWNLOAD				(WM_USER + 0x103)

//����ʱʹ�õĲ���
typedef struct _BACKUP_INFO
{
	string  createDate;
	string	deviceId;
	string  size;
	string  version;
	string  deviceName;
	int		uId;
	int		index;
	string	zipPath;
	string	zipSize;
	int		isFileExist;

}BACKUP_INFO, *PBACKUP_INFO;



// ����ʹ��
// �û���
//#define USERNAME	L"184465046@qq.com"
// ����
//#define PASSWORD	L"Taotao91811"

// �û���
#define USERNAME	L"543830444@qq.com"
// ����
#define PASSWORD	L"19890608Zhoubo"

// ��ȡ�����б�
#define LIST_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/getBackupList"

// �����ļ�
#define DOWNLOAD_FILE_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/downloadFile"

// ��ȡ����
#define PROGRESS_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/getProgress"

// ��ѯ״̬
#define QUERY_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/query"





string& replace_all_distinct(string& str, const string& old_value, const string& new_value);


string wstringTostring(wstring& inputws);


string GetModuleDir();

//CString GetWorkDir();

void DEBUG_PRINT(TCHAR *format, ...);

string   timetodate(time_t const timer);

bool  CheckFolderExist(const string &strPath);