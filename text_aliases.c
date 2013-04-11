#include <stdio.h>
#include <string.h>
 #ifdef OSX
  #include <sys/malloc.h>
 #elif BSD
 #include <stdlib.h>
 #else
  #include <malloc.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "text_aliases.h"
#include "io/elpathwrapper.h"
#include "errors.h"
#include "translate.h"
#include "text.h"
#include "client_serv.h"
#include "console.h"
#include "makeargv.h"
#include "dbuffer.h"
#include "asc.h"
#include "chat.h"

static char *numeric_aliases[100]; /* Stores the alias buffer */
static int numeric_alias_sizes[100]; /* Stores the alias buffer size */

static void escape_octal_putstring (const unsigned char *text, int len, FILE * fp);
static int generic_numeric_process (char *text, int len, int (*callback) (int index, char *text, int len));

/**
 \brief Save current aliases to a file
*/
static int save_aliases ()
{
	int i;
	FILE *fp = open_file_config (NUMERIC_ALIASES_FILENAME, "w");
	if (fp == NULL)
	{
		LOG_ERROR ("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file,
				   NUMERIC_ALIASES_FILENAME, strerror(errno));
		return -1;
	}

	for (i = 0; i < 100; i++)
	{
		if (NULL != numeric_aliases[i])
		{
			fprintf (fp, "%d ", i);
			escape_octal_putstring ((unsigned char *) numeric_aliases[i],
									numeric_alias_sizes[i], fp);
			fputs ("\n", fp);
		}
	}

	return fclose (fp);
}

static int internal_bind_alias (int index, char *text, int len)
{
	int seen = 0;
    char buf[128];

	if ( len > 128 ) {
		sprintf (buf, "Alias %d too large (%d exceeds max len of %d).",index,len,128);
		LOG_TO_CONSOLE (c_red1, buf);
        return -1;
	}

	if (numeric_aliases[index])
	{
		free (numeric_aliases[index]);
		seen = 1;
	}
	numeric_aliases[index] = malloc (len);
	memcpy (numeric_aliases[index], text, len);
	numeric_alias_sizes[index] = len;
	save_aliases ();
	return seen;
}

int bind_alias (int index, char *text, int len)
{
	char buf[512];

	int seen = internal_bind_alias (index, text, len);
	if (seen < 0)
		return -1;

	sprintf (buf, "Alias %d %s as '%s'", index, seen ? "rebound" : "bound", text);

	LOG_TO_CONSOLE (c_orange1, buf);
	return 0;
}

int unbind_alias (int index, char *text, int len)
{
	char buf[128];

	int seen = ( numeric_aliases[index] != NULL );
	if (seen) {
		free(numeric_aliases[index]);
        numeric_aliases[index] = NULL;
	}
	sprintf (buf, "Alias %d %s", index, seen ?  "unbound": "is not bound");

	LOG_TO_CONSOLE (c_orange1, buf);
	return 0;
}


static void escape_octal_putstring (const unsigned char *text, int len, FILE * fp)
{
	while (len)
	{
		if (*text == '\\')
		{
			fputs ("\\\\", fp);
			text++;
			len--;
			continue;
		}
		if (*text < ' ' || *text > 127)
		{
			/* Escape */
			fprintf (fp, "\\%03o", (unsigned int) *text);
		}
		else
		{
			fputc (*text, fp);
		}
		text++;
		len--;
	}
}

static char *unescape_octal_string (const unsigned char *text, int len, int *newlen)
{
	char *retval;
	char octal_value[4];
	int octptr = 0;
	unsigned long value;
	char *endp;

	enum
	{
		CHAR_NORMAL,
		CHAR_ESCAPED
	} state;

	state = CHAR_NORMAL;

	retval = malloc (len);
	*newlen = 0;

	if (NULL == retval)
		return NULL;

	while (len > 0)
	{
		switch (state)
		{
		case CHAR_NORMAL:
			if (*text == '\\')
			{
				state = CHAR_ESCAPED;
				octptr = 0;
				break;
			}
			retval[*newlen] = *text;
			(*newlen)++;
			break;
		case CHAR_ESCAPED:
			if (isdigit (*text))
			{
				octal_value[octptr++] = *text;
				octal_value[octptr] = '\0';

				if (octptr == 3)
				{
					/* Convert it */
					value = strtoul (octal_value, &endp, 8);
					if (*endp)
					{
						LOG_ERROR ("%s: %s \\%s\n", reg_error_str,
								   "Invalid alias escape sequence",
								   octal_value);
						free (retval);
						return NULL;
					}
					retval[*newlen] = (char) value;
					(*newlen)++;

					state = CHAR_NORMAL;
				}
			}
			else
			{
				LOG_ERROR ("%s: %s '%c'\n", reg_error_str,
						   "Invalid alias escape sequence char", *text);
				free (retval);
				return NULL;
			}
		}
		text++;
		len--;
	}
	return retval;
}

