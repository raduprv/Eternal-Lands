#include <stdlib.h>
#include "global.h"
#include "elwindows.h"

actor *actor_under_mouse;
int object_under_mouse;
int thing_under_the_mouse;
int current_cursor;
int read_mouse_now=0;
int elwin_mouse=-1;

struct cursors_struct cursors_array[20];
struct harvest_names_struct harvestable_objects[100];
struct enter_names_struct entrable_objects[100];

Uint8 *cursors_mem=NULL;
int cursors_x_lenght;
int cursors_y_lenght;

void load_cursors()
{
	int f_size,cursors_colors_no,x,y,i;
	FILE *f = NULL;
	Uint8 * cursors_mem_bmp;
	Uint8 *handle_cursors_mem_bmp;
	Uint8 cur_color;
	f = fopen ("./textures/cursors.bmp", "rb");
	if(!f)
		{
			char str[100];
			sprintf(str,"%s %s",reg_error_str,cursors_file_str);
            		log_error(str);
			return;
		}
	fseek (f, 0, SEEK_END);
	f_size = ftell (f);
	//ok, allocate memory for it
	cursors_mem_bmp = (Uint8 *)calloc ( f_size, sizeof(char) );
	handle_cursors_mem_bmp=cursors_mem_bmp;
	fseek (f, 0, SEEK_SET);
	fread (cursors_mem_bmp, 1, f_size, f);
	fclose (f);

	cursors_mem_bmp += 18;		//x lenght is at offset+18
	cursors_x_lenght = *((int *) cursors_mem_bmp);
	cursors_mem_bmp += 4;		//y lenght is at offset+22
	cursors_y_lenght = *((int *) cursors_mem_bmp);
	cursors_mem_bmp += 46 - 22;
	cursors_colors_no = *((int *) cursors_mem_bmp);
	cursors_mem_bmp += 54 - 46 + cursors_colors_no * 4;

	//ok, now transform the bitmap in cursors info
	if(cursors_mem) free(cursors_mem);
	cursors_mem = (Uint8 *)calloc ( cursors_x_lenght*cursors_y_lenght*2, sizeof(char));

	for(y=cursors_y_lenght-1;y>=0;y--)
		{
			i=(cursors_y_lenght-y-1)*cursors_x_lenght;
			for(x=0;x<cursors_x_lenght;x++)
				{
					cur_color=*(cursors_mem_bmp+y*cursors_x_lenght+x);
					switch(cur_color) {
					case 0: //transparent
						*(cursors_mem+(i+x)*2)=0;
						*(cursors_mem+(i+x)*2+1)=0;
						break;
					case 1: //white
						*(cursors_mem+(i+x)*2)=0;
						*(cursors_mem+(i+x)*2+1)=1;
						break;
					case 2: //black
						*(cursors_mem+(i+x)*2)=1;
						*(cursors_mem+(i+x)*2+1)=1;
						break;
					case 3: //reverse
						*(cursors_mem+(i+x)*2)=1;
						*(cursors_mem+(i+x)*2+1)=0;
						break;
					}
				}

		}
	free(handle_cursors_mem_bmp);
}

