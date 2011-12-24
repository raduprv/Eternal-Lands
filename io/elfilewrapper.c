#include "elfilewrapper.h"
#include "unzip.h"
#include "elpathwrapper.h"
#include "fileutil.h"
#include <sys/stat.h>
#include <errno.h>
#include "../elc_private.h"
#include "../errors.h"
#include "../asc.h"
#include "../init.h"
#include "../threads.h"
#include "../hash.h"
#include "../xz/7zCrc.h"

#ifdef FASTER_MAP_LOAD
typedef enum
{
	EL_FILE_HAVE_CRC
} el_file_flags_t;
#endif

struct el_file_t
{
#ifdef FASTER_STARTUP
	unsigned char* buffer;
	unsigned char* current;
	unsigned char *end;
#else
	Sint64 size;
	Sint64 position;
	void* buffer;
#endif
	char* file_name;
	Uint32 crc32;
#ifdef FASTER_MAP_LOAD
	el_file_flags_t flags;
#endif
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
	if (!file)
		return;

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

	ENTER_DEBUG_MARK("unload zips");

	CHECK_AND_LOCK_MUTEX(zip_mutex);

	for (i = 0; i < MAX_NUM_ZIP_FILES; i++)
	{
		clear_zip(&zip_files[i]);

		SDL_DestroyMutex(zip_files[i].mutex);
	}

	num_zip_files = 0;

	CHECK_AND_UNLOCK_MUTEX(zip_mutex);

	SDL_DestroyMutex(zip_mutex);

	LEAVE_DEBUG_MARK("unload zips");
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

		files[i].file_name = calloc(size + 1, 1);

		unzGetCurrentFileInfo64(file, 0, files[i].file_name, size,
			0, 0, 0, 0);

		LOG_DEBUG("Loading file (%d) '%s' from zip file '%s'.", i,
			files[i].file_name, file_name);

		files[i].hash = mem_hash(files[i].file_name, size);

		unzGoToNextFile(file);
	}

	size = strlen(file_name);
	name = calloc(size + 1, 1);
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

	LOG_DEBUG("Loaded zip file '%s' with %d files", file_name, count);
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
	Uint32 found;

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);
	safe_strcat(buffer, ".xz", size);

	found = stat(buffer, &fstat) == 0;

	LOG_DEBUG("Checking file '%s': %s.", buffer, found ? "found" :
		"not found");

	if (found)
	{
		return 1;
	}

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);
	safe_strcat(buffer, ".gz", size);

	found = stat(buffer, &fstat) == 0;

	LOG_DEBUG("Checking file '%s': %s.", buffer, found ? "found" :
		"not found");

	if (found)
	{
		return 1;
	}

	safe_strncpy2(buffer, path, size, strlen(path));
	safe_strcat(buffer, file_name, size);

	found = stat(buffer, &fstat) == 0;

	LOG_DEBUG("Checking file '%s': %s.", buffer, found ? "found" :
		"not found");

	if (found)
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
		LOG_ERROR("Can't open file '%s': %s", file_name, strerror(errno));

		return 0;
	}

	result = calloc(1, sizeof(el_file_t));

	error = xz_file_read(file, (void**)&(result->buffer), &size);
#ifdef FASTER_STARTUP
	result->current = result->buffer;
	result->end = result->buffer + size;
#else
	result->size = size;
#endif

	fclose(file);

	if (error == 0)
	{
		file_name_len = strlen(file_name) + 1;
		result->file_name = malloc(file_name_len);
		safe_strncpy(result->file_name, file_name, file_name_len);

#ifdef FASTER_MAP_LOAD
		LOG_DEBUG("File '%s' [crc:0x%08X] opened.", file_name,
			el_crc32(result));
#else
		result->crc32 = CrcCalc(result->buffer, result->size);

		LOG_DEBUG("File '%s' [crc:0x%08X] opened.", file_name,
			result->crc32);
#endif


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

	file = gzopen(file_name, "rb");
	if (!file)
	{
		LOG_ERROR("Can't open file '%s': %s", file_name, strerror(errno));
		return NULL;
	}

	result = calloc(1, sizeof(el_file_t));
	result->file_name = strdup(file_name);

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
#ifdef FASTER_STARTUP
	result->current = result->buffer;
	result->end = result->buffer + size;
#else
	result->size = size;
#endif
#ifndef FASTER_MAP_LOAD
	result->crc32 = CrcCalc(result->buffer, result->size);
#endif

	gzclose(file);

#ifdef FASTER_MAP_LOAD
	LOG_DEBUG_VERBOSE("File '%s' [crc:0x%08X] opened.", file_name,
		el_crc32(result));
#else
	LOG_DEBUG_VERBOSE("File '%s' [crc:0x%08X] opened.", file_name,
		result->crc32);
#endif

	return result;
}

static el_file_ptr xz_gz_file_open(const char* file_name)
{
	el_file_ptr result;

	result = xz_file_open(file_name);

	if (!result)
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
		return NULL;
	}

	if (unzGetCurrentFileInfo64(file, &file_info, 0, 0, 0, 0, 0, 0) !=
		UNZ_OK)
	{
		return NULL;
	}

	result = calloc(1, sizeof(el_file_t));

	size = file_info.size_filename;
	result->file_name = calloc(size + 1, 1);

#ifndef FASTER_STARTUP
	result->size = file_info.uncompressed_size;
#endif
	result->crc32 = file_info.crc;
#ifdef FASTER_MAP_LOAD
	result->flags |= EL_FILE_HAVE_CRC;
#endif
	result->buffer = malloc(file_info.uncompressed_size);
#ifdef FASTER_STARTUP
	result->current = result->buffer;
	result->end = result->buffer + file_info.uncompressed_size;
