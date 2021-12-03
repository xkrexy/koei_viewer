#include "ls11_mod.h"


// LS11�n�𕪉����āA�P�P�̃t�@�C���ɂ������A�Ȃ������̃t�@�C�����A������o�C�i���f�[�^�ł���B
// ���̃o�C�i���f�[�^�́A���̂܂܂��ƁA�e�f�[�^�̒�����ς��Ă͂����Ȃ��̂��B
// ����āA������Ӗ��P�ʂŕ������A�e�X�̒����̕ύX�𔺂��悤�ȏC�����\�Ƃ���B
// ls11_SplitData�������̂́Als11_JoinData����΁A�A�����邱�Ƃ��\���B
void ls11_SplitData(vector<byte> &vSrcJoinedData, vector<vector<byte>> *vDstSplittedData) {

	// ����
	vDstSplittedData->clear();

	// �_�~�[�f�[�^���Ō�ɂ�������B
	// �̕��ŁA &vOrgData[vAddressList[i+1]] �ƃA�N�Z�X���Ă�̂��A�͈͊O�A�N�Z�X�G���[�ɂȂ�Ȃ��悤�ɂ��邽�߁B
	vSrcJoinedData.push_back( byte(0) );

	uint16_t *pAddressCnt = (uint16_t *)&vSrcJoinedData[0];
	uint16_t iAddressCnt = (*pAddressCnt) / 2;   // �P�̃A�h���X������2�o�C�g�Ȃ̂ŁA�ŏ��̃f�[�^�ʒu�w��A�h���X�ł���A&vOrgData[0] ���Q�Ŋ���ƁA�A�h���X�w�肪������̂����Z�o�ł���B

	// �e�f�[�^�J�n�A�h���X�̈ꗗ
	vector<uint16_t> vAddressList;
	for (int i=0; i<iAddressCnt; i++) {
		vAddressList.push_back( pAddressCnt[i] );
	}
	// �t�@�C���̒��������邱�ƂŁA�Ō�̃A�h���X�Ƃ���B
	// ������A����vDataList���v�f���P�傫���Ȃ�̂ŁA����vAddressList.size()���ȍ~�̃X�e�[�g�����g�Ɏg��Ȃ�����!!
	vAddressList.push_back( vSrcJoinedData.size()-1 ); // �_�~�[�f�[�^�̕��}�C�i�X1���Ă�B
	

	// �e�f�[�^�̈ꗗ
	for (int i=0; i<iAddressCnt; i++) {
		// vOrgData�̎w��̃A�h���X�`���̎w��̃A�h���X�܂ł��P�̃f�[�^�Ƃ��Ċi�[����B
		vector<byte> vData( &vSrcJoinedData[vAddressList[i]], &vSrcJoinedData[vAddressList[i+1]]); 
		vDstSplittedData->push_back( vData );
	}

	// �ŏ��ɂ��������_�~�[�f�[�^������
	vSrcJoinedData.pop_back();
}

/*
void main (void) {
	ifstream ifs;
	size_t iFileSize;

	ifs.open("C:\\bfile\\bfile.001", ios::binary);
	iFileSize = (size_t)ifs.seekg(0, std::ios::end).tellg();
	ifs.seekg(0, std::ios::beg);
	
	byte tmp[10000];
    
    ifs.read( ( char * ) tmp, iFileSize );

	vector<byte> v(&tmp[0], &tmp[iFileSize]);

	vector<vector<byte>> vSplittedData;
	ls11_SplitData( v, &vSplittedData );

	char *test = (char *)&vSplittedData[3][0];

	// ���炩�̕ύX������ۂɂ́A��x�Y����vector�v�f���N���A���������ǂ��B
	// �����łȂ��ƁA�������Z���Ȃ�Ȃ����낤�B

	vector<byte> vv;
	ls11_JoinData( vSplittedData, &vv );

	for ( int i=0; i<8000; i++ ) {
		if (tmp[i] != vv[i]) {
			MessageBox(NULL, "OK", "OK", NULL);
		}
	}

	return;
}
*/