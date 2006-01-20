#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include	"global.h"
#endif
#include	"update.h"
#include    "asc.h"
#include    <stdio.h>
#include    <ctype.h>


int update_attempt_count;   // count how many update attempts have been tried (hopefully diff servers)
int temp_counter;           // collision prevention during downloads just incase more then one ever starts
int update_busy;            // state & lockout control to prevent two updates running at the saem rime
int restart_required;       // critical files have changed, restart is required
char    update_server[128]; // the current server we are getting updates from
int num_update_servers;
char    *update_servers[32];    // we cant handle more then 32 different servers

// we need a simple queue system so that the MD5 processing is in parallel with downloading
SDL_mutex *download_mutex;
int download_queue_size;
char    *download_queue[256];
char    *download_cur_file;
char    download_temp_file[256];


// initialize the auto update system, start the downloading
void    init_update()
{
	FILE    *fp;
	
	if(update_busy){
		return;
	}
	// initialize variables
	update_busy++;
	update_attempt_count= 0;    // no downloads have been attempted
	temp_counter= 0;    	//start with download name with 0
	restart_required= 0;    // no restart needed yet
	// create the mutex & init the download que
	if(!download_mutex){
       	download_mutex= SDL_CreateMutex();
		download_queue_size= 0;
		memset(download_queue, 0, sizeof(download_queue));
		download_cur_file= NULL;
	}
	// load the server list
	num_update_servers= 0;
	update_server[0]= '\0';
	fp= my_fopen("mirrors.lst", "r");
	if(fp){
		char    buffer[1024];
		char	*ptr;
		
		ptr= fgets(buffer, 1024, fp);
		while(ptr && !ferror(fp) && num_update_servers < 32){
			int len= strlen(buffer);
			
			// is this line worth handling?
			if(len > 6 && *buffer > ' ' && *buffer != '#'){
				while(isspace(buffer[len-1])){
					buffer[len-1]= '\0';
					len--;
				}
				if(len > 6){
					update_servers[num_update_servers++]= strdup(buffer);
				}
			}
			// read the next line
			ptr= fgets(buffer, 1024, fp);
		}
		fclose(fp);
	} else {
		// oops, no mirror file, no downloading
		update_servers[0]= "";
		return;
	}
	
	// start the process
	if(download_mutex){
		handle_update_download(NULL);
	}
}


// handle the update file event
void    handle_update_download(struct http_get_struct *get)
{
	if(get != NULL){
		// did we finish properly?
		if(get->status == 0){\
			// release the memory
			fclose(get->fp);
			free(get);

			// yes, lets start using the new file
			remove("files.lst");
			rename("temp000.dat", "files.lst");

			// trigger processing this file
			do_updates();
			
			// and go back to normal processing
			return;
		}
		//no, we need to free the memory and try again
		fclose(get->fp);
		free(get);
	}

	// we need to download the update file if we get here
	if(++update_attempt_count < 3){
		char	filename[256];
		FILE    *fp;
		
		// select a server
		if(num_update_servers > 1){
			int num;
			
           	srand( (unsigned)time( NULL ) );
			num= rand()%num_update_servers;
			if(!strcmp(update_server, update_servers[num])){
				// oops, the same server twice in a row, try to avoid
				num= rand()%num_update_servers;
				if(!strcmp(update_server, update_servers[num])){
					// oops, the same server twice in a row, try to avoid
					num= rand()%num_update_servers;
					if(!strcmp(update_server, update_servers[num])){
						// oops, the same server twice in a row, try to avoid
						num= rand()%num_update_servers;
					}
				}
			}
			strncpy(update_server, update_servers[num], 128);
			update_server[127]= '\0';
log_error("downloading from mirror %d of %d %s", num, num_update_servers, update_server);
		} else {
			strcpy(update_server, update_servers[0]);
		}
		sprintf(filename, "temp000.dat");
		++temp_counter;
		fp= my_fopen(filename, "wb+");
		if(fp){
			sprintf(filename, "http://%s/updates/files.lst", update_server);
			http_threaded_get_file(update_server, filename, fp, EVENT_UPDATES_DOWNLOADED);
		}
		// and keep running until we get a response
		return;
	}
	
	// total failure, clear the busy flag
	update_busy= 0;
}


// start the background checking of updates
void    do_updates()
{
	if(update_busy++ > 1){    // dont double process
		return;
	}
	// start the background process
	SDL_CreateThread(&do_threaded_update, NULL);
}


int    do_threaded_update(void *ptr)
{
	char    buffer[1024];
	FILE    *fp;
	char	*buf;
	
	// open the update file
	fp= my_fopen("files.lst", "r");
	if(fp == NULL){
		// error, we stop processing now
		update_busy= 0;
		return(0);
	}

	buf= fgets(buffer, 1024, fp);
	while(buf && !ferror(fp)){
		char	filename[256];
		char    asc_md5[256];
		Uint8	md5[16];
		Uint8	digest[16];
		
		// parse the line
		filename[0]= '\0';
		asc_md5[0]= '\0';
	    sscanf(buffer, "%*[^(](%250[^)])%*[^0-9a-zA-Z]%32s", filename, asc_md5);

	    // check for something to process
		if(*filename && *asc_md5 && !strstr(filename, "..") && filename[0] != '/' && filename[0] != '\\' && filename[1] != ':'){
			// check for one special case
			if(!strcasecmp(asc_md5, "none")){
				// this file is to be removed
				remove(filename);
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

				// get the MD5 for the file
				get_file_digest(filename, digest);
  				// if MD5's don't match, start a download
  				if(memcmp(md5, digest, 16) != 0){
					add_to_download(filename);
				}
			}
		}
			
		// read the next line, if any
		buf= fgets(buffer, 1024, fp);
	}
	// close the file, clear that we are busy
	fclose(fp);
	update_busy= 0;

	// all done
	return(0);
}


