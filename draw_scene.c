#include <SDL.h>
#include "global.h"
#include <math.h>
void Move();

void draw_scene()
{
        unsigned char str [180];
        int fps;
        int y_line;

        cur_time = SDL_GetTicks();


        if(!shadows_on)glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        else glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        glLoadIdentity();					// Reset The Matrix
        Move();
        if(minimap_on)
            {
                Enter2DMode();
                draw_minimap();
                Leave2DMode();
                SDL_GL_SwapBuffers();
                return;
            }
        new_minute();
        if(!dungeon)draw_global_light();
        else draw_dungeon_light();
        update_scene_lights();
        draw_lights();

          if(find_reflection())
            {
                if(!dungeon)draw_sky_background();
                else draw_dungeon_sky_background();
                display_3d_reflection();
                glNormal3f(0.0f,0.0f,1.0f);
                //draw_lake_tiles();
            }
           //if the shadows are on, then draw everything back to front
          if(shadows_on)
            {
                glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up
                if(view_tile || cur_mode==mode_tile)draw_tile_map();
                if(view_2d || cur_mode==mode_2d)display_2d_objects();
                if(find_reflection())draw_lake_tiles();
            }

          if(view_3d || cur_mode==mode_3d)
          {
            if(!dungeon)
                {
                if(shadows_on)if(day_shadows_on)draw_sun_shadowed_scene();
                if(shadows_on)if(night_shadows_on)draw_night_shadowed_scene();
                }
            if(dungeon)
                {
                if(shadows_on)draw_night_shadowed_scene();
                }

            if(!shadows_on)display_objects();
           }
           //if the shadows are off, then draw everything front to back
          if(!shadows_on)
            {
                glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up
                if(view_2d || cur_mode==mode_2d)display_2d_objects();
                if(view_tile || cur_mode==mode_tile)draw_tile_map();
                if(find_reflection())draw_lake_tiles();
            }

          if(view_grid)draw_heights_wireframe();
          if(view_height || cur_mode==mode_height)draw_height_map();
          if(view_light || cur_mode==mode_light)visualise_lights();
          if(move_tile_a_tile)move_tile();
          if(move_tile_a_height)move_height_tile();

          Enter2DMode();

          //get the FPS, etc
          if(cur_time-last_time)fps=1000/(cur_time-last_time);
          else fps=1000;

          glColor3f(1.0f,1.0f,1.0f);//default color is white
          sprintf(str, "Sx: %03.1f,Sy: %03.1f, Sz: %03.1f, cx: %03.2f, cy: %03.2f,rx: %03.2f, rz: %03.2f\nFPS: %i, Minute: %i",fLightPos[0],fLightPos[1],fLightPos[2],cx,cy,rx,rz,fps,game_minute);

          draw_string(10,40,str,2);
          draw_toolbar();
          if(view_tiles_list)display_tiles_list();
          if(view_heights_list)display_heights_list();
          draw_3d_obj_info();
          draw_2d_obj_info();
          draw_light_info();
          draw_height_info();
          display_new_map_menu();
          display_map_settings();

          Leave2DMode();
          glEnable(GL_LIGHTING);
          SDL_GL_SwapBuffers();
}

void Move()
{
 	   glRotatef(rx, 1.0f, 0.0f, 0.0f);
	   glRotatef(rz, 0.0f, 0.0f, 1.0f);
	   glTranslatef(cx, cy, cz);

}

Uint32 my_timer(unsigned int some_int)
{
    if(lake_waves_timer>2)
        {
            lake_waves_timer=0;
            make_lake_water_noise();
        }
    lake_waves_timer++;
    water_movement_u+=0.0004f;
    water_movement_v+=0.0002f;

    return some_int;
}
