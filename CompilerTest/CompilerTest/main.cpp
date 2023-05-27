#include "stdafx.h"
    /********* main.c *********/

#include <stdio.h>
#include "getSource.h"
#include "codegen.h"
#include "compile.h"
void testmain()
{
	char fileName[30];		/*　ソースプログラムファイルの名前　*/
	printf("enter source file name\n");
	scanf("%s", fileName);
	if (!openSource(fileName))	/*　ソースプログラムファイルのopen　*/
		return;			/*　openに失敗すれば終わり　*/
	if (compile())			/*　コンパイルして　*/
		execute();			/*　エラーがなければ実行　*/
	closeSource();			/*　ソースプログラムファイルのclose　*/
}

