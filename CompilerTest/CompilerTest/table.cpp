#include "stdafx.h"
/*********table.c**********/

#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		//名前表の最大長さ
#define MAXNAME  31		//名前の最大長さ
#define MAXLEVEL 5		//ブロックの最大深さ

typedef struct tableE 
{		//名前表のエントリーの型
	KindTable kind;			//名前の種類
	char name[MAXNAME];	//名前のつづり
	union 
	{
		int value;			//定数の場合：値
		struct 
		{
			RelAddr raddr;	//関数の場合：先頭アドレス
			int pars;		//関数の場合：パラメタ数
		}f;
		RelAddr raddr;		//変数、パラメタの場合：アドレス
	}u;
}TabelE;

static TabelE s_tableName[MAXTABLE];		//名前表
static int s_iNameIndex = 0;			//名前表のインデックス
static int s_iBlockLevel = -1;			//現在のブロックレベル
static int index[MAXLEVEL];   	//index[i]にはブロックレベルiの最後のインデックス
static int addr[MAXLEVEL];    	//addr[i]にはブロックレベルiの最後の変数の番地
static int localAddr;			//現在のブロックの最後の変数の番地
static int tfIndex;

static char* kindName(KindTable k)		//名前の種類の出力用関数
{
	switch (k)
	{
	case varId: {return "var";}
	case parId: {return "par";}
	case funcId: {return "func";}
	case constId: {return "const";}
	}
}

void blockBegin(int firstAddr)	//ブロックの始まり(最初の変数の番地)で呼ばれる
{
	if (s_iBlockLevel == -1)
	{			//主ブロックの時、初期設定
		localAddr = firstAddr;
		s_iNameIndex = 0;
		s_iBlockLevel++;
		return;
	}
	if (s_iBlockLevel == MAXLEVEL-1)
	{
		errorF("too many nested blocks");
	}
	index[s_iBlockLevel] = s_iNameIndex;		//今までのブロックの情報を格納
	addr[s_iBlockLevel] = localAddr;
	localAddr = firstAddr;		//新しいブロックの最初の変数の番地
	s_iBlockLevel++;				//新しいブロックのレベル
	return;
}

void blockEnd()				//ブロックの終りで呼ばれる
{
	s_iBlockLevel--;
	s_iNameIndex = index[s_iBlockLevel];		//一つ外側のブロックの情報を回復
	localAddr = addr[s_iBlockLevel];
}

int bLevel()				//現ブロックのレベルを返す
{
	return s_iBlockLevel;
}

int fPars()					//現ブロックの関数のパラメタ数を返す
{
	return s_tableName[index[s_iBlockLevel-1]].u.f.pars;
}

void enterT(char *id)			//名前表に名前を登録
{
	if (s_iNameIndex < MAXTABLE)
	{
		s_iNameIndex++;
		strcpy(s_tableName[s_iNameIndex].name, id);
	}
	else 
	{
		s_iNameIndex++;
		errorF("too many names");
	}
}

int enterTfunc(char *id, int v)		//名前表に関数名と先頭番地を登録
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = funcId;
	s_tableName[s_iNameIndex].u.f.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.f.raddr.addr = v;  		 //関数の先頭番地
	s_tableName[s_iNameIndex].u.f.pars = 0;  			 //パラメタ数の初期値
	tfIndex = s_iNameIndex;
	return s_iNameIndex;
}

int enterTpar(char *id)				//名前表にパラメタ名を登録
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = parId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[tfIndex].u.f.pars++;  		 //関数のパラメタ数のカウント
	return s_iNameIndex;
}

int enterTvar(char *id)			//名前表に変数名を登録
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = varId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.raddr.addr = localAddr;
	 localAddr++;
	return s_iNameIndex;
}

int enterTconst(char *id, int v)		//名前表に定数名とその値を登録
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = constId;
	s_tableName[s_iNameIndex].u.value = v;
	return s_iNameIndex;
}

void endpar()					//パラメタ宣言部の最後で呼ばれる
{
	int i;
	int pars = s_tableName[tfIndex].u.f.pars;
	if (pars == 0)  {return;}
	for (i=1; i<=pars; i++)	
	{		//各パラメタの番地を求める
		s_tableName[tfIndex+i].u.raddr.addr = i-1-pars;
	}
}

void changeV(int ti, int newVal)		//名前表[ti]の値（関数の先頭番地）の変更
{
	s_tableName[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindTable k)		//名前idの名前表の位置を返す
	//未宣言の時エラーとする
{
	int i;
	i = s_iNameIndex;
	strcpy(s_tableName[0].name, id);			//番兵をたてる
	while( strcmp(id, s_tableName[i].name) ){i--;}

	if ( i ){return i;}//名前があった
		
	//名前がなかった
	errorType("undef");
	if (k==varId) {return enterTvar(id);}	//変数の時は仮登録
	return 0;
}

KindTable kindT(int i)				//名前表[i]の種類を返す
{
	return s_tableName[i].kind;
}

RelAddr relAddr(int ti)				//名前表[ti]のアドレスを返す
{
	return s_tableName[ti].u.raddr;
}

int val(int ti)					//名前表[ti]のvalueを返す
{
	return s_tableName[ti].u.value;
}

int pars(int ti)				//名前表[ti]の関数のパラメタ数を返す
{
	return s_tableName[ti].u.f.pars;
}

int frameL()				//そのブロックで実行時に必要とするメモリー容量
{
	return localAddr;
}

