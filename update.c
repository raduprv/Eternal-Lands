#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifdef	WINDOWS
 #define strdup	_strdup
 #include <direct.h>
#else   //WINDOWS
 #include <sys/types.h>
 #include <sys/stat.h>
#endif	//WINDOWS
 #include <zlib.h>
#include <time.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <assert.h>
#include <errno.h>
#include "update.h"
#include "asc.h"
#include "draw_scene.h"
#include "elc_private.h"
#include "errors.h"
#include "events.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "main.h"
#include "misc.h"
#include "translate.h"
#include "io/elpathwrapper.h"
#include "threads.h"

void create_update_root_window (int width, int height, int time);
static void do_updates(void);
static void http_threaded_get_file(struct update_server_struct *server, char *path, FILE *fp, Uint8 *md5, Uint32 event);
static int do_threaded_update(void *ptr);
static int http_get_file_thread_handler(void *specs);
static int http_get_file(struct update_server_struct *update_server, char *path, FILE *fp);

static int update_attempt_count;   // count how many update attempts have been tried (hopefully diff servers)
static int temp_counter;           // collision prevention during downloads just incase more then one ever starts
static int update_busy;            // state & lockout control to prevent two updates running at the same time
static struct update_server_struct update_server; // the current server we are getting updates from
static unsigned int num_update_servers;
static struct update_server_struct update_servers[32];	// we cant handle more then 32 different servers
static int is_this_files_lst= 0;	// files.lst changes its name if it is a custom update
static char files_lst[256]= {0};
static int allow_restart=1;

// we need a simple queue system so that the MD5 processing is in parallel with downloading
#define	MAX_UPDATE_QUEUE_SIZE	32768
static SDL_mutex *download_mutex;
static int download_queue_size;
static char    *download_queue[MAX_UPDATE_QUEUE_SIZE];
static char    *download_cur_file;
static char    download_temp_file[1024];
static Uint8	*download_MD5s[MAX_UPDATE_QUEUE_SIZE];
static Uint8	*download_cur_md5;
static int doing_custom = 0;

// keep a track of download threads so we can wait for there completion hence freeing resources
#define MAX_THREADS 5
static SDL_Thread *thread_list[MAX_THREADS] = {NULL, NULL, NULL, NULL, NULL};

// initialize the auto update system, start the downloading
void init_update()
{
	FILE    *fp;

	if(update_busy){
		return;
	}
	// initialize variables
	update_busy++;
	update_attempt_count= 0;	// no downloads have been attempted
	temp_counter= 0;			//start with download name with 0
	restart_required= 0;		// no restart needed yet
	allow_restart= 1;			// automated restart allowed

	// create the mutex & init the download que
	if(!download_mutex){
//		file_update_clear_old();	// There is no reason to delete all the previous files. We only remove the ones out of date.
		download_mutex= SDL_CreateMutex();
		download_queue_size= 0;
		memset(download_queue, 0, sizeof(download_queue));
		download_cur_file= NULL;
		download_cur_md5= NULL;
	}

	// load the server list
	num_update_servers= 0;
	update_server.host[0]= '\0';
	fp = open_file_data("mirrors.lst", "r");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"mirrors.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
	} else {
		char    buffer[1024];
		char	*ptr;

		ptr= fgets(buffer, sizeof(buffer), fp);
		while(ptr && !ferror(fp) && num_update_servers < sizeof(update_servers)){
			int len= strlen(buffer);

			// is this line worth handling?
			if(len > 6 && *buffer > ' ' && *buffer != '#'){
				while(isspace(buffer[len-1])){
					buffer[len-1]= '\0';
					len--;
				}
				if(len > 6){
					fill_update_server_struct(&update_servers[num_update_servers], buffer);
				}
			}
			// read the next line
			ptr= fgets(buffer, sizeof(buffer), fp);
		}
		if(fp){
			fclose(fp);
		}
	}
	if(!num_update_servers) {
		LOG_DEBUG("No mirrors, skipping download...");
		// oops, no mirror file, no downloading
		update_servers[0].host[0] = '\0';
		return;
	}

	// start the process
	if(download_mutex){
		strcpy(files_lst, "files.lst");
		is_this_files_lst= 1;
		handle_update_download(NULL);
	}
}

