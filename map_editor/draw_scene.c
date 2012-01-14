#include <SDL.h>
#include "../asc.h"
#include "global.h"
#include <math.h>
#include <string.h>

#ifdef EYE_CANDY
 #include "../eye_candy_wrapper.h"
 #include "eye_candy_window.h" 
#endif

float camera_x=0;
float camera_y=0;
float camera_z=0;
float old_camera_x=0;
float old_camera_y=0;
float old_camera_z=0;
float c_delta= 0.1f;
float rx=-60;
float ry=0;
float rz=45;

float mx = 0;
float my = 0;
float mz = 0;


float terrain_scale=2.0f;
float zoom_level=3.0f;
float name_zoom=1.0f;



float fine_camera_rotation_speed=10.0f;
float normal_camera_rotation_speed=10.0f;

float camera_rotation_speed;
int camera_rotation_frames;

float camera_tilt_speed;
int camera_tilt_frames;

double camera_x_speed;
int camera_x_frames;

double camera_y_speed;
int camera_y_frames;

double camera_z_speed;
int camera_z_frames;

int camera_zoom_dir;
int camera_zoom_frames=0;
float new_zoom_level=3.0f;
float camera_distance = 2.5f;

void move_camera();

void draw_scene()
{
    char str[256];
    int fps;
    int any_reflection=0;

    glClearColor( 0.1, 0.1, 0.1, 0.0 );

    if(!shadows_on)
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    else
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glLoadIdentity();   // Reset The Matrix
    if(minimap_on) {
        Enter2DMode();
        draw_minimap();
        Leave2DMode();
        SDL_GL_SwapBuffers();
        return;
    }


    move_camera();

    //CalculateFrustum();
    new_minute();

    any_reflection=find_reflection();

    if(!dungeon)
        draw_global_light();
    else 
        draw_dungeon_light();

    update_scene_lights();
    draw_lights();

	if(any_reflection>1) {
        if(!dungeon)
            draw_sky_background();
        else 
            draw_dungeon_sky_background();

        CHECK_GL_ERRORS();
        glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up

        if(view_tile || cur_mode==mode_tile)
            draw_tile_map();
        CHECK_GL_ERRORS();

        if(view_2d || cur_mode==mode_2d)
            display_2d_objects();
        CHECK_GL_ERRORS();

        display_3d_reflection();
        glNormal3f(0.0f,0.0f,1.0f);
        draw_lake_tiles();
    } else {
        glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up

        if(view_tile || cur_mode==mode_tile)
            draw_tile_map();

	    if(any_reflection)
            draw_lake_tiles();
        CHECK_GL_ERRORS();
        
        if(view_2d || cur_mode==mode_2d)
            display_2d_objects();
    }

	CHECK_GL_ERRORS();

	if(view_3d || cur_mode==mode_3d) {
        if(shadows_on) {
            if(!dungeon) {
                if(day_shadows_on)
                    draw_sun_shadowed_scene();
                else 
                    draw_night_shadowed_scene();

            } else {
                draw_night_shadowed_scene();
            }
        } else {
            display_objects();
        }
    }

    //if the shadows are off, then draw everything front to back
    if(!shadows_on) {
        glNormal3f(0.0f,0.0f,1.0f);//the normal for ground objects and such points up
        if(view_2d || cur_mode==mode_2d)display_2d_objects();
        if(view_tile || cur_mode==mode_tile)draw_tile_map();
        if(find_reflection())draw_lake_tiles();
    }
    
    if(view_particles || cur_mode==mode_particles) {
        glDisable(GL_LIGHTING);
        display_particles();
        if(view_particle_handles)
             display_particle_handles();
        glEnable(GL_LIGHTING);
    }
    
#ifdef  EYE_CANDY
    ec_idle();
    ec_draw();
    if (cur_mode == mode_eye_candy)
      draw_eye_candy_selectors();
#endif

    if(view_grid)
        draw_heights_wireframe();

    if(view_height || cur_mode==mode_height)
        draw_height_map();

    if(view_light || cur_mode==mode_light)
        visualise_lights();

    if(move_tile_a_tile)
        move_tile();

    if(move_tile_a_height)
        move_height_tile();

    Enter2DMode();

    //get the FPS, etc
    if(cur_time-last_time) 
        fps=1000/(cur_time-last_time);
    else 
        fps=1000;

    glColor3f(1.0f,1.0f,1.0f); //default color is white
    snprintf(str,sizeof(str), "Sx: %03.1f,Sy: %03.1f, Sz: %03.1f, camera_x: %03.2f, camera_y: %03.2f,rx: %03.2f, rz: %03.2f\nFPS: %i, Minute: %i",fLightPos[0],fLightPos[1],fLightPos[2],-camera_x,-camera_y,rx,rz,fps,game_minute);

    draw_string (10, 40, (const unsigned char*) str, 2);
    draw_toolbar();

    display_windows(1);

#ifdef EYE_CANDY
    draw_eye_candy_obj_info();
#endif
    draw_3d_obj_info();
    draw_2d_obj_info();
    draw_light_info();
    draw_height_info();
    //display_new_map_menu();
    display_map_settings();

    Leave2DMode();
    glEnable(GL_LIGHTING);
    SDL_GL_SwapBuffers();
}

