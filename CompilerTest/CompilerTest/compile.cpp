#include "stdafx.h"
/*************** compile.c *************/

#include "getSource.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3			//エラーがこれ以下なら実行
#define FIRSTADDR 2			//各ブロックの最初の変数のアドレス

static Token s_Token;				//次のトークンを入れておく

static int CompileTheBlock(int pIndex);	//ブロックのコンパイル
//pIndex はこのブロックの関数名のインデックス
static int constDecl();			//定数宣言のコンパイル
static int varDecl();				//変数宣言のコンパイル
static int funcDecl();			//関数宣言のコンパイル

static int CompileTheStatement();			//文のコンパイル
static int CompileTheExpression();			//式のコンパイル
static int CompileTheTerm();				//式の項のコンパイル
static int CompileTheFactor();				//式の因子のコンパイル
static int CompileTheCondition();			//条件式のコンパイル
static int IsStartWithBeginKey(Token t);		//トークンtは文の先頭のキーか？

int compile()
{
	printf("start compilation\n");
	initSource();				//getSourceの初期設定
	s_Token = ProgressAndGetNextToken();			//最初のトークン
	TreatBlockBegin(FIRSTADDR);		//これ以後の宣言は新しいブロックのもの
	int iRet;
	iRet = CompileTheBlock(0);					//0 はダミー（主ブロックの関数名はない）
	if(iRet < 0){return iRet;}

	finalSource();
	int i = errorN();				//エラーメッセージの個数
	if (i != 0){printf("%d errors\n", i);}
	/*	listCode();	*/			//目的コードのリスト（必要なら）
	return i<MINERROR;		//エラーメッセージの個数が少ないかどうかの判定
}

