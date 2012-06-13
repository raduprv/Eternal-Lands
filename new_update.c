#include <zlib.h>
#include <time.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <string.h>
#include "md5.h"
#include "elc_private.h"
#include "io/zip.h"
#include "io/unzip.h"
#include "io/ziputil.h"
#include "io/fileutil.h"
#include "io/elfilewrapper.h"
#include "new_update.h"
#include "errors.h"
#include "threads.h"
#include "queue.h"
#include "misc.h"
#include "asc.h"

#define MAX_OLD_UPDATE_FILES	5
#define	UPDATE_DOWNLOAD_THREAD_COUNT 2

typedef struct
{
	const char* server;
	const char* path;
	queue_t* files;
	zipFile dest;
	Uint32 count;
	Uint32 index;
	Uint32 running;
	SDL_Thread* threads[UPDATE_DOWNLOAD_THREAD_COUNT];
	SDL_mutex* mutex;
	SDL_cond* condition;
	progress_fnc update_progress_function;
	void* user_data;
} download_files_thread_data_t;

typedef struct
{
	char file_name[256];
	MD5_DIGEST digest;
} update_info_t;

static Uint32 download_file(const char* file_name, FILE* file,
	const char* server, const char* path, Uint64* file_size,
	const Uint32 size, char* buffer, const Uint32 etag_size,
	char* etag)
{
	char str[64];
	char* pos;
	IPaddress http_ip;
	TCPsocket http_sock;
	Uint32 got_header, http_status;
	int i, len;

	// resolve the hostname
	if ((file_size == 0) || (buffer == 0) || (size == 0))
	{
		LOG_ERROR("buffer: %p, size: %d", buffer, size);

		return 1;  // can't resolve the hostname
	}

	got_header = 0;
	http_status = 0;

	// resolve the hostname
	if (SDLNet_ResolveHost(&http_ip, server, 80) < 0)
	{
		// caution, always port 80!
		return 1;  // can't resolve the hostname
	}

	// open the socket
	http_sock = SDLNet_TCP_Open(&http_ip);

	if (http_sock == 0)
	{
		return 2;  // failed to open the socket
	}

	// send the GET request, try to avoid ISP caching	
	if ((etag != 0) && (strlen(etag) > 0))
	{
		snprintf(buffer, size, "GET %s%s HTTP/1.1\r\nHost: %s\r\n"
			"CONNECTION:CLOSE\r\nREFERER:%s\r\n"
			"USER-AGENT:AUTOUPDATE\r\nIf-None-Match: \"%s\"\r\n\r\n",
			path, file_name, server, FILE_VERSION,
			etag);
	}
	else
	{
		snprintf(buffer, size, "GET %s%s HTTP/1.1\r\nHost: %s\r\n"
			"CONNECTION:CLOSE\r\nCACHE-CONTROL:NO-CACHE\r\nREFERER:%s\r\n"
			"USER-AGENT:AUTOUPDATE\r\n\r\n", path, file_name,
			server, FILE_VERSION);
	}

	if (SDLNet_TCP_Send(http_sock, buffer, size) < (int)size)
	{
		SDLNet_TCP_Close(http_sock);

		return 3;  // error in sending the get request
	}

	*file_size = 0;

	// get the response & data
	do
	{
		memset(buffer, 0, size);
		// get a packet

		len = SDLNet_TCP_Recv(http_sock, buffer, size);
		// have we gotten the full header?

		if (len < 0) 
		{
		    // Read error (connection lost);
		    LOG_ERROR("Read error: %d", len);
		    SDLNet_TCP_Close(http_sock);
		    return 5;
		}

		if (!got_header)
		{
			// check for http status
			sscanf(buffer, "HTTP/%*s %i ", &http_status);

			if ((etag_size > 0) && (etag != 0))
			{
				pos = strstr(buffer, "ETag: \"");

				if (pos != 0)
				{
					memset(etag, 0, etag_size);
					snprintf(str, sizeof(str),
						"ETag: \"%%%ds", etag_size - 1);
					sscanf(pos, str, etag);

					if (strlen(etag) > 0)
					{
						etag[strlen(etag) - 1] = '\0';
					}
				}
			}

			if (http_status != 200)
			{
				break;
			}

			// look for the end of the header (a blank line)
			for (i = 0; i < len; i++)
			{
				if (buffer[i] == 0x0D && buffer[i+1] == 0x0A &&
					buffer[i+2] == 0x0D && buffer[i+3] == 0x0A)
				{
					// flag we got the header and write what is left to the file
					got_header = 1;

					if (http_status == 200)
					{
						*file_size += len - i - 4;

						fwrite(buffer + i + 4, 1,
							len - i - 4, file);
					}
					break;
				}
			}
		}
		else
		{
			if (http_status == 200)
			{
				*file_size += len;

				fwrite(buffer, 1, len, file);
			}
			else
			{
				break;
			}
		}
	}
	while (len > 0);

	SDLNet_TCP_Close(http_sock);

	if (http_status != 200)
	{
		if (http_status != 0)
		{
			return http_status;
		}
		else
		{
			return 5;
		}
	}

	return 0;  // finished
}

