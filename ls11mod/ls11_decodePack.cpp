#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/stat.h> // mkdir
#include "ls11_lib.h"

using namespace std;



int ls11_DecodePack(char *szInputFileName, char *szOutputPattern, vector<vector<uint8_t>> *pvecDstDataArray) {

	// �n���ꂽ�x�N�^�[�͔O�̂��߃N���A�[
	if ( pvecDstDataArray ) {
		pvecDstDataArray->clear();
	}

	int iTtargetFileSize=0;
	int iKeepableMemSize=0;
	int iOutputFileSize=0;
	char *inData=NULL;
	char *outData=NULL;
	/*
	printf("���̓t�@�C����:");
	scanf("%s",input);
	printf("�o�̓t�@�C����:");
	scanf("%s",output);
	*/
	/* �t�@�C���T�C�Y�擾 */
	iTtargetFileSize = ls11_getFileSize(szInputFileName);
	if(iTtargetFileSize < 0){
		fflush(stdin);
		return -1;
	}
	inData = (char *)calloc(iTtargetFileSize , sizeof(char));
	if(inData == NULL ){
		//printf("�������̈�擾�G���[\n");
		fflush(stdin); 
		return -1;
	}
	iTtargetFileSize = ls11_getFileData(szInputFileName, inData, iTtargetFileSize);
	if(iTtargetFileSize < 0){
		free(inData);
		fflush(stdin);
		return -1;
	}
	//printf("���f�[�^:%d\n",iTtargetFileSize);

	/* �W�J���� */
	int result = ls11_memLocationDataArray(inData, iTtargetFileSize);
	if ( result == -1 ) {
		free(inData);
		fflush(stdin);
		return -1;
	}

	// ���������͋��ʂȂ̂ŃR�s�[
	memcpy( &ls11_onedata.dictionary, inData, sizeof(ls11_onedata.dictionary) );
	ls11_onedata.zero = 0;

	// ���������ɂ���
	for (int i=0; i<(int)ls11_location_data_list.size(); i++ ) {
		// �T�C�Y�֘A�������R�s�[����
		memcpy(&ls11_onedata.locationdata, &ls11_location_data_list[i], sizeof(ls11_location_data_list) );
		memcpy(&ls11_onedata.data, inData+ls11_location_data_list[i].iStartAddress, ls11_location_data_list[i].iDataLenSize);

		//printf("�m�ۃT�C�Y:%d\n",ls11_location_data_list[i].iExpandedSize);

		if(ls11_location_data_list[i].iExpandedSize <= 0){
			//printf("�W�J��T�C�Y���擾�ł��܂���\n");
			//fflush(stdin);
			return -1;
		}

		outData = (char *)calloc(ls11_location_data_list[i].iExpandedSize+50000 , sizeof(char)); // ����50000�Đ��l�͕ʂ̂Ƃ���ŎQ�Ƃ��Ă���B

		if(outData == NULL ){
			free(inData);
			//printf("�������̈�擾�G���[\n");
			fflush(stdin); 
			return -1;
		}
		iKeepableMemSize = ls11_Decode((char *)&ls11_onedata, sizeof(ls11_onedata), outData, ls11_location_data_list[i].iExpandedSize );	
		//printf("�o�̓T�C�Y:%d\n",iKeepableMemSize);
		// �������m�ۂł��Ȃ�
		if(iKeepableMemSize < 0){
			//printf("�G���R�[�h���s:%d\n",iKeepableMemSize);
			fflush(stdin); 
			free(outData);
			return -1;
		}

		// �t�@�C���o�̓p�^�[�����킽���Ă��Ă���ꍇ�́A�t�@�C���o��
		if ( szOutputPattern ) {
			// �f�B���N�g���쐬
			mkdir(szOutputPattern, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			// �t�@�C�����쐬�B*****.001�@�Ȃ�
			char szTmpOutputPattern[256] = "";
			sprintf(szTmpOutputPattern, "%s/%s.%03d", szOutputPattern,szOutputPattern, i);
			printf(szTmpOutputPattern);

			/* �ϊ����ꂽ�f�[�^���o�� */
			iOutputFileSize = ls11_outFileData(szTmpOutputPattern, outData, ls11_location_data_list[i].iExpandedSize);
			// �o�͂Ɏ��s����
			if ( iOutputFileSize == -1 ) {
				free(outData);// malloc�ɑΉ�
				return -1;
			}
		}

		// �x�N�g�����X�g���킽���Ă��Ă�ꍇ�́A�x�N�g���ɏo��
		if ( pvecDstDataArray ) {
			// vector�Ɉꊇ�R�s�y
			vector<uint8_t> buf(&outData[0], &outData[ls11_location_data_list[i].iExpandedSize]);
			// �n���ꂽ�x�N�g���z���Append
			pvecDstDataArray->push_back(buf);
		}
		free(outData);// malloc�ɑΉ�
	}

	free(inData);

	//	}

	//printf("�ϊ��I��\n");
	//fflush(stdin);
	return 0;
}

/*
�g����

vector<vector<uint8_t>> v; �^�C�v��n�����ƂŁA�e�X��v�̒��ɁA���������e�o�C�i���t�@�C��(****.000, ****.001�̒��g)�����̂܂ܓ���

int main(int argc , char *argv[]){
	vector<vector<uint8_t>> vDataArray;
	ls11_DecodePack("hexgrp.nb6", "hexgrp" , &vDataArray);

	return 0;
}
*/