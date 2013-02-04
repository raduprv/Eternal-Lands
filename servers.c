#include <string.h>
#include "servers.h"
#include "asc.h"
#include "errors.h"
#include "misc.h"
#include "multiplayer.h"
#include "io/elpathwrapper.h"
#define MAX_SERVERS 10

typedef struct
{
	char id[20];						// The ID of the server - to be specified on the command line
	char dir[20];						// The dir under $CONF_DIR
	unsigned char address[60];
	int port;
	char desc[100];						// Description of the server - to be shown on in the Server Selection screen
} server_def;

server_def servers[MAX_SERVERS];		// The details of all the servers we know about
int num_servers = 0;
int cur_server = -1;

char * check_server_id_on_command_line();	// From main.c

const char * get_server_name(void)
{
	if (cur_server >= 0)
		return servers[cur_server].id;
	else
		return "<unset>";
}

int find_server_from_id (const char* id)
{
	int i;

	if (num_servers <= 0) return -1;

	for (i = 0; i < num_servers; i++)
	{
		if (!strcasecmp(servers[i].id, id))
		{
			return i;
		}
	}
	return -1;
}

void set_server_details()
{
	char id[20];
	int num;
	safe_strncpy(id, check_server_id_on_command_line(), sizeof(id));
	if (!strcmp(id, ""))
	{
		safe_strncpy(id, "main", sizeof(id));
	}
	num = find_server_from_id(id);
	if (num == -1)
	{
		// Oops... what they they specify on the command line?
		LOG_ERROR("Server profile not found in servers.lst for server: %s. Failover to server: main.", id);
		// Failover to the main server
		num = find_server_from_id("main");
		if (num == -1)
		{
			// Error, this is a problem!
			LOG_ERROR("Fatal error: Server profile not found in servers.lst for server: main");
			exit(1);
		}
	}
	// We found a valid profile so set some vars
	LOG_DEBUG("Using the server profile: %s", servers[num].id);
	cur_server = num;
	safe_strncpy((char *)server_address, (char *)servers[num].address, sizeof(server_address));
	port = servers[num].port;
	// Check if the config directory for the profile exists and if not then create and
	// copy main's ini file into it
	if (!check_configdir())
	{
		char src[1000];
		char dest[1000];
		
		mkdir_tree(get_path_config(), 0);
		// First, try to copy the ini file out of $CONF/main
		safe_snprintf(src, sizeof(src), "%smain/el.ini", get_path_config_base());
		safe_snprintf(dest, sizeof(dest), "%sel.ini", get_path_config());
		copy_file(src, dest);
		// Secondly, try to copy the ini file out of $CONF (this will fail without harm if above succeeds)
		safe_snprintf(src, sizeof(src), "%s/el.ini", get_path_config_base());
		safe_snprintf(dest, sizeof(dest), "%sel.ini", get_path_config());
		copy_file(src, dest);
	}
}

const char * get_server_dir()
{
	if (cur_server >= 0)
		return servers[cur_server].dir;
	else
		return "";
}

