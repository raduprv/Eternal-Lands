#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "manufacture.h"
#include "asc.h"
#include "cursors.h"
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
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

#define MAX_RECIPE 11
#define NUM_MIX_SLOTS 6
#define MIX_SLOT_OFFSET 36
#define SHOW_MAX_RECIPE (MAX_RECIPE-1)

int recipe_status[MAX_RECIPE];
item recipes[MAX_RECIPE][6];
item *manu_recipe=recipes[SHOW_MAX_RECIPE];
int cur_recipe=0;
item manufacture_list[ITEM_NUM_ITEMS];
int manufacture_win= -1;
int recipe_win= -1;
int recipes_shown=0;
static int recipes_loaded=0;
static int mouse_over_recipe = -1;
int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;

static char items_string[350]={0};
static size_t last_items_string_id = 0;

/* vars for saving recipe names */
static char *recipe_name[SHOW_MAX_RECIPE];
static item last_mix[NUM_MIX_SLOTS];
static int recipe_names_changed = 0;
static int initialised_recipe_names = 0;

/* called on client exit to free memory and clean up */
void cleanup_manufacture(void)
{
	size_t i;
	for (i=0; i<SHOW_MAX_RECIPE; i++)
		if (recipe_name[i] != NULL)
		{
			free(recipe_name[i]);
			recipe_name[i] = NULL;
		}
}

/* initialse recipe name vars */
static void init_recipe_names(void)
{
	size_t i;
	if (initialised_recipe_names)
		return;
	for (i=0; i<SHOW_MAX_RECIPE; i++)
		recipe_name[i] = NULL;
	for (i=0; i<NUM_MIX_SLOTS; i++)
		last_mix[i].quantity = 0;
	initialised_recipe_names = 1;
}

/* create a new recipe name entry */
static void new_recipe_name(size_t recipe_no, const char *name)
{
	size_t len = strlen(name);
	if ((recipe_no >= SHOW_MAX_RECIPE) || (recipe_name[recipe_no] != NULL))
		return;
	recipe_name[recipe_no] = (char *)malloc(len+1);
	safe_strncpy(recipe_name[recipe_no], name, len+1);
	recipe_names_changed = 1;
}

/* save recipe names to file if any have changed */
static void save_recipe_names(void)
{
	char fname[128];
	FILE *fp;
	size_t i;
	int errorflag = 0;

	if (!recipe_names_changed)
		return;

	safe_snprintf(fname, sizeof(fname), "recipes_%s.names",username_str);
	my_tolower(fname);
	fp = open_file_config(fname,"w");
	if(fp == NULL)
	{
		LOG_ERROR("%s() %s \"%s\": %s\n", __FUNCTION__, cant_open_file, fname, strerror(errno));
		return;
	}

	for (i=0; i<SHOW_MAX_RECIPE; i++)
	{
		if (recipe_name[i] != NULL)
		{
			if (fputs(recipe_name[i], fp) < 0)
			{
				errorflag = 1;
				break;
			}
		}
		if (fputc('\n', fp) != '\n')
		{
			errorflag = 1;
			break;
		}
	}
	if (errorflag)
		LOG_ERROR("%s() %s \"%s\": %s\n", __FUNCTION__, cant_open_file, fname, strerror(errno));

	fclose(fp);
}

/* load saved recipe names from file */
static void load_recipe_names(void)
{
	char fname[128];
	FILE *fp;
	char line [128];
	size_t recipe_no;

	recipe_names_changed = 0;

	safe_snprintf(fname, sizeof(fname), "recipes_%s.names",username_str);
	my_tolower(fname);
	fp = open_file_config(fname,"r");
	if(fp == NULL)
	{
		LOG_ERROR("%s() %s \"%s\": %s\n", __FUNCTION__, cant_open_file, fname, strerror(errno));
		return;
	}

	recipe_no = 0;
 	while (fgets(line, sizeof(line), fp) != NULL)
	{
		size_t len = strlen(line);
		while ((len > 0) && ((line[len-1] == '\r') || (line[len-1] == '\n') || (line[len-1] == ' ')))
		{
			line[len-1] = '\0';
			len--;
		}
		if (len > 0)
			new_recipe_name(recipe_no, line);
		recipe_no++;
	}

	fclose(fp);
}

/* each time a mix is called, save the ingredients - so we can compare later */
static void save_last_mix(void)
{
	size_t i;
	for (i=MIX_SLOT_OFFSET;i<MIX_SLOT_OFFSET+NUM_MIX_SLOTS;i++)
		if (manufacture_list[i].quantity > 0)
			last_mix[i-MIX_SLOT_OFFSET] = manufacture_list[i];
}

/* if the name is set, delete it */
static void clear_recipe_name(size_t recipe_no)
{
	if ((recipe_no < SHOW_MAX_RECIPE) && (recipe_name[cur_recipe] != NULL))
	{
		free(recipe_name[cur_recipe]);
		recipe_name[cur_recipe] = NULL;
		recipe_names_changed = 1;
	}
}

/*	Compare the last mixed ingredients to all the recipes without a name.
 *	If one matches then set the name to the provided string.
 */
void check_for_recipe_name(const char *name)
{
	size_t i, recipe_index;
	int num_last_ing = 0;

	// exit now if no last recipe
	for (i=0; i<NUM_MIX_SLOTS; i++)
		if (last_mix[i].quantity > 0)
			num_last_ing++;
	if (!num_last_ing)
		return;

	// check any not-set recipe name to see if the recipe matches the last_mix
	for (recipe_index=0; recipe_index<SHOW_MAX_RECIPE; recipe_index++)
	{
		int num_recipe_ing = 0;
		int num_match_ing = 0;
		size_t recipe_slot_index;
		item last_mix_cpy[NUM_MIX_SLOTS];
		item *recipe = recipes[recipe_index];

		// move on if already have name, no recipe or if the ingredient counts don't match
		if (recipe_name[recipe_index] != NULL)
			continue;
		for (i=0; i<NUM_MIX_SLOTS; i++)
			if (recipe[i].quantity > 0)
				num_recipe_ing++;
		if (!num_recipe_ing || (num_recipe_ing!=num_last_ing))
			continue;

		// allowing for any order, check if ingredients are the same
		memcpy(last_mix_cpy, last_mix, sizeof(item)*NUM_MIX_SLOTS);
		for (recipe_slot_index=0; recipe_slot_index<NUM_MIX_SLOTS; recipe_slot_index++)
		{
			size_t last_mix_slot_index;
			if (recipe[recipe_slot_index].quantity < 1)
				continue;
			for (last_mix_slot_index=0; last_mix_slot_index<NUM_MIX_SLOTS; last_mix_slot_index++)
				if ((last_mix_cpy[last_mix_slot_index].quantity > 0) &&
					(recipe[recipe_slot_index].quantity == last_mix_cpy[last_mix_slot_index].quantity) &&
					(recipe[recipe_slot_index].image_id == last_mix_cpy[last_mix_slot_index].image_id) &&
					(recipe[recipe_slot_index].id == last_mix_cpy[last_mix_slot_index].id))
				{
					last_mix_cpy[last_mix_slot_index].quantity = 0;
					num_match_ing++;
					break;
				}
		}

		// if ingredients are the same, eureka!
		if (num_match_ing == num_recipe_ing)
		{
			new_recipe_name(recipe_index, name);
			break;
		}
	}

	/* clear the last mix in all cases */
	for (i=0; i<NUM_MIX_SLOTS; i++)
		last_mix[i].quantity = 0;
}

