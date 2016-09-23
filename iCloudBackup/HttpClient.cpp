#include "stdafx.h"
#include "HttpClient.h"
#include "curl/curl.h"

#include "common.h"

#ifdef _DEBUG
#	pragma comment(lib, "curl/libcurl_lib/libcurl_d.lib")
#else
#	pragma comment(lib, "curl/libcurl_lib/libcurl.lib")
#endif // _DEBUG



//1、给工程添加依赖的库：项目->属性->链接器->输入->附加依赖项，把 ws2_32.lib winmm.lib wldap32.lib  添加进去
//2、加入预编译选项：项目->属性->c/c++ ->预处理器->预处理器，把 CURL_STATICLIB; 添加进去
//3、C/C++代码生成运行库MTd（多线程调试，用于Debug版本）或者MT（多线程，用于Release版本）


CHttpClient::CHttpClient()
{
	m_dwFileCount = 0;
	m_nIndex = -1;
	m_bStop = FALSE;
	m_pSaveFile = NULL;

	m_nProgress = 0;
}


CHttpClient::~CHttpClient(void)
{
	if (NULL != m_pSaveFile)
	{
		fclose(m_pSaveFile);
		m_pSaveFile = NULL;
	}
}


static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

static size_t OnWriteFile(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	FILE* fp = (FILE*)lpVoid;
	size_t nWrite = fwrite(buffer, size, nmemb, fp);

	return nWrite;
}

int CHttpClient::ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CHttpClient* pThis = (CHttpClient*)clientp;

	if (dltotal > -0.1 && dltotal < 0.1)
	{
		return 0;
	}

	pThis->m_nProgress = (int)((dlnow / dltotal) * 100);

	if (pThis->m_bStop)   //退出
		return -1;  

	//DEBUG_PRINT(L"dlnow = %lf, dltotal = %lf, nPos = %d", dlnow, dltotal, pThis->m_nProgress);

	return 0;
}

int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
	string strP = strPost;
	strP = replace_all_distinct(strP, "+", "%2b");  //数据转换

	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strP.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int CHttpClient::Get(const std::string & strUrl, std::string & strSavePath)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}

	fopen_s(&m_pSaveFile, strSavePath.c_str(), "wb+");

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteFile);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, m_pSaveFile);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	res = curl_easy_perform(curl);

	m_nIndex = -1;  //下载完成

	fclose(m_pSaveFile);

	curl_easy_cleanup(curl);
	return res;
}
