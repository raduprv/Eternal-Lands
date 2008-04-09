#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void remove_quotes( char *string )
{
	char *source = string;
	char *dest = string;
	while (*source) {
		if (*source == '"') {
			source++;
			continue;
		}
		if (source!=dest) {
			*dest = *source;
		}
		source++;
		dest++;
	}
	*dest = '\0';
}

// splits a char* into a char **

#define PARSER_MAX_ARGS 512

int makeargv( char *string, char ***argv )
{
	enum {
		READ_ARG_CHAR,
		READ_STR_CHAR,
		READ_DELIMITER
	} state = READ_ARG_CHAR;

	char *current_argv[ PARSER_MAX_ARGS ];
	unsigned int current_token = 0;
	char *current_token_start = string;
	char *current_char = string;
	int i;

	while ( *current_char != '\0' ) {
		switch ( state ) {
		case READ_ARG_CHAR:
			switch ( *current_char ) {
			case '\n':
			case '\t':
			case ' ':
				*current_char = '\0';
				state = READ_DELIMITER;
				current_argv[ current_token ] = current_token_start;
				current_token++;
				break;

			case '"':
				/* String coming, probably. */
				state = READ_STR_CHAR;
				break;
			default:
				break;
			}
			break;

		case READ_STR_CHAR:
			switch (*current_char) {
			case '"':
				state = READ_ARG_CHAR;
				break;
			default:
				break;
			}
			break;
		case READ_DELIMITER:
			switch (*current_char) {
			case '\n':
			case '\t':
			case ' ':
				*current_char = '\0';
				break;
			case '"':
				state = READ_STR_CHAR;
				current_token_start = current_char;
				break;
			default:
				state = READ_ARG_CHAR;
				current_token_start = current_char;
				break;
			}
		}
		current_char++;
	}

	/* Remaining */

	if ( *current_token_start ) {
		current_argv[current_token++] = current_token_start;
	}

	/* Allocate */
	if (current_token==0) {
		*argv = NULL;
		return 0;
	}

	*argv = (char**)malloc((current_token+1)*sizeof(char *));

	for (i=0; i<current_token; i++) {
		remove_quotes( current_argv[i] );
		(*argv)[i] = strdup(current_argv[i]);
	}

	(*argv)[i] = NULL;
	return current_token;
}

//frees the char** created by makeargv
void freemakeargv(char **argv)
{
	char **saveargv = argv;
	if (argv == NULL)
		return;
	while (*argv != NULL) {
		free(*argv);
		argv++;
	}
	free(saveargv);
}
