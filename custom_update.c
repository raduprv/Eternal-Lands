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
#include "errors.h"
#include "events.h"

typedef struct
{
	char str[4096];
	SDL_Thread* thread;
	SDL_mutex* mutex;
	SDL_cond* condition;
	const char* dir;
	const char* zip;
	const char* name;
	Uint32 running;
	Uint32 error;
} UpdateThreaData_t;

static UpdateThreaData_t update_thread_data[2];

static const char* update_names[2] =
{
	"EL", "Unofficial"
};

static const char* zip_names[2] =
{
	"custom_clothes.zip", "unofficial_custom_clothes.zip"
};

static Uint32 progress_function(const char* str, const Uint32 max,
	const Uint32 current, void* user_data)
{
	UpdateThreaData_t* data;

	data = (UpdateThreaData_t*)user_data;

	CHECK_AND_LOCK_MUTEX(data->mutex);

	if (max > 0)
	{
		safe_snprintf(data->str, sizeof(data->str),
			"%s custom updates: %s %.2f%%", data->name,
			str, (current * 100.0f) / max);
	}
	else
	{
		safe_snprintf(data->str, sizeof(data->str),
			"%s custom updates: %s", data->name, str);
	}

	LOG_INFO("%s", data->str);

	if (data->running == 0)
	{
		CHECK_AND_UNLOCK_MUTEX(data->mutex);

		return 0;
	}

	CHECK_AND_UNLOCK_MUTEX(data->mutex);

	return 1;
}

static Uint32 custom_update_threaded(const char* dir, const char* zip_file,
	void* data)
{
	char *buffer = NULL;
	size_t buffer_size = 1024;
	char *str = NULL;
	size_t str_size = 256;
	char* server;
	FILE* file;
	Uint32 count, index, idx, len, result;
	const char* file_name = "custom_mirrors.lst";

	if ((buffer = (char *)calloc(sizeof(char), buffer_size)) == NULL)
		return 1;
	if ((str = (char *)calloc(sizeof(char), str_size)) == NULL)
	{
		free(buffer);
		return 1;
	}

	safe_snprintf(str, str_size, "%s%s", dir, file_name);

	file = fopen(str, "r");

	if (file == 0)
	{
		safe_snprintf(buffer, buffer_size, "Can't find server list file"
			" '%s'.", str);

		progress_function(buffer, 0, 0, data);

		free(buffer);
		free(str);
		return 1;
	}

	count = 0;
	server = fgets(buffer, buffer_size, file);

	while (server != 0)
	{
		len = strlen(buffer);

		// is this line worth handling?
		if ((len > 6) && (buffer[0] > ' ') && (buffer[0] != '#'))
		{
			count++;
		}

		// read the next line
		server = fgets(buffer, buffer_size, file);
	}

	if (count == 0)
	{
		safe_snprintf(buffer, buffer_size, "No server in file '%s'", str);

		progress_function(buffer, 0, 0, data);

		fclose(file);

		free(buffer);
		free(str);
		return 1;
	}

	rewind(file);

	srand(time(0));

	idx = rand() % count;

	index = 0;

	server = fgets(buffer, buffer_size, file);

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
		server = fgets(buffer, buffer_size, file);
	}

	fclose(file);

	if (server == 0)
	{
		safe_snprintf(buffer, buffer_size, "Can't get server from file"
			" '%s'", str);

		progress_function(buffer, 0, 0, data);

		free(buffer);
		free(str);
		return 1;
	}

	safe_snprintf(str, str_size, "Downloading from server %s", server);
	progress_function(str, 0, 0, data);

	safe_snprintf(str, str_size, "%s%s", get_path_config_base(), zip_file);

	result = update(server, "custom_files.lst", "updates", str,
		progress_function, data);

	free(buffer);
	free(str);

	return result;
}

