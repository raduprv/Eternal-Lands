#ifdef COUNTERS

#include <stdlib.h>
#include <ctype.h>
#include "counters.h"
#include "asc.h"
#include "elwindows.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "multiplayer.h"
#include "spells.h"
#include "tabs.h"
#ifdef NEW_FILE_IO
#include "io/elpathwrapper.h"
#else
#include "misc.h"
#endif
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

#define NUM_COUNTERS 13
#define NUM_LINES 18
#define MAX(a,b) (a > b ? a : b)

/* Counter IDs */
enum {
	KILLS = 1,
	DEATHS,
	HARVESTS,
	ALCHEMY,
	CRAFTING,
	MANUFACTURING,
	POTIONS,
	SPELLS,
	SUMMONS,
	ENGINEERING,
	BREAKS,
	MISC_EVENTS,
	TAILORING
};

/* Columns IDs */
enum {
	NAME = 1,
	SESSION,
	TOTAL
};

/* Counter structure */
struct Counter {
	char *name;
	Uint32 n_session;
	Uint32 n_total;
	Uint32 extra;
};

static struct Counter *counters[NUM_COUNTERS];
static int counters_initialized = 0;
static int selected_counter_id = KILLS;
static int last_selected_counter_id = KILLS;
static int sort_counter_id = KILLS;
static int entries[NUM_COUNTERS];
static int sort_by[NUM_COUNTERS];
static int multiselect_id;

static int mouseover_name = 0;
static int mouseover_session = 0;
static int mouseover_total = 0;
static int mouseover_entry_y = -1;

static int product_count = 0;
static char product_name[128];
static char to_count_name[128];
static char *spell_names[128] = { NULL };
static int requested_spell_id = -1;

static const char *temp_event_string[] =
	{	"%s found a", /* keep this one first in the list as its different from the rest*/
		"%s was blessed by the Queen of Nature with ", /* referenced laters as second in the list */
		"A cavern wall collapsed on %s",
		"Mother Nature got pissed off at %s",
		"%s was stung by a bee, losing ",
		"%s just hit a teleport nexus, ",
		"While harvesting, %s upset a radon pouch, ",
		"%s found Joker and got ",
		"%s found Joker and failed to get" };
static const char *count_str[] =
	{	"dummy",
		"Blessed by the Queen of Nature",
		"A cavern wall collapsed",
		"Mother Nature got pissed off",
		"Stung by a bee",
		"Hit a teleport nexus",
		"Upset a radon pouch",
		"Gift from Joker",
		"Gift from Joker (lost)" };
static const int num_search_str = sizeof(count_str)/sizeof(char *);
static char **search_str = NULL;
static size_t *search_len = NULL;
static Uint32 misc_event_time = 0;

int harvesting = 0;
Uint32 disconnect_time;
char harvest_name[32];

int counters_win = -1;
int counters_scroll_id = 16;

void increment_counter(int counter_id, const char *name, int quantity, int extra);

int display_counters_handler(window_info *win);
int click_counters_handler(window_info *win, int mx, int my, Uint32 extra);
int mouseover_counters_handler(window_info *win, int mx, int my);

int sort_counter_func(const void *a, const void *b)
{
	const struct Counter *ca, *cb;

	if (sort_by[sort_counter_id-1] < 0) {
		ca = b;
		cb = a;
	} else {
		ca = a;
		cb = b;
	}
	
	switch (abs(sort_by[sort_counter_id-1])) {
	case TOTAL:
		if (ca->n_total < cb->n_total)
			return -1;
		if (ca->n_total > cb->n_total)
			return 1;
	case SESSION:
		if (ca->n_session < cb->n_session)
			return -1;
		if (ca->n_session > cb->n_session)
			return 1;
	case NAME:
		return strcasecmp(ca->name, cb->name);
	}

	return 0;
}

void sort_counter(int counter_id)
{
	if (sort_by[counter_id-1]) {
		sort_counter_id = counter_id;
		qsort(counters[counter_id-1], entries[counter_id-1], sizeof(struct Counter), sort_counter_func);
	}
}

