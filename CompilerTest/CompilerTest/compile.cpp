#include "stdafx.h"
/*************** compile.c *************/

#include "getSource.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "codegen.h"

#define MINERROR 3			/*�@�G���[������ȉ��Ȃ���s�@*/
#define FIRSTADDR 2			/*�@�e�u���b�N�̍ŏ��̕ϐ��̃A�h���X�@*/

static Token token;				/*�@���̃g�[�N�������Ă����@*/

static void block(int pIndex);	/*�@�u���b�N�̃R���p�C���@*/
/*�@pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X�@*/
static void constDecl();			/*�@�萔�錾�̃R���p�C���@*/
static void varDecl();				/*�@�ϐ��錾�̃R���p�C���@*/
static void funcDecl();			/*�@�֐��錾�̃R���p�C���@*/
static void statement();			/*�@���̃R���p�C���@*/
static void expression();			/*�@���̃R���p�C���@*/
static void term();				/*�@���̍��̃R���p�C���@*/
static void factor();				/*�@���̈��q�̃R���p�C���@*/
static void condition();			/*�@�������̃R���p�C���@*/
static int isStBeginKey(Token t);		/*�@�g�[�N��t�͕��̐擪�̃L�[���H�@*/

int compile()
{
	int i;
	printf("start compilation\n");
	initSource();				/*�@getSource�̏����ݒ�@*/
	token = nextToken();			/*�@�ŏ��̃g�[�N���@*/
	blockBegin(FIRSTADDR);		/*�@����Ȍ�̐錾�͐V�����u���b�N�̂��́@*/
	block(0);					/*�@0 �̓_�~�[�i��u���b�N�̊֐����͂Ȃ��j�@*/
	finalSource();
	i = errorN();				/*�@�G���[���b�Z�[�W�̌��@*/
	if (i!=0)
		printf("%d errors\n", i);
	/*	listCode();	*/			/*�@�ړI�R�[�h�̃��X�g�i�K�v�Ȃ�j�@*/
	return i<MINERROR;		/*�@�G���[���b�Z�[�W�̌������Ȃ����ǂ����̔���@*/
}

void block(int pIndex)		/*�@pIndex �͂��̃u���b�N�̊֐����̃C���f�b�N�X�@*/
{
	int backP;
	backP = genCodeV(jmp, 0);		/*�@�����֐����щz�����߁A��Ńo�b�N�p�b�`�@*/
	while (1) {				/*�@�錾���̃R���p�C�����J��Ԃ��@*/
		switch (token.kind){
		case Const:			/*�@�萔�錾���̃R���p�C���@*/
			token = nextToken();
			constDecl(); continue;
		case Var:				/*�@�ϐ��錾���̃R���p�C���@*/
			token = nextToken();
			varDecl(); continue;
		case Func:				/*�@�֐��錾���̃R���p�C���@*/
			token = nextToken();
			funcDecl(); continue;
		default:				/*�@����ȊO�Ȃ�錾���͏I���@*/
			break;
		}
		break;
	}			
	backPatch(backP);			/*�@�����֐����щz�����߂Ƀp�b�`�@*/
	changeV(pIndex, nextCode());	/*�@���̊֐��̊J�n�Ԓn���C���@*/
	genCodeV(ict, frameL());		/*�@���̃u���b�N�̎��s���̕K�v�L������Ƃ閽�߁@*/
	statement();				/*�@���̃u���b�N�̎啶�@*/		
	genCodeR();				/*�@���^�[�����߁@*/
	blockEnd();				/*�@�u���b�N���I�������Ƃ�table�ɘA���@*/
}	

void constDecl()			/*�@�萔�錾�̃R���p�C���@*/
{
	Token temp;
	while(1){
		if (token.kind==Id){
			setIdKind(constId);				/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
			temp = token; 					/*�@���O�����Ă����@*/
			token = checkGet(nextToken(), Equal);		/*�@���O�̎���"="�̂͂��@*/
			if (token.kind==Num)
				enterTconst(temp.u.id, token.u.value);	/*�@�萔���ƒl���e�[�u���Ɂ@*/
			else
				errorType("number");
			token = nextToken();
		}else
			errorMissingId();
		if (token.kind!=Comma){		/*�@�����R���}�Ȃ�萔�錾�������@*/
			if (token.kind==Id){		/*�@�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���@*/
				errorInsert(Comma);
				continue;
			}else
				break;
		}
		token = nextToken();
	}
	token = checkGet(token, Semicolon);		/*�@�Ō��";"�̂͂��@*/
}

void varDecl()				/*�@�ϐ��錾�̃R���p�C���@*/
{
	while(1){
		if (token.kind==Id){
			setIdKind(varId);		/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
			enterTvar(token.u.id);		/*�@�ϐ������e�[�u���ɁA�Ԓn��table�����߂�@*/
			token = nextToken();
		}else
			errorMissingId();
		if (token.kind!=Comma){		/*�@�����R���}�Ȃ�ϐ��錾�������@*/
			if (token.kind==Id){		/*�@�������O�Ȃ�R���}��Y�ꂽ���Ƃɂ���@*/
				errorInsert(Comma);
				continue;
			}else
				break;
		}
		token = nextToken();
	}
	token = checkGet(token, Semicolon);		/*�@�Ō��";"�̂͂��@*/
}

