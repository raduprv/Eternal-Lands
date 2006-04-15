#include <ctype.h>
#include "global.h"

enum {
	SORT_BY_NAME,
	SORT_BY_SESSION,
	SORT_BY_TOTAL
};

struct Kill {
	char *name;
	int is_player;
	int n;
	int session_n;
};

int kills_win = -1;

static struct Kill *kills = 0;
static int kills_scroll_id = 15;
static int kills_n = 0;
static int sort_by = SORT_BY_NAME;
static int sort_desc = 0;

static int mouseover_name = 0;
static int mouseover_session = 0;
static int mouseover_total = 0;

int display_kills_handler(window_info *win);
int click_kills_handler(window_info *win, int mx, int my, Uint32 flags);
int mouseover_kills_handler(window_info *win, int mx, int my);

int sort_kills_func(const void *a, const void *b)
{
	const struct Kill *ka, *kb;

	if (sort_desc) {
		ka = b;
		kb = a;
	} else {
		ka = a;
		kb = b;
	}
	
	switch (sort_by) {
	case SORT_BY_NAME:
		return strcasecmp(ka->name, kb->name);
	case SORT_BY_SESSION:
		if (ka->session_n < kb->session_n)
			return -1;
		if (ka->session_n > kb->session_n)
			return 1;
		return 0;
	case SORT_BY_TOTAL:
		if (ka->n < kb->n)
			return -1;
		if (ka->n > kb->n)
			return 1;
		return 0;
	}

	return 0;
}

void sort_kills()
{
	qsort(kills, kills_n, sizeof(struct Kill), sort_kills_func);
}

void load_kills()
{
	char filename[256], stored_actor_name[16], username[16];
	FILE *f;
	int i, j;
	Uint8 l, p;
	Uint32 n;
	
	if (kills) {
		kills = realloc(kills, 0);
	}
	
	strncpy(username, username_str, sizeof(username));
	for (i = 0; username[i]; i++) {
		username[i] = tolower(username[i]);
	}
	
	snprintf(filename, sizeof(filename), "%s/kills_%s.dat", configdir, username);

	if (!(f = my_fopen(filename, "r+b")) && !(f = my_fopen(filename, "w+b"))) {
#ifdef WINDOWS
		snprintf(filename, sizeof(filename), "kills_%s.dat", username);
		if (!(f = my_fopen(filename, "r+b")) && !(f = my_fopen(filename, "w+b"))) {
			return;
		}
#else
		return;
#endif
	}

	j = 0;
	
	while (fread(&l, sizeof(l), 1, f) > 0) {
		fread(stored_actor_name, l, 1, f);
		stored_actor_name[l] = '\0';
		fread(&p, sizeof(p), 1, f);
		fread(&n, sizeof(n), 1, f);

		kills = realloc(kills, ++j * sizeof(struct Kill));
		kills[j-1].name = strdup(stored_actor_name);
		kills[j-1].is_player = p;
		kills[j-1].n = n;
		kills[j-1].session_n = 0;
	}

	fclose(f);

	kills_n = j;
	
	kills = realloc(kills, (kills_n + 1) * sizeof(struct Kill));
	kills[kills_n].name = 0;

	sort_kills();
}

/*
 * Remove colors and guild tag from an actor's name.
 */
char *strip_actor_name(char *actor_name)
{
	static char buf[16];
	int i, j;

	for (i = 0, j = 0; actor_name[i]; i++) {
		if ((unsigned char)actor_name[i] == 0x8F) {
			j--;
			break;
		}
		if ((unsigned char)actor_name[i] >= 127) {
			continue;
		}
		buf[j++] = actor_name[i];
	}

	buf[j] = '\0';

	return buf;
}

void log_kill(char *actor_name, int is_player)
{
	char filename[256], stored_actor_name[16], username[16];
	FILE *f;
	int i;
	Uint8 l, p;
	Uint32 n;
	
	strncpy(username, username_str, sizeof(username));
	for (i = 0; username[i]; i++) {
		username[i] = tolower(username[i]);
	}
	
	snprintf(filename, sizeof(filename), "%s/kills_%s.dat", configdir, username);

	if (!(f = my_fopen(filename, "r+b")) && !(f = my_fopen(filename, "w+b"))) {
#ifdef _WIN32
		snprintf(filename, sizeof(filename), "kills_%s.dat", username);
		if (!(f = my_fopen(filename, "r+b")) && !(f = my_fopen(filename, "w+b"))) {
			return;
		}
#else
		return;
#endif
	}

	while (fread(&l, sizeof(l), 1, f) > 0) {
		fread(stored_actor_name, l, 1, f);
		stored_actor_name[l] = '\0';
		
		fread(&p, sizeof(p), 1, f);
		
		if (p == is_player && !strcasecmp(stored_actor_name, actor_name)) {
			fread(&n, sizeof(n), 1, f);
			fseek(f, -sizeof(n), SEEK_CUR);
			n++;
			fwrite(&n, sizeof(n), 1, f);
			fclose(f);
			return;
		} else {
			fseek(f, sizeof(n), SEEK_CUR);
		}
	}

	l = strlen(actor_name);
	fwrite(&l, sizeof(l), 1, f);
	fwrite(actor_name, l, 1, f);
	p = is_player;
	fwrite(&p, sizeof(p), 1, f);
	n = 1;
	fwrite(&n, sizeof(n), 1, f);

	fclose(f);
}

