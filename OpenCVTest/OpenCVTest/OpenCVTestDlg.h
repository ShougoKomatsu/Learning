
// OpenCVTestDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once

#include "ImageWindow.h"
// COpenCVTestDlg �_�C�A���O
class COpenCVTestDlg : public CDialogEx
{
// �R���X�g���N�V����
public:
	COpenCVTestDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^�[

// �_�C�A���O �f�[�^
	enum { IDD = IDD_OPENCVTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g
		ImageWindowDlg m_wh;


// ����
protected:
	HICON m_hIcon;

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