void funcDecl()			/*�@�֐��錾�̃R���p�C���@*/
{
	int fIndex;
	if (token.kind==Id){
		setIdKind(funcId);				/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
		fIndex = enterTfunc(token.u.id, nextCode());		/*�@�֐������e�[�u���ɓo�^�@*/
		/*�@���̐擪�Ԓn�́A�܂��A���̃R�[�h�̔ԒnnextCode()�Ƃ���@*/
		token = checkGet(nextToken(), Lparen);
		blockBegin(FIRSTADDR);	/*�@�p�����^���̃��x���͊֐��̃u���b�N�Ɠ����@*/
		while(1){
			if (token.kind==Id){			/*�@�p�����^��������ꍇ�@*/
				setIdKind(parId);		/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
				enterTpar(token.u.id);		/*�@�p�����^�����e�[�u���ɓo�^�@*/
				token = nextToken();
			}else
				break;
			if (token.kind!=Comma){		/*�@�����R���}�Ȃ�p�����^���������@*/
				if (token.kind==Id){		/*�@�������O�Ȃ�R���}��Y�ꂽ���ƂɁ@*/
					errorInsert(Comma);
					continue;
				}else
					break;
			}
			token = nextToken();
		}
		token = checkGet(token, Rparen);		/*�@�Ō��")"�̂͂��@*/
		endpar();				/*�@�p�����^�����I��������Ƃ��e�[�u���ɘA���@*/
		if (token.kind==Semicolon){
			errorDelete();
			token = nextToken();
		}
		block(fIndex);	/*�@�u���b�N�̃R���p�C���A���̊֐����̃C���f�b�N�X��n���@*/
		token = checkGet(token, Semicolon);		/*�@�Ō��";"�̂͂��@*/
	} else 
		errorMissingId();			/*�@�֐������Ȃ��@*/
}

void statement()			/*�@���̃R���p�C���@*/
{
	int tIndex;
	KindT k;
	int backP, backP2;				/*�@�o�b�N�p�b�`�p�@*/

	while(1) {
		switch (token.kind) {
		case Id:					/*�@������̃R���p�C���@*/
			tIndex = searchT(token.u.id, varId);	/*�@���ӂ̕ϐ��̃C���f�b�N�X�@*/
			setIdKind(k=kindT(tIndex));			/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
			if (k != varId && k != parId) 		/*�@�ϐ������p�����^���̂͂��@*/
				errorType("var/par");
			token = checkGet(nextToken(), Assign);			/*�@":="�̂͂��@*/
			expression();					/*�@���̃R���p�C���@*/
			genCodeT(sto, tIndex);				/*�@���ӂւ̑�����߁@*/
			return;
		case If:					/*�@if���̃R���p�C���@*/
			token = nextToken();
			condition();					/*�@�������̃R���p�C���@*/
			token = checkGet(token, Then);		/*�@"then"�̂͂��@*/
			backP = genCodeV(jpc, 0);			/*�@jpc���߁@*/
			statement();					/*�@���̃R���p�C���@*/
			backPatch(backP);				/*�@���jpc���߂Ƀo�b�N�p�b�`�@*/
			return;
		case Ret:					/*�@return���̃R���p�C���@*/
			token = nextToken();
			expression();					/*�@���̃R���p�C���@*/
			genCodeR();					/*�@ret���߁@*/
			return;
		case Begin:				/*�@begin . . end���̃R���p�C���@*/
			token = nextToken();
			while(1){
				statement();				/*�@���̃R���p�C���@*/
				while(1){
					if (token.kind==Semicolon){		/*�@����";"�Ȃ當�������@*/
						token = nextToken();
						break;
					}
					if (token.kind==End){			/*�@����end�Ȃ�I��@*/
						token = nextToken();
						return;
					}
					if (isStBeginKey(token)){		/*�@�������̐擪�L���Ȃ�@*/
						errorInsert(Semicolon);	/*�@";"��Y�ꂽ���Ƃɂ���@*/
						break;
					}
					errorDelete();	/*�@����ȊO�Ȃ�G���[�Ƃ��ēǂݎ̂Ă�@*/
					token = nextToken();
				}
			}
		case While:				/*�@while���̃R���p�C���@*/
			token = nextToken();
			backP2 = nextCode();			/*�@while���̍Ō��jmp���߂̔�ѐ�@*/
			condition();				/*�@�������̃R���p�C���@*/
			token = checkGet(token, Do);	/*�@"do"�̂͂��@*/
			backP = genCodeV(jpc, 0);		/*�@���������U�̂Ƃ���яo��jpc���߁@*/
			statement();				/*�@���̃R���p�C���@*/
			genCodeV(jmp, backP2);		/*�@while���̐擪�ւ̃W�����v���߁@*/
			backPatch(backP);	/*�@�U�̂Ƃ���яo��jpc���߂ւ̃o�b�N�p�b�`�@*/
			return;
		case Write:			/*�@write���̃R���p�C���@*/
			token = nextToken();
			expression();				/*�@���̃R���p�C���@*/
			genCodeO(wrt);				/*�@���̒l���o�͂���wrt���߁@*/
			return;
		case WriteLn:			/*�@writeln���̃R���p�C���@*/
			token = nextToken();
			genCodeO(wrl);				/*�@���s���o�͂���wrl���߁@*/
			return;
		case End: case Semicolon:			/*�@�󕶂�ǂ񂾂��Ƃɂ��ďI��@*/
			return;
		default:				/*�@���̐擪�̃L�[�܂œǂݎ̂Ă�@*/
			errorDelete();				/*�@���ǂ񂾃g�[�N����ǂݎ̂Ă�@*/
			token = nextToken();
			continue;
		}		
	}
}

