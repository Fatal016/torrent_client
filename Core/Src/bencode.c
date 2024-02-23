#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../Inc/bencode.h"

#define BUFFER_SIZE 128
#define ANNOUNCE_LIST_SIZE 10
#define INFO_FILE_SIZE 5

#define THREAD_NUM 1

int main(int argc, char **argv) {	

	if (argc != 2) {
		printf("passed wrong amount\n");
		exit(-1);
	}

	char *filepath = argv[1];
	
	parse_single(filepath);
}

int parse_single(char *filepath) {

	FILE *file = fopen(filepath, "r");
	char file_char;

	int result;

	struct bencode_module bencode;
	bencode.head_pointer = NULL;
	bencode.announce_list = (char **)malloc(ANNOUNCE_LIST_SIZE * sizeof(char *));
	bencode.announce_list_index = 0;
	bencode.info_file_index = 0;

	
	id type;

	/* Error checking for existence of file */
	if (file == NULL) {
		fprintf(stderr, "Error reading from file: File not found\n");
		return -1;
	}
	
	file_char = fgetc(file);
	type = identify(file_char);
	
	/* Should improve to look just for dictionary */
	if (type == NULL) {
		printf("Parse error: First character was not the beginning of a dictionary\n");
		return -1;
	}
	
	result = type(&bencode, file);
	
	if (result == 0) {
		printf("Dictionary parsed!\n");
		return 0;
	} else {
		printf("Error in parsing dictionary\n");
		return -1;
	}
}


void allocate(struct bencode_module *bencode, char **m) {
	*m = (char *)calloc(BUFFER_SIZE, sizeof(char));
	bencode->head_pointer = m;
}

/* Can likely malloc everything since im putting in the null character anyway? */


int dictionary(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index;
	int result;
	size_t length;

	char file_char = '\0';
	char buffer[BUFFER_SIZE];
	char compare_buffer[BUFFER_SIZE];

	id type;

	/* Will need to change this like list to avoid accidental exit from dict parse */
	while (file_char != 'e' && !feof(file)) {
		for (buffer_index = 0; buffer_index < BUFFER_SIZE; buffer_index++) {
			file_char = fgetc(file);
		
			type = identify(file_char);
			
			if (type != NULL) {
				result = type(bencode, file);
				bencode->head_pointer = NULL;
				buffer_index = -1;
			} else {
				if (file_char == ':') {
					buffer[buffer_index] = '\0';
					length = atoi(buffer);	

					result = fread(compare_buffer, 1, length, file);
					compare_buffer[length] = '\0';

					if (result != (int)length) {	
						printf("Parse error: Could not capture full segment. Verify the integrity of your .torrent file\n");
						return -1;
					}

					if (bencode->head_pointer == NULL) {
						if (strcmp(compare_buffer, "announce") == 0) {
							
							bencode->announce = (char *)malloc(sizeof(BUFFER_SIZE * sizeof(char)));
							bencode->head_pointer = (void *)&bencode->announce;

							//allocate(bencode, &bencode->announce);
						

						} else if (strcmp(compare_buffer, "announce-list") == 0) {
							
							bencode->announce_list[bencode->announce_list_index] = (char *)malloc(sizeof(BUFFER_SIZE * sizeof(char)));
							bencode->head_pointer = (void *)&bencode->announce_list;
							bencode->index_pointer = &bencode->announce_list_index;

							//allocate(bencode, &bencode->announce_list[bencode->announce_list_index]);	
							//bencode->index_pointer = &bencode->announce_list_index;
						} else {
							printBencode(bencode, &bencode->announce_list_index);	
							exit(0);
						}
/*
else if (strcmp(compare_buffer, "comment") == 0) {
							allocate(bencode, &bencode->comment);
						} else if (strcmp(compare_buffer, "created by") == 0) {
							allocate(bencode, &bencode->created_by);
						} else if (strcmp(compare_buffer, "creation date") == 0) {
							bencode->creation_date = (time_t *)malloc(sizeof(time_t *));
							bencode->head_pointer = (char **)&bencode->creation_date;							
						} else if (strcmp(compare_buffer, "encoding") == 0) {
							allocate(bencode, &bencode->encoding);
						} else if (strcmp(compare_buffer, "info") == 0) {
							bencode->info = (struct bencode_info *)malloc(sizeof(struct bencode_info *));
							bencode->head_pointer = (char **)&bencode->info;
						} else if (strcmp(compare_buffer, "files") == 0) {
							exit(0);
							printf("Hit exit\n");
							bencode->info->files = (struct info_file *)malloc(INFO_FILE_SIZE * sizeof(struct info_file *));
							bencode->head_pointer = (char **)&bencode->info->files;
						} else {
							exit(0);
						} 

*/
/*
else if(strcmp(compare_buffer, "length") == 0) {
							bencode->info->files[bencode->info_file_index].length = (int *)malloc(sizeof(int *));
							bencode->head_pointer = (char **)&bencode->info->files[bencode->info_file_index].length;
						} else if (strcmp(compare_buffer, "path") == 0) {
							
						}

*/


						//	bencode->info.files = (info_file *)malloc(INFO_FILE_SIZE * sizeof(info_file *));
						//	bencode->index_pointer = &bencode->info.files[bencode->info_file_index];
		
						//printBencode(bencode, &bencode->announce_list_index); 

					} else {
						strcpy((char *)bencode->head_pointer, compare_buffer);
						bencode->head_pointer = NULL;
					}		
					buffer_index = -1;	
				} else {
					buffer[buffer_index] = file_char;
				}
			}
		}
	}
	return 0;
}

int list(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index;
	int result;
	size_t length;

	char file_char = '\0';
	char buffer[BUFFER_SIZE];
	char compare_buffer[BUFFER_SIZE];

	id type;
	
	for (buffer_index = 0; buffer_index < BUFFER_SIZE; buffer_index++) {
		file_char = fgetc(file);

		type = identify(file_char);
		
		if (type != NULL) {
			result = type(bencode, file);
			
			if (result == 1) {
				return 2;
			}
		
		} else {
			if (file_char == ':') {
				buffer[buffer_index] = '\0';
				length = atoi(buffer);
				
				result = fread(compare_buffer, 1, length, file);
				compare_buffer[length] = '\0';

				if (result != (int)length) {
					printf("Parse error: Could not capture full segment. Verify the integrity of your .torrent file\n");
					return -1;
				}
			
				strcpy(((char **)bencode->head_pointer)[*bencode->index_pointer], compare_buffer);
				(*bencode->index_pointer)++;
				((char **)bencode->head_pointer)[*bencode->index_pointer] = (char *)calloc(BUFFER_SIZE, sizeof(char));
			} else {
				buffer[buffer_index] = file_char;
			}
		}
	}
	return 2;
}

int integer(struct bencode_module *bencode, FILE *file) {

	int buffer_index;
	int result;
	
	char file_char = '\0';
	char buffer[BUFFER_SIZE];

	id type;

	for (buffer_index = 0; buffer_index < BUFFER_SIZE; buffer_index++) {
		file_char = fgetc(file);
		
		type = identify(file_char);

		if (type != NULL) {
			result = type(bencode, file);
			
			if (result == 1) {
				buffer[buffer_index] = '\0';
				*(int *)bencode->head_pointer = atoi(buffer);
				return 0;
			}
		} else {
			buffer[buffer_index] = file_char;
		}
	}
	printf("Buffer exceeded!\n");
	return -1;	
}

int end(struct bencode_module *bencode __attribute__((unused)), FILE *file __attribute__((unused))) {
	return 1;
}