static int download_files_thread(void* _data)
{
	char file_name[256];
	char comment[64];
	download_files_thread_data_t *data;
	update_info_t* info;
	char* download_buffer;
	void* file_buffer;
	FILE *file;
	Uint64 file_size, size;
	Uint32 i, len, result, error, count, index, running;
	Uint32 download_buffer_size;

	data = (download_files_thread_data_t*)_data;

	init_thread_log("download_files");

	error = 0;
	result = 0;
	file_buffer = 0;
	download_buffer_size = 4096;
	download_buffer = malloc(download_buffer_size);
	count = data->count;

#ifdef WINDOWS
	file = my_tmpfile();
#else
	file = tmpfile();
#endif

	if (file == 0)
	{
		return 1;
	}

	while (1)
	{
		CHECK_AND_LOCK_MUTEX(data->mutex);

		do
		{
			info = queue_pop(data->files);

			running = data->running;

			if ((running == 1) && (info == 0))
			{
				SDL_CondWait(data->condition, data->mutex);
			}
		}
		while ((data->running == 1) && (info == 0));

		index = data->index;

		CHECK_AND_UNLOCK_MUTEX(data->mutex);

		if (info == 0)
		{
			break;
		}

		if (data->update_progress_function("Downloading files", count,
			index, data->user_data) != 1)
		{
			CHECK_AND_LOCK_MUTEX(data->mutex);

			data->running = 0;

			CHECK_AND_UNLOCK_MUTEX(data->mutex);

			error = 2;

			break;
		}

		for (i = 0; i < 5; i++)
		{
			fseek(file, 0, SEEK_SET);

			result = download_file(info->file_name, file,
				data->server, data->path, &size,
				download_buffer_size,
				download_buffer, 0, 0);

			if (result != 0)
			{
				LOG_ERROR("Download error %d while updating "
					"file '%s' from server '%s', retrying"
					" it", result, info->file_name,
					data->server);

				error = 3;
				continue;
			}

			if (file_read(file, size, &file_buffer, &file_size) !=
				0)
			{
				LOG_ERROR("Read error while updating file '%s' "
					"from server '%s', retrying it",
					info->file_name, data->server);

				error = 3;
				continue;
			}

			convert_md5_digest_to_comment_string(info->digest,
				sizeof(comment), comment);

			len = strlen(info->file_name);

			snprintf(file_name, sizeof(file_name), "%s",
				info->file_name);

			if (has_suffix(file_name, len, ".xz", 3))
			{
				file_name[len - 3] = 0;
			}

			CHECK_AND_LOCK_MUTEX(data->mutex);

			add_to_zip(file_name, file_size, file_buffer,
				data->dest, comment);
			data->index++;

			CHECK_AND_UNLOCK_MUTEX(data->mutex);

			free(file_buffer);

			error = 0;
			break;
		}

		if (error != 0)
		{
			break;
		}
	}

	free(download_buffer);

	fclose(file);

	return error;
}

static void init_threads(download_files_thread_data_t *data, const Uint32 count,
	const char* server, const char* path, zipFile dest,
	progress_fnc update_progress_function, void* user_data)
{
	Uint32 i;

	data->files = 0;

	queue_initialise(&(data->files));
	data->mutex = SDL_CreateMutex();
	data->condition = SDL_CreateCond();

	data->server = server;
	data->path = path;
	data->dest = dest;
	data->count = count;
	data->index = 0;
	data->running = 1;
	data->update_progress_function = update_progress_function;
	data->user_data = user_data;

	for (i = 0; i < UPDATE_DOWNLOAD_THREAD_COUNT; i++)
	{
		data->threads[i] = SDL_CreateThread(download_files_thread,
			data);
	}
}

