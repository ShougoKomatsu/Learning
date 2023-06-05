
 /***********table.h***********/

typedef enum kindTable 
{		//Identifier�̎��
	varId, funcId, parId, constId
}KindTable;

typedef struct relAddr
{		//�ϐ��A�p�����^�A�֐��̃A�h���X�̌^
	int level;
	int addr;
}RelAddr; 

void TreatBlockBegin(int firstAddr);	//�u���b�N�̎n�܂�(�ŏ��̕ϐ��̔Ԓn)�ŌĂ΂��
void TreatBlockEnd();			//�u���b�N�̏I��ŌĂ΂��
int iGetBlockLevel();				//���u���b�N�̃��x����Ԃ�
int GetFunctionParameterNum();				//���u���b�N�̊֐��̃p�����^����Ԃ�
int RegisterFunction(char *id, int v);	//���O�\�Ɋ֐����Ɛ擪�Ԓn��o�^
int RegisterVarName(char *id);		//���O�\�ɕϐ�����o�^
int RegisterParameterName(char *id);		//���O�\�Ƀp�����^����o�^
int RegisterConstName(char *id, int v);	//���O�\�ɒ萔���Ƃ��̒l��o�^
void TreatParameterEnd();				//�p�����^�錾���̍Ō�ŌĂ΂��
void changeV(int ti, int newVal);	//���O�\[ti]�̒l�i�֐��̐擪�Ԓn�j�̕ύX

int GetNameIndex(char *szIdD, KindTable k);	//���Oid�̖��O�\�̈ʒu��Ԃ�
						//���錾�̎��G���[�Ƃ���
KindTable GetKind(int i);			//���O�\[i]�̎�ނ�Ԃ�

RelAddr relAddr(int ti);		//���O�\[ti]�̃A�h���X��Ԃ�
int val(int ti);				//���O�\[ti]��value��Ԃ�
int GetParameterNum(int ti);				//���O�\[ti]�̊֐��̃p�����^����Ԃ�
int frameL();				//���̃u���b�N�Ŏ��s���ɕK�v�Ƃ��郁�����[�e��

