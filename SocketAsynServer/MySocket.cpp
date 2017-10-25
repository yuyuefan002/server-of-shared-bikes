// MySocket.cpp : implementation file
//

#include "stdafx.h"
#include "MySocket.h"
#include "SocketAsynServer.h"
#include "SocketAsynServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySocket

CMySocket::CMySocket()
{
	m_pDlg=NULL;
}

CMySocket::~CMySocket()
{
	m_pDlg=NULL;
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CMySocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CMySocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CMySocket member functions

void CMySocket::OnAccept(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (nErrorCode==0)
	{
	m_pDlg->OnAccept();	
	}
	//CAsyncSocket::OnAccept(nErrorCode);
}

void CMySocket::OnReceive(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (nErrorCode==0)
	{
		m_pDlg->OnReceive();
	}
 // CAsyncSocket::OnReceive(nErrorCode);
}

void CMySocket::OnClose(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (nErrorCode==0)
	{
		m_pDlg->OnClose();

	}
	//CAsyncSocket::OnClose(nErrorCode);
}

void CMySocket::SetParent(CSocketAsynServerDlg *plg)
{
	m_pDlg=plg;
}
