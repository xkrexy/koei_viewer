#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ls11_lib.h"

using namespace std;


int ls11_EncodePack(char *szOutputFileName, char *szInputPattern, vector<vector<uint8_t>> *pvecSrcDataArray, vector<uint8_t> *pDstLS11PackedData) {

	ls11_out_loc_data_list.clear();
	vPackOutDataArray.clear();

	// �o���o���̂��ꂼ��̖��O
	char szCurFileName[256] = "";

	// �t�@�C����T���B
	for ( int iNo=0; iNo<3000; iNo++ ) {
		sprintf(szCurFileName, "%s/%s.%03d", szInputPattern, szInputPattern, iNo );

		int iTargetFileSize=0;
		int iKeepableMemSize=0;
		int iInputFileSize=0;
		int iOutputFileSize=0;
		char *inData=NULL;
		char *outData=NULL;
		/*
		printf("���̓t�@�C����:");
		scanf("%s",input);
		printf("�o�̓t�@�C����:");
		scanf("%s",output);
		*/
		// �t�@�C���̏ꍇ
		if ( !pvecSrcDataArray && szInputPattern )  {
			/* �t�@�C���T�C�Y�擾 */
			iInputFileSize = ls11_getFileSize(szCurFileName);
			if(iInputFileSize < 0){
				//fflush(stdin);
				// �t�@�C�����I�������B
				break;
			}
			inData = (char *)calloc(iInputFileSize , sizeof(char));
			if(inData == NULL ){
				//printf("�������̈�擾�G���[\n");
				//fflush(stdin); 
				return -1;
			}
			iInputFileSize = ls11_getFileData(szCurFileName, inData, iInputFileSize);
			if(iInputFileSize < 0){
				free(inData);
				//fflush(stdin);
				return -1;
			}
			// Vector�̏ꍇ
		} else if (pvecSrcDataArray) {
			if ( iNo >= (int)(*pvecSrcDataArray).size() ) { break; }
			iInputFileSize = (*pvecSrcDataArray)[iNo].size();
			inData = (char *)calloc(iInputFileSize , sizeof(char));
			memcpy(inData, (char *)&(*pvecSrcDataArray)[iNo][0], iInputFileSize);
		} else {
			return -1;
		}

		iKeepableMemSize = iInputFileSize * 5;
		outData = (char *)calloc(iKeepableMemSize, sizeof(char));
		if(outData == NULL ){
			free(inData);
			//printf("�������̈�擾�G���[\n");
			//fflush(stdin); 
			return -1;
		}

		iOutputFileSize = ls11_Encode(inData, iInputFileSize, outData, iKeepableMemSize);
		free(inData); // malloc

		if(iOutputFileSize < 0){
			// printf("�G���R�[�h���s:%d\n",fsize3);
			free(outData);
			//fflush(stdin); 
			return -1;
		}
		// �w�b�_�[�������R�s�[(����͂��́A�t�@�C�����ς���Ă��Œ�I)
		memcpy( &ls11_out_pack_header.header, outData, sizeof(ls11_out_pack_header.header));

		// ���P�[�V�����̕������R�s�[(�A�f�B�V����)
		LS11LOCATIONDATA *p_cur_loc_data = (LS11LOCATIONDATA *)(char *)(outData+0x110);
		// �͂��߂ĂȂ�΁A���̂܂ܑ��������B
		ls11_out_loc_data_list.push_back( *p_cur_loc_data );

		// �f�[�^��������C�ɃR�s�[
		vector<uint8_t> vCurLS11Buf(&outData[0x120], &outData[iOutputFileSize]);
		vPackOutDataArray.push_back(vCurLS11Buf);

		// �p���ς񂾂̂ŉ���B���̃p�b�N�����t�@�C����
		free(outData);
	}

	// �Ō�� ls11_out_loc_data_list �̈�ԍŏ��̃A�h���X���Ԉ���Ă���̂ŁA�ŏ��̃A�h���X���C�����A����Ɋ�Â��đS�ẴA�h���X���C������K�v������B
	for ( int loc=0; loc < (int)ls11_out_loc_data_list.size(); loc++ ) {
		if ( loc==0 ) {
			ls11_out_loc_data_list[0].iStartAddress = ls11_ChangeEndian(0x110 + 12*ls11_out_loc_data_list.size() + 4); // �����̃��P�[�V�����f�[�^���p�b�N����Ă�̂ŁA1�ڂ̈ʒu���ω�����B
		} else {
			// �P�O�̃A�h���X�{�P�O�̒���
			// ���f�[�^���r�b�O�G���f�B�A���Ȃ̂ŁA���g���G���f�B�A���ɂ��āA�v�Z�B
			int iStartAddress = ls11_ChangeEndian(ls11_out_loc_data_list[loc-1].iStartAddress) + ls11_ChangeEndian(ls11_out_loc_data_list[loc-1].iDataLenSize);
			// ���g���G���f�B�A�����r�b�O�G���f�B�A���ɖ߂�
			iStartAddress = ls11_ChangeEndian( iStartAddress );
			ls11_out_loc_data_list[loc].iStartAddress = iStartAddress;
		}
	}

	if ( pDstLS11PackedData ) {
		// �ꎞ�o�b�t�@���ǂ̂��炢�m�ۂ���̂��v�Z
		char *tmpBuf = NULL;
		int iCntBufSize = 0;
		for (int loc=0; loc<(int)ls11_out_loc_data_list.size(); loc++ ) {
			iCntBufSize += 12;
		}
		for (int buf=0; buf<(int)vPackOutDataArray.size(); buf++ ) {
			iCntBufSize += vPackOutDataArray[buf].size();
		}
		tmpBuf = (char *)calloc(iCntBufSize+1000, sizeof(char));

		char *pTmpBuf = tmpBuf;

		// �w�b�_�[�{������������
		memcpy( pTmpBuf, (char *)ls11_out_pack_header.header, sizeof(ls11_out_pack_header.header) );
		pTmpBuf += sizeof(ls11_out_pack_header.header);

		// ���P�[�V�����֘A��������
		for (int loc=0; loc<(int)ls11_out_loc_data_list.size(); loc++ ) {
			memcpy( pTmpBuf, (char *)(char *)&ls11_out_loc_data_list[loc].iDataLenSize, 12 );
			pTmpBuf += 12;
		}
		// ���P�[�V�����I���̂O�������݁B�S�o�C�g(int �P��)
		int iEnd = 0;
		memcpy( pTmpBuf, (char *)&iEnd, 4 );
		pTmpBuf += 4;

		// �f�[�^������������
		for (int buf=0; buf<(int)vPackOutDataArray.size(); buf++ ) {
			memcpy( pTmpBuf, (char *)&vPackOutDataArray[buf][0], vPackOutDataArray[buf].size() );
			pTmpBuf += vPackOutDataArray[buf].size();
		}

		// �����n���ꂽ������pDstLS11PackedData�ɔ��f
		int cpysize =  (int)pTmpBuf-(int)tmpBuf;
		(*pDstLS11PackedData).resize( cpysize );
		memcpy( (char *)&(*pDstLS11PackedData)[0], tmpBuf, cpysize );

		// �ꎞ�o�b�t�@�̉��
		free(tmpBuf);
	}

	if ( szOutputFileName ) {

		// �o�C�i���ŃI�[�v��
		FILE *fp = fopen(szOutputFileName, "wb");

		// �w�b�_�[�{������������
		fwrite( (char *)ls11_out_pack_header.header, sizeof(ls11_out_pack_header.header), 1, fp);

		// ���P�[�V�����֘A��������
		for (int loc=0; loc<(int)ls11_out_loc_data_list.size(); loc++ ) {
			fwrite( (char *)&ls11_out_loc_data_list[loc].iDataLenSize, 12, 1, fp);
		}
		// ���P�[�V�����I���̂O�������݁B�S�o�C�g(int �P��)
		int iEnd = 0;
		fwrite( (char *)&iEnd, 4, 1, fp );

		// �f�[�^������������
		for (int buf=0; buf<(int)vPackOutDataArray.size(); buf++ ) {
			fwrite( (char *)&vPackOutDataArray[buf][0], vPackOutDataArray[buf].size(), 1, fp );
		}
		fclose(fp);
	}

	return 0;
}