void load_recipes (){
	char fname[128];
	FILE *fp;

	init_recipe_names();

	if (recipes_loaded) {
		/*
		 * save existing recipes instead of loading them if we are already logged in
		 * this will take place when relogging after disconnection
		 */
		save_recipes();
		return;
	}

	recipes_loaded=1;
	memset (recipes, 0, sizeof (recipes));

	safe_snprintf(fname, sizeof(fname), "recipes_%s.dat",username_str);
	my_tolower(fname);
	fp = open_file_config(fname,"rb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	if (fread (recipes,sizeof(recipes),1, fp) != 1)
		LOG_ERROR("%s() read failed for file [%s]\n", __FUNCTION__, fname);
	fclose (fp);

	load_recipe_names();
}

void save_recipes(){
	char fname[128];
	FILE *fp;

	if (!recipes_loaded)
		return;

	save_recipe_names();

	safe_snprintf(fname, sizeof(fname), "recipes_%s.dat",username_str);
	my_tolower(fname);
	fp=open_file_config(fname,"wb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	fwrite(recipes, sizeof(recipes), 1, fp);

	fclose(fp);
}

void build_manufacture_list()
{
	int i,j,l,k;

	for(i=0;i<36+6;i++)manufacture_list[i].quantity=0;

	//ok, now see which items are resources
	j=0;
	for(i=0;i<ITEM_WEAR_START;i++) {
		if(item_list[i].quantity && item_list[i].is_resource) {
			manufacture_list[j].quantity=item_list[i].quantity;
			manufacture_list[j].image_id=item_list[i].image_id;
			manufacture_list[j].pos=item_list[i].pos;
			j++;
		}
	}
	//now check for all items in all recipes
	for(k=0;k<MAX_RECIPE;k++){
		l=1;
		for(i=0; l>0 && i<6; i++) {
			if(recipes[k][i].quantity > 0) {
				for(j=0; l>0 && j<36; j++) {
					if(manufacture_list[j].quantity>0 && recipes[k][i].image_id == manufacture_list[j].image_id) {
						if(recipes[k][i].quantity > manufacture_list[j].quantity) {
							l=0;	// can't make
						}
						break;
					}
				}
				
				// watch for the item missing
				if(j >= 36) {
					l=0;
				}
			}
		}
		//updates recipe_status
		recipe_status[k]=l;
	}
	//all there? good, put them in from current recipe
	l=recipe_status[SHOW_MAX_RECIPE];
	if(l>0) {
		for(i=0; i<6; i++) {
			if(manu_recipe[i].quantity > 0)	{
				for(j=0;j<36;j++){
					if(manufacture_list[j].quantity>0 && manufacture_list[j].quantity>=manu_recipe[i].quantity && manufacture_list[j].image_id==manu_recipe[i].image_id){
						//found an empty space in the "production pipe"
						manufacture_list[j].quantity-=manu_recipe[i].quantity;
						manufacture_list[i+36].quantity=manu_recipe[i].quantity;
						manufacture_list[i+36].pos=manufacture_list[j].pos;
						manufacture_list[i+36].image_id=manufacture_list[j].image_id;
						break;
					}
				}
			} else {
				manufacture_list[i+36].quantity = 0;
			}
		}
	}

}

void copy_recipe()
{
	int i;

	for(i=36;i<36+6;i++){
		manu_recipe[i-36]=manufacture_list[i];
	}
}

static void use_recipe(int recipe_to_use)
{
	memcpy(manu_recipe, recipes[recipe_to_use], sizeof(item)*6);
}


//DRAWING FUNCTIONS

void draw_recipe_controls(){
	int wpx=33*6+2;
	int wpy=manufacture_menu_y_len-37;
	int lpx=18;
	int lpy=33;

	if (recipes_shown){
	/* Up arrow */	
		glBegin(GL_QUADS);	
			glVertex3i(wpx+lpx/2, wpy+lpy-10, 0); //top
			glVertex3i(wpx+5, wpy+lpy, 0); //left
			glVertex3i(wpx+lpx-5, wpy+lpy, 0); //right
			glVertex3i(wpx+lpx/2, wpy+lpy-10, 0); //top
		glEnd();
	} else {
		/* Dn arrow */	
		glBegin(GL_QUADS);
			glVertex3i(wpx+lpx/2, wpy+lpy, 0); //top
			glVertex3i(wpx+5, wpy+lpy-10, 0); //left
			glVertex3i(wpx+lpx-5, wpy+lpy-10, 0); //right
			glVertex3i(wpx+lpx/2, wpy+lpy, 0); //top
		glEnd();
	}	

	/* Add btn */	
	glEnable(GL_TEXTURE_2D);
	draw_string_zoomed(wpx+3, wpy-2, (unsigned char *)"+", 1, 1);
}

//draws a 6x1 grid of items+grid
int draw_production_pipe(int x, int y, int recipe_num){
	int i,ofs,valid;
	Uint8 str[80];
	item *the_list;

	//if recipe_num is negative we draw the current manufacture_list, else a recipe	
	if (recipe_num<0) {
		the_list=manufacture_list;
		ofs=36;
		valid=1;
	} else {
		the_list=recipes[recipe_num];
		ofs=0;
		valid=recipe_status[recipe_num];
	}

	glEnable(GL_TEXTURE_2D);
	//ok, now let's draw the mixed objects
	for(i=ofs;i<6+ofs;i++) {
		glColor3f(1.0f,1.0f,1.0f);
		if(the_list[i].quantity > 0){
			int x_start,y_start;

			//get the x and y
			x_start=x+33*(i%6)+1;
			y_start=y;

			draw_item(the_list[i].image_id,x_start,y_start,33);
			safe_snprintf((char *)str, sizeof(str), "%i", the_list[i].quantity);
			draw_string_small_shadowed(x_start,y_start+17,str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);

			if (!valid)
				gray_out(x_start,y_start,33);
		}
	}
	glDisable(GL_TEXTURE_2D);
	
	//draw the grid, in red if selected
	if (recipe_num==cur_recipe) glColor3f(1.0f,0.0f,0.0f);
	else glColor3f(0.77f,0.57f,0.39f);
	rendergrid(6,1,x, y, 33, 33);

	glEnable(GL_TEXTURE_2D);
	return 1;
}

int	display_manufacture_handler(window_info *win)
{
	Uint8 str[80];
	int i;

	//dirty hack for opacity
	//if manufacture_win is opaque then recipe_win should be and viceversa
	if (recipes_shown) win->opaque=windows_list.window[recipe_win].opaque;
	else windows_list.window[recipe_win].opaque=win->opaque;	

	glColor3f(0.77f,0.57f,0.39f);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=35;i>=0;i--){
		if(manufacture_list[i].quantity > 0) {
			int x_start,y_start;

			//get the x and y
			x_start=33*(i%12)+1;
			y_start=33*(i/12);

			draw_item(manufacture_list[i].image_id,x_start,y_start,33);
			
			safe_snprintf((char *)str, sizeof(str), "%i",manufacture_list[i].quantity);
			draw_string_small_shadowed(x_start, (i&1)?(y_start+17):(y_start+7), (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	//ok, now let's draw the mixed objects
	draw_production_pipe(2,manufacture_menu_y_len-37, -1);

	//now, draw the inventory text, if any.
	if (last_items_string_id != inventory_item_string_id)
	{		
		put_small_text_in_box((unsigned char*)inventory_item_string, strlen(inventory_item_string), win->len_x-8, items_string);
		last_items_string_id = inventory_item_string_id;
	}
	draw_string_small(4,manufacture_menu_y_len-85,(unsigned char *)items_string,4);

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	
	//draw the grid
	rendergrid(12,3,0,0,33,33);
	
	//Draw the bottom grid - NOT NEEDED, DONE IN draw_production_pipe
	//rendergrid(6,1,0, manufacture_menu_y_len-37, 33, 33);

	//Draw recipe control buttons
	draw_recipe_controls();
	glEnable(GL_TEXTURE_2D);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}



int recipe_dropdown_draw(window_info *win){
	int i;

	for (i=0;i<SHOW_MAX_RECIPE;i++){
		draw_production_pipe(0,33*i,i);
	}
	draw_production_pipe(0,33*cur_recipe,cur_recipe);

	if ((cur_recipe < SHOW_MAX_RECIPE) && (recipe_name[cur_recipe] != NULL))
		show_help(recipe_name[cur_recipe], win->len_x+5, 33*cur_recipe+(33-SMALL_FONT_Y_LEN)/2);

	if (mouse_over_recipe != -1)
	{
		if ((mouse_over_recipe != cur_recipe) && (mouse_over_recipe < SHOW_MAX_RECIPE) &&
			(recipe_name[mouse_over_recipe] != NULL))
			show_help(recipe_name[mouse_over_recipe], win->len_x+5, 33*mouse_over_recipe+(33-SMALL_FONT_Y_LEN)/2);
		if (show_help_text)
		{
			show_help(recipe_select_str, 0, win->len_y+10);
			show_help(recipe_load_str, 0, win->len_y+10+SMALL_FONT_Y_LEN);
		}
	}
	mouse_over_recipe = -1;

	return 1;
}




//CLICK HANDLERS
int recipe_dropdown_click_handler(window_info *win, int mx, int my, Uint32 flags){

	static int last_clicked=0;
	static int last_recipe=0;

	if (flags & ELW_WHEEL_UP) {
		cur_recipe=(cur_recipe-1+SHOW_MAX_RECIPE)%SHOW_MAX_RECIPE;			
	} else
	if (flags & ELW_WHEEL_DOWN) {
		cur_recipe=(cur_recipe+1)%SHOW_MAX_RECIPE;			
	} else {
		//normal click
		select_window(recipe_win);
		cur_recipe=my/(33+1);	
		if ( ((SDL_GetTicks() - last_clicked) < 400)&& last_recipe==cur_recipe){
			//double click on the same recipe to select it and close the dropdown
			use_recipe(cur_recipe);
			recipes_shown=0;
			hide_window(recipe_win);
		}	
		last_clicked = SDL_GetTicks();
	}
	build_manufacture_list();
	last_recipe = cur_recipe;
	do_click_sound();
	return 1;
}

int recipe_controls_click_handler(int mx, int my, Uint32 flags){

	int i;
	int wpx=33*6+2;
	int wpy=manufacture_menu_y_len-37;
	int lpx=18;
	int lpy=33;	

	if (mx>wpx&&mx<wpx+lpx&&my>wpy+lpy-10&&my<wpy+lpy){
		//arrow
		if (flags & ELW_WHEEL_UP) {
			cur_recipe=(cur_recipe-1+SHOW_MAX_RECIPE)%SHOW_MAX_RECIPE;			
			use_recipe(cur_recipe);
		} else
		if (flags & ELW_WHEEL_DOWN) {
			cur_recipe=(cur_recipe+1)%SHOW_MAX_RECIPE;			
			use_recipe(cur_recipe);
		} else {
			//normal click	
			recipes_shown=!recipes_shown;
		}
		
		build_manufacture_list();
		if (recipes_shown) show_window(recipe_win);
		else hide_window(recipe_win);
		do_click_sound();
	} else
	if (mx>wpx+3&&mx<wpx+lpx-3&&my>wpy&&my<wpy+15){
		//+ button
		//copy the recipe
		for(i=36;i<36+6;i++)
			recipes[cur_recipe][i-36]=manufacture_list[i];
		clear_recipe_name(cur_recipe);
		build_manufacture_list();
		do_click_sound();
		// save recipes to disk to avoid loss on disconnects/crashes
		save_recipes();
	}
	return 0;
}

int click_manufacture_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	static int last_slot=-1;
	Uint8 str[100];

	int quantitytomove=1;
 
	if ((flags & ELW_CTRL) || (flags & ELW_SHIFT) || (flags & ELW_ALT))
		quantitytomove = 10;

	/* if the eye cursor is active and we right click, change to standard walk */
	if(action_mode==ACTION_LOOK && (flags&ELW_RIGHT_MOUSE))
		action_mode = ACTION_WALK;

	//see if we clicked on any item in the main category
	pos=get_mouse_pos_in_grid(mx, my, 12, 3, 0, 0, 33, 33);

	if (pos >= 0 && manufacture_list[pos].quantity > 0)
	{
		if(action_mode==ACTION_LOOK || (flags&ELW_RIGHT_MOUSE)) {
			str[0]=LOOK_AT_INVENTORY_ITEM;
			str[1]=manufacture_list[pos].pos;
			my_tcp_send(my_socket,str,2);
			return 1;
		} else	{
			int j;

			last_slot=-1;
			
			for(j=36;j<36+6;j++)
				if(manufacture_list[j].pos==manufacture_list[pos].pos && manufacture_list[j].quantity > 0){
					//found an empty space in the "production pipe"
					if (flags & ELW_WHEEL_UP) {
						if (manufacture_list[j].quantity < quantitytomove)
							quantitytomove = -manufacture_list[j].quantity;
						else quantitytomove*=-1;
					} else
					if (manufacture_list[pos].quantity < quantitytomove)
						quantitytomove = manufacture_list[pos].quantity;
					manufacture_list[j].quantity += quantitytomove;
					manufacture_list[j].pos=manufacture_list[pos].pos;
					manufacture_list[j].image_id=manufacture_list[pos].image_id;
					manufacture_list[pos].quantity -= quantitytomove;
					copy_recipe();
					return 1;
				}


			for(j=36;j<36+6;j++)
				if(!manufacture_list[j].quantity > 0){
					//found an empty space in the "production pipe"
					if (flags & ELW_WHEEL_UP) return 1; //quantity already 0 in production pipeline
					if (manufacture_list[pos].quantity < quantitytomove)
						quantitytomove = manufacture_list[pos].quantity;
					manufacture_list[j].quantity += quantitytomove;
					manufacture_list[j].pos=manufacture_list[pos].pos;
					manufacture_list[j].image_id=manufacture_list[pos].image_id;
					manufacture_list[pos].quantity -= quantitytomove;
					copy_recipe();
					return 1;
				}
		}
	} else 
	if (pos>=0) last_slot=-1;

	//see if we clicked on any item from the "production pipe"
	pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, manufacture_menu_y_len-37, 33, 33);

	if (pos >= 0 && manufacture_list[36+pos].quantity > 0)
	{
		if(action_mode==ACTION_LOOK || (flags&ELW_RIGHT_MOUSE)){
			str[0]=LOOK_AT_INVENTORY_ITEM;
			str[1]=manufacture_list[36+pos].pos;
			my_tcp_send(my_socket,str,2);
			return 1;
		} else {
			int j;

			last_slot=pos;
			for(j=0;j<36;j++)
				if(manufacture_list[j].quantity && manufacture_list[j].pos==manufacture_list[36+pos].pos){
					//found item in ingredients slot, move from "production pipe" back to this slot
					if (flags & ELW_WHEEL_DOWN) {
						if (manufacture_list[36+pos].quantity < quantitytomove)
							quantitytomove = -manufacture_list[36+pos].quantity;
						else quantitytomove*=-1;
					} else
					if (manufacture_list[36+pos].quantity < quantitytomove)
						quantitytomove = manufacture_list[36+pos].quantity;
					manufacture_list[j].quantity += quantitytomove;
					manufacture_list[j].pos=manufacture_list[36+pos].pos;
					manufacture_list[j].image_id=manufacture_list[36+pos].image_id;
					manufacture_list[36+pos].quantity -= quantitytomove;
					copy_recipe();
					return 1;
				}


			for(j=0;j<36;j++)
				if(!manufacture_list[j].quantity){
					//found item in ingredients slot, move from "production pipe" back to this slot
					if (manufacture_list[36+pos].quantity < quantitytomove)
						quantitytomove = manufacture_list[36+pos].quantity;
					//handles mouse wheel
					if (flags & ELW_WHEEL_DOWN) return 1; //No more items to put in production pipe
					manufacture_list[j].quantity += quantitytomove;
					manufacture_list[j].pos=manufacture_list[36+pos].pos;
					manufacture_list[j].image_id=manufacture_list[36+pos].image_id;
					manufacture_list[36+pos].quantity -= quantitytomove;
					copy_recipe();
					return 1;
				}
		}
	} else
	if (pos>=0) {
		//click on an empty slot
		//handle the mouse wheel
		if (pos!=last_slot && ((flags&ELW_WHEEL_UP)||(flags&ELW_WHEEL_DOWN))) {
			//simulate a click on the dropdown
			last_slot=-1;
			recipe_dropdown_click_handler(win,0,0,flags);
			use_recipe(cur_recipe);
			build_manufacture_list();
			return 0;
		} 
	} else last_slot=-1;
	//see if we clicked on the recipe handler
	recipe_controls_click_handler(mx,my,flags);

	// clear the message area if double-clicked
	if ((my > manufacture_menu_y_len-85) && my < (manufacture_menu_y_len-37)) {
		static Uint32 last_click = 0;
		if (safe_button_click(&last_click)) {
			set_shown_string(0,"");
			return 1;
		}
	}

	return 0;
}



int mix_handler(Uint8 quantity, const char* empty_error_str)
{
	Uint8 str[32];
	int items_no=0;
	int i;
	int cannot_manu = 0;

	if(manufacture_win < 0)
		cannot_manu = 1;
	else {
		cannot_manu = 1;
		for(i=36;i<36+6;i++)
			if(manufacture_list[i].quantity > 0)
				cannot_manu = 0;
	}
	
	if (cannot_manu) {
		set_shown_string(c_red2, empty_error_str);
		return 1;
	}
	
	str[0]=MANUFACTURE_THIS;
	for(i=36;i<36+6;i++){
		if(manufacture_list[i].quantity > 0){
			str[items_no*3+2]=manufacture_list[i].pos;
			*((Uint16 *)(str+items_no*3+2+1))=SDL_SwapLE16(manufacture_list[i].quantity);
			items_no++;
		}
	}

	str[1]=items_no;
	if(items_no){
		//don't send an empty string
		save_last_mix();
		str[items_no*3+2]= quantity;
		my_tcp_send(my_socket,str,items_no*3+3);
	}
		
	return 1;
}

int clear_handler()
{
	int i;

	for(i=0; i<6; i++) manu_recipe[i].quantity= manu_recipe[i].image_id= 0; // clear the recipe
	build_manufacture_list();
	return 1;
}

/* ">" button clicked - manufacture just one item */
int mixone_handler()
{
  return mix_handler(1, mix_empty_str);
}

/* ">>" button clicked - manufacture the maxiumum nuber of items */
int mixall_handler()
{
  return mix_handler(255, mix_empty_str);
}


/* show help for recipe window */
int mouseover_recipe_handler(window_info *win, int mx, int my)
{
	mouse_over_recipe = my/(33+1);
	return 0;
}

//MOUSEOVER HANDLERS
int recipe_controls_mouseover_handler(int mx, int my){

	int wpx=33*6+2;
	int wpy=manufacture_menu_y_len-37;
	int lpx=18;
	int lpy=33;

	if (!show_help_text)
		return 0;

	if (mx>wpx&&mx<wpx+lpx&&my>wpy+lpy-10&&my<wpy+lpy){
		//on arrow
		show_help(recipe_show_hide_str, 0, manufacture_menu_y_len+10);
	} else
	if (mx>wpx+3&&mx<wpx+lpx-3&&my>wpy&&my<wpy+15){
		//on + button
		show_help(recipe_save_str, 0, manufacture_menu_y_len+10);
	}
	return 0;
}

/* mouse over slots - show tool tips */
int mouseover_manufacture_slot_handler(window_info *win, int mx, int my)
{
	int pos;
	int check_for_eye = 0;

	/* See if we're over a message - and offer clear help if so */
	if (show_help_text && *inventory_item_string && (my > manufacture_menu_y_len-85) && my < (manufacture_menu_y_len-37)) {
		show_help((disable_double_click)?click_clear_str :double_click_clear_str, 0, win->len_y+10);
	}

	/* see if we clicked on any item in the main category */
	pos=get_mouse_pos_in_grid(mx, my, 12, 3, 0, 0, 33, 33);
	if (pos >= 0 && manufacture_list[pos].quantity > 0){
		if (show_help_text)
			show_help(manu_add_str, 0, manufacture_menu_y_len+10);
		check_for_eye = 1;
	}

	/* see if we clicked on any item from the "production pipe" */
	pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, manufacture_menu_y_len-37, 33, 33);
	if (pos >= 0 && manufacture_list[36+pos].quantity > 0){
		if (show_help_text)
			show_help(manu_remove_str, 0, manufacture_menu_y_len+10);
		check_for_eye = 1;	
	}

	/*check recipe controls*/
	if (show_help_text)
		recipe_controls_mouseover_handler(mx,my);

	/* if we're over an occupied slot and the eye cursor function is active, show the eye cursor */
	if (check_for_eye){
		if (action_mode == ACTION_LOOK){
			elwin_mouse = CURSOR_EYE;
			return 1;
		}
	}

	return 0;
}

/* mouse over ">" button - show tool tip */
int mouseover_mixone_handler(window_info *win, int mx, int my)
{
	if (show_help_text)
		show_help(mix_str, 33*6+25, manufacture_menu_y_len+10);
	return 0;
}

/* mouse over ">>" button - show tool tip */
int mouseover_mixall_handler(window_info *win, int mx, int my)
{
	if (show_help_text)
		show_help(mixall_str, 33*8-5, manufacture_menu_y_len+10);
	return 0;
}


void display_manufacture_menu()
{
	if(manufacture_win < 0){
		static int clear_button_id=100;
		static int mixone_button_id=101;
		static int mixall_button_id=102;

		int our_root_win = -1;
			
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		manufacture_win= create_window(win_manufacture, our_root_win, 0, manufacture_menu_x, manufacture_menu_y, manufacture_menu_x_len, manufacture_menu_y_len, ELW_WIN_DEFAULT);
		set_window_handler(manufacture_win, ELW_HANDLER_DISPLAY, &display_manufacture_handler );
		set_window_handler(manufacture_win, ELW_HANDLER_CLICK, &click_manufacture_handler );
		set_window_handler(manufacture_win, ELW_HANDLER_MOUSEOVER, &mouseover_manufacture_slot_handler );

		mixone_button_id=button_add_extended(manufacture_win, mixone_button_id,
			NULL, 33*6+15+10, manufacture_menu_y_len-36, 43, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, ">");
		widget_set_OnClick(manufacture_win, mixone_button_id, mixone_handler);
		widget_set_OnMouseover(manufacture_win, mixone_button_id, mouseover_mixone_handler);
		
		mixall_button_id=button_add_extended(manufacture_win, mixall_button_id,
			NULL, 33*8+10, manufacture_menu_y_len-36, 43, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, ">>");
		widget_set_OnClick(manufacture_win, mixall_button_id, mixall_handler);
		widget_set_OnMouseover(manufacture_win, mixall_button_id, mouseover_mixall_handler);
		
		clear_button_id=button_add_extended(manufacture_win, clear_button_id, NULL, 33*9+18+10, manufacture_menu_y_len-36, 70, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, clear_str);
		widget_set_OnClick(manufacture_win, clear_button_id, clear_handler);

		//Create a child window to show recipes in a dropdown panel
		recipe_win= create_window("w_recipe", manufacture_win, 0, 2, manufacture_menu_y_len-2, 33*6, SHOW_MAX_RECIPE*33, 
			ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE|ELW_USE_BORDER);
		set_window_handler(recipe_win, ELW_HANDLER_DISPLAY, &recipe_dropdown_draw);
		set_window_handler(recipe_win, ELW_HANDLER_CLICK, &recipe_dropdown_click_handler );
		set_window_handler(recipe_win, ELW_HANDLER_MOUSEOVER, &mouseover_recipe_handler );
		hide_window(recipe_win); //start hidden
	} else {
		show_window(manufacture_win);
		if (!recipes_shown) hide_window(recipe_win);
		else show_window(recipe_win);
		select_window(manufacture_win);
	}
}
