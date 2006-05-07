#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdlib.h>

#ifndef BSD
	#if defined (OSX) || defined (OSX86)
		#include <sys/malloc.h>
	#else
		#include <malloc.h>
	#endif
#endif

#include "global.h"

#define TITLE 	0
#define RULE	1
#define INFO 	2

#define INTERFACE_GAME 0
#define INTERFACE_LOG_IN 1
#define INTERFACE_NEW_CHAR 2
#define INTERFACE_CONSOLE 3
#define INTERFACE_OPENING 4
#define INTERFACE_MAP 5
#define INTERFACE_CONT 6
#define INTERFACE_RULES 7

/*Window*/
int rules_win=-1;
int rules_win_x=100;
int rules_win_y=100;
int rules_win_x_len=HELP_TAB_WIDTH-ELW_CLOSE_BOX;
int rules_win_y_len=HELP_TAB_HEIGHT;
int rules_scroll_id = 0;

/*Shared*/
int reached_end=0;
int read_all_rules=0;
int have_rules=0;
int rule_offset=0;
rule_string * display_rules;
int last_display=-1;

/*Interface*/
int countdown = 0;

int rules_root_scroll_id = 0;
int rules_root_accept_id = 0;

int next_win_id;

/* Colors */
const float rules_winRGB[8][3] = {{0.0f,1.0f,0.0f},{1.0f,0.0f,0.0f},{1.2f,0.5f,1.2f},{1.0f,0.3f,1.0f},{0.1f,0.5f,0.9f},{0.1f,0.5f,1.0f},{0.8f,0.0f,0.0f},{1.0f,1.0f,1.0f}};

/* Rule parser */
static struct rules_struct rules = {0,{{NULL,0,NULL,0,0}}};

void free_rules(rule_string * d);
rule_string * get_interface_rules(int chars_per_line);
int draw_rules(rule_string * rules_ptr, int rules_no, int x_in, int y_in, int lenx, int leny, float text_size, const float rgb[8][3]);

void add_rule(char * short_desc, char * long_desc, int type)
{
	int no=rules.no++;
	int len;

	len=MY_XMLSTRCPY(&rules.rule[no].short_desc, short_desc);
	rules.rule[no].short_len=len;
	len=MY_XMLSTRCPY(&rules.rule[no].long_desc, long_desc);
	rules.rule[no].long_len=len;

	rules.rule[no].type=type;
}

char * get_id_str(xmlNode * node, char *id)
{
	xmlNode * cur;
	for(cur=node;cur;cur=cur->next) {
		if(cur->type == XML_ELEMENT_NODE && cur->children){
			if(!xmlStrcasecmp(cur->name,id)) return cur->children->content;
		}
	}

	return NULL;
}

int parse_rules(xmlNode * root)
{
	xmlNode * cur;
	
	if(!root) return 0;

	for(cur=root;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if(cur->children){
				if(!xmlStrcasecmp(cur->name,"title")) {
					add_rule(cur->children->content, NULL, TITLE);
				} else if(!xmlStrcasecmp(cur->name,"rule")) {
					add_rule(get_id_str(cur->children, "short"), get_id_str(cur->children, "long"), RULE);
				} else if(!xmlStrcasecmp(cur->name,"info")) {
					add_rule(get_id_str(cur->children, "short"), get_id_str(cur->children, "long"), INFO);
				}
			}
		}
	}
	
	return 1;
}

int read_rules()
{
	char file_name[120];
	xmlDoc * doc;
	xmlNode * root;
	sprintf(file_name,"languages/%s/rules.xml",lang);
	
	if ((doc = xmlReadFile(file_name, NULL, 0)) == NULL) {
		if((doc=xmlReadFile("languages/en/rules.xml",NULL,0))==NULL){
			//report this error:
			log_error(read_rules_str);
			return 0;
		}
	}
	
	if ((root = xmlDocGetRootElement(doc))==NULL) {
		log_error(read_rules_str);
	} else if(!parse_rules(root->children)){
		log_error(parse_rules_str);
	}

	xmlFreeDoc(doc);
	
	return (rules.no>0);
}

/*Rules window interface*/

int rules_scroll_handler ()
{
	rule_offset = vscrollbar_get_pos (rules_win, rules_scroll_id);
	return 1;
}

int click_rules_handler (window_info *win, int mx, int my, Uint32 flags)
{
	rule_string *rules_ptr = display_rules + rule_offset;
	int i;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(rules_win, rules_scroll_id);
		rules_scroll_handler();
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(rules_win, rules_scroll_id);
		rules_scroll_handler();
		return 1;
	} else {
		for (i = 0; rules_ptr[i].type != -1 && rules_ptr[i].y_start < win->len_y; i++)
		{
			if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end)
			{
				rules_ptr[i].show_long_desc = !rules_ptr[i].show_long_desc;
				return 1;
			}
		}
	}
	return 0;
}

