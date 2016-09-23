#pragma once

#include "..\DuiLib\UIlib.h"
using namespace DuiLib;

#ifdef _DEBUG
#   pragma comment(lib, "..\\DuiLib\\Lib\\DuiLib_d.lib")
#else
#   pragma comment(lib, "..\\DuiLib\\Lib\\DuiLib.lib")
#endif

class CDuiFrameBase : public WindowImplBase
{
public:
	CDuiFrameBase(LPCTSTR pszXMLName)
		: m_strXMLName(pszXMLName) {};
	~CDuiFrameBase() {};

public:
	virtual LPCTSTR GetWindowClassName() const
	{
		return _T("XMLWnd");
	}

	virtual CDuiString GetSkinFile()
	{
		return m_strXMLName;
	}

	virtual CDuiString GetSkinFolder()
	{
		return _T("");
	}

protected:
	CDuiString m_strXMLName;    // XMLµÄÃû×Ö
	CWndShadow	m_wndShadow;
};

