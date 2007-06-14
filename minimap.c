#ifdef MINIMAP

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"
#include "interface.h"

GLuint minimap_texture = 0;
GLuint circle_texture = 0;
GLuint exploration_texture = 0;
int minimap_win = -1;
int minimap_win_x = 5;
int minimap_win_y = 20;
int minimap_flags = 1<<2;
int minimap_zoom = 2;		//zoom in, from 0 being only the visable area, 5 being full on largest maps
GLubyte exploration_map[256][256][4];
char current_exploration_map_filename[256];

int max_zoom = 1;
/*
 * TODO:
 *
 *  -draw arrow showing players direction?
 *  -load/save the exploration data from ~
 *  -update description of the draw function below
 *
 * POSSIBLE OPTIMIZATION:
 *  -the window's content only changes when actors are updated or map is changed.
 *   maybe draw on another buffer so the window dont have to be redrawn every frame?
 */


__inline int minimap_get_pin(void){
	return minimap_flags & 1<<0;
}

__inline int minimap_get_above(void){
	return minimap_flags & 1<<1;
}

__inline int minimap_get_FOW(void){
	//Note: Since this should default to on, we use an inverted bit. Since no-where else
	//will ever care what the bit is set to (it will only be bitwise toggled or checked
	//through through this function) this is currently safe
	return minimap_flags & 1<<2;
}

__inline float minimap_get_zoom(void){
	return powf(2,min2i(minimap_zoom,max_zoom)-max_zoom);
}


__inline void full_zoom(void){
	minimap_zoom = 0;
}

__inline void increase_zoom(void){
	if(minimap_zoom > max_zoom)minimap_zoom = max_zoom - 1;
	else if(minimap_zoom > 0)--minimap_zoom;
}

__inline void decrease_zoom(void){
	if(minimap_zoom < max_zoom)++minimap_zoom;
}

__inline void no_zoom(void){
	if(minimap_zoom < max_zoom)minimap_zoom = max_zoom;
}


/* 
 * draw minimap
 *   
 * It first fills the window with grey, that will be fog area, then it draws white
 * circles on player and guildmates positions, that will be visible areas, then it
 * draws black on unexplored areas. then the minimap is drawn with blending function
 * GL_DST_COLOR, GL_ZERO. then draw boxes where the actors are.
 */
