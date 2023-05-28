#include "stdafx.h"
/**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		//�P�s�̍ő啶����
#define MAXERROR 30		//����ȏ�̃G���[����������I��
#define MAXNUM  14		//�萔�̍ő包��
#define TAB   5				//�^�u�̃X�y�[�X
#define INSERT_C  "#0000FF"  /* �}�������̐F */
#define DELETE_C  "#FF0000"  /* �폜�����̐F */
#define TYPE_C  "#00FF00"  /* �^�C�v�G���[�����̐F */

static FILE *s_fpi;				//�\�[�X�t�@�C��
static FILE *s_fptex;			//LaTeX�o�̓t�@�C��
static char s_szLine[MAXLINE];	//�P�s���̓��̓o�b�t�@�[
static int s_iLineIndex;			//���ɓǂޕ����̈ʒu
static char s_cLast;				//�Ō�ɓǂ񂾕���

static Token s_tokenLast;			//�Ō�ɓǂ񂾃g�[�N��
static KindTable s_idKind;			//���g�[�N��(Id)�̎��
static int s_iSpaceNum;			//���̃g�[�N���̑O�̃X�y�[�X�̌�
static int s_iCRNum;				//���̑O��CR�̌�
static int s_iTokenHasBeenPrinted;			//�g�[�N���͈󎚍ς݂�

static int s_iErrorCount = 0;			//�o�͂����G���[�̐�
static char nextChar();		//���̕�����ǂފ֐�
static int isKeySym(KeyId k);	//t�͋L�����H
static int isKeyWd(KeyId k);		//t�͗\��ꂩ�H
static void printSpaces();		//�g�[�N���̑O�̃X�y�[�X�̈�
static void printcToken();		//�g�[�N���̈�

struct keyWd 
{				//�\����L���Ɩ��O(KeyId)
	char *word;
	KeyId keyId;
};

static struct keyWd KeywordTable[] = 
{	//�\����L���Ɩ��O(KeyId)�̕\
	{"begin", Begin},
	{"end", End},
	{"if", If},
	{"then", Then},
	{"while", While},
	{"do", Do},
	{"return", Ret},
	{"function", Func},
	{"var", Var},
	{"const", Const},
	{"odd", Odd},
	{"write", Write},
	{"writeln",WriteLn},
	{"$dummy1",end_of_KeyWd},
	//�L���Ɩ��O(KeyId)�̕\
	{"+", Plus},
	{"-", Minus},
	{"*", Mult},
	{"/", Div},
	{"(", Lparen},
	{")", Rparen},
	{"=", Equal},
	{"<", Lss},
	{">", Gtr},
	{"<>", NotEq},
	{"<=", LssEq},
	{">=", GtrEq},
	{",", Comma},
	{".", Period},
	{";", Semicolon},
	{":=", Assign},
	{"$dummy2",end_of_KeySym}
};

int isKeyWd(KeyId k)			//�L�[k�͗\��ꂩ�H
{
	return (k < end_of_KeyWd);
}

int isKeySym(KeyId k)		//�L�[k�͋L�����H
{
	if (k < end_of_KeyWd){return 0;}
	return (k < end_of_KeySym);
}

static KeyId charClassT[256];		//�����̎�ނ������\�ɂ���

static void initCharClassT()		//�����̎�ނ������\�����֐�
{
	int i;
	for (i=0; i<256; i++){charClassT[i] = others;}
	for (i='0'; i<='9'; i++){charClassT[i] = digit;}
	for (i='A'; i<='Z'; i++){charClassT[i] = letter;}
	for (i='a'; i<='z'; i++){charClassT[i] = letter;}
	charClassT['+'] = Plus; charClassT['-'] = Minus;
	charClassT['*'] = Mult; charClassT['/'] = Div;
	charClassT['('] = Lparen; charClassT[')'] = Rparen;
	charClassT['='] = Equal; charClassT['<'] = Lss;
	charClassT['>'] = Gtr; charClassT[','] = Comma;
	charClassT['.'] = Period; charClassT[';'] = Semicolon;
	charClassT[':'] = colon;
}

