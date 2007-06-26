#ifdef NEW_FILE_IO
#include "elpathwrapper.h"
#include "../global.h"
#include "../elc_private.h"
#include "../md5.h"
#include "../errors.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#ifdef WINDOWS
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
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
 * FEATURES += CONFIGDIR=\"elc/\"
 *
 * Note that you should not have a leading slash, but you should have a trailing one.
 * If ELC is able to get your home directory; it, including a trailing slash, is put in front of CONFIGDIR.
 * If not, there is no slash in front, so the failover is to use a dir in datadir (the current working directory).
 *
 * Unless you have a real need for it (such as having to maintain multiple configdirs) it's better to just use
 * the default; this isn't really intended for just using a different name if you din't like "elc"
 */
#ifdef CONFIGDIR
const static char* cfgdirname = CONFIGDIR;
#elif defined(OSX)
const static char* cfgdirname = "Library/Application\ Support/Eternal\ Lands/";
#elif defined(WINDOWS)
const static char* cfgdirname = "elc/";
#else /* *nix */
const static char* cfgdirname = ".elc/";
#endif // platform check


char * get_path_config(void){
	/*Note: Unless you get a NULL (which you won't unless there's a far more serious problems than finding configdir),
	 * it's your job to free() the pointer once you're finished with it. In most cases, you shouldn't need to call
	 * this function at all anyway; the other functions below (which are expected to be used) take care of it for you.
	 *
	 * TODO: Clean up the strcpy()/return pairings for when there's an error detected.
	 * It works, but isn't as pretty as it should be.
	 */
	char* locbuffer = (char*)malloc(MAX_PATH);
	char pwd[MAX_PATH];
	if(locbuffer == NULL){
		//This is NOT good. What, are we out of memory?
		return NULL;
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
#else /* !WINDOWS */
	strcpy(locbuffer, getenv("HOME"));	//Warning! Enormous home paths could overflow our variable (so don't do that).
	if(pwd[0] != '\0'){
		if(chdir(locbuffer) == -1){strcpy(locbuffer, cfgdirname); return locbuffer;}
		if(mkdir_tree(cfgdirname) == -1 && errno != EEXIST){chdir(pwd); strcpy(locbuffer, cfgdirname); return locbuffer;}
		if(chdir(pwd) == -1){strcpy(locbuffer, cfgdirname); return locbuffer;}
	}
	strcat(locbuffer, "/");
	strcat(locbuffer, cfgdirname);
#endif // platform check

	return locbuffer;
}


FILE * open_file_config(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	char * cfgdir = get_path_config();
	FILE * fp;
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		free(cfgdir);
		return NULL;
	}
	strcpy(locbuffer, cfgdir);
	strcat(locbuffer, filename);
	fp = fopen(locbuffer, mode);
	free(cfgdir);
	if(fp != NULL){
		return fp;
	}
	//Not there? okay, try the current directory
	return fopen(filename, mode);
}

#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
FILE * open_file_data_updates(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	char updatepath[MAX_PATH];
	char * cfgdir = get_path_config();
	snprintf(updatepath, sizeof(updatepath), "updates_%d_%d_%d_%d", VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD);
	updatepath[sizeof(updatepath)-1] = '\0';
	if(strlen(cfgdir) + strlen(updatepath) + strlen(filename) + 1 < MAX_PATH){
		strcpy(locbuffer, cfgdir);
		strcat(locbuffer, updatepath);
		strcat(locbuffer, filename);
		free(cfgdir);
		return fopen(locbuffer, mode);
	}
	free(cfgdir);
	return NULL;
}
#endif // UPDATE