void   add_to_download(const char *filename)
{
	// lock the mutex
	SDL_mutexP(download_mutex);
	if(download_queue_size <256){
		// add the file to the list, and increase the count
		download_queue[download_queue_size++]= strdup(filename);
		// start a thread if one isn't running
		if(!download_cur_file){
			char	buffer[256];
			FILE    *fp;

			snprintf(download_temp_file, sizeof(buffer), "temp%03d.dat", ++temp_counter);
			buffer[sizeof(buffer)-1]= '\0';
			fp= my_fopen(download_temp_file, "wb+");
			if(fp){
				// build the prope URL to download
				download_cur_file= download_queue[--download_queue_size];
				snprintf(buffer, sizeof(buffer), "http://%s/updates/%s", update_server, download_cur_file);
				buffer[sizeof(buffer)-1]= '\0';
				http_threaded_get_file(update_server, buffer, fp, EVENT_DOWNLOAD_COMPLETE);
			}
		}
	}
	// unlock the mutex
	SDL_mutexV(download_mutex);
}


// finish up on one file that just downloaded
void    handle_file_download(struct http_get_struct *get)
{
	if(!get){   // huh? what are you doing?
		return;
	}
	
	fclose(get->fp);
	if(get->status == 0){
		// the download was successful
		// TODO: verify the MD5? if so, we need to save it

		// lock the mutex
		SDL_mutexP(download_mutex);

		// replace the current file
		remove(download_cur_file);
		rename(download_temp_file, download_cur_file);

		// release the filename
		free(download_cur_file);
		download_cur_file= NULL;

		// unlock mutex
		SDL_mutexV(download_mutex);

		// TODO: make the restart intelligent
		restart_required++;
	} else {
		// lock the mutex
		SDL_mutexP(download_mutex);

		// release the filename
		free(download_cur_file);
		download_cur_file= NULL;

		// unlock mutex
		SDL_mutexV(download_mutex);
	}
	
	// now, release everything
	free(get);
	
	// lock the mutex
	SDL_mutexP(download_mutex);
	if(download_queue_size > 0 && !download_cur_file){
		// start a thread if a file is waiting to download and no download active
		char	buffer[512];
		FILE    *fp;

		sprintf(download_temp_file, "temp%03d.dat", ++temp_counter);
		fp= my_fopen(download_temp_file, "wb+");
		if(fp){
			// build the prope URL to download
			download_cur_file= download_queue[--download_queue_size];
			snprintf(buffer, sizeof(buffer), "http://%s/updates/%s", update_server, download_cur_file);
			buffer[sizeof(buffer)-1]= '\0';
			http_threaded_get_file(update_server, buffer, fp, EVENT_UPDATES_DOWNLOADED);
		}
	}

	// check to see if this was the last file && a restart is required
	if(!update_busy && restart_required && download_queue_size <= 0){
		// yes, now trigger a restart
		log_error("Restart required because of update");
		exit_now=1;
	}

	// unlock mutex
	SDL_mutexV(download_mutex);
}


// start a download in another thread, return an even when complete
void http_threaded_get_file(char *server, char *path, FILE *fp, Uint32 event)
{
	struct http_get_struct  *spec;

	// allocate & fill the spec structure
	spec= (struct http_get_struct  *)calloc(1, sizeof(struct http_get_struct));
	strcpy(spec->server, server);
	strcpy(spec->path, path);
	spec->fp= fp;
	spec->event= event;
	spec->status= -1;   // just so we don't start with 0
	
	// NOTE: it is up to the EVENT handler to close the handle & free the spec pointer in data1

	// start the download in the background
	SDL_CreateThread(&http_get_file_thread_handler, (void *) spec);
}


// the actualy background downloader
int http_get_file_thread_handler(void *specs){
    struct http_get_struct *spec= (struct http_get_struct *) specs;
    SDL_Event event;

	// load the file
	spec->status= http_get_file(spec->server, spec->path, spec->fp);

	// signal we are done
	event.type= SDL_USEREVENT;
	event.user.code= spec->event;
	event.user.data1= spec;
	SDL_PushEvent(&event);
	// NOTE: it is up to the EVENT handler to close the handle & free the spec pointer in data1

	return(0);
}


// the http downloader that can be used in the foreground or background
int http_get_file(char *server, char *path, FILE *fp)
{
	IPaddress http_ip;
	TCPsocket http_sock;
	char message[1024];
	int len;
	int got_header= 0;
	int http_status= 0;

	// resolve the hostname
	if(SDLNet_ResolveHost(&http_ip, server, 80) < 0){   // caution, always port 80!
		return(1);  // can't resolve the hostname
	}
	// open the socket
	http_sock= SDLNet_TCP_Open(&http_ip);
	if(!http_sock){
		return(2);  // failed to open the socket
	}
	
	// send the GET request
	snprintf(message, sizeof(message), "GET %s HTTP/1.0\n\n", path);
	len= strlen(message);
	if(SDLNet_TCP_Send(http_sock,message,len) < len){
		// close the socket to prevent memory leaks
		SDLNet_TCP_Close(http_sock);
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
									if(http_status == 200){
										fwrite(buf+i+4, 1, len-i-4, fp);
									}
									break;
								}
						}
				}
			else
			    {
					if(http_status == 200){
						fwrite(buf, 1, len, fp);
					} else {
						break;
					}
				}
		}
	SDLNet_TCP_Close(http_sock);
	
	if(http_status != 200){
		if(http_status != 0){
			return(http_status);
		} else {
			return(5);
		}
	}

	return(0);  // finished
}