int mouseover_rules_handler (window_info *win, int mx, int my)
{
	rule_string *rules_ptr = display_rules + rule_offset;
	int i;

	for (i = 0; rules_ptr[i].type != -1 && rules_ptr[i].y_start < win->len_y; i++)
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

int display_rules_handler(window_info *win)
{
	int len;
	if(rule_offset < 0)rule_offset=0;
	len=(float)draw_rules(display_rules, rule_offset, 0, 20, win->len_x, win->len_y-40,0.8f, rules_winRGB)/(float)rules.no*250;

	return 1;
}

void fill_rules_window()
{
	rules_scroll_id = vscrollbar_add_extended (rules_win, rules_scroll_id, NULL, HELP_TAB_WIDTH - 20, 0, 20, HELP_TAB_HEIGHT, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 3, rules.no-1);
	
	widget_set_OnClick (rules_win, rules_scroll_id, rules_scroll_handler);
	widget_set_OnDrag (rules_win, rules_scroll_id, rules_scroll_handler);
	
	if(display_rules)free_rules(display_rules);
	display_rules=get_interface_rules((float)(rules_win_x_len-70)/(12*0.8f)-1);
		
	set_window_handler(rules_win, ELW_HANDLER_DISPLAY, &display_rules_handler);
	set_window_handler(rules_win, ELW_HANDLER_MOUSEOVER, &mouseover_rules_handler);
	set_window_handler(rules_win, ELW_HANDLER_CLICK, &click_rules_handler);
}

void toggle_rules_window()
{
	if(last_display<=0||display_rules==NULL){
		if(display_rules)free_rules(display_rules);
		display_rules=get_interface_rules((float)(rules_win_x_len-70)/(12*0.8f)-1);
	}

	view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_RULES);

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

void free_rules(rule_string * d)
{
	int i, j;
	if(!d)return;
	for(i=0;d[i].type!=-1;i++){
		if(d[i].short_str) for(j=0;d[i].short_str[j];j++) free(d[i].short_str[j]);
		if(d[i].long_str)  for(j=0;d[i].long_str[j];j++)  free(d[i].long_str[j]);
	}
	free(d);
}

void reset_rules(rule_string * r)
{
	int i;
	for(i=0;r[i].type!=-1;i++){
		r[i].show_long_desc=0;
		r[i].x_start=0;
		r[i].y_start=0;
		r[i].x_end=0;
		r[i].y_end=0;
		r[i].highlight=0;
		r[i].mouseover=0;
	}
	rule_offset=0;
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
					create_newchar_root_window (window_width, window_height);
				next_win_id = newchar_root_win;
				break;
			case INTERFACE_CONSOLE:
				if (console_root_win < 0)
					create_console_root_window (window_width, window_height);
				next_win_id = console_root_win;
				break;
			case INTERFACE_OPENING:
				if (opening_root_win < 0)
					create_opening_root_window (window_width, window_height);
				next_win_id = opening_root_win;
				break;
			case INTERFACE_MAP:
			case INTERFACE_CONT:
				if (map_root_win < 0)
					create_map_root_window (window_width, window_height);
				next_win_id = map_root_win;
				break;
			case INTERFACE_RULES: 
				// doesn't make sense, use the default
			case INTERFACE_GAME:
			default:
				if (game_root_win < 0)
					create_game_root_window (window_width, window_height);
				next_win_id = game_root_win; break;
		}
			
		create_rules_root_window ( window_width, window_height, next_win_id, *((Uint16*)(rule)) );
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
			
			for(j=0;display_rules[j].type!=-1;j++){
				if(display_rules[j].type==RULE)r++;
				if(r==cur_rule) break;
			}

			if(display_rules[j].type!=-1){
				display_rules[j].highlight=1;
				display_rules[j].show_long_desc=1;
			}
		}
		
		for(i=0;display_rules[i].type!=-1;i++)
			if(display_rules[i].type == RULE && display_rules[i].highlight) {
				rule_offset=i;//Get the first highlighted entry
				if (type == RULE_WIN)
					vscrollbar_set_pos (rules_win, rules_scroll_id, rule_offset);
				return;
			}
	}
}

rule_string * get_interface_rules(int chars_per_line)
{
	int i;
	rule_string * _rules=(rule_string*)calloc((rules.no+1),sizeof(rule_string));
	
	for(i=0;i<rules.no;i++){
		_rules[i].type=rules.rule[i].type;
		_rules[i].short_str=get_lines(rules.rule[i].short_desc,chars_per_line);
		_rules[i].long_str=get_lines(rules.rule[i].long_desc,chars_per_line);
	}

	_rules[rules.no].type=-1;

	reset_rules(_rules);
	
	return _rules;
}

