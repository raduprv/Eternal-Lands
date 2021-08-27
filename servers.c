#include <string.h>
#include "servers.h"
#include "asc.h"
#ifdef USE_SSL
#include "connection.h"
#endif
#include "elconfig.h"
#include "errors.h"
#include "gl_init.h"
#include "misc.h"
#include "multiplayer.h"
#include "io/elpathwrapper.h"

#define DEFAULT_SERVERS_SIZE 4

typedef struct
{
	char id[20];						// The ID of the server - to be specified on the command line
	char dir[20];						// The dir under $CONF_DIR
	unsigned char address[60];
	int port;
	char desc[100];						// Description of the server - to be shown on in the Server Selection screen
#ifdef USE_SSL
	int encrypt;
#endif // USE_SSL
} server_def;

static server_def* servers = NULL;		// The details of all the servers we know about
static int servers_size = 0;
static int num_servers = 0;
static int cur_server = -1;

const char* check_server_id_on_command_line();	// From main.c

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
		// Oops... what did they specify on the command line?
		LOG_ERROR("Server profile not found in servers.lst for server: %s. Failover to server: main.", id);
		// Failover to the main server
		num = find_server_from_id("main");
		if (num == -1)
		{
			// Error, this is a problem!
			static char *error_str = "Fatal error: Server profile not found in servers.lst for server: main";
			LOG_ERROR(error_str);
			FATAL_ERROR_WINDOW(error_str);
			exit(1);
		}
	}
	// We found a valid profile so set some vars
	LOG_DEBUG("Using the server profile: %s", servers[num].id);
	cur_server = num;
#ifdef USE_SSL
	connection_set_server((char *)servers[num].address, servers[num].port, servers[num].encrypt);
#else
	safe_strncpy((char *)server_address, (char *)servers[num].address, sizeof(server_address));
	port = servers[num].port;
