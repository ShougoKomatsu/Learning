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

static TabelE s_tableName[MAXTABLE];		//���O�\
static int s_iNameIndex = 0;			//���O�\�̃C���f�b�N�X
static int s_iBlockLevel = -1;			//���݂̃u���b�N���x��
static int s_iBlockindex[MAXLEVEL];   	//index[i]�ɂ̓u���b�N���x��i�̍Ō�̃C���f�b�N�X
static int s_iValAddress[MAXLEVEL];    	//addr[i]�ɂ̓u���b�N���x��i�̍Ō�̕ϐ��̔Ԓn
static int s_iLocalValAddress;			//���݂̃u���b�N�̍Ō�̕ϐ��̔Ԓn
static int s_tfIndex;

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
	if (s_iBlockLevel == -1)
	{			//��u���b�N�̎��A�����ݒ�
		s_iLocalValAddress = firstAddr;
		s_iNameIndex = 0;
		s_iBlockLevel++;
		return;
	}
	if (s_iBlockLevel == MAXLEVEL-1)
	{
		errorF("too many nested blocks");
	}
	s_iBlockindex[s_iBlockLevel] = s_iNameIndex;		//���܂ł̃u���b�N�̏����i�[
	s_iValAddress[s_iBlockLevel] = s_iLocalValAddress;
	s_iLocalValAddress = firstAddr;		//�V�����u���b�N�̍ŏ��̕ϐ��̔Ԓn
	s_iBlockLevel++;				//�V�����u���b�N�̃��x��
	return;
}

void blockEnd()				//�u���b�N�̏I��ŌĂ΂��
{
	s_iBlockLevel--;
	s_iNameIndex = s_iBlockindex[s_iBlockLevel];		//��O���̃u���b�N�̏�����
	s_iLocalValAddress = s_iValAddress[s_iBlockLevel];
}

int bLevel()				//���u���b�N�̃��x����Ԃ�
{
	return s_iBlockLevel;
}

int fPars()					//���u���b�N�̊֐��̃p�����^����Ԃ�
{
	return s_tableName[s_iBlockindex[s_iBlockLevel-1]].u.f.pars;
}

void enterT(char *id)			//���O�\�ɖ��O��o�^
{
	if (s_iNameIndex < MAXTABLE)
	{
		s_iNameIndex++;
		strcpy(s_tableName[s_iNameIndex].name, id);
	}
	else 
	{
		s_iNameIndex++;
		errorF("too many names");
	}
}

int enterTfunc(char *id, int v)		//���O�\�Ɋ֐����Ɛ擪�Ԓn��o�^
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = funcId;
	s_tableName[s_iNameIndex].u.f.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.f.raddr.addr = v;  		 //�֐��̐擪�Ԓn
	s_tableName[s_iNameIndex].u.f.pars = 0;  			 //�p�����^���̏����l
	s_tfIndex = s_iNameIndex;
	return s_iNameIndex;
}

int enterTpar(char *id)				//���O�\�Ƀp�����^����o�^
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = parId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_tfIndex].u.f.pars++;  		 //�֐��̃p�����^���̃J�E���g
	return s_iNameIndex;
}

int enterTvar(char *id)			//���O�\�ɕϐ�����o�^
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = varId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.raddr.addr = s_iLocalValAddress;
	 s_iLocalValAddress++;
	return s_iNameIndex;
}

int enterTconst(char *id, int v)		//���O�\�ɒ萔���Ƃ��̒l��o�^
{
	enterT(id);
	s_tableName[s_iNameIndex].kind = constId;
	s_tableName[s_iNameIndex].u.value = v;
	return s_iNameIndex;
}

void endpar()					//�p�����^�錾���̍Ō�ŌĂ΂��
{
	int i;
	int pars = s_tableName[s_tfIndex].u.f.pars;
	if (pars == 0)  {return;}
	for (i=1; i<=pars; i++)	
	{		//�e�p�����^�̔Ԓn�����߂�
		s_tableName[s_tfIndex+i].u.raddr.addr = i-1-pars;
	}
}

void changeV(int ti, int newVal)		//���O�\[ti]�̒l�i�֐��̐擪�Ԓn�j�̕ύX
{
	s_tableName[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindTable k)		//���Oid�̖��O�\�̈ʒu��Ԃ�
	//���錾�̎��G���[�Ƃ���
{
	int i;
	i = s_iNameIndex;
	strcpy(s_tableName[0].name, id);			//�ԕ������Ă�
	while( strcmp(id, s_tableName[i].name) ){i--;}

	if ( i ){return i;}//���O��������
		
	//���O���Ȃ�����
	errorType("undef");
	if (k==varId) {return enterTvar(id);}	//�ϐ��̎��͉��o�^
	return 0;
}

KindTable kindT(int i)				//���O�\[i]�̎�ނ�Ԃ�
{
	return s_tableName[i].kind;
}

RelAddr relAddr(int ti)				//���O�\[ti]�̃A�h���X��Ԃ�
{
	return s_tableName[ti].u.raddr;
}

int val(int ti)					//���O�\[ti]��value��Ԃ�
{
	return s_tableName[ti].u.value;
}

int pars(int ti)				//���O�\[ti]�̊֐��̃p�����^����Ԃ�
{
	return s_tableName[ti].u.f.pars;
}

int frameL()				//���̃u���b�N�Ŏ��s���ɕK�v�Ƃ��郁�����[�e��
{
	return s_iLocalValAddress;
}

