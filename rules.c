#include "global.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <malloc.h>

#define TITLE 	0
#define RULE	1
#define INFO 	2

/*Window*/
int rules_win;
int rules_win_x=100;
int rules_win_y=100;
int rules_win_x_len=400;
int rules_win_y_len=300;

/*Shared*/
int reached_end=0;
int have_rules=0;
int rule_offset=0;
rule_string * display_rules;
int last_display=-1;

/*Interface*/
int countdown=-1;
int next_interface=interface_log_in;

/* Rule parser */
static struct rules_struct rules = {0,{{NULL,0,NULL,0,0}}};

void add_rule(char * short_desc, char * long_desc, int type)
{
	int no=rules.no++;
	int len;

	len=my_xmlStrcpy(&rules.rule[no].short_desc, short_desc);
	rules.rule[no].short_len=len;
	len=my_xmlStrcpy(&rules.rule[no].long_desc, long_desc);
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
		//report this error:
		log_error("The rules could not be read!");
		return 0;
	}

	if ((root = xmlDocGetRootElement(doc))==NULL) {
		xmlFreeDoc(doc);
		log_error("The rules could not be read!");
		return 0;
	}

	if(!parse_rules(root->children)){
		xmlFreeDoc(doc);
		log_error("Error while parsing the rules");
		return 0;
	}

	xmlFreeDoc(doc);
	
	return 1;
}

/*Rules window interface*/

int mouseover_rules_handler(window_info * win, int mx, int my)
{
	check_mouse_rules_interface(display_rules+rule_offset, win->len_x, win->len_y, mx, my);
	return 0;
}

int click_rules_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if(mx > win->len_x-16){
		if(my<=32 && my>=18) {
			if(rule_offset>1){
				rule_offset--;
				reached_end=0;
			}
		}  else if(my<=win->len_y-2 && my>=win->len_y-20) 
			if(!reached_end) {
				rule_offset++;
			}
	}
	
	return 1;
}

