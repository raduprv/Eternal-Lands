#include <stdlib.h>
#include <ctype.h>
#include "counters.h"
#include "context_menu.h"
#include "asc.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "manufacture.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "sound.h"
#include "spells.h"
#include "tabs.h"
#include "io/elpathwrapper.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "translate.h"

#define NUM_COUNTERS 14
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
	TAILORING,
	CRIT_FAILS
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
static int mouse_clicked = 0;
static int selected_entry = -1;
static int counters_show_win_help = 0;

static int product_count = 0;
static char product_name[128];
static char to_count_name[128];
static char *spell_names[128] = { NULL };
static int requested_spell_id = -1;

static const char *cat_str[NUM_COUNTERS] = { "Kills", "Deaths", "Harvests", "Alchemy", "Crafting", "Manufac.",
	"Potions", "Spells", "Summons", "Engineering", "Breakages", "Events", "Tailoring", "Crit Fails" };

static const char *temp_event_string[] =
	{	"%s found a", /* keep this one first in the list as its different from the rest*/
		"%s was blessed by the Queen of Nature with ", /* referenced laters as second in the list */
		"A cavern wall collapsed on %s",
		"Mother Nature got pissed off at %s",
		"%s was stung by a bee, losing ",
		"%s just hit a teleport nexus, ",
		"While harvesting, %s upset a radon pouch, ",
		"%s found Joker and got ",
		"%s found Joker and failed to get",
		"%s found 1 Pear.",
		"While harvesting, %s lit up a match to check the dung level, ",
		"While trying to harvest, the outhouse lid fell on %s, ",
		"You just got food poisoned!" };
static const char *count_str[] =
	{	"dummy",
		"Blessed by the Queen of Nature",
		"A cavern wall collapsed",
		"Mother Nature got pissed off",
		"Stung by a bee",
		"Hit a teleport nexus",
		"Upset a radon pouch",
		"Gift from Joker",
		"Gift from Joker (lost)",
		"Found a Pear",
		"Explosion while harvesting dung",
		"Outhouse lid accident",
		"Food poisoned" };
static const int num_search_str = sizeof(count_str)/sizeof(char *);
static char **search_str = NULL;
static size_t *search_len = NULL;
static Uint32 misc_event_time = 0;

int harvesting = 0;
Uint32 disconnect_time;
char harvest_name[32] = {0};
int killed_by_player = 0;
char last_spell_name[60] = {0};

int counters_win = -1;
int counters_scroll_id = 16;

void increment_counter(int counter_id, const char *name, int quantity, int extra);

int display_counters_handler(window_info *win);
int click_counters_handler(window_info *win, int mx, int my, Uint32 extra);
int mouseover_counters_handler(window_info *win, int mx, int my);

