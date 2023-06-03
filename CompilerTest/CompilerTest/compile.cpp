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

static void CompileTheBlock(int pIndex);	//ブロックのコンパイル
//pIndex はこのブロックの関数名のインデックス
static void constDecl();			//定数宣言のコンパイル
static void varDecl();				//変数宣言のコンパイル
static void funcDecl();			//関数宣言のコンパイル

static void CompileTheStatement();			//文のコンパイル
static void CompileTheExpression();			//式のコンパイル
static void CompileTheTerm();				//式の項のコンパイル
static void CompileTheFactor();				//式の因子のコンパイル
static void CompileTheCondition();			//条件式のコンパイル
static BOOL IsStartWithBeginKey(Token t);		//トークンtは文の先頭のキーか？

int compile()
{
	printf("start compilation\n");
	initSource();				//getSourceの初期設定
	s_Token = ProgressAndGetNextToken();			//最初のトークン
	blockBegin(FIRSTADDR);		//これ以後の宣言は新しいブロックのもの

	CompileTheBlock(0);					//0 はダミー（主ブロックの関数名はない）

	finalSource();
	int i = errorN();				//エラーメッセージの個数
	if (i!=0){printf("%d errors\n", i);}
	/*	listCode();	*/			//目的コードのリスト（必要なら）
	return i<MINERROR;		//エラーメッセージの個数が少ないかどうかの判定
}

void CompileTheBlock(int pIndex)		//pIndex はこのブロックの関数名のインデックス
{
	int backP;
	backP = genCodeV(jmp, 0);		//内部関数を飛び越す命令、後でバックパッチ
	while (1) 
	{				//宣言部のコンパイルを繰り返す
		switch (s_Token.kind)
		{
		case Const:
			{
				//定数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				constDecl(); 
				continue;
			}
		case Var:
			{//変数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				varDecl(); 
				continue;
			}
		case Func:
			{//関数宣言部のコンパイル
				s_Token = ProgressAndGetNextToken();
				funcDecl(); 
				continue;
			}
		default:
			{//それ以外なら宣言部は終わり
				break;
			}
		}
		break;
	}			
	backPatch(backP);			//内部関数を飛び越す命令にパッチ
	changeV(pIndex, nextCode());	//この関数の開始番地を修正
	genCodeV(ict, frameL());		//このブロックの実行時の必要記憶域をとる命令
	CompileTheStatement();				//このブロックの主文		
	genCodeR();				//リターン命令
	blockEnd();				//ブロックが終ったことをtableに連絡
}	

