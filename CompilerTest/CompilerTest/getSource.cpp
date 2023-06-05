#include "stdafx.h"
/**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		//１行の最大文字数
#define MAXERROR 30		//これ以上のエラーがあったら終り
#define MAXNUM 14		//定数の最大桁数
#define TAB  5				//タブのスペース
#define INSERT_C "#0000FF" /* 挿入文字の色 */
#define DELETE_C "#FF0000" /* 削除文字の色 */
#define TYPE_C "#00FF00" /* タイプエラー文字の色 */

static FILE *s_fpi;				//ソースファイル
static FILE *s_fptex;			//LaTeX出力ファイル
static char s_szLine[MAXLINE];	//１行分の入力バッファー
static int s_iLineIndex;			//次に読む文字の位置
static char s_cLast;				//最後に読んだ文字

static Token s_tokenLast;			//最後に読んだトークン
static int s_idKind;			//現トークン(Id)の種類
static int s_iSpaceNum;			//そのトークンの前のスペースの個数
static int s_iCRNum;				//その前のCRの個数
static int s_iTokenHasBeenPrinted;			//トークンは印字済みか

static int s_iErrorCount = 0;			//出力したエラーの数
static char GetNextCharAndProgress();		//次の文字を読む関数
static int isKeySymbole(int iKind);	//tは記号か？
static int isKeyWord(int iKind);		//tは予約語か？
static void printSpaces();		//トークンの前のスペースの印字
static void printcToken();		//トークンの印字

struct keyWd 
{				//予約語や記号と名前(int)
	char *word;
	int iKeyID;
};

static struct keyWd KeywordTable[] = 
{	//予約語や記号と名前(int)の表
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
	//記号と名前(int)の表
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

int isKeyWord(int iKind)			//キーkは予約語か？
{
	return (iKind < KIND_END_OF_KEYWORD);
}

int isKeySymbole(int iKind)		//キーkは記号か？
{
	if (iKind < KIND_END_OF_KEYWORD){return 0;}
	return (iKind < KIND_END_OF_SYMBOLE);
}

static int s_ClassTable[256];		//文字の種類を示す表にする

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

BOOL openSource(char fileName[]) 		//ソースファイルのopen
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

void closeSource()				 //ソースファイルと.html（または.tex）ファイルをclose
{
	fclose(s_fpi);
	fclose(s_fptex);
}

void initSource()
{
	s_iLineIndex = -1;				 //初期設定
	s_cLast= '\n';
	s_iTokenHasBeenPrinted = 1;
	CreateClassTable();
	fprintf(s_fptex,"<HTML>\n");  //htmlコマンド
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


int IncrementErrorCount()			//エラーの個数のカウント、多すぎたら終わり
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

BOOL OutputErrorType(char *m)		//型エラーを.html（または.tex）ファイルに出力
{
	printSpaces();
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	//fprintf(s_fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{", m);
	printcToken();
	//fprintf(s_fptex, "}}\\)");
	return IncrementErrorCount();
}

BOOL OutputErrorInsert(int iKind)		//keyString(k)を.html（または.tex）ファイルに挿入
{
	fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeywordTable[iKind].word);
	// 	if (k < KIND_END_OF_KEYWORD) 	//予約語
	//		 fprintf(s_fptex, "\\ \\insert{{\\bf %s}}", KeywordTable[k].word); 
	//	else 					//演算子か区切り記号
	//	fprintf(s_fptex, "\\ \\insert{$%s$}", KeywordTable[k].word);
	return IncrementErrorCount();
}

int OutputErrorMissingID()			//名前がないとのメッセージを.html（または.tex）ファイルに挿入
{
	fprintf(s_fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{Id}");
	return IncrementErrorCount();
}

int OutputErrorMissingOperator()		//演算子がないとのメッセージを.html（または.tex）ファイルに挿入
{
	fprintf(s_fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
	//fprintf(s_fptex, "\\insert{$\\otimes$}");
	return IncrementErrorCount();
}

int OutputErrorDelete()			//今読んだトークンを読み捨てる
{
	int i=(int)s_tokenLast.m_iKind;
	printSpaces();
	s_iTokenHasBeenPrinted = 1;
	if (i < KIND_END_OF_KEYWORD){fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeywordTable[i].word);} 							//予約語
	else if (i < KIND_END_OF_SYMBOLE){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeywordTable[i].word);}					//演算子か区切り記号
	else if (i == KIND_ID){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, s_tokenLast.u.szIdentifier);}								//Identfier
	else if (i == KIND_NUM){fprintf(s_fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, s_tokenLast.u.value);}								//Num
	return ERR_OUTPUT_NORMAL;
}

int OutputErrMessage(char *szMessage)	//エラーメッセージを.html（または.tex）ファイルに出力
{
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, szMessage);
	//fprintf(s_fptex, "$^{%s}$", m);
	return IncrementErrorCount();
}

int OutputErrAndFinish(char *szMessage)			//エラーメッセージを出力し、コンパイル終了
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

int errorN()				//エラーの個数を返す
{
	return s_iErrorCount;
}

char GetNextCharAndProgress()				//次の１文字を返す関数
{
	char ch;
	if (s_iLineIndex == -1)
	{
		if (fgets(s_szLine, MAXLINE, s_fpi) != NULL)
		{
			//			puts(line); 	//通常のエラーメッセージの出力の場合（参考まで）
			s_iLineIndex = 0;
		} 
		else 
		{
			OutputErrAndFinish("end of file\n");   // end of fileならコンパイル終了
		}
	}

	if ((ch= s_szLine[s_iLineIndex]) == '\n')
	{
		s_iLineIndex = -1;				//それが改行文字なら次の行の入力準備
		return '\n';				//文字としては改行文字を返す
	}
	s_iLineIndex++;
	return ch;
}

Token ProgressAndGetNextToken()			//次のトークンを読んで返す関数
{
	int num;
	int cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			//前のトークンを印字
	s_iSpaceNum = 0; 
	s_iCRNum = 0;
	while (1)
	{				//次のトークンまでの空白や改行をカウント
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
					temp.m_iKind = KeywordTable[i].iKeyID; 		//予約語の場合
					s_tokenLast = temp; 
					s_iTokenHasBeenPrinted = 0;
					return temp;
				}
			}
			temp.m_iKind = KIND_ID;		//ユーザの宣言した名前の場合
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

Token GetTokenWithCheck(Token token, int iKeyID)			//t.m_iKind == k のチェック
	//t.m_iKind == k なら、次のトークンを読んで返す
	//t.m_iKind != k ならエラーメッセージを出し、t と k が共に記号、または予約語なら
	//t を捨て、次のトークンを読んで返す（ t を k で置き換えたことになる）
	//それ以外の場合、k を挿入したことにして、t を返す
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

static void printSpaces()			//空白や改行の印字
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

void printcToken()				//現在のトークンの印字
{
	int i=(int)s_tokenLast.m_iKind;
	if (s_iTokenHasBeenPrinted == 1)
	{
		s_iTokenHasBeenPrinted = 0; 
		return;
	}

	s_iTokenHasBeenPrinted = 1;
	printSpaces();				//トークンの前の空白や改行印字
	if (i < KIND_END_OF_KEYWORD){fprintf(s_fptex, "<b>%s</b>", KeywordTable[i].word);} 						//予約語

	else if (i < KIND_END_OF_SYMBOLE){fprintf(s_fptex, "%s", KeywordTable[i].word);}					//演算子か区切り記号

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

void setIdKind (int iKind)			//現トークン(Id)の種類をセット
{
	s_idKind = iKind;
}



