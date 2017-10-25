// SocketAsynServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SocketAsynServer.h"
#include "SocketAsynServerDlg.h"
#include ".\socketasynserverdlg.h"
#include "fstream"
#define HAVE_STRUCT_TIMESPEC									//解决my_global.h中timespec重复定义的问题
#include <iostream>
#include <cstdlib>												//为了使用system函数
#include <my_global.h>											//包含基础库如stdio.h，如果在win下编程最好包括了，虽然会出一些问题，比如需要预定义HAVE_STRUCT_TIMESPEC
#include <my_sys.h>												//包含可移植的宏和定义，好像删除了也没大碍
#include <mysql.h>												//各类mysql操作所需要的库
using namespace std;
static void print_error(MYSQL *temp, char *message);			//汇报出错详情
void process_statement(MYSQL *conn, char *stmt_str);			//一种通用的指令输入模式
void process_result_set(MYSQL *conn, MYSQL_RES *res_set);		//结果集返回函数
void print_dashes(MYSQL_RES *res_set);							//画出指定格式的函数
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
ofstream outfile;
static MYSQL *conn;											//连接处理器指针
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
int id_point = -1;
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSocketAsynServerDlg 对话框



CSocketAsynServerDlg::CSocketAsynServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSocketAsynServerDlg::IDD, pParent)
	, m_port(0)
	, m_sMsg(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSocketAsynServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_LIST_RECV, m_sListRecv);
	DDX_Text(pDX, IDC_EDIT_Send, m_sMsg);
}

