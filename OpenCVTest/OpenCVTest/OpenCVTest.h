
// OpenCVTest.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// COpenCVTestApp:
// このクラスの実装については、OpenCVTest.cpp を参照してください。
//

class COpenCVTestApp : public CWinApp
{
public:
	COpenCVTestApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern COpenCVTestApp theApp;