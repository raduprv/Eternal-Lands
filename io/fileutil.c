#include "fileutil.h"
#include "../xz/Xz.h"
#include "../xz/7zCrc.h"
#include "../xz/XzCrc64.h"

static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc lzmaAlloc = { SzAlloc, SzFree };

void init_crc_tables()
{
	CrcGenerateTable();
	Crc64GenerateTable();
}

static Uint32 xz_file_read_unchecked(FILE* file, const Uint64 file_size,
	void** buffer, Uint64* size)
{
	CXzUnpacker state;
	void* src_buffer;
	Uint64 compressed_size, uncompressed_size, dst_idx, src_idx;
	SizeT dst_size, src_size;
	Uint32 err;
	ECoderStatus status;

	compressed_size = file_size;

	src_buffer = malloc(compressed_size);

	fseek(file, 0, SEEK_SET);
	fread(src_buffer, compressed_size, 1, file);

	err = XzUnpacker_Create(&state, &lzmaAlloc);

	*buffer = 0;
	*size = 0;

	if (err != SZ_OK)
	{
		return err;
	}

	dst_idx = 0;
	src_idx = 0;

	uncompressed_size = 0;

	do
	{
		uncompressed_size += 0x40000;

		*buffer = realloc(*buffer, uncompressed_size);

		dst_size = uncompressed_size - dst_idx;
		src_size = compressed_size - src_idx;

		err = XzUnpacker_Code(&state, (Byte*)*buffer + dst_idx,
			&dst_size, (Byte*)src_buffer + src_idx,
			&src_size, CODER_FINISH_ANY, &status);

		src_idx += src_size;
		dst_idx += dst_size;
	}
	while ((err == SZ_OK) && (status == CODER_STATUS_NOT_FINISHED));

	XzUnpacker_Free(&state);

	if (err == SZ_OK)
	{
		*size = dst_idx;
		*buffer = realloc(*buffer, dst_idx);
	}
	else
	{
		free(*buffer);

		*buffer = 0;
	}

	return err;
}

Uint32 file_read(FILE* file, const Uint64 file_size, void** buffer, Uint64* size)
{
	Byte sig[XZ_SIG_SIZE];

	fseek(file, 0, SEEK_SET);
	fread(sig, XZ_SIG_SIZE, 1, file);

	if (memcmp(sig, XZ_SIG, XZ_SIG_SIZE) == 0)
	{
		return xz_file_read_unchecked(file, file_size, buffer, size);
	}

	*size = file_size;

	if (*size == 0)
	{
		return 1;
	}

	*buffer = malloc(*size);

	fseek(file, 0, SEEK_SET);
	fread(*buffer, *size, 1, file);

	return 0;
}

Uint32 xz_file_read(FILE* file, void** buffer, Uint64* size)
{
	Byte sig[XZ_SIG_SIZE];
	Uint64 file_size;

	fseek(file, 0, SEEK_SET);
	fread(sig, XZ_SIG_SIZE, 1, file);

	if (memcmp(sig, XZ_SIG, XZ_SIG_SIZE) != 0)
	{
		return 1;
	}

	fseek(file, 0, SEEK_END);

	file_size = ftell(file);

	return xz_file_read_unchecked(file, file_size, buffer, size);
}