void on_actor_killed(actor *a)
{
	int x1, y1;
	int x2, y2;
	int i;
	int is_player;
	actor *s;
	char *name;
	static int face_offsets[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

	if (a->actor_id == yourself) {
		return;
	}

	if (!(s = get_actor_ptr_from_id(yourself))) {
		return;
	}

	if (!a->async_fighting || !s->async_fighting) {
		return;
	}

	/* get the coords of the tile we're facing */
	x1 = s->async_x_tile_pos + face_offsets[((int)s->async_z_rot)/45][0];
	y1 = s->async_y_tile_pos + face_offsets[((int)s->async_z_rot)/45][1];

	/* get the coords of the next tile we're facing */
	x2 = x1 + face_offsets[((int)s->async_z_rot)/45][0];
	y2 = y1 + face_offsets[((int)s->async_z_rot)/45][1];

	/* return if this actor is not on one of these tiles */
	if ((a->async_x_tile_pos != x1 || a->async_y_tile_pos != y1) && (a->async_x_tile_pos != x2 || a->async_y_tile_pos != y2)) {
		return;
	}
	
	name = strip_actor_name(a->actor_name);
	is_player = (a->is_enhanced_model && (a->kind_of_actor == HUMAN || a->kind_of_actor == PKABLE_HUMAN));
	
	for (i = 0; kills[i].name; i++) {
		if (kills[i].is_player == is_player && !strcasecmp(kills[i].name, name)) {
			kills[i].n++;
			kills[i].session_n++;
			log_kill(strip_actor_name(name), is_player);
			break;
		}
	}

	if (!kills[i].name) {
		kills[i].name = strdup(name);
		kills[i].is_player = is_player;
		kills[i].n = 1;
		kills[i].session_n = 1;
		
		log_kill(strip_actor_name(name), kills[i].is_player);

		i++;
	
		kills = realloc(kills, (i + 1) * sizeof(struct Kill));
		kills[i].name = 0;

		kills_n++;
	}

	sort_kills();

	vscrollbar_set_bar_len(kills_win, kills_scroll_id, kills_n - 19);
}

int command_kills(char *text, int len)
{
	char buffer[32];
	int i;

	LOG_TO_CONSOLE(c_green1, "Name                Session  Total");
	LOG_TO_CONSOLE(c_green1, "----------------------------------");
	
	for (i = 0; kills[i].name; i++) {
		sprintf(buffer, "%-20s %6d %6d", kills[i].name, kills[i].session_n, kills[i].n);
		LOG_TO_CONSOLE(c_green1, buffer);
	}
	
	return 1;
}

void fill_kills_win()
{
	set_window_handler(kills_win, ELW_HANDLER_DISPLAY, &display_kills_handler);
	set_window_handler(kills_win, ELW_HANDLER_CLICK, &click_kills_handler);
	set_window_handler(kills_win, ELW_HANDLER_MOUSEOVER, &mouseover_kills_handler);

	kills_scroll_id = vscrollbar_add_extended (kills_win, kills_scroll_id, NULL, STATS_TAB_WIDTH - 20, 25, 20, STATS_TAB_HEIGHT - 25, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, kills_n-19);
}

int display_kills_handler(window_info *win)
{
	int i, n, x, y;
	int scroll;
	char buffer[32];
	
	x = 10;
	y = 8;

	if (mouseover_name) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x, y, "Name", 1);

	if (mouseover_session) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 200, y, "Session", 1);

	if (mouseover_total) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 300, y, "Total", 1);

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	glVertex3i(0, 25, 0);
	glVertex3i(win->len_x, 25, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);

	y = 30;

	scroll = vscrollbar_get_pos(kills_win, kills_scroll_id);

	if (scroll < 0) {
		scroll = 0;
	}
	
	for (i = scroll, n = 0; kills[i].name; i++, n++) {
		if (n == 19) {
			break;
		}

		if (kills[i].is_player) {
			glColor3f(1.0f, 0.0f, 0.0f);
		}

		draw_string_small(x, y, kills[i].name, 1);
		sprintf(buffer, "%d", kills[i].session_n);
		draw_string_small(x + 200, y, buffer, 1);
		sprintf(buffer, "%d", kills[i].n);
		draw_string_small(x + 300, y, buffer, 1);
		y += 16;

		if (kills[i].is_player) {
			glColor3f(1.0f, 1.0f, 1.0f);
		}
	}
	
	return 1;
}

int click_kills_handler(window_info *win, int mx, int my, Uint32 flags)
{
	 if (my > 25) {
		 if (flags&ELW_WHEEL_UP) {
			vscrollbar_scroll_up(kills_win, kills_scroll_id);
			return 1;
		 }

		 if (flags&ELW_WHEEL_DOWN) {
			vscrollbar_scroll_down(kills_win, kills_scroll_id);
			return 1;
		 }
	} else {
		if (mouseover_name) {
			if (sort_by == SORT_BY_NAME) {
				sort_desc = !sort_desc;
			} else {
				sort_by = SORT_BY_NAME;
				sort_desc = 0;
			}
		}
		if (mouseover_session) {
			if (sort_by == SORT_BY_SESSION) {
				sort_desc = !sort_desc;
			} else {
				sort_by = SORT_BY_SESSION;
				sort_desc = 0;
			}
		}
		if (mouseover_total) {
			if (sort_by == SORT_BY_TOTAL) {
				sort_desc = !sort_desc;
			} else {
				sort_by = SORT_BY_TOTAL;
				sort_desc = 0;
			}
		}
		sort_kills();
	}

	return 1;
}

int mouseover_kills_handler(window_info *win, int mx, int my)
{
	mouseover_name = mouseover_session = mouseover_total = 0;

	if (my > 25) {
		return 0;
	}
	
	if (mx >= 10 && mx <= 40) {
		mouseover_name = 1;
		return 0;
	}

	if (mx >= 210 && mx <= 265) {
		mouseover_session = 1;
		return 0;
	}

	if (mx >= 310 &&  mx <= 350) {
		mouseover_total = 1;
		return 0;
	}

	return 0;
}
