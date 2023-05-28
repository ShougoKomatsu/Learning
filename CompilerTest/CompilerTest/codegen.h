#include "stdafx.h"
   /******************** codegen.h ********************/

typedef enum codes{			//���ߌ�̃R�[�h
	lit, opr, lod, sto, cal, ret, ict, jmp, jpc
}OpCode;

typedef enum ops{			//���Z���߂̃R�[�h
	neg, add, sub, mul, div_, odd, eq, ls, gr,
	neq, lseq, greq, wrt, wrl
}Operator;

int genCodeV(OpCode op, int v);		//���ߌ�̐����A�A�h���X����v
int genCodeT(OpCode op, int ti);		//���ߌ�̐����A�A�h���X�͖��O�\����
int genCodeO(Operator p);			//���ߌ�̐����A�A�h���X���ɉ��Z����
int genCodeR();					//ret���ߌ�̐���
void backPatch(int i);			//���ߌ�̃o�b�N�p�b�`�i���̔Ԓn���j

int nextCode();			//���̖��ߌ�̃A�h���X��Ԃ�
void listCode();			//�ړI�R�[�h�i���ߌ�j�̃��X�e�B���O
void execute();			//�ړI�R�[�h�i���ߌ�j�̎��s
