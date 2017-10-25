#if !defined(AFX_MYSOCKET_H__23E4A6FE_33D1_4418_9C0D_395C21281216__INCLUDED_)
#define AFX_MYSOCKET_H__23E4A6FE_33D1_4418_9C0D_395C21281216__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MySocket.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CMySocket command target
class CSocketAsynServerDlg;
class CMySocket : public CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CMySocket();
	virtual ~CMySocket();
	CString ClientIP;//连接客户端ip
	UINT Port;//端口号

// Overrides
public:
	void SetParent(CSocketAsynServerDlg *plg);
	CSocketAsynServerDlg *m_pDlg;
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySocket)
	public:
	virtual void OnAccept(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CMySocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSOCKET_H__23E4A6FE_33D1_4418_9C0D_395C21281216__INCLUDED_)

