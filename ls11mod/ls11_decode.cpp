
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ls11_lib.h"

using namespace std;


vector<LS11LOCATIONDATA> ls11_location_data_list;

LS11ONEDATA ls11_onedata;
 



// �r�b�O�G���f�B�A���̃��g���G���f�B�A��
int ls11_ChangeEndian(int iCng) {
	iCng = iCng<<24 | iCng<<8 & 0x00ff0000 | iCng>>8 & 0x0000ff00 | iCng>>24 & 0x000000ff;
	return iCng;
}


int ls11_memLocationDataArray(const char *inData, int inlen) {
	if(strncmp(inData,"LS11",4)){	/* �w�b�_���� */
		printf("�t�@�C���w�b�_�[���Ⴂ�܂��B\n");
		return -1;
	};

	ls11_location_data_list.clear(); // ���ׂăN���A

	LS11LOCATIONDATA *pLS11SizeData;

	LS11LOCATIONDATA curLS11SizeData;

	// �܂�3000�ȏ㕪������܂��I�Ȋ����ŁB
	for (pLS11SizeData = (LS11LOCATIONDATA *)(char *)(inData+0x110); // 0x110�̃X�^�[�g�ʒu����A
		 pLS11SizeData->iDataLenSize;								 // �f�[�^�����O�łȂ��Ǝw�肳��Ă���B
		 pLS11SizeData++) {

		// ���ꂼ����r�b�O�G���f�B�A���˃��g���G���f�B�A���ɂ��Ċi�[
		curLS11SizeData.iDataLenSize  = ls11_ChangeEndian( pLS11SizeData->iDataLenSize );
		curLS11SizeData.iExpandedSize = ls11_ChangeEndian( pLS11SizeData->iExpandedSize );
		curLS11SizeData.iStartAddress = ls11_ChangeEndian( pLS11SizeData->iStartAddress );

		// ���X�g�ɒǉ�
		ls11_location_data_list.push_back( curLS11SizeData );

	}

	return 0;
}

extern int ls11_bit;
extern int ls11_byte;

//LS11�L��
int		ls11_Decode(const char *inData, int inlen, char *outData, int outlen){
	
	ls11_bit = 0;
	ls11_byte = 0;

	char	dict[256]  = "";		/* ���� */
	int		dataSize   = 0;			/* �f�[�^���T�C�Y */
	int		writePoint = 0;			/* �������݈ʒu */
	int		serchPoint = 0;			/* �R�s�[���擪�ʒu */
	int		i,j,k;					/* ��Ɨp */
	int		bitcount = 0;			/* �ǂݎ��r�b�g�� */
	int		tempNum = 0;			/* ���l�ǂݎ��� */
	int		checkNum = 0;			/* ����or�R�s�[�̔���p */
	int 	isGet1 = TRUE;			/* �ǂݎ�菈������ */
	int		isLz   = FALSE;			/* �R�s�[�������� */


	if(strncmp(inData,"LS11",4)){	/* �w�b�_���� */
		//printf("�t�@�C���w�b�_�[���Ⴂ�܂��B");
		return -1;
	}

	for(i=0;i<256;i++){
		dict[i] = inData[0x10+i];
//		printf("%02X ",(unsigned char)dict[i]);
//		if(i%16 == 15){printf("\n");}
	}	/* �����o�^ */


	/* �o�C�g�o�͈ʒu�ݒ�(0x120~) */
	ls11_getBit((const char*)0,1);
	ls11_getBit((const char*)0,2);
	ls11_getBit((const char*)255,2);
	ls11_getBit((const char*)33,2);
//	printf("Bit :%x\n",ls11_getBit(dict,3));
//	printf("Byte:%x\n",ls11_getBit(dict,4));
//	printf("data:%x\n",(unsigned char)inData[ls11_getBit(dict,4)]);

	for(i=0x120;i<inlen;i++){
		for(j=0;j<8;j++){
			tempNum = tempNum << 1;
			if(isGet1){	/* �O�̃r�b�g���擾 */
				bitcount++;
				if(	ls11_getBit(inData,0) ){
					tempNum++;
				}else{
					isGet1 = FALSE;
					checkNum = tempNum;
					tempNum = 0;
				}
				continue;
			}else{	/* ���̃r�b�g���擾 */
				bitcount--;
				if( ls11_getBit(inData,0) ){ tempNum++;}
				if(bitcount>0){	continue;}
			}
			checkNum += tempNum;
			tempNum   = 0;
			isGet1 = TRUE;

			if(isLz){	/* LZ���������� */
				checkNum += 3;
				for(k=0;k<checkNum;k++){
					outData[writePoint] = (unsigned char)outData[serchPoint];
					writePoint++;
					serchPoint++;
					if(writePoint>outlen+50000){ // �������͂͂ݏo�����Ƃ�����̂ŁA�p�b�`
						return -2;
					}
				}
				isLz = FALSE;
			}else{		/* ������������ */
				if(checkNum == 256){
					return -3;	/* �Q�ƈʒu�G���[ */
				}
				if(checkNum < 256){	/* �{���� */
					outData[writePoint] = dict[checkNum];
					writePoint++;
					if(writePoint>outlen){
						return outlen; 
					}
					
				}else{	/* LZ�������� */
					serchPoint = writePoint - (checkNum - 256);
					if(inData-serchPoint<0){
						printf("err");return -5;
					}
					isLz = TRUE;
				}
			}
		}
	}
	return writePoint;
}

int ls11_bit  = 0;
int ls11_byte = 0;

int	ls11_getBit(const char *inData, int type){

	/*	type0 ���̃r�b�g���o��
		type1 �o�̓r�b�g��ݒ�(0:���Z�b�g) inDaga[1~7]:���Z [8~255]:���Z�b�g
		type2 �o�̓o�C�g��ݒ�(0:���Z�b�g) inData[1~255]:���Z
		type3 ���r�b�g�ڂ����o��
		tyoe4 ���o�C�g�ڂ����o��
	*/
	int pick = 0;

	if(type){
		if(type & 1){
			if(type == 1){
				if(inData == 0){
					ls11_bit = 0;
				}else{
					ls11_bit += (int)(unsigned char)inData;
				}
			}
			return ls11_bit;
		}else{
			if(type == 2){
				if(inData == 0){
					ls11_byte = 0;
				}else{
					ls11_byte += (int)(unsigned char)inData;
				}
			}
			return ls11_byte;
		}
	}
	/* �r�b�g�̏o�� */
	pick = 0x80 >> ls11_bit;
	pick = inData[ls11_byte] & pick;
	/* �㏈�� */
	ls11_bit++;
	if(ls11_bit > 7){
		ls11_bit = 0;
		ls11_byte++;
	}
	if(pick){
		return 1;
	}else{
		return 0;
	}
	return -1;
}
