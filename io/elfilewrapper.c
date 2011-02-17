#include "elfilewrapper.h"
#include "unzip.h"
#include "elpathwrapper.h"
#include <sys/stat.h>
#include "../elc_private.h"
#include "../errors.h"
#include "../asc.h"
#include "../init.h"
#include "../threads.h"

struct el_file_t
{
	Sint64 size;
	Sint64 position;
	void* buffer;
	char* file_name;
	Uint32 crc32;
};

typedef struct
{
	unzFile file;
	SDL_mutex* mutex;
} el_zip_file_t;

#define MAX_NUM_ZIP_FILES 128

Uint32 num_zip_files = 0;
el_zip_file_t zip_files[MAX_NUM_ZIP_FILES];

void add_zip_archive(const char* file_name)
{
	if (num_zip_files < MAX_NUM_ZIP_FILES)
	{
		zip_files[num_zip_files].file = unzOpen64(file_name);
		zip_files[num_zip_files].mutex = SDL_CreateMutex();

		num_zip_files++;
	}
	else
	{
		LOG_ERROR("Can't add zip file %s", file_name);
	}
}

static Uint32 do_file_exists(const char* file_name, const char* path,
	const Uint32 size, char* buffer)
{
	struct stat fstat;

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);
	safe_strcat(buffer, ".gz", size);

	if (stat(buffer, &fstat) == 0)
	{
		return 1;
	}

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);

	if (stat(buffer, &fstat) == 0)
	{
		return 1;
	}

	return 0;
}

static Uint32 file_exists_path(const char* file_name, const char* extra_path)
{
	char str[1024];
	Uint32 i;

	if (file_name == 0)
	{
		return 0;
	}

	if (extra_path != 0)
	{
		if (do_file_exists(file_name, extra_path, sizeof(str), str) == 1)
		{
			return 1;
		}
	}

	if (do_file_exists(file_name, get_path_updates(), sizeof(str), str) == 1)
	{
		return 1;
	}

	if (do_file_exists(file_name, datadir, sizeof(str), str) == 1)
	{
		return 1;
	}

	for (i = 0; i < num_zip_files; i++)
	{
		if (unzLocateFile(zip_files[i].file, file_name, 1) == UNZ_OK)
		{
			return 1;
		}
	}

	return 0;
}

el_file_ptr gz_file_open(const char* file_name)
{
	gzFile file;
	el_file_ptr result;
	Sint64 read, size;
	Uint32 file_name_len;

	file = gzopen(file_name, "rb");

	if (file == 0)
	{
		LOG_ERROR("Can't open file '%s'", file_name);

		return 0;
	}

#ifdef	EXTRA_DEBUG
	LOG_EXTRA_INFO("File '%s' opened.", file_name);
#endif	// EXTRA_DEBUG

	result = malloc(sizeof(el_file_t));
	memset(result, 0, sizeof(el_file_t));

	file_name_len = strlen(file_name) + 1;
	result->file_name = malloc(file_name_len);
	safe_strncpy(result->file_name, file_name, file_name_len);

	size = 0;

#if	(ZLIB_VERNUM >= 0x1235)
	gzbuffer(file, 0x40000); // 256k
#endif

	do
	{
		result->buffer = realloc(result->buffer, size + 0x40000);

		read = gzread(file, result->buffer + size, 0x40000);

		size += read;
	}
	while (gzeof(file) == 0);

	result->buffer = realloc(result->buffer, size);
	result->size = size;
	result->crc32 = crc32(0, result->buffer, result->size);

	gzclose(file);

	return result;
}

