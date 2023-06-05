#include "stdafx.h"
  /*****************codegen.c******************/

#include <stdio.h>
#include "codegen.h"
#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXCODE 200			//目的コードの最大長さ
#define MAXMEM 2000			//実行時スタックの最大長さ
#define MAXREG 20			//演算レジスタスタックの最大長さ
#define MAXLEVEL 5			//ブロックの最大深さ

typedef struct inst
{				//命令語の型
	int opCode;
	union
	{
		RelAddr addr;
		int value;
		int optr;
	}u;
}Inst;

static Inst s_instCode[MAXCODE];		//目的コードが入る
static int s_iCodeIndex = -1;				//最後に生成した命令語のインデックス
static void IncrimentCodeIndex();	   		 //目的コードのインデックスの増加とチェック
static void printCode(int i);		//命令語の印字

int GetNextCodeIndex()					//次の命令語のアドレスを返す
{
	return s_iCodeIndex +1;
}

int genCodeV(int op, int v)		//命令語の生成、アドレス部にv
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.value = v;
	return s_iCodeIndex;
}

int genCodeT(int op, int ti)		//命令語の生成、アドレスは名前表から
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = op;
	s_instCode[s_iCodeIndex].u.addr = relAddr(ti);
	return s_iCodeIndex;
}

int genCodeO(int p)			//命令語の生成、アドレス部に演算命令
{
	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = OPERATION_CODE_OPR;
	s_instCode[s_iCodeIndex].u.optr = p;
	return s_iCodeIndex;
}

int genCodeR()					//ret命令語の生成
{
	if (s_instCode[s_iCodeIndex].opCode == OPERATION_CODE_RET){return s_iCodeIndex;}			//直前がretなら生成せず

	IncrimentCodeIndex();
	s_instCode[s_iCodeIndex].opCode = OPERATION_CODE_RET;
	s_instCode[s_iCodeIndex].u.addr.level = iGetBlockLevel();
	s_instCode[s_iCodeIndex].u.addr.addr = GetFunctionParameterNum();	//パラメタ数（実行スタックの解放用）*/
	return s_iCodeIndex;
}

void IncrimentCodeIndex()		//目的コードのインデックスの増加とチェック
{
	s_iCodeIndex++;
	if (s_iCodeIndex < MAXCODE){return;}

	OutputErrAndFinish("too many code");
}
	
void backPatch(int i)		//命令語のバックパッチ（次の番地を）
{
	s_instCode[i].u.value = s_iCodeIndex+1;
}

void listCode()			//命令語のリスティング
{
	int i;
	printf("\ncode\n");
	for(i=0; i<=s_iCodeIndex; i++)
	{
		printf("%3d: ", i);
		printCode(i);
	}
}

void printCode(int i)		//命令語の印字
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

void execute()			//目的コード（命令語）の実行
{
	int stack[MAXMEM];		//実行時スタック
	int display[MAXLEVEL];	//現在見える各ブロックの先頭番地のディスプレイ
	int pc, top, lev, temp;
	Inst i;					//実行する命令語
	printf("start execution\n");
	top = 0; pc = 0;			//top:次にスタックに入れる場所、pc:命令語のカウンタ
	stack[0] = 0; stack[1] = 0; 	//stack[top]はcalleeで壊すディスプレイの退避場所
	//stack[top+1]はcallerへの戻り番地
	display[0] = 0;			//主ブロックの先頭番地は 0
	do 
	{
		i = s_instCode[pc];			//これから実行する命令語
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
				lev = i.u.addr.level +1;		// i.u.addr.levelはcalleeの名前のレベル
				// calleeのブロックのレベルlevはそれに＋１したもの
				stack[top] = display[lev]; 	//display[lev]の退避
				stack[top+1] = pc; display[lev] = top; 
				//現在のtopがcalleeのブロックの先頭番地
				pc = i.u.addr.addr;
				break;
			}
		case OPERATION_CODE_RET: 
			{
				top--;
				temp = stack[top];		//スタックのトップにあるものが返す値
				top = display[i.u.addr.level]; 	//topを呼ばれたときの値に戻す
				display[i.u.addr.level] = stack[top];		/* 壊したディスプレイの回復 */
				pc = stack[top+1];
				top -= i.u.addr.addr;		//実引数の分だけトップを戻す
				stack[top] = temp;		//返す値をスタックのトップへ
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