void constDecl()			//定数宣言のコンパイル
{
	Token temp;
	while(1)
	{
		if (s_Token.kind==Id)
		{
			setIdKind(constId);				//印字のための情報のセット
			temp = s_Token; 					//名前を入れておく
			s_Token = checkGet(ProgressAndGetNextToken(), Equal);		//名前の次は"="のはず
			if (s_Token.kind==Num)
			{
				enterTconst(temp.u.id, s_Token.u.value);	//定数名と値をテーブルに
			}
			else
			{
				errorType("number");
			}
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			errorMissingId();
		}

		if (s_Token.kind!=Comma)
		{		//次がコンマなら定数宣言が続く
			if (s_Token.kind!=Id){break;}
			//次が名前ならコンマを忘れたことにする
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Semicolon);		//最後は";"のはず
}

void varDecl()				//変数宣言のコンパイル
{
	while(1)
	{
		if (s_Token.kind==Id)
		{
			setIdKind(varId);		//印字のための情報のセット
			enterTvar(s_Token.u.id);		//変数名をテーブルに、番地はtableが決める
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			errorMissingId();
		}

		if (s_Token.kind!=Comma)
		{		//次がコンマなら変数宣言が続く
			if (s_Token.kind!=Id){break;}

			//次が名前ならコンマを忘れたことにする
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Semicolon);		//最後は";"のはず
}

void funcDecl()			//関数宣言のコンパイル
{
	int iFuncIndex ;
	if (s_Token.kind!=Id)
	{
		errorMissingId();			//関数名がない
		return;
	}


	setIdKind(funcId);				//印字のための情報のセット
	iFuncIndex = enterTfunc(s_Token.u.id, nextCode());		//関数名をテーブルに登録
	//その先頭番地は、まず、次のコードの番地nextCode()とする
	s_Token = checkGet(ProgressAndGetNextToken(), Lparen);
	blockBegin(FIRSTADDR);	//パラメタ名のレベルは関数のブロックと同じ
	while(1)
	{
		if (s_Token.kind!=Id){break;}
		//パラメタ名がある場合
		setIdKind(parId);		//印字のための情報のセット
		enterTpar(s_Token.u.id);		//パラメタ名をテーブルに登録
		s_Token = ProgressAndGetNextToken();


		if (s_Token.kind!=Comma)
		{		//次がコンマならパラメタ名が続く
			if (s_Token.kind!=Id){break;}

			//次が名前ならコンマを忘れたことに
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Rparen);		//最後は")"のはず
	endpar();				//パラメタ部が終わったことをテーブルに連絡
	if (s_Token.kind==Semicolon)
	{
		errorDelete();
		s_Token = ProgressAndGetNextToken();
	}
	CompileTheBlock(iFuncIndex );	//ブロックのコンパイル、その関数名のインデックスを渡す
	s_Token = checkGet(s_Token, Semicolon);		//最後は";"のはず

}

void CompileTheStatement()			//文のコンパイル
{
	int tIndex;
	KindTable k;
	int backP, backP2;				//バックパッチ用

	while(1) 
	{
		switch (s_Token.kind) 
		{
		case Id:
			{//代入文のコンパイル
				tIndex = searchT(s_Token.u.id, varId);	//左辺の変数のインデックス
				setIdKind(k=kindT(tIndex));			//印字のための情報のセット
				if (k != varId && k != parId){errorType("var/par");} 		//変数名かパラメタ名のはず
					
				s_Token = checkGet(ProgressAndGetNextToken(), Assign);			//":="のはず
				CompileTheExpression();	
				genCodeT(sto, tIndex);				//左辺への代入命令
				return;
			}
		case If:
			{//if文のコンパイル
				s_Token = ProgressAndGetNextToken();
				CompileTheCondition();					//条件式のコンパイル
				s_Token = checkGet(s_Token, Then);		//"then"のはず
				backP = genCodeV(jpc, 0);			//jpc命令
				CompileTheStatement();					//文のコンパイル
				backPatch(backP);				//上のjpc命令にバックパッチ
				return;
			}
		case Ret:
			{//return文のコンパイル
				s_Token = ProgressAndGetNextToken();
				CompileTheExpression();	
				genCodeR();					//ret命令
				return;
			}
		case Begin:
			{
				//begin . . end文のコンパイル
				s_Token = ProgressAndGetNextToken();
				while(1)
				{
					CompileTheStatement();				//文のコンパイル
					while(1)
					{
						if (s_Token.kind==Semicolon)
						{		//次が";"なら文が続く
							s_Token = ProgressAndGetNextToken();
							break;
						}
						if (s_Token.kind==End)
						{			//次がendなら終り
							s_Token = ProgressAndGetNextToken();
							return;
						}
						if (IsStartWithBeginKey(s_Token)==TRUE)
						{		//次が文の先頭記号なら
							errorInsert(Semicolon);	//";"を忘れたことにする
							break;
						}
						errorDelete();	//それ以外ならエラーとして読み捨てる
						s_Token = ProgressAndGetNextToken();
					}
				}
			}
		case While:
			{//while文のコンパイル
				s_Token = ProgressAndGetNextToken();
				backP2 = nextCode();			//while文の最後のjmp命令の飛び先
				CompileTheCondition();				//条件式のコンパイル
				s_Token = checkGet(s_Token, Do);	//"do"のはず
				backP = genCodeV(jpc, 0);		//条件式が偽のとき飛び出すjpc命令
				CompileTheStatement();				//文のコンパイル
				genCodeV(jmp, backP2);		//while文の先頭へのジャンプ命令
				backPatch(backP);	//偽のとき飛び出すjpc命令へのバックパッチ
				return;
			}
		case Write:
			{			//write文のコンパイル
				s_Token = ProgressAndGetNextToken();
				CompileTheExpression();
				genCodeO(wrt);				//その値を出力するwrt命令
				return;
			}
		case WriteLn:
			{			//writeln文のコンパイル
				s_Token = ProgressAndGetNextToken();
				genCodeO(wrl);				//改行を出力するwrl命令
				return;
			}
		case End:			{return;}	
		case Semicolon:	{return;}		//空文を読んだことにして終り

		default:
			{//文の先頭のキーまで読み捨てる
				errorDelete();				//今読んだトークンを読み捨てる
				s_Token = ProgressAndGetNextToken();
				continue;
			}
		}		
	}
}

BOOL IsStartWithBeginKey(Token t)			//トークンtは文の先頭のキーか？
{
	switch (t.kind)
	{
	case If: {return TRUE;}
	case Begin: {return TRUE;}
	case Ret:{return TRUE;}
	case While:{return TRUE;}
	case Write: {return TRUE;}
	case WriteLn:{return TRUE;}
	default:{return FALSE;}
	}
}

void CompileTheExpression()
{
	KeyId k;
	k = s_Token.kind;
	if (k==Plus || k==Minus)
	{
		s_Token = ProgressAndGetNextToken();
		CompileTheTerm();
		if (k==Minus)
		{
			genCodeO(neg);
		}
	}
	else
	{
		CompileTheTerm();
	}

	k = s_Token.kind;
	while (k==Plus || k==Minus)
	{
		s_Token = ProgressAndGetNextToken();
		CompileTheTerm();
		if (k==Minus){genCodeO(sub);}
		else{genCodeO(add);}
		k = s_Token.kind;
	}
}

void CompileTheTerm()					//式の項のコンパイル
{
	KeyId k;
	CompileTheFactor();
	k = s_Token.kind;
	while (k==Mult || k==Div)
	{	
		s_Token = ProgressAndGetNextToken();
		CompileTheFactor();
		if (k==Mult){genCodeO(mul);}
		else{genCodeO(div_);}
		k = s_Token.kind;
	}
}

void CompileTheFactor()					//式の因子のコンパイル
{
	int tIndex, i;
	KeyId k;
	if (s_Token.kind==Id)
	{
		tIndex = searchT(s_Token.u.id, varId);
		k=static_cast<KeyId>(kindT(tIndex));
		setIdKind(kindT(tIndex));			//印字のための情報のセット
		switch (k) 
		{
		case varId:
			{//変数名かパラメタ名
				genCodeT(lod, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case parId:	
			{//変数名かパラメタ名
				genCodeT(lod, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case constId:		
			{			//定数名
				genCodeV(lit, val(tIndex));
				s_Token = ProgressAndGetNextToken(); break;
			}
		case funcId:
			{//関数呼び出し
				s_Token = ProgressAndGetNextToken();
				if (s_Token.kind==Lparen)
				{
					i=0; 					//iは実引数の個数
					s_Token = ProgressAndGetNextToken();
					if (s_Token.kind != Rparen) 
					{
						for (; ; ) 
						{
							CompileTheExpression(); //実引数のコンパイル
							i++;	
							if (s_Token.kind==Comma)
							{	/* 次がコンマなら実引数が続く */
								s_Token = ProgressAndGetNextToken();
								continue;
							}
							s_Token = checkGet(s_Token, Rparen);
							break;
						}
					} 
					else{s_Token = ProgressAndGetNextToken();}
					if (pars(tIndex) != i) {errorMessage("\\#par");}	//pars(tIndex)は仮引数の個数
				}
				else
				{
					errorInsert(Lparen);
					errorInsert(Rparen);
				}
				genCodeT(cal, tIndex);				//call命令
				break;
			}
		}
	}
	else if (s_Token.kind==Num)
	{			//定数
		genCodeV(lit, s_Token.u.value);
		s_Token = ProgressAndGetNextToken();
	}
	else if (s_Token.kind==Lparen)
	{			//「(」「因子」「)」
		s_Token = ProgressAndGetNextToken();
		CompileTheExpression();
		s_Token = checkGet(s_Token, Rparen);
	}

	switch (s_Token.kind)
	{					//因子の後がまた因子ならエラー
	case Id: 
		{
			errorMissingOp();
			CompileTheFactor();
			return;
		}
	case Num: 
		{
			errorMissingOp();
			CompileTheFactor();
			return;
		}
	case Lparen:
		{
			errorMissingOp();
			CompileTheFactor();
			return;
		}
	default:
		{
			return;
		}
	}	
}

void CompileTheCondition()					//条件式のコンパイル
{
	KeyId k;
	if (s_Token.kind==Odd)
	{
		s_Token = ProgressAndGetNextToken();
		CompileTheExpression();
		genCodeO(odd);
		return;
	}

	CompileTheExpression();
	k = s_Token.kind;
	switch(k)
	{
	case Equal: {break;}
	case Lss: {break;}
	case Gtr:{break;}
	case NotEq: {break;}
	case LssEq:{break;}
	case GtrEq:{break;}

	default:
		{
			errorType("rel-op");
			break;
		}
	}

	s_Token = ProgressAndGetNextToken();
	CompileTheExpression();
	switch(k)
	{
	case Equal:	{genCodeO(eq); break;}
	case Lss:	{genCodeO(ls); break;}
	case Gtr:	{genCodeO(gr); break;}
	case NotEq:	{genCodeO(neq); break;}
	case LssEq:	{genCodeO(lseq); break;}
	case GtrEq:	{genCodeO(greq); break;}
	}
}

