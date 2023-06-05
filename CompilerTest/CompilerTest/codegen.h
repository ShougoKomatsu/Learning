#include "stdafx.h"
  /******************** codegen.h ********************/

//���ߌ�̃R�[�h
#define OPERATION_CODE_LIT (1)
#define OPERATION_CODE_OPR (2)
#define OPERATION_CODE_LOD (3)
#define OPERATION_CODE_STO (4)
#define OPERATION_CODE_CAL (5)
#define OPERATION_CODE_RET (6)
#define OPERATION_CODE_ICT (7)
#define OPERATION_CODE_JMP (8)
#define OPERATION_CODE_JPC (9)


#define OPERATOR_NEG (1)
#define OPERATOR_ADD (2)
#define OPERATOR_SUB (3)
#define OPERATOR_MUL (4)
#define OPERATOR_DIV (5)
#define OPERATOR_ODD (6)
#define OPERATOR_EQ (7)
#define OPERATOR_LS (8)
#define OPERATOR_GR (9)
#define OPERATOR_NEQ (10)
#define OPERATOR_LSEQ (11)
#define OPERATOR_GREQ (12)
#define OPERATOR_WRT (13)
#define OPERATOR_WRL (14)

int genCodeV(int op, int v);		//���ߌ�̐����A�A�h���X����v
int genCodeT(int op, int ti);		//���ߌ�̐����A�A�h���X�͖��O�\����
int genCodeO(int p);			//���ߌ�̐����A�A�h���X���ɉ��Z����
int genCodeR();					//ret���ߌ�̐���
void backPatch(int i);			//���ߌ�̃o�b�N�p�b�`�i���̔Ԓn���j

int GetNextCodeIndex();			//���̖��ߌ�̃A�h���X��Ԃ�
void listCode();			//�ړI�R�[�h�i���ߌ�j�̃��X�e�B���O
void execute();			//�ړI�R�[�h�i���ߌ�j�̎��s