static dbuffer_t *expand_alias_parameters( char *parameters, const char *aliastext, int alias_size )
{
	dbuffer_t *return_text;

	const char *cptr = aliastext;
	int argc = 0;
    int param_index;
	char **argv = NULL;
    unsigned char nullchar = '\0';
	enum {
		PARAM_NORMAL_CHAR,
		PARAM_INDEX
	} state;

	ENTER_DEBUG_MARK("expand text aliases");

	return_text = dbuffer_new( alias_size );

	if (NULL==return_text) {
		ENTER_DEBUG_MARK("expand text aliases");

		LOG_ERROR("Cannot allocate buffer %d", alias_size);
		return NULL;
	}

	state = PARAM_NORMAL_CHAR;

	LOG_DEBUG("Starting expansion, size is %d", alias_size);

	while (alias_size) {
        LOG_DEBUG("Handling char '%c', state %d", *cptr, state);
		switch (state) {
		case PARAM_NORMAL_CHAR:
			if (*cptr == '$') {
				state = PARAM_INDEX;
				break;
			}
            return_text = dbuffer_append_data(return_text, (unsigned char*)cptr, 1);
			break;
		case PARAM_INDEX:
			if (isdigit(*cptr)) {
				/* Ok, try to expand this */
				param_index = ((unsigned char)(*cptr)) - '0';

				LOG_DEBUG("Index for expansion is %d, params %s", param_index, parameters);

				if (NULL==argv) {
					argc = makeargv( parameters, &argv);

					LOG_DEBUG("Argc is %d", argc);
					if (NULL==argv || argc == 0) {
						LOG_TO_CONSOLE( c_orange1, "Text alias requires parameters, but none given");
						free( return_text );
                        return NULL;
					}
				}

				// Ok, see if it is out of bounds.


				if ( param_index >= argc ) {
					LOG_TO_CONSOLE( c_orange1, "Text alias requires more parameters than given");
					dbuffer_destroy( return_text );
					if (NULL!=argv)
						freemakeargv(argv);
					LEAVE_DEBUG_MARK("expand text aliases");
					return NULL;
				}

				// Nice, try to append the argument.

				return_text = dbuffer_append_data(return_text, (unsigned char*)argv[param_index], strlen(argv[param_index]));

                state = PARAM_NORMAL_CHAR;
                break;
			}
			if (*cptr == '$') {
				return_text = dbuffer_append_data(return_text, (unsigned char*)cptr, 1);
				state = PARAM_NORMAL_CHAR;
                break;
			}
			// Invalid $ sequence
			LOG_TO_CONSOLE( c_orange1, "Invalid sequence found");
            LOG_DEBUG("Invalid sequence");
			dbuffer_destroy( return_text );
			if (NULL!=argv)
				freemakeargv(argv);
			LEAVE_DEBUG_MARK("expand text aliases");
			return NULL;
		}
		alias_size--;
        cptr++;
	}

	if (state == PARAM_INDEX) {
        /* What now ? Last char in alias is a $ */
	}

	if (NULL!=argv)
		freemakeargv(argv);

	/* Add a NULL to the end, just to make sure */

	return_text = dbuffer_append_data(return_text, &nullchar, 1);

	LOG_DEBUG("Finished, text is '%s', len %d\n", return_text->data, return_text->current_size);

	LEAVE_DEBUG_MARK("expand text aliases");

	return return_text;
}

static int read_alias_line (int index, char *text, int len)
{
	int newlen;
	char *realline =
		unescape_octal_string ((unsigned char *) text, len, &newlen);
	if (NULL != realline)
	{
		numeric_aliases[index] = realline;
		numeric_alias_sizes[index] = newlen;

		return 0;
	}
	return -1;
}



int init_text_aliases ()
{
	int i;
	char line[512];
	int error = -1;
	char *endl;
	FILE *fp;

	ENTER_DEBUG_MARK("init text aliases");

	for (i = 0; i < 100; i++)
		numeric_aliases[i] = NULL;

	/* Load file */
	LOG_DEBUG("Loading aliases");

	fp = open_file_config (NUMERIC_ALIASES_FILENAME, "r");
	if (fp == NULL)
	{
		LOG_DEBUG("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file,
				   NUMERIC_ALIASES_FILENAME, strerror(errno));
		error = 0;
	}
	else
	{
		while (fgets (line, 512, fp))
		{
			endl = strpbrk (line, "\n\r");
			if (endl)
				*endl = 0;
			LOG_DEBUG("Line %s", line);

			if (generic_numeric_process(line, strlen (line), read_alias_line) < 0)
			{
				LOG_ERROR ("%s: %s \"%s\"\n", reg_error_str,
					"Invalid line read in file ",
					NUMERIC_ALIASES_FILENAME);
				error = -1;
				break;
			}
			LOG_DEBUG("Loaded alias from %s", line);
			error = 0;
		}
		fclose (fp);
	}

	LEAVE_DEBUG_MARK("init text aliases");

	return error;
}

