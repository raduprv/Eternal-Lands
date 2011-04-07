#include "custom_update.h"
#include "new_update.h"
#include "threads.h"
#include "console.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include <time.h>
#include <SDL_thread.h>
#include <ctype.h>
#include "asc.h"
#include "init.h"

SDL_Thread* update_thread = 0;
SDL_mutex* update_mutex = 0;
SDL_cond* update_condition = 0;
Uint32 update_running = 0;
char update_str[4096];
Uint32 update_error = 0;

Uint32 progress_function(const char* str, const Uint32 max,
	const Uint32 current, void* user_data)
{
	memset(update_str, 0, sizeof(update_str));

	CHECK_AND_LOCK_MUTEX(update_mutex);

	if (max > 0)
	{
		snprintf(update_str, sizeof(update_str), "%s %.2f%%", str,
			(current * 100.0f) / max);
	}
	else
	{
		snprintf(update_str, sizeof(update_str), "%s", str);
	}

	if (update_running == 0)
	{
		CHECK_AND_UNLOCK_MUTEX(update_mutex);

		return 0;
	}

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	return 1;
}

Uint32 custom_update_threaded(void* data)
{
	char buffer[1024];
	char str[256];
	char* server;
	FILE* file;
	Uint32 count, index, idx, len, result;
	const char* file_name = "custom_mirrors.lst";

	snprintf(str, sizeof(str), "%s%s", datadir, file_name);

	file = fopen(str, "r");

	if (file == 0)
	{
		snprintf(str, sizeof(str), "%s%s", get_path_config_base(),
			file_name);

		file = fopen(str, "r");
	}

	if (file == 0)
	{
		snprintf(buffer, sizeof(buffer),
			"Can't find server list file '%s' at dir '%s' or '%s'.",
			file_name, datadir, get_path_config_base());

		progress_function(buffer, 0, 0, 0);

		return 1;
	}

	count = 0;
	server = fgets(buffer, sizeof(buffer), file);

	while (server != 0)
	{
		len = strlen(buffer);

		// is this line worth handling?
		if ((len > 6) && (buffer[0] > ' ') && (buffer[0] != '#'))
		{
			count++;
		}

		// read the next line
		server = fgets(buffer, sizeof(buffer), file);
	}

	if (count == 0)
	{
		snprintf(buffer, sizeof(buffer), "No server in file '%s'",
			str);

		progress_function(buffer, 0, 0, 0);

		fclose(file);

		return 1;
	}

	rewind(file);

	srand(time(0));

	idx = rand() % count;

	index = 0;

	server = fgets(buffer, sizeof(buffer), file);

	while (server != 0)
	{
		len = strlen(buffer);

		// is this line worth handling?
		if ((len > 6) && (buffer[0] > ' ') && (buffer[0] != '#'))
		{
			index++;

			if (index > idx)
			{
				while (isspace(buffer[len-1]))
				{
					buffer[len - 1] = '\0';
					len--;

					if (len == 0)
					{
						break;
					}
				}

				break;
			}
		}

		// read the next line
		server = fgets(buffer, sizeof(buffer), file);
	}

	fclose(file);

	if (server == 0)
	{
		snprintf(buffer, sizeof(buffer),
			"Can't get server from file '%s'", str);

		progress_function(buffer, 0, 0, 0);

		return 1;
	}

	snprintf(str, sizeof(str), "%s%s", get_path_config_base(),
		"custom_files.zip");

	unload_zip_archive(str);

	result = update(server, "custom_files.lst", "updates", str,
		progress_function, data);

	if (result == 0)
	{
		load_zip_archive(str);
	}

	return result;
}

int custom_update_thread(void* data)
{
	Uint32 result;

	result = 0;

	while (1)
	{
		CHECK_AND_LOCK_MUTEX(update_mutex);

		update_error = result;

		while (update_running == 1)
		{
			SDL_CondWait(update_condition, update_mutex);
		}

		if (update_running == 0)
		{
			CHECK_AND_UNLOCK_MUTEX(update_mutex);

			return 1;
		}

		update_running = 1;

		CHECK_AND_UNLOCK_MUTEX(update_mutex);

		result = custom_update_threaded(data);
	}

	return 0;
}

void start_custom_update()
{
	snprintf(update_str, sizeof(update_str), "Custom updates started");

	update_condition = SDL_CreateCond();
	update_mutex = SDL_CreateMutex();
	update_running = 2;
	update_thread = SDL_CreateThread(custom_update_thread, 0);
}

void stopp_custom_update()
{
	int result;

	if (update_mutex == 0)
		return;

	CHECK_AND_LOCK_MUTEX(update_mutex);

	update_running = 0;

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	SDL_CondSignal(update_condition);
	SDL_WaitThread(update_thread, &result);

	SDL_DestroyCond(update_condition);
	SDL_DestroyMutex(update_mutex);

	update_mutex = 0;
	update_thread = 0;
}

int command_update_status(char *text, int len)
{
	if (update_mutex == 0)
	{
		LOG_TO_CONSOLE(c_red1, "Update disabled");

		return 1;
	}

	CHECK_AND_LOCK_MUTEX(update_mutex);

	switch (update_error)
	{
		case 0:
			LOG_TO_CONSOLE(c_green1, update_str);
			break;
		case 1:
			LOG_TO_CONSOLE(c_red1, update_str);
			break;
		case 2:
			LOG_TO_CONSOLE(c_red1, update_str);
			break;
	}

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	return 1;
}

int command_update(char *text, int len)
{
	if (update_mutex == 0)
	{
		LOG_TO_CONSOLE(c_red1, "Update disabled");

		return 1;
	}

	CHECK_AND_LOCK_MUTEX(update_mutex);

	if (update_running == 1)
	{
		update_running = 2;
	}

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	SDL_CondSignal(update_condition);

	return 1;
}

