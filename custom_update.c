#include "custom_update.h"
#include "new_update.h"
#include "threads.h"
#include "console.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include <time.h>

SDL_Thread* update_thread = 0;
SDL_mutex* update_mutex = 0;
Uint32 update_running = 0;
char update_str[1024];
Uint32 error = 0;

Uint32 progress_function(const char* str, const char* extra, const Uint32 max,
	const Uint32 current, void* user_data)
{
	CHECK_AND_LOCK_MUTEX(update_mutex);

	memset(update_str, 0, sizeof(update_str));

	if (max > 0)
	{
		snprintf(update_str, sizeof(update_str), "%s %.2f%%", str,
			(current * 100.0f) / max);
	}
	else
	{
		snprintf(update_str, sizeof(update_str), "%s", str);
	}

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	return update_running;
}

int custom_update_thread(void* data)
{
	char buffer[1024];
	char str[256];
	char* server;
	FILE* file;
	Uint32 count, index, idx, len;

	snprintf(str, sizeof(str), "%s%s", get_path_config_base(),
		"custom_mirrors.lst");

	file = fopen(str, "r");

	if (file == 0)
	{
		return 0;
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
		fclose(file);

		return 0;
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
		return 0;
	}

	snprintf(str, sizeof(str), "%s%s", get_path_config_base(),
		"custom_files.zip");

	if (update(server, "custom_files.lst", "updates", str,
		progress_function, data) == 0)
	{
		add_zip_archive(str);
	}
	else
	{
		error = 1;
	}

	return 0;
}

void start_custom_update()
{
	if (update_thread != 0)
	{
		return;
	}

	update_mutex = SDL_CreateMutex();

	update_running = 1;

	update_thread = SDL_CreateThread(custom_update_thread, 0);
}

void stopp_custom_update()
{
	int result;

	if (update_thread == 0)
	{
		return;
	}

	CHECK_AND_LOCK_MUTEX(update_mutex);

	update_running = 0;

	CHECK_AND_UNLOCK_MUTEX(update_mutex);

	SDL_WaitThread(update_thread, &result);

	SDL_DestroyMutex(update_mutex);

	update_mutex = 0;
	update_thread = 0;
}

int command_update(char *text, int len)
{
	if (update_mutex == 0)
	{
		LOG_TO_CONSOLE(c_red1, "Update not started");

		return 1;
	}
		
	CHECK_AND_LOCK_MUTEX(update_mutex);

	if (error == 1)
	{
		LOG_TO_CONSOLE(c_red1, update_str);
	}
	else
	{
		LOG_TO_CONSOLE(c_green1, update_str);
	}

	CHECK_AND_UNLOCK_MUTEX(update_mutex);
	
	return 1;
}

