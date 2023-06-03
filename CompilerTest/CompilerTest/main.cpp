#include "stdafx.h"

#include <stdio.h>
#include "getSource.h"
#include "codegen.h"
#include "compile.h"
void testmain()
{
	char fileName[30];		/*　ソースプログラムファイルの名前　*/
	printf("enter source file name\n");
	scanf("%s", fileName);
	if (!openSource(fileName))	{return;}/*　ソースプログラムファイルのopen　*/
					/*　openに失敗すれば終わり　*/
	if (compile()){execute();}			/*　コンパイルして　*/
					/*　エラーがなければ実行　*/
	closeSource();			/*　ソースプログラムファイルのclose　*/
}

