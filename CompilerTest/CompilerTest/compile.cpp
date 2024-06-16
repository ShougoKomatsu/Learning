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

static int CompileTheBlock(int pIndex);	//�u���b�N�̃R���p�C��
//pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X
static int constDecl();			//�萔�錾�̃R���p�C��
static int varDecl();				//�ϐ��錾�̃R���p�C��
static int funcDecl();			//�֐��錾�̃R���p�C��

static int CompileTheStatement();			//���̃R���p�C��
static int CompileTheExpression();			//���̃R���p�C��
static int CompileTheTerm();				//���̍��̃R���p�C��
static int CompileTheFactor();				//���̈��q�̃R���p�C��
static int CompileTheCondition();			//�������̃R���p�C��
static int IsStartWithBeginKey(Token t);		//�g�[�N��t�͕��̐擪�̃L�[���H

int compile()
{
	printf("start compilation\n");
	initSource();				//getSource�̏����ݒ�
	s_Token = ProgressAndGetNextToken();			//�ŏ��̃g�[�N��
	TreatBlockBegin(FIRSTADDR);		//����Ȍ�̐錾�͐V�����u���b�N�̂���
	int iRet;
	iRet = CompileTheBlock(0);					//0 �̓_�~�[�i��u���b�N�̊֐����͂Ȃ��j
	if(iRet < 0){return iRet;}

	finalSource();
	int i = errorN();				//�G���[���b�Z�[�W�̌�
	if (i != 0){printf("%d errors\n", i);}
	/*	listCode();	*/			//�ړI�R�[�h�̃��X�g�i�K�v�Ȃ�j
	return i<MINERROR;		//�G���[���b�Z�[�W�̌������Ȃ����ǂ����̔���
}

