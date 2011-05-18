#include "elfilewrapper.h"
#include "unzip.h"
#include "elpathwrapper.h"
#include "fileutil.h"
#include <sys/stat.h>
#include "../elc_private.h"
#include "../errors.h"
#include "../asc.h"
#include "../init.h"
#include "../threads.h"
#include "../hash.h"
#include "../xz/7zCrc.h"

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
	unz64_file_pos position;
	char* file_name;
	Uint32 hash;
} el_zip_file_entry_t;

typedef struct
{
	char* file_name;
	unzFile file;
	SDL_mutex* mutex;
	el_zip_file_entry_t* files;
	Uint32 count;
} el_zip_file_t;

static void free_el_file(el_file_t* file)
{
	if (file == 0)
	{
		return;
	}

	free(file->buffer);
	free(file->file_name);
	free(file);
}

int compare_el_zip_file_entry(const void* a, const void* b)
{
	if (((el_zip_file_entry_t*)a)->hash == ((el_zip_file_entry_t*)b)->hash)
	{
		return strcmp(((el_zip_file_entry_t*)a)->file_name,
			((el_zip_file_entry_t*)b)->file_name);
	}
	else
	{
		if (((el_zip_file_entry_t*)a)->hash <
			((el_zip_file_entry_t*)b)->hash)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}

#define MAX_NUM_ZIP_FILES 128

Uint32 num_zip_files = 0;
el_zip_file_t zip_files[MAX_NUM_ZIP_FILES];
SDL_mutex* zip_mutex;

static void clear_zip(el_zip_file_t* zip)
{
	Uint32 i;

	if (zip == 0)
	{
		LOG_ERROR("Invalid zip");

		return;
	}

	CHECK_AND_LOCK_MUTEX(zip->mutex);

	LOG_DEBUG("Clearing zip file '%s'", zip->file_name);

	for (i = 0; i < zip->count; i++)
	{
		free(zip->files[i].file_name);
	}

	if (zip->files != 0)
	{
		free(zip->files);
	}

	if (zip->file_name != 0)
	{
		free(zip->file_name);
	}

	if (zip->file != 0)
	{
		unzClose(zip->file);
	}

	zip->file = 0;
	zip->count = 0;
	zip->files = 0;
	zip->file_name = 0;

	CHECK_AND_UNLOCK_MUTEX(zip->mutex);
}

static Uint32 find_in_zip(el_zip_file_t* zip, const el_zip_file_entry_t* key)
{
	if ((zip == 0) || (key == 0))
	{
		LOG_ERROR("Invalid key or zip");

		return 0;
	}

	LOG_DEBUG("Searching file '%s' in zip file '%s'.", zip->file_name,
		key->file_name);

	if (zip->count == 0)
	{
		return 0;
	}

	if (bsearch(key, zip->files, zip->count, sizeof(el_zip_file_entry_t),
		compare_el_zip_file_entry) != 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static Uint32 locate_in_zip(el_zip_file_t* zip, const el_zip_file_entry_t* key)
{
	el_zip_file_entry_t* file;

	if ((zip == 0) || (key == 0))
	{
		LOG_ERROR("Invalid key or zip");

		return 0;
	}

	if (zip->count == 0)
	{
		return 0;
	}

	file = (el_zip_file_entry_t*) bsearch(key, zip->files, zip->count,
		sizeof(el_zip_file_entry_t), compare_el_zip_file_entry);

	if (file != 0)
	{
		unzGoToFilePos64(zip->file, &file->position);

		return 1;
	}
	else
	{
		return 0;
	}
}

static void init_key(const char* file_name, el_zip_file_entry_t* key,
	const Uint32 size, char* buffer)
{
	char* ptr;
	Uint32 src_idx, dst_idx, len, count;

	if ((key == 0) || (file_name == 0))
	{
		LOG_ERROR("Invalid key or file_name");

		return;
	}

	memset(key, 0, sizeof(el_zip_file_entry_t));
	memset(buffer, 0, size);

	len = strlen(file_name);
	src_idx = 0;
	dst_idx = 0;

	while ((src_idx < len) && (dst_idx < size))
	{
		ptr = strstr(file_name + src_idx, "./");

		if (ptr != 0)
		{
			count = ptr - file_name + src_idx;

			memcpy(buffer + dst_idx, file_name + src_idx,
				min2u(count, size - dst_idx - 1));
		}
		else
		{
			count = len - src_idx;

			memcpy(buffer + dst_idx, file_name + src_idx,
				min2u(count, size - dst_idx - 1));
		}

		dst_idx += count;
		src_idx += count + 2;
	}

	len = strlen(buffer);

	key->hash = mem_hash(buffer, len);
	key->file_name = buffer;
}

void clear_zip_archives()
{
	Uint32 i;

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	for (i = 0; i < MAX_NUM_ZIP_FILES; i++)
	{
		clear_zip(&zip_files[i]);

		SDL_DestroyMutex(zip_files[i].mutex);
	}

	num_zip_files = 0;

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	SDL_DestroyMutex(zip_mutex);
}

void init_zip_archives()
{
	Uint32 i;

	zip_mutex = SDL_CreateMutex();

	for (i = 0; i < MAX_NUM_ZIP_FILES; i++)
	{
		memset(&zip_files[i], 0, sizeof(el_zip_file_t));

		zip_files[i].mutex = SDL_CreateMutex();
	}
}

void load_zip_archive(const char* file_name)
{
	unzFile file;
	unz_file_info64 info;
	unz_global_info64 global_info;
	el_zip_file_entry_t* files;
	char* name;
	Uint32 i, count, size, index;

	if (file_name == 0)
	{
		LOG_ERROR("Empty zip file name", file_name);

		return;
	}

	if (num_zip_files >= MAX_NUM_ZIP_FILES)
	{
		LOG_ERROR("Can't add zip file %s", file_name);

		return;
	}

	file = unzOpen64(file_name);

	if (unzGetGlobalInfo64(file, &global_info) != UNZ_OK)
	{
		LOG_ERROR("Can't load zip file %s", file_name);

		unzClose(file);

		return;
	}

	count = global_info.number_entry;

	if (unzGoToFirstFile(file) != UNZ_OK)
	{
		LOG_ERROR("Can't load zip file %s", file_name);

		unzClose(file);

		return;
	}

	ENTER_DEBUG_MARK("load zip");

	LOG_DEBUG("Loading zip file '%s' with %d files", file_name, count);

	files = malloc(count * sizeof(el_zip_file_entry_t));

	for (i = 0; i < count; i++)
	{
		unzGetFilePos64(file, &files[i].position);

		unzGetCurrentFileInfo64(file, &info, 0, 0, 0, 0, 0, 0);

		size = info.size_filename;

		files[i].file_name = malloc(size + 1);
		memset(files[i].file_name, 0, size + 1);

		unzGetCurrentFileInfo64(file, 0, files[i].file_name, size,
			0, 0, 0, 0);

		LOG_DEBUG("Loading file (%d) '%s' from zip file '%s'.", i,
			files[i].file_name, file_name);

		files[i].hash = mem_hash(files[i].file_name, size);

		unzGoToNextFile(file);
	}

	size = strlen(file_name);
	name = malloc(size + 1);
	memset(name, 0, size + 1);
	memcpy(name, file_name, size);

	LOG_DEBUG("Sorting files from zip file '%s'.", file_name);

	qsort(files, count, sizeof(el_zip_file_entry_t),
		compare_el_zip_file_entry);

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	index = num_zip_files;

	for (i = 0; i < num_zip_files; i++)
	{
		if (zip_files[i].file_name == 0)
		{
			index = i;

			break;
		}
	}

	num_zip_files = max2u(num_zip_files, index + 1);

	CHECK_AND_LOCK_MUTEX(zip_files[index].mutex);

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	LOG_DEBUG("Adding zip file '%s' at position %d.", file_name, index);

	zip_files[index].file_name = name;
	zip_files[index].file = file;
	zip_files[index].files = files;
	zip_files[index].count = count;

	CHECK_AND_UNLOCK_MUTEX(zip_files[index].mutex);

	LEAVE_DEBUG_MARK("load zip");

	LOG_INFO("Loaded zip file '%s' with %d files", file_name, count);
}

void unload_zip_archive(const char* file_name)
{
	Uint32 i, count;

	if (file_name == 0)
	{
		LOG_ERROR("Invalid file name");

		return;
	}

	ENTER_DEBUG_MARK("unload zip");

	LOG_DEBUG("Unloading zip '%s'.", file_name);

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	count = num_zip_files;

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	LOG_DEBUG("Checking %d zip files", count);

	for (i = 0; i < count; i++)
	{
		CHECK_AND_LOCK_MUTEX(zip_files[i].mutex);

		LOG_DEBUG("Checking zip file '%s'", zip_files[i].file_name);

		if (zip_files[i].file_name != 0)
		{
			if (strcmp(zip_files[i].file_name, file_name) == 0)
			{
				clear_zip(&zip_files[i]);

				CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);

				LEAVE_DEBUG_MARK("unload zip");

				LOG_INFO("Unloaded zip '%s'", file_name);

				return;
			}
		}

		CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);
	}

	LEAVE_DEBUG_MARK("unload zip");
}

static Uint32 do_file_exists(const char* file_name, const char* path,
	const Uint32 size, char* buffer)
{
	struct stat fstat;

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);
	safe_strcat(buffer, ".xz", size);

	LOG_DEBUG("Checking file '%s' exits.", buffer);

	if (stat(buffer, &fstat) == 0)
	{
		return 1;
	}

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);
	safe_strcat(buffer, ".gz", size);

	LOG_DEBUG("Checking file '%s' exits.", buffer);

	if (stat(buffer, &fstat) == 0)
	{
		return 1;
	}

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);

	LOG_DEBUG("Checking file '%s' exits.", buffer);

	if (stat(buffer, &fstat) == 0)
	{
		return 1;
	}

	return 0;
}