void fill_update_server_struct(struct update_server_struct *update_server, const char *buf)
{
	char *ptr;
	char server_without_port[128];
	int index;
	char port_buf[6];
	int port;

	// check if format is host:port and parse it
	ptr = strchr(buf, ':');
	if (ptr == NULL) {
		LOG_DEBUG("Using port 80 as default...");
		port = 80; // default HTTP port
		safe_strncpy(server_without_port,buf,128);
	} else {
		index = ptr - buf;
		safe_strncpy2(port_buf,ptr+1,6,strlen(buf)-index+1);
		port = atoi(port_buf);
		if ( port == 0) {
			LOG_ERROR("Cannot parse port! Using port 80 as default...");
			port = 80; // default HTTP port
		}
		safe_strncpy2(server_without_port,buf,128,index);
	}

	update_server->port = port;
	safe_strncpy(update_server->host, server_without_port, 128);
	LOG_DEBUG("Adding update server mirror - host: %s | port: %d", update_server->host, update_server->port);

	num_update_servers++;
}

// clean up the auto update system
void clean_update()
{
	int i;

	SDL_DestroyMutex(download_mutex);
	for(i=0; i< num_update_servers; i++)
	{
		if(update_servers[i].host)
			update_servers[i].host[0] = '\0';
	}
	num_update_servers = 0;
}

// compare two update_server_structs
int cmp_update_server(struct update_server_struct *server1, struct update_server_struct *server2)
{
	return strcmp(server1->host, server2->host) && !(server1->port == server2->port);
}

