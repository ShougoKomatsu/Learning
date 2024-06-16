#include "stdafx.h"
/**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		//１行の最大文字数
#define MAXERROR 30		//これ以上のエラーがあったら終り
#define MAXNUM 14		//定数の最大桁数
#define TAB  5				//タブのスペース
#define INSERT_C _T("#0000FF") /* 挿入文字の色 */
#define DELETE_C _T("#FF0000") /* 削除文字の色 */
#define TYPE_C _T("#00FF00") /* タイプエラー文字の色 */

static FILE *s_fpi;				//ソースファイル
static FILE *s_fptex;			//LaTeX出力ファイル
static TCHAR s_szLine[MAXLINE];	//１行分の入力バッファー
static int s_iLineIndex;			//次に読む文字の位置
static TCHAR s_cLast;				//最後に読んだ文字

static Token s_tokenLast;			//最後に読んだトークン
static int s_idKind;			//現トークン(Id)の種類
static int s_iSpaceNum;			//そのトークンの前のスペースの個数
static int s_iCRNum;				//その前のCRの個数
static int s_iTokenHasBeenPrinted;			//トークンは印字済みか

static int s_iErrorCount = 0;			//出力したエラーの数
static TCHAR GetNextCharAndProgress();		//次の文字を読む関数
static int isKeySymbole(int iKind);	//tは記号か？
static int isKeyWord(int iKind);		//tは予約語か？
static void printSpaces();		//トークンの前のスペースの印字
static void printcToken();		//トークンの印字

struct keyWd 
{				//予約語や記号と名前(int)
	TCHAR *word;
	int iKeyID;
};