void assign_cursor(int cursor_id)
{
	int hot_x,hot_y,x,y,i,cur_color,cur_byte,cur_bit;
	Uint8 cur_mask=0;
	Uint8 cursor_data[16*16/8];
	Uint8 cursor_mask[16*16/8];
	Uint8 *cur_cursor_mem;
	//clear the data and mask
	for(i=0;i<16*16/8;i++)cursor_data[i]=0;
	for(i=0;i<16*16/8;i++)cursor_mask[i]=0;

	cur_cursor_mem=(Uint8 *)calloc(16*16*2, sizeof(char));

	i=0;
	for(y=0;y<cursors_y_lenght;y++)
		for(x=cursor_id*16;x<cursor_id*16+16;x++)
			{
				cur_color=*(cursors_mem+(y*cursors_x_lenght+x)*2);
				*(cur_cursor_mem+i)=cur_color;//data
				cur_color=*(cursors_mem+(y*cursors_x_lenght+x)*2+1);
				*(cur_cursor_mem+i+256)=cur_color;//mask
				i++;
			}
	//ok, now put the data into the bit data and bit mask
	for(i=0;i<16*16;i++)
		{
			cur_color=*(cur_cursor_mem+i);
			cur_byte=i/8;
			cur_bit=i%8;
			if(cur_color)//if it is 0, let it alone, no point in setting it
				{
					switch(cur_bit) {
					case 0:
						cur_mask=128;break;
					case 1:
						cur_mask=64;break;
					case 2:
						cur_mask=32;break;
					case 3:
						cur_mask=16;break;
					case 4:
						cur_mask=8;break;
					case 5:
						cur_mask=4;break;
					case 6:
						cur_mask=2;break;
					case 7:
						cur_mask=1;break;
					}
					cursor_data[cur_byte]|=cur_mask;
				}

		}
	for(i=0;i<16*16;i++)
		{
			cur_color=*(cur_cursor_mem+i+256);
			cur_byte=i/8;
			cur_bit=i%8;
			if(cur_color)//if it is 0, let it alone, no point in setting it
				{
					if(cur_bit==0)cur_mask=128;
					else if(cur_bit==1)cur_mask=64;
					else if(cur_bit==2)cur_mask=32;
					else if(cur_bit==3)cur_mask=16;
					else if(cur_bit==4)cur_mask=8;
					else if(cur_bit==5)cur_mask=4;
					else if(cur_bit==6)cur_mask=2;
					else if(cur_bit==7)cur_mask=1;
					cursor_mask[cur_byte]|=cur_mask;
				}
		}

	hot_x=cursors_array[cursor_id].hot_x;
	hot_y=cursors_array[cursor_id].hot_y;
	cursors_array[cursor_id].cursor_pointer=(Uint8 *)SDL_CreateCursor(cursor_data,cursor_mask,16,16,hot_x,hot_y);
    free(cur_cursor_mem);
}

void change_cursor(int cursor_id)
{

	SDL_SetCursor((SDL_Cursor*)cursors_array[cursor_id].cursor_pointer);
	current_cursor=cursor_id;
}

void change_cursor_show(int cursor_id)
{
	SDL_SetCursor((SDL_Cursor*)cursors_array[cursor_id].cursor_pointer);
	current_cursor=cursor_id;
	SDL_WarpMouse(mouse_x,mouse_y);
}

void build_cursors()
{
	cursors_array[CURSOR_EYE].hot_x=3;
	cursors_array[CURSOR_EYE].hot_y=0;
	assign_cursor(CURSOR_EYE);

	cursors_array[CURSOR_TALK].hot_x=3;
	cursors_array[CURSOR_TALK].hot_y=0;
	assign_cursor(CURSOR_TALK);

	cursors_array[CURSOR_ATTACK].hot_x=3;
	cursors_array[CURSOR_ATTACK].hot_y=0;
	assign_cursor(CURSOR_ATTACK);

	cursors_array[CURSOR_ENTER].hot_x=3;
	cursors_array[CURSOR_ENTER].hot_y=0;
	assign_cursor(CURSOR_ENTER);

	cursors_array[CURSOR_PICK].hot_x=3;
	cursors_array[CURSOR_PICK].hot_y=15;
	assign_cursor(CURSOR_PICK);

	cursors_array[CURSOR_HARVEST].hot_x=3;
	cursors_array[CURSOR_HARVEST].hot_y=0;
	assign_cursor(CURSOR_HARVEST);

	cursors_array[CURSOR_WALK].hot_x=3;
	cursors_array[CURSOR_WALK].hot_y=0;
	assign_cursor(CURSOR_WALK);

	cursors_array[CURSOR_ARROW].hot_x=3;
	cursors_array[CURSOR_ARROW].hot_y=0;
	assign_cursor(CURSOR_ARROW);

	cursors_array[CURSOR_TRADE].hot_x=3;
	cursors_array[CURSOR_TRADE].hot_y=0;
	assign_cursor(CURSOR_TRADE);

	cursors_array[CURSOR_MAGIC].hot_x=3;
	cursors_array[CURSOR_MAGIC].hot_y=0;
	assign_cursor(CURSOR_MAGIC);

	cursors_array[CURSOR_USE].hot_x=3;
	cursors_array[CURSOR_USE].hot_y=0;
	assign_cursor(CURSOR_USE);
}

