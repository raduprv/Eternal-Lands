#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdlib.h>

#ifndef BSD
	#ifdef OSX
		#include <sys/malloc.h>
	#else
		#include <malloc.h>
	#endif
#endif

#include "rules.h"
#include "asc.h"
#include "consolewin.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "events.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "loginwin.h"
#include "mapwin.h"
#include "multiplayer.h"
#include "new_character.h"
#include "openingwin.h"
#include "tabs.h"
#include "translate.h"

typedef enum
{
	END = -1,
	TITLE,
	RULE,
	INFO
} rule_type;

#define INTERFACE_GAME 0
#define INTERFACE_LOG_IN 1
#define INTERFACE_NEW_CHAR 2
#define INTERFACE_CONSOLE 3
#define INTERFACE_OPENING 4
#define INTERFACE_MAP 5
#define INTERFACE_CONT 6
#define INTERFACE_RULES 7

/*!
 * holds the data for up to 40 rules.
 */
struct rules_struct {
	int no; /*!< the rule no (0-39) */
    /*!
     * inner struct containing the data for a single rule
     */
	struct rule_struct {
		char* short_desc; /*!< a short description for this rule */
		char* long_desc;  /*!< the complete description for this rule */
		rule_type type; /*!< the type of the rule */
	} rule[40];
};


/*!
 * a conversion of a \see rule_struct to be displayed in-game and used by, for example, moderators
 */
typedef struct {
	int type; /*!< type of the rule */
	int show_long_desc; /*!< flag whether to show the long description of the rule or not */
	int mouseover; /*!< mouseover */
	int highlight;/*!< flag indicating whether the rule should be highlighted or not (Used for moderators) */
    /*!
     * \name coordinates where the rule should appear
     */
    /* \{ */
	int x_start;
	int y_start;
	int x_end;
	int y_end;
    /* \} */
	unsigned char* short_str; /*!< the short description of the rule */
	int short_str_nr_lines;   /*!< The number of lines in the long description */
	int short_str_nr_indent;  /*!< Indentation in pixels for the rule number */
	int short_str_width;      /*!< Width of the short description, in pixels */
	unsigned char* long_str;  /*!< the long description of the rule */
	int long_str_nr_lines;    /*!< The number of lines in the long description */
	int y_virt;               /*!< virtual window offset of rule start */
} rule_string;


/*Window*/
static int rules_win=-1;
static int rules_scroll_id = 0;

/*Shared*/
static int reached_end=0;
static int read_all_rules=0;
int have_rules=0;
static rule_string * display_rules=NULL;
int last_display=-1;

/* virtual window */
static int virt_win_len = 0;			/* length in pixels of all rules/info and current expanded rules */
static int virt_win_offset = 0;		/* virtual line number at top of displayed text */
static int recalc_virt_win_len = 1;	/* if true, force a virtual window recal before display */
static int set_rule_offset = -1;		/* if > 0, set the first displayed rule to this */

/*Interface*/
int countdown = 0;

static int rules_root_scroll_id = 0;
static int rules_root_accept_id = 0;

static int next_win_id;

/* Colors */
static const float rules_winRGB[8][3] = {{1.0f,0.6f,0.0f},{1.0f,0.0f,0.0f},{0.0f,0.7f,1.0f},{0.9f,0.9f,0.9f},{0.6f,1.0f,1.2f},{0.4f,0.8f,1.0f},{0.8f,0.0f,0.0f},{1.0f,1.0f,1.0f}};

/* Rule parser */
static struct rules_struct rules = {0, {{NULL, NULL, 0}}};

static void free_rules(rule_string * d);
static rule_string *get_interface_rules(int width, float zoom);
static int draw_rules(rule_string * rules_ptr, int x_in, int y_in, int lenx, int leny, float text_size, const float rgb[8][3]);
static int rules_root_scroll_handler();

static void add_rule(const char* short_desc, const char* long_desc, int type)
{
	struct rule_struct* rule = &rules.rule[rules.no++];

	MY_XMLSTRCPY(&rule->short_desc, short_desc);
	MY_XMLSTRCPY(&rule->long_desc, long_desc);

	rule->type=type;
}

