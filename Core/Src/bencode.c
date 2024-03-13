#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../Inc/bencode.h"

#define BUFFER_SIZE 128
#define ANNOUNCE_LIST_SIZE 10
#define INFO_FILE_SIZE 5
#define FILE_PATH_SIZE 1
#define URL_LIST_SIZE 1

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
	
	bencode.buffer_size = BUFFER_SIZE;
	bencode.buffer = (char *)malloc(bencode.buffer_size * sizeof(char));

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
		printBencode(&bencode);
		return 0;
	} else {
		printf("Error in parsing dictionary\n");
		return -1;
	}
}

int dictionary(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index, result;
	size_t length, return_size;
	char file_char = '\0';

	id type;

	for (buffer_index = 0; buffer_index < BUFFER_SIZE; buffer_index++) {

		file_char = fgetc(file);
	
		type = identify(file_char);
		
		if (type != NULL) {
			result = type(bencode, file);

			/* If saw the end of the dictionary, return */
			if (result == 1) {
				return 0;
			}

			bencode->head_pointer = NULL;
			buffer_index = -1;
		} else {
			if (file_char == ':') {

				bencode->buffer[buffer_index] = '\0';
				length = atoi(bencode->buffer);		
				
				/* Expanding buffer to accomodate item length */
				if (length > bencode->buffer_size) {	
					
					/* Doubling buffer size until it can fit data */
					while (length > bencode->buffer_size) bencode->buffer_size = bencode->buffer_size * 2;	
					
					bencode->buffer = realloc(bencode->buffer, bencode->buffer_size);
				}


				return_size = fread(bencode->buffer, 1, length, file);
				
				/* Error handling for item read */
				if (return_size != length) {	
					printf("Parse error: Could not capture full segment. Verify the integrity of your .torrent file\n");
					return -1;
				}

				bencode->buffer[length] = '\0';

				/* If member of struct to place data hasn't been set, we need to set it */
				if (bencode->head_pointer == NULL) {

					if (strcmp(bencode->buffer, "announce") == 0) {
						
						bencode->announce = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->announce;
					
					} else if (strcmp(bencode->buffer, "announce-list") == 0) {
						
						bencode->announce_list = (char **)malloc(ANNOUNCE_LIST_SIZE * sizeof(char *));
						bencode->head_pointer = (void *)bencode->announce_list;
						bencode->index_pointer = &bencode->announce_list_index;
					
					} else if (strcmp(bencode->buffer, "comment") == 0) {
						
						bencode->comment = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->comment;
					
					} else if (strcmp(bencode->buffer, "created by") == 0) {
							
						bencode->created_by = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->created_by;
					
					} else if (strcmp(bencode->buffer, "creation date") == 0) {
							
						bencode->creation_date = (int *)malloc(sizeof(int));
						bencode->head_pointer = (void *)bencode->creation_date;

					} else if (strcmp(bencode->buffer, "encoding") == 0) {
						
						bencode->encoding = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->encoding;
						
					} else if (strcmp(bencode->buffer, "info") == 0) {
							
						bencode->info = (struct bencode_info *)malloc(sizeof(struct bencode_info));
						bencode->head_pointer = NULL;

					} else if (strcmp(bencode->buffer, "files") == 0) {
							
						bencode->info->files = (struct info_file **)malloc(INFO_FILE_SIZE * sizeof(struct info_file *));

					} else if (strcmp(bencode->buffer, "length") == 0) {
							
						bencode->info->files[bencode->info_file_index] = (struct info_file *)malloc(sizeof(struct info_file *));
						bencode->info->files[bencode->info_file_index]->length = (int *)malloc(sizeof(int));
						bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->length;
						
					} else if (strcmp(bencode->buffer, "path") == 0) {
						
						bencode->file_path_index = 0;
						bencode->info->files[bencode->info_file_index]->path = (char **)malloc(FILE_PATH_SIZE * sizeof(char *));
						bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->path;
						bencode->index_pointer = &bencode->file_path_index;
						bencode->info_file_index++;

					} else if (strcmp(bencode->buffer, "name") == 0) {
							
						bencode->info->name = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->info->name;
						
					} else if (strcmp(bencode->buffer, "piece length") == 0) {
						
						bencode->info->piece_length = (int *)malloc(sizeof(int));
						bencode->head_pointer = (void *)bencode->info->piece_length;
						
					} else if (strcmp(bencode->buffer, "pieces") == 0) {
							
						bencode->info->pieces = (char *)malloc(*bencode->info->piece_length * sizeof(char));
						bencode->head_pointer = (void *)bencode->info->pieces;
						
					} else if (strcmp(bencode->buffer, "url-list") == 0) {
							
						bencode->url_list = (char **)malloc(URL_LIST_SIZE * sizeof(char *));
						bencode->head_pointer = (void *)bencode->url_list;
						bencode->index_pointer = &bencode->url_list_index;
						
					} else {
						printBencode(bencode);
						printf("Parse error: Unrecognized key in dictionary\nKey Value: %s\n", bencode->buffer);
						return -1;
					}
				} else {
				
					/* If not looking for key, store buffer as value */
					strcpy((char *)bencode->head_pointer, bencode->buffer);
					bencode->head_pointer = NULL;
				}		
				buffer_index = -1;	
			} else {
				bencode->buffer[buffer_index] = file_char;
			}
		}
	}
	return 0;
}

int list(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index;
	int result;

	size_t return_size;
	size_t length;

	char file_char = '\0';

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
				bencode->buffer[buffer_index] = '\0';
				length = atoi(bencode->buffer);
				
				return_size = fread(bencode->buffer, 1, length, file);
				bencode->buffer[length] = '\0';

				if (return_size != length) {
					printf("Parse error: Could not capture full segment. Verify the integrity of your .torrent file\n");
					return -1;
				}
			
				((char **)bencode->head_pointer)[*bencode->index_pointer] = (char *)malloc(BUFFER_SIZE * sizeof(char));
				strcpy(((char **)bencode->head_pointer)[*bencode->index_pointer], bencode->buffer);
				(*bencode->index_pointer)++;
			} else {
				bencode->buffer[buffer_index] = file_char;
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
