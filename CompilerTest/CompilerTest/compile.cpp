#include "stdafx.h"
/*************** compile.c *************/

#include "getSource.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3			//�G���[������ȉ��Ȃ���s
#define FIRSTADDR 2			//�e�u���b�N�̍ŏ��̕ϐ��̃A�h���X

static Token token;				//���̃g�[�N�������Ă���

static void CompileTheBlock(int pIndex);	//�u���b�N�̃R���p�C��
//pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X
static void constDecl();			//�萔�錾�̃R���p�C��
static void varDecl();				//�ϐ��錾�̃R���p�C��
static void funcDecl();			//�֐��錾�̃R���p�C��

static void CompileTheStatement();			//���̃R���p�C��
static void CompileTheExpression();			//���̃R���p�C��
static void CompileTheTerm();				//���̍��̃R���p�C��
static void CompileTheFactor();				//���̈��q�̃R���p�C��
static void CompileTheCondition();			//�������̃R���p�C��
static BOOL IsStartWithBeginKey(Token t);		//�g�[�N��t�͕��̐擪�̃L�[���H

int compile()
{
	printf("start compilation\n");
	initSource();				//getSource�̏����ݒ�
	token = ProgressAndGetNextToken();			//�ŏ��̃g�[�N��
	blockBegin(FIRSTADDR);		//����Ȍ�̐錾�͐V�����u���b�N�̂���

	CompileTheBlock(0);					//0 �̓_�~�[�i��u���b�N�̊֐����͂Ȃ��j

	finalSource();
	int i = errorN();				//�G���[���b�Z�[�W�̌�
	if (i!=0){printf("%d errors\n", i);}
	/*	listCode();	*/			//�ړI�R�[�h�̃��X�g�i�K�v�Ȃ�j
	return i<MINERROR;		//�G���[���b�Z�[�W�̌������Ȃ����ǂ����̔���
}

void CompileTheBlock(int pIndex)		//pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X
{
	int backP;
	backP = genCodeV(jmp, 0);		//�����֐����щz�����߁A��Ńo�b�N�p�b�`
	while (1) 
	{				//�錾���̃R���p�C�����J��Ԃ�
		switch (token.kind)
		{
		case Const:
			{
				//�萔�錾���̃R���p�C��
				token = ProgressAndGetNextToken();
				constDecl(); 
				continue;
			}
		case Var:
			{//�ϐ��錾���̃R���p�C��
				token = ProgressAndGetNextToken();
				varDecl(); 
				continue;
			}
		case Func:
			{//�֐��錾���̃R���p�C��
				token = ProgressAndGetNextToken();
				funcDecl(); 
				continue;
			}
		default:
			{//����ȊO�Ȃ�錾���͏I���
				break;
			}
		}
		break;
	}			
	backPatch(backP);			//�����֐����щz�����߂Ƀp�b�`
	changeV(pIndex, nextCode());	//���̊֐��̊J�n�Ԓn���C��
	genCodeV(ict, frameL());		//���̃u���b�N�̎��s���̕K�v�L������Ƃ閽��
	CompileTheStatement();				//���̃u���b�N�̎啶		
	genCodeR();				//���^�[������
	blockEnd();				//�u���b�N���I�������Ƃ�table�ɘA��
}	

void constDecl()			//�萔�錾�̃R���p�C��
{
	Token temp;
	while(1)
	{
		if (token.kind==Id)
		{
			setIdKind(constId);				//�󎚂̂��߂̏��̃Z�b�g
			temp = token; 					//���O�����Ă���
			token = checkGet(ProgressAndGetNextToken(), Equal);		//���O�̎���"="�̂͂�
			if (token.kind==Num)
			{
				enterTconst(temp.u.id, token.u.value);	//�萔���ƒl���e�[�u����
			}
			else
			{
				errorType("number");
			}
			token = ProgressAndGetNextToken();
		}
		else
		{
			errorMissingId();
		}

		if (token.kind!=Comma)
		{		//�����R���}�Ȃ�萔�錾������
			if (token.kind!=Id){break;}
			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			errorInsert(Comma);
			continue;
		}
		token = ProgressAndGetNextToken();
	}
	token = checkGet(token, Semicolon);		//�Ō��";"�̂͂�
}

