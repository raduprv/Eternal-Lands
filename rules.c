#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdlib.h>
#ifndef BSD
#include <malloc.h>
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
int have_rules=0;
int rule_offset=0;
rule_string * display_rules;
int last_display=-1;

/*Interface*/
int countdown = 0;

int x_arrow;
int y_arrow_up;
int y_arrow_down;
int arrow_size;

int mouse_over_up = 1;
int mouse_over_down = 1;

int accept_x;
int accept_y;
int accept_width;
int accept_height;

int next_win_id;

/* Colors */
const float rules_winRGB[8][3] = {{0.0f,1.0f,0.0f},{1.0f,0.0f,0.0f},{1.2f,0.5f,1.2f},{1.0f,0.3f,1.0f},{0.1f,0.5f,0.9f},{0.1f,0.5f,1.0f},{0.8f,0.0f,0.0f},{1.0f,1.0f,1.0f}};
const float rules_screenRGB[8][3] = {{1.0f,0.0f,0.0f},{1.0f,0.0f,0.0f},{0.77f,0.5f,0.4f},{0.77f, 0.57f, 0.39f},{0.8f,0.5f,0.5f},{0.76f,0.48f,0.39f},{0.8f,0.0f,0.0f},{0.76f,0.5f,0.37f}};

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
	rules_scroll_id = vscrollbar_add_extended (rules_win, rules_scroll_id, NULL, HELP_TAB_WIDTH - 20, 0, 20, HELP_TAB_HEIGHT, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, rules.no-1);
	
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
	
	if(rules_ptr[rules_no+1].type==-1) reached_end=1;
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
	float window_ratio = (float)len_x / 640.0f;
	
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
	
	arrow_size = 32;
	x_arrow = (len_x + len_y) / 2 - 50 * window_ratio - arrow_size / 2;
	y_arrow_up = 128 * window_ratio - arrow_size / 2;
	y_arrow_down = len_y - 128 * window_ratio - arrow_size / 2;

	// Roja prefers the button to be natural size
	accept_width = 64; //100 * window_ratio;
	accept_height = 32; //40 * window_ratio;
	
	accept_x = (len_x - accept_width)/ 2; // len_x / 2 - 50 * window_ratio;
	accept_y = len_y - 95 * window_ratio;
}

float rules_u_start=(float)1/256;
float rules_v_start=1.0f-(float)63/256;
float rules_u_end=(float)127/256;
float rules_v_end=1.0f-(float)127/256;

float accept_u_start=(float)1/256;
float accept_v_start=1.0f-(float)1/256;
float accept_u_end=(float)63/256;
float accept_v_end=1.0f-(float)31/256;

float colored_accept_u_start=(float)1/256;
float colored_accept_v_start=1.0f-(float)33/256;
float colored_accept_u_end=(float)63/256;
float colored_accept_v_end=1.0f-(float)63/256;

float arrow_u_start=(float)64/256;
float arrow_v_start=1.0f;
float arrow_u_end=(float)79/256;
float arrow_v_end=1.0f-(float)31/256;

float colored_arrow_u_start=(float)80/256;
float colored_arrow_v_start=1.0f;
float colored_arrow_u_end=(float)95/256;
float colored_arrow_v_end=1.0f-(float)31/256;