int openSource(char fileName[]) 		//�\�[�X�t�@�C����open
{
	char fileNameO[30];
	if ( (s_fpi = fopen(fileName,"r")) == NULL ) 
	{
		printf("can't open %s\n", fileName);
		return 0;
	}
	strcpy(fileNameO, fileName);
	strcat(fileNameO,".html");  // strcat(fileNameO,".tex");
	if ( (s_fptex = fopen(fileNameO,"w")) == NULL ) 
	{	 //.html�i�܂���.tex�j�t�@�C�������
		printf("can't open %s\n", fileNameO);
		return 0;
	} 
	return 1;
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
	initCharClassT();
	fprintf(s_fptex,"<HTML>\n");   //html�R�}���h
	fprintf(s_fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(s_fptex,"<BODY>\n<PRE>\n");

}

void finalSource()
{
	if (s_tokenLast.kind==Period){printcToken();}
	else{errorInsert(Period);}
	fprintf(s_fptex,"\n</PRE>\n</BODY>\n</HTML>\n");
	// 	fprintf(s_fptex,"\n\\end{document}\n");
}


void errorNoCheck()			//�G���[�̌��̃J�E���g�A����������I���
{
	if (s_iErrorCount++ > MAXERROR)
	{
		fprintf(s_fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
		//fprintf(s_fptex, "too many errors\n\\end{document}\n");
		printf("abort compilation\n");	
		exit (1);
	}
}

void errorType(char *m)		//�^�G���[��.html�i�܂���.tex�j�t�@�C���ɏo��
{
	printSpaces();
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	//fprintf(s_fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m);
	printcToken();
	//fprintf(s_fptex, "}}\\)");
	errorNoCheck();
}

void errorInsert(KeyId k)		//keyString(k)��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeywordTable[k].word);
	// 	if (k < end_of_KeyWd) 	//�\���
	//		 fprintf(s_fptex, "\\ \\insert{{\\bf %s}}", KeywordTable[k].word); 
	//	else 					//���Z�q����؂�L��
	//	fprintf(s_fptex, "\\ \\insert{$%s$}", KeywordTable[k].word);
	errorNoCheck();
}

void errorMissingId()			//���O���Ȃ��Ƃ̃��b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{Id}");
	errorNoCheck();
}

void errorMissingOp()		//���Z�q���Ȃ��Ƃ̃��b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɑ}��
{
	fprintf(s_fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{$\\otimes$}");
	errorNoCheck();
}

void errorDelete()			//���ǂ񂾃g�[�N����ǂݎ̂Ă�
{
	int i=(int)s_tokenLast.kind;
	printSpaces();
	s_iTokenHasBeenPrinted = 1;
	if (i < end_of_KeyWd){fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeywordTable[i].word);} 							//�\���
	else if (i < end_of_KeySym){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeywordTable[i].word);}					//���Z�q����؂�L��
	else if (i==(int)Id){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, s_tokenLast.u.id);}								//Identfier
	else if (i==(int)Num){fprintf(s_fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, s_tokenLast.u.value);}								//Num

}

void errorMessage(char *m)	//�G���[���b�Z�[�W��.html�i�܂���.tex�j�t�@�C���ɏo��
{
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	//fprintf(s_fptex, "$^{%s}$", m);
	errorNoCheck();
}