void Move()
{
    glRotatef(rx, 1.0f, 0.0f, 0.0f);
    glRotatef(rz, 0.0f, 0.0f, 1.0f);
    glTranslatef(camera_x, camera_y, camera_z);
}

#define TIMER_RATE 20
int my_timer_clock=0;
int normal_animation_timer=0;

Uint32 my_timer (Uint32 interval)
{
    int new_time;
    SDL_Event e;

    if(my_timer_clock==0)
        my_timer_clock=SDL_GetTicks();
    else 
        my_timer_clock+=TIMER_RATE;

    if(normal_animation_timer>2) {
        normal_animation_timer=0;
        update_particles();

        if(lake_waves_timer>2) {
            lake_waves_timer=0;
            make_lake_water_noise();
        }
        lake_waves_timer++;
        water_movement_u+=0.0004f;
        water_movement_v+=0.0002f;
    }
    normal_animation_timer++;

    e.type = SDL_USEREVENT;
    e.user.code = EVENT_UPDATE_CAMERA;
    SDL_PushEvent(&e);


    new_time = TIMER_RATE-(SDL_GetTicks()-my_timer_clock);

    if(new_time<10) 
        new_time=10;

    return new_time;
}


void move_camera ()
{
    float x, y, z;

    x=mx;
    y=my;
    z=mz;

    camera_x_speed=(x-(-camera_x))/1.0;
    camera_x_frames=1;
    camera_y_speed=(y-(-camera_y))/1.0;
    camera_y_frames=1;
    camera_z_speed=(z-(-camera_z))/1.0;
    camera_z_frames=1;

//    glTranslatef(0.0f, 0.0f, -zoom_level*camera_distance);
    glRotatef(rx, 1.0f, 0.0f, 0.0f);
    glRotatef(rz, 0.0f, 0.0f, 1.0f);
    glTranslatef(camera_x, camera_y, camera_z);

}


void update_camera()
{
    int adjust_view= 0;


    if(camera_rotation_frames) {

        rz+=camera_rotation_speed;
        if(rz > 360) {
            rz -= 360;
        } else if (rz < 0) {
            rz += 360;
        }
        camera_rotation_frames--;
        adjust_view++;
    }

    if(camera_x_frames) {
        if(camera_x_speed>0.005 || camera_x_speed<-0.005){
            camera_x-=camera_x_speed;
            if(fabs(camera_x-old_camera_x) >= c_delta){
                adjust_view++;
            }
        }
        camera_x_frames--;
    }

    if(camera_y_frames) {
        if(camera_y_speed>0.0005 || camera_y_speed<-0.005){
            camera_y-=camera_y_speed;
            if(fabs(camera_y-old_camera_y) >= c_delta){
                adjust_view++;
            }
        }
        camera_y_frames--;
    }

    if(camera_z_frames) {
        if(camera_z_speed>0.0005 || camera_z_speed<-0.005) {
            camera_z-=camera_z_speed;
#ifdef  PARANOID_CAMERA
            if(fabs(camera_z-old_camera_z) >= c_delta){
                adjust_view++;
            }
#endif
        }
        camera_z_frames--;
    }

    if(camera_tilt_frames) {
        if(camera_tilt_speed<0) {
            if(rx>-70)
                rx+=camera_tilt_speed;
            
            if(rx<-70) {
                rx=-70;
                camera_tilt_frames=0;
            } else
                camera_tilt_frames--;
        } else {
            if(rx< 0)
                rx+=camera_tilt_speed;
            if(rx > 0) {
                rx=0;
                camera_tilt_frames=0;
            } else
                camera_tilt_frames--;
        }
    }

/*
    if(camera_zoom_frames) {
        if(camera_zoom_dir == 1) {
            if(zoom_level<3.75f){
                new_zoom_level+=0.05f;
                camera_zoom_frames--;
                adjust_view++;
            } else 
                camera_zoom_frames = 0;
        } else {
//            if(zoom_level>sitting){
//                new_zoom_level-=0.05f;
//                camera_zoom_frames--;
//                adjust_view++;
//            } else 
                camera_zoom_frames = 0;
        }
    }
*/
    if(adjust_view){
//        set_all_intersect_update_needed(main_bbox_tree);
        old_camera_x= camera_x;
        old_camera_y= camera_y;
        old_camera_z= camera_z;
    }

}


