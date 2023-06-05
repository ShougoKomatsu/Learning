#include "stdafx.h"
/*********table.c**********/

#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		//名前表の最大長さ
#define MAXNAME 31		//名前の最大長さ
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
			int ParameterNum;		//関数の場合：パラメタ数
		}f;
		RelAddr raddr;		//変数、パラメタの場合：アドレス
	}u;
}TabelE;

static TabelE s_tableName[MAXTABLE];		//名前表
static int s_iNameIndex = 0;			//名前表のインデックス
static int s_iBlockLevel = -1;			//現在のブロックレベル
static int s_iBlockindex[MAXLEVEL];  	//index[i]にはブロックレベルiの最後のインデックス
static int s_iValAddress[MAXLEVEL];  	//addr[i]にはブロックレベルiの最後の変数の番地
static int s_iLocalValAddress;			//現在のブロックの最後の変数の番地
static int s_tfIndex;

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

void TreatBlockBegin(int firstAddr)	//ブロックの始まり(最初の変数の番地)で呼ばれる
{
	if (s_iBlockLevel == -1)
	{			//主ブロックの時、初期設定
		s_iLocalValAddress = firstAddr;
		s_iNameIndex = 0;
		s_iBlockLevel++;
		return;
	}
	if (s_iBlockLevel == MAXLEVEL-1)
	{
		OutputErrAndFinish("too many nested blocks");
	}
	s_iBlockindex[s_iBlockLevel] = s_iNameIndex;		//今までのブロックの情報を格納
	s_iValAddress[s_iBlockLevel] = s_iLocalValAddress;
	s_iLocalValAddress = firstAddr;		//新しいブロックの最初の変数の番地
	s_iBlockLevel++;				//新しいブロックのレベル
	return;
}

void TreatBlockEnd()				//ブロックの終りで呼ばれる
{
	s_iBlockLevel--;
	s_iNameIndex = s_iBlockindex[s_iBlockLevel];		//一つ外側のブロックの情報を回復
	s_iLocalValAddress = s_iValAddress[s_iBlockLevel];
}

int iGetBlockLevel()				//現ブロックのレベルを返す
{
	return s_iBlockLevel;
}

int GetFunctionParameterNum()					//現ブロックの関数のパラメタ数を返す
{
	return s_tableName[s_iBlockindex[s_iBlockLevel-1]].u.f.ParameterNum;
}

void RegisterTheIdentifier(char *szID)			//名前表に名前を登録
{
	if (s_iNameIndex < MAXTABLE) 
	{
		s_iNameIndex++;
		OutputErrAndFinish("too many names");
		return;
	}

	s_iNameIndex++;
	strcpy(s_tableName[s_iNameIndex].name, szID);
}

int RegisterFunction(char *szID, int v)		//名前表に関数名と先頭番地を登録
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].kind = funcId;
	s_tableName[s_iNameIndex].u.f.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.f.raddr.addr = v; 		 //関数の先頭番地
	s_tableName[s_iNameIndex].u.f.ParameterNum= 0; 			 //パラメタ数の初期値
	s_tfIndex = s_iNameIndex;
	return s_iNameIndex;
}

int RegisterParameterName(char *szID)				//名前表にパラメタ名を登録
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].kind = parId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_tfIndex].u.f.ParameterNum++; 		 //関数のパラメタ数のカウント
	return s_iNameIndex;
}

int RegisterVarName(char *szID)			//名前表に変数名を登録
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].kind = varId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.raddr.addr = s_iLocalValAddress;
	 s_iLocalValAddress++;
	return s_iNameIndex;
}

int RegisterConstName(char *szID, int iVal)		//名前表に定数名とその値を登録
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].kind = constId;
	s_tableName[s_iNameIndex].u.value = iVal;
	return s_iNameIndex;
}

void TreatParameterEnd()					//パラメタ宣言部の最後で呼ばれる
{
	int iParameterNum = s_tableName[s_tfIndex].u.f.ParameterNum;
	if (iParameterNum == 0) {return;}

	for (int i=1; i<=iParameterNum; i++)	
	{		//各パラメタの番地を求める
		s_tableName[s_tfIndex+i].u.raddr.addr = i-1-iParameterNum;
	}
}

void changeV(int ti, int newVal)		//名前表[ti]の値（関数の先頭番地）の変更
{
	s_tableName[ti].u.f.raddr.addr = newVal;
}

		//名前idの名前表の位置を返す
	//未宣言の時エラーとする
int GetNameIndex(char* szID, KindTable k)
{
	int i;
	i = s_iNameIndex;
	strcpy(s_tableName[0].name, szID);			//番兵をたてる
	while( strcmp(szID, s_tableName[i].name) != 0 ){i--;}

	if ( i>0 ){return i;}//名前があった

	//名前がなかった
	int iRet;
	iRet = OutputErrorType("undef");
	if(iRet<0){exit(1);}

	if (k == varId) {return RegisterVarName(szID);}	//変数の時は仮登録
	return 0;
}

KindTable GetKind(int i)				//名前表[i]の種類を返す
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

int GetParameterNum(int ti)				//名前表[ti]の関数のパラメタ数を返す
{
	return s_tableName[ti].u.f.ParameterNum;
}

int frameL()				//そのブロックで実行時に必要とするメモリー容量
{
	return s_iLocalValAddress;
}

