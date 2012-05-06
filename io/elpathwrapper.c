#include "../platform.h"
#include "elpathwrapper.h"
#include "../asc.h"
#include "../elc_private.h"
#include "../errors.h"
#include "../init.h"
#include "../md5.h"
#ifdef MAP_EDITOR
# include "../map_editor/misc.h"
#else
# include "../misc.h"
#endif //MAP_EDITOR
#include "../servers.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif // S_ISDIR

#ifdef WINDOWS
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <ctype.h>
#define MKDIR(file) mkdir(file)
#else // !WINDOWS
#include <unistd.h>
/*
MAX_PATH is generally 256 on Windows. We'll set it to something a lot higher for *nix, but won't expect to need it.
Theoretically safe, unless someone has a HOME that has a really long path. Such as over 900 characters long. Quite unlikely.
*/
#define MAX_PATH 1024
#define MKDIR(file) (mkdir(file, S_IRWXU | S_IRWXG))
#endif //WINDOWS

/*
 * Note: If you wish to use this functionality, be very careful.
 * For the typical linux usage, you'd add the following to make.conf:
 * FEATURES += CONFIGDIR=\"elc\"
 *
 * Note that you should not have a leading or trailing slash.
 * If ELC is able to get your home directory; it, including a trailing slash, is put in front of CONFIGDIR.
 * If not, there is no slash in front, so the failover is to use a dir in datadir (the current working directory).
 *
 * Unless you have a real need for it (such as having to maintain multiple configdirs) it's better to just use
 * the default; this isn't really intended for just using a different name if you din't like "elc"
 */
#ifdef CONFIGDIR
const static char* cfgdirname = CONFIGDIR;
#elif defined(OSX)
const static char* cfgdirname = "Library/Application\ Support/Eternal\ Lands";
#elif defined(WINDOWS)
const static char* cfgdirname = "Eternal Lands";
#else /* *nix */
const static char* cfgdirname = ".elc";
#endif // platform check


static __inline__ int dir_exists (const char* path)
{
	struct stat d_stat;
	return stat (path, &d_stat) == 0 && S_ISDIR (d_stat.st_mode);
}

