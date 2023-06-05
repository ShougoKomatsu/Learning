
 /***********table.h***********/

typedef enum kindTable 
{		//Identifierの種類
	varId, funcId, parId, constId
}KindTable;

typedef struct relAddr
{		//変数、パラメタ、関数のアドレスの型
	int level;
	int addr;
}RelAddr; 

void TreatBlockBegin(int firstAddr);	//ブロックの始まり(最初の変数の番地)で呼ばれる
void TreatBlockEnd();			//ブロックの終りで呼ばれる
int iGetBlockLevel();				//現ブロックのレベルを返す
int GetFunctionParameterNum();				//現ブロックの関数のパラメタ数を返す
int RegisterFunction(char *id, int v);	//名前表に関数名と先頭番地を登録
int RegisterVarName(char *id);		//名前表に変数名を登録
int RegisterParameterName(char *id);		//名前表にパラメタ名を登録
int RegisterConstName(char *id, int v);	//名前表に定数名とその値を登録
void TreatParameterEnd();				//パラメタ宣言部の最後で呼ばれる
void changeV(int ti, int newVal);	//名前表[ti]の値（関数の先頭番地）の変更

int GetNameIndex(char *szIdD, KindTable k);	//名前idの名前表の位置を返す
						//未宣言の時エラーとする
KindTable GetKind(int i);			//名前表[i]の種類を返す

RelAddr relAddr(int ti);		//名前表[ti]のアドレスを返す
int val(int ti);				//名前表[ti]のvalueを返す
int GetParameterNum(int ti);				//名前表[ti]の関数のパラメタ数を返す
int frameL();				//そのブロックで実行時に必要とするメモリー容量

