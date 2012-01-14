#include "../asc.h"
#include "global.h"
#include <string.h>

int view_browser=0;
int browser_menu_x=150;
int browser_menu_y=70;
int browser_menu_x_len=420;
int browser_menu_y_len=400;
int browser_menu_dragged=0;
int browser_win=0;
int close_browser_on_select=0;

object3d o3d[4];
_Dir Dir[240];
_Cat Cat[44];

int dc=-1,cd=-1,cp=0,cc=-1,mc=1,ccat=0;

int setobject(int n, char *fn,float xrot, float yrot, float zrot)
{
	object3d *our_object=&o3d[n];
	snprintf(our_object->file_name,80,"%s",fn);
	
	our_object->e3d_data=load_e3d_cache(fn);
	if(our_object->e3d_data==NULL)return 0;
	our_object->x_pos=0;
	our_object->y_pos=0;
	our_object->z_pos=0;
	
	our_object->x_rot=xrot;
	our_object->y_rot=yrot;
	our_object->z_rot=zrot;

	our_object->color[0] = 0.0f;
	our_object->color[1] = 0.0f;
	our_object->color[2] = 0.0f;
	our_object->color[3] = 0.0f;
	our_object->self_lit=0;
	our_object->blended=0;
	return 1;

}

int check_browser_interface (window_info *win, int _x, int _y)
{
   	if (view_browser && _x > win->len_x-20 && _y <= 20)
	{
		view_browser = 0;
		return 1;
	}

	if(cd==-1){
		int id = _y / 18;
		if (_x > 210) id += 22;

		if(mc==1){
			if(id<=cc){
				ccat=id;
				mc=0;
				if(Cat[ccat].ns==1){
					int i=0;
					for(;i<dc;i++){
						if(Cat[ccat].Sub[0]==&Dir[i])
							cd=i;
					}
					mc=1;
				}
				
			}
		}else{
			if(id<Cat[ccat].ns){
				int i=0;
				for(;i<dc;i++){
					if(Cat[ccat].Sub[id]==&Dir[i])
						cd=i;
				}
			} else if(_x > win->len_x-16 && _y > 160 && _y < 240){
				mc=1;
			}
		}

	}else{
		if(_x>0 && _x<200 && _y>0 && _y<200){
			char fn[256];
			strcpy(fn,".");
			strcat(fn,Dir[cd].Files[cp]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
			if(close_browser_on_select) toggle_window(browser_win);
		}
		if(cp+2<Dir[cd].nf && _x>0 && _x<200 && _y>200 && _y<400){
			char fn[256];
			strcpy(fn,".");
			strcat(fn,Dir[cd].Files[cp+2]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
			if(close_browser_on_select) toggle_window(browser_win);
		}
		if(cp+1<Dir[cd].nf && _x>200 && _x<400 && _y>0 && _y<200){
			char fn[256];
			strcpy(fn,".");
			strcat(fn,Dir[cd].Files[cp+1]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
			if(close_browser_on_select) toggle_window(browser_win);
		}
		if(cp+3<Dir[cd].nf && _x>200 && _x<400 && _y>200 && _y<400){
			char fn[256];
			strcpy(fn,".");
			strcat(fn,Dir[cd].Files[cp+3]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
			if(close_browser_on_select) toggle_window(browser_win);
		}
	

		if(_x > win->len_x-16 && _x < win->len_x && _y > 18 && _y < 18+16){
			if(cp>=4)cp-=4;	
		}

		if(_x > win->len_x-16 && _x < win->len_x && _y > 380 && _y < 380+16){
			if(cp<(Dir[cd].nf-4))cp+=4;
		}

		if(_x > win->len_x-16 && _x < win->len_x && _y > 160 && _y < 240){
			cp=0;
			cd=-1;
		}
   }
   return 1;
}

void display_browser()
{
	if(browser_win <= 0){
		browser_win= create_window("browser", 0, 0, browser_menu_x, browser_menu_y, browser_menu_x_len, browser_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(browser_win, ELW_HANDLER_DISPLAY, &display_browser_handler );
		set_window_handler(browser_win, ELW_HANDLER_CLICK, &check_browser_interface );
		
	} else {
		show_window(browser_win);
		select_window(browser_win);
	}
	display_window(browser_win);
}

int display_browser_handler(window_info *win)
{
   //title bar
   //draw_menu_title_bar(browser_menu_x,browser_menu_y-16,browser_menu_x_len);
   // window drawing
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE,GL_SRC_ALPHA);
   glDisable(GL_TEXTURE_2D);
  
   glDisable(GL_BLEND);
   glColor3f(0.77f,0.57f,0.39f);
   glBegin(GL_LINES);

		//scroll bar
   if(!mc){
	glVertex3i(win->len_x-20,20,0);
	glVertex3i(win->len_x-20,400,0);
   }
   if(cd!=-1){
	glVertex3i(win->len_x-15,30,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-5,30,0);
	glVertex3i(win->len_x-15,385,0);
	glVertex3i(win->len_x-10,390,0);
	glVertex3i(win->len_x-10,390,0);
	glVertex3i(win->len_x-5,385,0);
	//separators
	glVertex3i(0+200,0,0);
	glVertex3i(0+200,0+400,0);
	glVertex3i(0,0+200,0);
	glVertex3i(0+400,0+200,0);
   }

   glEnd();
   glEnable(GL_TEXTURE_2D);
   // The X
	
   if(!mc){
	draw_string(win->len_x-16,160,(unsigned char *)"B",1);
	draw_string(win->len_x-16,180,(unsigned char *)"A",1);
	draw_string(win->len_x-16,200,(unsigned char *)"C",1);
	draw_string(win->len_x-16,220,(unsigned char *)"K",1);
   }

   draw_string(0+win->len_x-16,0+2,(unsigned char *)"X",1);

   if(cd==-1){ //display dir select menu
	   int i,x=0,y=2;
	   if(mc==1){
			for(i=0;i<=cc;i++){
				draw_string(x+2,0+y,(unsigned char *)Cat[i].Name,1);
				y+=18;
				if(y>=398){
					x=210;
					y=2;
				}
			}	
	   }else{
			for(i=0;i<Cat[ccat].ns;i++){
				draw_string(x+2,0+y,(unsigned char *)Cat[ccat].Sub[i]->DirName,1);
				y+=18;
				if(y>=398){
					x=210;
					y=2;
				}
			}
	   }
   }else{ // display specified dir
		int i=cp,valid_object=0;
		float tz=zoom_level;
		char fn[256];
		
		// Prepare to render
		Leave2DMode();
		glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(have_multitexture && clouds_shadows){
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
			bind_texture_unbuffered(ground_detail_text);
#else	/* NEW_TEXTURES */
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
#endif	/* NEW_TEXTURES */
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
		}

		// Now we draw the 4 objects
		zoom_level=3.0;
		window_resize();
		glRotatef(-60, 1.0f, 0.0f, 0.0f);
		glRotatef(0, 0.0f, 0.0f, 1.0f);
		glRotatef(45, 0.0f, 0.0f, 1.0f);

		glViewport(win->pos_x,window_height-win->pos_y-150,200,150);
		glClearStencil(0);
		glClear (GL_DEPTH_BUFFER_BIT);
#ifdef LINUX
		strcpy(fn,exec_path);
#else
		//Fedora: don't ask me why, if you use exec_path, e3d files are not found
		strcpy(fn,"."/*exec_path*/);
#endif
		strcat(fn,Dir[cd].Files[i]);
		valid_object=setobject(0,fn,Dir[cd].xrot[i],Dir[cd].yrot[i],Dir[cd].zrot[i]);
		if(valid_object){
			glPushMatrix();
			glScalef(Dir[cd].size[i],Dir[cd].size[i],Dir[cd].size[i]);
			draw_3d_object(&o3d[0]);
			glPopMatrix();
		}
		

		if(i+1<Dir[cd].nf){
			glViewport(win->pos_x+200,window_height-win->pos_y-150,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
#ifdef LINUX
			strcpy(fn,exec_path);
#else
			//Fedora: don't ask me why, if you use exec_path, e3d files are not found
			strcpy(fn,"."/*exec_path*/);
#endif
			strcat(fn,Dir[cd].Files[i+1]);
			valid_object=setobject(1,fn,Dir[cd].xrot[i+1],Dir[cd].yrot[i+1],Dir[cd].zrot[i+1]);
			if(valid_object){
				glPushMatrix();
				glScalef(Dir[cd].size[i+1],Dir[cd].size[i+1],Dir[cd].size[i+1]);
				draw_3d_object(&o3d[1]);
				glPopMatrix();
			}
		}

		if(i+2<Dir[cd].nf){
			glViewport(win->pos_x,window_height-win->pos_y-350,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
#ifdef LINUX
			strcpy(fn,exec_path);
#else
			//Fedora: don't ask me why, if you use exec_path, e3d files are not found
			strcpy(fn,"."/*exec_path*/);
#endif
			strcat(fn,Dir[cd].Files[i+2]);
			valid_object=setobject(2,fn,Dir[cd].xrot[i+2],Dir[cd].yrot[i+2],Dir[cd].zrot[i+2]);
			if(valid_object){
				glPushMatrix();
				glScalef(Dir[cd].size[i+2],Dir[cd].size[i+2],Dir[cd].size[i+2]);
				draw_3d_object(&o3d[2]);
				glPopMatrix();
			}
		}

		if(i+3<Dir[cd].nf){
			glViewport(win->pos_x+200,window_height-win->pos_y-350,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
#ifdef LINUX
			strcpy(fn,exec_path);
#else
			//Fedora: don't ask me why, if you use exec_path, e3d files are not found
			strcpy(fn,"."/*exec_path*/);
#endif
			strcat(fn,Dir[cd].Files[i+3]);
			valid_object=setobject(3,fn,Dir[cd].xrot[i+3],Dir[cd].yrot[i+3],Dir[cd].zrot[i+3]);
			if(valid_object){
				glPushMatrix();
				glScalef(Dir[cd].size[i+3],Dir[cd].size[i+3],Dir[cd].size[i+3]);
				draw_3d_object(&o3d[3]);
				glPopMatrix();
			}
		}

		zoom_level=tz;
		window_resize();
		// Back to normal
		glViewport(0,0,window_width,window_height);
		glDisable(GL_CULL_FACE);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		if(have_multitexture && clouds_shadows)
			{
				//disable the second texture unit
				glActiveTextureARB(GL_TEXTURE1_ARB);
				glDisable(GL_TEXTURE_2D);
				glActiveTextureARB(GL_TEXTURE0_ARB);
			}
		Enter2DMode();
		CHECK_GL_ERRORS();

		// Object names
		draw_string(win->pos_x+2,win->pos_y+200-18,(unsigned char *)Dir[cd].Names[i],1);
		draw_string(win->pos_x+2,win->pos_y+400-18,(unsigned char *)Dir[cd].Names[i+2],1);
		draw_string(win->pos_x+202,win->pos_y+200-18,(unsigned char *)Dir[cd].Names[i+1],1);
		draw_string(win->pos_x+202,win->pos_y+400-18,(unsigned char *)Dir[cd].Names[i+3],1);

   }
	return 1;
}

void init_browser()
{
	char temp[512];
	char *pch;
	int idx, line;
	FILE *fp=fopen("browser.lst","r");
	if(!fp){
		log_error(__FILE__, __LINE__, "browser.lst not found");
		return;
	}
	line = 1;
	while(!feof(fp)){
		fgets(temp,511,fp);
		if(!strncmp(temp,"Category",8)){
			cc++;
			strcpy(Cat[cc].Name,&temp[9]);		
		}else
		if(!strncmp(temp,"SubCategory",11)){
			dc++;
			strcpy(Dir[dc].DirName,&temp[12]);
			Cat[cc].Sub[Cat[cc].ns]=&Dir[dc];
			Cat[cc].ns++;
		}else if(temp[0]!='\n'){
			int i=0,j;
			while(temp[i]!=','){
				Dir[dc].Names[Dir[dc].nf][i]=temp[i];
				i++;
			}
			j=i+1;
			i=0;
			while(temp[j]!=','){
				Dir[dc].Files[Dir[dc].nf][i++]=temp[j++];
			}

			idx = Dir[dc].nf;

			pch = strtok(&temp[j+1], ",");
			if (pch != 0)
			{
				Dir[dc].xrot[idx] = atof(pch);
				pch = strtok(NULL, ",");
				if (pch != 0)
				{
					Dir[dc].yrot[idx] = atof(pch);
					pch = strtok(NULL, ",");
					if (pch != 0)
					{
						Dir[dc].zrot[idx] = atof(pch);
						pch = strtok(NULL, ",");
						if (pch != 0)
						{
							Dir[dc].size[idx] = atof(pch);
						}
						else
						{
							log_error(__FILE__, __LINE__, "line %d in browser.lst is too short!", line);
						}
					}
					else
					{
						log_error(__FILE__, __LINE__, "line %d in browser.lst is too short!", line);
					}
				}
				else
				{
					log_error(__FILE__, __LINE__, "line %d in browser.lst is too short!", line);
				}
			}
			else
			{
				log_error(__FILE__, __LINE__, "line %d in browser.lst is too short!", line);
			}
			Dir[dc].nf++;
		}
		line++;
	}
	dc++;
}