static void wait_for_threads(download_files_thread_data_t *data, Uint32* error)
{
	Sint32 result;
	Uint32 i;

	CHECK_AND_LOCK_MUTEX(data->mutex);

	if (*error == 0)
	{
		data->running = 2;
	}
	else
	{
		data->running = 0;
	}

	CHECK_AND_UNLOCK_MUTEX(data->mutex);

	for (i = 0; i < UPDATE_DOWNLOAD_THREAD_COUNT; i++)
	{
		SDL_CondBroadcast(data->condition);
		SDL_WaitThread(data->threads[i], &result);

		if (*error == 0)
		{
			*error = result;
		}
	}

	SDL_DestroyMutex(data->mutex);
	SDL_DestroyCond(data->condition);

	while (queue_pop(data->files) != 0);

	queue_destroy(data->files);
}

static Uint32 download_files(update_info_t* infos, const Uint32 count,
	const char* server, const char* path, const Uint32 source_count,
	unzFile* sources, zipFile dest, progress_fnc update_progress_function,
	void* user_data)
{
	char file_name[256];
	download_files_thread_data_t thread_data;
	FILE *file;
	Uint64 len;
	Uint32 i, j, download, error, index;

#ifdef WINDOWS
	file = my_tmpfile();
#else
	file = tmpfile();
#endif

	if (file == 0)
	{
		return 1;
	}

	error = 0;

	init_threads(&thread_data, count, server, path, dest,
		update_progress_function, user_data);

	for (i = 0; i < count; i++)
	{
		CHECK_AND_LOCK_MUTEX(thread_data.mutex);

		index = thread_data.index;

		CHECK_AND_UNLOCK_MUTEX(thread_data.mutex);

		if (update_progress_function("Updating files", count, index,
			user_data) != 1)
		{
			error = 2;

			break;
		}

		download = 1;

		len = strlen(infos[i].file_name);

		snprintf(file_name, sizeof(file_name), "%s",
			infos[i].file_name);

		if (has_suffix(file_name, len, ".xz", 3))
		{
			file_name[len - 3] = 0;
		}

		for (j = 0; j < source_count; j++)
		{
			if (check_md5_from_zip(sources[j], file_name,
				infos[i].digest) == 0)
			{
				CHECK_AND_LOCK_MUTEX(thread_data.mutex);

				copy_from_zip(sources[j], dest);
				thread_data.index++;

				CHECK_AND_UNLOCK_MUTEX(thread_data.mutex);

				download = 0;

				break;
			}
		}

		if (download == 1)
		{
			queue_push(thread_data.files, &infos[i]);
			SDL_CondSignal(thread_data.condition);
		}
	}

	wait_for_threads(&thread_data, &error);

	fclose(file);

	return error;
}

static Uint64 skip_line(const char** buffer, Uint64* size, Uint64* skip)
{
	Uint64 len, i;

	len = 0;

	if (*skip > *size)
	{
		return 0;
	}

	*buffer += *skip;
	*size -= *skip;
	*skip = 0;

	for (i = 0; i < *size; i++)
	{
		if (((*buffer)[i] == 0x0D) || ((*buffer)[i] == 0x0A))
		{
			if ((i + 1) < *size)
			{
				if (((*buffer)[i + 1] == 0x0D) ||
					((*buffer)[i + 1] == 0x0A))
				{
					*skip += 1;
				}
			}

			*skip += len + 1;

			return len;
		}

		len++;
	}

	*skip = *size;

	return *size;
}

