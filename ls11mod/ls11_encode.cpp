
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "ls11_lib.h"

using namespace std;



int		ls11_Encode(const char *inData, int inlen, char *outData, int outlen);
int 	ls11_bitset(char *out,int data,int type);
int		ls11_bitout(char *out,int bit);

typedef struct {	//LZ77�����p(1���R�[�h)
	int point;
	int count;
}	LZ;

typedef struct {	//�n�t�}������(1���R�[�h)
	int no;
	int count;
}	DICTIONARY;

typedef union {		//INT�ϊ��{�b�N�X
	int i;
	char c[4];
	unsigned char uc[4];
}	INTBOX;


LS11OUTPACKHEADER ls11_out_pack_header;

// �P��LS11�t�@�C���ɁA�����̃��P�[�V�����f�[�^�������Ă���B�o���o���̃t�@�C���ˏo�͗p
vector<LS11LOCATIONDATA> ls11_out_loc_data_list;

// �o���o���̃t�@�C�������ꂼ�ꕄ���������f�[�^�����̔z��B
vector<vector<uint8_t>> vPackOutDataArray;

extern int ls11_setPoint;
extern int ls11_bitPoint;


/*///////////////////////////////////////////////////////////////////////////*/
//LS11���k
int ls11_Encode(const char *inData, int inlen, char *outData, int outlen){

	ls11_setPoint=0;
	ls11_bitPoint=0;

	LZ	*lzRecord;
	DICTIONARY dict[256];
	INTBOX len;
	int	lzLen;
	int i;

	/* LZ77���� */
	lzLen = inlen;
	lzRecord = (LZ *)calloc(lzLen , sizeof(LZ));
	if(lzRecord == NULL ){
		printf("�������̈�擾�G���[");
		fflush(stdin); 
		getchar(); 
		return -1;
	}
	for(i=0;i<lzLen;i++){
		lzRecord[i].point = 0;
		lzRecord[i].count = (int)(unsigned char)inData[i];
	}

	/* �n�t�}�������쐬 */
	for(i=0;i<256;i++){	//������
		dict[i].no    = i;
		dict[i].count = 0;
	}

	
	/* �t�@�C���w�b�_�������� */
	outData[0] = 'L';
	outData[1] = 'S';
	outData[2] = '1';
	outData[3] = '1';
	for(i=0;i<12;i++){	outData[i+4] = 0x00; }				//Padding
	for(i=0;i<4;i++){	outData[i+0x110] = 0x00; }			//���k��f�[�^���T�C�Y(��œ����)
	len.i = inlen;
	for(i=0;i<4;i++){	outData[i+0x114] = len.uc[3-i]; }	//���k�O�f�[�^���T�C�Y
	len.i = 0x120;
	for(i=0;i<4;i++){	outData[i+0x118] = len.uc[3-i]; }	//�f�[�^�J�n�ʒu
	for(i=0;i<4;i++){	outData[i+0x11C] = 0x00; }			//Padding
	ls11_bitset(NULL,0x120,1);	//�o�͐擪�ʒu�ύX(0x120)


	/* �n�t�}���o�͏��� */
	for(i=0;i<lzLen;i++){
		if(lzRecord[i].point){
			ls11_bitset(outData,(lzRecord[i].point +256),0);
		}
		ls11_bitset(outData,dict[lzRecord[i].count].no,0);
	}
	free(lzRecord);
	outlen = ls11_bitset(NULL,0,2);
	if(ls11_bitset(NULL,0,4)){ outlen++; }

	/* �W�J�p�n�t�}�������쐬 */
	for(i=0;i<256;i++){	
		dict[i].count = dict[i].no;
		dict[i].no    = i;
	}

	//	printf("�����e�[�u��\n");
//	for(i=0;i<256;i++){printf("%02X ",dict[i].no);if(i%16==15)printf("\n");}

	/* �t�@�C���w�b�_�������� */
	for(i=0;i<256;i++){ outData[i+0x10] = dict[i].no; }			//����
	len.i = outlen - 0x120;
	for(i=0;i<4;i++){	outData[i+0x110] = len.uc[3-i];}		//���k��f�[�^���T�C�Y

	return outlen;
}

int ls11_setPoint = 0;
int ls11_bitPoint = 0;

/* �n�t�}���o�͏��� */
int ls11_bitset(char *out,int data,int type){

	unsigned int MASK = 0x02;	//�}�X�N
	unsigned int m_count = 0;	//�Obit����
	unsigned int num;
	int i;

	/* ���ݒ� */
	if(type){
		switch(type){

			case 1:	ls11_setPoint = data;	//�o�͕����ʒu�ݒ�
			case 2:	return ls11_setPoint;
					break;

			case 3:	if(data > 8){data = 0;}	//�o�̓r�b�g�ʒu�ݒ�
					ls11_bitPoint = data;
			case 4:	return ls11_bitPoint;
					break;

			default:break;
		}
	}

	/* �r�b�g�o�͏��� */
	num = (unsigned int)data;
	while(num >= ((MASK << (m_count +1)) -2) ){	//�r�b�g���Z�o
		m_count++;
	}
//	printf("m_count:%d |num:%d ",m_count,num);
	num = num - ((MASK << m_count) -2);
//	printf("- %d = %d\n",(MASK << m_count) -2,num);

	for(i=m_count;i>=0;i--){	//�㕔�o��
//		printf("%d|%03d|",ls11_setPoint,(0x80 >> ls11_bitPoint));
		if(i){	ls11_bitout(&out[ls11_setPoint],ls11_bitPoint);}
//		else{printf("\n");}
		ls11_bitPoint++;
		if(ls11_bitPoint>7){
			ls11_setPoint++;
			ls11_bitPoint = 0;
		}
	}
	for(i=m_count;i>=0;i--){ //�����o��
//		printf("%d|%03d|",ls11_setPoint,(0x80 >> ls11_bitPoint));
		if( num & (0x1 << i)){	ls11_bitout(&out[ls11_setPoint],ls11_bitPoint);}
//		else{printf("\n");}
		ls11_bitPoint++;
		if(ls11_bitPoint>7){
			ls11_setPoint++;
			ls11_bitPoint = 0;
		}
	}
	return 0;
}

/* �r�b�g�o�͑���(�n���ꂽchar�̎w��r�b�g�𗧂Ă�) */
int ls11_bitout(char *out,int bit){

	unsigned char ch=0x80;
//	printf("%02x:%d:",(unsigned char)*out,bit);
	/* |0|1|2|3|4|5|6|7| */
	if(bit >= 0 || bit < 8){
		*out = *out | (ch >> bit);
//		printf("[%02x]%02x\n",(ch>>bit),(unsigned char)*out);
	}else{
		return -1;
	}
	return 0;
}
