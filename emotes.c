#include <stdlib.h>
#include "emotes.h"
#include "client_serv.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "global.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "multiplayer.h"
#include "textures.h"
#include "translate.h"
#include "sound.h"
#include "io/elpathwrapper.h"
#include "errors.h"
#include "items.h"
#include "actors.h"
#include "colors.h"


#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

//TO ENABLE POSES, CHECK COMMENTS at lines 31,40,59


#define EMOTES_SCROLLBAR_CATEGORIES 1000
#define EMOTES_SCROLLBAR_ITEMS 1001
//Set EMOTE_CATEGORIES to 5 for poses
#define EMOTES_CATEGORIES 1
#define EMOTES_SHOWN 9

#define EMOTE_SPAM_TIME 1000

#define SET_COLOR(x) glColor4f((float) colors_list[x].r1 / 255.0f,(float) colors_list[x].g1 / 255.0f,(float) colors_list[x].b1 / 255.0f,1.0f)

char *emote_cats[EMOTES_CATEGORIES]= {
	"Actions"/*,
	"Sit poses",
	"Walk poses",
	"Run poses",
	"Stand poses"*/ //remove this comment and change EMOTE_CATEGORIES to 5 to enable poses
};

int emotes_win= -1;
int emotes_menu_x=10;
int emotes_menu_y=20;
int emotes_menu_x_len=6*33+15;
int emotes_menu_y_len=290;
int emotes_rect_x=170;
int emotes_rect_y=80;
int emotes_rect_x2=150;
int emotes_rect_y2=120;


int cur_cat=0;
emote_data* emote_sel[EMOTES_CATEGORIES]={NULL/*,NULL,NULL,NULL,NULL*/}; //remove the comment for poses
emote_data *selectables[EMOTES_SHOWN];

unsigned char emote_str1[100];
unsigned char emote_str2[100];


void send_emote(int emote_id){
	static int last_emote_time=0;
	Uint8 str[4];

	if(cur_time-last_emote_time>EMOTE_SPAM_TIME) {
		//Send message to server...	
		str[0]=DO_EMOTE;
		str[1]=emote_id;
		my_tcp_send(my_socket,str,2);
		last_emote_time=cur_time;
		//printf("Emote %i sent at time %i\n",emote_id,last_emote_time);
	}

}

int do_handler(){
	if(emote_sel[cur_cat]) send_emote(emote_sel[cur_cat]->id);
	return 0;
}



void update_selectables(){
	int pos,i;
	hash_entry *he;

	i=0;
	pos=vscrollbar_get_pos(emotes_win, EMOTES_SCROLLBAR_ITEMS);
	memset(selectables,0,sizeof(emote_data*)*EMOTES_SHOWN);
	hash_start_iterator(emotes);
	while((he=hash_get_next(emotes))&&i<EMOTES_SHOWN){
		emote_data *emote;

		emote=((emote_data *)he->item);
		if(!cur_cat&&emote->pose>EMOTE_STANDING) {
			//emotes
			pos--;
			if(pos>=0) continue;
			selectables[i]=emote;
			i++;
		} else if(cur_cat&&emote->pose==(cur_cat-1)){
			//poses
			pos--;
			if(pos>=0) continue;
			selectables[i]=emote;
			i++;
		}
	}

	emote_str1[1]=emote_str2[0]=emote_str2[1]=0;
	if(emote_sel[cur_cat]){
		emote_dict *emd;

		emote_str1[0]=127+c_orange2;
		safe_strcat((char*)emote_str1,(char*)emote_sel[cur_cat]->desc,/*sizeof(emote_str1)*/23);
		hash_start_iterator(emote_cmds);
		while((he=hash_get_next(emote_cmds))){
			emd = (emote_dict*)he->item;
			if (emd->emote==emote_sel[cur_cat]){
				int ll;
				//draw command
				if(!emote_str2[0]) {
					emote_str2[0]=127+c_grey1;
					safe_strcat((char*)emote_str2,"Trigger:",10);
				}
				ll=strlen((char*)emote_str2);
				emote_str2[ll]=127+c_green3;
				emote_str2[ll+1]=emote_str2[ll+2]=' ';
				emote_str2[ll+3]=0;
				safe_strcat((char*)emote_str2,emd->command,/*sizeof(emote_str2)*/23);
				break; //just one command
			}
		}
	}
	
}