FILE * open_file_data_datadir(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	if(strlen(datadir) + strlen(filename) + 2 < MAX_PATH){
		strcpy(locbuffer, datadir);
		strcat(locbuffer, "/");	//Shouldn't be needed, but won't hurt
		strcat(locbuffer, filename);
		if(mkdir_tree(filename)){
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
		fp = open_file_data_updates(filename, mode);
		if(fp != NULL){
			return fp;
		}
	}
#endif //UPDATE

	fp = open_file_data_datadir(filename, mode);
	if(fp != NULL){
		return fp;
	}
#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
	//Writing, and we didn't get the data_dir, likely a permissions problem, so use updates
	fp = open_file_data_updates(filename, mode);
	if(fp != NULL){
		return fp;
	}
#endif //UPDATE

	return NULL;

}

FILE * open_file_lang(const char* filename, const char* mode){
	char locbuffer[MAX_PATH];
	if(strlen("languages/") + strlen(lang) + strlen(filename) + 2 < MAX_PATH){
		FILE *fp;
		strcpy(locbuffer, "languages/");
		strcat(locbuffer, lang);
		strcat(locbuffer, "/");
		strcat(locbuffer, filename);
		fp = open_file_data(locbuffer, mode);
		if(fp != NULL){
			return fp;
		}
	}
	if(strlen("languages/") + strlen("en") + strlen(filename) + 2 < MAX_PATH){
		strcpy(locbuffer, "languages/en/");
		strcat(locbuffer, filename);
		return open_file_data(locbuffer, mode);
	}
	return NULL;
}

int mkdir_tree(const char *file)
{
	// First, check directory exists
	char dir[1024];
	char *slash;
	struct stat stats;

	if(strlen(file) >= sizeof(dir)-2){
		return 0;
	}
	strcpy(dir, file);
	slash= dir;

	// Skip over leading periods. this also prevents ../ accesses on purpose
	while(*slash == '.'){
		slash++;
	}

	// Skip over leading slashes.
	while(*slash == '/'){
		slash++;
	}

	while(slash){
		// watch for hidden ..
		if(*slash == '.' && slash[1] == '.'){
			log_error("cannot create directory %s", dir);
			return 0;
		}
		// find the next slash
		slash= strchr(slash, '/');
		if(slash == NULL){
			break;
		}

		// place a NULL there to break the string up
		*slash= '\0';
		if(!(stat(dir, &stats) == 0 && S_ISDIR(stats.st_mode) ) )
		if(MKDIR(dir)!= 0) {
			log_error("cannot create directory %s", dir);
			return 0;
		}
		// put the / back in, then advance past it
		*slash++ = '/';

		// Avoid unnecessary calls to mkdir when given
		// file names containing multiple adjacent slashes.
		while (*slash == '/'){
			slash++;
		}
	}
	return 1;
}


int mkdir_config(const char *path){
	//AKA mkdir(configdir+path);
	char locbuffer[MAX_PATH];
	char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(path) + 1 > MAX_PATH){
		free(cfgdir);
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
	char * cfgdir = get_path_config();
	if((strlen(cfgdir) + strlen(from_file) + 1 > MAX_PATH) ||
			(strlen(datadir) + strlen(to_file) +2 > MAX_PATH)){
		free(cfgdir);
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(locbufcfg, cfgdir);
	strcat(locbufcfg, from_file);

	strcpy(locbufdat, datadir);
	strcat(locbufdat, "/");
	strcat(locbufdat, to_file);

	return rename(locbufcfg, locbufdat);
}


void file_update_clear_old(void){	//TODO.
	//Need a recursive rmdir() call that also deletes files as it goes (rmdir() on linux, at the least, will only remove enpty directories)
	//Quite possibly call ftw() and delete non-directories as we go, adding directories to a list to rmdir once finished
}


#if defined(AUTO_UPDATE) || defined(CUSTOM_UPDATE)
void remove_file_data_updates(const char* filename){
	char locbuffer[MAX_PATH];
	char * cfgdir = get_path_config();
	if(strlen(cfgdir) + strlen(filename) + 1 > MAX_PATH){
		free(cfgdir);
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
		strcpy(locbuffer, datadir);
		strcat(locbuffer, "/");	//Shouldn't be needed, but won't hurt
		strcat(locbuffer, filename);
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
	if(stat(datadir, &fstat) != 0){
		log_error("Warning: Didn't find your data_dir, using the current directory instead. Please correct this in your el.ini . Given data_dir was: \"%s\"\n", datadir);
		strcpy(datadir, "./");
	}
}

#endif //NEW_FILE_IO