int display_minimap_handler(window_info *win)
{
	float zoom_multip;
	float size_x = 256.0f/(tile_map_size_x * 6);
	float size_y = 256.0f/(tile_map_size_y * 6);
	float x, y, view_distance, px = 0.0f, py = 0.0f, ax = 0.0f, ay = 0.0f, sx = 0.0f, sy = 0.0f;
	int i = 0;
	actor *player;
	actor *a;
	zoom_multip = minimap_get_zoom();

	if(!minimap_texture) 
	{
		//there's no minimap for this map :( draw a X
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glVertex3i(30, 30, 1);
		glVertex3i(226, 226, 1);
		glVertex3i(226, 30, 1);
		glVertex3i(30, 226, 1);
		glEnd();
		glLineWidth(1.0f); 
		glEnable(GL_TEXTURE_2D);
		glColor3f(0.8f,0.8f,0.8f);
		draw_string_small(14,125,(unsigned char*)"no minimap for this place",1);
	} else {
		//draw minimap

		//get player position in window coordinates
		player = NULL;
		for(i = 0; i < 1000; i++)
			if(actors_list[i] != NULL)
				if(actors_list[i]->actor_id == yourself)
				{
					player = actors_list[i];
					px = player->x_tile_pos * size_x;
					py = 256.0f - (player->y_tile_pos * size_y);
					break;
				}
		if(player == NULL){
			//Don't know who we are? can't draw then
			return 0;
		}
		//how far can you see? 30 tiles? at least this looks right.
		view_distance = size_x * 30.0f / zoom_multip;

		if(minimap_zoom < max_zoom){
			float su, sv, fu, fv;
			ax = 128.0f-px;
			ay = 128.0f-py;
			sx =  px/256.0f-0.50f*zoom_multip;
			sy = -py/256.0f-0.50f*zoom_multip;

			su = max2i(0.0f, -sx*256.0f/zoom_multip);
			fu = min2i(256.0f, (1.0f-sx)*256.0f/zoom_multip);
			sv = min2i(256.0f, 256.0f+(256.0f+sy*256.0f)/zoom_multip);
			fv = max2i(0.0f, -256.0f+(1.0f+sy)*256.0f/zoom_multip);
			//draw a black background for the window
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_QUADS);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(0.0f, 256.0f);
				glVertex2f(256.0f, 256.0f);
				glVertex2f(256.0f, 0.0f);
			glEnd();
			glEnable(GL_SCISSOR_TEST);
			//clip the drawable region to the map
			glScissor(win->cur_x+su, window_height - win->cur_y - sv, fu - su, sv - fv);
		} else {
			//lets not draw outside the window :)
			//maybe put this in elwindows.c around the call to display handler?
			glEnable(GL_SCISSOR_TEST);
			glScissor(win->cur_x, window_height - win->cur_y - 256, 256, 256);
		}

		if(minimap_get_FOW()){
			//draw exploration map here.
			glEnable(GL_TEXTURE_2D);
			bind_texture_id(exploration_texture);
			glColor4f(0.5f,0.5f,0.5f, 0.5f);
			glBegin(GL_QUADS);
			glTexCoord2f(sx+zoom_multip, sy);
				glVertex2i(0,0);
			glTexCoord2f(sx+zoom_multip, sy+zoom_multip);
				glVertex2i(256,0);
			glTexCoord2f(sx, sy+zoom_multip);
				glVertex2i(256,256);
			glTexCoord2f(sx, sy);
				glVertex2i(0,256);
			glEnd();

			//white circle around player
			bind_texture_id(circle_texture);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_COLOR,GL_ONE);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_QUADS);
			if(minimap_zoom < max_zoom){
				glTexCoord2f(0.0f,0.0f); glVertex2f(128-view_distance,128+view_distance);
				glTexCoord2f(0.0f,1.0f); glVertex2f(128-view_distance,128-view_distance); 
				glTexCoord2f(1.0f,1.0f); glVertex2f(128+view_distance,128-view_distance);
				glTexCoord2f(1.0f,0.0f); glVertex2f(128+view_distance,128+view_distance);
			} else {
				glTexCoord2f(0.0f,0.0f); glVertex2f(px-view_distance,py+view_distance);
				glTexCoord2f(0.0f,1.0f); glVertex2f(px-view_distance,py-view_distance); 
				glTexCoord2f(1.0f,1.0f); glVertex2f(px+view_distance,py-view_distance);
				glTexCoord2f(1.0f,0.0f); glVertex2f(px+view_distance,py+view_distance);
			}
			glEnd();
			glDisable(GL_BLEND);
		}

		//draw the minimap
		bind_texture_id(minimap_texture);
		if(minimap_get_FOW()){
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
		}
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glBegin(GL_QUADS);

		glTexCoord2f(sx, sy);
			glVertex3i(0,256,0);
		glTexCoord2f(sx, sy+zoom_multip);
			glVertex3i(0,0,0);
		glTexCoord2f(sx+zoom_multip, sy+zoom_multip);
			glVertex3i(256,0,0);
		glTexCoord2f(sx+zoom_multip, sy);
			glVertex3i(256,256,0);
		glEnd();

		if(minimap_get_FOW()){
			glDisable(GL_BLEND);
		}
		glDisable(GL_TEXTURE_2D);

		//display the actors
		glPointSize(max2f(3.0f,1.2f*size_x/zoom_multip));
		glBegin(GL_POINTS);
		for(i = 0; i < 1000; i++)
		{
			a = actors_list[i];
			if(a != NULL)
			{
				x = a->x_tile_pos * size_x;
				y = 256.0f - (a->y_tile_pos * size_y);
				if(minimap_zoom < max_zoom){
					//adjustments to the other actor positions for zoom
					x -= px; x /= zoom_multip; x += px; x += ax;
					y -= py; y /= zoom_multip; y += py; y += ay;
				}
				if(a->kind_of_actor == NPC){
					glColor3f(0.0f,0.0f,1.0f); //blue NPCs
				} else if(a->actor_id == yourself){
					glColor3f(0.0f,1.0f,0.0f); //green yourself
				} else if(a->is_enhanced_model && (a->kind_of_actor ==  PKABLE_HUMAN || a->kind_of_actor == PKABLE_COMPUTER_CONTROLLED)) {
					glColor3f(1.0f,0.0f,0.0f); //red PKable
				} else if(a->is_enhanced_model && is_in_buddylist(a->actor_name)){
					glColor3f(0.0f,0.9f,1.0f); //aqua buddy
				} else if(IS_COLOR((unsigned char)a->actor_name[0])){
					if(a->is_enhanced_model && is_in_buddylist(a->actor_name)){
						glColor3f(0.0f,0.9f,1.0f); //aqua buddy
					} else {	//Use the colour of their name. This gives purple bots, green demigods, etc.
						glColor3ub(colors_list[((unsigned char)a->actor_name[0])-127].r1,
							colors_list[((unsigned char)a->actor_name[0])-127].g1,
							colors_list[((unsigned char)a->actor_name[0])-127].b1);
					}
				} else if(!a->is_enhanced_model){
					glColor3f(1.0f, 1.0f, 0.0f); //yellow animal/monster
				} else {
					glColor3f(1.0f, 1.0f, 1.0f); //white other player
				}
				//Draw it!
				glVertex2f(x, y);
			}
		}
		glEnd();//GL_POINTS

		//Messer, Gabel, Schere, Licht... ...sind für kleine developer nicht!
		glDisable(GL_SCISSOR_TEST);

		glDisable(GL_TEXTURE_2D);
		glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);

		// zoom button boxes
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x, ELW_BOX_SIZE*2);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*2);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*6);
			glVertex2i(win->len_x, ELW_BOX_SIZE*6);
		glEnd();
		glBegin(GL_LINES);
			glVertex2i(win->len_x, ELW_BOX_SIZE*3);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*3);
			glVertex2i(win->len_x, ELW_BOX_SIZE*4);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*4);
			glVertex2i(win->len_x, ELW_BOX_SIZE*5);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*5);
		glEnd();

		//zoom right in button
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*2.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*2.5+1);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2.5+1);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*3-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*3-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2.5+1);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5+1);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5-1);
		glEnd();

		//plus/minus buttons
		glLineWidth(2.0f);
		glBegin(GL_LINES);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*3+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*4-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*3.5);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*3.5);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*4.5);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*4.5);
		glEnd();
		glLineWidth(1.0f);

		//zoom right out button
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*5.5-1);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*5.5+1);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5+1);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5-1);
		glEnd();

		//pinned button box
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x, ELW_BOX_SIZE*7);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*7);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*8);
			glVertex2i(win->len_x, ELW_BOX_SIZE*8);
		glEnd();

		//pinned button
		glBegin(GL_TRIANGLES);
		if(minimap_get_pin()){
			glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*8-5);
			glVertex2i(win->len_x-ELW_BOX_SIZE+5, ELW_BOX_SIZE*7+5);
			glVertex2i(win->len_x-5, ELW_BOX_SIZE*7+5);
		} else {
			glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*7+5);
			glVertex2i(win->len_x-ELW_BOX_SIZE+5, ELW_BOX_SIZE*8-5);
			glVertex2i(win->len_x-5, ELW_BOX_SIZE*8-5);
		}
		glEnd();

		//above button box
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x, ELW_BOX_SIZE*9);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*9);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*10);
			glVertex2i(win->len_x, ELW_BOX_SIZE*10);
		glEnd();

		//above button
		glBegin(GL_LINE_LOOP);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*9+8);
			glVertex2i(win->len_x-ELW_BOX_SIZE+7, ELW_BOX_SIZE*9+8);
			glVertex2i(win->len_x-ELW_BOX_SIZE+7, ELW_BOX_SIZE*10-4);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*10-4);
		glEnd();
		if(minimap_get_above()){
			glBegin(GL_QUADS);
		} else {
			glBegin(GL_LINE_LOOP);
		}
			glVertex2i(win->len_x-7, ELW_BOX_SIZE*9+4);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*9+4);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*10-8);
			glVertex2i(win->len_x-7, ELW_BOX_SIZE*10-8);
		glEnd();

		//FOW button box
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x, ELW_BOX_SIZE*11);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*11);
			glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*12);
			glVertex2i(win->len_x, ELW_BOX_SIZE*12);
		glEnd();
		if(minimap_get_FOW()){
			glBegin(GL_LINE_STRIP);
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
			glEnd();
			//draw square
		} else {
			glBegin(GL_QUADS);
				//draw solid square with hole. Done with 4 trapezoids.
				//top:
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5-3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5-3);
				//bottom:
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5+3);
				//right:
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5-3);
				//left:
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5+3);
				glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5-3);
			glEnd();
		}

		glLineWidth(1.0f);
		glEnable(GL_TEXTURE_2D);
	}  
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}


