#include "global.h"
#include <string.h>

#ifdef WINDOWS
#include <io.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

int view_browser=0;
int browser_menu_x=150;
int browser_menu_y=70;
int browser_menu_x_len=420;
int browser_menu_y_len=400;
int browser_menu_dragged=0;
int browser_win=0;

object3d o3d[4];
_Dir Dir[24];
int dc=-1,cd=-1,cp=0;

void setobject(int n, char *fn,float xrot, float yrot, float zrot)
{
	object3d *our_object=&o3d[n];
	snprintf(our_object->file_name,80,"%s",fn);
	
	our_object->e3d_data=load_e3d_cache(fn);

	our_object->x_pos=0;
	our_object->y_pos=0;
	our_object->z_pos=-(our_object->e3d_data->max_z-our_object->e3d_data->min_z)/2;
	
	our_object->x_rot=xrot;
	our_object->y_rot=yrot;
	our_object->z_rot=zrot;

	our_object->r=0;
	our_object->g=0;
	our_object->b=0;
	our_object->clouds_uv=NULL;
	our_object->self_lit=0;
	our_object->blended=0;
	

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

int display_browser_handler()
{
   int i=0,x=browser_menu_x+2,y=browser_menu_y+2;

   //title bar
   //draw_menu_title_bar(browser_menu_x,browser_menu_y-16,browser_menu_x_len);
   // window drawing
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE,GL_SRC_ALPHA);
   glDisable(GL_TEXTURE_2D);
  
   glDisable(GL_BLEND);
   glColor3f(0.77f,0.57f,0.39f);
   glBegin(GL_LINES);

   if(cd!=-1){
		//scroll bar
		glVertex3i(browser_menu_x_len-20,20,0);
		glVertex3i(browser_menu_x_len-20,400,0);
		glVertex3i(browser_menu_x_len-15,30,0);
		glVertex3i(browser_menu_x_len-10,25,0);
		glVertex3i(browser_menu_x_len-10,25,0);
		glVertex3i(browser_menu_x_len-5,30,0);
		glVertex3i(browser_menu_x_len-15,385,0);
		glVertex3i(browser_menu_x_len-10,390,0);
		glVertex3i(browser_menu_x_len-10,390,0);
		glVertex3i(browser_menu_x_len-5,385,0);
		//separators
		glVertex3i(0+200,0,0);
		glVertex3i(0+200,0+400,0);
		glVertex3i(0,0+200,0);
		glVertex3i(0+400,0+200,0);
	}

   glEnd();
   glEnable(GL_TEXTURE_2D);
   // The X
	
	if(cd!=-1){
		draw_string(browser_menu_x_len-16,160,(unsigned char *)"B",1);
		draw_string(browser_menu_x_len-16,180,(unsigned char *)"A",1);
		draw_string(browser_menu_x_len-16,200,(unsigned char *)"C",1);
		draw_string(browser_menu_x_len-16,220,(unsigned char *)"K",1);
   }

   draw_string(0+browser_menu_x_len-16,0+2,(unsigned char *)"X",1);

   if(cd==-1){ //display dir select menu
	   int i,y=2;
	   for(i=0;i<dc;i++){
		   draw_string(0+2,0+y,(unsigned char *)Dir[i].DirName,1);
		   y+=18;
	   }

   }else{ // display specified dir
		int i=cp;
		float tz=zoom_level;
		char fn[256];
		
		//zoom_level=3.0;
		//resize_window();
		// Prepare to render
		glEnable(GL_CULL_FACE);
		glEnableClientState(GL_VERTEX_ARRAY);
		if(have_multitexture && clouds_shadows){
			//bind the detail texture
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  texture_cache[ground_detail_text].texture_id);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
		}
		Leave2DMode();

		// Now we draw the 4 objects
		zoom_level=3.0;
		resize_window();
		glRotatef(-60, 1.0f, 0.0f, 0.0f);
		glRotatef(0, 0.0f, 0.0f, 1.0f);
		glRotatef(45, 0.0f, 0.0f, 1.0f);

		glViewport(browser_menu_x,window_height-browser_menu_y-150,200,150);
		glClearStencil(0);
		glClear (GL_DEPTH_BUFFER_BIT);
		strcpy(fn,exec_path);
		strcat(fn,Dir[cd].Files[i]);
		setobject(0,fn,Dir[cd].xrot[i],Dir[cd].yrot[i],Dir[cd].zrot[i]);
		glPushMatrix();
		glScalef(Dir[cd].size[i],Dir[cd].size[i],Dir[cd].size[i]);
		draw_3d_object(&o3d[0]);
		glPopMatrix();
		

		if(i+1<Dir[cd].nf){
			glViewport(browser_menu_x+200,window_height-browser_menu_y-150,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[i+1]);
			setobject(1,fn,Dir[cd].xrot[i+1],Dir[cd].yrot[i+1],Dir[cd].zrot[i+1]);
			glPushMatrix();
			glScalef(Dir[cd].size[i+1],Dir[cd].size[i+1],Dir[cd].size[i+1]);
			draw_3d_object(&o3d[1]);
			glPopMatrix();
		}

		if(i+2<Dir[cd].nf){
			glViewport(browser_menu_x,window_height-browser_menu_y-350,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[i+2]);
			setobject(2,fn,Dir[cd].xrot[i+2],Dir[cd].yrot[i+2],Dir[cd].zrot[i+2]);
			glPushMatrix();
			glScalef(Dir[cd].size[i+2],Dir[cd].size[i+2],Dir[cd].size[i+2]);
			draw_3d_object(&o3d[2]);
			glPopMatrix();
		}

		if(i+3<Dir[cd].nf){
			glViewport(browser_menu_x+200,window_height-browser_menu_y-350,200,150);	
			glClearStencil(0);
			glClear (GL_DEPTH_BUFFER_BIT);
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[i+3]);
			setobject(3,fn,Dir[cd].xrot[i+3],Dir[cd].yrot[i+3],Dir[cd].zrot[i+3]);
			glPushMatrix();
			glScalef(Dir[cd].size[i+3],Dir[cd].size[i+3],Dir[cd].size[i+3]);
			draw_3d_object(&o3d[3]);
			glPopMatrix();
		}

			zoom_level=tz;
		resize_window();
		// Back to normal
		glViewport(0,0,window_width,window_height);
		Enter2DMode();

		// Object names
		draw_string(browser_menu_x+2,browser_menu_y+200-18,(unsigned char *)Dir[cd].Names[i],1);
		draw_string(browser_menu_x+2,browser_menu_y+400-18,(unsigned char *)Dir[cd].Names[i+1],1);
		draw_string(browser_menu_x+202,browser_menu_y+200-18,(unsigned char *)Dir[cd].Names[i+2],1);
		draw_string(browser_menu_x+202,browser_menu_y+400-18,(unsigned char *)Dir[cd].Names[i+3],1);
		zoom_level=tz;
		resize_window();
   }
	return 1;
}