void load_server_list(const char *filename)
{
	int f_size;
	FILE * f = NULL;
	char * server_list_mem;
	int istart, iend, i, section;
	char string[128];
	int len;
	
	f = open_file_config(filename, "rb");
	if (f == NULL)
	{
		// Error, this is a problem!
		const char *err_message = "Fatal error: %s file missing!\n";
		LOG_ERROR(err_message, filename);
		fprintf(stderr, err_message, filename);
		exit(1);
	}

	// Ok, allocate memory for it and read it in
	fseek(f, 0, SEEK_END);
	f_size = ftell(f);
	if (f_size <= 0)
	{
		const char *err_message = "Fatal error: %s is empty!\n";
		LOG_ERROR(err_message, filename);
		fprintf(stderr, err_message, filename);
		fclose(f);
		exit(1);
	}
	
	server_list_mem = (char *) calloc (f_size, 1);
	fseek(f, 0, SEEK_SET);
	if (fread(server_list_mem, 1, f_size, f) != f_size)
	{
		const char *err_message = "Fatal error: %s read failed!\n";
		LOG_ERROR(err_message, filename);
		fprintf(stderr, err_message, filename);
		free(server_list_mem);
		fclose(f);
		exit(1);
	}
	fclose(f);

	istart = 0;
	num_servers = 0;
	while (istart < f_size)
	{
		// Find end of the line
		for (iend = istart; iend < f_size; iend++)
		{
			if (server_list_mem[iend] == '\n' || server_list_mem[iend] == '\r')
				break;
		}

		// Parse this line
		if (iend > istart)
		{
			section = 0;
			len = 0;
			for (i = istart; i < iend; i++)
			{
				if (server_list_mem[i] == '#')
					break;	// This is a comment so ignore the rest of the line
				else if (section < 4 && (server_list_mem[i] == ' ' || server_list_mem[i] == '\t' || i == iend))
				{
					// This is the end of a section so store it (except the description)
					// as we include whitespace in the description
					string[len] = '\0';
					switch(section)
					{
						case 0:		// Server ID
							safe_strncpy(servers[num_servers].id, string, sizeof(servers[num_servers].id));
							break;
						case 1:		// Config dir
							safe_strncpy(servers[num_servers].dir, string, sizeof(servers[num_servers].dir));
							break;
						case 2:		// Server address
							safe_strncpy((char *)servers[num_servers].address, string, sizeof(servers[num_servers].address));
							break;
						case 3:		// Server port
							servers[num_servers].port = atoi(string);
							break;
					}
					section++;
					// Reset the length to start the string again
					len = 0;
					// Skip any more spaces
					while (i < iend)
					{
						if (server_list_mem[i+1] != ' ' && server_list_mem[i+1] != '\t')
							break;
						i++;
					}
				}
				else //if (server_list_mem[i] == ) // Valid char!!)
				{
					string[len] = server_list_mem[i];
					len++;
				}
			}
			if (i > istart) {
				// Anything left should be the description so store it now
				string[len] = '\0';
				safe_strncpy(servers[num_servers].desc, string, sizeof(servers[num_servers].desc));
				// Check the line was valid
				if (!strcmp(servers[num_servers].id, "") || !strcmp(servers[num_servers].dir, "")
					|| !strcmp((char *)servers[num_servers].address, "") || servers[num_servers].port == 0
					|| !strcmp(servers[num_servers].desc, ""))
				{
					LOG_ERROR("%s: Invalid server details specified in %s - (%d) %s", "Servers list error", filename, num_servers, servers[num_servers].id);
					break;		// Bail, but do the free first
				}
				
				// we added a valid line
				num_servers++;
			}
		}

		// Move to next line
		istart = iend + 1;
	}

	free(server_list_mem);
}


/* Logic

Check for server id, if set
	If server id is valid
		Check for a server address and port		// Override
		If host and port are different but match alternate server id
			Switch and warn
		Else if different and new
			Create server id, dir and adjust servers.lst?
		Else
			All good
	Else
		Check for a server address and port, if set
			Use and warn
		Else
			Error
Else
	Check for a server address and port, if set
		Check for matching server id, if found
			Use id
		Else
			Create server id, dir and adjust servers.lst
	Else
		Error


When creating server id, copy files from current dir


*/


/* Server selection window code */
/*
int server_sel_root_win = -1;
int server_sel_root_connect_id = -1;

void init_server_sel_interface(int len_x, int len_y)
{
	// Nothing to do yet
}

void draw_server_sel_interface (int len_x, int len_y)
{
	char str[200] = "Please select a server:";
//	float diff = (float) (len_x - len_y) / 2;
	float window_ratio = (float) len_x / 640.0f;

	draw_string ((len_x - (strlen(str) * 11)) / 2, 200 * window_ratio, (unsigned char*)str, 0);
*/
/*	Possibly use this box to display the list of files updated?
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
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
/*	
	
	glDisable (GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int display_server_sel_root_handler (window_info *win)
{
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{	
		draw_console_pic (cons_text);
		draw_server_sel_interface (win->len_x, win->len_y);
		CHECK_GL_ERRORS();
	}
	
	draw_delay = 20;
	return 1;
}

int click_server_sel_root_connect ()
{
	// Ummm?
	return 1;
}

int click_server_sel_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	return 0;
}

int keypress_server_sel_root_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;

	// first, try to see if we pressed Alt+x, to quit.
	if (check_quit_or_fullscreen(key))
	{
		return 1;
	}
	else if (keysym == SDLK_RETURN)
	{
		// Ummm?
		return 1;
	}
	
	return 1;
}

int show_server_sel_handler (window_info *win) {
	hide_all_root_windows();
	return 1;
}

void create_server_sel_root_window (int width, int height)
{
	if (server_sel_root_win < 0)
	{
		float window_ratio = (float) width / 640.0f;
		int connect_width = (strlen("Connect") * 11) + 40;
		int connect_height = 32;
		
		server_sel_root_win = create_window("Server Selection", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);

//		server_sel_root_connect_id = button_add_extended(server_sel_root_win, server_sel_root_connect_id, NULL, (width - connect_width) /2, height - (160 * window_ratio), connect_width, connect_height, 0, 1.0f, 1.0f, 1.0f, 1.0f, "Connect");

		set_window_handler (server_sel_root_win, ELW_HANDLER_DISPLAY, &display_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_CLICK, &click_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_KEYPRESS, &keypress_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_SHOW, &show_server_sel_handler);
		widget_set_OnClick(server_sel_root_win, server_sel_root_connect_id, &click_server_sel_root_connect);
	}
	
	init_server_sel_interface (width, height);
}
*/
