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
	OpCode  opCode;
	union
	{
		RelAddr addr;
		int value;
		Operator optr;
	}u;
}Inst;

static Inst s_instCode[MAXCODE];		//�ړI�R�[�h������
static int s_iCodeIndex = -1;				//�Ō�ɐ����������ߌ�̃C���f�b�N�X
static void checkMax();	     		 //�ړI�R�[�h�̃C���f�b�N�X�̑����ƃ`�F�b�N
static void printCode(int i);		//���ߌ�̈�

int nextCode()					//���̖��ߌ�̃A�h���X��Ԃ�
{
	return s_iCodeIndex +1;
}

int genCodeV(OpCode op, int v)		//���ߌ�̐����A�A�h���X����v
{
	checkMax();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.value = v;
	return s_iCodeIndex;
}

int genCodeT(OpCode op, int ti)		//���ߌ�̐����A�A�h���X�͖��O�\����
{
	checkMax();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.addr = relAddr(ti);
	return s_iCodeIndex;
}

int genCodeO(Operator p)			//���ߌ�̐����A�A�h���X���ɉ��Z����
{
	checkMax();
	s_instCode[s_iCodeIndex].opCode = opr;
	s_instCode[s_iCodeIndex].u.optr = p;
	return s_iCodeIndex;
}

int genCodeR()					//ret���ߌ�̐���
{
	if (s_instCode[s_iCodeIndex].opCode == ret){return s_iCodeIndex;}			//���O��ret�Ȃ琶������

	checkMax();
	s_instCode[s_iCodeIndex].opCode = ret;
	s_instCode[s_iCodeIndex].u.addr.level = bLevel();
	s_instCode[s_iCodeIndex].u.addr.addr = fPars();	//�p�����^���i���s�X�^�b�N�̉���p�j*/
	return s_iCodeIndex;
}

void checkMax()		//�ړI�R�[�h�̃C���f�b�N�X�̑����ƃ`�F�b�N
{
	s_iCodeIndex++;
	if (s_iCodeIndex < MAXCODE){return;}

	errorF("too many code");
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
	case lit: {printf("lit"); flag=1; break;}
	case opr: {printf("opr"); flag=3; break;}
	case lod: {printf("lod"); flag=2; break;}
	case sto: {printf("sto"); flag=2; break;}
	case cal: {printf("cal"); flag=2; break;}
	case ret: {printf("ret"); flag=2; break;}
	case ict: {printf("ict"); flag=1; break;}
	case jmp: {printf("jmp"); flag=1; break;}
	case jpc: {printf("jpc"); flag=1; break;}
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
			case neg: printf(",neg\n"); {return;}
			case add: printf(",add\n"); {return;}
			case sub: printf(",sub\n"); {return;}
			case mul: printf(",mul\n"); {return;}
			case div_: printf(",div\n"); {return;}
			case odd: printf(",odd\n"); {return;}
			case eq: printf(",eq\n"); {return;}
			case ls: printf(",ls\n"); {return;}
			case gr: printf(",gr\n"); {return;}
			case neq: printf(",neq\n"); {return;}
			case lseq: printf(",lseq\n"); {return;}
			case greq: printf(",greq\n"); {return;}
			case wrt: printf(",wrt\n"); {return;}
			case wrl: printf(",wrl\n"); {return;}
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
	top = 0;  pc = 0;			//top:���ɃX�^�b�N�ɓ����ꏊ�Apc:���ߌ�̃J�E���^
	stack[0] = 0;  stack[1] = 0; 	//stack[top]��callee�ŉ󂷃f�B�X�v���C�̑ޔ��ꏊ
	//stack[top+1]��caller�ւ̖߂�Ԓn
	display[0] = 0;			//��u���b�N�̐擪�Ԓn�� 0
	do 
	{
		i = s_instCode[pc];			//���ꂩ����s���閽�ߌ�
		pc++;
		switch(i.opCode)
		{
		case lit:
			{
				stack[top] = i.u.value; 
				top++;
				break;
			}
		case lod:
			{
				stack[top] = stack[display[i.u.addr.level] + i.u.addr.addr]; 
				top++;
				break;
			}
		case sto: 
			{
				top--;
				stack[display[i.u.addr.level] + i.u.addr.addr] = stack[top]; 
				break;
			}
		case cal:
			{
				lev = i.u.addr.level +1;		// i.u.addr.level��callee�̖��O�̃��x��
				// callee�̃u���b�N�̃��x��lev�͂���Ɂ{�P��������
				stack[top] = display[lev]; 	//display[lev]�̑ޔ�
				stack[top+1] = pc; display[lev] = top; 
				//���݂�top��callee�̃u���b�N�̐擪�Ԓn
				pc = i.u.addr.addr;
				break;
			}
		case ret: 
			{
				top--;
				temp = stack[top];		//�X�^�b�N�̃g�b�v�ɂ�����̂��Ԃ��l
				top = display[i.u.addr.level];  	//top���Ă΂ꂽ�Ƃ��̒l�ɖ߂�
				display[i.u.addr.level] = stack[top];		/* �󂵂��f�B�X�v���C�̉� */
				pc = stack[top+1];
				top -= i.u.addr.addr;		//�������̕������g�b�v��߂�
				stack[top] = temp;		//�Ԃ��l���X�^�b�N�̃g�b�v��
				top++;
				break;
			}
		case ict:
			{
				top += i.u.value; 
				if (top >= MAXMEM-MAXREG){errorF("stack overflow");}
				break;
			}
		case jmp: 
			{
				pc = i.u.value; 
				break;
			}
		case jpc: 
			{
				top--;
				if (stack[top] == 0){pc = i.u.value;}
				break;
			}
		case opr: 
			switch(i.u.optr)
			{
			case neg:{ stack[top-1] = -stack[top-1]; continue;}
			case add:{ top--;  stack[top-1] += stack[top]; continue;}
			case sub:{ top--; stack[top-1] -= stack[top]; continue;}
			case mul:{ top--;  stack[top-1] *= stack[top];  continue;}
			case div_:{ --top;  stack[top-1] /= stack[top]; continue;}
			case odd:{ stack[top-1] = stack[top-1] & 1; continue;}
			case eq:{ top--;  stack[top-1] = (stack[top-1] == stack[top]); continue;}
			case ls:{ top--;  stack[top-1] = (stack[top-1] < stack[top]); continue;}
			case gr:{ top--;  stack[top-1] = (stack[top-1] > stack[top]); continue;}
			case neq:{ top--;  stack[top-1] = (stack[top-1] != stack[top]); continue;}
			case lseq:{ top--;  stack[top-1] = (stack[top-1] <= stack[top]); continue;}
			case greq:{ top--;  stack[top-1] = (stack[top-1] >= stack[top]); continue;}
			case wrt:{ top--; printf("%d ", stack[top]); continue;}
			case wrl:{ printf("\n"); continue;}
			}
		}
	} while (pc != 0);
}

