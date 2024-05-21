#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "../Inc/bencode.h"

#define BUFFER_SIZE 128
#define ANNOUNCE_LIST_SIZE 256
#define INFO_FILE_SIZE 128
#define FILE_PATH_SIZE 1
#define URL_LIST_SIZE 1

#define NOT_A_TYPE NULL
#define BUFFER_EXCEEDED -1
#define END_OF_TYPE 1

struct bencode_module* parse_single(char *filepath, struct bencode_module* bencode) {

	FILE *file = fopen(filepath, "r");
	char file_char;
	int result;
	id type;

	/* Struct initialization */
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

	/* Allocating buffer for reading in and evaluating file contents */
	bencode->buffer = (char *)malloc(bencode->buffer_size * sizeof(char));

	/* Running function pointed to by function pointer 'type' */
	result = type(bencode, file);
	
	if (result == 0) {
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

		//printBencode(bencode);
		return NULL;
	}
}

int dictionary(struct bencode_module *bencode, FILE *file) {
	
	int buffer_index, result;
	unsigned long int length;
	size_t return_size;

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
			
				if (bencode->buffer[0] == '-') {
        			fprintf(stderr, "Error: Negative input is not valid for unsigned long conversion\n");
        			return CONVERSION_FAILED;
   				}

				errno = 0;
				length = strtoul(bencode->buffer, NULL, 10);
				
				/* Error handling for str -> data segment length */
				if (errno != 0) {
					perror("strtoul");		
					fprintf(stderr, "Error: Unable to interpret data segment length\n");
					return CONVERSION_FAILED;
				}

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
				
						bencode->announce_list = (char **)malloc(bencode->announce_list_size * sizeof(char *));
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
							
						bencode->creation_date = (long long int *)malloc(sizeof(long long int));
						bencode->head_pointer = (void *)bencode->creation_date;

					} else if (strcmp(bencode->buffer, "encoding") == 0) {
						
						bencode->encoding = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->encoding;
						
					} else if (strcmp(bencode->buffer, "info") == 0) {
							
						bencode->info = (struct bencode_info *)malloc(sizeof(struct bencode_info));
						bencode->head_pointer = NULL;

					} else if (strcmp(bencode->buffer, "files") == 0) {
							
						bencode->info->files = (struct info_file **)malloc(bencode->info_file_size * sizeof(struct info_file *));

					} else if (strcmp(bencode->buffer, "length") == 0) {
							
						if (bencode->info->files != NULL) {
							/* Multi-file contents */
							bencode->info->files[bencode->info_file_index] = (struct info_file *)malloc(sizeof(struct info_file *));
							bencode->info->files[bencode->info_file_index]->length = (unsigned long int *)malloc(sizeof(unsigned long int));
							bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->length;
						} else {
							/* Single file contents */
							bencode->info->length = (long long int *)malloc(sizeof(long long int));
							bencode->head_pointer= (void *)bencode->info->length;
						}						

					} else if (strcmp(bencode->buffer, "path") == 0) {
						
						bencode->info->files[bencode->info_file_index]->file_path_index = 0;
						bencode->info->files[bencode->info_file_index]->path = (char **)malloc(bencode->file_path_size * sizeof(char *));
						bencode->head_pointer = (void *)bencode->info->files[bencode->info_file_index]->path;
						bencode->index_pointer = &bencode->info->files[bencode->info_file_index]->file_path_index;
						bencode->info_file_index++;

					} else if (strcmp(bencode->buffer, "name") == 0) {
							
						bencode->info->name = (char *)malloc(BUFFER_SIZE * sizeof(char));
						bencode->head_pointer = (void *)bencode->info->name;
						
					} else if (strcmp(bencode->buffer, "piece length") == 0) {
						
						bencode->info->piece_length = (long long int *)malloc(sizeof(long long int));
						bencode->head_pointer = (void *)bencode->info->piece_length;
						
					} else if (strcmp(bencode->buffer, "pieces") == 0) {
							
						bencode->info->pieces = (char *)malloc(*bencode->info->piece_length * sizeof(char));
						bencode->head_pointer = (void *)bencode->info->pieces;
						
					} else if (strcmp(bencode->buffer, "url-list") == 0) {
							
						bencode->url_list = (char **)malloc(bencode->url_list_size * sizeof(char *));
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
						bencode->head_pointer = NULL;
					}
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
	
	unsigned int result, return_size, buffer_index = 0;
	long long int length;
	char file_char = '\0';
	id type;

	while (buffer_index < bencode->buffer_size)	{
		file_char = fgetc(file);

		type = identify(file_char);
		
		if (type != NULL) {
			
			result = type(bencode, file);
			
			if (result == END_OF_TYPE) {
				return PARSE_SUCCESS;
			}
			
			buffer_index = 0;
		} else {
			if (file_char == ':') {
				bencode->buffer[buffer_index] = '\0';
				
				result = verify_int(bencode->buffer, &length);
				
				if (result != 0) {
					printf("Parse error: Length of data element could not be determined.\nPlease verify the integrity of your .torrent file.\n");
					return PARSE_FAILURE;
				}

				return_size = fread(bencode->buffer, 1, length, file);
				bencode->buffer[length] = '\0';

				if (return_size != length) {
					printf("Parse error: Could not capture full segment.\nPlease verify the integrity of your .torrent file.\n");
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
				
					(*bencode->index_pointer)++;
				}

				buffer_index = 0;
			} else {
				bencode->buffer[buffer_index] = file_char;
				buffer_index++;
			}
		}
	}
	return BUFFER_EXCEEDED;
}