int isStBeginKey(Token t)			/*�@�g�[�N��t�͕��̐擪�̃L�[���H�@*/
{
	switch (t.kind){
	case If: case Begin: case Ret:
	case While: case Write: case WriteLn:
		return 1;
	default:
		return 0;
	}
}

void expression()				/*�@���̃R���p�C���@*/
{
	KeyId k;
	k = token.kind;
	if (k==Plus || k==Minus){
		token = nextToken();
		term();
		if (k==Minus)
			genCodeO(neg);
	}else
		term();
	k = token.kind;
	while (k==Plus || k==Minus){
		token = nextToken();
		term();
		if (k==Minus)
			genCodeO(sub);
		else
			genCodeO(add);
		k = token.kind;
	}
}

void term()					/*�@���̍��̃R���p�C���@*/
{
	KeyId k;
	factor();
	k = token.kind;
	while (k==Mult || k==Div){	
		token = nextToken();
		factor();
		if (k==Mult)
			genCodeO(mul);
		else
			genCodeO(div_);
		k = token.kind;
	}
}

void factor()					/*�@���̈��q�̃R���p�C���@*/
{
	int tIndex, i;
	KeyId k;
	if (token.kind==Id){
		tIndex = searchT(token.u.id, varId);
		k=static_cast<KeyId>(kindT(tIndex));
		setIdKind(kindT(tIndex));			/*�@�󎚂̂��߂̏��̃Z�b�g�@*/
		switch (k) {
		case varId: case parId:			/*�@�ϐ������p�����^���@*/
			genCodeT(lod, tIndex);
			token = nextToken(); break;
		case constId:					/*�@�萔���@*/
			genCodeV(lit, val(tIndex));
			token = nextToken(); break;
		case funcId:					/*�@�֐��Ăяo���@*/
			token = nextToken();
			if (token.kind==Lparen){
				i=0; 					/*�@i�͎������̌��@*/
				token = nextToken();
				if (token.kind != Rparen) {
					for (; ; ) {
						expression(); i++;	/*�@�������̃R���p�C���@*/
						if (token.kind==Comma){	/* �����R���}�Ȃ������������ */
							token = nextToken();
							continue;
						}
						token = checkGet(token, Rparen);
						break;
					}
				} else
					token = nextToken();
				if (pars(tIndex) != i) 
					errorMessage("\\#par");	/*�@pars(tIndex)�͉������̌��@*/
			}else{
				errorInsert(Lparen);
				errorInsert(Rparen);
			}
			genCodeT(cal, tIndex);				/*�@call���߁@*/
			break;
		}
	}else if (token.kind==Num){			/*�@�萔�@*/
		genCodeV(lit, token.u.value);
		token = nextToken();
	}else if (token.kind==Lparen){			/*�@�u(�v�u���q�v�u)�v�@*/
		token = nextToken();
		expression();
		token = checkGet(token, Rparen);
	}
	switch (token.kind){					/*�@���q�̌オ�܂����q�Ȃ�G���[�@*/
	case Id: case Num: case Lparen:
		errorMissingOp();
		factor();
	default:
		return;
	}	
}

void condition()					/*�@�������̃R���p�C���@*/
{
	KeyId k;
	if (token.kind==Odd){
		token = nextToken();
		expression();
		genCodeO(odd);
	}else{
		expression();
		k = token.kind;
		switch(k){
		case Equal: case Lss: case Gtr:
		case NotEq: case LssEq: case GtrEq:
			break;
		default:
			errorType("rel-op");
			break;
		}
		token = nextToken();
		expression();
		switch(k){
		case Equal:	genCodeO(eq); break;
		case Lss:		genCodeO(ls); break;
		case Gtr:		genCodeO(gr); break;
		case NotEq:	genCodeO(neq); break;
		case LssEq:	genCodeO(lseq); break;
		case GtrEq:	genCodeO(greq); break;
		}
	}
}