static int custom_update_thread(void* thread_data)
{
	SDL_Event event;
	UpdateThreaData_t* data;
	Uint32 result;

	result = 0;
	data = (UpdateThreaData_t*)thread_data;

	init_thread_log("custom_update");

	while (1)
	{
		CHECK_AND_LOCK_MUTEX(data->mutex);

		data->error = result;

		while (data->running == 1)
		{
			SDL_CondWait(data->condition, data->mutex);
		}

		if (data->running == 0)
		{
			CHECK_AND_UNLOCK_MUTEX(data->mutex);

			return 1;
		}

		data->error = 0;
		safe_snprintf(data->str, sizeof(data->str), "%s custom updates: %s",
			data->name, "started");

		CHECK_AND_UNLOCK_MUTEX(data->mutex);

		result = custom_update_threaded(data->dir, data->zip, data);

		CHECK_AND_LOCK_MUTEX(data->mutex);

		if (data->running == 2)
		{
			data->running = 1;
		}

		LOG_INFO("%s", data->str);

		CHECK_AND_UNLOCK_MUTEX(data->mutex);

		// signal we are done
		event.type = SDL_USEREVENT;
		event.user.code = EVENT_CUSTOM_UPDATE_COMPLETE;
		event.user.data1 = 0;
		SDL_PushEvent(&event);
	}

	return 0;
}

void init_custom_update()
{
	char *str = NULL;
	size_t str_size = 256;
	Uint32 i;

	memset(update_thread_data, 0, sizeof(update_thread_data));

	update_thread_data[0].dir = datadir;
	update_thread_data[1].dir = get_path_config_base();

	if ((str = (char *)calloc(sizeof(char), str_size)) == NULL)
		return;

	for (i = 0; i < 2; i++)
	{
		safe_snprintf(str, str_size, "%s%s", get_path_config_base(),
			zip_names[i]);
		load_zip_archive(str);

		safe_snprintf(update_thread_data[i].str,
			sizeof(update_thread_data[i].str),
			"%s custom updates: %s", update_names[i], "waiting");
		update_thread_data[i].mutex = SDL_CreateMutex();
		update_thread_data[i].condition = SDL_CreateCond();
		update_thread_data[i].zip = zip_names[i];
		update_thread_data[i].name = update_names[i];
		update_thread_data[i].running = 1;
		update_thread_data[i].error = 0;
		update_thread_data[i].thread = SDL_CreateThread(
			custom_update_thread, &update_thread_data[i]);
	}
	free(str);
}

void start_custom_update()
{
	Uint32 i;

	for (i = 0; i < 2; i++)
	{
		CHECK_AND_LOCK_MUTEX(update_thread_data[i].mutex);

		if (update_thread_data[i].running == 1)
		{
			update_thread_data[i].running = 2;
		}

		CHECK_AND_UNLOCK_MUTEX(update_thread_data[i].mutex);

		SDL_CondSignal(update_thread_data[i].condition);
	}
}

void stopp_custom_update()
{
	Uint32 i;
	int result;

	for (i = 0; i < 2; i++)
	{
		CHECK_AND_LOCK_MUTEX(update_thread_data[i].mutex);

		update_thread_data[i].running = 0;

		CHECK_AND_UNLOCK_MUTEX(update_thread_data[i].mutex);

		SDL_CondSignal(update_thread_data[i].condition);
		SDL_WaitThread(update_thread_data[i].thread, &result);

		SDL_DestroyCond(update_thread_data[i].condition);
		SDL_DestroyMutex(update_thread_data[i].mutex);

		update_thread_data[i].mutex = 0;
		update_thread_data[i].thread = 0;
	}
}

int command_update_status(char *text, int len)
{
	Uint32 i;

	if (custom_update == 0)
	{
		LOG_TO_CONSOLE(c_red1, "Custom updates disabled");

		return 1;
	}

	for (i = 0; i < 2; i++)
	{
		CHECK_AND_LOCK_MUTEX(update_thread_data[i].mutex);

		switch (update_thread_data[i].error)
		{
			case 0:
				LOG_TO_CONSOLE(c_green1,
					update_thread_data[i].str);
				break;
			case 1:
				LOG_TO_CONSOLE(c_orange1,
					update_thread_data[i].str);
				break;
			default:
				LOG_TO_CONSOLE(c_red1,
					update_thread_data[i].str);
				break;
		}

		CHECK_AND_UNLOCK_MUTEX(update_thread_data[i].mutex);
	}

	return 1;
}

int command_update(char *text, int len)
{
	if (custom_update == 0)
	{
		LOG_TO_CONSOLE(c_red1, "Custom updates disabled");
	}
	else
	{
		start_custom_update();
		LOG_TO_CONSOLE(c_green1, "Custom updates started. "
			"Use #update_status to check progress");
	}

	return 1;
}