int CompileTheBlock(int pIndex)		//pIndex はこのブロックの関数名のインデックス
{
	int iRet;
	int intBackPatchNum;
	intBackPatchNum = genCodeV(OPERATION_CODE_JMP, 0);		//内部関数を飛び越す命令、後でバックパッチ
	while (1) 
	{				//宣言部のコンパイルを繰り返す
		switch (s_Token.m_iKind)
		{
		case KIND_CONST:
			{
				//定数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				iRet = constDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		case KIND_VAR:
			{//変数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				iRet = varDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		case KIND_FUNC:
			{//関数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				iRet = funcDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		default:
			{//それ以外なら宣言部は終わり
				break;
			}
		}
		break;
	}			
	backPatch(intBackPatchNum);			//内部関数を飛び越す命令にパッチ
	changeV(pIndex, GetNextCodeIndex());	//この関数の開始番地を修正
	genCodeV(OPERATION_CODE_ICT, frameL());		//このブロックの実行時の必要記憶域をとる命令

	iRet = CompileTheStatement();				//このブロックの主文		
	if(iRet != TRUE){return iRet;}

	genCodeR();				//リターン命令
	TreatBlockEnd();				//ブロックが終ったことをtableに連絡
	return 0;
}	

int constDecl()			//定数宣言のコンパイル
{
	Token temp;
	int iRet;
	while(1)
	{
		if (s_Token.m_iKind == KIND_ID)
		{
			setIdKind(KIND_constId);				//印字のための情報のセット
			temp = s_Token; 					//名前を入れておく
			s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_EQUAL);		//名前の次は"="のはず
			if (s_Token.m_iKind == KIND_NUM)
			{
				RegisterConstName(temp.u.szIdentifier, s_Token.u.value);	//定数名と値をテーブルに
			}
			else
			{
				iRet = OutputErrorType(_T("number"));
				if(iRet < 0){return iRet;}
			}
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			iRet = OutputErrorMissingID();
			if(iRet < iRet){return iRet;}
		}

		if (s_Token.m_iKind != KIND_COMMA)
		{		//次がコンマなら定数宣言が続く
			if (s_Token.m_iKind != KIND_ID){break;}
			//次が名前ならコンマを忘れたことにする
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet < 0){return iRet;}
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//最後は");"のはず
	return 0;
}

int varDecl()				//変数宣言のコンパイル
{
	int iRet;
	while(1)
	{
		if (s_Token.m_iKind == KIND_ID)
		{
			setIdKind(KIND_varId);		//印字のための情報のセット
			RegisterVarName(s_Token.u.szIdentifier);		//変数名をテーブルに、番地はtableが決める
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			iRet = OutputErrorMissingID();
			if(iRet < 0 ){return iRet;}
		}

		if (s_Token.m_iKind != KIND_COMMA)
		{		//次がコンマなら変数宣言が続く
			if (s_Token.m_iKind != KIND_ID){break;}

			//次が名前ならコンマを忘れたことにする
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet <0){return iRet;}
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//最後は");"のはず
	return 0;
}

int funcDecl()			//関数宣言のコンパイル
{
	int iRet;
	int iFuncIndex ;
	if (s_Token.m_iKind != KIND_ID)
	{
		iRet = OutputErrorMissingID();			//関数名がない
		if(iRet < 0){return iRet;}
		return 0;
	}


	setIdKind(KIND_funcId);				//印字のための情報のセット
	iFuncIndex = RegisterFunction(s_Token.u.szIdentifier, GetNextCodeIndex());		//関数名をテーブルに登録
	//その先頭番地は、まず、次のコードの番地GetNextCodeIndex()とする
	s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_PARENTHESIS_L);
	TreatBlockBegin(FIRSTADDR);	//パラメタ名のレベルは関数のブロックと同じ
	while(1)
	{
		if (s_Token.m_iKind != KIND_ID){break;}
		//パラメタ名がある場合
		setIdKind(KIND_parId);		//印字のための情報のセット
		RegisterParameterName(s_Token.u.szIdentifier);		//パラメタ名をテーブルに登録
		s_Token = ProgressAndGetNextToken();


		if (s_Token.m_iKind != KIND_COMMA)
		{		//次がコンマならパラメタ名が続く
			if (s_Token.m_iKind != KIND_ID){break;}

			//次が名前ならコンマを忘れたことに
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet <0){return iRet;}

			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_PARENTHESIS_R);		//最後は")"のはず
	TreatParameterEnd();				//パラメタ部が終わったことをテーブルに連絡
	if (s_Token.m_iKind == KIND_SEMICOLON)
	{
		iRet = OutputErrorDelete();
		if(iRet < 0){return iRet;}
		s_Token = ProgressAndGetNextToken();
	}
	iRet = CompileTheBlock(iFuncIndex );	//ブロックのコンパイル、その関数名のインデックスを渡す
	if(iRet < 0){return iRet;}

	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//最後は");"のはず
	return 0;
}

int CompileTheStatement()			//文のコンパイル
{
	int tIndex;
	int iKind;
	int backP, backP2;				//バックパッチ用
	int iRet;

	while(1) 
	{
		switch (s_Token.m_iKind) 
		{
		case KIND_ID:
			{//代入文のコンパイル
				tIndex = GetNameIndex(s_Token.u.szIdentifier, KIND_varId);	//左辺の変数のインデックス

				iKind=GetKind(tIndex);
				setIdKind(iKind);			//印字のための情報のセット

				if (iKind == KIND_varId ){}
				else if(iKind == KIND_parId){}
				else
				{
					iRet = OutputErrorType(_T("var/par"));
					if(iRet < 0){return iRet;}
				} 		

				s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_ASSIGN);			//":="のはず
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}

				genCodeT(OPERATION_CODE_STO, tIndex);				//左辺への代入命令
				return 0;
			}
		case KIND_IF:
			{//if文のコンパイル
				s_Token = ProgressAndGetNextToken();
				CompileTheCondition();					//条件式のコンパイル
				s_Token = GetTokenWithCheck(s_Token, KIND_THEN);		//"then"のはず
				backP = genCodeV(OPERATION_CODE_JPC, 0);			//jpc命令
				iRet = CompileTheStatement();					//文のコンパイル
				if(iRet < 0){return iRet;}
				backPatch(backP);				//上のjpc命令にバックパッチ
				return TRUE;
			}
		case KIND_RET:
			{//return文のコンパイル
				s_Token = ProgressAndGetNextToken();
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}
				genCodeR();					//ret命令
				return TRUE;
			}
		case KIND_BEGIN:
			{
				//begin . . end文のコンパイル
				s_Token = ProgressAndGetNextToken();
				while(1)
				{
					iRet = CompileTheStatement();				//文のコンパイル
					if(iRet < 0){return iRet;}
					while(1)
					{
						if (s_Token.m_iKind == KIND_SEMICOLON)
						{		//次が");"なら文が続く
							s_Token = ProgressAndGetNextToken();
							break;
						}
						if (s_Token.m_iKind == KIND_END)
						{			//次がendなら終り
							s_Token = ProgressAndGetNextToken();
							return TRUE;
						}
						if (IsStartWithBeginKey(s_Token) == TRUE)
						{		//次が文の先頭記号なら
							iRet = OutputErrorInsert(KIND_SEMICOLON);	//");"を忘れたことにする
							if(iRet < 0){return iRet;}

							break;
						}
						iRet = OutputErrorDelete();	//それ以外ならエラーとして読み捨てる
						if(iRet < 0){return iRet;}
						s_Token = ProgressAndGetNextToken();
					}
				}
			}
		case KIND_WHILE:
			{//while文のコンパイル
				s_Token = ProgressAndGetNextToken();
				backP2 = GetNextCodeIndex();			//while文の最後のjmp命令の飛び先
				CompileTheCondition();				//条件式のコンパイル
				s_Token = GetTokenWithCheck(s_Token, KIND_DO);	//"do"のはず
				backP = genCodeV(OPERATION_CODE_JPC, 0);		//条件式が偽のとき飛び出すjpc命令
				iRet = CompileTheStatement();				//文のコンパイル
				if(iRet < 0){return iRet;}

				genCodeV(OPERATION_CODE_JMP, backP2);		//while文の先頭へのジャンプ命令
				backPatch(backP);	//偽のとき飛び出すjpc命令へのバックパッチ
				return 0;
			}
		case KIND_WRITE:
			{			//write文のコンパイル
				s_Token = ProgressAndGetNextToken();
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}
				genCodeO(OPERATOR_WRT);				//その値を出力するwrt命令
				return 0;
			}
		case KIND_WRITE_LINE:
			{			//writeln文のコンパイル
				s_Token = ProgressAndGetNextToken();
				genCodeO(OPERATOR_WRL);				//改行を出力するwrl命令
				return 0;
			}
		case KIND_END:			{return 0;}	
		case KIND_SEMICOLON:	{return 0;}		//空文を読んだことにして終り

		default:
			{//文の先頭のキーまで読み捨てる
				iRet = OutputErrorDelete();				//今読んだトークンを読み捨てる
				if(iRet <0 ){return iRet;}

				s_Token = ProgressAndGetNextToken();
				continue;
			}
		}		
	}
	return 0;
}