static Uint32 file_exists_path(const char* file_name, const char* extra_path)
{
	char str[1024];
	el_zip_file_entry_t key;
	Sint32 i, count;

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

	init_key(file_name, &key, sizeof(str), str);

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	count = num_zip_files - 1;

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	for (i = count; i >= 0; i--)
	{
		CHECK_AND_LOCK_MUTEX(zip_files[i].mutex);

		if (find_in_zip(&zip_files[i], &key) == 1)
		{
			CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);

			return 1;
		}

		CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);
	}

	if (do_file_exists(file_name, datadir, sizeof(str), str) == 1)
	{
		return 1;
	}

	return 0;
}

static el_file_ptr xz_file_open(const char* file_name)
{
	el_file_ptr result;
	FILE* file;
	Uint64 size;
	Uint32 file_name_len, error;

	file = fopen(file_name, "rb");

	if (file == 0)
	{
		LOG_ERROR("Can't open file '%s'", file_name);

		return 0;
	}

	result = malloc(sizeof(el_file_t));
	memset(result, 0, sizeof(el_file_t));

	error = xz_file_read(file, &(result->buffer), &size);
	result->size = size;

	fclose(file);

	if (error == 0)
	{
		file_name_len = strlen(file_name) + 1;
		result->file_name = malloc(file_name_len);
		safe_strncpy(result->file_name, file_name, file_name_len);

		result->crc32 = CrcCalc(result->buffer, result->size);

		LOG_DEBUG("File '%s' [crc:0x%08X] opened.", file_name,
			result->crc32);

		return result;
	}

	free(result);

	return 0;
}