int display_rules_handler(window_info *win)
{
	int len,y;
	if(!rule_offset)rule_offset=1;
	len=(float)draw_rules(display_rules+rule_offset, rule_offset, 0, 20, win->len_x, win->len_y-40,0.8f)/(float)rules.no*250;
	y=(float)(rule_offset-1)*250/(float)rules.no;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	//scroll bar
	glBegin(GL_LINES);
		glVertex3i(win->len_x-20,20,0);
		glVertex3i(win->len_x-20,win->len_y,0);
		
		glVertex3i(win->len_x-15,30,0);
		glVertex3i(win->len_x-10,25,0);
		glVertex3i(win->len_x-10,25,0);
		glVertex3i(win->len_x-5,30,0);
		
		glVertex3i(win->len_x-15,win->len_y-15,0);
		glVertex3i(win->len_x-10,win->len_y-10,0);
		glVertex3i(win->len_x-10,win->len_y-10,0);
		glVertex3i(win->len_x-5,win->len_y-15,0);
	glEnd();

	glBegin(GL_QUADS);
	//scroll bar
		glVertex3i(win->len_x-13,35+y,0);
		glVertex3i(win->len_x-7,35+y,0);
		glVertex3i(win->len_x-7,35+len+y,0);
		glVertex3i(win->len_x-13,35+len+y,0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	
	return 1;
}

void display_rules_window()
{
	if(rules_win<=0){
		rules_win=create_window("Rules", 0, 0, rules_win_x, rules_win_y, rules_win_x_len, rules_win_y_len, ELW_TITLE_NAME|ELW_TITLE_BAR|ELW_CLOSE_BOX|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW);
		set_window_handler(rules_win, ELW_HANDLER_DISPLAY, &display_rules_handler);
		set_window_handler(rules_win, ELW_HANDLER_MOUSEOVER, &mouseover_rules_handler);
		set_window_handler(rules_win, ELW_HANDLER_CLICK, &click_rules_handler);
	} else {
		show_window(rules_win);
		select_window(rules_win);
	}
}

void toggle_rules_window(int toggle)
{
	if(!rules.no) {
		log_to_console(c_red2,"Rules were not found!");
		return;
	}
	
	if(last_display<=0||display_rules==NULL){
		if(display_rules)free_rules(display_rules);
		display_rules=get_interface_rules((float)(rules_win_x_len-60)/(12*0.8f));
	}
	if(toggle && rules_win>0) toggle_window(rules_win);
	else display_rules_window();

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

void highlight_rule(int type, Uint8 * rule, int no)
{
	int i,j;
	int r;
	int cur_rule;
	
	if(type==RULE_WIN)toggle_rules_window(0);
	else if(type==RULE_INTERFACE){
		init_rules_interface(rule[2], 1.0f, *((Uint16*)(rule)));
		interface_mode=interface_rules;
		rule+=3;
		no-=3;
	} else return; //Hmm...
	
	reset_rules(display_rules);
	
	if(display_rules) {
		for(i=0;i<no;i++){
			cur_rule=rule[i];
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
				return;
			}
	}
}

rule_string * get_interface_rules(int chars_per_line)
{
	int i;
	rule_string * _rules=(rule_string*)malloc((rules.no+1)*sizeof(rule_string));
	
	for(i=0;i<rules.no;i++){
		_rules[i].type=rules.rule[i].type;
		_rules[i].short_str=get_lines(rules.rule[i].short_desc,chars_per_line);
		_rules[i].long_str=get_lines(rules.rule[i].long_desc,chars_per_line);
	}

	_rules[rules.no].type=-1;

	reset_rules(_rules);
	
	return _rules;
}

int draw_rules(rule_string * rules_ptr, int rules_no, int x_in, int y_in, int lenx, int leny, float text_size)
{
	int xdiff=0,ydiff=18,i,j=0,tmplen=0,len=0;
	char str[200];
	char *ptr;
	float zoom=text_size;
	int x=0, y=y_in;
	
	if(rules_ptr[1].type==-1) reached_end=1;
	
	for(i=0;i<20 && y<leny;i++){
		ptr=str;
		len=0;
		switch(rules_ptr[i].type){
			case -1:
				rules_ptr[i].y_start=2*leny;//hehe ;-)
				return i;
			case TITLE:
				glColor3f(1.0f,0.0f,0.0f);
				zoom=text_size*1.5f;
				ydiff=30*zoom;
				xdiff=0;
				x=x_in+((lenx-x_in)>>1)-(strlen(rules_ptr[i].short_str[0])>>1)*11*zoom;
				break;
			case RULE:
				if(rules_ptr[i].highlight) glColor3f(1.0f,0.0f,0.0f);
				else if(rules_ptr[i].mouseover) glColor3f(0.7f,1.0f,0.0f);
				else glColor3f(0.0f,1.0f,0.0f);
				sprintf(str,"%d: ",rules_no++);
				ptr+=strlen(str);
				zoom=text_size;
				xdiff=(ptr-str)*11*zoom;
				x=x_in+20;
				ydiff=20*zoom;
				break;
			case INFO:
				if(rules_ptr[i].mouseover) glColor3f(0.0f,0.5f,1.0f);
				else glColor3f(0.0f,0.0f,1.0f);
				zoom=text_size;
				x=x_in+20;
				y+=10*zoom;
				ydiff=20*zoom;
				xdiff=0;
				break;
		}
		rules_ptr[i].x_start=x;
		rules_ptr[i].y_start=y;
		for(j=0;rules_ptr[i].short_str[j];j++){//Draw the lines
			if(j==1)ptr=str;
			if(j) y+=18*zoom;
			strcpy(ptr,rules_ptr[i].short_str[j]);
			if(!j)draw_string_zoomed(x,y,str,0,zoom);
			else draw_string_zoomed(x+xdiff,y,str,0,zoom);
			tmplen=strlen(str)*11.25*zoom;
			if(tmplen>len)len=tmplen;
		}
		y+=ydiff;
		rules_ptr[i].y_end=y;
		if(rules_ptr[i].show_long_desc && rules_ptr[i].long_str){//Draw the lines of the long description
			if(rules_ptr[i].highlight) glColor3f(0.8f,0.0f,0.0f);
			else glColor3f(0.6f,1.0f,1.0f);
			for(j=0;rules_ptr[i].long_str[j]&& y<leny;j++){
				if(j)y+=18*zoom*0.9f;
				strcpy(str,rules_ptr[i].long_str[j]);
				draw_string_zoomed(x+xdiff+10,y,str,0,zoom*0.9f);
			}
			y+=ydiff;
		}
		rules_ptr[i].x_end=rules_ptr[i].x_start+len;
	}
	rules_ptr[i].y_start=2*leny;//hehe ;-)
	
	return i;//The number of rules we're displaying
}

void check_mouse_rules_interface(rule_string * rules_ptr, int lenx, int leny, int mx, int my)
{
	int i;
	for(i=0;i<20 && rules_ptr[i].type!=-1 && rules_ptr[i].y_start<leny;i++){
		if(mx>rules_ptr[i].x_start && mx<rules_ptr[i].x_end &&
				my>rules_ptr[i].y_start && my<rules_ptr[i].y_end){
			if(left_click==1) {
				rules_ptr[i].show_long_desc=!rules_ptr[i].show_long_desc;
				left_click=2;
			}
			if(rules_ptr[i].long_str)rules_ptr[i].mouseover=1;
		} else rules_ptr[i].mouseover=0;
	}
}

/*Root window*/

void init_rules_interface(int next, float text_size, int count)
{
	next_interface=next;
	if(rules.no){
		if(last_display){//We need to format the rules again..
			if(display_rules) free_rules(display_rules);
			display_rules=get_interface_rules((float)(window_width-60)/(12*text_size));
		}
		countdown=count;//Countdown in 0.5 seconds...
	} else {
		countdown=0;//Just switch now, there are no rules
	}

	last_display=0;
}


void draw_rules_interface()
{
	char str[20];
	if(countdown==-1) init_rules_interface(disconnected?interface_opening:interface_new_char, 1.0f, 10);
	if(countdown==0) {
		interface_mode=next_interface;
		free_rules(display_rules);
		display_rules=NULL;
		if(next_interface==interface_opening)connect_to_server();
		return;
	}
	if(!rules.no) interface_mode=next_interface;

	draw_rules(display_rules, 1, 20,20,window_width,window_height-50*((float)window_width/640.0f),1.0f);
	
	glColor3f(0.6f,1.0f,1.0f);
	sprintf(str,"%d",countdown/2);
	draw_string(10,20,str,0);

	check_mouse_rules_interface(display_rules, window_width, window_height, mouse_x, mouse_y);
}