#endif

	if (unzGetCurrentFileInfo64(file, 0, result->file_name, size, 0, 0,
		0, 0) != UNZ_OK)
	{
		free_el_file(result);
		return NULL;
	}

	if (unzReadCurrentFile(file, result->buffer, file_info.uncompressed_size) < 0)
	{
		free_el_file(result);
		return NULL;
	}

	if (unzCloseCurrentFile(file) != UNZ_OK)
	{
		free_el_file(result);
		return NULL;
	}

#ifdef FASTER_STARTUP
	crc = CrcCalc(result->buffer, result->end - result->buffer);
#else
	crc = CrcCalc(result->buffer, result->size);
#endif

	if (result->crc32 != crc)
	{
		LOG_ERROR("crc value is 0x%08X, but should be 0x%08X", crc,
			result->crc32);
		free_el_file(result);
		return NULL;
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

	if (!file_name || !*file_name)
		return NULL;

	if (extra_path)
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

	LOG_ERROR("Can't open file '%s'.", file_name);

	return NULL;
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

	if (!file)
		return -1;

#ifdef FASTER_STARTUP
	count = file->end - file->current;
#else
	count = file->size - file->position;
#endif

	if (count > size)
	{
		count = size;
	}

	if (count <= 0)
	{
		return -1;
	}

#ifdef FASTER_STARTUP
	memcpy(buffer, file->current, count);
	file->current += count;
#else
	memcpy(buffer, file->buffer + file->position, count);
	file->position += count;
#endif

	return count;
}

#ifdef FASTER_STARTUP
int el_read_float(el_file_ptr file, float *f)
{
	if (file->current + sizeof(float) > file->end)
		return 0;
#ifdef EL_FORCE_ALIGNED_READ
	{
		float tmp;
		memcpy(&tmp, file->current, sizeof(float));
		*f = SwapLEFloat(tmp);
	}
#else
	*f = SwapLEFloat(*((float*)file->current));
#endif
	file->current += sizeof(float);
	return 1;
}

int el_read_int(el_file_ptr file, int *i)
{
	if (file->current + sizeof(int) > file->end)
		return 0;
#ifdef EL_FORCE_ALIGNED_READ
	{
		int tmp;
		memcpy(&tmp, file->current, sizeof(int));
		*i = SDL_SwapLE32(tmp);
	}
#else
	*i = SDL_SwapLE32(*((int*)file->current));
#endif
	file->current += sizeof(int);
	return 1;
}
#endif // FASTER_STARTUP

#ifdef FASTER_STARTUP
Sint64 el_seek(el_file_ptr file, Sint64 offset, int seek_type)
{
	unsigned char* cur;

	if (!file)
		return -1;

	switch (seek_type)
	{
		case SEEK_SET:
			cur = file->buffer + offset;
			break;
		case SEEK_END:
			cur = file->end - offset;
			break;
		case SEEK_CUR:
			cur = file->current + offset;
			break;
		default:
			return -1;
	}

	if (cur < file->buffer || cur > file->end)
		return -1;

	file->current = cur;
	return file->current - file->buffer;
}
#else  // FASTER_STARTUP
Sint64 el_seek(el_file_ptr file, Sint64 offset, int seek_type)
{
	Sint64 pos;

	if (!file)
		return -1;

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
#endif // FASTER_STARTUP

Sint64 el_tell(el_file_ptr file)
{
#ifdef FASTER_STARTUP
	return file ? file->current - file->buffer : -1;
#else
	return file ? file->position : -1;
#endif
}

Sint64 el_get_size(el_file_ptr file)
{
#ifdef FASTER_STARTUP
	return file ? file->end - file->buffer : -1;
#else
	return file ? file->size : -1;
#endif
}

void el_close(el_file_ptr file)
{
	if (file)
		free_el_file(file);
}

void* el_get_pointer(el_file_ptr file)
{
	return file ? file->buffer : NULL;
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
	return file ? file->file_name : NULL;
}

Uint32 el_crc32(el_file_ptr file)
{
	if (!file)
		return 0;

#ifdef FASTER_MAP_LOAD
	if ((file->flags & EL_FILE_HAVE_CRC) == 0)
	{
#ifdef FASTER_STARTUP
		file->crc32 = CrcCalc(file->buffer, file->end - file->buffer);
#else
		file->crc32 = CrcCalc(file->buffer, file->size);
#endif
		file->flags |= EL_FILE_HAVE_CRC;
	}
#endif // FASTER_MAP_LOAD
	return file->crc32;
}

char *el_fgets(char *str, int size, el_file_ptr file)
{
	const char *sp;
	char *dp;
	int count;

#ifdef FASTER_STARTUP
	if (!file || file->current >= file->end || size <= 0)
#else
	if (!file || file->position >= file->size || size <= 0)
#endif
		return NULL;

	dp = str;
#ifdef FASTER_STARTUP
	sp = (const char*)file->current;
#else
	sp = (const char*)file->buffer + file->position;
#endif
	count = size - 1;
#ifdef FASTER_STARTUP
	if (file->current + count > file->end)
		count = file->end - file->current;
#else
	if (count > file->size - file->position)
		count = file->size - file->position;
#endif

	while (count-- > 0)
	{
		char c = *sp++;
		*dp++ = c;
		if (c == '\n')
			break;
		if (c == '\r')
		{
			if (*sp == '\n' && count > 0)
				*dp++ = *sp++;
			break;
		}
	}
	*dp = '\0';

#ifdef FASTER_STARTUP
	file->current = (unsigned char*)sp;
#else
	file->position = sp - (const char*)file->buffer;
#endif

	return str;
}
