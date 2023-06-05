#include "stdafx.h"
/**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		//�P�s�̍ő啶����
#define MAXERROR 30		//����ȏ�̃G���[����������I��
#define MAXNUM 14		//�萔�̍ő包��
#define TAB  5				//�^�u�̃X�y�[�X
#define INSERT_C "#0000FF" /* �}�������̐F */
#define DELETE_C "#FF0000" /* �폜�����̐F */
#define TYPE_C "#00FF00" /* �^�C�v�G���[�����̐F */

static FILE *s_fpi;				//�\�[�X�t�@�C��
static FILE *s_fptex;			//LaTeX�o�̓t�@�C��
static char s_szLine[MAXLINE];	//�P�s���̓��̓o�b�t�@�[
static int s_iLineIndex;			//���ɓǂޕ����̈ʒu
static char s_cLast;				//�Ō�ɓǂ񂾕���

static Token s_tokenLast;			//�Ō�ɓǂ񂾃g�[�N��
static int s_idKind;			//���g�[�N��(Id)�̎��
static int s_iSpaceNum;			//���̃g�[�N���̑O�̃X�y�[�X�̌�
static int s_iCRNum;				//���̑O��CR�̌�
static int s_iTokenHasBeenPrinted;			//�g�[�N���͈󎚍ς݂�

static int s_iErrorCount = 0;			//�o�͂����G���[�̐�
static char GetNextCharAndProgress();		//���̕�����ǂފ֐�
static int isKeySymbole(int iKind);	//t�͋L�����H
static int isKeyWord(int iKind);		//t�͗\��ꂩ�H
static void printSpaces();		//�g�[�N���̑O�̃X�y�[�X�̈�
static void printcToken();		//�g�[�N���̈�

struct keyWd 
{				//�\����L���Ɩ��O(int)
	char *word;
	int iKeyID;
};

static struct keyWd KeywordTable[] = 
{	//�\����L���Ɩ��O(int)�̕\
	{"begin", KIND_BEGIN},
	{"end", KIND_END},
	{"if", KIND_IF},
	{"then", KIND_THEN},
	{"while", KIND_WHILE},
	{"do", KIND_DO},
	{"return", KIND_RET},
	{"function", KIND_FUNC},
	{"var", KIND_VAR},
	{"const", KIND_CONST},
	{"odd", KIND_ODD},
	{"write", KIND_WRITE},
	{"writeln",KIND_WRITE_LINE},
	{"$dummy1",KIND_END_OF_KEYWORD},
	//�L���Ɩ��O(int)�̕\
	{"+", KIND_PLUS},
	{"-", KIND_MINUS},
	{"*", KIND_MULT},
	{"/", KIND_DIV},
	{"(", KIND_PARENTHESIS_L},
	{")", KIND_PARENTHESIS_R},
	{"=", KIND_EQUAL},
	{"<", KIND_LESS},
	{">", KIND_GREATER},
	{"<>", KIND_NOT_EQUAL},
	{"<=", KIND_LESS_EQUAL},
	{">=", KIND_GREATER_EQUAL},
	{",", KIND_COMMA},
	{".", KIND_PERIOD},
	{";", KIND_SEMICOLON},
	{":=", KIND_ASSIGN},
	{"$dummy2",KIND_END_OF_SYMBOLE}
};

int isKeyWord(int iKind)			//�L�[k�͗\��ꂩ�H
{
	return (iKind < KIND_END_OF_KEYWORD);
}

int isKeySymbole(int iKind)		//�L�[k�͋L�����H
{
	if (iKind < KIND_END_OF_KEYWORD){return 0;}
	return (iKind < KIND_END_OF_SYMBOLE);
}

static int s_ClassTable[256];		//�����̎�ނ������\�ɂ���

static void CreateClassTable()
{
	for (int i=0; i<256; i++){s_ClassTable[i] = KIND_OTHERS;}
	for (int i='0'; i<='9'; i++){s_ClassTable[i] = KIND_DIGIT;}
	for (int i='A'; i<='Z'; i++){s_ClassTable[i] = KIND_LETTER;}
	for (int i='a'; i<='z'; i++){s_ClassTable[i] = KIND_LETTER;}
	s_ClassTable['+'] = KIND_PLUS; 
	s_ClassTable['-'] = KIND_MINUS;
	s_ClassTable['*'] = KIND_MULT; 
	s_ClassTable['/'] = KIND_DIV;
	s_ClassTable['('] = KIND_PARENTHESIS_L; 
	s_ClassTable[')'] = KIND_PARENTHESIS_R;
	s_ClassTable['='] = KIND_EQUAL; 
	s_ClassTable['<'] = KIND_LESS;
	s_ClassTable['>'] = KIND_GREATER; 
	s_ClassTable[','] = KIND_COMMA;
	s_ClassTable['.'] = KIND_PERIOD; 
	s_ClassTable[';'] = KIND_SEMICOLON;
	s_ClassTable[':'] = KIND_COLON;
}