static el_file_ptr gz_file_open(const char* file_name)
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
	result->crc32 = CrcCalc(result->buffer, result->size);

	gzclose(file);

	LOG_DEBUG_VERBOSE("File '%s' [crc:0x%08X] opened.", file_name,
		result->crc32);

	return result;
}

static el_file_ptr xz_gz_file_open(const char* file_name)
{
	el_file_ptr result;

	result = xz_file_open(file_name);

	if (result == 0)
	{
		result = gz_file_open(file_name);
	}

	return result;
}

static el_file_ptr zip_file_open(unzFile file)
{
	unz_file_info64 file_info;
	el_file_ptr result;
	Uint32 size, crc;

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

	size = file_info.size_filename;
	result->file_name = malloc(size + 1);
	memset(result->file_name, 0, size + 1);

	result->size = file_info.uncompressed_size;
	result->crc32 = file_info.crc;
	result->buffer = malloc(file_info.uncompressed_size);

	if (unzGetCurrentFileInfo64(file, 0, result->file_name, size, 0, 0,
		0, 0) != UNZ_OK)
	{
		free_el_file(result);

		return 0;
	}

	if (unzReadCurrentFile(file, result->buffer, result->size) < 0)
	{
		free_el_file(result);

		return 0;
	}

	if (unzCloseCurrentFile(file) != UNZ_OK)
	{
		free_el_file(result);

		return 0;
	}

	crc = CrcCalc(result->buffer, result->size);

	if (result->crc32 != crc)
	{
		LOG_ERROR("crc value is 0x%08X, but should be 0x%08X", crc,
			result->crc32);

		free_el_file(result);

		return 0;
	}

	LOG_DEBUG_VERBOSE("File '%s' [crc:0x%08X] opened.", result->file_name,
		result->crc32);

	return result;
}

