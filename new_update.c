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
#include "new_update.h"

#define MAX_OLD_UPDATE_FILES	5

typedef struct
{
	char file_name[256];
	MD5_DIGEST digest;
} update_info_t;

Uint32 download_file(const char* file_name, FILE* file, const char* server,
	const char* path, const Uint32 size, char* buffer)
{
	IPaddress http_ip;
	TCPsocket http_sock;
	Uint32 i, len, got_header, http_status;

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
	
	snprintf(buffer, size, "GET %s%s HTTP/1.1\r\nHost: %s\r\n"
		"CONNECTION:CLOSE\r\nCACHE-CONTROL:NO-CACHE\r\nREFERER:%s\r\n"
		"USER-AGENT:AUTOUPDATE %s\r\n\r\n", path, file_name,
		server, "autoupdate", FILE_VERSION);

	len = strlen(buffer);

	if (SDLNet_TCP_Send(http_sock, buffer, size) < size)
	{
		SDLNet_TCP_Close(http_sock);

		return 3;  // error in sending the get request
	}

	// get the response & data
	while (len > 0)
	{
		memset(buffer, 0, size);
		// get a packet

		len = SDLNet_TCP_Recv(http_sock, buffer, size);
		// have we gotten the full header?

		if (!got_header)
		{
			// check for http status
			sscanf(buffer, "HTTP/%*s %i ", &http_status);

			// look for the end of the header (a blank line)
			for (i = 0; i < len && !got_header; i++)
			{
				if (buffer[i] == 0x0D && buffer[i+1] == 0x0A &&
					buffer[i+2] == 0x0D && buffer[i+3] == 0x0A)
				{
					// flag we got the header and write what is left to the file
					got_header = 1;

					if (http_status == 200)
					{
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
				fwrite(buffer, 1, len, file);
			}
			else
			{
				break;
			}
		}
	}

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

Uint32 download_files(update_info_t* infos, const Uint32 count,
	const char* server, const char* path, const Uint32 source_count,
	unzFile* sources, zipFile dest, progress_fnc update_progress_function,
	void* user_data)
{
	char buffer[4096];
	MD5 md5;
	MD5_DIGEST digest;
	FILE *file;
	void* data;
	Uint32 i, j, result, size, download, error;

	file = tmpfile();

	if (file == 0)
	{
		return 1;
	}

	data = 0;
	error = 0;
	result = 0;

	for (i = 0; i < count; i++)
	{
		if (update_progress_function("Updating", infos[i].file_name,
			count, i, user_data) != 1)
		{
			error = 2;

			break;
		}

		download = 1;

		for (j = 0; j < source_count; j++)
		{
			if (check_md5_from_zip(sources[j], infos[i].file_name,
				infos[i].digest) == 1)
			{
				copy_from_zip(sources[j], dest);

				download = 0;

				break;
			}
		}

		if (download == 1)
		{
			fseek(file, 0, SEEK_SET);

			result = download_file(infos[i].file_name, file,
				server, path, sizeof(buffer), buffer);

			if (result == 0)
			{
				size = ftell(file);

				fseek(file, 0, SEEK_SET);

				data = realloc(data, size);

				if (fread(data, size, 1, file) != 1)
				{
					error = 3;
					break;
				}

				MD5Open(&md5);
				MD5Digest(&md5, data, size);
				MD5Close(&md5, digest);

				if (memcmp(digest, infos[i].digest,
					sizeof(MD5_DIGEST)) == 0)
				{
					add_to_zip(infos[i].file_name, size,
						data, dest);
				}
			}
			else
			{
				error = 3;
			}
		}
	}

	free(data);

	fclose(file);

	return error;
}

Uint32 read_line(FILE* file, const Uint32 size, char* buffer)
{
	Sint64 len, skip;
	char* ptr;

	len = fread(buffer, 1, size, file);

	if (len > 0)
	{
		buffer[len - 1] = 0;
	}
	else
	{
		return 0;
	}

	skip = 0;

	ptr = strchr(buffer, 0x0D);

	if (ptr != 0)
	{
		*ptr = 0;
		skip += 1;
	}

	ptr = strchr(buffer, 0x0A);

	if (ptr != 0)
	{
		*ptr = 0;
		skip += 1;
	}

	skip += strlen(buffer) - len + 1;

	fseek(file, skip, SEEK_CUR);

	return 1;
}

Uint32 add_to_downloads(FILE* file, update_info_t** infos, Uint32* count,
	progress_fnc update_progress_function, void* user_data)
{
	char buffer[1024];
	char name[256];
	char md5[256];
	MD5_DIGEST digest;
	Uint32 size;

	if ((file == 0) || (infos == 0) || (count == 0))
	{
		return 0;
	}

	read_line(file, sizeof(buffer), buffer);

	if (strcmp(buffer, "CCCF") != 0)
	{
		fclose(file);

		return 0;
	}

	size = 0;

	while (read_line(file, sizeof(buffer), buffer) == 1)
	{
		// parse the line
		memset(name, 0, sizeof(name));
		memset(md5, 0, sizeof(md5));

		sscanf(buffer, "%*[^(](%250[^)])%*[^0-9a-zA-Z]%32s", name, md5);

		// check for something to process
		if (*name && *md5 && !strstr(name, "..") && name[0] != '/' && name[0] != '\\' && name[1] != ':')
		{
			if (convert_string_to_md5_digest(md5, digest) != 1)
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

		if (feof(file) != 0)
		{
			break;
		}
	}

	// close the file
	fclose(file);

	return 1;
}

Uint32 read_digest_file(FILE* file, MD5_DIGEST digest)
{
	char buffer[64];

	if (file == 0)
	{
		return 0;
	}

	memset(buffer, 0, sizeof(buffer));

	if (fread(buffer, 32, 1, file) != 1)
		return 0;

	return convert_string_to_md5_digest(buffer, digest);
}

Uint32 get_server_md5(const char* server, const char* file, const char* path,
	MD5_DIGEST digest)
{
	char buffer[1024];
	FILE *tmp_file;
	Uint32 result;	

	tmp_file = tmpfile();

	if (tmp_file == 0)
	{
		return 1;
	}

	if (download_file(file, tmp_file, server, path,
		sizeof(buffer), buffer) != 0)
	{
		fclose(tmp_file);

		return 2;
	}

	fseek(tmp_file, 0, SEEK_SET);

	result = read_digest_file(tmp_file, digest);

	fclose(tmp_file);

	if (result != 1)
	{
		return 3;
	}

	return 0;
}

Uint32 check_updates(const char* server, const char* file, const char* path,
	MD5_DIGEST digest, progress_fnc update_progress_function,
	void* user_data)
{
	char file_name[256];
	MD5_DIGEST server_digest;

	update_progress_function("Checking for updates", "", 0, 0, user_data);

	memset(file_name, 0, sizeof(file_name));
	strcpy(file_name, file);
	strcat(file_name, ".md5");

	if (get_server_md5(server, file_name, path, server_digest) != 0)
	{
		return 2;
	}

	if (memcmp(digest, server_digest, sizeof(MD5_DIGEST)) == 0)
	{
		return 0;
	}

	memcpy(digest, server_digest, sizeof(MD5_DIGEST));

	return 1;
}

Uint32 build_update_list(const char* server, const char* file,
	const char* path, update_info_t** infos, Uint32* count,
	progress_fnc update_progress_function, void* user_data)
{
	char buffer[1024];
	FILE* tmp_file;

	tmp_file = tmpfile();

	if (tmp_file == 0)
	{
		return 3;
	}

	if (download_file(file, tmp_file, server, path, sizeof(buffer),
		buffer) != 0)
	{
		fclose(tmp_file);

		return 4;
	}

	*infos = 0;
	*count = 0;

	fseek(tmp_file, 0, SEEK_SET);

	if (add_to_downloads(tmp_file, infos, count, update_progress_function,
		user_data) != 1)
	{
		return 5;
	}

	return 0;
}

Uint32 update(const char* server, const char* file, const char* dir,
	const char* zip, progress_fnc update_progress_function, void* user_data)
{
	char path[1024];
	char tmp[MAX_OLD_UPDATE_FILES][1024];
	char str[64];
	MD5_DIGEST digest;
	unzFile source_zips[MAX_OLD_UPDATE_FILES];
	zipFile dest_zip;
	update_info_t* infos;
	Uint32 count, result, i;

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "http://%s/%s/", server, dir);

	for (i = 0; i < MAX_OLD_UPDATE_FILES; i++)
	{
		memset(tmp[i], 0, sizeof(tmp[i]));

		snprintf(tmp[i], sizeof(tmp[i]), "%s%s%i", zip, ".t", i);
	}

	memset(str, 0, sizeof(str));

	source_zips[0] = unzOpen64(zip);
	unzGetGlobalComment(source_zips[0], str, sizeof(str));
	unzClose(source_zips[0]);

	convert_comment_string_to_md5_digest(str, digest);

	result = check_updates(server, file, path, digest,
		update_progress_function, user_data);

	if (result == 0)
	{
		update_progress_function("Update complete", "", 0, 0,
			user_data);

		return 0;
	}

	if (result >= 2)
	{
		update_progress_function("Can't get update list", "", 0, 0,
			user_data);

		return 1;
	}

	infos = 0;
	count = 0;

	result = build_update_list(server, file, path, &infos, &count,
		update_progress_function, user_data);

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
		convert_md5_digest_to_comment_string(digest, sizeof(str), str);
	}

	for (i = 0; i < MAX_OLD_UPDATE_FILES; i++)
	{
		unzClose(source_zips[i]);
	}

	zipClose(dest_zip, str);

	if (result != 0)
	{
		if (result == 2)
		{
			update_progress_function("Canceled updating", "", 0, 0,
				user_data);
		}
		else
		{
			update_progress_function("Failed downlaoding updates",
				"", 0, 0, user_data);
		}
	}

	free(infos);

	if (result != 0)
	{
		return 4;
	}

	remove(zip);

	rename(tmp[0], zip);

	for (i = 1; i < MAX_OLD_UPDATE_FILES; i++)
	{
		remove(tmp[i]);
	}

	update_progress_function("Update complete", "", 0, 0, user_data);

	return 0;
}