BOOL openSource(char fileName[]) 		//�\�[�X�t�@�C����open
{
	char fileNameO[30];
	s_fpi = fopen(fileName,"r");
	if ( s_fpi == NULL ) {printf("can't open %s\n", fileName);return FALSE;}

	strcpy(fileNameO, fileName);
	strcat(fileNameO,".html"); // strcat(fileNameO,".tex");

	s_fptex = fopen(fileNameO,"w");
	if ( s_fptex  == NULL ) {printf("can't open %s\n", fileNameO);return FALSE;} 
	return TRUE;
}

void closeSource()				 //�\�[�X�t�@�C����.html�i�܂���.tex�j�t�@�C����close
{
	fclose(s_fpi);
	fclose(s_fptex);
}

void initSource()
{
	s_iLineIndex = -1;				 //�����ݒ�
	s_cLast= '\n';
	s_iTokenHasBeenPrinted = 1;
	CreateClassTable();
	fprintf(s_fptex,"<HTML>\n");  //html�R�}���h
	fprintf(s_fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(s_fptex,"<BODY>\n<PRE>\n");

}

void finalSource()
{
	if (s_tokenLast.m_iKind == KIND_PERIOD){printcToken();}
	else{BOOL bRet; bRet = OutputErrorInsert(KIND_PERIOD);if(bRet != TRUE){exit(1);}}
	fprintf(s_fptex,"\n</PRE>\n</BODY>\n</HTML>\n");
	// 	fprintf(s_fptex,"\n\\end{document}\n");
}


int IncrementErrorCount()			//�G���[�̌��̃J�E���g�A����������I���
{
	if (s_iErrorCount > MAXERROR)
	{
		s_iErrorCount++;
		fprintf(s_fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
		//fprintf(s_fptex, "too many errors\n\\end{document}\n");
		printf("abort compilation\n");	
		return ERR_OUTPUT_COUNT_MAX;
	}

	s_iErrorCount++;
	return ERR_OUTPUT_NORMAL;
}

BOOL OutputErrorType(char *m)		//�^�G���[��.html�i�܂���.tex�j�t�@�C���ɏo��
{
	printSpaces();
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	//fprintf(s_fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m);
	printcToken();
	//fprintf(s_fptex, "}}\\)");
	return IncrementErrorCount();
}

BOOL OutputErrorInsert(int iKind)		//keyString(k)��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeywordTable[iKind].word);
	// 	if (k < KIND_END_OF_KEYWORD) 	//�\���
	//		 fprintf(s_fptex, "\\ \\insert{{\\bf %s}}", KeywordTable[k].word); 
	//	else 					//���Z�q����؂�L��
	//	fprintf(s_fptex, "\\ \\insert{$%s$}", KeywordTable[k].word);
	return IncrementErrorCount();
}

int OutputErrorMissingID()			//���O���Ȃ��Ƃ̃��b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{Id}");
	return IncrementErrorCount();
}

int OutputErrorMissingOperator()		//���Z�q���Ȃ��Ƃ̃��b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{$\\otimes$}");
	return IncrementErrorCount();
}

int OutputErrorDelete()			//���ǂ񂾃g�[�N����ǂݎ̂Ă�
{
	int i=(int)s_tokenLast.m_iKind;
	printSpaces();
	s_iTokenHasBeenPrinted = 1;
	if (i < KIND_END_OF_KEYWORD){fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeywordTable[i].word);} 							//�\���
	else if (i < KIND_END_OF_SYMBOLE){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeywordTable[i].word);}					//���Z�q����؂�L��
	else if (i == KIND_ID){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, s_tokenLast.u.szIdentifier);}								//Identfier
	else if (i == KIND_NUM){fprintf(s_fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, s_tokenLast.u.value);}								//Num
	return ERR_OUTPUT_NORMAL;
}

int OutputErrMessage(char *szMessage)	//�G���[���b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɏo��
{
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, szMessage);
	//fprintf(s_fptex, "$^{%s}$", m);
	return IncrementErrorCount();
}

