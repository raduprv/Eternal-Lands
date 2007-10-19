#ifdef NEW_FILE_IO
#include "../platform.h"
#include "elpathwrapper.h"
#include "../asc.h"
#include "../elc_private.h"
#include "../errors.h"
#include "../init.h"
#include "../md5.h"
#include "../misc.h"

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
const static char* cfgdirname = "elc";
#else /* *nix */
const static char* cfgdirname = ".elc";
#endif // platform check


static __inline__ int dir_exists (const char* path)
{
	struct stat d_stat;
	return stat (path, &d_stat) == 0 && S_ISDIR (d_stat.st_mode);
}

const char * get_path_config(void){
	/* Note: In most cases, you shouldn't need to call this function at all anyway;
	 * the other functions below (which are expected to be used) take care of it for you.
	 *
	 * TODO: Clean up the strcpy()/return pairings for when there's an error detected.
	 * It works, but isn't as pretty as it should be.
	 */
	static char locbuffer[MAX_PATH] = {0};
	char pwd[MAX_PATH];
	if(locbuffer[0] != '\0'){
		return locbuffer;
	}
	if(getcwd(pwd, MAX_PATH) == NULL){
		pwd[0] = '\0';
	}
#ifdef WINDOWS
	/* Ask Windows for HOME/appdata. Note that on Vista, this is deprecated and a wrapper for the new function.
	 * We use the MAX_PATH as defined by Windows. It's technically possible under some circumstances for programs
	 * to work with paths longer than MAX_PATH, but it sounds like it's expected to fail.
	 * As such, it should be fair enough for us to give up after just one try, and use a safe failover.
	 */
	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, locbuffer)) && 
			(strlen(locbuffer) < MAX_PATH + 5)){
			if(pwd[0] != '\0'){
				if(chdir(locbuffer) == -1){
					log_error("chdir(1) failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
					strcpy(locbuffer, cfgdirname);
					return locbuffer;
				}
				if(mkdir(cfgdirname) == -1 && errno != EEXIST){
					log_error("mkdir() failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
					chdir(pwd);
					strcpy(locbuffer, cfgdirname);
					return locbuffer;
				}
				if(chdir(pwd) == -1){
					log_error("chdir(2) failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
					strcpy(locbuffer, cfgdirname);
					return locbuffer;
				}
			}
		strcat(locbuffer, "/");
		strcat(locbuffer, cfgdirname);
	} else {
		log_error("getpath() failed.\tcfgdirname: \"%s\"\tlocbuffer: \"%s\"\tpwd: \"%s\"\n", cfgdirname, locbuffer, pwd);
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


FILE *open_file_config (const char* filename, const char* mode)
{
	char locbuffer[MAX_PATH];
	const char* cfgdir = get_path_config();
	FILE * fp;

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

	if((fp = fopen(locbuffer, mode)) != NULL){
		return fp;
	}

	//Not there? okay, try the current directory
	return fopen(filename, mode);
}

#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
FILE * open_file_data_updates(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	char updatepath[MAX_PATH];
	const char * cfgdir = get_path_config();
	safe_snprintf(updatepath, sizeof(updatepath), "updates_%d_%d_%d_%d/", VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
	updatepath[sizeof(updatepath)-1] = '\0';
	if(strlen(cfgdir) + strlen(updatepath) + strlen(filename) + 1 < MAX_PATH){
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s%s%s", cfgdir, updatepath, filename);	//We could roll the preceding creation of updatepath into this sprintf(), but then we wouldn't have the length check
		return fopen(locbuffer, mode);
	}
	return NULL;
}
#endif // UPDATE


FILE * open_file_data_datadir(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	if(strlen(datadir) + strlen(filename) + 2 < MAX_PATH){
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s/%s", datadir, filename);
		if (mkdir_tree (filename, 1)){
			return fopen(locbuffer, mode);
		}
	}
	return NULL;
}


FILE * open_file_data(const char* filename, const char* mode){
	FILE* fp = NULL;
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	if(strchr(mode, 'w') == NULL){
		//Reading? okay, we check updates first
		if((fp = open_file_data_updates(filename, mode)) != NULL){
			//If there, return it. Otherwise we keep looking.
			return fp;
		}
	}
#endif //UPDATE

	if((fp = open_file_data_datadir(filename, mode)) != NULL){
		//If there, return it. Otherwise we keep looking.
		return fp;
	}
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	//Writing, and we didn't get the data_dir, likely a permissions problem, so use updates
	if((fp = open_file_data_updates(filename, mode)) != NULL){
		//If there, return it. Otherwise we keep looking.
		return fp;
	}
#endif //UPDATE

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
		// failed to parse the path
		return 0;

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
			log_error ("Cannot create directory (Invalid character): %s, %s", dir, path);
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
				log_error("Cannot create directory (mkdir() failed): %s, %s", dir, path);
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


int move_file_to_data(const char* from_file, const char* to_file){
	char locbufcfg[MAX_PATH];
	char locbufdat[MAX_PATH];
	const char * cfgdir = get_path_config();
	if((strlen(cfgdir) + strlen(from_file) + 1 > MAX_PATH) ||
			(strlen(datadir) + strlen(to_file) +2 > MAX_PATH)){
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(locbufcfg, cfgdir);
	strcat(locbufcfg, from_file);

	safe_snprintf(locbufdat, sizeof(locbufdat), "%s/%s", datadir, to_file);

	return rename(locbufcfg, locbufdat);
}


void file_update_clear_old(void){	//TODO.
	//Need a recursive rmdir() call that also deletes files as it goes (rmdir() on linux, at the least, will only remove enpty directories)
	//Quite possibly call ftw() and delete non-directories as we go, adding directories to a list to rmdir once finished
}


#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
void remove_file_data_updates(const char* filename){
	char locbuffer[MAX_PATH];
	const char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		errno = ENAMETOOLONG;
		return;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	remove(locbuffer);
}
#endif //UPDATES

void remove_file_data_datadir(const char* filename){
	char locbuffer[MAX_PATH];
	if(strlen(datadir) + strlen(filename) + 2 < MAX_PATH){
		safe_snprintf(locbuffer, sizeof(locbuffer), "%s/%s", datadir, filename);
		remove(locbuffer);
	}
}

void remove_file_data(const char * filename){
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	remove_file_data_updates(filename);
#endif //UPDATES

	remove_file_data_datadir(filename);
}


int file_update_check(const char * filename, const unsigned char * md5){
	FILE* fp = NULL;
	MD5 local;
	unsigned char digest_d[16];
	int res_d = 0;
	int length;
	unsigned char buffer[1024];
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	unsigned char digest_u[16];
	int res_u = 0;

	memset (digest_u, 0, sizeof (digest_u));
	fp = open_file_data_updates(filename, "r");
	if(fp != NULL){
		MD5Open(&local);
		while ((length= fread(buffer, 1, sizeof(buffer), fp)) > 0){
			MD5Digest(&local, buffer, length);
		}
		MD5Close(&local, digest_u);
		fclose(fp);
		if(memcmp(md5, digest_u, 16) != 0){
			res_u = 1;	//Old
		} else {
			res_u = 0;	//Current
		}
	} else {
		res_u = -1;	//Not present
	}
#endif //UPDATE

	memset (digest_d, 0, sizeof (digest_d));
	fp = open_file_data_datadir(filename, "r");
	if(fp != NULL){
		MD5Open(&local);
		while ((length= fread(buffer, 1, sizeof(buffer), fp)) > 0){
			MD5Digest(&local, buffer, length);
		}
		MD5Close(&local, digest_d);
		fclose(fp);									
		if(memcmp(md5, digest_d, 16) != 0){
			res_d = 1;	//Old
		} else {
			res_d = 0;	//Current
		}
	} else {
		res_d = -1;	//Not present
	}
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	if(res_u == 0){
		//Move
		return 0;
	} else if(res_u == 1){
		remove_file_data_updates(filename);
	}
#endif //UPDATE

	if(res_d == 0){
		return 0;
	}
	return 1;
}

void file_check_datadir(void){
	struct stat fstat;
#ifdef WINDOWS
	if (datadir[strlen(datadir)-1] == '/')		// stat() fails with a trailing slash under Windows. :-S
		datadir[strlen(datadir)-1] = '\0';
#endif // WINDOWS
	if (stat(datadir, &fstat) != 0){
		log_error("Warning: Didn't find your data_dir, using the current directory instead. Please correct this in your el.ini . Given data_dir was: \"%s\"\n", datadir);
		strcpy(datadir, "./");
	}
#ifdef WINDOWS
	else
	{
		datadir[strlen(datadir)] = '/';			// Replace the trailing slash
	}
#endif // WINDOWS
}

#endif //NEW_FILE_IO