static char * get_id_str(xmlNode * node, xmlChar *id)
{
	xmlNode * cur;
	for(cur=node;cur;cur=cur->next) {
		if(cur->type == XML_ELEMENT_NODE && cur->children){
			if(!xmlStrcasecmp(cur->name,id)) return (char*)cur->children->content;
		}
	}

	return NULL;
}

static int parse_rules(xmlNode * root)
{
	xmlNode * cur;

	if(!root) return 0;

	for(cur=root;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if(cur->children){
				if(!xmlStrcasecmp(cur->name, (xmlChar*)"title")) {
					add_rule((char*)cur->children->content, NULL, TITLE);
				} else if(!xmlStrcasecmp(cur->name, (xmlChar*)"rule")) {
					add_rule(get_id_str(cur->children, (xmlChar*)"short"), get_id_str(cur->children, (xmlChar*)"long"), RULE);
				} else if(!xmlStrcasecmp(cur->name, (xmlChar*)"info")) {
					add_rule(get_id_str(cur->children, (xmlChar*)"short"), get_id_str(cur->children, (xmlChar*)"long"), INFO);
				}
			}
		}
	}

	return 1;
}

int read_rules(void)
{
	char file_name[120];
	xmlDoc * doc;
	xmlNode * root;
	safe_snprintf(file_name, sizeof(file_name), "languages/%s/rules.xml",lang);

	if ((doc = xmlReadFile(file_name, NULL, 0)) == NULL) {
		if((doc=xmlReadFile("languages/en/rules.xml",NULL,0))==NULL){
			//report this error:
			LOG_ERROR(read_rules_str);
			return 0;
		}
	}

	if ((root = xmlDocGetRootElement(doc))==NULL) {
		LOG_ERROR(read_rules_str);
	} else if(!parse_rules(root->children)){
		LOG_ERROR(parse_rules_str);
	}

	xmlFreeDoc(doc);

	return (rules.no>0);
}

/*Rules window interface*/

static int rules_scroll_handler (void)
{
	virt_win_offset = vscrollbar_get_pos (rules_win, rules_scroll_id);
	return 1;
}

static int click_rules_handler (window_info *win, int mx, int my, Uint32 flags)
{
	rule_string *rules_ptr = display_rules;
	int i;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(win->window_id, rules_scroll_id);
		rules_scroll_handler();
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(win->window_id, rules_scroll_id);
		rules_scroll_handler();
		return 1;
	} else {
		for (i = 0; rules_ptr[i].type != END && rules_ptr[i].y_start < win->len_y; i++)
		{
			if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end)
			{
				rules_ptr[i].show_long_desc = !rules_ptr[i].show_long_desc;
				recalc_virt_win_len = 1;
				return 1;
			}
		}
	}
	return 0;
}

static int mouseover_rules_handler (window_info *win, int mx, int my)
{
	rule_string *rules_ptr = display_rules;
	int i;

	for (i = 0; rules_ptr[i].type != END && rules_ptr[i].y_start < win->len_y; i++)
	{
		if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end)
		{
			if(rules_ptr[i].long_str)
				rules_ptr[i].mouseover = 1;
		}
		else
		{
			rules_ptr[i].mouseover=0;
		}
	}

	return 1;
}

static int display_rules_handler(window_info *win)
{
	if (virt_win_offset < 0) virt_win_offset=0;
	draw_rules(display_rules, 0, win->small_font_len_y*0.8, win->len_x, win->len_y-win->small_font_len_y*0.8, win->current_scale*0.8f, rules_winRGB);
	return 1;
}

static int resize_rules_handler(window_info *win, int new_width, int new_height)
{
	widget_resize(win->window_id, rules_scroll_id, win->box_size, win->len_y);
	widget_move(win->window_id, rules_scroll_id, win->len_x - win->box_size, 0);
	vscrollbar_set_pos(win->window_id, rules_scroll_id, 0);

	if(display_rules)free_rules(display_rules);
	display_rules = get_interface_rules(win->len_x - 70 * win->current_scale,
		win->current_scale * 0.8f);

	return 0;
}

static int change_rules_font_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	resize_window(win->window_id, win->len_x, win->len_y);
	return 1;
}

void fill_rules_window(int window_id)
{
	rules_win = window_id;
	set_window_custom_scale(window_id, MW_HELP);
	set_window_font_category(window_id, RULES_FONT);

	rules_scroll_id = vscrollbar_add_extended (window_id, rules_scroll_id, NULL,
		0, 0, 0, 0, 0, 1.0, 0, 3, rules.no-1);

	widget_set_OnClick (window_id, rules_scroll_id, rules_scroll_handler);
	widget_set_OnDrag (window_id, rules_scroll_id, rules_scroll_handler);

	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_rules_handler);
	set_window_handler(window_id, ELW_HANDLER_MOUSEOVER, &mouseover_rules_handler);
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_rules_handler);
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_rules_handler);
	set_window_handler(window_id, ELW_HANDLER_FONT_CHANGE, &change_rules_font_handler);
}

static void toggle_rules_window()
{
	// Stop the window from closing if already open
	if (!get_show_window_MW(MW_HELP) || tab_collection_get_tab (get_id_MW(MW_HELP), tab_help_collection_id) != HELP_TAB_RULES) {
		view_tab(MW_HELP, tab_help_collection_id, HELP_TAB_RULES);
	}

	last_display=1;
}

/*Generic code*/

void cleanup_rules()
{
	int i;

	if(display_rules){
		free_rules(display_rules);
		display_rules=NULL;
	}

	for(i=0;i<rules.no;i++){
		if(rules.rule[i].short_desc) {
			free(rules.rule[i].short_desc);
			rules.rule[i].short_desc=NULL;
		}
		if(rules.rule[i].long_desc) {
			free(rules.rule[i].long_desc);
			rules.rule[i].long_desc=NULL;
		}
	}

	rules.no=0;
}

static void free_rules(rule_string * d)
{
	if (d)
	{
		int i;
		for (i = 0; d[i].type != END; ++i)
		{
			free(d[i].short_str);
			free(d[i].long_str);
		}
		free(d);
	}
}

static void reset_rules(rule_string * r)
{
	int i;
	for (i = 0; r[i].type != END; ++i)
	{
		r[i].show_long_desc=0;
		r[i].x_start=0;
		r[i].y_start=0;
		r[i].x_end=0;
		r[i].y_end=0;
		r[i].highlight=0;
		r[i].mouseover=0;
	}
	virt_win_offset = 0;
	recalc_virt_win_len = 1;
	set_rule_offset = -1;
}

void highlight_rule (int type, const Uint8 *rule, int no)
{
	int i,j;
	int r;
	int cur_rule;
	const Uint8 *rule_start;

	if(type==RULE_WIN)
	{
		toggle_rules_window();
		rule_start = rule;
	}
	else if(type==RULE_INTERFACE)
	{
		switch (rule[2])
		{
			case INTERFACE_LOG_IN:
				if (login_root_win < 0)
					create_login_root_window (window_width, window_height);
				next_win_id = login_root_win;
				break;
			case INTERFACE_NEW_CHAR:
				if (newchar_root_win < 0)
					create_newchar_root_window ();
				next_win_id = newchar_root_win;
				break;
			case INTERFACE_CONSOLE:
				if (get_id_MW(MW_CONSOLE) < 0)
					create_console_root_window (window_width, window_height);
				next_win_id = get_id_MW(MW_CONSOLE);
				break;
			case INTERFACE_OPENING:
				if (opening_root_win < 0)
					create_opening_root_window (window_width, window_height);
				next_win_id = opening_root_win;
				break;
			case INTERFACE_MAP:
			case INTERFACE_CONT:
				if (get_id_MW(MW_TABMAP) < 0)
					create_map_root_window (window_width, window_height);
				next_win_id = get_id_MW(MW_TABMAP);
				break;
			case INTERFACE_RULES:
				// doesn't make sense, use the default
			case INTERFACE_GAME:
			default:
				if (game_root_win < 0)
					create_game_root_window (window_width, window_height);
				next_win_id = game_root_win; break;
		}

		create_rules_root_window ( window_width, window_height, next_win_id, SDL_SwapLE16(*((Uint16*)(rule))) );
		hide_all_root_windows ();
		hide_hud_windows ();
		show_window (rules_root_win);

		rule_start = &rule[3];
		no-=3;
	} else return; //Hmm...

	if(display_rules) {
		reset_rules(display_rules);

		for(i=0;i<no;i++){
			cur_rule = rule_start[i];
			r=0;

			for(j=0;display_rules[j].type!=END;j++){
				if(display_rules[j].type==RULE)r++;
				if(r==cur_rule) break;
			}

			if(display_rules[j].type!=END){
				display_rules[j].highlight=1;
				display_rules[j].show_long_desc=1;
				recalc_virt_win_len = 1;
			}
		}

		for(i=0;display_rules[i].type!=END;i++)
			if(display_rules[i].type == RULE && display_rules[i].highlight) {
				set_rule_offset = i;	//Get the first highlighted entry
				recalc_virt_win_len = 1;
				return;
			}
	}
}

static void set_short_desc(const char* desc, int nr, int width, float zoom, rule_string *rule)
{
	static const size_t max_nr_lines = 5;

	rule->short_str = NULL;
	rule->short_str_nr_lines = 0;
	rule->short_str_nr_indent = 0;
	if (desc)
	{
		int nr_lines;
		size_t size = strlen(desc) + max_nr_lines;

		unsigned char *buf = calloc(size, 1);
		if (!buf)
			return;

		if (nr)
		{
			safe_snprintf((char*)buf, size, "%d: ", nr);
			rule->short_str_nr_indent = get_string_width_zoom(buf, RULES_FONT, zoom);
		}

		safe_strcat((char*)buf, desc, size);
		nr_lines = reset_soft_breaks(buf, strlen((const char*)buf), size, RULES_FONT,
			zoom, width, NULL, NULL);
		if (nr && nr_lines > 0)
		{
			size_t eol = strcspn((const char*)buf, "\r\n");
			if (buf[eol])
			{
				width -= rule->short_str_nr_indent;
				nr_lines = 1 + reset_soft_breaks(buf+eol+1, strlen((const char*)buf+eol+1),
					size-eol-1, RULES_FONT, zoom, width, NULL, NULL);
			}
		}

		rule->short_str = buf;
		rule->short_str_nr_lines = nr_lines;
	}
}

static void set_long_desc(const char* desc, int width, float zoom, rule_string *rule)
{
	static const size_t max_nr_lines = 50;

	rule->long_str = NULL;
	rule->long_str_nr_lines = 0;
	if (desc)
	{
		int nr_lines;
		size_t len = strlen(desc);
		size_t size = len + max_nr_lines;
		unsigned char *buf = malloc(size);
		if (!buf)
			return;

		safe_strncpy((char*)buf, desc, size);
		nr_lines = reset_soft_breaks(buf, len, size, RULES_FONT, zoom, width, NULL, NULL);

		rule->long_str = buf;
		rule->long_str_nr_lines = nr_lines;
	}
}

#define SHORT_RULE_INDENT 20
static const int short_rule_indent = SHORT_RULE_INDENT;
static const int long_rule_indent = 2 * SHORT_RULE_INDENT;