void varDecl()				//�ϐ��錾�̃R���p�C��
{
	while(1)
	{
		if (token.kind==Id)
		{
			setIdKind(varId);		//�󎚂̂��߂̏��̃Z�b�g
			enterTvar(token.u.id);		//�ϐ������e�[�u���ɁA�Ԓn��table�����߂�
			token = ProgressAndGetNextToken();
		}
		else
		{
			errorMissingId();
		}

		if (token.kind!=Comma)
		{		//�����R���}�Ȃ�ϐ��錾������
			if (token.kind!=Id){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			errorInsert(Comma);
			continue;
		}
		token = ProgressAndGetNextToken();
	}
	token = checkGet(token, Semicolon);		//�Ō��";"�̂͂�
}

void funcDecl()			//�֐��錾�̃R���p�C��
{
	int iFuncIndex ;
	if (token.kind!=Id)
	{
		errorMissingId();			//�֐������Ȃ�
		return;
	}


	setIdKind(funcId);				//�󎚂̂��߂̏��̃Z�b�g
	iFuncIndex = enterTfunc(token.u.id, nextCode());		//�֐������e�[�u���ɓo�^
	//���̐擪�Ԓn�́A�܂��A���̃R�[�h�̔ԒnnextCode()�Ƃ���
	token = checkGet(ProgressAndGetNextToken(), Lparen);
	blockBegin(FIRSTADDR);	//�p�����^���̃��x���͊֐��̃u���b�N�Ɠ���
	while(1)
	{
		if (token.kind!=Id){break;}
		//�p�����^��������ꍇ
		setIdKind(parId);		//�󎚂̂��߂̏��̃Z�b�g
		enterTpar(token.u.id);		//�p�����^�����e�[�u���ɓo�^
		token = ProgressAndGetNextToken();


		if (token.kind!=Comma)
		{		//�����R���}�Ȃ�p�����^��������
			if (token.kind!=Id){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃ�
			errorInsert(Comma);
			continue;
		}
		token = ProgressAndGetNextToken();
	}
	token = checkGet(token, Rparen);		//�Ō��")"�̂͂�
	endpar();				//�p�����^�����I��������Ƃ��e�[�u���ɘA��
	if (token.kind==Semicolon)
	{
		errorDelete();
		token = ProgressAndGetNextToken();
	}
	CompileTheBlock(iFuncIndex );	//�u���b�N�̃R���p�C���A���̊֐����̃C���f�b�N�X��n��
	token = checkGet(token, Semicolon);		//�Ō��";"�̂͂�

}

void CompileTheStatement()			//���̃R���p�C��
{
	int tIndex;
	KindTable k;
	int backP, backP2;				//�o�b�N�p�b�`�p

	while(1) 
	{
		switch (token.kind) 
		{
		case Id:
			{//������̃R���p�C��
				tIndex = searchT(token.u.id, varId);	//���ӂ̕ϐ��̃C���f�b�N�X
				setIdKind(k=kindT(tIndex));			//�󎚂̂��߂̏��̃Z�b�g
				if (k != varId && k != parId){errorType("var/par");} 		//�ϐ������p�����^���̂͂�
					
				token = checkGet(ProgressAndGetNextToken(), Assign);			//":="�̂͂�
				CompileTheExpression();	
				genCodeT(sto, tIndex);				//���ӂւ̑������
				return;
			}
		case If:
			{//if���̃R���p�C��
				token = ProgressAndGetNextToken();
				CompileTheCondition();					//�������̃R���p�C��
				token = checkGet(token, Then);		//"then"�̂͂�
				backP = genCodeV(jpc, 0);			//jpc����
				CompileTheStatement();					//���̃R���p�C��
				backPatch(backP);				//���jpc���߂Ƀo�b�N�p�b�`
				return;
			}
		case Ret:
			{//return���̃R���p�C��
				token = ProgressAndGetNextToken();
				CompileTheExpression();	
				genCodeR();					//ret����
				return;
			}
		case Begin:
			{
				//begin . . end���̃R���p�C��
				token = ProgressAndGetNextToken();
				while(1)
				{
					CompileTheStatement();				//���̃R���p�C��
					while(1)
					{
						if (token.kind==Semicolon)
						{		//����";"�Ȃ當������
							token = ProgressAndGetNextToken();
							break;
						}
						if (token.kind==End)
						{			//����end�Ȃ�I��
							token = ProgressAndGetNextToken();
							return;
						}
						if (IsStartWithBeginKey(token)==TRUE)
						{		//�������̐擪�L���Ȃ�
							errorInsert(Semicolon);	//";"��Y�ꂽ���Ƃɂ���
							break;
						}
						errorDelete();	//����ȊO�Ȃ�G���[�Ƃ��ēǂݎ̂Ă�
						token = ProgressAndGetNextToken();
					}
				}
			}
		case While:
			{//while���̃R���p�C��
				token = ProgressAndGetNextToken();
				backP2 = nextCode();			//while���̍Ō��jmp���߂̔�ѐ�
				CompileTheCondition();				//�������̃R���p�C��
				token = checkGet(token, Do);	//"do"�̂͂�
				backP = genCodeV(jpc, 0);		//���������U�̂Ƃ���яo��jpc����
				CompileTheStatement();				//���̃R���p�C��
				genCodeV(jmp, backP2);		//while���̐擪�ւ̃W�����v����
				backPatch(backP);	//�U�̂Ƃ���яo��jpc���߂ւ̃o�b�N�p�b�`
				return;
			}
		case Write:
			{			//write���̃R���p�C��
				token = ProgressAndGetNextToken();
				CompileTheExpression();
				genCodeO(wrt);				//���̒l���o�͂���wrt����
				return;
			}
		case WriteLn:
			{			//writeln���̃R���p�C��
				token = ProgressAndGetNextToken();
				genCodeO(wrl);				//���s���o�͂���wrl����
				return;
			}
		case End:			{return;}	
		case Semicolon:	{return;}		//�󕶂�ǂ񂾂��Ƃɂ��ďI��

		default:
			{//���̐擪�̃L�[�܂œǂݎ̂Ă�
				errorDelete();				//���ǂ񂾃g�[�N����ǂݎ̂Ă�
				token = ProgressAndGetNextToken();
				continue;
			}
		}		
	}
}

BOOL IsStartWithBeginKey(Token t)			//�g�[�N��t�͕��̐擪�̃L�[���H
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
	k = token.kind;
	if (k==Plus || k==Minus)
	{
		token = ProgressAndGetNextToken();
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

	k = token.kind;
	while (k==Plus || k==Minus)
	{
		token = ProgressAndGetNextToken();
		CompileTheTerm();
		if (k==Minus){genCodeO(sub);}
		else{genCodeO(add);}
		k = token.kind;
	}
}

void CompileTheTerm()					//���̍��̃R���p�C��
{
	KeyId k;
	CompileTheFactor();
	k = token.kind;
	while (k==Mult || k==Div)
	{	
		token = ProgressAndGetNextToken();
		CompileTheFactor();
		if (k==Mult){genCodeO(mul);}
		else{genCodeO(div_);}
		k = token.kind;
	}
}

void CompileTheFactor()					//���̈��q�̃R���p�C��
{
	int tIndex, i;
	KeyId k;
	if (token.kind==Id)
	{
		tIndex = searchT(token.u.id, varId);
		k=static_cast<KeyId>(kindT(tIndex));
		setIdKind(kindT(tIndex));			//�󎚂̂��߂̏��̃Z�b�g
		switch (k) 
		{
		case varId:
			{//�ϐ������p�����^��
				genCodeT(lod, tIndex);
				token = ProgressAndGetNextToken(); break;
			}
		case parId:	
			{//�ϐ������p�����^��
				genCodeT(lod, tIndex);
				token = ProgressAndGetNextToken(); break;
			}
		case constId:		
			{			//�萔��
				genCodeV(lit, val(tIndex));
				token = ProgressAndGetNextToken(); break;
			}
		case funcId:
			{//�֐��Ăяo��
				token = ProgressAndGetNextToken();
				if (token.kind==Lparen)
				{
					i=0; 					//i�͎������̌�
					token = ProgressAndGetNextToken();
					if (token.kind != Rparen) 
					{
						for (; ; ) 
						{
							CompileTheExpression(); //�������̃R���p�C��
							i++;	
							if (token.kind==Comma)
							{	/* �����R���}�Ȃ������������ */
								token = ProgressAndGetNextToken();
								continue;
							}
							token = checkGet(token, Rparen);
							break;
						}
					} 
					else{token = ProgressAndGetNextToken();}
					if (pars(tIndex) != i) {errorMessage("\\#par");}	//pars(tIndex)�͉������̌�
				}
				else
				{
					errorInsert(Lparen);
					errorInsert(Rparen);
				}
				genCodeT(cal, tIndex);				//call����
				break;
			}
		}
	}
	else if (token.kind==Num)
	{			//�萔
		genCodeV(lit, token.u.value);
		token = ProgressAndGetNextToken();
	}
	else if (token.kind==Lparen)
	{			//�u(�v�u���q�v�u)�v
		token = ProgressAndGetNextToken();
		CompileTheExpression();
		token = checkGet(token, Rparen);
	}

	switch (token.kind)
	{					//���q�̌オ�܂����q�Ȃ�G���[
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

void CompileTheCondition()					//�������̃R���p�C��
{
	KeyId k;
	if (token.kind==Odd)
	{
		token = ProgressAndGetNextToken();
		CompileTheExpression();
		genCodeO(odd);
		return;
	}

	CompileTheExpression();
	k = token.kind;
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

	token = ProgressAndGetNextToken();
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

