#include "stdafx.h"
  /******************** codegen.h ********************/

//命令語のコード
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

int genCodeV(int op, int v);		//命令語の生成、アドレス部にv
int genCodeT(int op, int ti);		//命令語の生成、アドレスは名前表から
int genCodeO(int p);			//命令語の生成、アドレス部に演算命令
int genCodeR();					//ret命令語の生成
void backPatch(int i);			//命令語のバックパッチ（次の番地を）

int GetNextCodeIndex();			//次の命令語のアドレスを返す
void listCode();			//目的コード（命令語）のリスティング
void execute();			//目的コード（命令語）の実行