const char * get_path_config_base(void)
{
	/* Note: In most cases, you shouldn't need to call this function at all anyway;
	 * the other functions below (which are expected to be used) take care of it for you.
	 *
	 * TODO: Clean up the strcpy()/return pairings for when there's an error detected.
	 * It works, but isn't as pretty as it should be.
	 */
	static char locbuffer[MAX_PATH] = {0};
	char pwd[MAX_PATH];
	if (locbuffer[0] != '\0')
	{
		return locbuffer;
	}
	if (getcwd(pwd, MAX_PATH) == NULL)
	{
		pwd[0] = '\0';
	}
#ifdef WINDOWS
	/* Ask Windows NT for HOME/appdata. Note that on Vista, this is deprecated and a wrapper for the new function.
	 * We use the MAX_PATH as defined by Windows. It's technically possible under some circumstances for programs
	 * to work with paths longer than MAX_PATH, but it sounds like it's expected to fail.
	 * As such, it should be fair enough for us to give up after just one try, and use a safe failover.
	 *
	 * FIXME: A handler should be coded for Windows 98 backwards compatability!!
	 */
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, locbuffer)) && 
			(strlen(locbuffer) < MAX_PATH + 5))
	{
		if (pwd[0] != '\0')
		{
			if (chdir(locbuffer) == -1)
			{
				LOG_ERROR("chdir(1) failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
				strcpy(locbuffer, cfgdirname);
				return locbuffer;
			}
			if (mkdir(cfgdirname) == -1 && errno != EEXIST)
			{
				LOG_ERROR("mkdir() failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
				chdir(pwd);
				strcpy(locbuffer, cfgdirname);
				return locbuffer;
			}
			if (chdir(pwd) == -1)
			{
				LOG_ERROR("chdir(2) failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
				strcpy(locbuffer, cfgdirname);
				return locbuffer;
			}
		}
		strcat(locbuffer, "/");
		strcat(locbuffer, cfgdirname);
	}
	else
	{
		LOG_ERROR("getpath() failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
		//No luck. fall through to using the folder in PWD
		strcpy(locbuffer, cfgdirname);
	}
	strcat(locbuffer, "/");
#else /* !WINDOWS */
	safe_snprintf (locbuffer, sizeof(locbuffer), "%s/%s/", getenv("HOME"), cfgdirname);
	if (!mkdir_tree (locbuffer, 0))
	{
		// Failed to create a configuration direction in the home directory, 
		// try in the current directory and hope that succeeds.
		safe_snprintf (locbuffer, sizeof (locbuffer), "%s/", cfgdirname);
		mkdir_tree (locbuffer, 1);
	}
#endif // platform check

	return locbuffer;
}
const char * get_path_config(void)
{
	static char locbuffer[MAX_PATH] = {0};

	// Check if we have selected a server yet, otherwise return the base config dir
#ifndef MAP_EDITOR
	safe_snprintf(locbuffer, sizeof(locbuffer), "%s%s/", get_path_config_base(), get_server_dir());
#else 
	safe_snprintf(locbuffer, sizeof(locbuffer), "%s/", get_path_config_base());
#endif //!MAP_EDITOR
	
	return locbuffer;
}

const char * get_path_updates(void)
{
	static char locbuffer[MAX_PATH] = {0};
	if (locbuffer[0] != '\0')
	{
		return locbuffer;
	}

	safe_snprintf(locbuffer, sizeof(locbuffer), "%supdates/%d_%d_%d/", get_path_config_base(), VER_MAJOR, VER_MINOR, VER_RELEASE);
	
	return locbuffer;
}

const char * get_path_custom(void)
{
	static char locbuffer[MAX_PATH] = {0};
	if (locbuffer[0] != '\0')
	{
		return locbuffer;
	}

	safe_snprintf(locbuffer, sizeof(locbuffer), "%scustom/", get_path_config_base());
	
	return locbuffer;
}

char * check_custom_dir(char * in_file)
{
	char dir[10] = "custom/";
	
	// Check if there is an extra "custom/" on the front of this filename
	if (strncmp(in_file, dir, strlen(dir)) == 0)
	{
		return in_file+strlen(dir);
	}
	return in_file;
}

FILE *open_file_config (const char* filename, const char* mode)
{
	FILE *fp = open_file_config_no_local(filename, mode);
	if (fp != NULL)
		return fp;
	//Not there? okay, try the current directory
	return fopen(filename, mode);
}


FILE * open_file_config_no_local(const char* filename, const char* mode)
{
	char locbuffer[MAX_PATH];
	const char* cfgdir = get_path_config();

	if (strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH)
		return NULL;
	
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	if (strchr (mode, 'w'))
	{
		// Requested file for writing, try to create a directory structure 
		// if necessary
		if (!mkdir_tree (locbuffer, 0))
		{
			// Failed to create the directory tree in the config dir,
			// try the current directory.
			if (!mkdir_tree (filename, 1))
				// That too failed, give up
				return NULL;
		}
	}

	return fopen(locbuffer, mode);
}

FILE * open_file_data_temp(const char* filename, const char* mode)
{
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	
	if (strlen(cfgdir) + strlen(filename) + 2 < MAX_PATH)
	{
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s/%s", cfgdir, filename);
		return fopen(locbuffer, mode);
	}
	return NULL;
}

FILE * open_file_data_updates(char* filename, const char* mode, int custom){
	char locbuffer[MAX_PATH];
	const char * updatepath = custom ? get_path_custom() : get_path_updates();
	
	if (custom) filename = check_custom_dir(filename);
	if (strlen(updatepath) + strlen(filename) + 1 < MAX_PATH)
	{
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s%s", updatepath, filename);	//We could roll the preceding creation of updatepath into this sprintf(), but then we wouldn't have the length check
		return fopen(locbuffer, mode);
	}
	return NULL;
}

FILE * open_file_data_datadir(const char* filename, const char* mode) {
	char locbuffer[MAX_PATH];
	if (strlen(datadir) + strlen(filename) + 2 < MAX_PATH) {
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s/%s", datadir, filename);
		if (!strcmp(mode, "r") || !strcmp(mode, "rb")) {
			return fopen(locbuffer, mode);					// Don't try to create all the directories if we are only trying to read the file!
		} else {
			if (mkdir_tree(locbuffer, 0)) {
				return fopen(locbuffer, mode);
			}
		}
	}
	return NULL;
}


FILE * open_file_data(const char* in_filename, const char* mode){
	char filename[MAX_PATH];
	FILE* fp = NULL;
	
	safe_strncpy(filename, in_filename, sizeof(filename));
	if(strchr(mode, 'w') == NULL){
		//Reading? okay, we check updates first
		if((fp = open_file_data_updates(filename, mode, 0)) != NULL){
			//If there, return it. Otherwise we keep looking.
			return fp;
		}
	}

	if((fp = open_file_data_datadir(filename, mode)) != NULL){
		//If there, return it. Otherwise we keep looking.
		return fp;
	}
	//Writing, and we didn't get the data_dir, likely a permissions problem, so use updates
	if((fp = open_file_data_updates(filename, mode, 0)) != NULL){
		//If there, return it. Otherwise we keep looking.
		return fp;
	}

	return NULL;

}

FILE * open_file_lang(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	if(strlen("languages/") + strlen(lang) + strlen(filename) + 2 < MAX_PATH){
		FILE *fp;
		safe_snprintf(locbuffer, sizeof(locbuffer), "languages/%s/%s", lang, filename);
		if((fp = open_file_data(locbuffer, mode)) != NULL){
			//Found in the set language dir? Goodie!
			return fp;
		}
	}
	if(strlen("languages/") + strlen("en") + strlen(filename) + 2 < MAX_PATH){
		safe_snprintf(locbuffer, sizeof(locbuffer), "languages/en/%s", filename);
		//Okay, we check the 'en' dir as a fallback
		return open_file_data(locbuffer, mode);
	}
	//We got here? Then _someone_ used a huge filename...
	return NULL;
}

int normalize_path (const char* path, char* norm_path, int size, int relative_only)
{
	int idx, n_idx;
	int len = strlen (path);

	if (len >= size)
	{
		// Probably not enough space, don't bother
		return 0;
	}

	idx = n_idx = 0;
	norm_path[n_idx] = '\0';

	if ((path[idx] == '/' || path[idx] == '\\') && !relative_only)
	{
		// absolute path
		norm_path[n_idx++] = path[idx++];
	}

	while (idx < len)
	{
		int sep;

		for (sep = idx; sep < len; sep++)
		{
			if (path[sep] == '/' || path[sep] == '\\')
				// separator found
				break;
		}

		// skip consecutive slashes and "./"
		if (sep - idx > 1 || (sep - idx == 1 && path[idx] != '.'))
		{
			if (sep - idx == 2 && path[idx] == '.' && path[idx+1] == '.')
			{
				// Drats, we need to go one directory up. Strictly speaking
				// this is only valid if the current path actually exists, but 
				// we'll simply remove the last directory (if any) from our 
				// normalized path.
				if (n_idx > 1 || (n_idx == 1 && norm_path[0] != '/'))
				{
					for (--n_idx ; n_idx > 0; n_idx--)
					{
						if (norm_path[n_idx-1] == '/')
							break;
					}
					norm_path[n_idx] = '\0';
				}
				else if (n_idx == 0 && !relative_only)
				{
					// copy the ".." into the normalized path
					norm_path[n_idx++] = '.';
					norm_path[n_idx++] = '.';
					norm_path[n_idx++] = '/';
					norm_path[n_idx] = '\0';
				}
				// else: only relative paths allowed, or parent of root
				// requested. In both cases, ignore the '..'
			}
			else
			{
				// Copy the directory name into the path
				for ( ; idx < sep; idx++, n_idx++)
					norm_path[n_idx] = path[idx];
				norm_path[n_idx++] = '/';
				norm_path[n_idx] = '\0';
			}
		}

		idx = sep+1;
	}

	if (n_idx == 0)
		// path was empy or invalid
		return 0;

	// remove trailing slash if the path was not a directory
	if (path[len-1] != '/' && path[len-1] != '\\')
		norm_path[n_idx-1] = '\0';

	return 1;
}

int mkdir_tree (const char *path, int relative_only)
{
	// First, check directory exists
	char dir[MAX_PATH];
	char *slash;

	// Get rid of possible cruft in path
	if (!normalize_path (path, dir, sizeof (dir), relative_only))
	{
		// failed to parse the path
		return 0;
	}

	if (dir_exists (dir) || file_exists(dir))
	{
		// directory is there, don't bother
		return 1;
	}

	slash = dir;
#ifdef WINDOWS
	if (isalpha(*slash) && slash[1] == ':' && (slash[2] == '\\' || slash[2] == '/'))
	{
		// let's assume the drive exists...
		// FIXME: This could include a check for the drive if someone wanted to add it
		slash += 3;
	}
#else // !WINDOWS
	if (*slash == '/')
		// let's assume the root directory exists...
		slash++;
#endif // WINDOWS
	
	while (slash)
	{
		// watch for hidden ..
		if (*slash == '.' && slash[1] == '.')
		{
			LOG_ERROR ("Cannot create directory (Invalid character): %s, %s", dir, path);
			return 0;
		}

		// find the next slash
		slash = strchr (slash, '/');
		if (slash == NULL)
			break;

		// place a NULL there to break the string up
		*slash = '\0';
		if (!dir_exists (dir))
		{
			if (MKDIR (dir) != 0)
			{
				LOG_ERROR("Cannot create directory (mkdir() failed): %s, %s", dir, path);
				return 0;
			}
		}

		// put the / back in, then advance past it
		*slash++ = '/';
	}

	return 1;
}


int mkdir_config(const char *path){
	//AKA mkdir(configdir+path);
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(path) + 1 > MAX_PATH){
		//Path is too large, so do the same thing as system libraries do
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, path);
	return MKDIR(locbuffer);
}

int file_copy(const char* from_file, char* to_file)
{
	FILE *in, *out;
	char ch;
	int fe = 0;

	in = fopen(from_file,"rb");
	out = fopen(to_file,"wb");
	while(!feof(in))
	{
		ch = getc(in);
		if((fe = ferror(in))) {
			LOG_ERROR("unable to copy %s to %s, read error",from_file, to_file);			
			clearerr(in);			
			break;
		} else {
			if(!feof(in))
				putc(ch, out);
			if((fe = ferror(out))) {
				LOG_ERROR("unable to copy %s to %s, write error",from_file, to_file);
				clearerr(out);
				break;
			}
		}		
	}
	fclose (in);
	fclose(out);

	return fe;
}

int move_file_to_updates(const char* from_file, char* to_file, int custom)
{
	// We never overwrite files in the original client install!!
	char locbuftmp[MAX_PATH];
	char locbufupd[MAX_PATH];
	const char * cfgdir = get_path_config();
	const char * updatesdir = custom ? get_path_custom() : get_path_updates();
	int retval = 0;

	if (custom) to_file = check_custom_dir(to_file);
	if ((strlen(cfgdir) + strlen(from_file) + 1 > MAX_PATH) ||
			(strlen(updatesdir) + strlen(to_file) +2 > MAX_PATH))
	{
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(locbuftmp, cfgdir);
	strcat(locbuftmp, from_file);
	
	strcpy(locbufupd, updatesdir);
	strcat(locbufupd, to_file);

	// Make sure the dir exists
	if (!mkdir_tree(locbufupd, 0))
		return -1;

	// Remove the file if it exists first (important under Windows)
	remove(locbufupd);
	
	retval = rename(locbuftmp, locbufupd);
	if(retval == 18) {
		// special case - moving a file between drives or partitions is not allowed
		retval = file_copy(locbuftmp, locbufupd);
		if (retval)
			return retval;
		else		
			return remove(locbuftmp);
	}
	return retval;
}

void file_update_clear_old(void){	//TODO.
	//Need a recursive rmdir() call that also deletes files as it goes (rmdir() on linux, at the least, will only remove enpty directories)
	//Quite possibly call ftw() and delete non-directories as we go, adding directories to a list to rmdir once finished
}


void remove_file_updates(char* filename, int custom)
{
	char locbuffer[MAX_PATH];
	const char * updatesdir = custom ? get_path_custom() : get_path_updates();

	if (custom) filename = check_custom_dir(filename);
	if (strlen(updatesdir) + strlen(filename) + 1 > MAX_PATH)
	{
		errno = ENAMETOOLONG;
		return;
	}
	strcpy(locbuffer, updatesdir);
	strcat(locbuffer, filename);
	remove(locbuffer);
}

int file_md5_check(FILE * fp, const unsigned char * md5)
{
	MD5 local;
	unsigned char digest[16];
	int res = 0;
	int length;
	unsigned char buffer[1024];
	
	memset (digest, 0, sizeof (digest));
	if (fp != NULL)
	{
		MD5Open(&local);
		while ((length= fread(buffer, 1, sizeof(buffer), fp)) > 0)
		{
			MD5Digest(&local, buffer, length);
		}
		MD5Close(&local, digest);
		fclose(fp);
		if (memcmp(md5, digest, 16) != 0)
		{
			res = 1;	//Old
		}
		else
		{
			res = 0;	//Current
		}
	}
	else
	{
		res = -1;	//Not present
	}
	return res;
}

int file_temp_check(const char * filename, const unsigned char * md5)
{
	FILE* fp = NULL;

	fp = open_file_data_temp(filename, "rb");
	return file_md5_check(fp, md5);
}

int file_update_check(char * filename, const unsigned char * md5, int custom)
{
	FILE* fp = NULL;
	int res_d = 0;

	char *cust_filename = filename;
	int res_u = 0;

	if (custom) cust_filename = check_custom_dir(filename);
	fp = open_file_data_updates(cust_filename, "rb", custom);
	res_u = file_md5_check(fp, md5);

	fp = open_file_data_datadir(filename, "rb");
	res_d = file_md5_check(fp, md5);

	if (res_u == 0)
	{
		return 0;
	}
	else if (res_u == 1)
	{
		remove_file_updates((char *)filename, custom);
	}

	if (res_d == 0)
	{
		return 0;
	}
	return 1;
}

void file_check_datadir(void)
{
	struct stat fstat;
#ifdef WINDOWS
	if (datadir[strlen(datadir)-1] == '/' || datadir[strlen(datadir)-1] == '\\')		// stat() fails with a trailing slash under Windows. :-S
		datadir[strlen(datadir)-1] = '\0';
#endif // WINDOWS
	if (stat(datadir, &fstat) != 0)
	{
		char pwd[MAX_PATH];
		if (getcwd(pwd, MAX_PATH) == NULL)
			pwd[0] = '\0';
		LOG_WARNING("Didn't find your data_dir, using the current directory instead. Please correct this in your el.ini . Given data_dir was: \"%s\". Using \"%s\".\n", datadir, pwd);
		strcpy(datadir, "./");
	}
#ifdef WINDOWS
	else
	{
		datadir[strlen(datadir)] = '/';			// Replace the trailing slash
	}
#endif // WINDOWS
}

off_t get_file_size_config( const char *filename )
{
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		return -1;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	return get_file_size(locbuffer);
}

int file_exists_config( const char *filename )
{
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		return -1;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	return file_exists(locbuffer);
}

int file_rename_config( const char *old_filename, const char *new_filename )
{
	char locbuffer_old[MAX_PATH];
	char locbuffer_new[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(old_filename) + 1 > MAX_PATH){
		return -1;
	}
	if(strlen(cfgdir) + strlen(new_filename) + 1 > MAX_PATH){
		return -1;
	}
	strcpy(locbuffer_old, cfgdir);
	strcat(locbuffer_old, old_filename);
	strcpy(locbuffer_new, cfgdir);
	strcat(locbuffer_new, new_filename);
	return rename(locbuffer_old, locbuffer_new);
}

int file_remove_config( const char *filename )
{
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		return -1;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	return remove(locbuffer);
}


int copy_file(const char *source, const char *dest)
{
	FILE *f_in,*f_out;
	char buffer[4096];
	size_t bytes_read, bytes_written = 0;

	if (file_exists(dest))
		return -1; /* File exists */

	f_in = fopen(source, "r");
	if (NULL==f_in) {
		return -2; /* Source file does not exist */
	}

	f_out = fopen(dest, "w");
	if (NULL==f_out) {
		fclose(f_in);
		return -3; /* Destination file cannot be created */
	}

	do {
		bytes_read = fread((void*)buffer, 1, 4096, f_in);

		if (bytes_read > 0) {
			bytes_written = fwrite((void*)buffer, bytes_read, 1, f_out);
		}

		if (
			(bytes_read == 0 && ferror(f_in)) || /* Error in read */
			(bytes_read > 0 && bytes_written == 0) /* Error in write */
		   )
		{
			/* Aarg, read or write error. */
			fclose(f_in);
			fclose(f_out);
			remove(dest);
			return -4; /* IO error */
		}
	} while (bytes_read > 0);

	fclose(f_in); /* Ignore errors here */

	if ( fclose(f_out)!=0 ) {
		remove(dest);
		return -4; /* IO error */
	}

	return 0;
}


int check_configdir(void)
{
	return dir_exists(get_path_config());
}

