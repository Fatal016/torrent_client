#include <stdio.h>
#include <stdlib.h>

#include "../Inc/bencode.h"

int main(int argc, char **argv) {
	
	if (argc != 2) {
		printf("passed wrong amount\n");
		exit(-1);
	}
	
	char *filepath = argv[1];

	struct bencode_module bencode;
	parse_single(filepath, &bencode);

	printf("Announce: %s", bencode.announce);

	return 1;
}