int check_browser_interface()
{
   int x,y;
   if(!view_browser || mouse_x>browser_menu_x+browser_menu_x_len || mouse_x<browser_menu_x
      || mouse_y<browser_menu_y || mouse_y>browser_menu_y+browser_menu_y_len)return 0;

   	if(view_browser && mouse_x>(browser_menu_x+browser_menu_x_len-20) && mouse_x<=(browser_menu_x+browser_menu_x_len)
	&& mouse_y>browser_menu_y && mouse_y<=browser_menu_y+20)
	{
		view_browser=0;
		return 1;
	}

   x=mouse_x-browser_menu_x;
   y=mouse_y-browser_menu_y;
	
	if(cd==-1){
		int id=y/18;
		if(id>=dc)id=-1;
		cd=id;
	}else{
		if(x>0 && x<200 && y>0 && y<200){
			char fn[256];
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[cp]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
		}
		if(cp+2<Dir[cd].nf && x>0 && x<200 && y>200 && y<400){
			char fn[256];
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[cp+2]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
		}
		if(cp+1<Dir[cd].nf && x>200 && x<400 && y>0 && y<200){
			char fn[256];
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[cp+1]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
		}
		if(cp+3<Dir[cd].nf && x>200 && x<400 && y>200 && y<400){
			char fn[256];
			strcpy(fn,exec_path);
			strcat(fn,Dir[cd].Files[cp+3]);
			selected_3d_object=add_e3d(fn,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
			cur_tool=tool_select;
		}
	

		if(x > browser_menu_x_len-16 && x < browser_menu_x_len && y > 18 && y < 18+16){
			if(cp>=4)cp-=4;	
		}

		if(x > browser_menu_x_len-16 && x < browser_menu_x_len && y > 380 && y < 380+16){
			if(cp<(Dir[cd].nf-4))cp+=4;
		}

		if(x > browser_menu_x_len-16 && x < browser_menu_x_len && y > 160 && y < 240){
			cp=0;
			cd=-1;
		}
   }
   
   return 1;
}


void add_dir(char *n)
{
#ifdef	WINDOWS
	char execp[256];
	struct _finddata_t c_file;
    long hFile;
	int i=0;

	strcpy(execp,exec_path);
	strcat(execp,"/3dobjects/");
	strcat(execp,n);
	strcat(execp,"/*.e3d");

	strcpy(Dir[dc].DirName,n);
	Dir[dc].nf=1;

	hFile = _findfirst(execp, &c_file );
	strcpy(Dir[dc].Files[i++],c_file.name);

    while(_findnext(hFile, &c_file)==0)
    {
		strcpy(Dir[dc].Files[i++],c_file.name);
		Dir[dc].nf++;
	}
	dc++;
#endif
}

void init_browser()
{
	char temp[512];
	FILE *fp=fopen("browser.lst","r");
	if(!fp){
		log_error("browser.lst not found");
		return;
	}
	
	while(!feof(fp)){
		fgets(temp,511,fp);
		if(!strncmp(temp,"Category",8)){
			dc++;
			strcpy(Dir[dc].DirName,&temp[9]);
		}else{
			int i=0,j;
			while(temp[i]!=','){
				Dir[dc].Names[Dir[dc].nf][i]=temp[i++];
			}
			j=i+1;
			i=0;
			while(temp[j]!=','){
				Dir[dc].Files[Dir[dc].nf][i++]=temp[j++];
			}

			sscanf(&temp[j+1],"%g,%g,%g,%g\n",&Dir[dc].xrot[Dir[dc].nf],&Dir[dc].yrot[Dir[dc].nf],&Dir[dc].zrot[Dir[dc].nf],&Dir[dc].size[Dir[dc].nf++]);
		}
	}
	dc++;
/*
#ifdef WINDOWS
	char execp[256];
	struct _finddata_t c_file;
    long hFile;

	strcpy(execp,exec_path);
	strcat(execp,"/3dobjects/*");
	hFile = _findfirst(execp, &c_file );

    while(_findnext(hFile, &c_file)==0)
    {
		if(c_file.attrib & _A_SUBDIR){
			if(c_file.name[0]!='.')
				add_dir(c_file.name);
		}
	}

	_findclose( hFile );
#else
	char execp[256];
	struct dirent * ffile;
	DIR * dir;
	
	strcpy(execp,exec_path);
	strcat(execp,"/3dobjects/");

	if (!(dir = opendir(execp)))
		return;

	while ((ffile = readdir(dir))){
		struct dirent * ffile2;
		DIR * dir2;
		char temp[256];
		int i=0;

		strcpy(temp,exec_path);
		strcat(temp,"/3dobjects/");
		strcat(temp,ffile->d_name);

		if (ffile->d_name[0]=='.' || !(dir2 = opendir(temp)))
			continue;
		strcpy(Dir[dc].DirName,ffile->d_name);
		Dir[dc].nf=0;
		while ((ffile2 = readdir(dir2))){
			char *t=ffile2->d_name;
			
			while(*t!='.')t++;
			if(!strcmp(t,".e3d")){//its an e3d add to list
				strcpy(Dir[dc].Files[i++],ffile2->d_name);
				Dir[dc].nf++;
			}    
		}
		closedir(dir2);
		dc++;
  }
  closedir(dir);
  
#endif
*/
}
