#include "stdafx.h"
#include "HttpClient.h"
#include "curl/curl.h"

#include "common.h"

#ifdef _DEBUG
#	pragma comment(lib, "curl/libcurl_lib/libcurl_d.lib")
#else
#	pragma comment(lib, "curl/libcurl_lib/libcurl.lib")
#endif // _DEBUG



//1����������������Ŀ⣺��Ŀ->����->������->����->����������� ws2_32.lib winmm.lib wldap32.lib  ��ӽ�ȥ
//2������Ԥ����ѡ���Ŀ->����->c/c++ ->Ԥ������->Ԥ���������� CURL_STATICLIB; ��ӽ�ȥ
//3��C/C++�����������п�MTd�����̵߳��ԣ�����Debug�汾������MT�����̣߳�����Release�汾��


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

	if (pThis->m_bStop)   //�˳�
		return -1;  

	//DEBUG_PRINT(L"dlnow = %lf, dltotal = %lf, nPos = %d", dlnow, dltotal, pThis->m_nProgress);

	return 0;
}

int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
	string strP = strPost;
	strP = replace_all_distinct(strP, "+", "%2b");  //����ת��

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
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8);//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
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

	m_nIndex = -1;  //�������

	fclose(m_pSaveFile);

	curl_easy_cleanup(curl);
	return res;
}
