// SocketAsynServerDlg.h : 头文件
//

#pragma once
#include "MySocket.h"
#include "afxwin.h"

// CSocketAsynServerDlg 对话框
class CSocketAsynServerDlg : public CDialog
{
// 构造
public:
	CSocketAsynServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SOCKETASYNSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	//CMySocket ListenSocket;
	int ListenPort;
	CPtrList m_connectList;
	void OnClose();
	void OnReceive();
	CMySocket m_connectSocket;
	void OnAccept();
	CMySocket m_sListenSocket;
	// 用于监听的端口
	int m_port;
	// 通信状态信息打印
	CListBox m_sListRecv;
	afx_msg void OnBnClickedButtonSend();
	CString m_sMsg;
	afx_msg void OnLbnSelchangeListRecv();
	afx_msg void OnEnChangeEditSend();
	afx_msg void OnEnChangeEditPort();
	afx_msg void OnBnClickedButton2();
};
class IDVECTOR
{public:
	char id[20];
	IDVECTOR()
	{
		strcpy(id, "");
	}
};
