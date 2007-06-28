#include <string.h>
#include "../text.h"
#include "../client_serv.h"
#include "amxel.h"
#include "amxcons.h"

#define MAX_LOG_MSG_SIZE 256

static int append_char (void *dest, char ch)
{
	char* str = dest;
	size_t len = strlen (dest);
	if (len + 1 < MAX_LOG_MSG_SIZE)
	{
		str[len++] = ch; 
		str[len] = '\0';
	}
	return 0;
}

static int append_string (void *dest, const char* src)
{
	char* str = dest;
	size_t len = strlen (dest);
	if (len + strlen (src) < MAX_LOG_MSG_SIZE)
	{
		strcpy (str+len, src);
	}
	else if (len + 1 < MAX_LOG_MSG_SIZE)
	{
		// we have space for at least one more character
		strncpy (str+len, src, MAX_LOG_MSG_SIZE-len-1);
		str[MAX_LOG_MSG_SIZE-1] = '\0';
	}
	return 0;
}

const char* format_log_message (AMX *amx, const cell fmt, const cell* params, int nr_params)
{
	static char msg[MAX_LOG_MSG_SIZE];
	AMX_FMTINFO info;
	cell *cstr;
 
	msg[0] = '\0';
	
	memset (&info, 0, sizeof (info));
	info.params = params;
	info.numparams = nr_params;
	info.skip = 0;
	info.length = MAX_LOG_MSG_SIZE; 
	info.f_putstr = append_string;
	info.f_putchar = append_char;
	info.user = msg;

	amx_GetAddr (amx, fmt, &cstr);
	amx_printstring (amx, cstr, &info);
	
	return msg;
}

static cell AMX_NATIVE_CALL n_log_to_console (AMX *amx, const cell *params)
{
	int nr_params = params[0] / sizeof (cell) - 1;
	const char* msg = format_log_message (amx, params[1], params+2, nr_params);

	LOG_TO_CONSOLE (c_red1, msg);

	return 0;
}

#ifdef __cplusplus
extern "C"
#endif

const AMX_NATIVE_INFO el_Natives[] = {
	{ "log_to_console", n_log_to_console },
 	{ NULL,             NULL             }  /* terminator */
};

int AMXEXPORT amx_ElInit (AMX *amx)
{
	return amx_Register (amx, el_Natives, -1);
}

int AMXEXPORT amx_ElCleanup (AMX *amx)
{
	(void) amx;
	return AMX_ERR_NONE;
}

