
 /***********table.h***********/

typedef enum kindT {		/*�@Identifier�̎�ށ@*/
	varId, funcId, parId, constId
}KindT;

typedef struct relAddr{		/*�@�ϐ��A�p�����^�A�֐��̃A�h���X�̌^�@*/
	int level;
	int addr;
}RelAddr; 

void blockBegin(int firstAddr);	/*�@�u���b�N�̎n�܂�(�ŏ��̕ϐ��̔Ԓn)�ŌĂ΂��@*/
void blockEnd();			/*�@�u���b�N�̏I��ŌĂ΂��@*/
int bLevel();				/*�@���u���b�N�̃��x����Ԃ��@*/
int fPars();				/*�@���u���b�N�̊֐��̃p�����^����Ԃ��@*/
int enterTfunc(char *id, int v);	/*�@���O�\�Ɋ֐����Ɛ擪�Ԓn��o�^�@*/
int enterTvar(char *id);		/*�@���O�\�ɕϐ�����o�^�@*/
int enterTpar(char *id);		/*�@���O�\�Ƀp�����^����o�^�@*/
int enterTconst(char *id, int v);	/*�@���O�\�ɒ萔���Ƃ��̒l��o�^�@*/
void endpar();				/*�@�p�����^�錾���̍Ō�ŌĂ΂��@*/
void changeV(int ti, int newVal);	/*�@���O�\[ti]�̒l�i�֐��̐擪�Ԓn�j�̕ύX�@*/

int searchT(char *id, KindT k);	/*�@���Oid�̖��O�\�̈ʒu��Ԃ��@*/
						/*�@���錾�̎��G���[�Ƃ���@*/
KindT kindT(int i);			/*�@���O�\[i]�̎�ނ�Ԃ��@*/

RelAddr relAddr(int ti);		/*�@���O�\[ti]�̃A�h���X��Ԃ��@*/
int val(int ti);				/*�@���O�\[ti]��value��Ԃ��@*/
int pars(int ti);				/*�@���O�\[ti]�̊֐��̃p�����^����Ԃ��@*/
int frameL();				/*�@���̃u���b�N�Ŏ��s���ɕK�v�Ƃ��郁�����[�e�ʁ@*/