int OutputErrAndFinish(char *szMessage)			//�G���[���b�Z�[�W���o�͂��A�R���p�C���I��
{
	int iRet;
	iRet = OutputErrMessage(szMessage);
	if(iRet <0){return ERR_OUTPUT_COUNT_MAX;}

	fprintf(s_fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
	//fprintf(s_fptex, "fatal errors\n\\end{document}\n");
	if (s_iErrorCount){printf("total %d errors\n", s_iErrorCount);}
	printf("abort compilation\n");	
	return ERR_OUTPUT_ABORT;
}

int errorN()				//�G���[�̌���Ԃ�
{
	return s_iErrorCount;
}

char GetNextCharAndProgress()				//���̂P������Ԃ��֐�
{
	char ch;
	if (s_iLineIndex == -1)
	{
		if (fgets(s_szLine, MAXLINE, s_fpi) != NULL)
		{
			//			puts(line); 	//�ʏ�̃G���[���b�Z�[�W�̏o�͂̏ꍇ�i�Q�l�܂Łj
			s_iLineIndex = 0;
		} 
		else 
		{
			OutputErrAndFinish("end of file\n");   // end of file�Ȃ�R���p�C���I��
		}
	}

	if ((ch= s_szLine[s_iLineIndex]) == '\n')
	{
		s_iLineIndex = -1;				//���ꂪ���s�����Ȃ玟�̍s�̓��͏���
		return '\n';				//�����Ƃ��Ă͉��s������Ԃ�
	}
	s_iLineIndex++;
	return ch;
}

Token ProgressAndGetNextToken()			//���̃g�[�N����ǂ�ŕԂ��֐�
{
	int num;
	int cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			//�O�̃g�[�N������
	s_iSpaceNum = 0; 
	s_iCRNum = 0;
	while (1)
	{				//���̃g�[�N���܂ł̋󔒂���s���J�E���g
		if (s_cLast == ' ')
		{
			s_iSpaceNum++;
		}
		else if	(s_cLast == '\t')
		{
			s_iSpaceNum+=TAB;
		}
		else if (s_cLast == '\n')
		{
			s_iSpaceNum = 0; 
			s_iCRNum++;
		}
		else {break;}
		s_cLast= GetNextCharAndProgress();
	}

	switch (cc = s_ClassTable[s_cLast]) 
	{
	case KIND_LETTER:
		{//identifier
			ident[0] = s_cLast;
			s_cLast= GetNextCharAndProgress();
			int i = 1;
			while(1);
			{
				if(s_ClassTable[s_cLast] == KIND_LETTER){break;}	
				if(s_ClassTable[s_cLast] == KIND_DIGIT) {break;}

				if (i < MAXNAME){ident[i] = s_cLast;}
				i++; 
				s_cLast= GetNextCharAndProgress();
			} 

			if (i >= MAXNAME)
			{
				BOOL bRet;
				bRet = OutputErrMessage("too long");
				if(bRet != TRUE){exit (1);}

				i = MAXNAME - 1;
			}	
			ident[i] = '\0'; 
			for (i=0; i<KIND_END_OF_KEYWORD; i++)
			{
				if (strcmp(ident, KeywordTable[i].word) == 0) 
				{
					temp.m_iKind = KeywordTable[i].iKeyID; 		//�\���̏ꍇ
					s_tokenLast = temp; 
					s_iTokenHasBeenPrinted = 0;
					return temp;
				}
			}
			temp.m_iKind = KIND_ID;		//���[�U�̐錾�������O�̏ꍇ
			strcpy(temp.u.szIdentifier, ident);
			break;
		}
	case KIND_DIGIT:
		{//number
			num = 0;

			num = s_cLast-'0';
			s_cLast= GetNextCharAndProgress();

			int i = 1;
			while(1) 
			{
				num = 10*num+(s_cLast-'0');
				i++; 
				s_cLast= GetNextCharAndProgress();
				if(s_ClassTable[s_cLast] != KIND_DIGIT){break;}
			} 

			if (i>MAXNUM)
			{
				BOOL bRet;
				bRet = OutputErrMessage("too large");
				if(bRet != TRUE){exit (1);}
			}
			temp.m_iKind = KIND_NUM;
			temp.u.value = num;
			break;
		}
	case KIND_COLON:
		{
			s_cLast= GetNextCharAndProgress();
			if (s_cLast == '=') 
			{
				s_cLast= GetNextCharAndProgress();
				temp.m_iKind = KIND_ASSIGN;		//":="
				break;
			} 

			temp.m_iKind = KIND_NUL;
			break;
		}
	case KIND_LESS:
		{
			s_cLast= GetNextCharAndProgress();
			if (s_cLast == '=') 
			{
				s_cLast= GetNextCharAndProgress();
				temp.m_iKind = KIND_LESS_EQUAL;		//"<="
				break;
			} 

			if (s_cLast == '>') 
			{
				s_cLast= GetNextCharAndProgress();
				temp.m_iKind = KIND_NOT_EQUAL;		//"<>"
				break;
			} 

			temp.m_iKind = KIND_LESS;
			break;
		}
	case KIND_GREATER:
		{
			s_cLast= GetNextCharAndProgress();
			if (s_cLast == '=') 
			{
				s_cLast= GetNextCharAndProgress();
				temp.m_iKind = KIND_GREATER_EQUAL;		//">="
				break;
			} 
			temp.m_iKind = KIND_GREATER;
			break;
		}
	default:
		{
			temp.m_iKind = cc;
			s_cLast= GetNextCharAndProgress(); 
			break;
		}
	}
	s_tokenLast = temp;
	s_iTokenHasBeenPrinted = 0;
	return temp;
}

Token GetTokenWithCheck(Token token, int iKeyID)			//t.m_iKind == k �̃`�F�b�N
	//t.m_iKind == k �Ȃ�A���̃g�[�N����ǂ�ŕԂ�
	//t.m_iKind != k �Ȃ�G���[���b�Z�[�W���o���At �� k �����ɋL���A�܂��͗\���Ȃ�
	//t ���̂āA���̃g�[�N����ǂ�ŕԂ��i t �� k �Œu�����������ƂɂȂ�j
	//����ȊO�̏ꍇ�Ak ��}���������Ƃɂ��āAt ��Ԃ�
{
	int iRet;
	if (token.m_iKind == iKeyID){return ProgressAndGetNextToken();}


	if (((isKeyWord(iKeyID) == TRUE) && (isKeyWord(token.m_iKind)) == TRUE) )
	{
		iRet = OutputErrorDelete();
		if(iRet < 0){exit(1);}
		iRet = OutputErrorInsert(iKeyID);
		if(iRet < 0){exit(1);}
		return ProgressAndGetNextToken();
	}

	if (((isKeySymbole(iKeyID) == TRUE) && (isKeySymbole(token.m_iKind) == TRUE)))
	{
		iRet = OutputErrorDelete();
		if(iRet<0){exit(1);}
		iRet = OutputErrorInsert(iKeyID);
		if(iRet<0){exit(1);}
		return ProgressAndGetNextToken();
	}

	iRet = OutputErrorInsert(iKeyID);
	if(iRet <0 ){exit(1);}

	return token;
}

static void printSpaces()			//�󔒂���s�̈�
{
	while (s_iCRNum > 0)
	{
		s_iCRNum--;
		fprintf(s_fptex, "\n");
	}
	while (s_iSpaceNum > 0)
	{
		s_iSpaceNum--;
		fprintf(s_fptex, " ");
	}
	s_iCRNum = 0; 
	s_iSpaceNum = 0;
}

void printcToken()				//���݂̃g�[�N���̈�
{
	int i=(int)s_tokenLast.m_iKind;
	if (s_iTokenHasBeenPrinted == 1)
	{
		s_iTokenHasBeenPrinted = 0; 
		return;
	}

	s_iTokenHasBeenPrinted = 1;
	printSpaces();				//�g�[�N���̑O�̋󔒂���s��
	if (i < KIND_END_OF_KEYWORD){fprintf(s_fptex, "<b>%s</b>", KeywordTable[i].word);} 						//�\���

	else if (i < KIND_END_OF_SYMBOLE){fprintf(s_fptex, "%s", KeywordTable[i].word);}					//���Z�q����؂�L��

	else if (i == KIND_ID)
	{							//Identfier
		switch (s_idKind) 
		{
		case KIND_varId: {fprintf(s_fptex, "%s", s_tokenLast.u.szIdentifier); return;}
		case KIND_parId: {fprintf(s_fptex, "<i>%s</i>", s_tokenLast.u.szIdentifier); return;}
		case KIND_funcId: {fprintf(s_fptex, "<i>%s</i>", s_tokenLast.u.szIdentifier); return;}
		case KIND_constId: {fprintf(s_fptex, "<tt>%s</tt>", s_tokenLast.u.szIdentifier); return;}
		}
	}
	else if (i == KIND_NUM){fprintf(s_fptex, "%d", s_tokenLast.u.value);}			//Num

}

void setIdKind (int iKind)			//���g�[�N��(Id)�̎�ނ��Z�b�g
{
	s_idKind = iKind;
}



