#pragma once


#include <string> 
using namespace std;

#define UWM_CREATE_BACKUPLIST			(WM_USER + 0x100)
#define UWM_BACKUPLIST_UPDATE			(WM_USER + 0x101)
#define	UWM_BACKUP_DOWNLOAD_FINISH		(WM_USER + 0x102)
#define UWM_CANCEL_DOWNLOAD				(WM_USER + 0x103)

//下载时使用的参数
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



// 测试使用
// 用户名
//#define USERNAME	L"184465046@qq.com"
// 密码
//#define PASSWORD	L"Taotao91811"

// 用户名
#define USERNAME	L"543830444@qq.com"
// 密码
#define PASSWORD	L"19890608Zhoubo"

// 获取备份列表
#define LIST_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/getBackupList"

// 下载文件
#define DOWNLOAD_FILE_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/downloadFile"

// 获取进度
#define PROGRESS_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/getProgress"

// 查询状态
#define QUERY_URL	"http://icloud.api.huifudashi.com/1.0/iCloudBackup/query"





string& replace_all_distinct(string& str, const string& old_value, const string& new_value);


string wstringTostring(wstring& inputws);


string GetModuleDir();

//CString GetWorkDir();

void DEBUG_PRINT(TCHAR *format, ...);

string   timetodate(time_t const timer);

bool  CheckFolderExist(const string &strPath);