static rule_string *get_interface_rules(int width, float zoom)
{
	int i, cur_rule_nr = 0;
	rule_string *_rules = calloc(rules.no + 1, sizeof(rule_string));

	for (i = 0; i < rules.no; ++i)
	{
		const struct rule_struct *rule = &rules.rule[i];
		rule_string *rs = &_rules[i];
		int nr = rule->type == RULE ? ++cur_rule_nr : 0;
		rs->type = rule->type;
		set_short_desc(rule->short_desc, nr, width - short_rule_indent * zoom, zoom, rs);
		set_long_desc(rule->long_desc, width - long_rule_indent * zoom, zoom, rs);
		if (rs->short_str)
		{
			size_t len = strlen((const char*)rs->short_str);
			int str_width, str_height;
			get_buf_dimensions(rs->short_str, len, RULES_FONT, zoom,
				&str_width, &str_height);
			rs->short_str_width = str_width;
		}
	}

	_rules[rules.no].type=END;

	reset_rules(_rules);

	return _rules;
}

/*	Originally, the rules window scrolled by stepping the rule number
	displayed at the top of the window. This did not allow you to scroll
	through a long rule. This change switches to scrolling a line at a
	time and uses a virtual window concept that models the full text
	currently expanded. This function calculates the virtual length of
	the rules window so we can set the scroll bar length. Called initially,
	then after rules are expanded and contracted. The code mirrors the
	draw_rules() function with the intent to track the y axis increase as
	if actually being drawn. Scrolling is now controlled by an offset into
	this virtual window.
*/
static void calc_virt_win_len(rule_string * rules_ptr, int win_heigth, float zoom)
{
	int line_height = get_line_height(RULES_FONT, zoom);
	int i;
	int ydiff = 0;
	int max_scroll_pos = 0;

	virt_win_len = 0;
	recalc_virt_win_len = 0;

	/* model the draw_rules() keeping track of the y axis value
		NOTE: if draw_rules() is modified, you need to modify this too */
	for (i = 0; rules_ptr[i].type != END; ++i)
	{
		switch(rules_ptr[i].type)
		{
			case TITLE:
				ydiff = (int)(0.5 + 1.67 * line_height);
				break;
			case RULE:
				ydiff = (int)(0.5 + 1.11 * line_height);
				break;
			case INFO:
        		virt_win_len += (int)(0.5 + 0.55 * line_height);
				ydiff = (int)(0.5 + 1.11 * line_height);
        		break;
			}
		/* remember the offset for each rule start */
		rules_ptr[i].y_virt = virt_win_len;

		virt_win_len += ydiff + line_height * (rules_ptr[i].short_str_nr_lines - 1);
		if (rules_ptr[i].show_long_desc && rules_ptr[i].long_str)
		{
			virt_win_len += ydiff + line_height * (rules_ptr[i].long_str_nr_lines - 1);
		}
	}

	/* make sure the max scroll value leaves a mostly full screen */
	max_scroll_pos = virt_win_len - win_heigth * 0.9;

	/* set the scroll bar lengths and increment values */
	vscrollbar_set_bar_len(rules_root_win, rules_root_scroll_id, max_scroll_pos);
	vscrollbar_set_pos_inc(rules_root_win, rules_root_scroll_id, 18*zoom);
	vscrollbar_set_bar_len(rules_win, rules_scroll_id, max_scroll_pos);
	vscrollbar_set_pos_inc(rules_win, rules_scroll_id, 18*zoom);

	/* if we're been asked to start at a particular rule, set the scroll offset */
	if (set_rule_offset >= 0)
	{
		virt_win_offset = rules_ptr[set_rule_offset].y_virt;
		set_rule_offset = -1;
		vscrollbar_set_pos(rules_root_win, rules_root_scroll_id, virt_win_offset);
		vscrollbar_set_pos(rules_win, rules_scroll_id, virt_win_offset);
	}

	/* closing a rule could leave the offset past the end, so fix that if needed */
	if (vscrollbar_get_pos(rules_root_win, rules_root_scroll_id) >= max_scroll_pos)
	{
		vscrollbar_set_pos(rules_root_win, rules_root_scroll_id, max_scroll_pos -1);
		rules_root_scroll_handler();
	}
	if (vscrollbar_get_pos(rules_win, rules_scroll_id) >= max_scroll_pos)
	{
		vscrollbar_set_pos(rules_win, rules_scroll_id, max_scroll_pos -1);
		rules_scroll_handler();
	}

} /* end calc_virt_win_len() */