static Uint32 add_to_downloads(const char* buffer, const Uint64 buffer_size,
	update_info_t** infos, Uint32* count,
	progress_fnc update_progress_function, void* user_data)
{
	char name[256];
	char md5[256];
	MD5_DIGEST digest;
	const char* line_buffer;
	Uint64 line_size, line_buffer_size, skip;
	Uint32 size;

	skip = 0;
	line_buffer = buffer;
	line_buffer_size = buffer_size;

	line_size = skip_line(&line_buffer, &line_buffer_size, &skip);

	if (strncmp(buffer, "CCCF", line_size) != 0)
	{
		return 0;
	}

	size = 0;

	line_size = skip_line(&line_buffer, &line_buffer_size, &skip);

	while (line_buffer_size > 0)
	{
		// parse the line
		memset(name, 0, sizeof(name));
		memset(md5, 0, sizeof(md5));

		sscanf(line_buffer, "%*[^(](%250[^)])%*[^0-9a-zA-Z]%32s", name,
			md5);

		// check for something to process
		if (*name && *md5 && (name[0] != '/') && (name[0] != '\\') &&
			(name[1] != ':'))
		{
			if (convert_string_to_md5_digest(md5, digest) != 0)
			{
				break;
			}

			if (*count >= size)
			{
				size = *count + 1024;
				*infos = realloc(*infos, size * sizeof(update_info_t));
			}

			if ((name[0] == '.') && (name[1] == '/'))
			{
				memcpy((*infos)[*count].file_name, name + 2, sizeof(name) - 2);
			}
			else
			{
				memcpy((*infos)[*count].file_name, name, sizeof(name));
			}

			memcpy((*infos)[*count].digest, digest, sizeof(MD5_DIGEST));

			*count += 1;
		}

		line_size = skip_line(&line_buffer, &line_buffer_size, &skip);
	}

	return 1;
}

static Uint32 check_server_digest_file(const char* file, FILE* tmp_file,
	const char* server, const char* path, const Uint32 size, char* buffer,
	const Uint32 digest_size, char* digest)
{
	Uint64 file_size;
	Uint32 result;

	result = download_file(file, tmp_file, server, path, &file_size,
		size, buffer, 0, 0);

	if ((result == 0) && (file_size >= digest_size))
	{
		fseek(tmp_file, 0, SEEK_SET);

		memset(buffer, 0, sizeof(buffer));

		result = fread(buffer, digest_size, 1, tmp_file);

		if (result == 1)
		{
			if (memcmp(buffer, digest, digest_size) == 0)
			{
				return 0;
			}

			memcpy(digest, buffer, digest_size);
		}

		return 1;
	}

	return 2;
}

const char* digest_extensions[2] =
{
	".md5", ".xz.md5"
};

static Uint32 check_server_digest_files(const char* file, FILE* tmp_file,
	const char* server, const char* path, const Uint32 size, char* buffer,
	char md5[32])
{
	char file_name[1024];
	Uint32 i, result;

	for (i = 0; i < 2; i++)
	{
		memset(file_name, 0, sizeof(file_name));
		strcpy(file_name, file);
		strcat(file_name, digest_extensions[i]);

		result = check_server_digest_file(file_name, tmp_file, server,
			path, size, buffer, 32, md5);

		if (result < 2)
		{
			return result;
		}
	}

	return 2;
}

static Uint32 build_update_list(const char* server, const char* file,
	const char* path, update_info_t** infos, Uint32* count, char md5[32],
	const Uint32 etag_size, char* etag,
	progress_fnc update_progress_function, void* user_data)
{
	char error_str[4096];
	char file_name[1024];
	char buffer[1024];
	Uint64 file_size;
	FILE* tmp_file;
	void* file_buffer;
	Uint32 result;

#ifdef WINDOWS
	tmp_file = my_tmpfile();
#else
	tmp_file = tmpfile();
#endif

	if (tmp_file == 0)
	{
		LOG_ERROR("Can't get tmp file");
		return 3;
	}

	update_progress_function("Checking for updates", 0, 0, user_data);

	result = check_server_digest_files(file, tmp_file, server, path,
		sizeof(buffer), buffer, md5);

	if (result == 0)
	{
		update_progress_function("No update needed", 0, 0, user_data);
			
		fclose(tmp_file);

		return 1;
	}

	fseek(tmp_file, 0, SEEK_SET);

	result = download_file(file, tmp_file, server, path, &file_size,
		sizeof(buffer), buffer, etag_size, etag);

	if (result == 304)
	{
		fclose(tmp_file);

		update_progress_function("No update needed", 0, 0, user_data);

		return 1;
	}

	if (result != 0)
	{
		memset(file_name, 0, sizeof(file_name));
		strcpy(file_name, file);
		strcat(file_name, ".xz");

		fseek(tmp_file, 0, SEEK_SET);

		result = download_file(file_name, tmp_file, server, path,
			&file_size, sizeof(buffer), buffer, etag_size, etag);

		if (result == 304)
		{
			fclose(tmp_file);

			update_progress_function("No update needed", 0, 0,
				user_data);

			return 1;
		}
	}

	if (result != 0)
	{
		fclose(tmp_file);

		snprintf(error_str, sizeof(error_str), "Can't get update list"
			" file '%s' from server '%s' using path '%s', error %d.",
			file, server, path, result);

		LOG_ERROR(error_str);

		update_progress_function(error_str, 0, 0, user_data);

		return 4;
	}

	*infos = 0;
	*count = 0;

	fseek(tmp_file, 0, SEEK_SET);
	file_read(tmp_file, file_size, &file_buffer, &file_size);
	fclose(tmp_file);

	if (add_to_downloads(file_buffer, file_size, infos, count,
		update_progress_function, user_data) != 1)
	{
		free(file_buffer);

		snprintf(error_str, sizeof(error_str), "Update list"
			" file '%s' from server '%s' using path '%s' has wrong"
			" magic number in first line.", file, server, path);

		LOG_ERROR(error_str);

		update_progress_function(error_str, 0, 0, user_data);

		return 5;
	}

	free(file_buffer);

	return 0;
}

