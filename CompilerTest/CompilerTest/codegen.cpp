#include "stdafx.h"
  /*****************codegen.c******************/

#include <stdio.h>
#include "codegen.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXCODE 200			//�ړI�R�[�h�̍ő咷��
#define MAXMEM 2000			//���s���X�^�b�N�̍ő咷��
#define MAXREG 20			//���Z���W�X�^�X�^�b�N�̍ő咷��
#define MAXLEVEL 5			//�u���b�N�̍ő�[��

typedef struct inst
{				//���ߌ�̌^
	int opCode;
	union
	{
		RelAddr addr;
		int value;
		int optr;
	}u;
}Inst;

static Inst s_instCode[MAXCODE];		//�ړI�R�[�h������
static int s_iCodeIndex = -1;				//�Ō�ɐ����������ߌ�̃C���f�b�N�X
static void IncrimentCodeIndex();	   		 //�ړI�R�[�h�̃C���f�b�N�X�̑����ƃ`�F�b�N
static void printCode(int i);		//���ߌ�̈�

int GetNextCodeIndex()					//���̖��ߌ�̃A�h���X��Ԃ�
{
	return s_iCodeIndex +1;
}

int genCodeV(int op, int v)		//���ߌ�̐����A�A�h���X����v
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.value = v;
	return s_iCodeIndex;
}

int genCodeT(int op, int ti)		//���ߌ�̐����A�A�h���X�͖��O�\����
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.addr = relAddr(ti);
	return s_iCodeIndex;
}

int genCodeO(int p)			//���ߌ�̐����A�A�h���X���ɉ��Z����
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = OPERATION_CODE_OPR;
	s_instCode[s_iCodeIndex].u.optr = p;
	return s_iCodeIndex;
}

int genCodeR()					//ret���ߌ�̐���
{
	if (s_instCode[s_iCodeIndex].opCode == OPERATION_CODE_RET){return s_iCodeIndex;}			//���O��ret�Ȃ琶������

	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = OPERATION_CODE_RET;
	s_instCode[s_iCodeIndex].u.addr.level = iGetBlockLevel();
	s_instCode[s_iCodeIndex].u.addr.addr = GetFunctionParameterNum();	//�p�����^���i���s�X�^�b�N�̉���p�j*/
	return s_iCodeIndex;
}

void IncrimentCodeIndex()		//�ړI�R�[�h�̃C���f�b�N�X�̑����ƃ`�F�b�N
{
	s_iCodeIndex++;
	if (s_iCodeIndex < MAXCODE){return;}

	OutputErrAndFinish("too many code");
}
	
void backPatch(int i)		//���ߌ�̃o�b�N�p�b�`�i���̔Ԓn���j
{
	s_instCode[i].u.value = s_iCodeIndex+1;
}

void listCode()			//���ߌ�̃��X�e�B���O
{
	int i;
	printf("\ncode\n");
	for(i=0; i<=s_iCodeIndex; i++)
	{
		printf("%3d: ", i);
		printCode(i);
	}
}

void printCode(int i)		//���ߌ�̈�
{
	int flag;
	switch(s_instCode[i].opCode)
	{
	case OPERATION_CODE_LIT: {printf("lit"); flag=1; break;}
	case OPERATION_CODE_OPR: {printf("opr"); flag=3; break;}
	case OPERATION_CODE_LOD: {printf("lod"); flag=2; break;}
	case OPERATION_CODE_STO: {printf("sto"); flag=2; break;}
	case OPERATION_CODE_CAL: {printf("cal"); flag=2; break;}
	case OPERATION_CODE_RET: {printf("ret"); flag=2; break;}
	case OPERATION_CODE_ICT: {printf("ict"); flag=1; break;}
	case OPERATION_CODE_JMP: {printf("jmp"); flag=1; break;}
	case OPERATION_CODE_JPC: {printf("jpc"); flag=1; break;}
	}

	switch(flag)
	{
	case 1:
		{
			printf(",%d\n", s_instCode[i].u.value);
			return;
		}
	case 2:
		{
			printf(",%d", s_instCode[i].u.addr.level);
			printf(",%d\n", s_instCode[i].u.addr.addr);
			return;
		}
	case 3:
		{
			switch(s_instCode[i].u.optr)
			{
			case OPERATOR_NEG: printf(",neg\n"); {return;}
			case OPERATOR_ADD: printf(",add\n"); {return;}
			case OPERATOR_SUB: printf(",sub\n"); {return;}
			case OPERATOR_MUL: printf(",mul\n"); {return;}
			case OPERATOR_DIV: printf(",div\n"); {return;}
			case OPERATOR_ODD: printf(",odd\n"); {return;}
			case OPERATOR_EQ: printf(",eq\n"); {return;}
			case OPERATOR_LS: printf(",ls\n"); {return;}
			case OPERATOR_GR: printf(",gr\n"); {return;}
			case OPERATOR_NEQ: printf(",neq\n"); {return;}
			case OPERATOR_LSEQ: printf(",lseq\n"); {return;}
			case OPERATOR_GREQ: printf(",greq\n"); {return;}
			case OPERATOR_WRT: printf(",wrt\n"); {return;}
			case OPERATOR_WRL: printf(",wrl\n"); {return;}
			}
		}
	}
}	