// handle the update file event
static void do_handle_update_download(struct http_get_struct *get)
{
	static int  mkdir_res= -1;  // flag as not tried
	int sts;

	// try to make sure the directory is there
	if(mkdir_res < 0){
		mkdir_res = mkdir_config("tmp");
	}
	if(get != NULL){
		// did we finish properly?
		if(get->status == 0){
			// release the memory
			if(get->fp){
				fclose(get->fp);
			}
			free(get);

			// yes, lets start using the new file
			sts = move_file_to_updates("tmp/temp000.dat", files_lst, doing_custom);

			// trigger processing this file
			if(!sts){
				do_updates();
			} else {
				LOG_ERROR("Unable to finish %s processing (%d)", files_lst, errno);
			}

			// and go back to normal processing
			return;
		}

		// wait for the just completed thread so we free it's resources
		assert(get->thread_index<MAX_THREADS);
		SDL_WaitThread(thread_list[get->thread_index], NULL);
		thread_list[get->thread_index] = NULL;

		//no, we need to free the memory and try again
		if(get->fp){
			fclose(get->fp);
		}
		free(get);
	}

	LOG_DEBUG("Downloading the update file");
	// we need to download the update file if we get here
	if(update_attempt_count++ < 3){
		char	filename[1024];
		FILE    *fp;

		// select a server
		if(num_update_servers > 1){
			int num;

			srand( (unsigned)time( NULL ) );
			num= rand()%num_update_servers;
			if(!cmp_update_server(&update_server, &update_servers[num])){
				// oops, the same server twice in a row, try to avoid
				num= rand()%num_update_servers;
				if(!cmp_update_server(&update_server, &update_servers[num])){
					// oops, the same server twice in a row, try to avoid
					num= rand()%num_update_servers;
					if(!cmp_update_server(&update_server, &update_servers[num])){
						// oops, the same server twice in a row, try to avoid
						num= rand()%num_update_servers;
					}
				}
			}
			safe_strncpy(update_server.host, update_servers[num].host, sizeof(update_server.host));
			update_server.port = update_servers[num].port;
			LOG_DEBUG("downloading from mirror %d of %d %s", num+1, num_update_servers, update_server.host);
		} else {
			safe_strncpy(update_server.host, update_servers[0].host, sizeof(update_server));
			update_server.port = update_servers[0].port;
		}
		++temp_counter;
		fp = open_file_config("tmp/temp000.dat", "wb+");
		if(fp == NULL){
			LOG_ERROR("%s: %s \"tmp/temp000.dat\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		} else {
			if(is_this_files_lst)	//files.lst
			{
				safe_snprintf(filename, sizeof(filename), "http://%s:%d/updates%d%d%d/%s", update_server.host, update_server.port, VER_MAJOR, VER_MINOR, VER_RELEASE, files_lst);
			} else {	//custom_files.lst
				safe_snprintf(filename, sizeof(filename), "http://%s:%d/updates/%s", update_server.host, update_server.port, files_lst);
			}
			LOG_DEBUG("* server %s filename %s", update_server.host, filename);
			http_threaded_get_file(&update_server, filename, fp, NULL, EVENT_UPDATES_DOWNLOADED);
		}
		// and keep running until we get a response
		return;
	}

	// total failure, error and clear the busy flag
	LOG_INFO("Failed to download (%s) 3 times. Giving up.", files_lst);
	update_busy= 0;
}

void handle_update_download(struct http_get_struct *get)
{
	ENTER_DEBUG_MARK("update download");

	do_handle_update_download(get);

	LEAVE_DEBUG_MARK("update download");
}

// start the background checking of updates
static void do_updates(void)
{
	if(update_busy++ > 1){    // dont double process
		return;
	}
	// start the background process
	SDL_CreateThread(&do_threaded_update, "UpdatecheckThread", NULL);
}


static int do_threaded_update(void *ptr)
{
	char    buffer[1024];
	char	*buf;
	int	num_files= 0;
	gzFile fp= NULL;
	char filename[1024];

	init_thread_log("update");

	// open the update file
	safe_snprintf(filename, sizeof(filename), "%s%s", doing_custom ? get_path_custom() : get_path_updates(), files_lst);
	fp = my_gzopen(filename, "rb");
	if(fp == NULL){
		// error, we stop processing now
		update_busy= 0;
		return(0);
	}

	ENTER_DEBUG_MARK("update");

	buf= gzgets(fp, buffer, 1024);
	while(buf && buf != Z_NULL){
		char	filename[1024];
		char    asc_md5[256];
		Uint8	md5[16];

		// parse the line
		filename[0]= '\0';
		asc_md5[0]= '\0';
		sscanf(buffer, "%*[^(](%250[^)])%*[^0-9a-zA-Z]%32s", filename, asc_md5);

		// check for something to process
		if(*filename && *asc_md5 && !strstr(filename, "..") && filename[0] != '/' && filename[0] != '\\' && filename[1] != ':'){
			// check for one special case
			if(!strcasecmp(asc_md5, "none")){
				// this file is to be removed
				remove_file_updates(filename, doing_custom);
			} else {
				int i;

				// convert the md5 to binary
				for(i=0; i<16; i++){
					int val;

					strncpy(buffer, asc_md5+i*2, 2);
					buffer[2]= '\0';
					sscanf(buffer, "%x", &val);
					md5[i]= val;
				}

				if (file_update_check(filename, md5, doing_custom) != 0)
				{
					add_to_download(filename, md5);
					num_files++;
				}
			}
		}

		// read the next line, if any
		buf= gzgets(fp, buffer, 1024);
	}
	// close the file, clear that we are busy
	if(fp){
		gzclose(fp);
	}
	update_busy= 0;

	LEAVE_DEBUG_MARK("update");

	// all done
	return(0);
}

void add_to_download(const char *filename, const Uint8 *md5)
{
	// lock the mutex
	CHECK_AND_LOCK_MUTEX(download_mutex);
	if(download_queue_size < MAX_UPDATE_QUEUE_SIZE){
		// add the file to the list, and increase the count
		download_queue[download_queue_size]= strdup(filename);
		download_MD5s[download_queue_size]= calloc(1, 16);
		memcpy(download_MD5s[download_queue_size], md5, 16);
		download_queue_size++;

		// start a thread if one isn't running
		if(!download_cur_file){
			char	buffer[1024];
			FILE    *fp;

			safe_snprintf(download_temp_file, sizeof(download_temp_file), "tmp/temp%03d.dat", ++temp_counter);
			buffer[sizeof(buffer)-1]= '\0';
			fp = open_file_config(download_temp_file, "wb+");
			if(fp == NULL){
				LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, download_temp_file, strerror(errno));
			} else {
				// build the proper URL to download
				download_cur_file= download_queue[--download_queue_size];
				download_cur_md5= download_MD5s[download_queue_size];
				if(is_this_files_lst){
					safe_snprintf(buffer, sizeof(buffer), "http://%s:%d/updates%d%d%d/%s", update_server.host, update_server.port, VER_MAJOR, VER_MINOR, VER_RELEASE, download_cur_file);
				} else {
					safe_snprintf(buffer, sizeof(buffer), "http://%s:%d/updates/%s", update_server.host, update_server.port, download_cur_file);
				}
				buffer[sizeof(buffer)-1]= '\0';
				LOG_DEBUG("@@ %s %d %s",update_server.host, update_server.port, buffer);
				http_threaded_get_file(&update_server, buffer, fp, download_cur_md5, EVENT_DOWNLOAD_COMPLETE);
			}
		}
	}
	// unlock the mutex
	CHECK_AND_UNLOCK_MUTEX(download_mutex);
}

