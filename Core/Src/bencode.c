#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Inc/bencode.h"

#define BUFFER_SIZE 128
#define ANNOUNCE_LIST_SIZE 128
#define INFO_FILE_SIZE 8
#define FILE_PATH_SIZE 1
#define URL_LIST_SIZE 1

#define THREAD_NUM 1

struct bencode_module* parse_single(char *filepath, struct bencode_module* bencode) {

	FILE *file = fopen(filepath, "r");
	char file_char;
	int result;
	id type;

	/* Bencode struct initialization */
	bencode->announce 				= NULL;
	bencode->announce_list 			= NULL;
	bencode->comment 				= NULL;
	bencode->created_by 			= NULL;
	bencode->creation_date 			= NULL;
	bencode->encoding 				= NULL;
	bencode->info 					= NULL;
	bencode->url_list 				= NULL;
	bencode->head_pointer 			= NULL;
	bencode->size_pointer			= NULL;
	bencode->announce_list_index	= 0;
	bencode->info_file_index 		= 0;
	bencode->file_path_index 		= 0;
	bencode->url_list_index 		= 0;
	bencode->buffer_size 			= BUFFER_SIZE;
	bencode->announce_list_size		= ANNOUNCE_LIST_SIZE;
	bencode->info_file_size			= INFO_FILE_SIZE;
	bencode->file_path_size			= FILE_PATH_SIZE;
	bencode->url_list_size			= URL_LIST_SIZE;

	/* Error checking for existence of file */
	if (file == NULL) {
		fprintf(stderr, "Error reading from file: File not found\n");
		return NULL;
	}

	/* Checking first character of file for dictionary */	
	file_char = fgetc(file);
	type = identify(file_char);

	/* Error checking for presence of dictionary */	
	if (type != &dictionary) {
		printf("Parse error: First character was not the beginning of a dictionary\n");
		return NULL;
	}

	/* Allocating variable-size buffer for reading in and evaluating file contents */
	bencode->buffer = (char *)malloc(bencode->buffer_size * sizeof(char));

	/* Running function pointed to by function pointer 'type' */
	result = type(bencode, file);
	
	/* Should implement PAPI for time optimizations/tracking */
	if (result == 0) {
		printf("Dictionary parsed in ... seconds!\n");
		printBencode(bencode);
		return bencode;
	} else {
		switch (result) {
			case -1:
				printf("some error\n");
				break;
			default:
				printf("unknown error\n");
				break;
		}		

		printBencode(bencode);
		return NULL;
	}
}