BOOL IsStartWithBeginKey(Token t)			//トークンtは文の先頭のキーか？
{
	switch (t.m_iKind)
	{
	case KIND_IF: {return TRUE;}
	case KIND_BEGIN: {return TRUE;}
	case KIND_RET:{return TRUE;}
	case KIND_WHILE:{return TRUE;}
	case KIND_WRITE: {return TRUE;}
	case KIND_WRITE_LINE:{return TRUE;}
	default:{return FALSE;}
	}
}

int CompileTheExpression()
{
	int iKind;
	iKind = s_Token.m_iKind;
	if (iKind == KIND_PLUS || iKind == KIND_MINUS)
	{
		s_Token = ProgressAndGetNextToken();
		int iRet;
		iRet = CompileTheTerm();
		if(iRet <0){return iRet;}

		if (iKind == KIND_MINUS)
		{
			genCodeO(OPERATOR_NEG);
		}
	}
	else
	{
		CompileTheTerm();
	}

	iKind = s_Token.m_iKind;
	while (iKind == KIND_PLUS || iKind == KIND_MINUS)
	{
		s_Token = ProgressAndGetNextToken();
		CompileTheTerm();
		if (iKind == KIND_MINUS){genCodeO(OPERATOR_SUB);}
		else{genCodeO(OPERATOR_ADD);}
		iKind = s_Token.m_iKind;
	}
	return 0;
}

int CompileTheTerm()					//式の項のコンパイル
{
	int iKind;
	int iRet;
	iRet = CompileTheFactor();
	if(iRet <0){return iRet;}

	iKind = s_Token.m_iKind;
	while (iKind == KIND_MULT || iKind == KIND_DIV)
	{	
		s_Token = ProgressAndGetNextToken();
		iRet = CompileTheFactor();
		if(iRet <0){return iRet;}

		if (iKind == KIND_MULT){genCodeO(OPERATOR_MUL);}
		else{genCodeO(OPERATOR_DIV);}
		iKind = s_Token.m_iKind;
	}
	return 0;
}