static void draw_rule_string(const unsigned char* str, int nr_lines, int x, int y,
	int y_in, int leny, int line_height, const float* color, int nr_width, float zoom)
{
	int lstart, lend;
	lstart = (virt_win_offset + y_in - y + line_height - 1) / line_height;
	if (lstart < 0)
		lstart = 0;
	lend = (virt_win_offset + leny - y) / line_height;
	if (lend > nr_lines)
		lend = nr_lines;

	if (lend > lstart)
	{
		int skip;
		for (skip = 0; *str && skip < lstart; ++skip)
		{
			str += strcspn((const char*) str, "\r\n");
			if (*str) ++str;
		}

		glColor3f(color[0], color[1], color[2]);
		y += lstart * line_height - virt_win_offset;
		if (nr_width > 0)
		{
			if (lstart == 0)
			{
				draw_string_zoomed_width_font(x, y, str, window_width, 1, RULES_FONT, zoom);
				str += strcspn((const char*) str, "\r\n");
				if (*str) ++str;
				++lstart;
				y += line_height;
			}
			x += nr_width;
		}

		if (lend > lstart)
			draw_string_zoomed_width_font(x, y, str, window_width, lend - lstart, RULES_FONT, zoom);
	}
}

static int draw_rules(rule_string* rules_ptr, int x_in, int y_in, int lenx, int leny,
	float zoom, const float rgb[8][3])
{
	int line_height = get_line_height(RULES_FONT, zoom);
	int i;
	int x=0, y_curr = y_in;

	if (recalc_virt_win_len)
		calc_virt_win_len(rules_ptr, leny-y_in, zoom);

	reached_end=0;

	for (i = 0; y_curr - virt_win_offset < leny; ++i)
	{
		rule_string *rule = &rules_ptr[i];
		int color_idx = 0;
		int ydiff = 0;

		switch (rule->type)
		{
			case END:
				read_all_rules = reached_end = 1;
				rule->y_start = 2*leny; // Minor trick
				return i;
			case TITLE:
				color_idx = 0;
				ydiff = (int)(0.5 + 1.67 * line_height);
				x = x_in + (lenx - x_in - rule->short_str_width) / 2;
				break;
			case RULE:
				color_idx = rule->highlight ? 1 : (rule->mouseover ? 2 : 3);
				ydiff = (int)(0.5 + 1.11 * line_height);
				x = x_in + short_rule_indent * zoom;
				break;
			case INFO:
				color_idx = rules_ptr[i].mouseover ? 4 : 5;
				ydiff = (int)(0.5 + 1.11 * line_height);
				x = x_in + short_rule_indent * zoom;
				y_curr += (int)(0.5 + 0.55 * line_height);
				break;
		}

		draw_rule_string(rule->short_str, rule->short_str_nr_lines, x, y_curr,
			y_in, leny, line_height, &rgb[color_idx][0],
			rule->short_str_nr_indent, zoom);
		rule->x_start = x;
		rule->y_start = y_curr - virt_win_offset;
		y_curr += (rule->short_str_nr_lines - 1) * line_height + ydiff;
		rule->x_end = rule->x_start + rule->short_str_width;
		rule->y_end = y_curr - virt_win_offset;

		if (rule->show_long_desc && rule->long_str)
		{
			color_idx = rules_ptr[i].highlight ? 6 : 7;
			draw_rule_string(rule->long_str, rule->long_str_nr_lines,
				x_in + long_rule_indent * zoom, y_curr, y_in, leny, line_height,
				&rgb[color_idx][0], 0, zoom);
			y_curr += (rule->long_str_nr_lines - 1) * line_height + ydiff;
		}
	}

	rules_ptr[i].y_start = 2*leny; // hehe ;-)
	return i; // The number of rules we're displaying
}

int has_accepted=0;

/*Root window*/

