// SocketAsynServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CSocketAsynServerApp:
// �йش����ʵ�֣������ SocketAsynServer.cpp
//

class CSocketAsynServerApp : public CWinApp
{
public:
	CSocketAsynServerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSocketAsynServerApp theApp;