int draw_rules(rule_string * rules_ptr, int rules_no, int x_in, int y_in, int lenx, int leny, float text_size, const float rgb[8][3])
{
	int xdiff=0,ydiff=18,i,j=0,tmplen=0,len=0;
	char str[1024];
	char *ptr;
	float zoom=text_size;
	int x=0, y=y_in;
	int nr;
	
	if(rules_ptr[rules_no+1].type==-1) read_all_rules = reached_end = 1;
	else reached_end=0;

	nr = 1;
	for(i=0;i < rules_no; i++)
		if (rules_ptr[i].type == RULE) nr++;
		
	for(i=rules_no;y<leny;i++){
		ptr=str;
		len=0;
		switch(rules_ptr[i].type){
			case -1:
				rules_ptr[i].y_start=2*leny;//Minor trick
				return i;
			case TITLE:
				glColor3f(rgb[0][0],rgb[0][1],rgb[0][2]);
				zoom=text_size*1.5f;
				ydiff=30*zoom;
				xdiff=0;
				x=x_in+((lenx-x_in)>>1)-(strlen(rules_ptr[i].short_str[0])>>1)*11*zoom;
				break;
			case RULE:
				if(rules_ptr[i].highlight) glColor3f(rgb[1][0],rgb[1][1],rgb[1][2]);
				else if(rules_ptr[i].mouseover) glColor3f(rgb[2][0],rgb[2][1],rgb[2][2]);
				else glColor3f(rgb[3][0],rgb[3][1],rgb[3][2]);
				sprintf(str,"%d: ", nr++);
				ptr+=strlen(str);
				zoom=text_size;
				xdiff=(ptr-str)*11*zoom;
				x=x_in+20;
				ydiff=20*zoom;
				break;
			case INFO:
				if(rules_ptr[i].mouseover) glColor3f(rgb[4][0],rgb[4][1],rgb[4][2]);
				else glColor3f(rgb[5][0],rgb[5][1],rgb[5][2]);
				zoom=text_size;
				x=x_in+20;
				y+=10*zoom;
				ydiff=20*zoom;
				xdiff=0;
				break;
		}
		rules_ptr[i].x_start=x;
		rules_ptr[i].y_start=y;
		for(j=0;rules_ptr[i].short_str[j] &&y<leny;j++){//Draw the lines
			if(j==1)ptr=str;
			if(j) y+=18*zoom;
			strcpy(ptr,rules_ptr[i].short_str[j]);
			if(!j)draw_string_zoomed(x,y,str,0,zoom);
			else draw_string_zoomed(x+xdiff,y,str,0,zoom);
			tmplen=strlen(str)*11*zoom;
			if(tmplen>len)len=tmplen;
		}
		y+=ydiff;
		rules_ptr[i].y_end=y;
		rules_ptr[i].x_end=rules_ptr[i].x_start+len;
		if(rules_ptr[i].show_long_desc && rules_ptr[i].long_str){//Draw the lines of the long description
			if(rules_ptr[i].highlight) glColor3f(rgb[6][0],rgb[6][1],rgb[6][2]);
			else glColor3f(rgb[7][0],rgb[7][1],rgb[7][2]);
			for(j=0;rules_ptr[i].long_str[j]&& y<leny;j++){
				if(j)y+=18*zoom;
				strcpy(str,rules_ptr[i].long_str[j]);
				draw_string_zoomed(x+20,y,str,0,zoom);
			}
			y+=ydiff;
		}
	}
	
	rules_ptr[i].y_start=2*leny;//hehe ;-)

	return i;//The number of rules we're displaying
}

int has_accepted=0;

/*Root window*/

void init_rules_interface(float text_size, int count, int len_x, int len_y)
{
//	float window_ratio = (float)len_x / 640.0f;
	
	if(rules.no)
	{
		if (last_display)
		{
			//We need to format the rules again..
			if (display_rules)
				free_rules (display_rules);
			display_rules = get_interface_rules ((float)(len_y - 120 * len_y / 480.0f) / (12 * text_size) - 1);
		}
		countdown = count;	// Countdown in 0.5 seconds...
	}

	last_display = 0;
	has_accepted = 0;

}

void draw_rules_interface (int len_x, int len_y)
{
	char str[200];
	float diff = (float) (len_x - len_y) / 2;
	int y;//, width, height;	// Width/Height are 0.5*width/height
	float window_ratio = (float) len_x / 640.0f;

	y = 66*window_ratio;
    
	if ((countdown <= 0) && (read_all_rules))
	{
		widget_unset_flag(rules_root_win, rules_root_accept_id, WIDGET_DISABLED);
	}

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

	glColor3f (0.77f, 0.57f, 0.39f);
	
	if (countdown != 0) 
		sprintf (str, you_can_proceed, countdown / 2);
	else 
		strcpy (str, accepted_rules);
		
	draw_string ((len_x - (strlen (str) * 11)) / 2, len_y - 40 * window_ratio, str, 0);
	
	set_font(3);
	draw_rules (display_rules, rule_offset, diff + 30 * window_ratio, 60 * window_ratio, len_y + diff / 2 - 50, len_y - 140 * window_ratio, 1.0f, rules_winRGB);
	set_font(0);

    	glDisable (GL_ALPHA_TEST);
}

