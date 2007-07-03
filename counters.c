#ifdef COUNTERS

#include <ctype.h>
#include "global.h"
#include "elwindows.h"

#define NUM_COUNTERS 10
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
	ENGINEERING
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

static int product_count = 0;
static char product_name[128];
static char *spell_names[128] = { NULL };
static int requested_spell_id = -1;

int harvesting = 0;
Uint32 disconnect_time;
char harvest_name[32];

int counters_win = -1;
int counters_scroll_id = 16;

void increment_counter(int counter_id, char *name, int quantity, int extra);

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

	harvesting = 0;
	counters_initialized = 0;
}

void increment_counter(int counter_id, char *name, int quantity, int extra)
{
	int i, j;

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

		sort_counter(counter_id);
		return;
	}

	/* Create a new entry. */
	j = entries[i]++;
	counters[i] = realloc(counters[i], entries[i] * sizeof(struct Counter));
	counters[i][j].name = strdup(name);
	counters[i][j].n_session = quantity;
	counters[i][j].n_total = quantity;
	counters[i][j].extra = extra;

	sort_counter(counter_id);
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
	multiselect_button_add(counters_win, multiselect_id, 0, 50, "Harvests", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 75, "Alchemy", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 100, "Crafting", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 124, "Manufac.", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 150, "Potions", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 175, "Spells", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 200, "Summons", 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 225, "Engineering", 0);

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
		
		if (counters[i][j].name) {
			draw_string_small(x, y, (unsigned char*)counters[i][j].name, 1);
		}
		safe_snprintf(buffer, sizeof(buffer), "%12d", counters[i][j].n_session);
		draw_string_small(x + 200, y, (unsigned char*)buffer, 1);
		safe_snprintf(buffer, sizeof(buffer), "%12d", counters[i][j].n_total);
		draw_string_small(x + 314, y, (unsigned char*)buffer, 1);
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

	if (my > 25) {
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
char *strip_actor_name(char *actor_name)
{
	static char buf[32];
	int i;

	/* strip a leading color code */
	if ((unsigned char)actor_name[0] >= 127) {
		actor_name++;
	}
	
	/* copy the name minus the guild tag */
	for (i = 0; actor_name[i] && i < sizeof(buf); i++) {
		if ((unsigned char)actor_name[i] >= 127) {
			i--;
			break;
		}
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
	actor *me = get_actor_ptr_from_id(yourself);
	
	if (!me || !me->async_fighting) {
		return;
	}
	
	if (a != me) {
		increment_kill_counter(me, a);
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
		}
	}
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

#endif /* COUNTERS */