Uint32 update(const char* server, const char* file, const char* dir,
	const char* zip, progress_fnc update_progress_function, void* user_data)
{
	char tmp[MAX_OLD_UPDATE_FILES][1024];
	char path[1024];
	char str[1024];
	char etag[1024];
	char md5[32];
	unzFile source_zips[MAX_OLD_UPDATE_FILES];
	zipFile dest_zip;
	update_info_t* infos;
	Uint32 count, result, i;

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "http://%s/%s/", server, dir);

	snprintf(str, sizeof(str), "Downloading from server %s", path);
	update_progress_function(str, 0, 0, user_data);

	for (i = 0; i < MAX_OLD_UPDATE_FILES; i++)
	{
		memset(tmp[i], 0, sizeof(tmp[i]));

		snprintf(tmp[i], sizeof(tmp[i]), "%s%s%i", zip, ".t", i);
	}

	snprintf(str, sizeof(str), "Opening %s", zip);
	update_progress_function(str, 0, 0, user_data);

	memset(str, 0, sizeof(str));

	source_zips[0] = unzOpen64(zip);
	unzGetGlobalComment(source_zips[0], str, sizeof(str));
	unzClose(source_zips[0]);

	infos = 0;
	count = 0;

	memset(etag, 0, sizeof(etag));
	memset(md5, 0, sizeof(md5));
	sscanf(str, "ETag: %s MD5: %s", etag, md5);

	result = build_update_list(server, file, path, &infos, &count, md5,
		sizeof(etag), etag, update_progress_function, user_data);

	if (result == 1)
	{
		return 0;
	}

	if (result != 0)
	{
		free(infos);

		return 3;
	}

	source_zips[0] = unzOpen64(zip);

	remove(tmp[MAX_OLD_UPDATE_FILES - 1]);

	for (i = MAX_OLD_UPDATE_FILES - 1; i > 0; i--)
	{
		rename(tmp[i - 1], tmp[i]);
		source_zips[i] = unzOpen64(tmp[i]);
	}

	dest_zip = zipOpen64(tmp[0], APPEND_STATUS_CREATE);

	result = download_files(infos, count, server, path,
		MAX_OLD_UPDATE_FILES, source_zips, dest_zip,
		update_progress_function, user_data);

	memset(str, 0, sizeof(str));

	if (result == 0)
	{
		snprintf(str, sizeof(str), "ETag: %s MD5: %s", etag, md5);
	}

	for (i = 0; i < MAX_OLD_UPDATE_FILES; i++)
	{
		unzClose(source_zips[i]);
	}

	zipClose(dest_zip, str);

	if (result == 2)
	{
		update_progress_function("Canceled updating", 0, 0,
			user_data);
	}

	free(infos);

	if (result != 0)
	{
		return result;
	}

	unload_zip_archive(zip);
	remove(zip);
	rename(tmp[0], zip);
	load_zip_archive(zip);

	for (i = 1; i < MAX_OLD_UPDATE_FILES; i++)
	{
		remove(tmp[i]);
	}

	update_progress_function("Update complete", 0, 0, user_data);

	return 0;
}