int integer(struct bencode_module *bencode, FILE *file) {

	unsigned int result, buffer_index = 0;
	char file_char;
	id type;

	while (buffer_index < bencode->buffer_size) {
		file_char = fgetc(file);
		
		/* Identify if character signifies beginning of type */
		type = identify(file_char);

		if (type != NULL) {
	
			/* Running function to parse detected type */
			result = type(bencode, file);
			
			/* If captured character which signifies end of type -> store value */
			if (result == END_OF_TYPE) {
				bencode->buffer[buffer_index] = '\0';
				if (bencode->head_pointer != (void *)IGNORE_FLAG) {
					return verify_int(bencode->buffer, bencode->head_pointer);	
				}
			}
		} else {
			bencode->buffer[buffer_index] = file_char;
			buffer_index++;
		}
	}
	return BUFFER_EXCEEDED;
}

/*
 * Function: verify_int
 * -------------------
 * Converts string to integer and checks whether output is a valid and clean integer
 * 
 * returns: Error code associated with whether valid integer was determined
 */
int verify_int(char *input, long long int *output) {
		
	long long int val = 0;
	errno = 0;

	/* Performing strtoull on string and then checking errno and output buffers to check if valid integer */
    val = strtoull(input, NULL, 10);
    if ((errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN)) || (errno != 0 && val == 0)) {
		perror("strtol");
        return CONVERSION_FAILED;
   	}

	/* Setting value of second parameter to parsed value of integer */
    *(long long int *)output = val;

	return CONVERSION_SUCCESS;
}

int end(struct bencode_module *bencode __attribute__((unused)), FILE *file __attribute__((unused))) {
	return END_OF_TYPE;
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
            return NOT_A_TYPE;
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
    if (bencode->creation_date != NULL) printf("Creation Date: %lld\n", *bencode->creation_date);
    if (bencode->encoding != NULL) printf("Encoding: %s\n\n", bencode->encoding);
    for (int i = 0; i < bencode->info_file_index; i++) {
        for (int j = 0; j < bencode->info->files[i]->file_path_index; j++) {
            printf("Info File %d: Length: %ld Path: %s\n", i, *bencode->info->files[i]->length, bencode->info->files[i]->path[j]);
        }
    }
    if (bencode->info->name != NULL) printf("\nName: %s\n", bencode->info->name);
    if (bencode->info->piece_length != NULL) printf("Piece Length: %lld\n", *bencode->info->piece_length);
    if (bencode->info->pieces != NULL) printf("Pieces: %s\n\n", bencode->info->pieces);
    
	if (bencode->url_list != NULL) {
		for (int i = 0; i < bencode->url_list_index; i++) {
       		printf("Url List %d: %s\n", i, bencode->url_list[i]);
    	}
	}
}
