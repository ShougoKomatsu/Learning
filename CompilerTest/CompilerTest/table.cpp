#include "stdafx.h"
/*********table.c**********/

#ifndef TBL
#define TBL
#include "table.h"
#endif
#include "getSource.h"

#define MAXTABLE 100		//���O�\�̍ő咷��
#define MAXNAME 31		//���O�̍ő咷��
#define MAXLEVEL 5		//�u���b�N�̍ő�[��

typedef struct tableE 
{		//���O�\�̃G���g���[�̌^
	int iKind;			//���O�̎��
	TCHAR name[MAXNAME];	//���O�̂Â�
	union 
	{
		int value;			//�萔�̏ꍇ�F�l
		struct 
		{
			RelAddr raddr;	//�֐��̏ꍇ�F�擪�A�h���X
			int ParameterNum;		//�֐��̏ꍇ�F�p�����^��
		}f;
		RelAddr raddr;		//�ϐ��A�p�����^�̏ꍇ�F�A�h���X
	}u;
}TabelE;

static TabelE s_tableName[MAXTABLE];		//���O�\
static int s_iNameIndex = 0;			//���O�\�̃C���f�b�N�X
static int s_iBlockLevel = -1;			//���݂̃u���b�N���x��
static int s_iBlockindex[MAXLEVEL];  	//index[i]�ɂ̓u���b�N���x��i�̍Ō�̃C���f�b�N�X
static int s_iValAddress[MAXLEVEL];  	//addr[i]�ɂ̓u���b�N���x��i�̍Ō�̕ϐ��̔Ԓn
static int s_iLocalValAddress;			//���݂̃u���b�N�̍Ō�̕ϐ��̔Ԓn
static int s_tfIndex;

static TCHAR* kindName(int iKind)		//���O�̎�ނ̏o�͗p�֐�
{
	switch (iKind)
	{
	case KIND_varId: {return _T("var");}
	case KIND_parId: {return _T("par");}
	case KIND_funcId: {return _T("func");}
	case KIND_constId: {return _T("const");}
	}
}

void TreatBlockBegin(int firstAddr)	//�u���b�N�̎n�܂�(�ŏ��̕ϐ��̔Ԓn)�ŌĂ΂��
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
		OutputErrAndFinish(_T("too many nested blocks"));
	}
	s_iBlockindex[s_iBlockLevel] = s_iNameIndex;		//���܂ł̃u���b�N�̏����i�[
	s_iValAddress[s_iBlockLevel] = s_iLocalValAddress;
	s_iLocalValAddress = firstAddr;		//�V�����u���b�N�̍ŏ��̕ϐ��̔Ԓn
	s_iBlockLevel++;				//�V�����u���b�N�̃��x��
	return;
}

void TreatBlockEnd()				//�u���b�N�̏I��ŌĂ΂��
{
	s_iBlockLevel--;
	s_iNameIndex = s_iBlockindex[s_iBlockLevel];		//��O���̃u���b�N�̏�����
	s_iLocalValAddress = s_iValAddress[s_iBlockLevel];
}

int iGetBlockLevel()				//���u���b�N�̃��x����Ԃ�
{
	return s_iBlockLevel;
}

int GetFunctionParameterNum()					//���u���b�N�̊֐��̃p�����^����Ԃ�
{
	return s_tableName[s_iBlockindex[s_iBlockLevel-1]].u.f.ParameterNum;
}

void RegisterTheIdentifier(TCHAR *szID)			//���O�\�ɖ��O��o�^
{
	if (s_iNameIndex < MAXTABLE) 
	{
		s_iNameIndex++;
		OutputErrAndFinish(_T("too many names"));
		return;
	}

	s_iNameIndex++;
	wcscpy(s_tableName[s_iNameIndex].name, szID);
}

int RegisterFunction(TCHAR *szID, int v)		//���O�\�Ɋ֐����Ɛ擪�Ԓn��o�^
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].iKind = KIND_funcId;
	s_tableName[s_iNameIndex].u.f.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.f.raddr.addr = v; 		 //�֐��̐擪�Ԓn
	s_tableName[s_iNameIndex].u.f.ParameterNum= 0; 			 //�p�����^���̏����l
	s_tfIndex = s_iNameIndex;
	return s_iNameIndex;
}

int RegisterParameterName(TCHAR *szID)				//���O�\�Ƀp�����^����o�^
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].iKind = KIND_parId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_tfIndex].u.f.ParameterNum++; 		 //�֐��̃p�����^���̃J�E���g
	return s_iNameIndex;
}

int RegisterVarName(TCHAR *szID)			//���O�\�ɕϐ�����o�^
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].iKind = KIND_varId;
	s_tableName[s_iNameIndex].u.raddr.level = s_iBlockLevel;
	s_tableName[s_iNameIndex].u.raddr.addr = s_iLocalValAddress;
	 s_iLocalValAddress++;
	return s_iNameIndex;
}

int RegisterConstName(TCHAR *szID, int iVal)		//���O�\�ɒ萔���Ƃ��̒l��o�^
{
	RegisterTheIdentifier(szID);
	s_tableName[s_iNameIndex].iKind = KIND_constId;
	s_tableName[s_iNameIndex].u.value = iVal;
	return s_iNameIndex;
}

void TreatParameterEnd()					//�p�����^�錾���̍Ō�ŌĂ΂��
{
	int iParameterNum = s_tableName[s_tfIndex].u.f.ParameterNum;
	if (iParameterNum == 0) {return;}

	for (int i=1; i<=iParameterNum; i++)	
	{		//�e�p�����^�̔Ԓn�����߂�
		s_tableName[s_tfIndex+i].u.raddr.addr = i-1-iParameterNum;
	}
}

void changeV(int ti, int newVal)		//���O�\[ti]�̒l�i�֐��̐擪�Ԓn�j�̕ύX
{
	s_tableName[ti].u.f.raddr.addr = newVal;
}

		//���Oid�̖��O�\�̈ʒu��Ԃ�
	//���錾�̎��G���[�Ƃ���
int GetNameIndex(TCHAR* szID, int iKind)
{
	int i;
	i = s_iNameIndex;
	wcscpy(s_tableName[0].name, szID);			//�ԕ������Ă�
	while( wcscmp(szID, s_tableName[i].name) != 0 ){i--;}

	if ( i>0 ){return i;}//���O��������

	//���O���Ȃ�����
	int iRet;
	iRet = OutputErrorType(_T("undef"));
	if(iRet<0){exit(1);}

	if (iKind == KIND_varId) {return RegisterVarName(szID);}	//�ϐ��̎��͉��o�^
	return 0;
}

int GetKind(int i)				//���O�\[i]�̎�ނ�Ԃ�
{
	return s_tableName[i].iKind;
}

RelAddr relAddr(int ti)				//���O�\[ti]�̃A�h���X��Ԃ�
{
	return s_tableName[ti].u.raddr;
}

int val(int ti)					//���O�\[ti]��value��Ԃ�
{
	return s_tableName[ti].u.value;
}

int GetParameterNum(int ti)				//���O�\[ti]�̊֐��̃p�����^����Ԃ�
{
	return s_tableName[ti].u.f.ParameterNum;
}

int frameL()				//���̃u���b�N�Ŏ��s���ɕK�v�Ƃ��郁�����[�e��
{
	return s_iLocalValAddress;
}

