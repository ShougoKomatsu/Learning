
  /*************** getSource.h ***************/

#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif
 
#define MAXNAME 31			/*　名前の最大長さ　*/
 
#define KIND_BEGIN (1)
#define KIND_END (2)
#define KIND_IF (3)
#define KIND_THEN (4)
#define KIND_WHILE (5)
#define KIND_DO (6)
#define KIND_RET (7)
#define KIND_FUNC (8)
#define KIND_VAR (9)
#define KIND_CONST (10)
#define KIND_ODD (11)
#define KIND_WRITE (12)
#define KIND_WRITE_LINE (13)
#define KIND_END_OF_KEYWORD (14)

#define KIND_PLUS (101)
#define KIND_MINUS (102)
#define KIND_MULT (103)
#define KIND_DIV (104)
#define KIND_PARENTHESIS_L (105)
#define KIND_PARENTHESIS_R (106)
#define KIND_EQUAL (107)		
#define KIND_LESS (108)
#define KIND_GREATER (109)
#define KIND_NOT_EQUAL (110)
#define KIND_LESS_EQUAL (111)
#define KIND_GREATER_EQUAL (112)
#define KIND_COMMA (113)
#define KIND_PERIOD (114)
#define KIND_SEMICOLON (115)
#define KIND_ASSIGN (116)
#define KIND_END_OF_SYMBOLE (117)

#define KIND_ID (200)
#define KIND_NUM (201)
#define KIND_NUL (202)
#define KIND_END_OF_TOKEN (203)
#define KIND_LETTER (204)
#define KIND_DIGIT (205)
#define KIND_COLON (206)
#define KIND_OTHERS (207)


typedef struct token 
{			/*　トークンの型　*/
	int kind;				/*　トークンの種類かキーの名前　*/
	union 
	{
		char szIdentifier[MAXNAME]; 		/*　Identfierの時、その名前　*/
		int value;				/*　Numの時、その値　*/
	} u;
}Token;

Token ProgressAndGetNextToken();				/*　次のトークンを読んで返す　*/
Token GetTokenWithCheck(Token t, int k);	/*　t.kind == kのチェック　*/
	/*　t.kind == kなら、次のトークンを読んで返す　*/
	/*　t.kind != kならエラーメッセージを出し、tとkが共に記号、または予約語なら　*/
	/*　tを捨て、次のトークンを読んで返す（tをkで置き換えたことになる）　*/
	/*　それ以外の場合、kを挿入したことにして、tを返す　*/

#define ERR_OUTPUT_NORMAL (1)
#define ERR_OUTPUT_COUNT_MAX (-1)
#define ERR_OUTPUT_ABORT (-2)

int openSource(char fileName[]); 	/*　ソースファイルのopen　*/
void closeSource();			/*　ソースファイルのclose　*/
void initSource();			/*　テーブルの初期設定、texファイルの初期設定　*/ 
void finalSource(); 			/*　ソースの最後のチェック、texファイルの最終設定　*/ 
int OutputErrorType(char *m);		/*　型エラーを.texファイルに出力　*/
int OutputErrorInsert(int k);		/*　keyString(k)を.texファイルに挿入　*/
int OutputErrorMissingID();		/*　名前がないとのメッセージを.texファイルに挿入　*/
int OutputErrorMissingOperator();		/*　演算子がないとのメッセージを.texファイルに挿入　*/
int OutputErrorDelete();			/*　今読んだトークンを読み捨て（.texファイルに出力）*/
int OutputErrMessage(char *m);	/*　エラーメッセージを.texファイルに出力　*/
int OutputErrAndFinish(char *m);			/*　エラーメッセージを出力し、コンパイル終了　*/
int errorN();				/*　エラーの個数を返す　*/

void setIdKind(KindTable k);   /*　現トークン(Id)の種類をセット（.texファイル出力のため）*/