FILE *open_counters_file(char *mode)
{
	char filename[256], username[16];
	int i;
	
	safe_strncpy(username, username_str, sizeof(username));
	for (i = 0; username[i]; i++) {
		username[i] = tolower(username[i]);
	}

#if !defined(NEW_FILE_IO) && !defined(WINDOWS)
	safe_snprintf(filename, sizeof(filename), "%s/counters_%s.dat", configdir, username);
#else
	safe_snprintf(filename, sizeof(filename), "counters_%s.dat", username);
#endif

#ifndef NEW_FILE_IO
	return my_fopen(filename, mode);
#else /* NEW_FILE_IO */
	return open_file_config(filename, mode);
#endif /* NEW_FILE_IO */
}

void load_counters()
{
	FILE *f;
	int i, j;
	Uint8 io_counter_id;
	Uint8 io_name_len;
	Uint32 io_extra;
	Uint32 io_n_total;
	char io_name[64];
	
	if (counters_initialized) {
		/*
		 * save eny existing counters before reloading
		 * this will take place when relogging after disconnection
		 */
		flush_counters();
		return;
	}
	
	for (i = 0; i < NUM_COUNTERS; i++) {
		counters[i] = NULL;
		entries[i] = 0;
		sort_by[i] = 0;
	}
	
	/* allocate and set misc event matching strings */
	search_str = malloc (sizeof (char *) * num_search_str);
	search_len = malloc (sizeof (size_t) * num_search_str);
	for (i=0; i<num_search_str; i++)
	{
		size_t max_len = strlen (username_str) + strlen (temp_event_string[i]) + 1;
		search_str[i] = malloc (max_len);
		safe_snprintf (search_str[i], max_len, temp_event_string[i], username_str);
		search_len[i] = strlen (search_str[i]);
	}

	if (!spell_names[0]) {
		memset(&spell_names, 0, sizeof(spell_names));
	}
	
	if (!(f = open_counters_file("rb"))) {
		counters_initialized = 1;
		return;
	}

	while (fread(&io_counter_id, sizeof(io_counter_id), 1, f) > 0) {
		fread(&io_name_len, sizeof(io_name_len), 1, f);
		fread(io_name, io_name_len, 1, f);
		io_name[io_name_len] = '\0';

		fread(&io_extra, sizeof(io_extra), 1, f);
		fread(&io_n_total, sizeof(io_n_total), 1, f);

		if(strlen(io_name)<1 || strlen(io_name)>100){
			//doesn't seem to have a real name, so we don't want it
			continue;
		}

		i = io_counter_id - 1;
		j = entries[i]++;
		counters[i] = realloc(counters[i], entries[i] * sizeof(struct Counter));
		counters[i][j].name = strdup(io_name);
		counters[i][j].n_session = 0;
		counters[i][j].n_total = io_n_total;
		counters[i][j].extra = io_extra;
	}

	fclose(f);
	
	counters_initialized = 1;
}

void flush_counters()
{
	FILE *f;
	int i, j;
	Uint8 io_counter_id;
	Uint8 io_name_len;
	
	if (!counters_initialized) {
		return;
	}

	if (!(f = open_counters_file("wb"))) {
		return;
	}

	for (i = 0; i < NUM_COUNTERS; i++) {
		io_counter_id = i + 1;

		for (j = 0; j < entries[i]; j++) {
			io_name_len = strlen(counters[i][j].name);
			
			fwrite(&io_counter_id, sizeof(io_counter_id), 1, f);
			fwrite(&io_name_len, sizeof(io_name_len), 1, f);
			fwrite(counters[i][j].name, io_name_len, 1, f);
			fwrite(&counters[i][j].extra, sizeof(counters[i][j].extra), 1, f);
			fwrite(&counters[i][j].n_total, sizeof(counters[i][j].n_total), 1, f);
		}
	}

	fclose(f);
}

