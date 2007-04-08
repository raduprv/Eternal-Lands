#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"
#include "interface.h"

GLuint minimap_text = 0;
GLuint circle_text = 0;
GLuint exploration_text = 0;
int minimap_win = -1;
GLubyte exploration_map[256][256][4];
char current_exploration_map_filename[256];

/*
 * TODO:
 *
 *  -scrolling view when dragging mouse on minimap. how far?
 *  -draw arrow showing players direction?
 *  -different color for buddies
 *  -call save_exploration_map on client exit
 *
 * POSSIBLE OPTIMIZATION:
 *  -the window's content only changes when actors are updated or map is changed.
 *   maybe draw on another buffer so the window dont have to be redrawn every frame?
 */



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
	float size_x = tile_map_size_x * 6;
	float size_y = tile_map_size_y * 6;
	float x, y, view_distance, px = 0.0f, py = 0.0f, tx = 0.0f, ty = 0.0f;
	int i = 0;
	actor *player;
	actor *a;
	
	if(!minimap_text) 
	{
		//theres no minimap for this map :( draw a X
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
		
		//lets not draw outside the window :)
		//maybe put this in elwindows.c around the call to display handler?
		glEnable(GL_SCISSOR_TEST);
		glScissor(win->cur_x, window_height - win->cur_y - 256, 256, 256);
		
		//get player position in window coordinates
		player = NULL;
		for(i = 0; i < 1000; i++)
			if(actors_list[i] != NULL)
				if(actors_list[i]->actor_id == yourself)
				{
					player = actors_list[i];
					px = (player->x_tile_pos / size_x) * 256.0f;
					py = 256.0f - ((player->y_tile_pos / size_y) * 256.0f);
					break;
				}
		
		//how far can you see? 30 tiles? at least this looks right.
		view_distance = (256.0f / size_x) * 30.0f;
		
		//grey out window
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glColor3f(0.5f,0.5f,0.5f);
		glBegin(GL_QUADS);
		glVertex2i(0,256);
		glVertex2i(0,0); 
		glVertex2i(256,0);
		glVertex2i(256,256);
		glEnd();
		
		//draw exploration map here.
		glEnable(GL_TEXTURE_2D);
		bind_texture_id(exploration_text);
		glColor4f(0.5f,0.5f,0.5f, 0.5f);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex2i(0,0);
		glTexCoord2f(1.0f,1.0f); glVertex2i(256,0); 
		glTexCoord2f(0.0f,1.0f); glVertex2i(256,256);
		glTexCoord2f(0.0f,0.0f); glVertex2i(0,256);
		glEnd();
		
		//white circle around player
		bind_texture_id(circle_text);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR,GL_ONE);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex2f(px-view_distance,py+view_distance);
		glTexCoord2f(0.0f,1.0f); glVertex2f(px-view_distance,py-view_distance); 
		glTexCoord2f(1.0f,1.0f); glVertex2f(px+view_distance,py-view_distance);
		glTexCoord2f(1.0f,0.0f); glVertex2f(px+view_distance,py+view_distance);
		glEnd();
		
		//find guildmates and draw their white circles
		if(player)
			if(player->body_parts->guild_id != 0)
				for(i = 0; i < 1000; i++)
					if(actors_list[i] != NULL)
						if(actors_list[i]->is_enhanced_model && actors_list[i] != player)
							if(actors_list[i]->body_parts->guild_id == player->body_parts->guild_id)
							{
								tx = (actors_list[i]->x_tile_pos / size_x) * 256.0f;
								ty = 256.0f - ((actors_list[i]->y_tile_pos / size_y) * 256.0f);
								glBegin(GL_QUADS);
								glTexCoord2f(0.0f,0.0f); glVertex2f(tx-view_distance,ty+view_distance);
								glTexCoord2f(0.0f,1.0f); glVertex2f(tx-view_distance,ty-view_distance); 
								glTexCoord2f(1.0f,1.0f); glVertex2f(tx+view_distance,ty-view_distance);
								glTexCoord2f(1.0f,0.0f); glVertex2f(tx+view_distance,ty+view_distance);
								glEnd();
							}
		
		//draw the minimap
		bind_texture_id(minimap_text);
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex3i(1,256,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(1,1,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(256,1,0);
		glTexCoord2f(1.0f,0.0f); glVertex3i(256,256,0);
		glEnd();
		
		//display the actors
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		for(i = 0; i < 1000; i++)
		{
			a = actors_list[i];
			if(a != NULL)
			{
				x = (a->x_tile_pos / size_x) * 256.0f;
				y = 256.0f - ((a->y_tile_pos / size_y) * 256.0f);
				
				if(a->kind_of_actor == NPC)
					glColor3f(0.0f,0.0f,1.0f); //blue NPCs
				else {
					if(a->is_enhanced_model)
						if(a->body_parts->guild_tag_color == 0)
							glColor3f(1.0f,1.0f,1.0f); //white - players
						else
							glColor3ub(colors_list[a->body_parts->guild_tag_color].r1,
								  colors_list[a->body_parts->guild_tag_color].g1,
								  colors_list[a->body_parts->guild_tag_color].b1);
					else
						glColor3f(1.0f,1.0f,0.0f); //yellow animals/monsters
					if(a->kind_of_actor ==  PKABLE_HUMAN || a->kind_of_actor == PKABLE_COMPUTER_CONTROLLED)
						glColor3f(1.0f,0.0f,0.0f); //red PKable
				}
				if(a->actor_id == yourself){
					glColor3f(0.0f,1.0f,0.0f); //green yourself
				}
				
				glBegin(GL_QUADS);
				glVertex2f(x-1.5f, y-1.5f);
				glVertex2f(x+1.5f, y-1.5f);
				glVertex2f(x+1.5f, y+1.5f);
				glVertex2f(x-1.5f, y+1.5f);
				glEnd();
			}
		}
		//draw the corner, with the X. maybe in elwindows.c first call display handler and then call draw_window_border?
		glDisable(GL_TEXTURE_2D);
		/*
		glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3i(win->len_x, ELW_BOX_SIZE, 0);
		glVertex3i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE, 0);
		glVertex3i(win->len_x-ELW_BOX_SIZE, 0, 0);
		glEnd();
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, 3);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE-3);
		glVertex2i(win->len_x-3, 3);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE-3);
		glEnd();
		glLineWidth(1.0f);
		*/
		
		//Messer, Gabel, Schere, Licht... ...sind für kleine developer nicht!
		glDisable(GL_SCISSOR_TEST);
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
	
	if(!minimap_text || minimap_win < 0)
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
		glBindTexture(GL_TEXTURE_2D, exploration_text);	//failsafe
		bind_texture_id(exploration_text);
		
		if(have_arb_compression)
		{
			if(have_s3_compression)
				glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
			else
				glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
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
	
	if(!minimap_text)
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
	
	if(have_arb_compression)
	{
		if(have_s3_compression)
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT, 256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		else
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB, 256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,&exploration_map);
		
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
	
	if(!minimap_text)
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

void change_minimap()
{
	char map_minimap_file_name[256];
	
	if(minimap_win < 0)
		return;
	
	save_exploration_map();
	
	my_strcp(map_minimap_file_name,map_file_name);
	map_minimap_file_name[strlen(map_minimap_file_name)-4] = 0;
	strcat(map_minimap_file_name, "_m.bmp");
	if(minimap_text)
		glDeleteTextures(1,&minimap_text);
	minimap_text=load_bmp8_fixed_alpha(map_minimap_file_name,128);
	
	if(exploration_text)
		glDeleteTextures(1,&exploration_text);
	glGenTextures(1, &exploration_text);
	glBindTexture(GL_TEXTURE_2D, exploration_text);	//failsafe
	bind_texture_id(exploration_text);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	load_exploration_map();
}


void display_minimap()
{
	char map_minimap_file_name[256];
	
	if(minimap_win < 0)
	{
		//init minimap
		my_strcp(map_minimap_file_name,map_file_name);
		map_minimap_file_name[strlen(map_minimap_file_name)-4] = 0;
		strcat(map_minimap_file_name, "_m.bmp");
		minimap_text=load_bmp8_fixed_alpha(map_minimap_file_name,128);
		circle_text = load_bmp8_fixed_alpha("./textures/circle.bmp",0);
		minimap_win = create_window("Minimap", game_root_win, 0, 0, 0, 256+ELW_BOX_SIZE+2, 256+2, ELW_WIN_DEFAULT);
		set_window_handler(minimap_win, ELW_HANDLER_DISPLAY, &display_minimap_handler);	
		
		//init exploration map
		glGenTextures(1, &exploration_text);
		glBindTexture(GL_TEXTURE_2D, exploration_text);	//failsafe
		bind_texture_id(exploration_text);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		load_exploration_map();
	} else {
		show_window(minimap_win);
		select_window(minimap_win);
	}
}

//EOF
