#include <stdlib.h>
#include "emotes.h"
#include "client_serv.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
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
#include "text.h"

#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

// TO ENABLE POSES, follow comments for EMOTES_CATEGORIES, emote_cats and emote_sel

#define EMOTES_CATEGORIES 1 //Set EMOTE_CATEGORIES to 5 for poses
#define EMOTES_SHOWN 9

#define EMOTE_SPAM_TIME 1000

#define SET_COLOR(x) glColor4f((float) colors_list[x].r1 / 255.0f,(float) colors_list[x].g1 / 255.0f,(float) colors_list[x].b1 / 255.0f,1.0f)

static char *emote_cats[EMOTES_CATEGORIES]= {
	"Actions"/*,
	"Sit poses",
	"Walk poses",
	"Run poses",
	"Stand poses"*/ //remove this comment and change EMOTE_CATEGORIES to 5 to enable poses
};

static int emotes_rect_x = 0;
static int emotes_rect_y = 0;
static int emotes_rect_x2 = 0;
static int emotes_rect_y2 = 0;

static int border_space = 0;
static int inbox_space = 0;
static int top_border = 0;
static int box_width = 0;
static int box_sep = 0;
static int category_y_step = 0;
static int EMOTES_SCROLLBAR_ITEMS = 1001;

static int cur_cat=0;
static emote_data* emote_sel[EMOTES_CATEGORIES]={NULL/*,NULL,NULL,NULL,NULL*/}; //remove the comment for poses
static emote_data *selectables[EMOTES_SHOWN];

static unsigned char emote_str1[100];
static unsigned char emote_str2[100];


