#include <stdio.h>
#include <stdlib.h>
#pragma pack(1)

#pragma warning(push)
#pragma warning(disable:4996)

//�t�@�C������f�[�^����
int 	ls11_getFileData(const char *filename, char *out, int outlen){
	FILE	*fp = NULL;
	long	fsize = 0;
	int		dsize = 0;

	//�t�@�C���I�[�v��
	if((fp = fopen(filename,"rb")) == NULL){
		printf("Input File Open Err");
		return -1;
	} 
	/* �t�@�C���T�C�Y�𒲍� */ 
	fseek(fp,0,SEEK_END); 
	fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	if(outlen > fsize){
		outlen = (int)fsize;
	}

	//�t�@�C���Ǎ�
	dsize = fread(out,sizeof(char),outlen,fp);
	//�t�@�C���N���[�Y
	fclose(fp);
	return dsize;
}

//�t�@�C���̃T�C�Y�擾
int 	ls11_getFileSize(const char *filename){
	FILE	*fp;
	long fsize = 0;
	
	if((fp = fopen(filename,"rb")) == NULL){
		printf("Input File Open Err");
		return -1;
	}
 
	/* �t�@�C���T�C�Y�𒲍� */ 
	fseek(fp,0,SEEK_END); 
	fsize = ftell(fp); 
 
	fclose(fp);
 
	return (int)fsize;
}

#pragma warning(pop)