#endif
	// Check if the config directory for the profile exists and if not then create and
	// copy main's ini file into it
	if (!check_configdir())
	{
		char src[1000];
		char dest[1000];
		
		mkdir_tree(get_path_config(), 0);
		// First, try to copy the ini file out of $CONF/main
		safe_snprintf(src, sizeof(src), "%smain/%s", get_path_config_base(), ini_filename);
		safe_snprintf(dest, sizeof(dest), "%s%s", get_path_config(), ini_filename);
		copy_file(src, dest);
		// Secondly, try to copy the ini file out of $CONF (this will fail without harm if above succeeds)
		safe_snprintf(src, sizeof(src), "%s/%s", get_path_config_base(), ini_filename);
		safe_snprintf(dest, sizeof(dest), "%s%s", get_path_config(), ini_filename);
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

static server_def* reallocate_servers_list(int min_size)
{
	if (servers_size < min_size)
	{
		server_def* new_servers;
		int new_size = servers_size > 0 ? 2*servers_size : DEFAULT_SERVERS_SIZE;
		while (new_size < min_size)
			new_size *= 2;
		new_servers = realloc(servers, new_size * sizeof(server_def));
		if (!new_servers)
			return NULL;

		servers = new_servers;
		servers_size = new_size;
	}
	return servers;
}

void load_server_list(const char *filename)
{
	int f_size;
	FILE * f = NULL;
	char *server_list_mem, *line;
	char format[128];
#ifdef USE_SSL
	char crypt[128];
#endif
	
	f = open_file_config(filename, "rb");
	if (f == NULL)
	{
		// Error, this is a problem!
		const char *err_message = "Fatal error: %s file missing!\n";
		LOG_ERROR(err_message, filename);
		fprintf(stderr, err_message, filename);
		FATAL_ERROR_WINDOW(err_message, filename);
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
		FATAL_ERROR_WINDOW(err_message, filename);
		exit(1);
	}
	
	server_list_mem = calloc(f_size+1, 1);
	fseek(f, 0, SEEK_SET);
	if (fread(server_list_mem, 1, f_size, f) != f_size)
	{
		const char *err_message = "Fatal error: %s read failed!\n";
		LOG_ERROR(err_message, filename);
		fprintf(stderr, err_message, filename);
		free(server_list_mem);
		fclose(f);
		FATAL_ERROR_WINDOW(err_message, filename);
		exit(1);
	}
	fclose(f);

#ifdef USE_SSL
	safe_snprintf(format, sizeof(format), "%%%zus %%%zus %%%zus %%u %%n%%%zus %%n",
		sizeof_field(server_def, id) - 1, sizeof_field(server_def, dir) - 1,
		sizeof_field(server_def, address) - 1, sizeof(crypt) - 1);
#else // USE_SSL
	safe_snprintf(format, sizeof(format), "%%%zus %%%zus %%%zus %%u %%%zu[^\r\n]",
		sizeof_field(server_def, id) - 1, sizeof_field(server_def, dir) - 1,
		sizeof_field(server_def, address) - 1, sizeof_field(server_def, desc) - 1);
#endif // USE_SSL

	num_servers = 0;
	line = server_list_mem;
	while (*line)
	{
		int nr_fields;
#ifdef USE_SSL
		int crypt_pos, desc_pos;
#endif
		size_t iend;
		char* comment;

		iend = strcspn(line, "\r\n");
		line[iend] = '\0';
		comment = strchr(line, '#');
		if (comment)
			*comment = '\0';

		if (num_servers >= servers_size && !reallocate_servers_list(num_servers + 1))
		{
			const char *errstg = "Fatal error: Too many servers specified in";
			LOG_ERROR("%s %s", errstg, filename);
			fprintf(stderr, "%s %s\n", errstg, filename);
			FATAL_ERROR_WINDOW(errstg);
			exit(1);
		}

#ifdef USE_SSL
		nr_fields = sscanf(line, format, servers[num_servers].id, servers[num_servers].dir,
			servers[num_servers].address, &servers[num_servers].port, &crypt_pos, crypt, &desc_pos);
#else
		nr_fields = sscanf(line, format, servers[num_servers].id, servers[num_servers].dir,
			servers[num_servers].address, &servers[num_servers].port, &servers[num_servers].desc);
#endif
		if (nr_fields == 4)
		{
			// No encryption or description field
			servers[num_servers].desc[0] = 0;
#ifdef USE_SSL
			servers[num_servers].encrypt = 0;
#endif // USE_SSL
			++num_servers;
		}
		else if (nr_fields == 5)
		{
#ifdef USE_SSL
			if (!strcasecmp(crypt, "crypt") || !strcasecmp(crypt, "encrypt")
				|| !strcasecmp(crypt, "encrypted"))
			{
				// There is an encryption field, and we should encrypt
				servers[num_servers].encrypt = 1;
			}
			else if (!strcasecmp(crypt, "plain") || !strcasecmp(crypt, "clear"))
			{
				// There is an encryption field, and we should not encrypt
				servers[num_servers].encrypt = 0;
			}
			else
			{
				// There is no encryption field, and the string is part of the description
				servers[num_servers].encrypt = 0;
				desc_pos = crypt_pos;
			}

			safe_strncpy(servers[num_servers].desc, line+desc_pos, sizeof(servers[num_servers].desc));
#endif // USE_SSL
			++num_servers;
		}

		line += iend + 1;
		while (*line == '\r' || *line == '\n')
			++line;
	}

	free(server_list_mem);
}

void free_servers(void)
{
	free(servers);
	servers = NULL;
	servers_size = num_servers = 0;
	cur_server = -1;
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

//		server_sel_root_connect_id = button_add_extended(server_sel_root_win, server_sel_root_connect_id, NULL, (width - connect_width) /2, height - (160 * window_ratio), connect_width, connect_height, 0, 1.0f, "Connect");

		set_window_handler (server_sel_root_win, ELW_HANDLER_DISPLAY, &display_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_CLICK, &click_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_KEYPRESS, &keypress_server_sel_root_handler);
		set_window_handler (server_sel_root_win, ELW_HANDLER_SHOW, &show_server_sel_handler);
		widget_set_OnClick(server_sel_root_win, server_sel_root_connect_id, &click_server_sel_root_connect);
	}
	
	init_server_sel_interface (width, height);
}
*/
