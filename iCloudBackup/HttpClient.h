#pragma once


#include <string>

class CHttpClient
{
public:
	CHttpClient();
	~CHttpClient(void);

public:
	
	int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);

	int Get(const std::string & strUrl, std::string & strSavePath);


private:
	static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

public:
	DWORD GetFileCount() { return m_dwFileCount; }  //下载文件大小计数
	DWORD GetProgress() { return m_nProgress; }  //下载文件大小计数

	void  SetDownloadZipIndex(int index) { m_nIndex = index; }  //下载索引
	int	  GetDownloadZipIndex() { return m_nIndex; }

	void  SetStopWriteFile(BOOL bStop) { m_bStop = bStop; }     //终止下载
	BOOL  GetStopWriteFlag() { return m_bStop; }



private:
	int		m_nProgress;
	DWORD	m_dwFileCount;
	int		m_nIndex;
	BOOL	m_bStop;
	FILE*	m_pSaveFile;



};