int dictionary(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index, result;
	size_t length, return_size;
	char file_char = '\0';
	id type;

	for (buffer_index = 0; buffer_index < (int)bencode->buffer_size; buffer_index++) {

		file_char = fgetc(file);
		type = identify(file_char);
		
		if (type != NULL) {
			result = type(bencode, file);

			/* If end of the dictionary, return */
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
					/* Should implement cap on maximum buffer_size otherwise risk leak */
					while (length > bencode->buffer_size) bencode->buffer_size = bencode->buffer_size * 2;	
					bencode->buffer = realloc(bencode->buffer, bencode->buffer_size);
				}

				return_size = fread(bencode->buffer, 1, length, file);
				
				/* Error handling for item read */
				if (return_size != length) {	
					printf("Parse error: Could not capture full data segment. Please verify the integrity of your .torrent file\n");
					return DATA_LENGTH_EXCEEDED;
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
						bencode->size_pointer = &bencode->announce_list_size;
					
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
							
						if (bencode->info->files != NULL) {
							/* Multi-file contents */
							bencode->info->files[bencode->info_file_index] = (struct info_file *)malloc(sizeof(struct info_file *));
							bencode->info->files[bencode->info_file_index]->length = (unsigned long int *)malloc(sizeof(unsigned long int));
							bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->length;
						} else {
							/* Single file contents */
							bencode->info->length = (int *)malloc(sizeof(int));
							bencode->head_pointer= (void *)bencode->info->length;
						}						

					} else if (strcmp(bencode->buffer, "path") == 0) {
						
						bencode->info->files[bencode->info_file_index]->file_path_index = 0;
						bencode->info->files[bencode->info_file_index]->path = (char **)malloc(FILE_PATH_SIZE * sizeof(char *));
						bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->path;
						bencode->index_pointer = &bencode->info->files[bencode->info_file_index]->file_path_index;
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
						/* Setting pointer to effective NULL value to then have unexpected values ignored */
						bencode->head_pointer = (void *)IGNORE_FLAG;	
					}
				} else {
				
					/* If not looking for key, store buffer as value */
					if (bencode->head_pointer != (void *)IGNORE_FLAG) {
						strcpy((char *)bencode->head_pointer, bencode->buffer);
					}
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
	
	int buffer_index, result;
	size_t return_size, length;
	char file_char = '\0';
	id type;
	
	for (buffer_index = 0; buffer_index < (int)bencode->buffer_size; buffer_index++) {
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
					return DATA_LENGTH_EXCEEDED;
				}

				if (bencode->head_pointer != (void *)IGNORE_FLAG) {

					/* Extending size of pointer array if necessary */	
					if (*bencode->index_pointer == *bencode->size_pointer) {
						*bencode->size_pointer *= 2;
						bencode->head_pointer = realloc((char **)bencode->head_pointer, *bencode->size_pointer);
					}

					((char **)bencode->head_pointer)[*bencode->index_pointer] = (char *)malloc(BUFFER_SIZE * sizeof(char));
					strcpy(((char **)bencode->head_pointer)[*bencode->index_pointer], bencode->buffer);
				}
/*	
				if (bencode->info != NULL) {
				bencode->info->files[*bencode->index_pointer]->file_path_index = bencode->info->files[*bencode->index_pointer]->file_path_index + 1;
				}
*/
				buffer_index = -1;
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

	id type;

	for (buffer_index = 0; buffer_index < BUFFER_SIZE; buffer_index++) {
		file_char = fgetc(file);
		
		type = identify(file_char);

		if (type != NULL) {
			result = type(bencode, file);
			
			if (result == 1) {
				bencode->buffer[buffer_index] = '\0';
				if (bencode->head_pointer != (void *)IGNORE_FLAG) *(int *)bencode->head_pointer = atoi(bencode->buffer);
				return 0;
			}
		} else {
			bencode->buffer[buffer_index] = file_char;
		}
	}
	printf("Buffer exceeded!\n");
	return -1;	
}

int end(struct bencode_module *bencode __attribute__((unused)), FILE *file __attribute__((unused))) {
	return 1;
}

id identify(char c) {
    switch (c) {
        case 'd':
            return dictionary;
            break;
        case 'l':
            return list;
            break;
        case 'i':
            return integer;
            break;
        case 'e':
            return end;
            break;
        default:
            return NULL;
            break;
    }
}

void printBencode(struct bencode_module *bencode) {
    printf("Announce: %s\n\n", bencode->announce);
    for (int i = 0; i < bencode->announce_list_index; i++) {
        printf("Announce-List %d: %s\n", i, bencode->announce_list[i]);
    }
    if (bencode->comment != NULL) printf("\nComment: %s\n", bencode->comment);
    if (bencode->created_by != NULL) printf("Created By: %s\n", bencode->created_by);
    if (bencode->creation_date != NULL) printf("Creation Date: %d\n", *bencode->creation_date);
    if (bencode->encoding != NULL) printf("Encoding: %s\n\n", bencode->encoding);
    for (int i = 0; i < bencode->info_file_index; i++) {
        for (int j = 0; j < bencode->info->files[i]->file_path_index; j++) {
            printf("Info File %d: Length: %ld Path: %s\n", i, *bencode->info->files[i]->length, bencode->info->files[i]->path[j]);
        }
    }
    if (bencode->info->name != NULL) printf("\nName: %s\n", bencode->info->name);
    if (bencode->info->piece_length != NULL) printf("Piece Length: %d\n", *bencode->info->piece_length);
    if (bencode->info->pieces != NULL) printf("Pieces: %s\n\n", bencode->info->pieces);
    for (int i = 0; i < bencode->url_list_index; i++) {
        printf("Url List %d: %s\n", i, bencode->url_list[i]);
    }
}
