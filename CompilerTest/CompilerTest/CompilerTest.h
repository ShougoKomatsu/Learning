
// CompilerTest.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error _T("PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CCompilerTestApp:
// このクラスの実装については、CompilerTest.cpp を参照してください。
//

class CCompilerTestApp : public CWinApp
{
public:
	CCompilerTestApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CCompilerTestApp theApp;