void cleanup_counters()
{
	int i, j;

	for (i = 0; i < NUM_COUNTERS; i++) {
		if (counters[i]) {
			for (j = 0; j < entries[i]; ++j) {
				free(counters[i][j].name);
			}
			free(counters[i]);
			counters[i] = NULL;
			entries[i] = 0;
		}
	}

	/* free memory used by misc event matching strings */
	if (search_str != NULL)
	{
		for (i=0; i<num_search_str; i++)
			free(search_str[i]);
		free(search_str);
		free(search_len);
		search_str = NULL;
		search_len = NULL;
	}
	
	harvesting = 0;
	counters_initialized = 0;
}

void increment_counter(int counter_id, const char *name, int quantity, int extra)
{
	int i, j;
	int new_entry = 1;

	if(name == 0 || strlen(name)<1 || strlen(name)>100){
		//doesn't seem to have a real name, so no point saving it
		return;
	}

	i = counter_id - 1;
	
	/* Look for an existing entry. */
	for (j = 0; j < entries[i]; j++) {
		if ((name && strcasecmp(counters[i][j].name, name)) || counters[i][j].extra != extra) {
			continue;
		}

		counters[i][j].n_session += quantity;
		counters[i][j].n_total += quantity;
		new_entry = 0;
		break;
	}

	if (new_entry) {
		/* Create a new entry. */
		j = entries[i]++;
		counters[i] = realloc(counters[i], entries[i] * sizeof(struct Counter));
		counters[i][j].name = strdup(name);
		counters[i][j].n_session = quantity;
		counters[i][j].n_total = quantity;
		counters[i][j].extra = extra;
	}

	sort_counter(counter_id);
	
	/* make sure any produce name/quanity is not used again */
	counters_set_product_info("",0);
}

void decrement_counter(int counter_id, char *name, int quantity, int extra)
{
	int i, j;

	i = counter_id - 1;

	for (j = 0; j < entries[i]; j++) {
		if (strcasecmp(counters[i][j].name, name) || counters[i][j].extra != extra) {
			continue;
		}

		counters[i][j].n_session -= quantity;
		counters[i][j].n_total -= quantity;

		return;
	}
}

void fill_counters_win()
{
	int idx = selected_counter_id > 0 ? selected_counter_id-1 : 0;
	
	set_window_handler(counters_win, ELW_HANDLER_DISPLAY, &display_counters_handler);
	set_window_handler(counters_win, ELW_HANDLER_CLICK, &click_counters_handler);
	set_window_handler(counters_win, ELW_HANDLER_MOUSEOVER, &mouseover_counters_handler);

	multiselect_id = multiselect_add(counters_win, NULL, 8, 8, 104);
	multiselect_button_add(counters_win, multiselect_id, 0, 0, "Kills", 1);
	multiselect_button_add(counters_win, multiselect_id, 0, 25, "Deaths", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 100, "Harvests", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 125, "Alchemy", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 150, "Crafting", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 175, "Manufac.", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 200, "Potions", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 225, "Spells", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 250, "Summons", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 275, "Engineering", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 50, "Breakages", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 75, "Events", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 300, "Tailoring", 0);

	counters_scroll_id = vscrollbar_add_extended(counters_win,
			counters_scroll_id, NULL,
			STATS_TAB_WIDTH - 20, 25, 20,
			STATS_TAB_HEIGHT - 50, 0,
			1.0f, 0.77f, 0.57f, 0.39f,
			0, 1, MAX(0, entries[idx] - NUM_LINES));
}