// finish up on one file that just downloaded
void handle_file_download(struct http_get_struct *get)
{
	int sts;

	if(!get){   // huh? what are you doing?
		return;
	}

	// lock the mutex
	CHECK_AND_LOCK_MUTEX(download_mutex);
	if(get->status == 0){
		// replace the current file (creates all required directories)
		sts = move_file_to_updates(download_temp_file, download_cur_file, doing_custom);
 		LOG_DEBUG("Moved \"%s\" to \"%s\"", download_temp_file, download_cur_file);
		// check for errors
		if(!sts){
			// TODO: make the restart more intelligent
			if(allow_restart){
				restart_required++;
			}
		} else {
			LOG_ERROR("Unable to finish processing of %s (%d)", download_cur_file, errno);
			// the final renamed failed, no restart permitted
			allow_restart= 0;
			restart_required= 0;
		}
	} else {
		// and make sure we can't restart since we had a total failure
		allow_restart= 0;
		restart_required= 0;
	}

	// wait for the just completed thread so we free it's resources
	assert(get->thread_index<MAX_THREADS);
	SDL_WaitThread(thread_list[get->thread_index], NULL);
	thread_list[get->thread_index] = NULL;

	// release the filename
	free(download_cur_file);
	free(download_cur_md5);
	download_cur_file= NULL;

	// unlock mutex
	CHECK_AND_UNLOCK_MUTEX(download_mutex);

	// now, release everything
	free(get);

	// lock the mutex
	CHECK_AND_LOCK_MUTEX(download_mutex);
	if(download_queue_size > 0 && !download_cur_file){
		// start a thread if a file is waiting to download and no download active
		char	buffer[512];
		FILE    *fp;

		safe_snprintf(download_temp_file, sizeof(download_temp_file), "tmp/temp%03d.dat", ++temp_counter);
		fp = open_file_config(download_temp_file, "wb+");
		if(fp == NULL){
			LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, download_temp_file, strerror(errno));
		} else {
			// build the proper URL to download
			download_cur_file= download_queue[--download_queue_size];
			download_cur_md5= download_MD5s[download_queue_size];
			if(is_this_files_lst) {
				safe_snprintf(buffer, sizeof(buffer), "http://%s:%d/updates%d%d%d/%s", update_server.host, update_server.port, VER_MAJOR, VER_MINOR, VER_RELEASE, download_cur_file);
			} else {
				safe_snprintf(buffer, sizeof(buffer), "http://%s:%d/updates/%s", update_server.host, update_server.port, download_cur_file);
			}
			buffer[sizeof(buffer)-1]= '\0';
			http_threaded_get_file(&update_server, buffer, fp, download_cur_md5, EVENT_DOWNLOAD_COMPLETE);
		}
	}

	// check to see if this was the last file && a restart is required
	if(!update_busy && restart_required && allow_restart && download_queue_size <= 0 && !download_cur_file){
		// yes, now trigger a restart
		LOG_INFO("Restart required because of update");
		// Display something on the screen for a little bit before restarting
		create_update_root_window (window_width, window_height, 10);
		show_window (update_root_win);
	}

	// unlock mutex
	CHECK_AND_UNLOCK_MUTEX(download_mutex);
}