int click_minimap_handler(window_info * win, int mx, int my, Uint32 flags){
	for(;pow(2,4+max_zoom) <= tile_map_size_x;++max_zoom);
	if((flags & ELW_WHEEL) && my < win->len_y && mx > 0 && mx < win->len_x-ELW_BOX_SIZE){
		if(flags & ELW_WHEEL_UP){
			increase_zoom();
		} else {
			decrease_zoom();
		}
		return 1;
	}
	if(!flags & ELW_LEFT_MOUSE){
		return 0;
	} else if(mx < win->len_x-ELW_BOX_SIZE || mx > win->len_x){
		return 0;
	} else if(my >= ELW_BOX_SIZE*2 && my < ELW_BOX_SIZE*3){
		full_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*3 && my < ELW_BOX_SIZE*4){
		increase_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*4 && my < ELW_BOX_SIZE*5){
		decrease_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*5 && my < ELW_BOX_SIZE*6){
		no_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*7 && my < ELW_BOX_SIZE*8){
		minimap_flags ^= 1<<0;	//bitwise toggle
		return 1;
	} else if(my >= ELW_BOX_SIZE*9 && my < ELW_BOX_SIZE*10){
		minimap_flags ^= 1<<1;	//bitwise toggle
		//TODO: ability to set window above others. 
		return 1;
	} else if(my >= ELW_BOX_SIZE*11 && my < ELW_BOX_SIZE*12){
		minimap_flags ^= 1<<2;	//bitwise toggle
		return 1;
	}
	return 0;
}


