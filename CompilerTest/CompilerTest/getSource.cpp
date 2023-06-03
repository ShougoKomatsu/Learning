#include "stdafx.h"
/**************getSource.c************/

#include <stdio.h>
#include <string.h>
#include "getSource.h"

#define MAXLINE 120		//１行の最大文字数
#define MAXERROR 30		//これ以上のエラーがあったら終り
#define MAXNUM  14		//定数の最大桁数
#define TAB   5				//タブのスペース
#define INSERT_C  "#0000FF"  /* 挿入文字の色 */
#define DELETE_C  "#FF0000"  /* 削除文字の色 */
#define TYPE_C  "#00FF00"  /* タイプエラー文字の色 */

static FILE *s_fpi;				//ソースファイル
static FILE *s_fptex;			//LaTeX出力ファイル
static char s_szLine[MAXLINE];	//１行分の入力バッファー
static int s_iLineIndex;			//次に読む文字の位置
static char s_cLast;				//最後に読んだ文字

static Token s_tokenLast;			//最後に読んだトークン
static KindTable s_idKind;			//現トークン(Id)の種類
static int s_iSpaceNum;			//そのトークンの前のスペースの個数
static int s_iCRNum;				//その前のCRの個数
static int s_iTokenHasBeenPrinted;			//トークンは印字済みか

static int s_iErrorCount = 0;			//出力したエラーの数
static char GetNextChar();		//次の文字を読む関数
static int isKeySym(KeyId k);	//tは記号か？
static int isKeyWd(KeyId k);		//tは予約語か？
static void printSpaces();		//トークンの前のスペースの印字
static void printcToken();		//トークンの印字

struct keyWd 
{				//予約語や記号と名前(KeyId)
	char *word;
	KeyId keyId;
};

static struct keyWd KeywordTable[] = 
{	//予約語や記号と名前(KeyId)の表
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
	//記号と名前(KeyId)の表
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

int isKeyWd(KeyId k)			//キーkは予約語か？
{
	return (k < end_of_KeyWd);
}

int isKeySym(KeyId k)		//キーkは記号か？
{
	if (k < end_of_KeyWd){return 0;}
	return (k < end_of_KeySym);
}

static KeyId s_ClassTable[256];		//文字の種類を示す表にする

static void CreateClassTable()
{
	for (int i=0; i<256; i++){s_ClassTable[i] = others;}
	for (int i='0'; i<='9'; i++){s_ClassTable[i] = digit;}
	for (int i='A'; i<='Z'; i++){s_ClassTable[i] = letter;}
	for (int i='a'; i<='z'; i++){s_ClassTable[i] = letter;}
	s_ClassTable['+'] = Plus; 
	s_ClassTable['-'] = Minus;
	s_ClassTable['*'] = Mult; 
	s_ClassTable['/'] = Div;
	s_ClassTable['('] = Lparen; 
	s_ClassTable[')'] = Rparen;
	s_ClassTable['='] = Equal; 
	s_ClassTable['<'] = Lss;
	s_ClassTable['>'] = Gtr; 
	s_ClassTable[','] = Comma;
	s_ClassTable['.'] = Period; 
	s_ClassTable[';'] = Semicolon;
	s_ClassTable[':'] = colon;
}

BOOL openSource(char fileName[]) 		//ソースファイルのopen
{
	char fileNameO[30];
	s_fpi = fopen(fileName,"r");
	if ( s_fpi == NULL ) {printf("can't open %s\n", fileName);return FALSE;}

	strcpy(fileNameO, fileName);
	strcat(fileNameO,".html");  // strcat(fileNameO,".tex");

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
	fprintf(s_fptex,"<HTML>\n");   //htmlコマンド
	fprintf(s_fptex,"<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
	fprintf(s_fptex,"<BODY>\n<PRE>\n");

}

void finalSource()
{
	if (s_tokenLast.kind==Period){printcToken();}
	else{BOOL bRet; bRet = OutputErrorInsert(Period);if(bRet != TRUE){exit(1);}}
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

BOOL OutputErrorInsert(KeyId k)		//keyString(k)を.html（または.tex）ファイルに挿入
{
	fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeywordTable[k].word);
	// 	if (k < end_of_KeyWd) 	//予約語
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
	int i=(int)s_tokenLast.kind;
	printSpaces();
	s_iTokenHasBeenPrinted = 1;
	if (i < end_of_KeyWd){fprintf(s_fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeywordTable[i].word);} 							//予約語
	else if (i < end_of_KeySym){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeywordTable[i].word);}					//演算子か区切り記号
	else if (i==(int)Id){fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, s_tokenLast.u.id);}								//Identfier
	else if (i==(int)Num){fprintf(s_fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, s_tokenLast.u.value);}								//Num
	return ERR_OUTPUT_NORMAL;
}