// start a download in another thread, return an even when complete
static void http_threaded_get_file(struct update_server_struct *server, char *path, FILE *fp, Uint8 *md5, Uint32 event)
{
	struct http_get_struct  *spec;
	LOG_DEBUG("Downloading %s from %s:%d", path, server->host, server->port);
	// allocate & fill the spec structure
	spec= (struct http_get_struct  *)calloc(1, sizeof(struct http_get_struct));
	safe_strncpy(spec->server.host, server->host, sizeof(spec->server.host));
	spec->server.port = server->port;
	safe_strncpy(spec->path, path, sizeof(spec->path));
	download_cur_md5= spec->md5= md5;
	spec->fp= fp;
	spec->event= event;
	spec->status= -1;   // just so we don't start with 0

	// find a slot to store the thread handle so we can wait for it later
	{
		size_t i;
		spec->thread_index = MAX_THREADS;
		for (i=0; i<MAX_THREADS; i++)
			if (thread_list[i] == NULL)
			{
				spec->thread_index = i;
				break;
			}
		assert(spec->thread_index<MAX_THREADS);
	}

	// NOTE: it is up to the EVENT handler to close the handle & free the spec pointer in data1

	// start the download in the background
	thread_list[spec->thread_index] = SDL_CreateThread(&http_get_file_thread_handler, "DownloadThread", (void *) spec);
}


// the actual background downloader
static int http_get_file_thread_handler(void *specs){
	struct http_get_struct *spec= (struct http_get_struct *) specs;
	SDL_Event event;

	init_thread_log("get_file");

	// load the file
	spec->status= http_get_file(&spec->server, spec->path, spec->fp);
	fclose(spec->fp);
	spec->fp= NULL;

	// check to see if the file is correct
	if (spec->md5 && *spec->md5)
	{
		// get the MD5 for the file
		if (file_temp_check(download_temp_file, spec->md5) != 0)
		{
			LOG_ERROR("Download of %s does not match the MD5 sum in the update file!", spec->path);
			spec->status= 404;
			// remove the temp-file
			file_remove_config(download_temp_file);
			// and make sure we can't restart
			allow_restart= 0;
			restart_required= 0;
		}
	}

	LOG_DEBUG("Finished downloading %s\n", spec->path);

	// signal we are done
	event.type= SDL_USEREVENT;
	event.user.code= spec->event;
	event.user.data1= spec;
	SDL_Delay(500);			// FIXME: This should _not_ be necessary, but appears to be so recently under Windows
	SDL_PushEvent(&event);
	// NOTE: it is up to the EVENT handler to close the handle & free the spec pointer in data1

	return(0);
}


// the http downloader that can be used in the foreground or background
static int http_get_file(struct update_server_struct *update_server, char *path, FILE *fp)
{
	IPaddress http_ip;
	TCPsocket http_sock;
	char message[1024];
	int len;
	int got_header= 0;
	int http_status= 0;

	LOG_DEBUG("update_server - host: %s, port: %d", update_server->host, update_server->port);
	LOG_DEBUG("path: %s", path);

	// resolve the hostname
	if(SDLNet_ResolveHost(&http_ip, update_server->host, update_server->port) < 0){
		LOG_ERROR("Can't resolve the hostname!");
		return(1);  // can't resolve the hostname
	}
	// open the socket
	http_sock= SDLNet_TCP_Open(&http_ip);
	if(!http_sock){
		LOG_ERROR("Failed to open the socket!");
		return(2);  // failed to open the socket
	}

	// send the GET request, try to avoid ISP caching

	safe_snprintf(message, sizeof(message), "GET %s HTTP/1.1\r\nHost: %s\r\nCONNECTION:CLOSE\r\nCACHE-CONTROL:NO-CACHE\r\nREFERER:%s\r\nUSER-AGENT:AUTOUPDATE %s\r\n\r\n", path, update_server->host, "autoupdate", FILE_VERSION);
	//printf("%s",message);
	len= strlen(message);
	if(SDLNet_TCP_Send(http_sock,message,len) < len){
		// close the socket to prevent memory leaks
		SDLNet_TCP_Close(http_sock);
		LOG_ERROR("Error in sending the get request!");
		return(3);  // error in sending the get request
	}

	// get the response & data
	while(len > 0)
	{
		char buf[1024];

		memset(buf, 0, 1024);
		// get a packet
		len= SDLNet_TCP_Recv(http_sock, buf, 1024);
		// have we gotten the full header?
		if(!got_header)
		{
			int i;

			// check for http status
			sscanf(buf, "HTTP/%*s %i ", &http_status);

			// look for the end of the header (a blank line)
			for(i=0; i < len && !got_header; i++)
			{
				if(buf[i] == 0x0D && buf[i+1] == 0x0A &&
					buf[i+2] == 0x0D && buf[i+3] == 0x0A)
				{
					// flag we got the header and write what is left to the file
					got_header= 1;
					if(http_status == 200) {
						fwrite(buf+i+4, 1, len-i-4, fp);
					}
					break;
				}
			}
		}
		else
		{
			if(http_status == 200) {
				fwrite(buf, 1, len, fp);
			} else {
				break;
			}
		}
	}
	SDLNet_TCP_Close(http_sock);

	if(http_status != 200) {
		if (http_status != 0) {
			LOG_ERROR("HTTP Status: %d", http_status);
			return (http_status);
		} else {
			LOG_ERROR("No HTTP status!");
			return (5);
		}
	}

	return(0);  // finished
}