static int text_box_width = 0;
static int text_box_height = 0;
static int box_border_x = 0;
static int ui_seperator_y = 0;

static void init_rules_interface(float text_size, int count, int len_x, int len_y)
{
	if(rules.no)
	{
		if (last_display)
		{
			//We need to format the rules again..
			if (display_rules)
				free_rules (display_rules);
			display_rules = get_interface_rules(text_box_width, text_size);
		}
		countdown = count;	// Countdown in 0.5 seconds...
	}

	last_display = 0;
	has_accepted = 0;

}

static void draw_rules_interface (window_info * win)
{
	char str[200];
	float string_width, string_zoom;

	if ((countdown <= 0) && (read_all_rules))
	{
		widget_unset_flags (rules_root_win, rules_root_accept_id, WIDGET_DISABLED);
	}

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(box_border_x, ui_seperator_y, 0);
	glVertex3i(box_border_x + text_box_width, ui_seperator_y, 0);
	glVertex3i(box_border_x, ui_seperator_y + text_box_height, 0);
	glVertex3i(box_border_x + text_box_width, ui_seperator_y + text_box_height, 0);
	glVertex3i(box_border_x, ui_seperator_y, 0);
	glVertex3i(box_border_x, ui_seperator_y + text_box_height, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f (0.77f, 0.57f, 0.39f);

	if (countdown != 0)
		safe_snprintf (str, sizeof(str), you_can_proceed, countdown / 2);
	else
		safe_strncpy (str, accepted_rules, sizeof(str));

	/* scale the string if it is too wide for the screen */
	string_zoom = win->current_scale;
	string_width = get_string_width_zoom((const unsigned char*)str, win->font_category,
			string_zoom)
		+ 2 * win->default_font_max_len_x;
	if (string_width > win->len_x)
		string_zoom *= win->len_x / string_width;
	draw_text(win->len_x/2, win->len_y - ui_seperator_y - win->default_font_len_y,
		(const unsigned char*)str, strlen(str), win->font_category, TDO_ZOOM, string_zoom,
		TDO_ALIGNMENT, CENTER, TDO_END);

	draw_rules(display_rules, box_border_x, ui_seperator_y + win->default_font_len_y / 2,
		box_border_x + text_box_width, text_box_height, win->current_scale, rules_winRGB);

	glDisable (GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int rules_root_win = -1;

static int display_rules_root_handler (window_info *win)
{
	if(virt_win_offset < 0) virt_win_offset=0;
	draw_console_pic (cons_text);
	draw_rules_interface (win);
	CHECK_GL_ERRORS();

	draw_delay = 20;
	return 1;
}

static int mouseover_rules_root_handler (window_info *win, int mx, int my)
{
	int i;
	rule_string *rules_ptr = display_rules;

	for (i = 0; rules_ptr[i].type != END && rules_ptr[i].y_start < win->len_y; i++)
	{
		rules_ptr[i].mouseover = mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end
			&& my > rules_ptr[i].y_start && my < rules_ptr[i].y_end
			&& rules_ptr[i].long_str;
	}
	return 1;
}

static void switch_rules_to_next ()
{
	has_accepted = 1;
	show_window (next_win_id);
	show_hud_windows();
	destroy_window (rules_root_win);
	rules_root_win = -1;
	if (disconnected) connect_to_server();
}

static int rules_root_scroll_handler ()
{
	virt_win_offset = vscrollbar_get_pos (rules_root_win, rules_root_scroll_id);
	return 1;
}

static int click_rules_root_accept ()
{
	switch_rules_to_next ();
	return 1;
}

static int click_rules_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int i;
	rule_string *rules_ptr = display_rules;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
		return 1;
	} else {
		for (i=0; rules_ptr[i].type != END && rules_ptr[i].y_start < win->len_y; i++)
		{
			if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end)
			{
				rules_ptr[i].show_long_desc = !rules_ptr[i].show_long_desc;
				recalc_virt_win_len = 1;
				return 1;
				break;
			}
		}
	}
	return 0;
}