int OutputErrMessage(char *m)	//エラーメッセージを.html（または.tex）ファイルに出力
{
	fprintf(s_fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
	//fprintf(s_fptex, "$^{%s}$", m);
	return IncrementErrorCount();
}

int OutputErrAndFinish(char *m)			//エラーメッセージを出力し、コンパイル終了
{
	int iRet;
	iRet = OutputErrMessage(m);
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

char GetNextChar()				//次の１文字を返す関数
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
			OutputErrAndFinish("end of file\n");      // end of fileならコンパイル終了
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
	int i = 0;
	int num;
	KeyId cc;
	Token temp;
	char ident[MAXNAME];
	printcToken(); 			//前のトークンを印字
	s_iSpaceNum = 0; 
	s_iCRNum = 0;
	while (1)
	{				//次のトークンまでの空白や改行をカウント
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
		s_cLast= GetNextChar();
	}
	switch (cc = s_ClassTable[s_cLast]) 
	{
	case letter:
		{//identifier
			do 
			{
				if (i < MAXNAME){ident[i] = s_cLast;}
				i++; 
				s_cLast= GetNextChar();
			} while (  s_ClassTable[s_cLast] == letter	|| s_ClassTable[s_cLast] == digit );


			if (i >= MAXNAME)
			{
				BOOL bRet;
				bRet = OutputErrMessage("too long");
					if(bRet != TRUE){exit (1);}

				i = MAXNAME - 1;
			}	
			ident[i] = '\0'; 
			for (i=0; i<end_of_KeyWd; i++)
			{
				if (strcmp(ident, KeywordTable[i].word) == 0) 
				{
					temp.kind = KeywordTable[i].keyId;  		//予約語の場合
					s_tokenLast = temp; 
					s_iTokenHasBeenPrinted = 0;
					return temp;
				}
			}
			temp.kind = Id;		//ユーザの宣言した名前の場合
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
				s_cLast= GetNextChar();
			} while (s_ClassTable[s_cLast] == digit);

			if (i>MAXNUM)
			{
				BOOL bRet;
				bRet = OutputErrMessage("too large");
					if(bRet != TRUE){exit (1);}
			}
			temp.kind = Num;
			temp.u.value = num;
			break;
		}
	case colon:
		{
			if ((s_cLast= GetNextChar()) == '=') 
			{
				s_cLast= GetNextChar();
				temp.kind = Assign;		//":="
				break;
			} 

			temp.kind = nul;
			break;
		}
	case Lss:
		{
			if ((s_cLast= GetNextChar()) == '=') 
			{
				s_cLast= GetNextChar();
				temp.kind = LssEq;		//"<="
				break;
			} 

			if (s_cLast== '>') 
			{
				s_cLast= GetNextChar();
				temp.kind = NotEq;		//"<>"
				break;
			} 

			temp.kind = Lss;
			break;
		}
	case Gtr:
		{
			if ((s_cLast= GetNextChar()) == '=') 
			{
				s_cLast= GetNextChar();
				temp.kind = GtrEq;		//">="
				break;
			} 
			temp.kind = Gtr;
			break;
		}
	default:
		{
			temp.kind = cc;
			s_cLast= GetNextChar(); 
			break;
		}
	}
	s_tokenLast = temp;
	s_iTokenHasBeenPrinted = 0;
	return temp;
}

Token checkGet(Token t, KeyId k)			//t.kind == k のチェック
	//t.kind == k なら、次のトークンを読んで返す
	//t.kind != k ならエラーメッセージを出し、t と k が共に記号、または予約語なら
	//t を捨て、次のトークンを読んで返す（ t を k で置き換えたことになる）
	//それ以外の場合、k を挿入したことにして、t を返す
{
	int iRet;
	if (t.kind==k){return ProgressAndGetNextToken();}

	if ((isKeyWd(k) && isKeyWd(t.kind)) )
	{
		iRet = OutputErrorDelete();
		if(iRet != TRUE){exit(1);}
		iRet = OutputErrorInsert(k);
		if(iRet != TRUE){exit(1);}
		return ProgressAndGetNextToken();
	}

	if ((isKeySym(k) && isKeySym(t.kind)))
	{
		iRet = OutputErrorDelete();
		if(iRet<0){exit(1);}
		iRet = OutputErrorInsert(k);
		if(iRet<0){exit(1);}
		return ProgressAndGetNextToken();
	}

	iRet = OutputErrorInsert(k);
	if(iRet <0 ){exit(1);}

	return t;
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
	int i=(int)s_tokenLast.kind;
	if (s_iTokenHasBeenPrinted==1)
	{
		s_iTokenHasBeenPrinted = 0; 
		return;
	}

	s_iTokenHasBeenPrinted = 1;
	printSpaces();				//トークンの前の空白や改行印字
	if (i < end_of_KeyWd){fprintf(s_fptex, "<b>%s</b>", KeywordTable[i].word);} 						//予約語

	else if (i < end_of_KeySym){fprintf(s_fptex, "%s", KeywordTable[i].word);}					//演算子か区切り記号

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

void setIdKind (KindTable k)			//現トークン(Id)の種類をセット
{
	s_idKind = k;
}



