
// CompilerTest.h : PROJECT_NAME �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#pragma once

#ifndef __AFXWIN_H__
	#error _T("PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"		// ���C�� �V���{��


// CCompilerTestApp:
// ���̃N���X�̎����ɂ��ẮACompilerTest.cpp ���Q�Ƃ��Ă��������B
//

class CCompilerTestApp : public CWinApp
{
public:
	CCompilerTestApp();

// �I�[�o�[���C�h
public:
	virtual BOOL InitInstance();

// ����

	DECLARE_MESSAGE_MAP()
};

extern CCompilerTestApp theApp;