void send_emote(int emote_id)
{
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

static int do_handler(void)
{
	if(emote_sel[cur_cat]) send_emote(emote_sel[cur_cat]->id);
	return 0;
}

static void update_selectables(void)
{
	int pos,i;
	hash_entry *he;

	i=0;
	pos=vscrollbar_get_pos(get_id_MW(MW_EMOTE), EMOTES_SCROLLBAR_ITEMS);
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

	emote_str1[1] = emote_str2[0] = 0;
	if(emote_sel[cur_cat]){
		put_small_colored_text_in_box_zoomed(c_orange2,
			(const unsigned char*)emote_sel[cur_cat]->desc,
			strlen(emote_sel[cur_cat]->desc), box_width, emote_str1, 1.0);
		hash_start_iterator(emote_cmds);
		while((he=hash_get_next(emote_cmds))){
			const emote_dict *emd = (const emote_dict*)he->item;
			if (emd->emote==emote_sel[cur_cat])
			{
				safe_snprintf((char*)emote_str2, sizeof(emote_str2), "%cTrigger: %c%s",
					127+c_grey1, 127+c_green3, emd->command);
				break; //just one command
			}
		}
	}

}

static int display_emotes_handler(window_info *win)
{

	int i,pos;
	actor *act = get_actor_ptr_from_id(yourself);
	static int last_pos=0;

	//check if vbar has been moved
	pos=vscrollbar_get_pos(win->window_id, EMOTES_SCROLLBAR_ITEMS);
	if(pos!=last_pos){
		last_pos=pos;
		update_selectables();
	}

	//draw texts
	glEnable(GL_TEXTURE_2D);

	SET_COLOR(c_orange1);
	draw_string_small_zoomed(border_space, top_border - win->small_font_len_y, (unsigned char*)"Categories",1, win->current_scale);
	draw_string_small_zoomed(border_space, top_border + emotes_rect_y + box_sep  - win->small_font_len_y, (unsigned char*)"Emotes",1, win->current_scale);

	for(i=0;i<EMOTES_CATEGORIES;i++){
		if(cur_cat==i) SET_COLOR(c_blue2);
		else glColor3f(1.0f, 1.0f, 1.0f);
		draw_string_small_zoomed(border_space + inbox_space, top_border + inbox_space + category_y_step * i, (unsigned char*)emote_cats[i],1, win->current_scale);
	}

	for(i=0;i<EMOTES_SHOWN;i++){
		if(emote_sel[cur_cat]==selectables[i]) SET_COLOR(c_blue2);
		else glColor3f(1.0f, 1.0f, 1.0f);
		if(cur_cat&&act&&selectables[i]==act->poses[cur_cat-1]) SET_COLOR(c_green1);
		if(selectables[i])
			draw_string_small_zoomed(border_space + inbox_space, top_border + emotes_rect_y + box_sep + inbox_space + category_y_step * i, (unsigned char*)selectables[i]->name,1, win->current_scale);
	}
	glColor3f(0.77f, 0.57f, 0.39f);
	//do grids
	glDisable(GL_TEXTURE_2D);

	rendergrid(1, 1, border_space, top_border, emotes_rect_x, emotes_rect_y);
	rendergrid(1, 1, border_space, top_border + emotes_rect_y + box_sep, emotes_rect_x2, emotes_rect_y2);
	glEnable(GL_TEXTURE_2D);


	//draw description
	if(emote_sel[cur_cat]){
		draw_string_small_zoomed(border_space, win->len_y - border_space - (3 * win->small_font_len_y), emote_str1,2, win->current_scale);
		draw_string_small_zoomed(border_space, win->len_y - border_space - (1 * win->small_font_len_y), emote_str2,1, win->current_scale);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static int click_emotes_handler(window_info *win, int mx, int my, Uint32 flags)
{
	static int last_clicked=0;
	static int last_pos=-1;
	int box_left = border_space;
	int box_right = box_left + emotes_rect_x2;
	int box_top = top_border + emotes_rect_y + box_sep;
	int box_bot = box_top + emotes_rect_y2;

	//scroll if wheel on selectables
	if(flags&ELW_WHEEL_UP) {
		if(mx > box_left && mx < box_right && my > box_top && my < box_bot)
			vscrollbar_scroll_up(win->window_id, EMOTES_SCROLLBAR_ITEMS);
		update_selectables();
		last_pos=-1;
		return 0;
	} else if(flags&ELW_WHEEL_DOWN) {
		if(mx > box_left && mx < box_right && my > box_top && my < box_bot)
			vscrollbar_scroll_down(win->window_id, EMOTES_SCROLLBAR_ITEMS);
		update_selectables();
		last_pos=-1;
		return 0;
	} else if ( (flags & ELW_MOUSE_BUTTON) == 0) {
		last_pos=-1;
		return 0;
	}


	if(mx > border_space && mx < border_space + emotes_rect_x && my > top_border && my < top_border + emotes_rect_y){
		//click on a cat
		cur_cat=(my - top_border) / category_y_step;
		if(cur_cat>=EMOTES_CATEGORIES) cur_cat=0;
		if(cur_cat>EMOTE_STANDING) cur_cat=EMOTE_STANDING+1;
		update_selectables();
		last_pos=-1;
	} else if(mx > box_left && mx < box_right && my > box_top && my < box_bot) {
		//click on selectables
		int w=(my - top_border - emotes_rect_y - box_sep)/ category_y_step;
		emote_sel[cur_cat]=selectables[(w>=EMOTES_SHOWN)?(EMOTES_SHOWN-1):(w)];
		update_selectables();
		if ( ((SDL_GetTicks() - last_clicked) < 300)&&last_pos==w) do_handler();
		last_pos=w;
	}

	last_clicked = SDL_GetTicks();
	return 0;
}

static int ui_scale_emotes_handler(window_info *win)
{
	float zoom = win->current_scale_small;
	int trigger_width = get_string_width_zoom((const unsigned char*)"Trigger: ",
		win->font_category, zoom);
	emote_dict *emd;
	hash_entry *he;

	box_width = 0;
	hash_start_iterator(emote_cmds);
	while ((he = hash_get_next(emote_cmds)))
	{
		emd = (emote_dict*)he->item;
		if (emd->emote)
		{
			int width = get_string_width_zoom((const unsigned char*)emd->emote->name,
				win->font_category, zoom);
			box_width = max2i(box_width, width);
			width = (get_string_width_zoom((const unsigned char*)emd->emote->desc,
				win->font_category, zoom) * 6) / 10;
			box_width = max2i(box_width, width);
			width = trigger_width + get_string_width_zoom((const unsigned char*)emd->command,
				win->font_category, zoom);
			box_width = max2i(box_width, width);
		}
	}

	inbox_space = (int)(0.5 + win->current_scale * 2);
	border_space = (int)(0.5 + win->current_scale * 5);
	top_border = border_space + win->small_font_len_y;
	box_sep = 2 * border_space + win->small_font_len_y;
	category_y_step = win->small_font_len_y;

	emotes_rect_x = 2 * inbox_space + box_width;
	emotes_rect_y = 2 * inbox_space + category_y_step * EMOTES_CATEGORIES;
	emotes_rect_x2 = 2 * inbox_space + box_width;
	emotes_rect_y2 = 2 * inbox_space + category_y_step * EMOTES_SHOWN;

	resize_window(win->window_id, emotes_rect_x2 + win->box_size + 2 * border_space,
		top_border + emotes_rect_y + box_sep + emotes_rect_y2 + 3 * win->small_font_len_y + 2 * border_space);

	widget_resize(win->window_id, EMOTES_SCROLLBAR_ITEMS, win->box_size, emotes_rect_y2);
	widget_move(win->window_id, EMOTES_SCROLLBAR_ITEMS, emotes_rect_x2 + border_space, emotes_rect_y + top_border + box_sep);

	return 0;
}

static int change_emotes_font_handler(window_info* win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	ui_scale_emotes_handler(win);
	return 1;
}

void display_emotes_menu(void)
{
	int emotes_win = get_id_MW(MW_EMOTE);

	if(emotes_win < 0){
		int num_emotes;
		hash_entry *he;
		emotes_win = create_window("Emotes", (not_on_top_now(MW_EMOTE) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_EMOTE), get_pos_y_MW(MW_EMOTE), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_EMOTE, emotes_win);
		set_window_custom_scale(emotes_win, MW_EMOTE);
		set_window_handler(emotes_win, ELW_HANDLER_DISPLAY, &display_emotes_handler );
		set_window_handler(emotes_win, ELW_HANDLER_CLICK, &click_emotes_handler );
		set_window_handler(emotes_win, ELW_HANDLER_UI_SCALE, &ui_scale_emotes_handler );
		set_window_handler(emotes_win, ELW_HANDLER_FONT_CHANGE, &change_emotes_font_handler);

		num_emotes = 0;
		hash_start_iterator(emotes);
		while((he=hash_get_next(emotes)))
			num_emotes++;

		EMOTES_SCROLLBAR_ITEMS = vscrollbar_add_extended(emotes_win, EMOTES_SCROLLBAR_ITEMS, NULL,
			0, 0, 0, 0, 0, 1.0, 0, 1, num_emotes-EMOTES_SHOWN);

		if (emotes_win >=0 && emotes_win < windows_list.num_windows)
			ui_scale_emotes_handler(&windows_list.window[emotes_win]);
		check_proportional_move(MW_EMOTE);

		update_selectables();

	} else {
		show_window(emotes_win);
		select_window(emotes_win);
	}
}



