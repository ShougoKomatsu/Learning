#include "stdafx.h"

#include <stdio.h>
#include "getSource.h"
#include "codegen.h"
#include "compile.h"
void testmain()
{
	char fileName[30];		/*�@�\�[�X�v���O�����t�@�C���̖��O�@*/
	printf("enter source file name\n");
	scanf("%s", fileName);
	if (!openSource(fileName))	{return;}/*�@�\�[�X�v���O�����t�@�C����open�@*/
					/*�@open�Ɏ��s����ΏI���@*/
	if (compile()){execute();}			/*�@�R���p�C�����ā@*/
					/*�@�G���[���Ȃ���Ύ��s�@*/
	closeSource();			/*�@�\�[�X�v���O�����t�@�C����close�@*/
}

