#pragma once

#include <stdint.h>
#include <vector>

#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif

using namespace std;

// �r�b�O�G���f�B�A���̃��g���G���f�B�A�� ���ݕϊ�
int ls11_ChangeEndian(int iCng);

// *inData = �C���v�b�g�̃X�g���[���A�h���X, inlen = �Y���̃X�g���[���̒���
int ls11_memLocationDataArray(const char *inData, int inlen);

// *inData = �C���v�b�g�̃X�g���[���A�h���X, inlen = �Y���̃X�g���[���̒���
// char* outData outData�p�̃o�b�t�@, outData�p�̒���
int	ls11_Decode(const char *inData, int inlen, char *outData, int outlen);
int	ls11_Encode(const char *inData, int inlen, char *outData, int outlen);

int	ls11_getBit(const char *inData, int type);

int ls11_getFileData(const char *filename, char *out, int outlen);
int ls11_getFileSize(const char *filename);
int ls11_outFileData(const char *filename, char *out, int outlen);




// ���P�[�V�����f�[�^�P��
struct LS11LOCATIONDATA {
	int iDataLenSize;  // �Y���̃��P�[�V����(iStartAddress)����n�܂�f�[�^�̃o�C�g�T�C�Y
	int iExpandedSize; // �Y���̃��P�[�V�����f�[�^��W�J�����ꍇ�ɕK�v�ƂȂ�o�C�g�T�C�Y
	int iStartAddress; // �Y���̃��P�[�V�����̊J�n�A�h���X(�J�n�ʒu)
};

// *.nb6�͌��������̃t�@�C���ł��邪�A�v�Z���߂�ǂ������̂ŁA�P�̎����{���P�[�V�����f�[�^�{�O�{�f�[�^ �Ƃ����`�œ����Ă���Ƃ݂Ȃ��B
struct LS11ONEDATA {
	uint8_t dictionary[0x110];
	LS11LOCATIONDATA locationdata;
	int zero;
	uint8_t data[500000];
};

// �P��LS11�t�@�C���ɁA�����̃��P�[�V�����f�[�^�������Ă���B����̃��X�g���o���Ă����B�P�̃p�b�N�˃o���o���̃t�@�C���p
extern vector<LS11LOCATIONDATA> ls11_location_data_list;

extern LS11ONEDATA ls11_onedata;



/*
* ls11_out_pack_header
*         +
* ls11_out_loc_data_list
*         +
*      (int)0
*         +
* vPackOutDataArray
*
* �łP��LS11�p�b�N�t�@�C���ƂȂ�B
*/
struct LS11OUTPACKHEADER {
	uint8_t header[0x110];
};

extern LS11OUTPACKHEADER ls11_out_pack_header;

// �P��LS11�t�@�C���ɁA�����̃��P�[�V�����f�[�^�������Ă���B�o���o���̃t�@�C���ˏo�͗p
extern vector<LS11LOCATIONDATA> ls11_out_loc_data_list;

// �o���o���̃t�@�C�������ꂼ�ꕄ���������f�[�^�����̔z��B
extern vector<vector<uint8_t>> vPackOutDataArray;