el_file_ptr zip_file_open(unzFile file, const char* file_name)
{
	unz_file_info64 file_info;
	el_file_ptr result;
	Uint32 file_name_len;

	if (unzOpenCurrentFile(file) != UNZ_OK)
	{
		return 0;
	}

	if (unzGetCurrentFileInfo64(file, &file_info, 0, 0, 0, 0, 0, 0) !=
		UNZ_OK)
	{
		return 0;
	}

	result = malloc(sizeof(el_file_t));
	memset(result, 0, sizeof(el_file_t));

	file_name_len = strlen(file_name) + 1;
	result->file_name = malloc(file_name_len);
	safe_strncpy(result->file_name, file_name, file_name_len);

	result->size = file_info.uncompressed_size;
	result->crc32 = file_info.crc;
	result->buffer = malloc(file_info.uncompressed_size);

	if (unzReadCurrentFile(file, result->buffer, result->size) < 0)
	{
		free(result->file_name);
		free(result->buffer);
		free(result);

		return 0;
	}

	if (unzCloseCurrentFile(file) != UNZ_OK)
	{
		free(result->file_name);
		free(result->buffer);
		free(result);

		return 0;
	}

	return result;
}

el_file_ptr file_open(const char* file_name, const char* extra_path)
{
	char str[1024];
	el_file_ptr result;
	Uint32 i;

	if (file_name == 0)
	{
		return 0;
	}

	if (extra_path != 0)
	{
		if (do_file_exists(file_name, extra_path, sizeof(str), str) == 1)
		{
			return gz_file_open(str);
		}
	}

	if (do_file_exists(file_name, get_path_updates(), sizeof(str), str) == 1)
	{
		return gz_file_open(str);
	}

	if (do_file_exists(file_name, datadir, sizeof(str), str) == 1)
	{
		return gz_file_open(str);
	}

	for (i = 0; i < num_zip_files; i++)
	{
		CHECK_AND_LOCK_MUTEX(zip_files[i].mutex);

		if (unzLocateFile(zip_files[i].file, file_name, 1) == UNZ_OK)
		{
			result = zip_file_open(zip_files[i].file, file_name);

			CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);

			return result;
		}

		CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);
	}

	return 0;
}

el_file_ptr el_open(const char* file_name)
{
	return file_open(file_name, 0);
}

el_file_ptr el_open_custom(const char* file_name)
{
	return file_open(file_name, get_path_config_base());
}

el_file_ptr el_open_anywhere(const char* file_name)
{
	return file_open(file_name, get_path_config());
}

Sint64 el_read(el_file_ptr file, Sint64 size, void* buffer)
{
	Sint64 count;

	if (file == 0)
	{
		return -1;
	}

	count = file->size - file->position;

	if (count > size)
	{
		count = size;
	}

	if (count <= 0)
	{
		return 0;
	}

	memcpy(buffer, file->buffer + file->position, count);

	file->position += count;

	return count;
}

Sint64 el_seek(el_file_ptr file, Sint64 offset, int seek_type)
{
	Sint64 pos;

	if (file == 0)
	{
		return -1;
	}

	switch (seek_type)
	{
		case SEEK_SET:
			pos = offset;
			break;
		case SEEK_END:
			pos = file->size - offset;
			break;
		case SEEK_CUR:
			pos = file->position + offset;
			break;
		default:
			return -1;
	}

	if ((pos < 0) || (pos > file->size))
	{
		return -1;
	}
	else
	{
		file->position = pos;

		return pos;
	}
}

Sint64 el_tell(el_file_ptr file)
{
	if (file == 0)
	{
		return -1;
	}

	return file->position;
}

Sint64 el_get_size(el_file_ptr file)
{
	if (file == 0)
	{
		return -1;
	}

	return file->size;
}

void el_close(el_file_ptr file)
{
	if (file == 0)
	{
		return;
	}

	free(file->file_name);
	free(file);
}

void* el_get_pointer(el_file_ptr file)
{
	if (file == 0)
	{
		return 0;
	}

	return file->buffer;
}

int el_file_exists(const char* file_name)
{
	return file_exists_path(file_name, 0);
}

int el_custom_file_exists(const char* file_name)
{
	return file_exists_path(file_name, get_path_config_base());
}

int el_file_exists_anywhere(const char* file_name)
{
	return file_exists_path(file_name, get_path_config());
}

const char* el_file_name(el_file_ptr file)
{
	if (file == 0)
	{
		return 0;
	}

	return file->file_name;
}

Uint32 el_crc32(el_file_ptr file)
{
	if (file == 0)
	{
		return 0;
	}

	return file->crc32;
}

