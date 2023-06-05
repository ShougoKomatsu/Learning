
  /*************** getSource.h ***************/

#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif
 
#define MAXNAME 31			/*�@���O�̍ő咷���@*/
 
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
{			/*�@�g�[�N���̌^�@*/
	int kind;				/*�@�g�[�N���̎�ނ��L�[�̖��O�@*/
	union 
	{
		char szIdentifier[MAXNAME]; 		/*�@Identfier�̎��A���̖��O�@*/
		int value;				/*�@Num�̎��A���̒l�@*/
	} u;
}Token;

Token ProgressAndGetNextToken();				/*�@���̃g�[�N����ǂ�ŕԂ��@*/
Token GetTokenWithCheck(Token t, int k);	/*�@t.kind == k�̃`�F�b�N�@*/
	/*�@t.kind == k�Ȃ�A���̃g�[�N����ǂ�ŕԂ��@*/
	/*�@t.kind != k�Ȃ�G���[���b�Z�[�W���o���At��k�����ɋL���A�܂��͗\���Ȃ�@*/
	/*�@t���̂āA���̃g�[�N����ǂ�ŕԂ��it��k�Œu�����������ƂɂȂ�j�@*/
	/*�@����ȊO�̏ꍇ�Ak��}���������Ƃɂ��āAt��Ԃ��@*/

#define ERR_OUTPUT_NORMAL (1)
#define ERR_OUTPUT_COUNT_MAX (-1)
#define ERR_OUTPUT_ABORT (-2)

int openSource(char fileName[]); 	/*�@�\�[�X�t�@�C����open�@*/
void closeSource();			/*�@�\�[�X�t�@�C����close�@*/
void initSource();			/*�@�e�[�u���̏����ݒ�Atex�t�@�C���̏����ݒ�@*/ 
void finalSource(); 			/*�@�\�[�X�̍Ō�̃`�F�b�N�Atex�t�@�C���̍ŏI�ݒ�@*/ 
int OutputErrorType(char *m);		/*�@�^�G���[��.tex�t�@�C���ɏo�́@*/
int OutputErrorInsert(int k);		/*�@keyString(k)��.tex�t�@�C���ɑ}���@*/
int OutputErrorMissingID();		/*�@���O���Ȃ��Ƃ̃��b�Z�[�W��.tex�t�@�C���ɑ}���@*/
int OutputErrorMissingOperator();		/*�@���Z�q���Ȃ��Ƃ̃��b�Z�[�W��.tex�t�@�C���ɑ}���@*/
int OutputErrorDelete();			/*�@���ǂ񂾃g�[�N����ǂݎ̂āi.tex�t�@�C���ɏo�́j*/
int OutputErrMessage(char *m);	/*�@�G���[���b�Z�[�W��.tex�t�@�C���ɏo�́@*/
int OutputErrAndFinish(char *m);			/*�@�G���[���b�Z�[�W���o�͂��A�R���p�C���I���@*/
int errorN();				/*�@�G���[�̌���Ԃ��@*/

void setIdKind(KindTable k);   /*�@���g�[�N��(Id)�̎�ނ��Z�b�g�i.tex�t�@�C���o�͂̂��߁j*/