void execute()			//�ړI�R�[�h�i���ߌ�j�̎��s
{
	int stack[MAXMEM];		//���s���X�^�b�N
	int display[MAXLEVEL];	//���݌�����e�u���b�N�̐擪�Ԓn�̃f�B�X�v���C
	int pc, top, lev, temp;
	Inst i;					//���s���閽�ߌ�
	printf("start execution\n");
	top = 0; pc = 0;			//top:���ɃX�^�b�N�ɓ����ꏊ�Apc:���ߌ�̃J�E���^
	stack[0] = 0; stack[1] = 0; 	//stack[top]��callee�ŉ󂷃f�B�X�v���C�̑ޔ��ꏊ
	//stack[top+1]��caller�ւ̖߂�Ԓn
	display[0] = 0;			//��u���b�N�̐擪�Ԓn�� 0
	do 
	{
		i = s_instCode[pc];			//���ꂩ����s���閽�ߌ�
		pc++;
		switch(i.opCode)
		{
		case OPERATION_CODE_LIT:
			{
				stack[top] = i.u.value; 
				top++;
				break;
			}
		case OPERATION_CODE_LOD:
			{
				stack[top] = stack[display[i.u.addr.level] + i.u.addr.addr]; 
				top++;
				break;
			}
		case OPERATION_CODE_STO: 
			{
				top--;
				stack[display[i.u.addr.level] + i.u.addr.addr] = stack[top]; 
				break;
			}
		case OPERATION_CODE_CAL:
			{
				lev = i.u.addr.level +1;		// i.u.addr.level��callee�̖��O�̃��x��
				// callee�̃u���b�N�̃��x��lev�͂���Ɂ{�P��������
				stack[top] = display[lev]; 	//display[lev]�̑ޔ�
				stack[top+1] = pc; display[lev] = top; 
				//���݂�top��callee�̃u���b�N�̐擪�Ԓn
				pc = i.u.addr.addr;
				break;
			}
		case OPERATION_CODE_RET: 
			{
				top--;
				temp = stack[top];		//�X�^�b�N�̃g�b�v�ɂ�����̂��Ԃ��l
				top = display[i.u.addr.level]; 	//top���Ă΂ꂽ�Ƃ��̒l�ɖ߂�
				display[i.u.addr.level] = stack[top];		/* �󂵂��f�B�X�v���C�̉� */
				pc = stack[top+1];
				top -= i.u.addr.addr;		//�������̕������g�b�v��߂�
				stack[top] = temp;		//�Ԃ��l���X�^�b�N�̃g�b�v��
				top++;
				break;
			}
		case OPERATION_CODE_ICT:
			{
				top += i.u.value; 
				if (top >= MAXMEM-MAXREG){OutputErrAndFinish("stack overflow");}
				break;
			}
		case OPERATION_CODE_JMP: 
			{
				pc = i.u.value; 
				break;
			}
		case OPERATION_CODE_JPC: 
			{
				top--;
				if (stack[top] == 0){pc = i.u.value;}
				break;
			}
		case OPERATION_CODE_OPR: 
			switch(i.u.optr)
			{
			case OPERATOR_NEG:{ stack[top-1] = -stack[top-1]; continue;}
			case OPERATOR_ADD:{ top--; stack[top-1] += stack[top]; continue;}
			case OPERATOR_SUB:{ top--; stack[top-1] -= stack[top]; continue;}
			case OPERATOR_MUL:{ top--; stack[top-1] *= stack[top]; continue;}
			case OPERATOR_DIV:{ --top; stack[top-1] /= stack[top]; continue;}
			case OPERATOR_ODD:{ stack[top-1] = stack[top-1] & 1; continue;}
			case OPERATOR_EQ:{ top--; stack[top-1] = (stack[top-1] == stack[top]); continue;}
			case OPERATOR_LS:{ top--; stack[top-1] = (stack[top-1] < stack[top]); continue;}
			case OPERATOR_GR:{ top--; stack[top-1] = (stack[top-1] > stack[top]); continue;}
			case OPERATOR_NEQ:{ top--; stack[top-1] = (stack[top-1] != stack[top]); continue;}
			case OPERATOR_LSEQ:{ top--; stack[top-1] = (stack[top-1] <= stack[top]); continue;}
			case OPERATOR_GREQ:{ top--; stack[top-1] = (stack[top-1] >= stack[top]); continue;}
			case OPERATOR_WRT:{ top--; printf("%d ", stack[top]); continue;}
			case OPERATOR_WRL:{ printf("\n"); continue;}
			}
		}
	} while (pc != 0);
}

