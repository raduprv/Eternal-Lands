#include "ziputil.h"
#include <time.h>

Uint32 convert_string_to_md5_digest(const char* str, MD5_DIGEST digest)
{
	Uint32 i, val;
	char buffer[4];

	if (str == 0)
	{
		return 0;
	}

	if (strlen(str) < 32)
	{
		return 0;
	}

	memset(buffer, 0, sizeof(buffer));

	// convert the md5 to binary
	for (i = 0; i < 16; i++)
	{
		buffer[0] = str[i * 2 + 0];
		buffer[1] = str[i * 2 + 1];

		sscanf(buffer, "%x", &val);

		digest[i] = val;
	}

	return 1;
}

Uint32 convert_comment_string_to_md5_digest(const char* str, MD5_DIGEST digest)
{
	char md5_str[64];

	memset(md5_str, 0, sizeof(md5_str));

	sscanf(str, "MD5:%*[^0-9a-zA-Z]%32s", md5_str);

	return convert_string_to_md5_digest(md5_str, digest);
}

Uint32 convert_md5_digest_to_comment_string(const MD5_DIGEST digest,
	const Uint32 size, char* str)
{
	if (size < 38)
	{
		return 0;
	}

	memset(str, 0, size);
	snprintf(str, size, "MD5: %02x%02x%02x%02x"
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15]);

	return 1;
}

Uint32 add_to_zip(const char* file_name, const Uint32 size,
	const Uint8* buffer, zipFile dest)
{
	char comment[128];
	zip_fileinfo info;
	MD5 md5;
	MD5_DIGEST digest;
	time_t raw_time;
	struct tm* ptm;

	MD5Open(&md5);
	MD5Digest(&md5, buffer, size);
	MD5Close(&md5, digest);

	memset(&info, 0, sizeof(info));

	time(&raw_time);

	ptm = gmtime(&raw_time);

	info.tmz_date.tm_sec = ptm->tm_sec;
	info.tmz_date.tm_min = ptm->tm_min;
	info.tmz_date.tm_hour = ptm->tm_hour;
	info.tmz_date.tm_mday = ptm->tm_mday;
	info.tmz_date.tm_mon = ptm->tm_mon;
	info.tmz_date.tm_year = ptm->tm_year - 80;

	convert_md5_digest_to_comment_string(digest, sizeof(comment), comment);

	zipOpenNewFileInZip2_64(dest, file_name, &info, 0, 0, 0, 0, comment,
		0, 9, 0, 1);
	zipWriteInFileInZip(dest, buffer, size);
	zipCloseFileInZip(dest);

	return 1;
}

Uint32 copy_from_zip(unzFile source, zipFile dest)
{
	char file_name[256];
	char comment[128];
	char md5_str[64];
	zip_fileinfo info;
	unz_file_info64 src_info;
	Uint64 size;
	Uint8* buffer;
	Uint32 crc, valid_md5;
	int method, level;

	unzGetCurrentFileInfo64(source, &src_info, file_name,
		sizeof(file_name), 0, 0, comment, sizeof(comment));

	valid_md5 = 0;

	if (src_info.size_file_comment >= 37)
	{
		memset(md5_str, 0, sizeof(md5_str));

		sscanf(comment, "MD5:%*[^0-9a-zA-Z]%32s", md5_str);

		if (*md5_str != 0)
		{
			valid_md5 = 1;
		}
	}

	if (valid_md5 == 0)
	{
		unzOpenCurrentFile(source);

		size = src_info.uncompressed_size;

		buffer = malloc(size);

		unzReadCurrentFile(source, buffer, size);
		unzCloseCurrentFile(source);

		add_to_zip(file_name, size, buffer, dest);

		free(buffer);

		return 1;
	}

	unzOpenCurrentFile2(source, &method, &level, 1);

	size = src_info.compressed_size;
	crc = src_info.crc;

	buffer = malloc(size);

	unzReadCurrentFile(source, buffer, size);
	unzCloseCurrentFile(source);

	info.tmz_date.tm_sec = src_info.tmu_date.tm_sec;
	info.tmz_date.tm_min = src_info.tmu_date.tm_min;
	info.tmz_date.tm_hour = src_info.tmu_date.tm_hour;
	info.tmz_date.tm_mday = src_info.tmu_date.tm_mday;
	info.tmz_date.tm_mon = src_info.tmu_date.tm_mon;
	info.tmz_date.tm_year = src_info.tmu_date.tm_year;

	zipOpenNewFileInZip2_64(dest, file_name, &info, 0, 0, 0, 0, comment,
		method, level, 1, 1);
	zipWriteInFileInZip(dest, buffer, size);
	zipCloseFileInZipRaw64(dest, size, crc);

	free(buffer);

	return 1;
}

Uint32 check_md5_from_zip(unzFile source, const char* file_name,
	const MD5_DIGEST digest)
{
	unz_file_info64 info;
	char comment[256];
	MD5 md5;
	MD5_DIGEST tmp_digest;
	void* buffer;
	Uint32 size, uncompress;

	if (unzLocateFile(source, file_name, 2) != UNZ_OK)
	{
		return 0;
	}

	if (unzGetCurrentFileInfo64(source, &info, 0, 0, 0, 0,
		comment, sizeof(comment)) != UNZ_OK)
	{
		return 0;
	}

	uncompress = 1;

	if (info.size_file_comment >= 37)
	{
		if (convert_comment_string_to_md5_digest(comment, tmp_digest)
			!= 1)
		{
			uncompress = 0;
		}
	}

	if (uncompress == 1)
	{
		unzOpenCurrentFile(source);

		size = info.uncompressed_size;

		buffer = malloc(size);

		unzReadCurrentFile(source, buffer, size);
		unzCloseCurrentFile(source);

		MD5Open(&md5);
		MD5Digest(&md5, buffer, size);
		MD5Close(&md5, tmp_digest);

		free(buffer);
	}

	if (memcmp(digest, tmp_digest, sizeof(MD5_DIGEST)) != 0)
	{
		return 0;
	}

	return 1;
}