static el_file_ptr file_open(const char* file_name, const char* extra_path)
{
	char str[1024];
	el_zip_file_entry_t key;
	el_file_ptr result;
	Sint32 i, count;

	if (file_name == 0)
	{
		return 0;
	}

	if (extra_path != 0)
	{
		if (do_file_exists(file_name, extra_path, sizeof(str), str) == 1)
		{
			return xz_gz_file_open(str);
		}
	}

	if (do_file_exists(file_name, get_path_updates(), sizeof(str), str) == 1)
	{
		return xz_gz_file_open(str);
	}

	init_key(file_name, &key, sizeof(str), str);

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	count = num_zip_files - 1;

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	for (i = count; i >= 0; i--)
	{
		CHECK_AND_LOCK_MUTEX(zip_files[i].mutex);

		if (locate_in_zip(&zip_files[i], &key) == 1)
		{
			result = zip_file_open(zip_files[i].file);

			CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);

			return result;
		}

		CHECK_AND_UNLOCK_MUTEX(zip_files[i].mutex);
	}

	if (do_file_exists(file_name, datadir, sizeof(str), str) == 1)
	{
		return xz_gz_file_open(str);
	}

	return 0;
}

el_file_ptr el_open(const char* file_name)
{
	el_file_ptr result;

	ENTER_DEBUG_MARK("file open");

	result = file_open(file_name, 0);

	LEAVE_DEBUG_MARK("file open");

	return result;
}

el_file_ptr el_open_custom(const char* file_name)
{
	el_file_ptr result;

	ENTER_DEBUG_MARK("file open");

	result = file_open(file_name, get_path_config_base());

	LEAVE_DEBUG_MARK("file open");

	return result;
}

el_file_ptr el_open_anywhere(const char* file_name)
{
	el_file_ptr result;

	ENTER_DEBUG_MARK("file open");

	result = file_open(file_name, get_path_config());

	LEAVE_DEBUG_MARK("file open");

	return result;
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

	free_el_file(file);
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
	int result;

	ENTER_DEBUG_MARK("file exists");

	result = file_exists_path(file_name, 0);

	LEAVE_DEBUG_MARK("file exists");

	return result;
}

int el_custom_file_exists(const char* file_name)
{
	int result;

	ENTER_DEBUG_MARK("file exists");

	result = file_exists_path(file_name, get_path_config_base());

	LEAVE_DEBUG_MARK("file exists");

	return result;
}

int el_file_exists_anywhere(const char* file_name)
{
	int result;

	ENTER_DEBUG_MARK("file exists");

	result = file_exists_path(file_name, get_path_config());

	LEAVE_DEBUG_MARK("file exists");

	return result;
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