int display_emotes_handler(window_info *win){

	int i,pos;
	actor *act = get_actor_ptr_from_id(yourself);
	static int last_pos=0;



	//check if vbar has been moved
	pos=vscrollbar_get_pos(emotes_win, EMOTES_SCROLLBAR_ITEMS);
	if(pos!=last_pos){
		last_pos=pos;
		update_selectables();
	}

	//draw texts
	glEnable(GL_TEXTURE_2D);
	
	SET_COLOR(c_orange1);
	draw_string_small(20, 15, (unsigned char*)"Categories",1);
	draw_string_small(20, emotes_rect_y+30+5, (unsigned char*)"Emotes",1);

	for(i=0;i<EMOTES_CATEGORIES;i++){
		if(cur_cat==i) SET_COLOR(c_blue2);
		else glColor3f(1.0f, 1.0f, 1.0f);
		draw_string_small(23, 32+13*i, (unsigned char*)emote_cats[i],1);
	}

	for(i=0;i<EMOTES_SHOWN;i++){
		if(emote_sel[cur_cat]==selectables[i]) SET_COLOR(c_blue2);
		else glColor3f(1.0f, 1.0f, 1.0f);
		if(cur_cat&&act&&selectables[i]==act->poses[cur_cat-1]) SET_COLOR(c_green1);
		if(selectables[i])
			draw_string_small(23, 30+emotes_rect_y+20+1+13*i, (unsigned char*)selectables[i]->name,1);
	}
	glColor3f(0.77f, 0.57f, 0.39f);
	//do grids
	glDisable(GL_TEXTURE_2D);
		
	rendergrid(1, 1, 20, 30, emotes_rect_x, emotes_rect_y);
	rendergrid(1, 1, 20, 30+emotes_rect_y+20, emotes_rect_x2, emotes_rect_y2);
	glEnable(GL_TEXTURE_2D);


	//draw description
	if(emote_sel[cur_cat]){
		draw_string_small(20, emotes_menu_y_len-36, emote_str1,2);
		draw_string_small(20, emotes_menu_y_len-36+16, emote_str2,1);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;	
}


int click_emotes_handler(window_info *win, int mx, int my, Uint32 flags){
	static int last_clicked=0;
	static int last_pos=-1;

	//scroll if wheel on selectables
	if(flags&ELW_WHEEL_UP) {
		if(mx>20&&mx<20+emotes_rect_x2&&my>30+emotes_rect_y+20&&my<30+emotes_rect_y+20+emotes_rect_y2)
			vscrollbar_scroll_up(emotes_win, EMOTES_SCROLLBAR_ITEMS);
		update_selectables();
		last_pos=-1;
		return 0;
	} else if(flags&ELW_WHEEL_DOWN) {
		if(mx>20&&mx<20+emotes_rect_x2&&my>30+emotes_rect_y+20&&my<30+emotes_rect_y+20+emotes_rect_y2)
			vscrollbar_scroll_down(emotes_win, EMOTES_SCROLLBAR_ITEMS);
		update_selectables();
		last_pos=-1;
		return 0;
	} else if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		last_pos=-1;
		return 0;
	}


	if(mx>20&&mx<20+emotes_rect_x&&my>30&&my<30+emotes_rect_y){
		//click on a cat
		cur_cat=(my-30)/13;
		if(cur_cat>=EMOTES_CATEGORIES) cur_cat=0;
		if(cur_cat>EMOTE_STANDING) cur_cat=EMOTE_STANDING+1;
		update_selectables();
		last_pos=-1;
	} else if(mx>20&&mx<20+emotes_rect_x2&&my>30+20+emotes_rect_y&&my<30+emotes_rect_y+20+emotes_rect_y2) {
		//click on selectables
		int w=(my-30-emotes_rect_y-20)/13;
		emote_sel[cur_cat]=selectables[(w>=EMOTES_SHOWN)?(EMOTES_SHOWN-1):(w)];
		update_selectables();
		if ( ((SDL_GetTicks() - last_clicked) < 300)&&last_pos==w) do_handler();
		last_pos=w;
	}

	last_clicked = SDL_GetTicks();
	return 0;
}




void display_emotes_menu()
{
	if(emotes_win < 0){
		//static int do_button_id=100;
		int our_root_win = -1;
			
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		emotes_win= create_window("Emotes", our_root_win, 0, emotes_menu_x, emotes_menu_y, emotes_menu_x_len, emotes_menu_y_len, ELW_WIN_DEFAULT);
		set_window_handler(emotes_win, ELW_HANDLER_DISPLAY, &display_emotes_handler );
		set_window_handler(emotes_win, ELW_HANDLER_CLICK, &click_emotes_handler );
		vscrollbar_add_extended(emotes_win, EMOTES_SCROLLBAR_ITEMS, NULL, emotes_rect_x2+20, 30+emotes_rect_y+20, 20, emotes_rect_y2, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, 20);

		//do_button_id=button_add_extended(emotes_win, do_button_id, NULL, 33*9+18+10, emotes_menu_y_len-36, 70, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Do!");
		//widget_set_OnClick(emotes_win, do_button_id, do_handler);
		update_selectables();

	} else {
		show_window(emotes_win);
		select_window(emotes_win);
	}
}