int display_counters_handler(window_info *win)
{
	int i, j, n, x, y;
	int scroll;
	int total, session_total;
	char buffer[32];

	i = multiselect_get_selected(counters_win, multiselect_id);
	selected_counter_id = i + 1;

	if (selected_counter_id != last_selected_counter_id) {
		vscrollbar_set_bar_len(counters_win, counters_scroll_id, MAX(0, entries[i] - NUM_LINES));
		vscrollbar_set_pos(counters_win, counters_scroll_id, 0);
		last_selected_counter_id = selected_counter_id;
	}

	x = 120;
	y = 8;
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f, 0.57f, 0.39f);
	glBegin(GL_LINES);
	
	glVertex3i(x, 0, 0);
	glVertex3i(x, win->len_y, 0);
	
	glVertex3i(x, 25, 0);
	glVertex3i(win->len_x, 25, 0);
	
	glVertex3i(x, win->len_y-25, 0);
	glVertex3i(win->len_x, win->len_y-25, 0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	x += 10;

	if (mouseover_name) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x, y, (unsigned char*)"Name", 1);

	if (mouseover_session) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 200, y, (unsigned char*)"This Session", 1);

	if (mouseover_total) glColor3f(0.6f, 0.6f, 0.6f);
	else glColor3f(1.0f, 1.0f, 1.0f);
	draw_string_small(x + 370, y, (unsigned char*)"Total", 1);

	if (counters_scroll_id != -1) {
		scroll = vscrollbar_get_pos(counters_win, counters_scroll_id);
	} else {
		scroll = 0;
	}
	
	for (j = scroll, n = 0, y = 30; j < entries[i]; j++, n++) {
		if (n == NUM_LINES) {
			break;
		}

		if ((selected_counter_id == KILLS || selected_counter_id == DEATHS) && counters[i][j].extra) {
			glColor3f(0.8f, 0.2f, 0.2f);
		} else {
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		
		/* draw first so left padding does not overwrite name */
		safe_snprintf(buffer, sizeof(buffer), "%12d", counters[i][j].n_session);
		draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
		safe_snprintf(buffer, sizeof(buffer), "%12d", counters[i][j].n_total);
		draw_string_small(x + 314, y, (unsigned char*)buffer, 1);

		if (counters[i][j].name) {
			float max_name_x;
			float font_ratio = 8.0/12.0;
			safe_snprintf(buffer, sizeof(buffer), "%d", counters[i][j].n_session);
			max_name_x = 425.0 - (130.0 + get_string_width((unsigned char*)buffer) * font_ratio);
			/* if the name would overlap the session total, truncate it */
			if ((get_string_width((unsigned char*)counters[i][j].name) * font_ratio) > max_name_x) {
				float len_dots = get_string_width((unsigned char*)"... ") * font_ratio;
				char *np = counters[i][j].name;
				float string_width = 0;
				char *used_name = (char *)malloc(strlen(counters[i][j].name)+1);
				char *unp = used_name;
				/* make a truncated copy of the name */
				while (*np != '\0') {
					float char_width = get_char_width(*np) * font_ratio;
					if ((string_width+char_width) > (max_name_x-len_dots))
						break;
					*unp++ = *np++;
					string_width += char_width;
				}
				*unp = '\0';				
				draw_string_small(x, y, (unsigned char*)used_name, 1);
				draw_string_small(x+string_width, y, (unsigned char*)"...", 1);
				/* if the mouse is over this line and its truncated, tooltip to full name */
				if (mouseover_entry_y > y && mouseover_entry_y < y+16)
					show_help(counters[i][j].name, 0, win->len_y+5);
				free(used_name);
			}
			else
				draw_string_small(x, y, (unsigned char*)counters[i][j].name, 1);
		}
		y += 16;
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	draw_string_small(x, win->len_y - 20, (unsigned char*)"Totals:", 1);
		
	for (j = 0, total = 0, session_total = 0; j < entries[i]; j++) {
		total += counters[i][j].n_total;
		session_total += counters[i][j].n_session;
	}

	safe_snprintf(buffer, sizeof(buffer), "%12d", session_total);
	draw_string_small(x + 200, win->len_y - 20, (unsigned char*)buffer, 1);

	safe_snprintf(buffer, sizeof(buffer), "%5d", total);
	draw_string_small(x + 370, win->len_y - 20, (unsigned char*)buffer, 1);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int click_counters_handler(window_info *win, int mx, int my, Uint32 extra)
{
	 if (mx > 120 && my > 25 && my < win->len_y - 25) {
		 if (extra&ELW_WHEEL_UP) {
			vscrollbar_scroll_up(counters_win, counters_scroll_id);
			return 1;
		 }

		 if (extra&ELW_WHEEL_DOWN) {
			vscrollbar_scroll_down(counters_win, counters_scroll_id);
			return 1;
		 }
	} else {
		if (mouseover_name) {
			if (sort_by[selected_counter_id-1] == NAME) {
				sort_by[selected_counter_id-1] = -NAME;
			} else {
				sort_by[selected_counter_id-1] = NAME;
			}
		}
		if (mouseover_session) {
			if (sort_by[selected_counter_id-1] == SESSION) {
				sort_by[selected_counter_id-1] = -SESSION;
			} else {
				sort_by[selected_counter_id-1] = SESSION;
			}
		}
		if (mouseover_total) {
			if (sort_by[selected_counter_id-1] == TOTAL) {
				sort_by[selected_counter_id-1] = -TOTAL;
			} else {
				sort_by[selected_counter_id-1] = TOTAL;
			}
		}
		sort_counter(selected_counter_id);
	}

	return 1;
}

int mouseover_counters_handler(window_info *win, int mx, int my)
{
	mouseover_name = mouseover_session = mouseover_total = 0;
	mouseover_entry_y = -1;
	
	if (my > 25){
		if (my > 30 && my < (30+NUM_LINES*16) && mx >= 130 && mx <= 425) {
			mouseover_entry_y = my;
		}
		return 0;
	}
	
	if (mx >= 130 && mx <= 165) {
		mouseover_name = 1;
		return 0;
	}

	if (mx >= 330 && mx <= 425) {
		mouseover_session = 1;
		return 0;
	}

	if (mx >= 500 &&  mx <= 540) {
		mouseover_total = 1;
		return 0;
	}

	return 0;
}

/*
 * Remove colors and guild tag from an actor's name.
 */
const char *strip_actor_name (const char *actor_name)
{
	static char buf[32];
	int i;

	/* strip a leading color code */
	if (is_color (actor_name[0])) {
		actor_name++;
	}
	
	/* copy the name minus the guild tag */
	for (i = 0; actor_name[i] && i < sizeof(buf); i++) {
		if (is_color (actor_name[i]))
			break;
		buf[i] = actor_name[i];
	}

	buf[i] = '\0';

	return buf;
}

/*
 * Called by increment_death_counter if it wasn't our character that died.
 */
void increment_kill_counter(actor *me, actor *them)
{
	int x1, y1, x2, y2;
	static int face_offsets[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};
	
	if (!them->async_fighting) {
		return;
	}

	/* get the coords of the tile we're facing */
	x1 = me->async_x_tile_pos + face_offsets[((int)me->async_z_rot)/45][0];
	y1 = me->async_y_tile_pos + face_offsets[((int)me->async_z_rot)/45][1];

	/* get the coords of the next tile we're facing (necessary for Chimerans) */
	x2 = x1 + face_offsets[((int)me->async_z_rot)/45][0];
	y2 = y1 + face_offsets[((int)me->async_z_rot)/45][1];

	/* return if this actor is not on one of these tiles */
	if ((them->async_x_tile_pos != x1 || them->async_y_tile_pos != y1) && (them->async_x_tile_pos != x2 || them->async_y_tile_pos != y2)) {
		return;
	}
	
	increment_counter(KILLS, strip_actor_name(them->actor_name), 1,	(them->is_enhanced_model && (them->kind_of_actor == HUMAN || them->kind_of_actor == PKABLE_HUMAN)));
}

/*
 * Called whenever an actor dies.
 */
void increment_death_counter(actor *a)
{
	int found_death_reason = 0;
	actor *me = get_actor_ptr_from_id(yourself);

	if (!me) {
		return;
	}
	
	/* count deaths that happend while harvesting, may not have been due to harvest event though */
	if ((a == me) && !me->async_fighting && harvesting) {
		/* a crude check to see if death was just after (1 second should be enough) a harvest event */
		if (abs(SDL_GetTicks() - misc_event_time) < 1000) {
			increment_counter(DEATHS, "Harvesting event", 1, 0);
			found_death_reason = 1;
		}
	}	
	
	/* count deaths while we were poisoned - possibily in adition to another possible reason */
	if ((a == me) && we_are_poisoned()) {
		increment_counter(DEATHS, "While poisoned", 1, 0);
		found_death_reason = 1;
	}
	
	/* if you are kill while harvesting, there is no "you stopped harvesting" message */
	/* resetting now prevents next items added to inventry getting added to harvest counter */
	if (harvesting && (a == me)) {
		harvesting = 0;
	}
	
	if (!me->async_fighting) {
		if (!found_death_reason && (a == me)) {
			/* if we died while not harvesting, fighting or poisoned, the cause is unknow */
			increment_counter(DEATHS, "Unknown cause", 1, 0);
		}
		return;
	}
	
	if (a != me) {
		increment_kill_counter(me, a);
		found_death_reason = 1;
	} else {
		int x1, y1, x2, y2;
		int i;
		actor *me, *them;
		static int face_offsets[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

		me = a;
		
		for (i = 0; i < max_actors; i++) {
			them = actors_list[i];

			if (!them->async_fighting) {
				continue;
			}

			/* get the coords of the tile they're facing */
			x1 = them->async_x_tile_pos + face_offsets[((int)them->async_z_rot)/45][0];
			y1 = them->async_y_tile_pos + face_offsets[((int)them->async_z_rot)/45][1];
			
			/* get the coords of the next tile they're facing (necessary for Chimerans) */
			x2 = x1 + face_offsets[((int)them->async_z_rot)/45][0];
			y2 = y1 + face_offsets[((int)them->async_z_rot)/45][1];
			
			/* continue if our actor is not on one of these tiles */
			if ((me->async_x_tile_pos != x1 || me->async_y_tile_pos != y1) && (me->async_x_tile_pos != x2 || me->async_y_tile_pos != y2)) {
				continue;
			}

			increment_counter(DEATHS, strip_actor_name(them->actor_name), 1, (them->is_enhanced_model && (them->kind_of_actor == HUMAN || them->kind_of_actor == PKABLE_HUMAN)));
			found_death_reason = 1;
		}
	}
	
	/* if we died while not harvesting or poisoned, but fighting and the opponent is unknown */
//	if (!found_death_reason && (a == me)) {
//		increment_counter(DEATHS, "While fighting unknown opponent", 1, 0);
//	}
}

/*
 * Called whenever we successfully make something.
 */
void counters_set_product_info(char *name, int count)
{
	safe_strncpy(product_name, name, sizeof(product_name));
	product_count = count;
}

void increment_alchemy_counter()
{
	increment_counter(ALCHEMY, product_name, product_count, 0);
}

void increment_crafting_counter()
{
	increment_counter(CRAFTING, product_name, product_count, 0);
}

void increment_engineering_counter()
{
	increment_counter(ENGINEERING, product_name, product_count, 0);
}

void increment_tailoring_counter()
{
	increment_counter(TAILORING, product_name, product_count, 0);
}

void increment_potions_counter()
{
	increment_counter(POTIONS, product_name, product_count, 0);
}

void increment_manufacturing_counter()
{
	increment_counter(MANUFACTURING, product_name, product_count, 0);
}

void increment_harvest_counter(int quantity)
{
	increment_counter(HARVESTS, harvest_name, quantity, 0);
}

void decrement_harvest_counter(int quantity)
{
	decrement_counter(HARVESTS, harvest_name, quantity, 0);
}

void counters_set_spell_name(int spell_id, char *name, int len)
{
	if (!spell_names[spell_id+1]) {
		int i, j;
		
		spell_names[spell_id+1] = malloc(len+1);
		safe_strncpy(spell_names[spell_id + 1], name, len + 1);

		i = SPELLS - 1;

		for (j = 0; j < entries[i]; j++) {
			if (!counters[i][j].name && counters[i][j].extra == spell_id) {
				counters[i][j].name = strdup(spell_names[spell_id+1]);
				break;
			}
		}
		// the name lookup must have been previously requested in increment_spell_counter()
		if (requested_spell_id == spell_id)
		{
			increment_counter(SPELLS, spell_names[spell_id+1], 1, spell_id);
			requested_spell_id = -1;
		}
	}
}

void increment_spell_counter(int spell_id)
{
	if (!spell_names[spell_id+1]) {
		Uint8 str[2];

		str[0] = SPELL_NAME;
		str[1] = (Sint8)spell_id;
		my_tcp_send(my_socket, str, 2);
		requested_spell_id = spell_id;

	}
	// delay the increment until we have the name
	else
		increment_counter(SPELLS, spell_names[spell_id+1], 1, spell_id);
}

void increment_summon_manu_counter()
{
	increment_counter(SUMMONS, product_name, product_count, 0);
}

void increment_summon_counter(char *string)
{
	if (strncmp(string, username_str, strlen(username_str))) {
		return;
	}

	string += strlen(username_str);

	if (strncmp(string, " summoned a ", 12)) {
		return;
	}

	string += 12;

	increment_counter(SUMMONS, string, 1, 0);
}


void reset_session_counters()
{
	int i, j;

	for (i = 0; i < NUM_COUNTERS; i++)
		if (counters[i])
			for (j = 0; j < entries[i]; ++j)
				counters[i][j].n_session=0;
}

/* checks text for breaks and harvest events */
void catch_counters_text(const char* text)
{
	size_t text_len = strlen(text);
	
	if (!counters_initialized)
		return;
	
	/* Your xxx ... */
	if (my_strncompare(text, "Your ", 5))
	{
		int i;
		char *mess_ends[] = {" has been destroyed", " broke, sorry!"};
		size_t to_count_name_len = 0;
		const char *item_string = &text[5];

		/* look for one of the endings, if found use it to locate the item name */
		for (i=0; i<sizeof(mess_ends)/sizeof(char *); i++)
		{
			char *located = strstr(item_string, mess_ends[i]);
			if (located)
			{
				to_count_name_len = (size_t)((located - item_string)/sizeof(char));
				break;
			}
		}

		/* if there was no match then its not a break string */
		if (!to_count_name_len)
			return;

		safe_strncpy2(to_count_name, item_string, sizeof(to_count_name), to_count_name_len);
		increment_counter(BREAKS, to_count_name, 1, 0);
	}
	
	/* "<user name> found a/an " */
	else if (my_strncompare(text, search_str[0], search_len[0]))
	{
		size_t start_from = search_len[0];
		size_t could_not_carry_index = get_string_occurance(". What a pity ", text, text_len, 1);
		if ((text_len > start_from) && (text[start_from] != ' '))
			start_from++; /* move past the n of an */
		start_from++; /* move past the space */
		
		/* some death messages match so crudely exclude them but catch bags of gold */
		if (strchr(&text[start_from], ',') != NULL)
		{
			char *gold_str = "bag of gold, getting ";
			size_t gold_len = strlen(gold_str);
			if (my_strncompare(&text[start_from], gold_str, gold_len))
			{
				int quanity = atoi(&text[start_from+gold_len]);
				increment_counter(MISC_EVENTS, "Total gold coin from bags", quanity, 0);
				increment_counter(MISC_EVENTS, "Bag of gold", 1, 0);
			}
		}

		/* check if we were not able to carry the found thing */
		else if (could_not_carry_index != -1)
		{
			size_t thing_len = could_not_carry_index - start_from;
			safe_strncpy2(to_count_name, &text[start_from], sizeof(to_count_name), thing_len);
			safe_strcat (to_count_name, " (lost)", sizeof(to_count_name));
			increment_counter(MISC_EVENTS, to_count_name, 1, 0);
		}
		
		/* otherwise, count the found thing */
		else
		{
			int to_count_len = text_len - start_from;
			safe_strncpy2(to_count_name, &text[start_from], sizeof(to_count_name), to_count_len);
			increment_counter(MISC_EVENTS, to_count_name, 1, 0);
		}
	}
	
	/* misc events, just translate the event text to a counted text string */
	else
	{
		int i;
		for (i=1; i<num_search_str; i++)
			if (my_strncompare(text, search_str[i], search_len[i]))
			{
				increment_counter(MISC_EVENTS, count_str[i], 1, 0);
				if (i==1)
				{
					int quanity = atoi(&text[search_len[i]]);
					increment_counter(MISC_EVENTS, "Exp from Queen of Nature blessing", quanity, 0);
				}
				/* record the event time so can check associate with a subsequent cause of death */
				misc_event_time = SDL_GetTicks();
			}
	}
}


/*
** Temporary command to scan chat_log.txt for break/misc event messages to
** populate the counters.  Should probably be removed the release after the
** these additional counters are included - assuming they will be:)
*/
int chat_to_counters_command(const char *text, int len)
{
#ifndef NEW_FILE_IO
	char chat_log_file[100];
#endif /* not NEW_FILE_IO */
	char line[1024];
	FILE *fp;
	struct Counter *old_counters[] = {NULL, NULL};
	int old_entries[] = {0, 0};
	size_t types[] = {BREAKS-1, MISC_EVENTS-1 };
	size_t type;
	
	/* get any parameter text */
	while(*text && !isspace(*text))
		text++;
	while(*text && isspace(*text))
		text++;

	/* if YES not specified, warn of consequences */
	if (strncmp(text, "YES", 3) != 0)
	{
		LOG_TO_CONSOLE(c_red2, "Scan chat_log.txt for break and miscellaneous event messages.");
		LOG_TO_CONSOLE(c_red2, "This may take some time and may cause lag.");
		LOG_TO_CONSOLE(c_red2, "Current break/event values will be reset.");
		LOG_TO_CONSOLE(c_red2, "Retype command and append YES to continue.");
		return 1;
	}

	/* save then reset any existing values */
	for (type=0; type<2; type++)
		if (counters[types[type]])
		{
			old_counters[type] = counters[types[type]];
			old_entries[type] = entries[types[type]];
			counters[types[type]] = NULL;
			entries[types[type]] = 0;
		}

#ifndef NEW_FILE_IO
#ifndef WINDOWS
	safe_snprintf (chat_log_file, sizeof (chat_log_file),  "%s/chat_log.txt", configdir);
#else
	strcpy (chat_log_file, "chat_log.txt");
#endif
	fp = my_fopen (chat_log_file, "r");
#else /* NEW_FILE_IO */
	fp = open_file_config ("chat_log.txt", "r");
#endif /* NEW_FILE_IO */

	/* consume the chat_log file, adding counter entries as they're found */
	while (!feof(fp))
	{
		if ((fgets(line, 1024, fp)) && (strlen(line) > 10))
		{
			if (line[strlen(line)-1] == '\n')
				line[strlen(line)-1] = '\0';
			if ((line[0] == '[') && (line[3] == ':') &&
					 (line[6] == ':') && (line[9] == ']'))
				catch_counters_text(line+11);
			else
				catch_counters_text(line);
		}
	}
	fclose(fp);

	/* restore the session totals and free the old memory */	
	for (type=0; type<2; type++)
	{
		int i,j;

		/* set the session totals to zero, then check if previously existed and restore if so */
		for (i = 0; i<entries[types[type]]; i++)
		{
			counters[types[type]][i].n_session = 0;
			if(old_counters[type])
				for (j = 0; j<old_entries[type]; j++)
					if ((old_counters[type][j].n_session > 0) &&
						(strcmp(old_counters[type][j].name, counters[types[type]][i].name) == 0))
						counters[types[type]][i].n_session = old_counters[type][j].n_session;
		}
		
		/* free any old memory */
		if(old_counters[type])
		{
			for (i = 0; i<old_entries[type]; i++)
				free(old_counters[type][i].name);
			free(old_counters[type]);
		}
	}
	
	return 1;
}

int is_death_message (const char * RawText)
{
	char *tmp;
	actor * act1,*act2;
	//check for kill message
	tmp = strstr (RawText,"You killed ");
	if(tmp)
	{
		tmp += strlen("You killed ");
		act1 = get_actor_ptr_from_id(yourself);
		act2 = get_actor_ptr_from_id(act1->last_target_id);
		if(act2 != 0)
		{
			increment_counter(KILLS, tmp, 1, (act2->is_enhanced_model && (act2->kind_of_actor == HUMAN || act2->kind_of_actor == PKABLE_HUMAN)));
		}
		return 1;
	}

	//check for death message
	tmp = strstr (RawText,"You got killed by ");
	if(tmp)
	{
		tmp += strlen("You got killed by ");
		increment_counter(DEATHS, tmp, 1, 1);
		return 1;
	}

	return 0;
}


#endif /* COUNTERS */
