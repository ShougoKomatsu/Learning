
   /*************** getSource.h ***************/

#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif
 
#define MAXNAME 31			/*�@���O�̍ő咷���@*/
 
typedef  enum  keys {			/*�@�L�[�╶���̎�ށi���O�j�@*/
	Begin, End,				/*�@�\���̖��O�@*/
	If, Then,
	While, Do,
	Ret, Func, 
	Var, Const, Odd,
	Write, WriteLn,
	end_of_KeyWd,				/*�@�\���̖��O�͂����܂Ł@*/
	Plus, Minus,				/*�@���Z�q�Ƌ�؂�L���̖��O�@*/
	Mult, Div,	
	Lparen, Rparen,
	Equal, Lss, Gtr,
	NotEq, LssEq, GtrEq, 
	Comma, Period, Semicolon,
	Assign,
	end_of_KeySym,				/*�@���Z�q�Ƌ�؂�L���̖��O�͂����܂Ł@*/
	Id, Num, nul,				/*�@�g�[�N���̎�ށ@*/
	end_of_Token,
	letter, digit, colon, others		/*�@��L�ȊO�̕����̎�ށ@*/
} KeyId;

typedef  struct  token {			/*�@�g�[�N���̌^�@*/
	KeyId kind;				/*�@�g�[�N���̎�ނ��L�[�̖��O�@*/
	union {
		char id[MAXNAME]; 		/*�@Identfier�̎��A���̖��O�@*/
		int value;				/*�@Num�̎��A���̒l�@*/
	} u;
}Token;

Token ProgressAndGetNextToken();				/*�@���̃g�[�N����ǂ�ŕԂ��@*/
Token checkGet(Token t, KeyId k);	/*�@t.kind==k�̃`�F�b�N�@*/
	/*�@t.kind==k�Ȃ�A���̃g�[�N����ǂ�ŕԂ��@*/
	/*�@t.kind!=k�Ȃ�G���[���b�Z�[�W���o���At��k�����ɋL���A�܂��͗\���Ȃ�@*/
	/*�@t���̂āA���̃g�[�N����ǂ�ŕԂ��it��k�Œu�����������ƂɂȂ�j�@*/
	/*�@����ȊO�̏ꍇ�Ak��}���������Ƃɂ��āAt��Ԃ��@*/

int openSource(char fileName[]); 	/*�@�\�[�X�t�@�C����open�@*/
void closeSource();			/*�@�\�[�X�t�@�C����close�@*/
void initSource();			/*�@�e�[�u���̏����ݒ�Atex�t�@�C���̏����ݒ�@*/  
void finalSource(); 			/*�@�\�[�X�̍Ō�̃`�F�b�N�Atex�t�@�C���̍ŏI�ݒ�@*/  
void errorType(char *m);		/*�@�^�G���[��.tex�t�@�C���ɏo�́@*/
void errorInsert(KeyId k);		/*�@keyString(k)��.tex�t�@�C���ɑ}���@*/
void errorMissingId();		/*�@���O���Ȃ��Ƃ̃��b�Z�[�W��.tex�t�@�C���ɑ}���@*/
void errorMissingOp();		/*�@���Z�q���Ȃ��Ƃ̃��b�Z�[�W��.tex�t�@�C���ɑ}���@*/
void errorDelete();			/*�@���ǂ񂾃g�[�N����ǂݎ̂āi.tex�t�@�C���ɏo�́j*/
BOOL OutputErrMessage(char *m);	/*�@�G���[���b�Z�[�W��.tex�t�@�C���ɏo�́@*/
void OutputErrAndFinish(char *m);			/*�@�G���[���b�Z�[�W���o�͂��A�R���p�C���I���@*/
int errorN();				/*�@�G���[�̌���Ԃ��@*/

void setIdKind(KindTable k);     /*�@���g�[�N��(Id)�̎�ނ��Z�b�g�i.tex�t�@�C���o�͂̂��߁j*/

