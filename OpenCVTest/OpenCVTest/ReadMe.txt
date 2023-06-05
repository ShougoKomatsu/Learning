By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Copyright (C) 2000-2018, Intel Corporation, all rights reserved.
Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
Copyright (C) 2009-2016, NVIDIA Corporation, all rights reserved.
Copyright (C) 2010-2013, Advanced Micro Devices, Inc., all rights reserved.
Copyright (C) 2015-2016, OpenCV Foundation, all rights reserved.
Copyright (C) 2015-2016, Itseez Inc., all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.

このプロジェクトは、小松翔悟（作者）が個人的勉強のために作成するものであり、このソフトを使ったことにより発生した、いかなる損害に対しても、作者は責任を負いません。



================================================================================
MICROSOFT FOUNDATION CLASS ライブラリ: OpenCVTest プロジェクトの概要
===============================================================================

アプリケーション ウィザードが作成した OpenCVTest には Microsoft 
Foundation Class の基本的な使い方が示されています。アプリケーション作成のひな型
としてお使いください。

このファイルには OpenCVTest アプリケーションを構成している各ファイ
ルの概要説明が含まれます。

OpenCVTest.vcxproj
   これはアプリケーション ウィザードで生成された VC++ プロジェクトのメイン プ
   ロジェクト ファイルです。ファイルが生成された Visual C++ のバージョン情報が
   含まれています。また、アプリケーション ウィザードで選択したプラットフォー
   ム、構成およびプロジェクト機能に関する情報も含まれています。

OpenCVTest.vcxproj.filters
   これは、アプリケーション ウィザードで生成された VC++ プロジェクトの
   フィルター ファイルです。
   このファイルには、プロジェクト内のファイルとフィルターとの間の関連付けに関する
   情報が含まれています。 この関連付けは、特定のノードで同様の拡張子を持つファイルの
   グループ化を示すために IDE で使用されます (たとえば、".cpp" ファイルは "ソース 
   ファイル" フィルターに関連付けられています)。

OpenCVTest.h
   これはアプリケーションのメイン ヘッダー ファイルです。このファイルには、
   Resource.h を含む、その他のプロジェクト固有のヘッダーが含まれていて、
   COpenCVTestApp アプリケーション クラスを宣言します。

OpenCVTest.cpp
   これは、アプリケーション クラス COpenCVTestApp を含むメイン アプリケー
   ション ソース ファイルです。

OpenCVTest.rc
   これは、プログラムが使用する Microsoft Windows のリソースの一覧ファイルで
   す。このファイルには RES サブディレクトリに保存されているアイコン、ビットマ
   ップ、カーソルが含まれています。このファイルは、Microsoft Visual C++ で直接
   編集することができます。プロジェクト リソースは 1041 にあります。

res\OpenCVTest.ico
   これは、アプリケーションのアイコンとして使用されるアイコンファイルです。この
   アイコンはメイン リソース ファイル OpenCVTest.rc に含まれていま
   す。

res\OpenCVTest.rc2
   このファイルは Microsoft Visual C++ を使用しないで編集されたリソースを含んで
   います。リソース エディタで編集できないリソースはすべてこのファイルに入れて
   ください。


/////////////////////////////////////////////////////////////////////////////

アプリケーション ウィザードは 1 つのダイアログ クラスを作成します:

OpenCVTestDlg.h, OpenCVTestDlg.cpp - ダイアログ
   これらのファイルは COpenCVTestDlg クラスを含みます。このクラスはアプ
   リケーションのメイン ダイアログの動作を定義します。ダイアログ テンプレートは
   Microsoft Visual C++ で編集可能な OpenCVTest.rc に含まれます。


/////////////////////////////////////////////////////////////////////////////

その他の機能:

ActiveX コントロール
   アプリケーションは ActiveX コントロールの使用に関するサポートを含みます。

印刷と印刷プレビューのサポート
   アプリケーション ウィザードは、 MFC ライブラリから CView クラスのメンバ関数
   を呼び出すことによって、印刷、印刷の設定、および印刷プレビュー コマンドを処
   理するコードを生成しました。

/////////////////////////////////////////////////////////////////////////////

その他の標準ファイル:

StdAfx.h, StdAfx.cpp
   これらのファイルは、既にコンパイルされたヘッダー ファイル (PCH) 
   OpenCVTest.pch や既にコンパイルされた型のファイル StdAfx.obj を
   ビルドするために使用されます。

Resource.h
   これは新規リソース ID を定義する標準ヘッダー ファイルです。Microsoft 
   Visual C++ はこのファイルの読み取りと更新を行います。

OpenCVTest.manifest
   アプリケーション マニフェスト ファイルは Windows XP で、Side-by-Side アセン
   ブリの特定のバージョンに関するアプリケーションの依存関係を説明するために使用
   されます。ローダーはこの情報を使用して、アセンブリ キャッシュから適切なアセ
   ンブリを、またはアプリケーションからプライベート アセンブリを読み込みます。
   アプリケーション マニフェストは再頒布用に、実行可能アプリケーションと同じフ
   ォルダにインストールされる外部 .manifest ファイルとして含まれているか、また
   はリソースのフォーム内の実行可能ファイルに含まれています。
/////////////////////////////////////////////////////////////////////////////

その他の注意:

アプリケーション ウィザードは "TODO:" で始まるコメントを使用して、追加したりカ
スタマイズする必要があるソース コードの部分を示します。

アプリケーションが共有 DLL 内で MFC を使用する場合は、MFC DLL を再頒布する必要
があります。また、アプリケーションがオペレーティング システムのロケール以外の言
語を使用している場合も、対応するローカライズされたリソース MFC100XXX.DLL を再頒
布する必要があります。これらのトピックの詳細については、MSDN ドキュメントの 
Visual C++ アプリケーションの再頒布に関するセクションを参照してください。

/////////////////////////////////////////////////////////////////////////////