int mouseover_minimap_handler(window_info * win, int mx, int my, Uint32 flags){
	if(mx < win->len_x-ELW_BOX_SIZE || mx > win->len_x){
	} else if(my >= ELW_BOX_SIZE*2 && my < ELW_BOX_SIZE*3){
		show_help("Zoom completely in", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*3 && my < ELW_BOX_SIZE*4){
		show_help("Zoom in", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*4 && my < ELW_BOX_SIZE*5){
		show_help("Zoom out", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*5 && my < ELW_BOX_SIZE*6){
		show_help("Zoom completely out", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*7 && my < ELW_BOX_SIZE*8){
		if(minimap_get_pin()){
			show_help("Un-pin window", mx+12, my+10);
		} else {
			show_help("Pin window (ignores alt+d)", mx+12, my+10);
		}
	} else if(my >= ELW_BOX_SIZE*9 && my < ELW_BOX_SIZE*10){
		if(minimap_get_above()){
			show_help("Un-above window", mx+12, my+10);
		} else {
			show_help("Display above other windows", mx+12, my+10);
		}
	} else if(my >= ELW_BOX_SIZE*11 && my < ELW_BOX_SIZE*12){
		if(minimap_get_FOW()){
			show_help("Disable Fog Of War", mx+12, my+10);
		} else {
			show_help("Enable Fog Of War", mx+12, my+10);
		}
	}
	return 0;
}


void update_exploration_map()
{
	actor * player;
	float size_x = tile_map_size_x * 6;
	float size_y = tile_map_size_y * 6;
	int px = 0, py = 0, i = 0, j = 0, view_distance = 0, d = 0, vd_square = 0;
	int explored = 0;
	GLubyte c;
	
	if(!minimap_texture || minimap_win < 0)
		return;
	
	//get player position in window coordinates
	player = NULL;
	for(i = 0; i < 1000; i++)
		if(actors_list[i] != NULL)
			if(actors_list[i]->actor_id == yourself)
			{
				player = actors_list[i];
				px = (player->x_tile_pos / size_x) * 256.0f;
				py = ((player->y_tile_pos / size_y) * 256.0f);
				break;
			}
	
	//how far can you see? 30 tiles? at least this looks right.
	view_distance = (256.0f / size_x) * 26.0f;
	vd_square = view_distance*view_distance;

	for(i = -view_distance; i < view_distance; i++)
		for(j = -view_distance; j < view_distance; j++)
			if(0 < (px + i) && (px + i) < 256 && 0 < (py + j) && (py + j) < 256)
			{
				d = i*i + j*j;
				if(d <= vd_square)
					if(exploration_map[px + i][py + j][0] < (GLubyte)0xFF)
					{
						if(d > (vd_square / 4)) //fade out exploration map from half to full radius
						{
							c = (GLubyte)0xFF - (GLubyte)0xFF * (float)((float)(d - (vd_square / 4)) / (float)vd_square);
							if(exploration_map[px + i][py + j][0] < c)
								exploration_map[px + i][py + j][0] = 
									exploration_map[px + i][py + j][1] = 
									exploration_map[px + i][py + j][2] = 
									exploration_map[px + i][py + j][3] = c;
						}
						else
							exploration_map[px + i][py + j][0] = 
								exploration_map[px + i][py + j][1] = 
								exploration_map[px + i][py + j][2] = 
								exploration_map[px + i][py + j][3] = (GLubyte)0xFF;
						explored = 1;
					}
			}
	
	
	if(explored)
	{
		glBindTexture(GL_TEXTURE_2D, exploration_texture);	//failsafe
		bind_texture_id(exploration_texture);
		
		if(have_extension(arb_texture_compression))
		{
			if(have_extension(ext_texture_compression_s3tc))
				glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
			else
				glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		}
		else
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		
		CHECK_GL_ERRORS();
	}
}


void load_exploration_map()
{
	int i, j;
	FILE *fp = NULL;
	char exploration_map_filename[256];
	GLubyte *mapfile;
	
	if(!minimap_texture)
		return;
	
	my_strcp(exploration_map_filename,map_file_name);
	exploration_map_filename[strlen(exploration_map_filename)-4] = 0;
	strcat(exploration_map_filename, ".xm");
	my_strcp(current_exploration_map_filename, exploration_map_filename);
	
	mapfile = (GLubyte *)malloc(sizeof(GLubyte) * 256 * 256);
	
	fp = my_fopen(exploration_map_filename, "rb");
	if(fp)
	{
		fread(mapfile, sizeof(GLubyte), 256 * 256, fp);
		for(i=0; i<256; i++)
			for(j=0; j<256; j++)
			{
				exploration_map[i][j][0] = exploration_map[i][j][1] = exploration_map[i][j][2] = exploration_map[i][j][3] = mapfile[j*256 + i];
			}
		fclose(fp);
	} else {
		memset(exploration_map, 0, 256 * 256 * 4 * sizeof(GLubyte));
	}
	
	free(mapfile);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	if(have_extension(arb_texture_compression))
	{
		if(have_extension(ext_texture_compression_s3tc))
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		else
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB, 256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		
	}
	else
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, 256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
	
	
	CHECK_GL_ERRORS();
	
}


void save_exploration_map()
{
	FILE *fp = NULL;
	int i, j;
	GLubyte *mapfile;
	
	if(!minimap_texture)
		return;
	
	mapfile = (GLubyte *)malloc(sizeof(GLubyte) * 256 * 256);
	
	for(i=0;i<256;i++)
		for(j=0;j<256;j++)
		{
			mapfile[j * 256 + i] = exploration_map[i][j][0];
		}
	
	fp = my_fopen(current_exploration_map_filename, "wb");
	if(fp)
	{
		fwrite(mapfile, sizeof(GLubyte), 256 * 256, fp);
		fclose(fp);
	} else {
		//log error and quit
	}	
	
	free(mapfile);
}

void change_minimap(){
	char minimap_file_name[256];

	if(minimap_win < 0)
		return;
	save_exploration_map();

	//unload all textures
	if(minimap_texture)
		glDeleteTextures(1,&minimap_texture);
	if(circle_texture)
		glDeleteTextures(1,&minimap_texture);
	if(exploration_texture)
		glDeleteTextures(1,&exploration_texture);

	//make filename
	my_strcp(minimap_file_name,map_file_name);
	minimap_file_name[strlen(minimap_file_name)-4] = '\0';
	strcat(minimap_file_name, ".bmp");

	//load textures
	minimap_texture = load_bmp8_fixed_alpha(minimap_file_name,128);
	circle_texture = load_bmp8_fixed_alpha("./textures/circle.bmp",0);
	glGenTextures(1, &exploration_texture);
	glBindTexture(GL_TEXTURE_2D, exploration_texture);	//failsafe
	bind_texture_id(exploration_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	load_exploration_map();

	for(max_zoom=0;pow(2,4+max_zoom) <= tile_map_size_x;++max_zoom);

	if(minimap_zoom > max_zoom)minimap_zoom = max_zoom ;
	else if(minimap_zoom < 0)minimap_zoom = 0;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void display_minimap()
{
	if(minimap_win < 0)
	{
		//init minimap
		minimap_win = create_window("Minimap", windows_on_top?-1:game_root_win, 0, minimap_win_x, minimap_win_y, 256+ELW_BOX_SIZE, 256+1, ELW_WIN_DEFAULT);
		set_window_handler(minimap_win, ELW_HANDLER_DISPLAY, &display_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_CLICK, &click_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_MOUSEOVER, &mouseover_minimap_handler);	

		change_minimap();
	} else {
		show_window(minimap_win);
		select_window(minimap_win);
	}
}

#endif //MINIMAP

//EOF
