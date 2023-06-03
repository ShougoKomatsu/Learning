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

static Token s_Token;				//���̃g�[�N�������Ă���

static void CompileTheBlock(int pIndex);	//�u���b�N�̃R���p�C��
//pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X
static void constDecl();			//�萔�錾�̃R���p�C��
static void varDecl();				//�ϐ��錾�̃R���p�C��
static void funcDecl();			//�֐��錾�̃R���p�C��

static BOOL CompileTheStatement();			//���̃R���p�C��
static BOOL CompileTheExpression();			//���̃R���p�C��
static BOOL CompileTheTerm();				//���̍��̃R���p�C��
static BOOL CompileTheFactor();				//���̈��q�̃R���p�C��
static BOOL CompileTheCondition();			//�������̃R���p�C��
static BOOL IsStartWithBeginKey(Token t);		//�g�[�N��t�͕��̐擪�̃L�[���H

int compile()
{
	printf("start compilation\n");
	initSource();				//getSource�̏����ݒ�
	s_Token = ProgressAndGetNextToken();			//�ŏ��̃g�[�N��
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
		switch (s_Token.kind)
		{
		case Const:
			{
				//�萔�錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				constDecl(); 
				continue;
			}
		case Var:
			{//�ϐ��錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				varDecl(); 
				continue;
			}
		case Func:
			{//�֐��錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
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
		if (s_Token.kind==Id)
		{
			setIdKind(constId);				//�󎚂̂��߂̏��̃Z�b�g
			temp = s_Token; 					//���O�����Ă���
			s_Token = checkGet(ProgressAndGetNextToken(), Equal);		//���O�̎���"="�̂͂�
			if (s_Token.kind==Num)
			{
				enterTconst(temp.u.id, s_Token.u.value);	//�萔���ƒl���e�[�u����
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
		{		//�����R���}�Ȃ�萔�錾������
			if (s_Token.kind!=Id){break;}
			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Semicolon);		//�Ō��";"�̂͂�
}

void varDecl()				//�ϐ��錾�̃R���p�C��
{
	while(1)
	{
		if (s_Token.kind==Id)
		{
			setIdKind(varId);		//�󎚂̂��߂̏��̃Z�b�g
			enterTvar(s_Token.u.id);		//�ϐ������e�[�u���ɁA�Ԓn��table�����߂�
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			errorMissingId();
		}

		if (s_Token.kind!=Comma)
		{		//�����R���}�Ȃ�ϐ��錾������
			if (s_Token.kind!=Id){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Semicolon);		//�Ō��";"�̂͂�
}

void funcDecl()			//�֐��錾�̃R���p�C��
{
	int iFuncIndex ;
	if (s_Token.kind!=Id)
	{
		errorMissingId();			//�֐������Ȃ�
		return;
	}


	setIdKind(funcId);				//�󎚂̂��߂̏��̃Z�b�g
	iFuncIndex = enterTfunc(s_Token.u.id, nextCode());		//�֐������e�[�u���ɓo�^
	//���̐擪�Ԓn�́A�܂��A���̃R�[�h�̔ԒnnextCode()�Ƃ���
	s_Token = checkGet(ProgressAndGetNextToken(), Lparen);
	blockBegin(FIRSTADDR);	//�p�����^���̃��x���͊֐��̃u���b�N�Ɠ���
	while(1)
	{
		if (s_Token.kind!=Id){break;}
		//�p�����^��������ꍇ
		setIdKind(parId);		//�󎚂̂��߂̏��̃Z�b�g
		enterTpar(s_Token.u.id);		//�p�����^�����e�[�u���ɓo�^
		s_Token = ProgressAndGetNextToken();


		if (s_Token.kind!=Comma)
		{		//�����R���}�Ȃ�p�����^��������
			if (s_Token.kind!=Id){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃ�
			errorInsert(Comma);
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = checkGet(s_Token, Rparen);		//�Ō��")"�̂͂�
	endpar();				//�p�����^�����I��������Ƃ��e�[�u���ɘA��
	if (s_Token.kind==Semicolon)
	{
		errorDelete();
		s_Token = ProgressAndGetNextToken();
	}
	CompileTheBlock(iFuncIndex );	//�u���b�N�̃R���p�C���A���̊֐����̃C���f�b�N�X��n��
	s_Token = checkGet(s_Token, Semicolon);		//�Ō��";"�̂͂�

}

BOOL CompileTheStatement()			//���̃R���p�C��
{
	int tIndex;
	KindTable k;
	int backP, backP2;				//�o�b�N�p�b�`�p
				BOOL bRet;

	while(1) 
	{
		switch (s_Token.kind) 
		{
		case Id:
			{//������̃R���p�C��
				tIndex = searchT(s_Token.u.id, varId);	//���ӂ̕ϐ��̃C���f�b�N�X
				setIdKind(k=GetKind(tIndex));			//�󎚂̂��߂̏��̃Z�b�g
				if (k != varId && k != parId){errorType("var/par");} 		//�ϐ������p�����^���̂͂�

				s_Token = checkGet(ProgressAndGetNextToken(), Assign);			//":="�̂͂�
				bRet = CompileTheExpression();
				if(bRet != TRUE){return FALSE;}

				genCodeT(sto, tIndex);				//���ӂւ̑������
				return TRUE;
			}
		case If:
			{//if���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				CompileTheCondition();					//�������̃R���p�C��
				s_Token = checkGet(s_Token, Then);		//"then"�̂͂�
				backP = genCodeV(jpc, 0);			//jpc����
				bRet = CompileTheStatement();					//���̃R���p�C��
				if(bRet != TRUE){return FALSE;}
				backPatch(backP);				//���jpc���߂Ƀo�b�N�p�b�`
				return TRUE;
			}
		case Ret:
			{//return���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				bRet = CompileTheExpression();
				if(bRet != TRUE){return FALSE;}
				genCodeR();					//ret����
				return TRUE;
			}
		case Begin:
			{
				//begin . . end���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				while(1)
				{
					bRet = CompileTheStatement();				//���̃R���p�C��
					if(bRet != TRUE){return FALSE;}
					while(1)
					{
						if (s_Token.kind==Semicolon)
						{		//����";"�Ȃ當������
							s_Token = ProgressAndGetNextToken();
							break;
						}
						if (s_Token.kind==End)
						{			//����end�Ȃ�I��
							s_Token = ProgressAndGetNextToken();
							return TRUE;
						}
						if (IsStartWithBeginKey(s_Token)==TRUE)
						{		//�������̐擪�L���Ȃ�
							errorInsert(Semicolon);	//";"��Y�ꂽ���Ƃɂ���
							break;
						}
						errorDelete();	//����ȊO�Ȃ�G���[�Ƃ��ēǂݎ̂Ă�
						s_Token = ProgressAndGetNextToken();
					}
				}
			}
		case While:
			{//while���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				backP2 = nextCode();			//while���̍Ō��jmp���߂̔�ѐ�
				CompileTheCondition();				//�������̃R���p�C��
				s_Token = checkGet(s_Token, Do);	//"do"�̂͂�
				backP = genCodeV(jpc, 0);		//���������U�̂Ƃ���яo��jpc����
				bRet = CompileTheStatement();				//���̃R���p�C��
				if(bRet != TRUE){return FALSE;}
				genCodeV(jmp, backP2);		//while���̐擪�ւ̃W�����v����
				backPatch(backP);	//�U�̂Ƃ���яo��jpc���߂ւ̃o�b�N�p�b�`
				return TRUE;
			}
		case Write:
			{			//write���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				bRet = CompileTheExpression();
				if(bRet != TRUE){return FALSE;}
				genCodeO(wrt);				//���̒l���o�͂���wrt����
				return TRUE;
			}
		case WriteLn:
			{			//writeln���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				genCodeO(wrl);				//���s���o�͂���wrl����
				return TRUE;
			}
		case End:			{return TRUE;}	
		case Semicolon:	{return TRUE;}		//�󕶂�ǂ񂾂��Ƃɂ��ďI��

		default:
			{//���̐擪�̃L�[�܂œǂݎ̂Ă�
				errorDelete();				//���ǂ񂾃g�[�N����ǂݎ̂Ă�
				s_Token = ProgressAndGetNextToken();
				continue;
			}
		}		
	}
	return TRUE;
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

BOOL CompileTheExpression()
{
	KeyId k;
	k = s_Token.kind;
	if (k==Plus || k==Minus)
	{
		s_Token = ProgressAndGetNextToken();
		BOOL bRet;
		bRet = CompileTheTerm();
		if(bRet != TRUE){return FALSE;}

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
	return TRUE;
}

BOOL CompileTheTerm()					//���̍��̃R���p�C��
{
	KeyId k;
	BOOL bRet;
	bRet = CompileTheFactor();
	if(bRet != TRUE){exit(1);}

	k = s_Token.kind;
	while (k==Mult || k==Div)
	{	
		s_Token = ProgressAndGetNextToken();
		bRet = CompileTheFactor();
		if(bRet != TRUE){return FALSE;}

		if (k==Mult){genCodeO(mul);}
		else{genCodeO(div_);}
		k = s_Token.kind;
	}
	return TRUE;
}

BOOL CompileTheFactor()					//���̈��q�̃R���p�C��
{
	int tIndex, i;
	KeyId k;
	if (s_Token.kind==Id)
	{
		tIndex = searchT(s_Token.u.id, varId);
		k=static_cast<KeyId>(GetKind(tIndex));
		setIdKind(GetKind(tIndex));			//�󎚂̂��߂̏��̃Z�b�g
		switch (k) 
		{
		case varId:
			{//�ϐ������p�����^��
				genCodeT(lod, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case parId:	
			{//�ϐ������p�����^��
				genCodeT(lod, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case constId:		
			{			//�萔��
				genCodeV(lit, val(tIndex));
				s_Token = ProgressAndGetNextToken(); break;
			}
		case funcId:
			{//�֐��Ăяo��
				s_Token = ProgressAndGetNextToken();
				if (s_Token.kind==Lparen)
				{
					i=0; 					//i�͎������̌�
					s_Token = ProgressAndGetNextToken();
					if (s_Token.kind != Rparen) 
					{
						for (; ; ) 
						{
							CompileTheExpression(); //�������̃R���p�C��
							i++;	
							if (s_Token.kind==Comma)
							{	/* �����R���}�Ȃ������������ */
								s_Token = ProgressAndGetNextToken();
								continue;
							}
							s_Token = checkGet(s_Token, Rparen);
							break;
						}
					} 
					else{s_Token = ProgressAndGetNextToken();}
					if (pars(tIndex) != i)
					{
						BOOL bRet;
						bRet = OutputErrMessage("\\#par");
						if(bRet != TRUE){return FALSE;}

					}	//pars(tIndex)�͉������̌�
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
	else if (s_Token.kind==Num)
	{			//�萔
		genCodeV(lit, s_Token.u.value);
		s_Token = ProgressAndGetNextToken();
	}
	else if (s_Token.kind==Lparen)
	{			//�u(�v�u���q�v�u)�v
		s_Token = ProgressAndGetNextToken();
		CompileTheExpression();
		s_Token = checkGet(s_Token, Rparen);
	}

	switch (s_Token.kind)
	{					//���q�̌オ�܂����q�Ȃ�G���[
	case Id: 
		{
			errorMissingOp();
			BOOL bRet;
			bRet = CompileTheFactor();
			if(bRet != TRUE){return FALSE;}

			return TRUE;
		}
	case Num: 
		{
			errorMissingOp();
			BOOL bRet;
			bRet = CompileTheFactor();
			if(bRet != TRUE){return FALSE;}

			return TRUE;
		}
	case Lparen:
		{
			errorMissingOp();
			BOOL bRet;
			bRet = CompileTheFactor();
			if(bRet != TRUE){return FALSE;}

			return TRUE;
		}
	default:
		{
			return TRUE;
		}
	}	
	return FALSE;
}

BOOL CompileTheCondition()					//�������̃R���p�C��
{
	KeyId k;
	BOOL bRet;
	if (s_Token.kind==Odd)
	{
		s_Token = ProgressAndGetNextToken();
		bRet = CompileTheExpression();
		if(bRet!=TRUE){return FALSE;}
		genCodeO(odd);
		return TRUE;
	}

	bRet = CompileTheExpression();
		if(bRet!=TRUE){return FALSE;}

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
	bRet = CompileTheExpression();
		if(bRet!=TRUE){return FALSE;}

		switch(k)
	{
	case Equal:	{genCodeO(eq); break;}
	case Lss:	{genCodeO(ls); break;}
	case Gtr:	{genCodeO(gr); break;}
	case NotEq:	{genCodeO(neq); break;}
	case LssEq:	{genCodeO(lseq); break;}
	case GtrEq:	{genCodeO(greq); break;}
	}
	return TRUE;
}