int rules_root_win = -1;

int display_rules_root_handler (window_info *win)
{
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{	
		if(rule_offset < 0)rule_offset=0;
		draw_console_pic (cons_text);
		draw_rules_interface (win->len_x, win->len_y);
		CHECK_GL_ERRORS();
	}
	
	draw_delay = 20;
	return 1;
}

int mouseover_rules_root_handler (window_info *win, int mx, int my)
{
	int i;
	rule_string *rules_ptr = display_rules + rule_offset;

	for (i = 0; rules_ptr[i].type != -1 && rules_ptr[i].y_start < win->len_y; i++)
	{
		if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end && rules_ptr[i].long_str)
			rules_ptr[i].mouseover = 1;
		else 
			rules_ptr[i].mouseover = 0;
	}
	return 1;
}

void switch_rules_to_next ()
{
	has_accepted = 1;
	show_window (next_win_id);
	show_hud_windows();
	destroy_window (rules_root_win);
	rules_root_win = -1;
	if (disconnected) connect_to_server();
}

int rules_root_scroll_handler ()
{
	rule_offset = vscrollbar_get_pos (rules_root_win, rules_root_scroll_id);
	return 1;
}

int click_rules_root_accept ()
{
	switch_rules_to_next ();
	return 1;
}

int click_rules_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int i;
	rule_string *rules_ptr = display_rules + rule_offset;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
		return 1;
	} else {
		for (i=0; rules_ptr[i].type != -1 && rules_ptr[i].y_start < win->len_y; i++)
		{
			if (mx > rules_ptr[i].x_start && mx < rules_ptr[i].x_end && my > rules_ptr[i].y_start && my < rules_ptr[i].y_end)
			{
				rules_ptr[i].show_long_desc = !rules_ptr[i].show_long_desc;
				return 1;
				break;
			}
		}
	}
	return 0;
}

int keypress_rules_root_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;

	// first, try to see if we pressed Alt+x, to quit.
	if ( check_quit_or_fullscreen (key) )
	{
		return 1;
	}
	else if (keysym == SDLK_DOWN && !reached_end)
	{
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (keysym == SDLK_UP && rule_offset > 0)
	{
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (keysym == SDLK_PAGEUP)
	{
		vscrollbar_scroll_up(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (keysym == SDLK_PAGEDOWN)
	{
		vscrollbar_scroll_down(rules_root_win, rules_root_scroll_id);
		rules_root_scroll_handler();
	}
	else if (keysym == SDLK_RETURN && countdown <= 0 && (read_all_rules))
	{
		switch_rules_to_next ();
	}
	
	return 1;
}

int resize_rules_root_handler (window_info *win, Uint32 w, Uint32 h)
{
	init_rules_interface (1.0, countdown, w, h);
	return 1;
}

void create_rules_root_window (int width, int height, int next, int time)
{
	if (rules_root_win < 0)
	{
		float diff = (float) (width - height) / 2;
		float window_ratio = (float) width / 640.0f;
		int accept_width = (strlen(accept_label) * 11) + 40;
		int accept_height = 32;
		
		rules_root_win = create_window (win_rules, -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);

		rules_root_scroll_id = vscrollbar_add_extended (rules_root_win, rules_root_scroll_id, NULL, width - (diff + 30 * window_ratio) - 20, 50 * window_ratio, 20, 320 * window_ratio, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 3, rules.no-1);
		rules_root_accept_id = button_add_extended (rules_root_win, rules_root_scroll_id + 1, NULL, (width - accept_width) /2, height - 80 * window_ratio, accept_width, accept_height, WIDGET_DISABLED, 1.0f, 1.0f, 1.0f, 1.0f, accept_label);

		set_window_handler (rules_root_win, ELW_HANDLER_DISPLAY, &display_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_CLICK, &click_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_KEYPRESS, &keypress_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_RESIZE, &resize_rules_root_handler);		
		widget_set_OnClick (rules_root_win, rules_root_scroll_id, rules_root_scroll_handler);
		widget_set_OnDrag (rules_root_win, rules_root_scroll_id, rules_root_scroll_handler);
		widget_set_OnClick(rules_root_win, rules_root_accept_id, &click_rules_root_accept);
	}
	
	init_rules_interface (1.0, 2*time, width, height);
	next_win_id = next;
}