static size_t cm_counters = CM_INIT_VALUE;
static int cm_selected_entry = -1;
static int cm_selected_id = -1;
static int cm_entry_count = -1;
static int cm_floating_flag = 0;
unsigned int floating_counter_flags = 0;		/* persisted in el.cfg file */
int floating_session_counters = 0;				/* persisted in el.ini */

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

	safe_snprintf(filename, sizeof(filename), "counters_%s.dat", username);

	LOG_DEBUG("Open counters file '%s'", filename);

	return open_file_config(filename, mode);
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
	int fread_ok = 1;
	
	if (counters_initialized) {
		/*
		 * save eny existing counters before reloading
		 * this will take place when relogging after disconnection
		 */
		flush_counters();
		return;
	}

	ENTER_DEBUG_MARK("load counters");

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

		LEAVE_DEBUG_MARK("load counters");

		return;
	}

	while (fread(&io_counter_id, sizeof(io_counter_id), 1, f) > 0) {
		fread_ok = 0;
		if (fread(&io_name_len, sizeof(io_name_len), 1, f) != 1)
			break;
		if (fread(io_name, io_name_len, 1, f) != 1)
			break;
		io_name[io_name_len] = '\0';

		LOG_DEBUG("Reading counter '%s'", io_name);

		if (fread(&io_extra, sizeof(io_extra), 1, f) != 1)
			break;
		if (fread(&io_n_total, sizeof(io_n_total), 1, f) != 1)
			break;
		fread_ok = 1;

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

	if (!fread_ok)
		LOG_ERROR("%s error reading counters\n", __FUNCTION__);

	fclose(f);
	
	counters_initialized = 1;

	LEAVE_DEBUG_MARK("load counters");
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

	ENTER_DEBUG_MARK("flush counters");

	for (i = 0; i < NUM_COUNTERS; i++) {
		io_counter_id = i + 1;

		for (j = 0; j < entries[i]; j++) {
			io_name_len = strlen(counters[i][j].name);

			LOG_DEBUG("Writing counter '%s'",
				counters[i][j].name);
			
			fwrite(&io_counter_id, sizeof(io_counter_id), 1, f);
			fwrite(&io_name_len, sizeof(io_name_len), 1, f);
			fwrite(counters[i][j].name, io_name_len, 1, f);
			fwrite(&counters[i][j].extra, sizeof(counters[i][j].extra), 1, f);
			fwrite(&counters[i][j].n_total, sizeof(counters[i][j].n_total), 1, f);
		}
	}

	fclose(f);

	LEAVE_DEBUG_MARK("flush counters");
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

static void increment_product_counter(int counter_id, const char *name, int quantity, int extra)
{
	increment_counter(counter_id, name, quantity, extra);
	/* make sure the name/quanity is not used again */
	counters_set_product_info("",0);
}

void increment_counter(int counter_id, const char *name, int quantity, int extra)
{
	int i, j;
	int new_entry = 1;

	//printf("%s: counter_id=%d name=[%s] quantity=%d extra=%d\n", __FUNCTION__, counter_id, name, quantity, extra);

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
		last_selected_counter_id = -1;  /* force recalculation of the scrollbar */
		counters[i] = realloc(counters[i], entries[i] * sizeof(struct Counter));
		counters[i][j].name = strdup(name);
		counters[i][j].n_session = quantity;
		counters[i][j].n_total = quantity;
		counters[i][j].extra = extra;
	}

	if (floating_session_counters && (floating_counter_flags & (1 << i)))
	{
		char str[128];
		safe_snprintf(str, sizeof(str), "%s: %d", name, counters[i][j].n_session);
		add_floating_message(yourself, str, FLOATINGMESSAGE_NORTH, 0.3, 0.3, 1.0, 1500);
	}

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

static void cm_counters_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	// get the counter id and entry indices
	cm_selected_id = multiselect_get_selected(counters_win, multiselect_id);
	cm_entry_count = entries[cm_selected_id];
	cm_selected_entry = (my - 30) / 16;
	if (counters_scroll_id != -1)
		cm_selected_entry += vscrollbar_get_pos(counters_win, counters_scroll_id);

	// if not a valid entry, the menu options are greyed out
	{
		int is_grey = ((cm_selected_entry <0) || (cm_selected_entry >= entries[cm_selected_id]));
		cm_grey_line(cm_counters, 0, is_grey);
		cm_grey_line(cm_counters, 2, is_grey);
	}

	// set the control var from floating flags
	cm_floating_flag = (floating_counter_flags & (1 << cm_selected_id)) ?1 :0;
}


//	Prints the entries (total and name) in the specified category.
//	If just_session is true, then session infomation is also
//	included but only non-zero entries are included.
//
static void print_category(size_t cat, int just_session)
{
	int i;
	char buf[256];

	if (cat >= NUM_COUNTERS)
		return;

	if (just_session)
	{
		int has_session = 0;
		for (i = 0; i < entries[cat]; i++)
			if (counters[cat][i].n_session > 0)
			{
				has_session = 1;
				break;
			}
		if (!has_session)
			return;
	}

	safe_snprintf(buf, sizeof(buf), "%s:", cat_str[cat]);
	LOG_TO_CONSOLE(c_green2, buf);

	for (i = 0; i < entries[cat]; i++)
	{
		if (just_session)
		{
			if (counters[cat][i].n_session <= 0)
				continue;
			safe_snprintf(buf, sizeof(buf), "%12d %12d  %s",
				counters[cat][i].n_session, counters[cat][i].n_total, counters[cat][i].name);
		}
		else
			safe_snprintf(buf, sizeof(buf), "%12d  %s", counters[cat][i].n_total, counters[cat][i].name);
		LOG_TO_CONSOLE(c_grey1,buf);
	}
}


// The #session_counters command and menu option
//
void print_session_counters(const char *category)
{
	int i;
	if ((category == NULL) || !(*category))
		for (i = 0; i < NUM_COUNTERS; i++)
			print_category(i, 1);
	else
	{
		for (i = 0; i < NUM_COUNTERS; i++)
			if (strncasecmp(category, cat_str[i], strlen(category)) == 0)
			{
				print_category(i, 1);
				break;
			}
	}
}