void check_cursor_change()
{
	int i;
	if(elwin_mouse >= 0) {
		if(current_cursor!=elwin_mouse)change_cursor(elwin_mouse);
		elwin_mouse=-1;
		return;
	}
	if(object_under_mouse ==-1)
		{
			if(current_cursor!=CURSOR_WALK)change_cursor(CURSOR_WALK);
			return;
		}
	if(thing_under_the_mouse==UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
		{
			//see if it is a bag. (bag1.e3d)
			if(get_string_occurance("bag1.e3d",objects_list[object_under_mouse]->file_name, 80,0)!=-1)
				{
					if(current_cursor!=CURSOR_PICK)change_cursor(CURSOR_PICK);
					return;
				}

			if(action_mode==action_look)
				{
					if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
					return;
				}
			if(action_mode==action_use)
				{
					if(current_cursor!=CURSOR_USE)change_cursor(CURSOR_USE);
					return;
				}
			//see if the object is a harvestable resource.
			for(i=0;i<100;i++)
				{
					if(!harvestable_objects[i].name[0])break;//end of the objects
					if(get_string_occurance(harvestable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
						{
							if(current_cursor!=CURSOR_HARVEST)change_cursor(CURSOR_HARVEST);
							return;
						}
				}

			//see if the object is an entrable resource.
			for(i=0;i<100;i++)
				{
					if(!entrable_objects[i].name[0])break;//end of the objects
					if(get_string_occurance(entrable_objects[i].name,objects_list[object_under_mouse]->file_name, 80,0)!=-1)
						{
							if(current_cursor!=CURSOR_ENTER)change_cursor(CURSOR_ENTER);
							return;
						}
				}

			//hmm, no usefull object, so select walk....
			if(current_cursor!=CURSOR_WALK)change_cursor(CURSOR_WALK);
			return;

		}

	else if(thing_under_the_mouse==UNDER_MOUSE_NPC)
		{
			if(action_mode==action_look)
				{
					if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
					return;
				}
			if(current_cursor!=CURSOR_TALK)change_cursor(CURSOR_TALK);
			return;
		}
	
	else if(thing_under_the_mouse==UNDER_MOUSE_PLAYER)
		{
			if(action_mode==action_use)
				{
					if(current_cursor!=CURSOR_USE)change_cursor(CURSOR_USE);
					return;
				}      
			if(action_mode==action_look)
				{
					if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
					return;
				}


			if(action_mode==action_trade)
				{
					if(current_cursor!=CURSOR_TRADE)change_cursor(CURSOR_TRADE);
					return;
				}

			if(alt_on || action_mode==action_attack)
				{
					if(current_cursor!=CURSOR_ATTACK)change_cursor(CURSOR_ATTACK);
					return;
				}
			if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
			return;

		}

	else if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
		{
			if(action_mode==action_use)
				{
					if(current_cursor!=CURSOR_USE)change_cursor(CURSOR_USE);
					return;
				}      
			if(action_mode==action_look)
				{
					if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
					return;
				}

			if(shift_on)
				{
					if(current_cursor!=CURSOR_EYE)change_cursor(CURSOR_EYE);
					return;
				}

			if(alt_on || action_mode==action_attack || (actor_under_mouse && !actor_under_mouse->dead))
				{
					if(current_cursor!=CURSOR_ATTACK)change_cursor(CURSOR_ATTACK);
					return;
				}
		}

	// when all fails - walk
	if(current_cursor!=CURSOR_WALK)change_cursor(CURSOR_WALK);
}