static struct keyWd KeywordTable[] = 
{	//予約語や記号と名前(int)の表
	{_T("begin"), KIND_BEGIN},
	{_T("end"), KIND_END},
	{_T("if"), KIND_IF},
	{_T("then"), KIND_THEN},
	{_T("while"), KIND_WHILE},
	{_T("do"), KIND_DO},
	{_T("return"), KIND_RET},
	{_T("function"), KIND_FUNC},
	{_T("var"), KIND_VAR},
	{_T("const"), KIND_CONST},
	{_T("odd"), KIND_ODD},
	{_T("write"), KIND_WRITE},
	{_T("writeln"),KIND_WRITE_LINE},
	{_T("$dummy1"),KIND_END_OF_KEYWORD},
	//記号と名前(int)の表
	{_T("+"), KIND_PLUS},
	{_T("-"), KIND_MINUS},
	{_T("*"), KIND_MULT},
	{_T("/"), KIND_DIV},
	{_T("("), KIND_PARENTHESIS_L},
	{_T(")"), KIND_PARENTHESIS_R},
	{_T("="), KIND_EQUAL},
	{_T("<"), KIND_LESS},
	{_T(">"), KIND_GREATER},
	{_T("<>"), KIND_NOT_EQUAL},
	{_T("<="), KIND_LESS_EQUAL},
	{_T(">="), KIND_GREATER_EQUAL},
	{_T("),"), KIND_COMMA},
	{_T("."), KIND_PERIOD},
	{_T(");"), KIND_SEMICOLON},
	{_T(":="), KIND_ASSIGN},
	{_T("$dummy2"),KIND_END_OF_SYMBOLE}
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

BOOL openSource(TCHAR fileName[]) 		//ソースファイルのopen
{
	TCHAR fileNameO[30];
	s_fpi = _wfopen(fileName,_T("r"));
	if ( s_fpi == NULL ) {printf("can't open %s\n", fileName);return FALSE;}

	wcscpy(fileNameO, fileName);
	wcscat(fileNameO,_T(".html")); // strcat(fileNameO,".tex");

	s_fptex = _wfopen(fileNameO,_T("w"));
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
	fwprintf(s_fptex,_T("<HTML>\n"));  //htmlコマンド
	fwprintf(s_fptex,_T("<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n"));
	fwprintf(s_fptex,_T("<BODY>\n<PRE>\n"));

}

void finalSource()
{
	if (s_tokenLast.m_iKind == KIND_PERIOD){printcToken();}
	else{BOOL bRet; bRet = OutputErrorInsert(KIND_PERIOD);if(bRet != TRUE){exit(1);}}
	fwprintf(s_fptex,_T("\n</PRE>\n</BODY>\n</HTML>\n"));
	// 	fwprintf(s_fptex,"\n\\end{document}\n");
}


int IncrementErrorCount()			//エラーの個数のカウント、多すぎたら終わり
{
	if (s_iErrorCount > MAXERROR)
	{
		s_iErrorCount++;
		fwprintf(s_fptex, _T("too many errors\n</PRE>\n</BODY>\n</HTML>\n"));
		//fwprintf(s_fptex, _T("too many errors\n\\end{document}\n");
		printf("abort compilation\n");	
		return ERR_OUTPUT_COUNT_MAX;
	}

	s_iErrorCount++;
	return ERR_OUTPUT_NORMAL;
}

BOOL OutputErrorType(TCHAR *m)		//型エラーを.html（または.tex）ファイルに出力
{
	printSpaces();
	fwprintf(s_fptex, _T("<FONT COLOR=%s>%s</FONT>"), TYPE_C, m);
	//fwprintf(s_fptex, _T("\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{_T("), m);
	printcToken();
	//fwprintf(s_fptex, _T("}}\\)");
	return IncrementErrorCount();
}

BOOL OutputErrorInsert(int iKind)		//keyString(k)を.html（または.tex）ファイルに挿入
{
	fwprintf(s_fptex, _T("<FONT COLOR=%s><b>%s</b></FONT>"), INSERT_C, KeywordTable[iKind].word);
	// 	if (k < KIND_END_OF_KEYWORD) 	//予約語
	//		 fwprintf(s_fptex, _T("\\ \\insert{{\\bf %s}}"), KeywordTable[k].word); 
	//	else 					//演算子か区切り記号
	//	fwprintf(s_fptex, _T("\\ \\insert{$%s$}"), KeywordTable[k].word);
	return IncrementErrorCount();
}

int OutputErrorMissingID()			//名前がないとのメッセージを.html（または.tex）ファイルに挿入
{
	fwprintf(s_fptex, _T("<FONT COLOR=%s>Id</FONT>"), INSERT_C);
	//fwprintf(s_fptex, _T("\\insert{Id}");
	return IncrementErrorCount();
}

int OutputErrorMissingOperator()		//演算子がないとのメッセージを.html（または.tex）ファイルに挿入
{
	fwprintf(s_fptex, _T("<FONT COLOR=%s>@</FONT>"), INSERT_C);
	//fwprintf(s_fptex, _T("\\insert{$\\otimes$}");
	return IncrementErrorCount();
}

int OutputErrorDelete()			//今読んだトークンを読み捨てる
{
	int i=(int)s_tokenLast.m_iKind;
	printSpaces();
	s_iTokenHasBeenPrinted = 1;
	if (i < KIND_END_OF_KEYWORD){fwprintf(s_fptex, _T("<FONT COLOR=%s><b>%s</b></FONT>"), DELETE_C, KeywordTable[i].word);} 							//予約語
	else if (i < KIND_END_OF_SYMBOLE){fwprintf(s_fptex, _T("<FONT COLOR=%s>%s</FONT>"), DELETE_C, KeywordTable[i].word);}					//演算子か区切り記号
	else if (i == KIND_ID){fwprintf(s_fptex, _T("<FONT COLOR=%s>%s</FONT>"), DELETE_C, s_tokenLast.u.szIdentifier);}								//Identfier
	else if (i == KIND_NUM){fwprintf(s_fptex, _T("<FONT COLOR=%s>%d</FONT>"), DELETE_C, s_tokenLast.u.value);}								//Num
	return ERR_OUTPUT_NORMAL;
}

int OutputErrMessage(TCHAR *szMessage)	//エラーメッセージを.html（または.tex）ファイルに出力
{
	fwprintf(s_fptex, _T("<FONT COLOR=%s>%s</FONT>"), TYPE_C, szMessage);
	//fwprintf(s_fptex, _T("$^{%s}$"), m);
	return IncrementErrorCount();
}

int OutputErrAndFinish(TCHAR *szMessage)			//エラーメッセージを出力し、コンパイル終了
{
	int iRet;
	iRet = OutputErrMessage(szMessage);
	if(iRet <0){return ERR_OUTPUT_COUNT_MAX;}

	fwprintf(s_fptex, _T("fatal errors\n</PRE>\n</BODY>\n</HTML>\n"));
	//fwprintf(s_fptex, _T("fatal errors\n\\end{document}\n");
	if (s_iErrorCount){printf("total %d errors\n", s_iErrorCount);}
	printf("abort compilation\n");	
	return ERR_OUTPUT_ABORT;
}

int errorN()				//エラーの個数を返す
{
	return s_iErrorCount;
}

TCHAR GetNextCharAndProgress()				//次の１文字を返す関数
{
	TCHAR ch;
	if (s_iLineIndex == -1)
	{
		if (fgetws(s_szLine, MAXLINE, s_fpi) != NULL)
		{
			//			puts(line); 	//通常のエラーメッセージの出力の場合（参考まで）
			s_iLineIndex = 0;
		} 
		else 
		{
			OutputErrAndFinish(_T("end of file\n"));   // end of fileならコンパイル終了
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
	TCHAR ident[MAXNAME];
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
				bRet = OutputErrMessage(_T("too long"));
				if(bRet != TRUE){exit (1);}

				i = MAXNAME - 1;
			}	
			ident[i] = '\0'; 
			for (i=0; i<KIND_END_OF_KEYWORD; i++)
			{
				if (wcscmp(ident, KeywordTable[i].word) == 0) 
				{
					temp.m_iKind = KeywordTable[i].iKeyID; 		//予約語の場合
					s_tokenLast = temp; 
					s_iTokenHasBeenPrinted = 0;
					return temp;
				}
			}
			temp.m_iKind = KIND_ID;		//ユーザの宣言した名前の場合
			wcscpy(temp.u.szIdentifier, ident);
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
				bRet = OutputErrMessage(_T("too large"));
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
		fwprintf(s_fptex, _T("\n"));
	}
	while (s_iSpaceNum > 0)
	{
		s_iSpaceNum--;
		fwprintf(s_fptex, _T(" "));
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
	if (i < KIND_END_OF_KEYWORD){fwprintf(s_fptex, _T("<b>%s</b>"), KeywordTable[i].word);} 						//予約語

	else if (i < KIND_END_OF_SYMBOLE){fwprintf(s_fptex, _T("%s"), KeywordTable[i].word);}					//演算子か区切り記号

	else if (i == KIND_ID)
	{							//Identfier
		switch (s_idKind) 
		{
		case KIND_varId: {fwprintf(s_fptex, _T("%s"), s_tokenLast.u.szIdentifier); return;}
		case KIND_parId: {fwprintf(s_fptex, _T("<i>%s</i>"), s_tokenLast.u.szIdentifier); return;}
		case KIND_funcId: {fwprintf(s_fptex, _T("<i>%s</i>"), s_tokenLast.u.szIdentifier); return;}
		case KIND_constId: {fwprintf(s_fptex, _T("<tt>%s</tt>"), s_tokenLast.u.szIdentifier); return;}
		}
	}
	else if (i == KIND_NUM){fwprintf(s_fptex, _T("%d"), s_tokenLast.u.value);}			//Num

}

void setIdKind (int iKind)			//現トークン(Id)の種類をセット
{
	s_idKind = iKind;
}