BOOL CompileTheFactor()					//式の因子のコンパイル
{
	int tIndex;
	int iKind;
	int iRet;
	if (s_Token.m_iKind == KIND_ID)
	{
		tIndex = GetNameIndex(s_Token.u.szIdentifier, KIND_varId);
		iKind = GetKind(tIndex);
		setIdKind(GetKind(tIndex));			//印字のための情報のセット
		switch (iKind) 
		{
		case KIND_varId:
			{//変数名かパラメタ名
				genCodeT(OPERATION_CODE_LOD, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_parId:	
			{//変数名かパラメタ名
				genCodeT(OPERATION_CODE_LOD, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_constId:		
			{			//定数名
				genCodeV(OPERATION_CODE_LIT, val(tIndex));
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_funcId:
			{//関数呼び出し
				s_Token = ProgressAndGetNextToken();
				if (s_Token.m_iKind == KIND_PARENTHESIS_L)
				{
					int i;
					i=0; 					//iは実引数の個数
					s_Token = ProgressAndGetNextToken();
					if (s_Token.m_iKind != KIND_PARENTHESIS_R) 
					{
						for (; ; ) 
						{
							CompileTheExpression(); //実引数のコンパイル
							i++;	
							if (s_Token.m_iKind == KIND_COMMA)
							{	// 次がコンマなら実引数が続く
								s_Token = ProgressAndGetNextToken();
								continue;
							}
							s_Token = GetTokenWithCheck(s_Token, KIND_PARENTHESIS_R);
							break;
						}
					} 
					else{s_Token = ProgressAndGetNextToken();}

					if (GetParameterNum(tIndex) != i)
					{
						iRet = OutputErrMessage(_T("\\#par"));
						if(iRet < 0){return iRet;}

					}	//pars(tIndex)は仮引数の個数
				}
				else
				{
					iRet = OutputErrorInsert(KIND_PARENTHESIS_L);
					if(iRet < 0){return iRet;}
					iRet = OutputErrorInsert(KIND_PARENTHESIS_R);
					if(iRet < 0){return iRet;}
				}
				genCodeT(OPERATION_CODE_CAL, tIndex);				//call命令
				break;
			}
		}
	}
	else if (s_Token.m_iKind == KIND_NUM)
	{			//定数
		genCodeV(OPERATION_CODE_LIT, s_Token.u.value);
		s_Token = ProgressAndGetNextToken();
	}
	else if (s_Token.m_iKind == KIND_PARENTHESIS_L)
	{			//「(」「因子」「)」
		s_Token = ProgressAndGetNextToken();
		CompileTheExpression();
		s_Token = GetTokenWithCheck(s_Token, KIND_PARENTHESIS_R);
	}
	switch (s_Token.m_iKind)
	{					//因子の後がまた因子ならエラー
	case KIND_ID: 
		{
			iRet = OutputErrorMissingOperator();
			if(iRet < 0){return iRet;}
			iRet = CompileTheFactor();
			if(iRet < 0){return iRet;}

			return TRUE;
		}
	case KIND_NUM: 
		{
			iRet = OutputErrorMissingOperator();
			if(iRet <0){return iRet;}
			iRet = CompileTheFactor();
			if(iRet < 0){return iRet;}

			return TRUE;
		}
	case KIND_PARENTHESIS_L:
		{
			iRet = OutputErrorMissingOperator();
			if(iRet <0){return iRet;}
			iRet = CompileTheFactor();
			if(iRet < 0){return iRet;}

			return TRUE;
		}
	default:
		{
			return TRUE;
		}
	}	
	return FALSE;
}

BOOL CompileTheCondition()					//条件式のコンパイル
{
	int iKeyID;
	int iRet;
	if (s_Token.m_iKind == KIND_ODD)
	{
		s_Token = ProgressAndGetNextToken();
		iRet = CompileTheExpression();
		if(iRet < 0){return iRet;}
		genCodeO(OPERATOR_ODD);
		return TRUE;
	}

	iRet = CompileTheExpression();
	if(iRet < 0){return iRet;}

	iKeyID = s_Token.m_iKind;
	switch(iKeyID)
	{
	case KIND_EQUAL: {break;}
	case KIND_LESS: {break;}
	case KIND_GREATER:{break;}
	case KIND_NOT_EQUAL: {break;}
	case KIND_LESS_EQUAL:{break;}
	case KIND_GREATER_EQUAL:{break;}

	default:
		{
			iRet = OutputErrorType(_T("rel-op"));
			if(iRet < 0){return iRet;}
			break;
		}
	}

	s_Token = ProgressAndGetNextToken();
	iRet = CompileTheExpression();
	if(iRet < 0){return iRet;}

	switch(iKeyID)
	{
	case KIND_EQUAL:	{genCodeO(OPERATOR_EQ); break;}
	case KIND_LESS:	{genCodeO(OPERATOR_LS); break;}
	case KIND_GREATER:	{genCodeO(OPERATOR_GR); break;}
	case KIND_NOT_EQUAL:	{genCodeO(OPERATOR_NEQ); break;}
	case KIND_LESS_EQUAL:	{genCodeO(OPERATOR_LSEQ); break;}
	case KIND_GREATER_EQUAL:	{genCodeO(OPERATOR_GREQ); break;}
	}
	return TRUE;
}