int CompileTheBlock(int pIndex)		//pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X
{
	int iRet;
	int intBackPatchNum;
	intBackPatchNum = genCodeV(OPERATION_CODE_JMP, 0);		//�����֐����щz�����߁A��Ńo�b�N�p�b�`
	while (1) 
	{				//�錾���̃R���p�C�����J��Ԃ�
		switch (s_Token.m_iKind)
		{
		case KIND_CONST:
			{
				//�萔�錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				iRet = constDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		case KIND_VAR:
			{//�ϐ��錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				iRet = varDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		case KIND_FUNC:
			{//�֐��錾���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				iRet = funcDecl(); 
				if(iRet < 0){return iRet;}

				continue;
			}
		default:
			{//����ȊO�Ȃ�錾���͏I���
				break;
			}
		}
		break;
	}			
	backPatch(intBackPatchNum);			//�����֐����щz�����߂Ƀp�b�`
	changeV(pIndex, GetNextCodeIndex());	//���̊֐��̊J�n�Ԓn���C��
	genCodeV(OPERATION_CODE_ICT, frameL());		//���̃u���b�N�̎��s���̕K�v�L������Ƃ閽��

	iRet = CompileTheStatement();				//���̃u���b�N�̎啶		
	if(iRet != TRUE){return iRet;}

	genCodeR();				//���^�[������
	TreatBlockEnd();				//�u���b�N���I�������Ƃ�table�ɘA��
	return 0;
}	

int constDecl()			//�萔�錾�̃R���p�C��
{
	Token temp;
	int iRet;
	while(1)
	{
		if (s_Token.m_iKind == KIND_ID)
		{
			setIdKind(KIND_constId);				//�󎚂̂��߂̏��̃Z�b�g
			temp = s_Token; 					//���O�����Ă���
			s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_EQUAL);		//���O�̎���"="�̂͂�
			if (s_Token.m_iKind == KIND_NUM)
			{
				RegisterConstName(temp.u.szIdentifier, s_Token.u.value);	//�萔���ƒl���e�[�u����
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
		{		//�����R���}�Ȃ�萔�錾������
			if (s_Token.m_iKind != KIND_ID){break;}
			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet < 0){return iRet;}
			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//�Ō��");"�̂͂�
	return 0;
}

int varDecl()				//�ϐ��錾�̃R���p�C��
{
	int iRet;
	while(1)
	{
		if (s_Token.m_iKind == KIND_ID)
		{
			setIdKind(KIND_varId);		//�󎚂̂��߂̏��̃Z�b�g
			RegisterVarName(s_Token.u.szIdentifier);		//�ϐ������e�[�u���ɁA�Ԓn��table�����߂�
			s_Token = ProgressAndGetNextToken();
		}
		else
		{
			iRet = OutputErrorMissingID();
			if(iRet < 0 ){return iRet;}
		}

		if (s_Token.m_iKind != KIND_COMMA)
		{		//�����R���}�Ȃ�ϐ��錾������
			if (s_Token.m_iKind != KIND_ID){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet <0){return iRet;}
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//�Ō��");"�̂͂�
	return 0;
}

int funcDecl()			//�֐��錾�̃R���p�C��
{
	int iRet;
	int iFuncIndex ;
	if (s_Token.m_iKind != KIND_ID)
	{
		iRet = OutputErrorMissingID();			//�֐������Ȃ�
		if(iRet < 0){return iRet;}
		return 0;
	}


	setIdKind(KIND_funcId);				//�󎚂̂��߂̏��̃Z�b�g
	iFuncIndex = RegisterFunction(s_Token.u.szIdentifier, GetNextCodeIndex());		//�֐������e�[�u���ɓo�^
	//���̐擪�Ԓn�́A�܂��A���̃R�[�h�̔ԒnGetNextCodeIndex()�Ƃ���
	s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_PARENTHESIS_L);
	TreatBlockBegin(FIRSTADDR);	//�p�����^���̃��x���͊֐��̃u���b�N�Ɠ���
	while(1)
	{
		if (s_Token.m_iKind != KIND_ID){break;}
		//�p�����^��������ꍇ
		setIdKind(KIND_parId);		//�󎚂̂��߂̏��̃Z�b�g
		RegisterParameterName(s_Token.u.szIdentifier);		//�p�����^�����e�[�u���ɓo�^
		s_Token = ProgressAndGetNextToken();


		if (s_Token.m_iKind != KIND_COMMA)
		{		//�����R���}�Ȃ�p�����^��������
			if (s_Token.m_iKind != KIND_ID){break;}

			//�������O�Ȃ�R���}��Y�ꂽ���Ƃ�
			iRet = OutputErrorInsert(KIND_COMMA);
			if(iRet <0){return iRet;}

			continue;
		}
		s_Token = ProgressAndGetNextToken();
	}
	s_Token = GetTokenWithCheck(s_Token, KIND_PARENTHESIS_R);		//�Ō��")"�̂͂�
	TreatParameterEnd();				//�p�����^�����I��������Ƃ��e�[�u���ɘA��
	if (s_Token.m_iKind == KIND_SEMICOLON)
	{
		iRet = OutputErrorDelete();
		if(iRet < 0){return iRet;}
		s_Token = ProgressAndGetNextToken();
	}
	iRet = CompileTheBlock(iFuncIndex );	//�u���b�N�̃R���p�C���A���̊֐����̃C���f�b�N�X��n��
	if(iRet < 0){return iRet;}

	s_Token = GetTokenWithCheck(s_Token, KIND_SEMICOLON);		//�Ō��");"�̂͂�
	return 0;
}

int CompileTheStatement()			//���̃R���p�C��
{
	int tIndex;
	int iKind;
	int backP, backP2;				//�o�b�N�p�b�`�p
	int iRet;

	while(1) 
	{
		switch (s_Token.m_iKind) 
		{
		case KIND_ID:
			{//������̃R���p�C��
				tIndex = GetNameIndex(s_Token.u.szIdentifier, KIND_varId);	//���ӂ̕ϐ��̃C���f�b�N�X

				iKind=GetKind(tIndex);
				setIdKind(iKind);			//�󎚂̂��߂̏��̃Z�b�g

				if (iKind == KIND_varId ){}
				else if(iKind == KIND_parId){}
				else
				{
					iRet = OutputErrorType(_T("var/par"));
					if(iRet < 0){return iRet;}
				} 		

				s_Token = GetTokenWithCheck(ProgressAndGetNextToken(), KIND_ASSIGN);			//":="�̂͂�
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}

				genCodeT(OPERATION_CODE_STO, tIndex);				//���ӂւ̑������
				return 0;
			}
		case KIND_IF:
			{//if���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				CompileTheCondition();					//�������̃R���p�C��
				s_Token = GetTokenWithCheck(s_Token, KIND_THEN);		//"then"�̂͂�
				backP = genCodeV(OPERATION_CODE_JPC, 0);			//jpc����
				iRet = CompileTheStatement();					//���̃R���p�C��
				if(iRet < 0){return iRet;}
				backPatch(backP);				//���jpc���߂Ƀo�b�N�p�b�`
				return TRUE;
			}
		case KIND_RET:
			{//return���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}
				genCodeR();					//ret����
				return TRUE;
			}
		case KIND_BEGIN:
			{
				//begin . . end���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				while(1)
				{
					iRet = CompileTheStatement();				//���̃R���p�C��
					if(iRet < 0){return iRet;}
					while(1)
					{
						if (s_Token.m_iKind == KIND_SEMICOLON)
						{		//����");"�Ȃ當������
							s_Token = ProgressAndGetNextToken();
							break;
						}
						if (s_Token.m_iKind == KIND_END)
						{			//����end�Ȃ�I��
							s_Token = ProgressAndGetNextToken();
							return TRUE;
						}
						if (IsStartWithBeginKey(s_Token) == TRUE)
						{		//�������̐擪�L���Ȃ�
							iRet = OutputErrorInsert(KIND_SEMICOLON);	//");"��Y�ꂽ���Ƃɂ���
							if(iRet < 0){return iRet;}

							break;
						}
						iRet = OutputErrorDelete();	//����ȊO�Ȃ�G���[�Ƃ��ēǂݎ̂Ă�
						if(iRet < 0){return iRet;}
						s_Token = ProgressAndGetNextToken();
					}
				}
			}
		case KIND_WHILE:
			{//while���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				backP2 = GetNextCodeIndex();			//while���̍Ō��jmp���߂̔�ѐ�
				CompileTheCondition();				//�������̃R���p�C��
				s_Token = GetTokenWithCheck(s_Token, KIND_DO);	//"do"�̂͂�
				backP = genCodeV(OPERATION_CODE_JPC, 0);		//���������U�̂Ƃ���яo��jpc����
				iRet = CompileTheStatement();				//���̃R���p�C��
				if(iRet < 0){return iRet;}

				genCodeV(OPERATION_CODE_JMP, backP2);		//while���̐擪�ւ̃W�����v����
				backPatch(backP);	//�U�̂Ƃ���яo��jpc���߂ւ̃o�b�N�p�b�`
				return 0;
			}
		case KIND_WRITE:
			{			//write���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				iRet = CompileTheExpression();
				if(iRet < 0){return iRet;}
				genCodeO(OPERATOR_WRT);				//���̒l���o�͂���wrt����
				return 0;
			}
		case KIND_WRITE_LINE:
			{			//writeln���̃R���p�C��
				s_Token = ProgressAndGetNextToken();
				genCodeO(OPERATOR_WRL);				//���s���o�͂���wrl����
				return 0;
			}
		case KIND_END:			{return 0;}	
		case KIND_SEMICOLON:	{return 0;}		//�󕶂�ǂ񂾂��Ƃɂ��ďI��

		default:
			{//���̐擪�̃L�[�܂œǂݎ̂Ă�
				iRet = OutputErrorDelete();				//���ǂ񂾃g�[�N����ǂݎ̂Ă�
				if(iRet <0 ){return iRet;}

				s_Token = ProgressAndGetNextToken();
				continue;
			}
		}		
	}
	return 0;
}

BOOL IsStartWithBeginKey(Token t)			//�g�[�N��t�͕��̐擪�̃L�[���H
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

int CompileTheTerm()					//���̍��̃R���p�C��
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

BOOL CompileTheFactor()					//���̈��q�̃R���p�C��
{
	int tIndex;
	int iKind;
	int iRet;
	if (s_Token.m_iKind == KIND_ID)
	{
		tIndex = GetNameIndex(s_Token.u.szIdentifier, KIND_varId);
		iKind = GetKind(tIndex);
		setIdKind(GetKind(tIndex));			//�󎚂̂��߂̏��̃Z�b�g
		switch (iKind) 
		{
		case KIND_varId:
			{//�ϐ������p�����^��
				genCodeT(OPERATION_CODE_LOD, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_parId:	
			{//�ϐ������p�����^��
				genCodeT(OPERATION_CODE_LOD, tIndex);
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_constId:		
			{			//�萔��
				genCodeV(OPERATION_CODE_LIT, val(tIndex));
				s_Token = ProgressAndGetNextToken(); break;
			}
		case KIND_funcId:
			{//�֐��Ăяo��
				s_Token = ProgressAndGetNextToken();
				if (s_Token.m_iKind == KIND_PARENTHESIS_L)
				{
					int i;
					i=0; 					//i�͎������̌�
					s_Token = ProgressAndGetNextToken();
					if (s_Token.m_iKind != KIND_PARENTHESIS_R) 
					{
						for (; ; ) 
						{
							CompileTheExpression(); //�������̃R���p�C��
							i++;	
							if (s_Token.m_iKind == KIND_COMMA)
							{	// �����R���}�Ȃ������������
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

					}	//pars(tIndex)�͉������̌�
				}
				else
				{
					iRet = OutputErrorInsert(KIND_PARENTHESIS_L);
					if(iRet < 0){return iRet;}
					iRet = OutputErrorInsert(KIND_PARENTHESIS_R);
					if(iRet < 0){return iRet;}
				}
				genCodeT(OPERATION_CODE_CAL, tIndex);				//call����
				break;
			}
		}
	}
	else if (s_Token.m_iKind == KIND_NUM)
	{			//�萔
		genCodeV(OPERATION_CODE_LIT, s_Token.u.value);
		s_Token = ProgressAndGetNextToken();
	}
	else if (s_Token.m_iKind == KIND_PARENTHESIS_L)
	{			//�u(�v�u���q�v�u)�v
		s_Token = ProgressAndGetNextToken();
		CompileTheExpression();
		s_Token = GetTokenWithCheck(s_Token, KIND_PARENTHESIS_R);
	}
	switch (s_Token.m_iKind)
	{					//���q�̌オ�܂����q�Ȃ�G���[
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

BOOL CompileTheCondition()					//�������̃R���p�C��
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

