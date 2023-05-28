#include "stdafx.h"
/*********table.c**********/

#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		//���O�\�̍ő咷��
#define MAXNAME  31		//���O�̍ő咷��
#define MAXLEVEL 5		//�u���b�N�̍ő�[��

typedef struct tableE 
{		//���O�\�̃G���g���[�̌^
	KindTable kind;			//���O�̎��
	char name[MAXNAME];	//���O�̂Â�
	union 
	{
		int value;			//�萔�̏ꍇ�F�l
		struct 
		{
			RelAddr raddr;	//�֐��̏ꍇ�F�擪�A�h���X
			int pars;		//�֐��̏ꍇ�F�p�����^��
		}f;
		RelAddr raddr;		//�ϐ��A�p�����^�̏ꍇ�F�A�h���X
	}u;
}TabelE;

static TabelE nameTable[MAXTABLE];		//���O�\
static int tIndex = 0;			//���O�\�̃C���f�b�N�X
static int level = -1;			//���݂̃u���b�N���x��
static int index[MAXLEVEL];   	//index[i]�ɂ̓u���b�N���x��i�̍Ō�̃C���f�b�N�X
static int addr[MAXLEVEL];    	//addr[i]�ɂ̓u���b�N���x��i�̍Ō�̕ϐ��̔Ԓn
static int localAddr;			//���݂̃u���b�N�̍Ō�̕ϐ��̔Ԓn
static int tfIndex;

static char* kindName(KindTable k)		//���O�̎�ނ̏o�͗p�֐�
{
	switch (k)
	{
	case varId: {return "var";}
	case parId: {return "par";}
	case funcId: {return "func";}
	case constId: {return "const";}
	}
}

void blockBegin(int firstAddr)	//�u���b�N�̎n�܂�(�ŏ��̕ϐ��̔Ԓn)�ŌĂ΂��
{
	if (level == -1)
	{			//��u���b�N�̎��A�����ݒ�
		localAddr = firstAddr;
		tIndex = 0;
		level++;
		return;
	}
	if (level == MAXLEVEL-1)
	{
		errorF("too many nested blocks");
	}
	index[level] = tIndex;		//���܂ł̃u���b�N�̏����i�[
	addr[level] = localAddr;
	localAddr = firstAddr;		//�V�����u���b�N�̍ŏ��̕ϐ��̔Ԓn
	level++;				//�V�����u���b�N�̃��x��
	return;
}

void blockEnd()				//�u���b�N�̏I��ŌĂ΂��
{
	level--;
	tIndex = index[level];		//��O���̃u���b�N�̏�����
	localAddr = addr[level];
}

int bLevel()				//���u���b�N�̃��x����Ԃ�
{
	return level;
}

int fPars()					//���u���b�N�̊֐��̃p�����^����Ԃ�
{
	return nameTable[index[level-1]].u.f.pars;
}

void enterT(char *id)			//���O�\�ɖ��O��o�^
{
	if (tIndex++ < MAXTABLE)
	{
		strcpy(nameTable[tIndex].name, id);
	}
	else 
	{
		errorF("too many names");
	}
}

int enterTfunc(char *id, int v)		//���O�\�Ɋ֐����Ɛ擪�Ԓn��o�^
{
	enterT(id);
	nameTable[tIndex].kind = funcId;
	nameTable[tIndex].u.f.raddr.level = level;
	nameTable[tIndex].u.f.raddr.addr = v;  		 //�֐��̐擪�Ԓn
	nameTable[tIndex].u.f.pars = 0;  			 //�p�����^���̏����l
	tfIndex = tIndex;
	return tIndex;
}

int enterTpar(char *id)				//���O�\�Ƀp�����^����o�^
{
	enterT(id);
	nameTable[tIndex].kind = parId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tfIndex].u.f.pars++;  		 //�֐��̃p�����^���̃J�E���g
	return tIndex;
}

int enterTvar(char *id)			//���O�\�ɕϐ�����o�^
{
	enterT(id);
	nameTable[tIndex].kind = varId;
	nameTable[tIndex].u.raddr.level = level;
	nameTable[tIndex].u.raddr.addr = localAddr;
	 localAddr++;
	return tIndex;
}

int enterTconst(char *id, int v)		//���O�\�ɒ萔���Ƃ��̒l��o�^
{
	enterT(id);
	nameTable[tIndex].kind = constId;
	nameTable[tIndex].u.value = v;
	return tIndex;
}

void endpar()					//�p�����^�錾���̍Ō�ŌĂ΂��
{
	int i;
	int pars = nameTable[tfIndex].u.f.pars;
	if (pars == 0)  {return;}
	for (i=1; i<=pars; i++)	
	{		//�e�p�����^�̔Ԓn�����߂�
		nameTable[tfIndex+i].u.raddr.addr = i-1-pars;
	}
}

void changeV(int ti, int newVal)		//���O�\[ti]�̒l�i�֐��̐擪�Ԓn�j�̕ύX
{
	nameTable[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindTable k)		//���Oid�̖��O�\�̈ʒu��Ԃ�
	//���錾�̎��G���[�Ƃ���
{
	int i;
	i = tIndex;
	strcpy(nameTable[0].name, id);			//�ԕ������Ă�
	while( strcmp(id, nameTable[i].name) ){i--;}

	if ( i ){return i;}//���O��������
		
	//���O���Ȃ�����
	errorType("undef");
	if (k==varId) {return enterTvar(id);}	//�ϐ��̎��͉��o�^
	return 0;
}

KindTable kindT(int i)				//���O�\[i]�̎�ނ�Ԃ�
{
	return nameTable[i].kind;
}

RelAddr relAddr(int ti)				//���O�\[ti]�̃A�h���X��Ԃ�
{
	return nameTable[ti].u.raddr;
}

int val(int ti)					//���O�\[ti]��value��Ԃ�
{
	return nameTable[ti].u.value;
}

int pars(int ti)				//���O�\[ti]�̊֐��̃p�����^����Ԃ�
{
	return nameTable[ti].u.f.pars;
}

int frameL()				//���̃u���b�N�Ŏ��s���ɕK�v�Ƃ��郁�����[�e��
{
	return localAddr;
}