void draw_rules_interface (int len_x, int len_y)
{
	char str[200];
	float diff = (float) (len_x - len_y) / 2;
	int y, width, height;	// Width/Height are 0.5*width/height
	float window_ratio = (float) len_x / 640.0f;

    	glColor3f(1.0f,1.0f,1.0f);
	
	get_and_set_texture_id(paper1_text);
	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.05f);
	
    	glPushMatrix();
	glTranslatef(diff,0,0);
	glBegin(GL_QUADS);
	//draw the texture
		glTexCoord2f(0.0f,1.0f); glVertex3i(0,0,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(0,len_y,0);
		glTexCoord2f(1.0f,0.0f); glVertex3i(len_y,len_y,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(len_y,0,0);
	glEnd();
	glPopMatrix ();
	
	get_and_set_texture_id(hud_text);
    	
	// Roja prefers the button to be natural size
	//width = 120*window_ratio;
	//height = 40*window_ratio;
	width = 128/2;
	height = 64/2;
	y = 66*window_ratio;
    
	glPushMatrix ();
	glTranslatef (len_x / 2, y, 0);
	glBegin (GL_QUADS);
		glTexCoord2f (rules_u_start, rules_v_end);	glVertex2i (-width, height);
		glTexCoord2f (rules_u_end, rules_v_end);	glVertex2i (width, height);
		glTexCoord2f (rules_u_end, rules_v_start);	glVertex2i (width, -height);
		glTexCoord2f (rules_u_start, rules_v_start);	glVertex2i (-width, -height);
	glEnd();
	glPopMatrix();

	glPushMatrix ();
	glTranslatef (accept_x, accept_y, 0);
	if (countdown > 0)
	{
		glAlphaFunc (GL_GREATER, 0.04f);
		
		glBegin (GL_QUADS);
			glTexCoord2f (accept_u_start, accept_v_end);	glVertex2i (0, accept_height);
			glTexCoord2f (accept_u_end, accept_v_end);	glVertex2i (accept_width, accept_height);
			glTexCoord2f (accept_u_end, accept_v_start);	glVertex2i (accept_width, 0);
			glTexCoord2f (accept_u_start, accept_v_start);	glVertex2i (0, 0);
	}
	else 
	{
		glAlphaFunc (GL_GREATER, 0.02f);
		
		glBegin(GL_QUADS);
			glTexCoord2f (colored_accept_u_start, colored_accept_v_end);	glVertex2i (0, accept_height);
			glTexCoord2f (colored_accept_u_end, colored_accept_v_end);	glVertex2i (accept_width, accept_height);
			glTexCoord2f (colored_accept_u_end, colored_accept_v_start);	glVertex2i (accept_width, 0);
			glTexCoord2f (colored_accept_u_start, colored_accept_v_start);	glVertex2i (0, 0);
		
	}
	glEnd();
	glPopMatrix();

	glColor3f(1.0f,1.0f,1.0f);
	
	glPushMatrix();
	glTranslatef (x_arrow, y_arrow_up, 0);

	if (mouse_over_up)
	{
		glAlphaFunc (GL_GREATER, 0.04f);
	
		glBegin (GL_QUADS);
			glTexCoord2f (colored_arrow_u_start, colored_arrow_v_end);	glVertex2i(0, arrow_size);
			glTexCoord2f (colored_arrow_u_end, colored_arrow_v_end);	glVertex2i(arrow_size, arrow_size);
			glTexCoord2f (colored_arrow_u_end, colored_arrow_v_start);	glVertex2i(arrow_size, 0);
			glTexCoord2f (colored_arrow_u_start, colored_arrow_v_start);	glVertex2i(0, 0);
	}
	else
	{
		glAlphaFunc (GL_GREATER, 0.01f);
		
		glBegin (GL_QUADS);	
			glTexCoord2f (arrow_u_start, arrow_v_end);	glVertex2i(0, arrow_size);
			glTexCoord2f (arrow_u_end, arrow_v_end);	glVertex2i(arrow_size, arrow_size);
			glTexCoord2f (arrow_u_end, arrow_v_start);	glVertex2i(arrow_size, 0);
			glTexCoord2f (arrow_u_start, arrow_v_start);	glVertex2i(0, 0);
	}
	glEnd ();
	glPopMatrix ();
	
	glPushMatrix ();
	glTranslatef (x_arrow+arrow_size, y_arrow_down+arrow_size, 0);
	glRotatef (180.0f, 0.0f, 0.0f, 1.0f);
	if (mouse_over_down)
	{
		glAlphaFunc (GL_GREATER, 0.04f);
	
		glBegin(GL_QUADS);
			glTexCoord2f (colored_arrow_u_start, colored_arrow_v_end);	glVertex2i(0, arrow_size);
			glTexCoord2f (colored_arrow_u_end, colored_arrow_v_end);	glVertex2i(arrow_size, arrow_size);
			glTexCoord2f (colored_arrow_u_end, colored_arrow_v_start);	glVertex2i(arrow_size, 0);
			glTexCoord2f (colored_arrow_u_start, colored_arrow_v_start);	glVertex2i(0, 0);
	} else {
		glAlphaFunc(GL_GREATER, 0.01f);
		
		glBegin(GL_QUADS);
			glTexCoord2f (arrow_u_start, arrow_v_end);	glVertex2i(0, arrow_size);
			glTexCoord2f (arrow_u_end, arrow_v_end);	glVertex2i(arrow_size, arrow_size);
			glTexCoord2f (arrow_u_end, arrow_v_start);	glVertex2i(arrow_size, 0);
			glTexCoord2f (arrow_u_start, arrow_v_start);	glVertex2i(0, 0);
	}
	glEnd ();
	glPopMatrix ();
	
	glColor3f (0.77f, 0.57f, 0.39f);
	
	if (countdown != 0) 
		sprintf (str, you_can_proceed, countdown / 2);
	else 
		strcpy (str, accepted_rules);
		
	draw_string (len_x / 2 - strlen (str) * 11 / 2, len_y - 40 * window_ratio, str, 0);
	
	set_font(3);
	draw_rules (display_rules, rule_offset, diff + 30 * window_ratio, 120 * window_ratio, len_y + diff / 2 - 50, len_y - 140 * window_ratio, 1.1f, rules_screenRGB);
	set_font(0);

    	glDisable (GL_ALPHA_TEST);
}

int rules_root_win = -1;

int display_rules_root_handler (window_info *win)
{
	if (SDL_GetAppState () & SDL_APPACTIVE)
	{	
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
	
	mouse_over_up = 0;
	mouse_over_down = 0;
	
	if (mx > x_arrow && mx < x_arrow + arrow_size)
	{
		if (my > y_arrow_up && my < y_arrow_up + arrow_size)
		{
			mouse_over_up = 1;
		}
		else if (my > y_arrow_down && my < y_arrow_down + arrow_size)
		{
			mouse_over_down = 1;
		}
	}
	
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
	destroy_window (rules_root_win);
	rules_root_win = -1;
	if (disconnected) connect_to_server();
}

int click_rules_root_handler (window_info *win, int mx, int my, Uint32 flags)
{
	int i;
	rule_string *rules_ptr = display_rules + rule_offset;

	if(flags&ELW_WHEEL_UP) {
		if (rule_offset > 0) {
			rule_offset--;
		}
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		if (!reached_end && display_rules[rule_offset].type != -1) { //Make sure we don't scroll too fast
			rule_offset++;
		}
		return 1;
	} else {
		if (mx > x_arrow && mx < x_arrow + arrow_size)
		{
			if (my > y_arrow_up && my < y_arrow_up + arrow_size)
			{
				if (rule_offset > 0)
					rule_offset--;
				return 1;
			}
			else if (my > y_arrow_down && my < y_arrow_down + arrow_size)
			{
				if (!reached_end)
					rule_offset++;
				return 1;
			}
		}
		else if (countdown <= 0 && mx > accept_x && mx < accept_x + accept_width && my > accept_y && my < accept_y + accept_height)
		{
			switch_rules_to_next ();
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
		rule_offset++;
	}
	else if (keysym == SDLK_UP && rule_offset > 0)
	{
		rule_offset--;
	}
	else if (keysym == SDLK_PAGEUP)
	{
		rule_offset = rule_offset < 3 ? 0 : rule_offset-3;
	}
	else if (keysym == SDLK_PAGEDOWN)
	{
		int i;
		for (i = 0; i < 3 && display_rules[rule_offset+1].type != -1; i++)
			rule_offset++;
	}
	else if (keysym == SDLK_RETURN && countdown <= 0)
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
		rules_root_win = create_window (win_rules, -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW_LAST);
		
		set_window_handler (rules_root_win, ELW_HANDLER_DISPLAY, &display_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_CLICK, &click_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_KEYPRESS, &keypress_rules_root_handler);
		set_window_handler (rules_root_win, ELW_HANDLER_RESIZE, &resize_rules_root_handler);		
	}
	
	init_rules_interface (1.0, 2*time, width, height);
	next_win_id = next;
}
