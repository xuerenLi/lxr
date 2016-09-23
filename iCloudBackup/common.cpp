#include "stdafx.h"
#include "common.h"
#include <time.h>

//string字符串中全部的子字符串替换成指定子字符串
string& replace_all_distinct(string& str, const string& old_value, const string& new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length())
	{
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}


//Converting a WChar string to a Ansi string
std::string WChar2Ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);
	delete[] pszDst;

	return strTemp;
}

string wstringTostring(wstring& inputws)
{
	return WChar2Ansi(inputws.c_str());
}


//获取应用程序所在路径
string GetModuleDir()
{
	HMODULE module = GetModuleHandle(0);
	char pFileName[MAX_PATH];
	GetModuleFileNameA(module, pFileName, MAX_PATH);

	string csFullPath(pFileName);
	int nPos = csFullPath.rfind('\\');
	if (nPos < 0)
		return string("");
	else
		return csFullPath.substr(0, nPos);
}

//获取工作路径  
//CString GetWorkDir()
//{
//	WCHAR pFileName[MAX_PATH];
//	int nPos = GetCurrentDirectory(MAX_PATH, pFileName);
//
//	CString csFullPath(pFileName);
//	if (nPos < 0)
//		return CString(L"");
//	else
//		return csFullPath;
//}
//
//


string   timetodate(time_t const timer)
{
	//struct tm *l = localtime(&timer);
	struct tm l = { 0 };
	localtime_s(&l, &timer);

	char buf[128];
	sprintf_s(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", l.tm_year + 1870, l.tm_mon + 1, l.tm_mday, l.tm_hour, l.tm_min, l.tm_sec);
	string s(buf);
	return s;
}


//目录是否存在的检查：
bool  CheckFolderExist(const string &strPath)
{
	WIN32_FIND_DATAA  wfd;
	bool rValue = false;
	HANDLE hFind = FindFirstFileA(strPath.c_str(), &wfd);
	if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = true;
	}
	FindClose(hFind);
	return rValue;
}



void DEBUG_PRINT(TCHAR *format, ...)
{
	va_list   argList;
	TCHAR   szFormat[256], szContent[1024];    //maximum buffer is 1K bytes

	_stprintf_s(szFormat, _countof(szFormat), TEXT(" %s"), format);
	va_start(argList, format);
	_vsnwprintf_s(szContent, 1024, szFormat, argList);
	va_end(argList);
	lstrcat(szContent, L"\n");
	OutputDebugString(szContent);
}

