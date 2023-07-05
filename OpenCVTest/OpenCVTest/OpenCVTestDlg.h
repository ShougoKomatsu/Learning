
// OpenCVTestDlg.h : ヘッダー ファイル
//

#pragma once

#include "ImageWindow.h"
// COpenCVTestDlg ダイアログ
class COpenCVTestDlg : public CDialogEx
{
// コンストラクション
public:
	COpenCVTestDlg(CWnd* pParent = NULL);	// 標準コンストラクター

// ダイアログ データ
	enum { IDD = IDD_OPENCVTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート
		ImageWindowDlg m_wh;


// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