static int cm_counters_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	struct Counter *the_entry = NULL;
	
	// if the number of entries has changed, we could be about to use the wrong entry, don't use it
	// if a entry index is invalid, don't use it
	if ((cm_entry_count == entries[cm_selected_id]) &&
		(cm_selected_entry >= 0) && (cm_selected_entry < entries[cm_selected_id]))
		the_entry = &counters[cm_selected_id][cm_selected_entry];

	switch (option)
	{
		case 0:		// delete entry
			if (the_entry != NULL)
			{
				int i;
				if (the_entry->name != NULL)
					free(the_entry->name);	
				// move the entries up, replacing the deleted one
				for (i=cm_selected_entry+1; i<entries[cm_selected_id]; i++)
				{
					the_entry->name = (the_entry+1)->name;
					the_entry->n_session = (the_entry+1)->n_session;
					the_entry->n_total = (the_entry+1)->n_total;
					the_entry->extra = (the_entry+1)->extra;
					the_entry++;
				}
				entries[cm_selected_id]--;
			}
			else
				return 0;
			break;

		case 2:		// reset session total for entry
			if (the_entry != NULL)
				the_entry->n_session = 0;
			else
				return 0;
			break;

		case 4:		// set the floating flag from the control var
			{
				int flagbit = multiselect_get_selected(counters_win, multiselect_id);
				if (cm_floating_flag)
				{
					floating_counter_flags |= 1 << flagbit;
					if (!floating_session_counters)
						toggle_OPT_BOOL_by_name("floating_session_counters");
				}
				else
					floating_counter_flags &= ~(1 << flagbit);
			}
			break;

		case 6:		// print the category to console
			print_category(cm_selected_id, 0);
			break;

		case 7:		// print all categories to console
			{
				int i;
				for (i = 0; i < NUM_COUNTERS; i++)
					print_category(i, 0);
			}
			break;

		case 8:		// print session information to console
			print_session_counters(NULL);
			break;

		default:
			return 0;
	}

	return 1;
}

void fill_counters_win()
{
	int idx = selected_counter_id > 0 ? selected_counter_id-1 : 0;
	
	set_window_handler(counters_win, ELW_HANDLER_DISPLAY, &display_counters_handler);
	set_window_handler(counters_win, ELW_HANDLER_CLICK, &click_counters_handler);
	set_window_handler(counters_win, ELW_HANDLER_MOUSEOVER, &mouseover_counters_handler);

	multiselect_id = multiselect_add(counters_win, NULL, 8, 2, 104);
	multiselect_button_add(counters_win, multiselect_id, 0, 0, cat_str[0], 1);
	multiselect_button_add(counters_win, multiselect_id, 0, 25, cat_str[1], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 125, cat_str[2], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 150, cat_str[3], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 175, cat_str[4], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 200, cat_str[5], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 225, cat_str[6], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 250, cat_str[7], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 275, cat_str[8], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 300, cat_str[9], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 50, cat_str[10], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 100, cat_str[11], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 325, cat_str[12], 0);
	multiselect_button_add(counters_win, multiselect_id, 0, 75, cat_str[13], 0);

	counters_scroll_id = vscrollbar_add_extended(counters_win,
			counters_scroll_id, NULL,
			STATS_TAB_WIDTH - 20, 25, 20,
			STATS_TAB_HEIGHT - 50, 0,
			1.0f, 0.77f, 0.57f, 0.39f,
			0, 1, MAX(0, entries[idx] - NUM_LINES));

	if (cm_counters == CM_INIT_VALUE)
	{
		cm_counters = cm_create(cm_counters_menu_str, cm_counters_handler);
		cm_add_region(cm_counters, counters_win, 120, 30, STATS_TAB_WIDTH-140, NUM_LINES*16);
		cm_set_pre_show_handler(cm_counters, cm_counters_pre_show_handler);
		cm_bool_line(cm_counters, 4, &cm_floating_flag, NULL);
	}
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
		selected_entry = -1;
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

	if (cm_window_shown() != cm_counters)
		cm_selected_entry = -1;
	
	for (j = scroll, n = 0, y = 30; j < entries[i]; j++, n++) {
		int mouse_over_this_entry = ((mouseover_entry_y >= y) && (mouseover_entry_y < y+16));

		if (n == NUM_LINES)
			break;

		if (mouse_over_this_entry && mouse_clicked) {
			selected_entry = j;
			mouse_clicked = 0;
		}

		if (cm_selected_entry == j)
			glColor3f(0.77f, 0.57f, 0.39f);
		else if ((selected_counter_id == KILLS || selected_counter_id == DEATHS) && counters[i][j].extra)
			glColor3f(0.8f, 0.2f, 0.2f);
		else if (selected_entry == j)
			elglColourN("global.mouseselected");
		else if (mouse_over_this_entry)
			elglColourN("global.mousehighlight");
		else
			glColor3f(1.0f, 1.0f, 1.0f);

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
				const char *append_str = "... ";
				size_t dest_max_len = strlen(counters[i][j].name) + strlen(append_str) + 1;
				char *used_name = (char *)malloc(dest_max_len);
				truncated_string(used_name, counters[i][j].name, dest_max_len, append_str, max_name_x, font_ratio);
				draw_string_small(x, y, (unsigned char*)used_name, 1);
				/* if the mouse is over this line and its truncated, tooltip to full name */
				if (mouseover_entry_y >= y && mouseover_entry_y < y+16) {
					show_help(counters[i][j].name, -TAB_MARGIN, win->len_y+10+TAB_MARGIN);
					counters_show_win_help = 0;
				}
				free(used_name);
			}
			else
				draw_string_small(x, y, (unsigned char*)counters[i][j].name, 1);
		}
		y += 16;
	}

	if (counters_show_win_help) {
		show_help(cm_help_options_str, -TAB_MARGIN, win->len_y+10+TAB_MARGIN);
		counters_show_win_help = 0;
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

		mouse_clicked = 1;
		selected_entry = -1;
		do_click_sound();

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
		selected_entry = -1;
	}

	return 1;
}

