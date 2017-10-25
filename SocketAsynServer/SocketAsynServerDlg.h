// SocketAsynServerDlg.h : ͷ�ļ�
//

#pragma once
#include "MySocket.h"
#include "afxwin.h"

// CSocketAsynServerDlg �Ի���
class CSocketAsynServerDlg : public CDialog
{
// ����
public:
	CSocketAsynServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SOCKETASYNSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	// ���ڼ����Ķ˿�
	int m_port;
	// ͨ��״̬��Ϣ��ӡ
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