/* Update window code */
int update_root_win = -1;
int update_countdown = 0;

static void draw_update_interface (window_info *win)
{
	char str[200];
	float window_ratio = (float) win->len_y / 480.0f;

	draw_string_zoomed_centered(win->len_x/2, 200 * window_ratio,
		(const unsigned char*)update_complete_str, 0, win->current_scale);

/*	Possibly use this box to display the list of files updated?

	glDisable(GL_TEXTURE_2D);
	glColor3fv(gui_color);
	glBegin(GL_LINES);
	glVertex3i(diff + 30 * window_ratio, 50 * window_ratio, 0);
	glVertex3i(len_x - (diff + 30 * window_ratio) - 20, 50 * window_ratio, 0);
	glVertex3i(diff + 30 * window_ratio, 50 * window_ratio, 0);
	glVertex3i(diff + 30 * window_ratio, 370 * window_ratio, 0);
	glVertex3i(diff + 30 * window_ratio, 370 * window_ratio, 0);
	glVertex3i(len_x - (diff + 30 * window_ratio) - 20, 370 * window_ratio, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
*/

	if (update_countdown != 0)
	{
		safe_snprintf (str, sizeof(str), client_restart_countdown_str, update_countdown);
	}
	else
	{
		safe_strncpy (str, client_restarting_str, sizeof(str));
		exit_now = 1;
	}

	draw_string_zoomed_centered(win->len_x/2, win->len_y - (200 * window_ratio),
		(const unsigned char*)str, 0, win->current_scale);

	glDisable (GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static int display_update_root_handler (window_info *win)
{
	draw_console_pic (cons_text);
	draw_update_interface (win);
	CHECK_GL_ERRORS();

	draw_delay = 20;
	return 1;
}

static int click_update_root_restart ()
{
	exit_now = 1;
	return 1;
}

static int keypress_update_root_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	// first, try to see if we pressed Alt+x, to quit.
	if ( check_quit_or_fullscreen (key_code, key_mod) )
	{
		return 1;
	}
	else if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER)
	{
		exit_now = 1;
		return 1;
	}
	return 0;
}

static int show_update_handler (window_info *win) {
	hide_all_root_windows();
	hide_hud_windows();
	return 1;
}

void create_update_root_window (int width, int height, int time)
{
	if (update_root_win < 0)
	{
		window_info *win = NULL;
		int update_root_restart_id = 0;

		update_root_win = create_window ("Update", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		if (update_root_win < 0 || update_root_win >= windows_list.num_windows)
			return;
		win = &windows_list.window[update_root_win];

		update_root_restart_id = button_add_extended (update_root_win, update_root_restart_id, NULL, 0, 0, 0, 0, 0, win->current_scale, restart_now_label);
		widget_set_color(update_root_win, update_root_restart_id, 1.0f, 1.0f, 1.0f);
		widget_move(update_root_win, update_root_restart_id,
			(width - widget_get_width(update_root_win, update_root_restart_id)) /2,
			height - 2 * widget_get_height(update_root_win, update_root_restart_id));

		set_window_handler (update_root_win, ELW_HANDLER_DISPLAY, &display_update_root_handler);
		set_window_handler (update_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_update_root_handler);
		set_window_handler (update_root_win, ELW_HANDLER_SHOW, &show_update_handler);
		widget_set_OnClick(update_root_win, update_root_restart_id, &click_update_root_restart);
	}

	update_countdown = time;
}
