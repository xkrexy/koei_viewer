#include "ls11_mod.h"



// �Ӗ��P�ʂŕ������ꂽ�f�[�^���A���ւƂȂ�����Ԃɖ߂�
void ls11_JoinData(vector<vector<uint8_t>> &vSrcSplittedData, vector<uint8_t> *vDstJoinedData) {

	// ����
	vDstJoinedData->clear();

	/*
	 * �A�h���X�����̌v�Z�ƘA��
	 */
	// �܂��A�S�Ẵf�[�^�Ɋւ��āA�e�X�̃T�C�Y(�o�C�g��)�����߂�B
	// ��������ƂɁA�f�[�^�̍ŏ��ɁA�f�[�^�J�n�A�h���X�̔z�����������ł��܂�Ȃ���΂Ȃ�Ȃ��B
	// ���p�̂ɂ������̂ł͂Ȃ��āAint��byte�̃L���X�g�Ń\�[�X�������̉����
	union STARTADDRESS {
		uint16_t iCurDataStartAddress;
		uint8_t szCurDataStartAddress[2];
	};
	STARTADDRESS sa;
	sa.iCurDataStartAddress=0;

	// �ŏ��͑S�̂Ƃ��Ẵf�[�^�̌� * 2���A�h���X�̃X�^�[�g�n�_�ƂȂ�B�����̕����ɃA�h���X�̈ꗗ���P������WORD�̒P�ʂœ���B
	sa.iCurDataStartAddress += (uint16_t)vSrcSplittedData.size() * 2;
	vDstJoinedData->push_back( sa.szCurDataStartAddress[0] );
	vDstJoinedData->push_back( sa.szCurDataStartAddress[1] );

	// �Ō�̂P�́A�����K�v���Ȃ��̂ŁA-1�B�Ō�̂ЂƂ̓t�@�C���I�[�̃A�h���X�ƂȂ邪�A����́A�f�[�^�Ɋ܂߂�K�v���Ȃ��B
	for ( int i=0; i<(int)vSrcSplittedData.size()-1; i++) {
		sa.iCurDataStartAddress += (uint16_t)vSrcSplittedData[i].size();
		vDstJoinedData->push_back( sa.szCurDataStartAddress[0] );
		vDstJoinedData->push_back( sa.szCurDataStartAddress[1] );
	}

	/*
	 * �f�[�^�����̘A��
	 */
	for ( int i=0; i<(int)vSrcSplittedData.size(); i++ ) {
		vDstJoinedData->insert( vDstJoinedData->end(), vSrcSplittedData[i].begin(), vSrcSplittedData[i].end() ); 
	}

}