void shutdown_text_aliases ()
{
	int i;
	for (i = 0; i < 100; i++)
		if (numeric_aliases[i])
			free (numeric_aliases[i]);
}

static int handle_text_alias (int index, char *text, int len)
{
	char msg[80];
	dbuffer_t *newmsg;
	static int we_are_nested = 0;
	static int previously_expanded[100];

	/* check if we are already expanding an alias */
	if (we_are_nested)
	{
		/* if the current alias has already been expanded, we have infinite recursion, stop now! */
		if (previously_expanded[index])
		{
			we_are_nested = 0;
			LOG_TO_CONSOLE (c_red2, "Error, you have infinitely recursive aliases");
			return 0;
		}
	}
	else
		memset(previously_expanded, 0, sizeof(int)*100);
	
	previously_expanded[index] = we_are_nested = 1;

	if (NULL != numeric_aliases[index])
	{
		newmsg = expand_alias_parameters( text, numeric_aliases[index], numeric_alias_sizes[index] );

		if (NULL!=newmsg) {
			parse_input ( (char*)newmsg->data, newmsg->current_size );
            dbuffer_destroy(newmsg);
		}
	}
	else
	{
		sprintf (msg, "Invalid alias #%d", index);
		LOG_TO_CONSOLE (c_orange1, msg);
	}
	
	/* any recursion is over so clear the flag */
	we_are_nested = 0;
	
	return 0;
}

int process_text_alias (char *text, int len)
{
	return generic_numeric_process (text, len, handle_text_alias);
}

static int generic_numeric_process (char *text, int len, int (*callback) (int index, char *text, int len))
{
	char num_to_process[3];
	char *endp;
	long index;
	int i;

	memset( num_to_process, 0, sizeof(num_to_process) );

	for (i = 0; i < 3; i++)
	{

		if (len > 0 && isdigit (*text))
		{
			num_to_process[i] = *text;
			text++;
			len--;
			continue;
		}
		/* If it's space, stop and handle */
		if (len == 0 || isspace (*text) || *text == 0)
		{
			if (len > 0 && *text)
			{
				text++;
				len--;
			}
			if (len > 0)
				while (isspace (*text))
				{
					text++;
					len--;
				}

			/* Convert number */

			index = strtoul (num_to_process, &endp, 10);
			if (endp && *endp == '\0')
			{
				/* Valid number */
				if (index < 0 || index > 99)
				{
					/* This should never happen, but just to be sure... */
					//fprintf(stderr,"Out of bounds\n");
                    LOG_ERROR("Index %lu out of bounds\n", index);
					return -1;
				}
				//fprintf(stderr,"Cb index %d\n", index);
				return callback (index, text, len);
			}
            LOG_ERROR("Invalid number '%s'\n", num_to_process);
			return -1;
		}
		return -1;		/* Invalid */
	}
	return -1;
}

int alias_command (char *text, int len)
{
	if (*text != ' ')
	{
		LOG_TO_CONSOLE (c_orange1, "Invalid syntax.");
		return 1;
	}
	while (len > 0 && *text == ' ') {
		text++;
		len--;
	}

	generic_numeric_process (text, len, bind_alias);
	return 1;			/* Don't pass to server */
}

int unalias_command ( char *text, int len)
{
	while (len > 0 && *text == ' ') {
		text++;
		len--;
	}

	generic_numeric_process (text, len, unbind_alias);
	return 1;			/* Don't pass to server */
}

int aliases_command ( char *text, int len)
{
	int i;
	char alias_temp[128+6]; /* 128 chars of alias + at most '#100 ' */
    size_t templen;

	LOG_TO_CONSOLE(c_green1, "List of current text aliases:");
	for (i=0; i<100; i++) {
		if (NULL!=numeric_aliases[i]) {
			sprintf(alias_temp, "#%d ", i);
			/* We have 60 -templen -1 chars remaining */
			templen = strlen(alias_temp);

			if ( numeric_alias_sizes[i] > ((sizeof(alias_temp)-1)-templen) ) {
				memcpy( alias_temp + templen, numeric_aliases[i], ((sizeof(alias_temp)-1)-templen) );
                alias_temp[(sizeof(alias_temp)-1)] = '\0';
			} else {
				memcpy( alias_temp + templen, numeric_aliases[i], numeric_alias_sizes[i] );
                alias_temp[ templen + numeric_alias_sizes[i] ] = '\0';
			}
			LOG_TO_CONSOLE(c_orange1,alias_temp);
		}
	}
	return 1;			/* Don't pass to server */
}