void errorF(char *m)			//�G���[���b�Z�[�W���o�͂��A�R���p�C���I��
{
	errorMessage(m);
	fprintf(s_fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
	//fprintf(s_fptex, "fatal errors\n\\end{document}\n");
	if (s_iErrorCount){printf("total %d errors\n", s_iErrorCount);}
	printf("abort compilation\n");	
	exit (1);
}

int errorN()				//�G���[�̌���Ԃ�
{
	return s_iErrorCount;
}

char nextChar()				//���̂P������Ԃ��֐�
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
			errorF("end of file\n");      // end of file�Ȃ�R���p�C���I��
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
	int i = 0;
	int num;
	KeyId cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			//�O�̃g�[�N������
	s_iSpaceNum = 0; 
	s_iCRNum = 0;
	while (1)
	{				//���̃g�[�N���܂ł̋󔒂���s���J�E���g
		if (s_cLast== ' ')
		{
			s_iSpaceNum++;
		}
		else if	(s_cLast== '\t')
		{
			s_iSpaceNum+=TAB;
		}
		else if (s_cLast== '\n')
		{
			s_iSpaceNum = 0; 
			s_iCRNum++;
		}
		else {break;}
		s_cLast= nextChar();
	}
	switch (cc = charClassT[s_cLast]) 
	{
	case letter:
		{//identifier
			do 
			{
				if (i < MAXNAME){ident[i] = s_cLast;}
				i++; 
				s_cLast= nextChar();
			} while (  charClassT[s_cLast] == letter	|| charClassT[s_cLast] == digit );


			if (i >= MAXNAME)
			{
				errorMessage("too long");
				i = MAXNAME - 1;
			}	
			ident[i] = '\0'; 
			for (i=0; i<end_of_KeyWd; i++)
			{
				if (strcmp(ident, KeywordTable[i].word) == 0) 
				{
					temp.kind = KeywordTable[i].keyId;  		//�\���̏ꍇ
					s_tokenLast = temp; 
					s_iTokenHasBeenPrinted = 0;
					return temp;
				}
			}
			temp.kind = Id;		//���[�U�̐錾�������O�̏ꍇ
			strcpy(temp.u.id, ident);
			break;
		}
	case digit:
		{//number
			num = 0;
			do 
			{
				num = 10*num+(s_cLast-'0');
				i++; 
				s_cLast= nextChar();
			} while (charClassT[s_cLast] == digit);

			if (i>MAXNUM){errorMessage("too large");}
			temp.kind = Num;
			temp.u.value = num;
			break;
		}
	case colon:
		{
			if ((s_cLast= nextChar()) == '=') 
			{
				s_cLast= nextChar();
				temp.kind = Assign;		//":="
				break;
			} 

			temp.kind = nul;
			break;
		}
	case Lss:
		{
			if ((s_cLast= nextChar()) == '=') 
			{
				s_cLast= nextChar();
				temp.kind = LssEq;		//"<="
				break;
			} 

			if (s_cLast== '>') 
			{
				s_cLast= nextChar();
				temp.kind = NotEq;		//"<>"
				break;
			} 

			temp.kind = Lss;
			break;
		}
	case Gtr:
		{
			if ((s_cLast= nextChar()) == '=') 
			{
				s_cLast= nextChar();
				temp.kind = GtrEq;		//">="
				break;
			} 
			temp.kind = Gtr;
			break;
		}
	default:
		{
			temp.kind = cc;
			s_cLast= nextChar(); 
			break;
		}
	}
	s_tokenLast = temp;
	s_iTokenHasBeenPrinted = 0;
	return temp;
}

Token checkGet(Token t, KeyId k)			//t.kind == k �̃`�F�b�N
	//t.kind == k �Ȃ�A���̃g�[�N����ǂ�ŕԂ�
	//t.kind != k �Ȃ�G���[���b�Z�[�W���o���At �� k �����ɋL���A�܂��͗\���Ȃ�
	//t ���̂āA���̃g�[�N����ǂ�ŕԂ��i t �� k �Œu�����������ƂɂȂ�j
	//����ȊO�̏ꍇ�Ak ��}���������Ƃɂ��āAt ��Ԃ�
{
	if (t.kind==k){return ProgressAndGetNextToken();}

	if ((isKeyWd(k) && isKeyWd(t.kind)) )
	{
		errorDelete();
		errorInsert(k);
		return ProgressAndGetNextToken();
	}

	if ((isKeySym(k) && isKeySym(t.kind)))
	{
		errorDelete();
		errorInsert(k);
		return ProgressAndGetNextToken();
	}

	errorInsert(k);
	return t;
}

static void printSpaces()			//�󔒂���s�̈�
{
	while (s_iCRNum-- > 0){fprintf(s_fptex, "\n");}
	while (s_iSpaceNum-- > 0){fprintf(s_fptex, " ");}
	s_iCRNum = 0; 
	s_iSpaceNum = 0;
}

void printcToken()				//���݂̃g�[�N���̈�
{
	int i=(int)s_tokenLast.kind;
	if (s_iTokenHasBeenPrinted==1)
	{
		s_iTokenHasBeenPrinted = 0; 
		return;
	}

	s_iTokenHasBeenPrinted = 1;
	printSpaces();				//�g�[�N���̑O�̋󔒂���s��
	if (i < end_of_KeyWd){fprintf(s_fptex, "<b>%s</b>", KeywordTable[i].word);} 						//�\���

	else if (i < end_of_KeySym){fprintf(s_fptex, "%s", KeywordTable[i].word);}					//���Z�q����؂�L��

	else if (i==(int)Id)
	{							//Identfier
		switch (s_idKind) 
		{
		case varId: {fprintf(s_fptex, "%s", s_tokenLast.u.id); return;}
		case parId: {fprintf(s_fptex, "<i>%s</i>", s_tokenLast.u.id); return;}
		case funcId: {fprintf(s_fptex, "<i>%s</i>", s_tokenLast.u.id); return;}
		case constId: {fprintf(s_fptex, "<tt>%s</tt>", s_tokenLast.u.id); return;}
		}
	}
	else if (i==(int)Num){fprintf(s_fptex, "%d", s_tokenLast.u.value);}			//Num

}

void setIdKind (KindTable k)			//���g�[�N��(Id)�̎�ނ��Z�b�g
{
	s_idKind = k;
}


