
// OpenCVTestDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "OpenCVTest.h"
#include "OpenCVTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COpenCVTestDlg ダイアログ




COpenCVTestDlg::COpenCVTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COpenCVTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpenCVTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COpenCVTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &COpenCVTestDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// COpenCVTestDlg メッセージ ハンドラー
#include "CVImage.h"
BOOL COpenCVTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void COpenCVTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void COpenCVTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR COpenCVTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//#include "decolor.h"

#include "ImageWindow.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

void COpenCVTestDlg::OnBnClickedButton1()
{
	CVImage test;
	test.Assign(_T("d:\\\\test.bmp"));
	CString sss;

	HWND hWnd;
	CWnd* cwnd;
	cwnd = GetDlgItem(IDC_STATIC_IMG);
	hWnd = cwnd->m_hWnd;
	m_wh.OpenWindow(hWnd, 10, 10, 400, 300,DISPLAY_VISIBLE);
	ImgRGB imgRGB;
	AfxMessageBox(_T("1"));
	imgRGB.Assign(&test);

	m_wh.SetPart(0, 0,imgRGB.iHeight-1, imgRGB.iWidth-1);
	AfxMessageBox(_T("2"));
	m_wh.DispImage(&imgRGB);
	AfxMessageBox(_T("3"));
	
	cv::VideoCapture capture; 
	int iRet;
	iRet =capture.open(0, cv::CAP_ANY);
	sss.Format(_T("%d"), iRet);
	AfxMessageBox(sss);
	cv::Mat frame;
	iRet = capture.read(frame);

	test.Assign(frame);
	imgRGB.Assign(&test);	
	m_wh.SetPart(0, 0,imgRGB.iHeight-1, imgRGB.iWidth-1);
	while(1)
	{
	iRet = capture.read(frame);
	test.Assign(frame);
	imgRGB.Assign(&test);	
	m_wh.DispImage(&imgRGB);
	Sleep(10);
	}
}
