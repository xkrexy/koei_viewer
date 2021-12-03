#include <stdio.h>
#include "ls11mod/ls11_mod.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <ls11 filename> <output directory>\n", argv[0]);
		return 0;
	}

	int ret = ls11_DecodePack(argv[1], argv[2], NULL);

	printf("Result: %d\n", ret);

	return 0;
}