int mouseover_counters_handler(window_info *win, int mx, int my)
{
	mouseover_name = mouseover_session = mouseover_total = 0;
	mouseover_entry_y = -1;

	if ((mx > 120) && (mx < (STATS_TAB_WIDTH-20)) && (my > 30) && (my < (30+NUM_LINES*16)))
		counters_show_win_help = 1;

	if (my > 25){
		if (my > 30 && my < (30+NUM_LINES*16) && mx >= 130 && mx <= 540) {
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
	
	/* strip trailing spaces */
	while (i>0 && buf[i-1] == ' ')
		i--;

	buf[i] = '\0';

	return buf;
}

/*
 * Called by increment_death_counter if it wasn't our character that died.
 */
static void increment_kill_counter(actor *me, actor *them)
{
	int x1, y1, x2, y2;
	static int face_offsets[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};
	
	if (!them->async_fighting || !me->async_fighting) {
		return;
	}

	/* get the coords of the tile we're facing */
	x1 = me->async_x_tile_pos + face_offsets[((int)me->async_z_rot)/45][0];
	y1 = me->async_y_tile_pos + face_offsets[((int)me->async_z_rot)/45][1];

	/* get the coords of the next tile we're facing (necessary for Chimerans) */
	x2 = x1 + face_offsets[((int)me->async_z_rot)/45][0];
	y2 = y1 + face_offsets[((int)me->async_z_rot)/45][1];

	/* return if this actor is not on one of these tiles */
	if ((them->async_x_tile_pos != x1 || them->async_y_tile_pos != y1) && (them->async_x_tile_pos != x2 || them->async_y_tile_pos != y1) && (them->async_x_tile_pos != x1 || them->async_y_tile_pos != y2) && (them->async_x_tile_pos != x2 || them->async_y_tile_pos != y2)) {
		return;
	}
	
	/* Now, we should always have non players actors here */
/* 	increment_counter(KILLS, strip_actor_name(them->actor_name), 1,	(them->is_enhanced_model && (them->kind_of_actor == HUMAN || them->kind_of_actor == PKABLE_HUMAN))); */
	increment_counter(KILLS, strip_actor_name(them->actor_name), 1,	0);
}

void increment_range_kill_counter(actor *me, actor *them)
{
	if (them->last_range_attacker_id == me->actor_id)
		increment_counter(KILLS, strip_actor_name(them->actor_name), 1,	0);
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
	
	if (a == me) {
		/* If we have intercepted a message from the server that telling us that
		 * we have been killed by someone, the counter has already been incremented */
		if (killed_by_player) {
			killed_by_player = 0;
			return;
		}

		/* count deaths that happend while harvesting, may not have been due to harvest event though */
		if (harvesting) {
			/* a crude check to see if death was just after (1 second should be enough) a harvest event */
			if (abs(SDL_GetTicks() - misc_event_time) < 1000) {
				increment_counter(DEATHS, "Harvesting event", 1, 0);
				found_death_reason = 1;
			}

			/* if you are killed while harvesting, there is no "you stopped harvesting" message */
			/* resetting now prevents next items added to inventry getting added to harvest counter */
			harvesting = 0;
		}

		if (!found_death_reason && me->async_fighting) {
			int x1, y1, x2, y2;
			int i;
			actor *them;
			static int face_offsets[8][2] = {{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}};

			for (i = 0; i < max_actors; i++) {
				them = actors_list[i];
				
				if (!them->async_fighting ||
					// PK deaths are handled with text messages from the server now
					(them->is_enhanced_model && (them->kind_of_actor == HUMAN ||
												 them->kind_of_actor == PKABLE_HUMAN))) {
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
				
				increment_counter(DEATHS, strip_actor_name(them->actor_name), 1, 0);
				found_death_reason = 1;
			}
		}
		
		if (!found_death_reason) {
			/* count deaths while we were poisoned - possibily in adition to another possible reason */
			if (we_are_poisoned()) {
				increment_counter(DEATHS, "While poisoned", 1, 0);
				found_death_reason = 1;
			}
			else if (me->async_fighting) {
				increment_counter(DEATHS, "While fighting unknown opponent", 1, 0);
			}
			else {
				/* if we don't die while harvesting, fighting or poisoned, the cause is unknow */
				increment_counter(DEATHS, "Unknown cause", 1, 0);
			}
		}
	}
	else { // a != me
		/* if the dead actor is a player, we don't have to check it because if
		 * we've killed him, we already catched it from a server message */
		if (a->is_enhanced_model && (a->kind_of_actor == HUMAN ||
									 a->kind_of_actor == PKABLE_HUMAN)) return;

		if (a->last_range_attacker_id < 0)
			increment_kill_counter(me, a);
		else
			increment_range_kill_counter(me, a);
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
	increment_product_counter(ALCHEMY, product_name, product_count, 0);
}

void increment_crafting_counter()
{
	increment_product_counter(CRAFTING, product_name, product_count, 0);
}

void increment_engineering_counter()
{
	increment_product_counter(ENGINEERING, product_name, product_count, 0);
}

void increment_tailoring_counter()
{
	increment_product_counter(TAILORING, product_name, product_count, 0);
}

void increment_potions_counter()
{
	increment_product_counter(POTIONS, product_name, product_count, 0);
}

void increment_manufacturing_counter()
{
	increment_product_counter(MANUFACTURING, product_name, product_count, 0);
}

void increment_critfail_counter(char *name)
{
	increment_counter(CRIT_FAILS, name, 1, 0);
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
			safe_strncpy2(last_spell_name, spell_names[spell_id+1], 60, strlen(spell_names[spell_id+1]));
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
	{
		safe_strncpy2(last_spell_name, spell_names[spell_id+1], 60, strlen(spell_names[spell_id+1]));
		increment_counter(SPELLS, spell_names[spell_id+1], 1, spell_id);
	}
}

void increment_summon_manu_counter()
{
	increment_product_counter(SUMMONS, product_name, product_count, 0);
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
	check_for_recipe_name(string);
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
	
	//printf("%s: [%s]\n", __FUNCTION__, text);
	
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
	
	/* "<user name> found a/an/a[n] " */
	else if (my_strncompare(text, search_str[0], search_len[0]))
	{
		size_t start_from = search_len[0];
		size_t could_not_carry_index = get_string_occurance(". What a pity ", text, text_len, 1);
		while ((text_len > start_from) && (text[start_from] != ' '))
			start_from++; /* move past the a/an/a[n] */
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
	
	/* loose coin find */
	else if (my_strncompare(text, "You found ", 10) && strstr(text, " coins."))
	{
		int quantity = atoi(&text[10]);
		increment_counter(MISC_EVENTS, "Total loose gold coin", quantity, 0);
		increment_counter(MISC_EVENTS, "Loose gold coin", 1, 0);
	}
	
	/* extra harvest exp */
	else if (my_strncompare(text, "You gained ", 11) && strstr(text, " extra harvesting exp."))
	{
		int quantity = atoi(&text[11]);
		increment_counter(MISC_EVENTS, "Total extra harvesting exp", quantity, 0);
		increment_counter(MISC_EVENTS, "Extra harvesting exp", 1, 0);
	}

	/* you hurt yourself */
        else if (my_strncompare(text, "You hurt yourself, and lost ", 28) && strstr(text, " HPs."))
	{
		increment_counter(MISC_EVENTS, "You hurt yourself", 1, 0);
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

	fp = open_file_config ("chat_log.txt", "r");

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
	if (!strncmp(RawText, "You killed ", 11)) {
		int i = 11;
		for (; i<strlen(RawText); i++)
			if (*(RawText+i) == ' ')
				return 0; // kills should just have the name, don't use invalid messages.
		increment_counter(KILLS, RawText+11, 1, 1);
		return 1;
	}
	else if (!strncmp(RawText, "You were killed by ", 19)) {
		killed_by_player = 1;
		increment_counter(DEATHS, RawText+19, 1, 1);
		return 1;
	}
	return 0;
}