static int keypress_rules_root_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	// first, try to see if we pressed Alt+x, to quit.
	if ( check_quit_or_fullscreen (key_unicode, key_mod) )
	{
		return 1;
	}
	else if (key_code == SDLK_DOWN)
	{
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (key_code == SDLK_UP)
	{
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (key_code == SDLK_PAGEUP)
	{
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (key_code == SDLK_PAGEDOWN)
	{
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && countdown <= 0 && (read_all_rules))
	{
		switch_rules_to_next ();
	}
	else
		return 0;

	return 1;
}

static void adjust_ui_elements(window_info *win, int scroll_id, int accept_id)
{
	int accept_label_width;
	int char_per_line = 65;
	if (char_per_line * win->default_font_max_len_x > win->len_x - 3 * win->box_size)
		char_per_line = (win->len_x - 3 * win->box_size) / win->default_font_max_len_x;

	accept_label_width = get_string_width_zoom((const unsigned char*)accept_label,
		win->font_category, win->current_scale);
	button_resize(win->window_id, accept_id, accept_label_width + 40, (int)(0.5 + win->current_scale * 32), win->current_scale);

	text_box_width = char_per_line * get_avg_char_width_zoom(win->font_category,
		win->current_scale);
	box_border_x = (win->len_x - text_box_width - win->box_size) / 2;

	text_box_height = 0.75 * (win->len_y - widget_get_height(win->window_id, accept_id) - win->default_font_len_y);
	ui_seperator_y = (win->len_y - widget_get_height(win->window_id, accept_id) - win->default_font_len_y - text_box_height) / 4;

	widget_resize(win->window_id, scroll_id, win->box_size, text_box_height);

	widget_move(win->window_id, scroll_id, box_border_x + text_box_width, ui_seperator_y);
	widget_move(win->window_id, accept_id, (win->len_x - widget_get_width(win->window_id, accept_id)) / 2, text_box_height + 2 * ui_seperator_y);
}

static int resize_rules_root_handler (window_info *win, Uint32 w, Uint32 h)
{
	last_display = 1;
	adjust_ui_elements(win, rules_root_scroll_id, rules_root_accept_id);
	init_rules_interface (win->current_scale, countdown, w, h);
	return 1;
}

static int ui_scale_rules_root_handler(window_info *win)
{
	resize_window(win->window_id, win->len_x, win->len_y);
	return 1;
}

void create_rules_root_window (int width, int height, int next, int time)
{
	if (rules_root_win < 0)
	{
		window_info *win = NULL;

		rules_root_win = create_window (win_rules, -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_font_category(rules_root_win, RULES_FONT);
		if (rules_root_win >= 0 && rules_root_win < windows_list.num_windows)
			win = &windows_list.window[rules_root_win];
		else
			return;

		rules_root_scroll_id = vscrollbar_add_extended (rules_root_win, rules_root_scroll_id, NULL, 0, 0, 0, 0, 0, win->current_scale, 0, 3, rules.no-1);
		rules_root_accept_id = button_add_extended (rules_root_win, rules_root_scroll_id + 1, NULL, 0, 0, 0, 0, WIDGET_DISABLED, win->current_scale, accept_label);
		widget_set_color(rules_root_win, rules_root_accept_id, 1.0f, 1.0f, 1.0f);

		adjust_ui_elements(win, rules_root_scroll_id, rules_root_accept_id);

		set_window_handler (rules_root_win, ELW_HANDLER_DISPLAY, &display_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_CLICK, &click_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_RESIZE, &resize_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_UI_SCALE, &ui_scale_rules_root_handler);
		widget_set_OnClick (rules_root_win, rules_root_scroll_id, rules_root_scroll_handler);
		widget_set_OnDrag (rules_root_win, rules_root_scroll_id, rules_root_scroll_handler);
		widget_set_OnClick(rules_root_win, rules_root_accept_id, &click_rules_root_accept);
	}

	init_rules_interface (windows_list.window[rules_root_win].current_scale, 2*time, width, height);
	next_win_id = next;
}
