#include "global.h"

int view_buddy=0;
int buddy_menu_x=150;
int buddy_menu_y=70;
int buddy_menu_x_len=150;
int buddy_menu_y_len=200;
int buddy_menu_dragged=0;
_buddy buddy_list[100];
int bpage_start = 0;


int compare2( const void *arg1, const void *arg2)
{
   _buddy *b1=(_buddy*)arg1, *b2=(_buddy*)arg2;
   if(b1->type==b2->type)
      return strcmp(b1->name,b2->name);
   else
      return b1->type>b2->type ? 1: -1;
}

void display_buddy()
{
   int c=0,i=0,j,x=buddy_menu_x+2,y=buddy_menu_y+2;
   int scroll = (130*bpage_start)/(100-19);
   //title bar
   draw_menu_title_bar(buddy_menu_x,buddy_menu_y-16,buddy_menu_x_len);
   // window drawing
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE,GL_SRC_ALPHA);
   glDisable(GL_TEXTURE_2D);
   glBegin(GL_QUADS);
   glColor4f(0.0f,0.0f,0.0f,0.5f);
   glVertex3i(buddy_menu_x,buddy_menu_y+buddy_menu_y_len,0);
   glVertex3i(buddy_menu_x,buddy_menu_y,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y+buddy_menu_y_len,0);
   glEnd();
   glDisable(GL_BLEND);
   glColor3f(0.77f,0.57f,0.39f);
   glBegin(GL_LINES);
   glVertex3i(buddy_menu_x,buddy_menu_y,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y+buddy_menu_y_len,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y+buddy_menu_y_len,0);
   glVertex3i(buddy_menu_x,buddy_menu_y+buddy_menu_y_len,0);
   glVertex3i(buddy_menu_x,buddy_menu_y+buddy_menu_y_len,0);
   glVertex3i(buddy_menu_x,buddy_menu_y,0);
   // X corner
   glVertex3i(buddy_menu_x+buddy_menu_x_len,buddy_menu_y+20,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-20,buddy_menu_y+20,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-20,buddy_menu_y+20,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-20,buddy_menu_y,0);
   //scroll bar
   glVertex3i(buddy_menu_x+buddy_menu_x_len-20,buddy_menu_y+20,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-20,buddy_menu_y+200,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-15,buddy_menu_y+30,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-10,buddy_menu_y+25,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-10,buddy_menu_y+25,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-5,buddy_menu_y+30,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-15,buddy_menu_y+185,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-10,buddy_menu_y+190,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-10,buddy_menu_y+190,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-5,buddy_menu_y+185,0);
   glEnd();
   glBegin(GL_QUADS);
   //scroll bar
   glVertex3i(buddy_menu_x+buddy_menu_x_len-13,buddy_menu_y+35+scroll,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-7,buddy_menu_y+35+scroll,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-7,buddy_menu_y+55+scroll,0);
   glVertex3i(buddy_menu_x+buddy_menu_x_len-13,buddy_menu_y+55+scroll,0);
   glEnd();
   glEnable(GL_TEXTURE_2D);
   // The X
   draw_string(buddy_menu_x+buddy_menu_x_len-16,buddy_menu_y+2,"X",1);
   // Draw budies
   /* TEST
   if(bpage_start==0)
   {
      for(i=0;i<100;i++){
         buddy_list[i].name[2]=0;
         buddy_list[i].name[0]=rand()%70+55;
         buddy_list[i].type=rand()%4+1;
      }
   }*/

   qsort(buddy_list,100,sizeof(_buddy),compare2);
   i=bpage_start;
   while(c==buddy_list[i].type){
      i++;
   }
   c++;
   glColor3f(1.0,0,0);
   while(c==buddy_list[i].type){
      if(i-bpage_start>18)return;
      draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
      y+=10;
      i++;
   }
   c++;
   glColor3f(0,1.0,0);
   while(c==buddy_list[i].type){
      if(i-bpage_start>18)return;
      draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
      y+=10;
      i++;
   }
   c++;
   glColor3f(0,0,1.0);
   while(c==buddy_list[i].type){
      if(i-bpage_start>18)return;
      draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
      y+=10;
      i++;
   }
   c++;
   glColor3f(1.0,1.0,0);
   while(c==buddy_list[i].type){
      if(i-bpage_start>18)return;
      draw_string_zoomed(x,y,buddy_list[i].name,1,0.7);
      y+=10;
      i++;
   }

}

int check_buddy_interface()
{
   int x,y;
   if(!view_buddy || mouse_x>buddy_menu_x+buddy_menu_x_len || mouse_x<buddy_menu_x
      || mouse_y<buddy_menu_y || mouse_y>buddy_menu_y+buddy_menu_y_len)return 0;

   x=mouse_x-buddy_menu_x;
   y=mouse_y-buddy_menu_y;

   if(x > buddy_menu_x_len-16 && x < buddy_menu_x_len &&
      y > 18 && y < 18+16)
      {
         if(bpage_start > 0)
            bpage_start--;
         return 1;
      }
   if(x > buddy_menu_x_len-16 && x < buddy_menu_x_len &&
      y > 180 && y < 180+16)
      {
         if(bpage_start < 100-19)
            bpage_start++;
         return 1;
      }
   if(x>buddy_menu_x_len-20)
      return 1;
   
   y/=10;
   y+=bpage_start;
   sprintf(input_text_line,"/%s ",buddy_list[y].name);
   input_text_lenght=strlen(input_text_line);
   return 1;
}
