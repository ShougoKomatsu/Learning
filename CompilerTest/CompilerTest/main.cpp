#include "stdafx.h"
    /********* main.c *********/

#include <stdio.h>
#include "getSource.h"
#include "codegen.h"
#include "compile.h"
void testmain()
{
	char fileName[30];		/*�@�\�[�X�v���O�����t�@�C���̖��O�@*/
	printf("enter source file name\n");
	scanf("%s", fileName);
	if (!openSource(fileName))	/*�@�\�[�X�v���O�����t�@�C����open�@*/
		return;			/*�@open�Ɏ��s����ΏI���@*/
	if (compile())			/*�@�R���p�C�����ā@*/
		execute();			/*�@�G���[���Ȃ���Ύ��s�@*/
	closeSource();			/*�@�\�[�X�v���O�����t�@�C����close�@*/
}