BEGIN_MESSAGE_MAP(CSocketAsynServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnBnClickedButtonSend)
	ON_LBN_SELCHANGE(IDC_LIST_RECV, &CSocketAsynServerDlg::OnLbnSelchangeListRecv)
	ON_EN_CHANGE(IDC_EDIT_Send, &CSocketAsynServerDlg::OnEnChangeEditSend)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CSocketAsynServerDlg::OnEnChangeEditPort)
	ON_BN_CLICKED(IDC_BUTTON2, &CSocketAsynServerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSocketAsynServerDlg 消息处理程序

BOOL CSocketAsynServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	m_sListenSocket.SetParent(this);
	m_connectSocket.SetParent(this);
	m_port=8080;
	UpdateData(FALSE);
	/////////////////////////////////////////////////////////////////////////////////数据库连接部分
	my_init();													//一些初始化工作
	static char *opt_host_name = "localhost";				//服务器主机 一台远程主机 本地就用localhost
	static char *opt_user_name = "root";						//用户名
	static char *opt_password = "960118";							//密码
	static unsigned int opt_port_num = 0;						//不指定连接端口，即连接默认端口
	static char *opt_socket_name = NULL;						//套接字名（此处使用内建值）
	static char *opt_db_name = "bicycle";							//连接的数据库名
	static unsigned int opt_flags = 0;
	//MYSQL 是一个包含连接信息的结构

	if (mysql_library_init(0, NULL, NULL)) {					//初始化客户端开发库
		print_error(conn, "wow, some errors happened");
		system("pause");
		exit(1);												//系统级别的结束进程
	}

	conn = mysql_init(NULL);									//初始化连接处理器，当参数为NULL会自动生成一个MYSQL结构并初始化，然后返回一个指向它的指针
	if (conn == NULL) {							 				//若内存不足则会返回NULL
		print_error(conn, "mysql_init() failed\
						 (probably out of memory)");
		system("pause");
		exit(1);
	}

	if (mysql_real_connect(conn, opt_host_name, opt_user_name,
		opt_password, opt_db_name, opt_port_num,
		opt_socket_name, opt_flags) == NULL)						//连接数据库,如果连接失败则返回NULL 
	{
		print_error(conn, "mysql_real_connect() failed");
		system("pause");
		exit(1);
	}
	/////////////////////////////////////
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CSocketAsynServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSocketAsynServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CSocketAsynServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSocketAsynServerDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//OnOK();
	UpdateData(TRUE);
	ListenPort=m_port;
	m_sListenSocket.Create(ListenPort);
	m_sListenSocket.Listen(10);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CSocketAsynServerDlg::OnAccept()
{
	CMySocket *pSocket=new CMySocket();
	if (m_sListenSocket.Accept(*pSocket)!=0)
	{  
		CString connectStr;
		//helloStr="接受到一连接";
		POSITION pos;
		UINT sport; 
		pSocket->GetPeerName(pSocket->ClientIP,pSocket->Port);
		connectStr.Format("接受到来自%s的连接",pSocket->ClientIP);
		
		for (pos=m_connectList.GetHeadPosition();pos!=NULL;)
		{
			CMySocket *p=(CMySocket *)m_connectList.GetNext(pos);
			//p->Send((LPCSTR)connectStr,connectStr.GetLength());
		}
		pSocket->SetParent(this);
		m_connectList.AddTail(pSocket);
		//m_sListRecvMsg.AddString(helloStr);
		m_sListRecv.AddString(connectStr);
		UpdateData(false);
		id_point++;
	}
	else
		delete pSocket;
}
IDVECTOR idvector[20];

void CSocketAsynServerDlg::OnReceive()
{
	char *pBuf=new char[1025];
	char *IDBuf = new char[20];
	strcpy(IDBuf, "");
	char *tempBuf = new char[20];
	char *test = new char[50];
	int nBufLen=1024;
	int nReceived;
	int clientpoint=-1;													//用于指明客户信息在第几快内存的指针
	int deletepoint = -1;												//用于删除不需要字符的指针
	int errorpoint = -1;												//在查询指令中起作用，告诉原客户端没有要找的人
	int	donotdisplaypoint = -1;											//用于控制 不将特殊指令发往群聊
	int quitpoint = -1;													//指明第几个地址快要退出连接
	int gpspoint = -1;													//指明哪个客户端要gps信息
	CString strReceived;
	CString IPAddress;
	UINT PORTAddress;
	POSITION local;
	POSITION pos;
	pos=m_connectList.GetHeadPosition();
	do{
		CMySocket *p=(CMySocket *)m_connectList.GetNext(pos);
		
		nReceived=p->Receive(pBuf,nBufLen);
		
		IPAddress=p->ClientIP;
		PORTAddress=p->Port;
	}while(pos!=NULL&&nReceived==SOCKET_ERROR);	
	local = pos;
	if (nReceived!=SOCKET_ERROR)
	{
		pBuf[nReceived]=NULL;
		//strReceived=pBuf;
		int cx=0;
		strReceived=IPAddress+":\n"+pBuf;
		//strReceived = pBuf;
		//m_sListRecv.AddString(strReceived);
		/***************************************************
		********服务器通用模块，提取发送消息的用户ID********
		***************************************************/
		int i = 0;
		int j = 0;
		strcpy(tempBuf, pBuf);
		while (tempBuf[i] != '\0')
		{
			if (tempBuf[i] == '&')
				break;
			if (tempBuf[i] >= '0'&&tempBuf[i] <= '9')
			{
				IDBuf[j++]=tempBuf[i];					
		}
			i++;
		}
		IDBuf[j] = '\0';
		/***************************************************
		***********服务器通用模块，记录用户的连接***********
		***************************************************/
		if (pBuf[0] == 'I'&&pBuf[1] == 'D'&&pBuf[2] == '@')
		{
			strcpy(idvector[id_point].id, IDBuf);
			donotdisplaypoint = 1;
		}
		/***************************************************
		***********服务器通用模块，离线用户连接*************
		***************************************************/
		if (pBuf[0] == 'q'&&pBuf[1] == 'u'&&pBuf[2] == 'i'&&pBuf[3] == 't'&&pBuf[4] == '$')
		{
			donotdisplaypoint = 1;
			for (int i = 0; i < 20; i++) {
				if (strcmp(IDBuf, idvector[i].id) == 0)
				{
					quitpoint = i;								//查找要退出连接的ID所在的地址块
					break;
				}
			}
			int q = 0;
			for (pos = m_connectList.GetHeadPosition(); pos != NULL;)
			{
				CMySocket *p = (CMySocket*)m_connectList.GetNext(pos);
				if (q == quitpoint) {
					//	m_connectList.RemoveAt(pos);
					p->Close();
				}
				q++;
			}
			for (int i = 0; i < 20; i++) {
				if (strcmp(idvector[i].id, IDBuf) == 0) {
					strcpy(idvector[i].id, "");			//删除ID表中的该ID，私聊就找不到这个人了
					break;
				}
			}
		}

		/***************************************************
		********物联网车锁模块，车辆定位上传、存储**********
		***************************************************/
		if (pBuf[0] == 'b'&&pBuf[1] == 'k'&&pBuf[2] == 'g'&&pBuf[3] == 'p'&&pBuf[4] == 's') {
			donotdisplaypoint = 1;
			char *GPSBuf_E=new char[50];
			char *GPSBuf_N = new char[50];
			char *BKID = new char[10];
			char sqlstr[500];
			int c = 0;
			int x = 5;
			while (pBuf[x] != '$')													//bkgps00001$3029.60430$11423.52267$
				BKID[c++] = pBuf[x++];												//UPDATE bicycle SET location_n='3029.60430',
																					//location_e='11423.52267'WHERE bicycle_id=00001;
			BKID[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')													
				GPSBuf_N[c++] = pBuf[x++];
			GPSBuf_N[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')
				GPSBuf_E[c++] = pBuf[x++];
			GPSBuf_E[c] = '\0';
			donotdisplaypoint = 1;
			strcpy(sqlstr, "UPDATE bicycle SET location_n='");
			strcat(sqlstr, GPSBuf_N);
			strcat(sqlstr, "', location_e='");
			strcat(sqlstr, GPSBuf_E);
			strcat(sqlstr, "' WHERE bicycle_id=");
			strcat(sqlstr, BKID);
			strcat(sqlstr, ";");
			process_statement(conn, sqlstr);
			free(GPSBuf_E);
			free(GPSBuf_N);
			free(BKID);
		}
		/***************************************************
		*******物联网车锁模块，手机用户查询附近车辆*********
		********************未完成**************************
		***************************************************/
		if (pBuf[0] == 'b'&&pBuf[1] == 'k'&&pBuf[2] == 'c'&&pBuf[3] == 'x')
		{
			donotdisplaypoint = 1;
			char sqlstr[500];
			char *BKUSRLN = new char[12];
			char *BKUSRLE = new char[12];
			char *GPSBuf_N = new char[20];
			char *GPSBuf_E = new char[20];
			int BKNB;
			char BKGPS[100];
			donotdisplaypoint = 1;
			int x = 4, c = 0;
			while (pBuf[x] != '$')
				BKUSRLN[c++] = pBuf[x++];
			BKUSRLN[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')
				BKUSRLE[c++] = pBuf[x++];
			BKUSRLE[c] = '\0';
																		//SELECT location_n,location_e FROM bicycle 
																		//WHERE location_n<(3029.61430+0.3) AND location_n>(3029.61430-0.3)
																		//AND location_e<(11423.52267+0.3)AND location_e>(11423.52267-0.3);
			strcpy(sqlstr, "SELECT location_n,location_e FROM bicycle WHERE location_n<(");
			strcat(sqlstr, BKUSRLN);
			strcat(sqlstr, "+0.3) AND location_n>(");
			strcat(sqlstr, BKUSRLN);
			strcat(sqlstr, "-0.3) AND location_e<(");
			strcat(sqlstr, BKUSRLE);
			strcat(sqlstr, "+0.3) AND location_e>(");
			strcat(sqlstr, BKUSRLN);
			strcat(sqlstr, "-0.3);");
			process_statement(conn, sqlstr);

			/////////////////////////////////////////////////
			/*获取每辆车的GPS信息*/
			/*数组存储每辆车的N和E，还要存储数量BKNB*/
			////////////////////////////////////////////////
			for (int i = 0; i < 20; i++) {
				if (strcmp(IDBuf, idvector[i].id) == 0)
				{
					gpspoint = i;										//查找发起请求的客户端
					break;
				}
			}
			int q = 0;
			for (pos = m_connectList.GetHeadPosition(); pos != NULL;)
			{
				CMySocket *p = (CMySocket*)m_connectList.GetNext(pos);
				if (q == gpspoint) {									//传送GPS信息
					int nSend;
					for (int i = 0; i < BKNB; i++) {
						//////////////////////////////////
						/*如何将每辆车的信息单独提取出来*/
						//////////////////////////////////
						nSend = p->Send(GPSBuf_N, 10);
						Sleep(1000);
						nSend = p->Send(GPSBuf_E, 10);
					}
				}
				q++;
			}
			free(BKUSRLE);
			free(BKUSRLN);
			free(GPSBuf_E);
			free(GPSBuf_N);
		}
		/***************************************************
		*********物联网车锁模块，手机用户完成注册***********
		***************************************************/
		if (pBuf[0] == 'p'&&pBuf[1] == 'h'&&pBuf[2] == 'r'&&pBuf[3] == 'g')
		{
			donotdisplaypoint = 1;
			int x = 4;
			int c = 0;
			char sqlstr[500];
			char *BKUSRPH = new char[12];
			char *BKUSREML = new char[30];
			char *BKUSRCTY = new char[20];
			char *BKUSRCITY = new char[20];

			while (pBuf[x] != '$')
				BKUSRPH[c++] = pBuf[x++];
			BKUSRPH[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')
				BKUSREML[c++] = pBuf[x++];
			BKUSREML[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')
				BKUSRCTY[c++] = pBuf[x++];
			BKUSRCTY[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != '$')
				BKUSRCITY[c++] = pBuf[x++];
			BKUSRCITY[c] = '\0';
																		//INSERT INTO user VALUES(13681751056,
																		//now(), '361655243@qq.com', 'China', 'Shanghai', '2888-08-18');
			strcpy(sqlstr, "INSERT INTO user VALUES (");
			strcat(sqlstr, BKUSRPH);
			strcat(sqlstr, ",now(), '");
			strcat(sqlstr, BKUSREML);
			strcat(sqlstr, "', '");
			strcat(sqlstr, BKUSRCTY);
			strcat(sqlstr, "', '");
			strcat(sqlstr, BKUSRCITY);
			strcat(sqlstr, "', now());");
			process_statement(conn, sqlstr);
			memset(sqlstr, 0, sizeof(sqlstr));
			free(BKUSRPH);
			free(BKUSREML);
			free(BKUSRCTY);
			free(BKUSRCITY);
		}

		/***************************************************
		***物联网车锁模块，记录开始骑车事件，更新车锁状态***
		***************************************************/
		if (pBuf[0] == 'b'&&pBuf[1] == 'k'&&pBuf[2] == 'o'&&pBuf[3] == 'p')
		{
			donotdisplaypoint = 1;
			int x = 4;
			int c = 0;
			char sqlstr[500];
			char *BKUSRPH = new char[12];
			char *BKID = new char[6];
																		//提取请求的手机号和用车车牌号
			while (pBuf[x] != '$')										//提取发出请求的手机号
				BKUSRPH[c++] = pBuf[x++];
			BKUSRPH[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != 'o')										//提取单车车牌
				BKID[c++] = pBuf[x++];
			BKID[c] = '\0';
																		//生成一条骑行记录
																		//INSERT INTO ride_event VAlUES(X,X,now(),curtime(),NULL,NULL);
			strcpy(sqlstr,"INSERT INTO ride_event VALUES (");
			strcat(sqlstr, BKUSRPH);
			strcat(sqlstr, ",");
			strcat(sqlstr, BKID);
			strcat(sqlstr,",now(), curtime(), NULL, NULL);\0");
			process_statement(conn, sqlstr);
			memset(sqlstr, 0, sizeof(sqlstr));
																		//更新自行车状态为O(open)
																		//UPDATE bicycle SET status = 'O' WHERE bicycle_id = XXXXX;
			strcpy(sqlstr,"UPDATE bicycle SET status = 'O' WHERE bicycle_id =");
			strcat(sqlstr, BKID);
			strcat(sqlstr, ";\0");
			process_statement(conn, sqlstr);	
			memset(sqlstr, 0, sizeof(sqlstr));							//必要清理工作
			free(BKID);
			free(BKUSRPH);
		}
		/***************************************************
		***物联网车锁模块，记录结束骑车事件，更新车锁状态***
		***************************************************/
		if (pBuf[0] == 'b'&&pBuf[1] == 'k'&&pBuf[2] == 'c'&&pBuf[3] == 'l')
		{
			donotdisplaypoint = 1;
			int x = 4;
			int c = 0;
			char sqlstr[500];
			char *BKUSRPH = new char[12];
			char *BKID = new char[6];
																		//提取请求的手机号和用车车牌号
			while (pBuf[x] != '$')										//提取发出请求的手机号
				BKUSRPH[c++] = pBuf[x++];
			BKUSRPH[c] = '\0';
			x++;
			c = 0;
			while (pBuf[x] != 'c')										//提取单车车牌
				BKID[c++] = pBuf[x++];
			BKID[c] = '\0';
																		//生成一条骑行记录
																		//UPDATE ride_event SET endday=now(),endtime=curtime() 
																		//WHERE bicycle_id = XXXXX && endday <= >NULL&&endtime <= >NULL;
			strcpy(sqlstr, "UPDATE ride_event SET endday=now(),endtime=curtime() WHERE bicycle_id = ");
			strcat(sqlstr, BKID);
			strcat(sqlstr, "&& endday <=>NULL&&endtime <=>NULL;");
			process_statement(conn, sqlstr);
			memset(sqlstr, 0, sizeof(sqlstr));
																		//更新自行车状态为C(close)
																		//UPDATE bicycle SET status = 'C' WHERE bicycle_id = XXXXX;
			strcpy(sqlstr, "UPDATE bicycle SET status = 'C' WHERE bicycle_id =");
			strcat(sqlstr, BKID);
			strcat(sqlstr, ";\0");
			process_statement(conn, sqlstr);
			memset(sqlstr, 0, sizeof(sqlstr));							//必要清理工作,不然处处会奔溃
			free(BKID);
			free(BKUSRPH);
		}
		/***************************************************
		******聊天程序专用模块，查找私聊对象地址************
		***************************************************/
		if( pBuf[0] == 's'&&pBuf[1] == 'l'&&pBuf[2] == '$')
		{
			donotdisplaypoint = 1;
			for (int i = 0; i < 20; i++) {
				if (strcmp(IDBuf, idvector[i].id) == 0)
				{
					clientpoint = i;								//查找私聊ID所在的地址块
					errorpoint = -1;								//告诉原客户端没找到你要私聊的人
					break;
				}
				errorpoint = 1;
			}
		}
		/***************************************************
		****聊天程序专用模块，查明请求获得在线列表的用户****
		***************************************************/

		if (pBuf[0]=='c'&&pBuf[1] == 'x'&&pBuf[2] == '$') {
			cx = 1;
			donotdisplaypoint = 1;
			//clientpoint = -1;
			for (int i = 0; i < 20; i++)
				if (strcmp(IDBuf, idvector[i].id) == 0)
				{
					clientpoint = i;
					break;
				}
		}
		strReceived = pBuf;
		/***************************************************
		*******聊天程序专用模块，判断是否为私聊信息*********
		***************************************************/
		if (pBuf[0] == '#'&&pBuf[1] == '#'&&pBuf[2] == '$')
		{
			j = 0;
			int x = 0;
			while (pBuf[x] != '\0')
			{
				if (pBuf[x] == '$')
				{
					deletepoint = x;
				}
				if (deletepoint != -1)
				{
					tempBuf[j++] = pBuf[x + 1];
				}
				x++;
			}
			deletepoint = -1;
			strReceived = tempBuf;
		}
		/***************************************************
		***********聊天程序专用模块，去除控制字符***********
		***************************************************/
		if (clientpoint >= 0) {													//判断是群聊还是私聊
			j = 0;																//去除控制字符
			int q = 0;
			while (pBuf[q] != '\0')
			{
				if (pBuf[q] == '$')
				{
					deletepoint = q;
				}
				if (deletepoint != -1)
				{
					tempBuf[j++] = pBuf[q + 1];
				}
				q++;
			}

			deletepoint = -1;
			//strReceived = tempBuf;
			//去除私聊对象ID编号
			j = 0;
			q = 0;
			while (pBuf[q] != '\0')
			{
				if (pBuf[q] == '&')
				{
					deletepoint = q;
				}
				if (deletepoint != -1)
				{
					tempBuf[j++] = pBuf[q + 1];
				}
				q++;
			}

			deletepoint = -1;
			strReceived = tempBuf;

		}
		m_sListRecv.AddString(strReceived);
		outfile.open("服务器聊天记录.txt", ios_base::app);
		outfile << strReceived << endl;
		outfile.close();
		/***************************************************
		**********服务器通用模块，广播信息或点对点**********
		***************************************************/

		int nSend;
		int nSendBufLen;
		nSendBufLen=strReceived.GetLength();
		POSITION pos2;
		POSITION pos;
		if (clientpoint <0) {
			for (pos = m_connectList.GetHeadPosition(); pos != NULL;)
			{
				CMySocket *p = (CMySocket*)m_connectList.GetNext(pos);

				if (pos == local&&errorpoint == 1) {
					errorpoint = -1;
					nSend = p->Send("未找到指定用户", 16);
				}
				if (pos != local&&donotdisplaypoint==-1) {
					if(strstr(strReceived,"ID@")==NULL&&strstr(strReceived, "quit$") == NULL)
						nSend = p->Send((LPCSTR)strReceived, nSendBufLen);
					}
				/*}*/
			}
		}
		else
		{
			int j = 0;
			for (pos = m_connectList.GetHeadPosition(); j <= clientpoint;)
			{
				CMySocket *p = (CMySocket*)m_connectList.GetNext(pos);
				if (j == clientpoint) {
					////////////////////////////////////////////////////////////////////查询在线列表
					if (cx == 1) {
						nSend = p->Send("!!!!!", 6);
						Sleep(100);
						int i = 0;
						for (pos2 = m_connectList.GetHeadPosition(); pos2 != NULL;)
						{
							CMySocket *q = (CMySocket*)m_connectList.GetNext(pos2);
							//if(pos==local)
							if (strcmp(idvector[i].id, "") != 0) {
								char sendtemp[20]="$$$";
								strcpy(tempBuf, idvector[i].id);
								strcat(sendtemp, tempBuf);
								nSend = p->Send(sendtemp, 20);
								//nSend = p->Send((LPCSTR)q->ClientIP, 20);
								Sleep(100);
							}
							i++;
						}
						//nSend = p->Send("***完毕***'\n'", 11);
					}
					//////////////////////////////////////////////////////////////////////
					else
					nSend = p->Send((LPCSTR)strReceived, nSendBufLen);
				}
				j++;
			}
		}
		donotdisplaypoint = -1;
		clientpoint = -1;
		cx = 0;
		UpdateData(false);
		
	}	
	else
		AfxMessageBox("接收信息错误！");
}
void CSocketAsynServerDlg::OnClose()
{
	m_connectSocket.Close();
}

void CSocketAsynServerDlg::OnBnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int nSend;
	int nBufLen;
	CString MsgStr;

	if (!m_sMsg.IsEmpty())
	{
		MsgStr.Format("本机服务器：%s",m_sMsg);
		m_sListRecv.AddString(MsgStr);

		nBufLen=m_sMsg.GetLength();

		POSITION pos;
		for (pos=m_connectList.GetHeadPosition();pos!=NULL;)
		{
			CMySocket *p=(CMySocket*)m_connectList.GetNext(pos);
			nSend=p->Send((LPCSTR)m_sMsg,nBufLen);
		}

		if (nSend!=SOCKET_ERROR)
		{
			//m_sListSendMsg+=m_sMsg;
			UpdateData(false);
		}	
		else
			AfxMessageBox("发送数据出错！");
	}

	m_sMsg.Empty();
	UpdateData(false);
}


void CSocketAsynServerDlg::OnLbnSelchangeListRecv()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CSocketAsynServerDlg::OnEnChangeEditSend()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CSocketAsynServerDlg::OnEnChangeEditPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

static void print_error(MYSQL *temp, char *message)				//错误报告函数
{
	fprintf(stderr, "%s\n", message);
	if (temp != NULL)
		fprintf(stderr, "Error %u (%s):%s\n", mysql_errno(temp),
			mysql_sqlstate(temp), mysql_error(temp));
}

void process_result_set(MYSQL *conn, MYSQL_RES *res_set)		//返回结果集函数
{
	MYSQL_ROW	row = NULL;									//MYSQL_ROW是指针类型，所以变量为row，而非*row	
	MYSQL_FIELD *field = NULL;
	unsigned long col_len;

	if (mysql_errno(conn))										//错误报告
		print_error(conn, "mysql_fetch_row() failed");
	else
		printf("Number of rows returned: %lu\n",
		(unsigned long)mysql_num_rows(res_set));				//mysql_num_rows()可以返回检索到的行数，强制转UNSIGNED LONG保持其可移植性

	mysql_field_seek(res_set, 0);								//mysql_field_seek()用来定位到第一个结构处
	for (unsigned int i = 0; i < mysql_num_fields(res_set); i++)//遍历每一列，获取列最大值,mysql_num_fields()可获取返回数据表的列数
	{
		field = mysql_fetch_field(res_set);						//返回指向下一列的指针
		col_len = strlen(field->name);							//获取每一列文字的长度
		col_len = (col_len < field->max_length) ? field->max_length : col_len;
		col_len = (col_len < 4 && !IS_NOT_NULL(field->flags)) ? 4 : col_len;
		field->max_length = col_len;							//重置列信息
	}
	print_dashes(res_set);										//固定格式
	fputc('|', stdout);											//固定格式
	mysql_field_seek(res_set, 0);								//校准位置

	for (unsigned int i = 0;									//取得并打印各字段的名称 
		i < mysql_num_fields(res_set); i++)						//mysql_num_fields()可获取返回数据表的列数
	{
		field = mysql_fetch_field(res_set);
		printf(" %-*s |", (int)field->max_length,
			field->name);										//%-*s 代表输入一个字符串，-号代表左对齐、后补空白，*号代表对齐宽度由输入时确定
	}
	fputc('\n', stdout);
	print_dashes(res_set);										//固定格式

	while ((row = mysql_fetch_row(res_set)) != NULL)			//调用mysql_fetch_row()可以依次取回结果集中的每一行，有值返回MYSQL_ROW（指针，指向各列值）,没值返回NULL
	{
		mysql_field_seek(res_set, 0);
		fputc('|', stdout);										//固定格式
		for (unsigned int i = 0; i< mysql_num_fields(res_set); i++)
		{
			field = mysql_fetch_field(res_set);
			printf(" %-*s |", (int)field->max_length,			//这里的每一项内容都是字符串形式
				row[i] != NULL ? row[i] : "NULL");				//每一列上有值就显示，没有就显示NULL
		}
		fputc('\n', stdout);
	}
	print_dashes(res_set);										//固定格式
}

void process_statement(MYSQL *conn, char *stmt_str)
{
	MYSQL_RES *res_set;											//为了获取结果集
	if (mysql_query(conn, stmt_str))							//发送指令，如果失败，报错问题，mysql_query是一个简单的调用函数，但是有较多的限制，比如传递的语句结尾是NULL
	{
		print_error(conn, "could not execute statement");
		system("pause");
		return;
	}
	res_set = mysql_store_result(conn);							//获取结果集
	if (res_set)												//有结果集返回时
	{
		process_result_set(conn, res_set);						//返回结果集
		mysql_free_result(res_set);								//释放占用的内存资源，如果处理完结果集不调用这句话，内存泄漏会导致程序越来越慢。
	}
	else														//无结果集返回时，判断是否有错误产生
	{
		if (mysql_field_count(conn) == 0)						//输入的语句不产生结果集
		{
			printf("Number of rows affected: %lu\n",
				(unsigned long)mysql_affected_rows(conn));
		}
		else
		{
			print_error(conn, "Could not retrieve result set");	//出现问题
		}
	}
}

void print_dashes(MYSQL_RES *res_set)							//画出指定格式的函数
{
	MYSQL_FIELD *field;
	mysql_field_seek(res_set, 0);								//mysql_field_seek()用来定位到第一个结构处
	fputc('+', stdout);
	for (unsigned int i = 0; i < mysql_num_fields(res_set); i++)//mysql_num_fields()可获取返回数据表的列数
	{
		field = mysql_fetch_field(res_set);						// mysql_fetch_field()用来返回后序列结构的指针
		for (unsigned int j = 0; j < field->max_length + 2; j++)
			fputc('-', stdout);
		fputc('+', stdout);
	}
	fputc('\n', stdout);
}

void CSocketAsynServerDlg::OnBnClickedButton2()
{
	mysql_close(conn);											//终止连接。在使用mysql_init(NULL)的情况下，此处可不写参数。
	mysql_library_end();										//做一些必要的清理工作
	mysql_server_end();
	GetDlgItem(IDC_BUTTON_QUIT)->EnableWindow(FALSE);

}
