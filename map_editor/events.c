#include "global.h"
#include "eye_candy_window.h"

int undo_type;
void *undo_object = NULL;
int undo_tile_value;
int undo_tile_height;
int undo_tile = -1;
int calhm = 0;

#define ROT_LSPEED    1.0f 
#define ROT_HSPEED   10.0f
#define ROT_DELTA(flag) (flag ? ROT_LSPEED : ROT_HSPEED)

void zoomin(){
	zoom_level -= ctrl_on ? 2.5f : 0.25f;
	if(zoom_level<1.0f) zoom_level = 1.0f;
	window_resize();
}

void zoomout(){
	zoom_level += ctrl_on ? 2.5f : 0.25f;	
	window_resize();
}

int HandleEvent(SDL_Event *event)
{
	int done=0;
	Uint8 ch=0;

    mod_key_status=SDL_GetModState();

    shift_on = mod_key_status&KMOD_SHIFT ? 1 : 0;
    ctrl_on  = mod_key_status&KMOD_CTRL  ? 1 : 0;
    alt_on   = mod_key_status&KMOD_ALT   ? 1 : 0;


    view_particles_window=get_show_window(particles_window);

    switch( event->type ) {
        case SDL_KEYDOWN:
            if (ctrl_on && !alt_on && !shift_on) {
                switch(event->key.keysym.sym) {
                    // FIXME what mean gcr,gcg,gcb and why this not in structure
                    case SDLK_1: gcr=1.0f; gcg=1.0f; gcb=1.0f; break;
                    case SDLK_2: gcr=1.0f; gcg=0.0f; gcb=0.0f; break;
                    case SDLK_3: gcr=0.0f; gcg=1.0f; gcb=0.0f; break;
                    case SDLK_4: gcr=0.0f; gcg=0.0f; gcb=1.0f; break;
                    case SDLK_5: gcr=0.0f; gcg=0.0f; gcb=0.0f; break;

                    case SDLK_b: toggle_window(browser_win);        break;
                    case SDLK_w: toggle_window(o3dow_win);          break;
                    case SDLK_r: toggle_window(replace_window_win); break;
                    case SDLK_e: toggle_window(edit_window_win);    break;
                    case SDLK_h: calhm = !calhm;                    break;

                    case SDLK_p: 
                        toggle_particles_window(); 
                        toggle_window(particles_window); 
                        break;

                    case SDLK_z:
                        if(undo_object != NULL) {
                            switch (undo_type) {
                                case mode_3d: {
                                        object3d *o = (object3d *)undo_object;
                                        add_e3d(o->file_name, o->x_pos, o->y_pos, o->z_pos, o->x_rot, o->y_rot, o->z_rot, o->self_lit, o->blended, o->color[0], o->color[1], o->color[2]);
                                        free(undo_object);
                                        undo_object = NULL;
                                    }
                                    break;

                                case mode_2d: {
                                        obj_2d *o = (obj_2d *) undo_object;
                                        add_2d_obj(o->file_name, o->x_pos, o->y_pos, o->z_pos, o->x_rot, o->y_rot, o->z_rot);
                                        free(undo_object);
                                        undo_object = NULL;
                                    }
                                    break;

                                case mode_light: {
                                        light *o = (light *) undo_object;
                                        add_light(o->pos_x, o->pos_y, o->pos_z, o->r, o->g, o->b, 1.0f, o->locked);
                                        free(undo_object);
                                        undo_object = NULL;
                                    }
                                    break;

                                case mode_particles: {
                                        particle_sys *o = (particle_sys *)undo_object;
                                        create_particle_sys(o->def,o->x_pos,o->y_pos,o->z_pos);
                                        free(undo_object);
                                        undo_object = NULL;
                                    }
                                    break;
                            }
                        } else {
                            if(undo_tile != -1 && undo_type == mode_tile) {
                                tile_map[undo_tile] = undo_tile_value;
                                undo_tile = -1;
                            }
                        }
                        break;
                    default:
                        break;
                }
            } else if (!ctrl_on && !alt_on && !shift_on) {
                switch(event->key.keysym.sym) {
                    case SDLK_o:
                        // FIXME what doing rx variable
                        if(rx == -60)
                            rx = 0;
                        else 
                            rx = -60;
                        break;

                    case SDLK_F12:      zoom_level=3.75f; window_resize(); break;
                    case SDLK_TAB:      heights_3d=!heights_3d; break;
                    case SDLK_g:        view_grid=!view_grid; break;

                    // FIXME what this?
                    case SDLK_ESCAPE:   done = 1; break;

                    case SDLK_F1:       game_minute=(game_minute >  60*5 ?     0: game_minute + 1); break;
                    case SDLK_F2:       game_minute=(game_minute == 0    ? 60*5 : game_minute - 1); break;
                    case SDLK_n:        game_minute=0;  break;
                    case SDLK_d:        game_minute=60; break;

                    case SDLK_KP_PLUS:
                        if(view_tiles_list){
                            if(tile_offset<192)
                                tile_offset+=64;
						} else 
                            grid_height+=0.1f;
                        break;

                    case SDLK_KP_MINUS:
                        if(view_tiles_list){
                            if(tile_offset>0)
                                tile_offset -= 64;
						} else 
                            grid_height -= 0.1f;
                        break;

                    default:
                        break;
                }
            }

            // process key, what push not depening on extended keys
            switch(event->key.keysym.sym) {
                case SDLK_LEFT:
                    if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
                        objects_list[selected_3d_object]->z_rot -= ROT_DELTA(shift_on);  

                    else if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
                        obj_2d_list[selected_2d_object]->z_rot -= ROT_DELTA(shift_on);
    
                    else {
                        mx -= sin((rz+90)*3.1415926/180);
                        my -= cos((rz+90)*3.1415926/180);
                    }
                    break;

                case SDLK_RIGHT:
                    if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
                        objects_list[selected_3d_object]->z_rot += ROT_DELTA(shift_on);  

                    else if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
                        obj_2d_list[selected_2d_object]->z_rot += ROT_DELTA(shift_on);

                    else {
                        mx += sin((rz+90)*3.1415926/180);
                        my += cos((rz+90)*3.1415926/180);
                    }
                    break;


                case SDLK_UP:
                    if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
                        objects_list[selected_3d_object]->x_rot -= ROT_DELTA(shift_on);
                    else if (ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
                        obj_2d_list[selected_2d_object]->x_rot -= ROT_DELTA(shift_on);
                    else {
                        mx += sin(rz*3.1415926/180);
                        my += cos(rz*3.1415926/180);
                    }
                    break;
                
                case SDLK_DOWN:
                    if(ctrl_on && cur_mode==mode_3d && selected_3d_object!=-1)
                        objects_list[selected_3d_object]->x_rot += ROT_DELTA(shift_on);
                    else if(ctrl_on && cur_mode==mode_2d && selected_2d_object!=-1)
                        obj_2d_list[selected_2d_object]->x_rot += ROT_DELTA(shift_on);
                    else {
                        mx -= sin(rz*3.1415926/180);
                        my -= cos(rz*3.1415926/180);
                    }

                    break;

                case SDLK_HOME: rz += (shift_on ? 1.0f : 20.0f); break;
                case SDLK_END:  rz -= (shift_on ? 1.0f : 20.0f); break;
                case SDLK_PAGEUP:
                    if (!shift_on && !ctrl_on)
                        if (view_particles_window)
                            particles_win_zoomin();
                        else 
                            zoomin();
                    else if (cur_mode == mode_3d && selected_3d_object != -1)
                        objects_list[selected_3d_object]->y_rot -= (shift_on ? 1.0f : 10.0f);  

                    break;

                case SDLK_PAGEDOWN:
                    if (!shift_on && !ctrl_on) {
                        if (view_particles_window)
                            particles_win_zoomout ();
                        else
                            zoomout ();
                    } else if (cur_mode == mode_3d && selected_3d_object != -1) {
                        objects_list[selected_3d_object]->y_rot += (shift_on ? 1.0f : 10.0f);
                    }
                    break;

                case SDLK_INSERT:
                case SDLK_BACKSPACE:
                    if (cur_mode == mode_3d && selected_3d_object != -1) {
                        objects_list[selected_3d_object]->z_pos += (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode == mode_2d && selected_2d_object != -1) {
                        obj_2d_list[selected_2d_object]->z_pos += (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode == mode_light && selected_light != -1 && !lights_list[selected_light]->locked) {
                        lights_list[selected_light]->pos_z += (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode == mode_particles) {
                        if (selected_particles_object != -1) {
                            particles_list[selected_particles_object]->z_pos += (shift_on ? 0.01f : 0.1f);
                            if (particles_list[selected_particles_object]->def->use_light) 
                                lights_list[particles_list[selected_particles_object]->light]->pos_z += (shift_on ? 0.01f : 0.1f);
                        }
                        if (view_particles_window) {
                            particles_win_move_preview (shift_on ? 0.01f : 0.1f);
                        }
                    } else if (cur_mode == mode_height && selected_height != -1) {
                        if (selected_height < 31) 
                            selected_height++;
#ifdef	EYE_CANDY
                    } else if (cur_mode == mode_eye_candy && eye_candy_confirmed) {
                    	eye_candy_adjust_z((shift_on ? 0.01f : 0.1f));
#endif	//EYE_CANDY
                    }
                    break;

                case SDLK_DELETE:
                    if (cur_mode == mode_3d && selected_3d_object != -1) {
                        objects_list[selected_3d_object]->z_pos -= (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode == mode_2d && selected_2d_object != -1) {
                        obj_2d_list[selected_2d_object]->z_pos -= (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode == mode_light && selected_light != -1 && !lights_list[selected_light]->locked) {
                        lights_list[selected_light]->pos_z -= (shift_on ? 0.01f : 0.1f);
                    } else if (cur_mode==mode_particles) {
                        if (selected_particles_object != -1) {
                            particles_list[selected_particles_object]->z_pos -= (shift_on ? 0.01f : 0.1f);
                            if (particles_list[selected_particles_object]->def->use_light) 
                                lights_list[particles_list[selected_particles_object]->light]->pos_z -= (shift_on ? 0.01f : 0.1f);
                        }
                        if (view_particles_window)
                            particles_win_move_preview (shift_on ? -0.01f : -0.1f);

                    } else if (cur_mode == mode_height && selected_height != -1) {
                        if (selected_height > 0)
                            selected_height--;
#ifdef	EYE_CANDY
                    } else if (cur_mode == mode_eye_candy && eye_candy_confirmed) {
                    	eye_candy_adjust_z(-(shift_on ? 0.01f : 0.1f));
#endif	//EYE_CANDY
                    }
                    break; // END DELETE
                default:
                    break;
            
            } //switch(event->key.keysym.sym)  




            //see if we get any text
            if ((event->key.keysym.unicode & 0xFF80)==0)
                ch = event->key.keysym.unicode & 0x7F;
            
            //check wehter we should switch shadows on/off
            if((ch=='s' || ch=='S') && alt_on)
                shadows_on=!shadows_on;
            
            //do we want to toggle the transparency of a 3d object?
            if(ch=='b' && selected_3d_object!=-1 && cur_mode==mode_3d)
                objects_list[selected_3d_object]->blended=!objects_list[selected_3d_object]->blended;
            
            if(ch=='l' && selected_3d_object!=-1 && cur_mode==mode_3d)
                objects_list[selected_3d_object]->self_lit=!objects_list[selected_3d_object]->self_lit;
            
            //do the lightening stuff
            if(ch=='1' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
                if(objects_list[selected_3d_object]->color[0]<1.0f)
                    objects_list[selected_3d_object]->color[0]+=0.05f;
            
            if(ch=='1' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
                if(objects_list[selected_3d_object]->color[0]>0.0f)
                    objects_list[selected_3d_object]->color[0]-=0.05f;
            
            if(ch=='2' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
                if(objects_list[selected_3d_object]->color[1]<1.0f)
                    objects_list[selected_3d_object]->color[1]+=0.05f;
            
            if(ch=='2' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
                if(objects_list[selected_3d_object]->color[1]>0.0f)
                    objects_list[selected_3d_object]->color[1]-=0.05f;
            
            if(ch=='3' && selected_3d_object!=-1 && cur_mode==mode_3d && !alt_on)
                if(objects_list[selected_3d_object]->color[2]<1.0f)
                    objects_list[selected_3d_object]->color[2]+=0.05f;
            
            if(ch=='3' && selected_3d_object!=-1 && cur_mode==mode_3d && alt_on)
                if(objects_list[selected_3d_object]->color[2]>0.0f)
                    objects_list[selected_3d_object]->color[2]-=0.05f;
            
            //for lights now
            if(ch=='1' && selected_light!=-1 && cur_mode==mode_light && !alt_on && !lights_list[selected_light]->locked)
                if(lights_list[selected_light]->r<5.0f)
                    lights_list[selected_light]->r+=0.1f;
            
            if(ch=='1' && selected_light!=-1 && cur_mode==mode_light && alt_on && !lights_list[selected_light]->locked)
                if(lights_list[selected_light]->r>0.0f)
                    lights_list[selected_light]->r-=0.1f;
            
            if(ch=='2' && selected_light!=-1 && cur_mode==mode_light && !alt_on && !lights_list[selected_light]->locked)
                if(lights_list[selected_light]->g<5.0f)
                    lights_list[selected_light]->g+=0.1f;
            
            if(ch=='2' && selected_light!=-1 && cur_mode==mode_light && alt_on && !lights_list[selected_light]->locked)
                if(lights_list[selected_light]->g>0.0f)
                    lights_list[selected_light]->g-=0.1f;
            
            if(ch=='3' && selected_light!=-1 && cur_mode==mode_light && !alt_on && !lights_list[selected_light]->locked)
            if(lights_list[selected_light]->b<5.0f)lights_list[selected_light]->b+=0.1f;
            if(ch=='3' && selected_light!=-1 && cur_mode==mode_light && alt_on && !lights_list[selected_light]->locked)
            if(lights_list[selected_light]->b>0.0f)lights_list[selected_light]->b-=0.1f;
            
            //for ambient light
            if(ch=='1' && cur_mode==mode_map && !alt_on)
            if(ambient_r<1.0f)ambient_r+=0.02f;
            if(ch=='1' && cur_mode==mode_map && alt_on)
            if(ambient_r>-0.05f)ambient_r-=0.02f;
            if(ch=='2' && cur_mode==mode_map && !alt_on)
            if(ambient_g<1.0f)ambient_g+=0.02f;
            if(ch=='2' && cur_mode==mode_map && alt_on)
            if(ambient_g>-0.05f)ambient_g-=0.02f;
            if(ch=='3' && cur_mode==mode_map && !alt_on)
            if(ambient_b<1.0f)ambient_b+=0.02f;
            if(ch=='3' && cur_mode==mode_map && alt_on)
            if(ambient_b>-0.05f)ambient_b-=0.02f;
            if((ch=='d' || ch=='D') && cur_mode==mode_map)dungeon=!dungeon;
            
            if ((ch=='m') || ((ch == 'q') && (minimap_on)))
            {
#ifdef	EYE_CANDY
              if ((cur_mode == mode_eye_candy) && (eye_candy_ready_to_add == 1))
              {
                  eye_candy_ready_to_add = 2;
                  change_eye_candy_effect();
//                eye_candy_add_effect();
//                cur_mode = mode_tile;
              }
              
#endif	//EYE_CANDY
              
              map_has_changed=(minimap_on=!minimap_on);
            }

            break;

        case SDL_VIDEORESIZE:
            window_width = event->resize.w;
            window_height = event->resize.h;
#ifdef LINUX
            if(SDL_SetVideoMode(window_width, window_height, bpp, SDL_OPENGL|SDL_RESIZABLE))
                window_resize();
#else
            handle_window_resize();
#endif
            break;

        case SDL_USEREVENT:
            switch(event->user.code){
                case EVENT_UPDATE_CAMERA:
                    update_camera();
                    break;
            }
            break;

        case SDL_QUIT:
		    done = 1;
		    break;
    }

	// zooming with mousewheel...
	if(event->type==SDL_MOUSEBUTTONDOWN){
	  if(event->button.button == SDL_BUTTON_WHEELUP){
	    if(view_particles_window)particles_win_zoomin();
	    else zoomin();
	  }
	  if(event->button.button == SDL_BUTTON_WHEELDOWN){
	    if(view_particles_window)particles_win_zoomout();
	    else zoomout();
	  }
	} // *

    if(event->type==SDL_MOUSEMOTION)
				{
					mouse_x= event->motion.x;
					mouse_y= event->motion.y;

					mouse_delta_x= event->motion.xrel;
					mouse_delta_y= event->motion.yrel;
				}
			else
				{
					// why was this here? whats broken by removing it?
					//mouse_x= event->button.x;
					//mouse_y= event->button.y;
					mouse_delta_x= mouse_delta_y= 0;
				}


	if(event->type==SDL_MOUSEMOTION || event->type==SDL_MOUSEBUTTONDOWN || event->type==SDL_MOUSEBUTTONUP)
		{
			char tool_bar_click=0;
			// why was this here? whats broken by removing it?
			//mouse_x=event->motion.x;
			//mouse_y=event->motion.y;

            get_world_x_y();

            if ( SDL_GetMouseState (NULL, NULL) & SDL_BUTTON(2) )
            {
                camera_rotation_speed = normal_camera_rotation_speed * mouse_delta_x / 220;
                camera_rotation_frames = 40;
                camera_tilt_speed = normal_camera_rotation_speed * mouse_delta_y / 220;
                camera_tilt_frames = 40;
//                printf("mouse_delta_x %i mouse_delta_y %i rotation_speed %f tilt_speed %f \n",mouse_delta_x,mouse_delta_y,camera_rotation_speed,camera_tilt_speed);   
            }

 		           //get the buttons state
			if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_LEFT))
			{
				left_click++;
			}
			else{
				if(left_click) end_drag_windows();
				left_click = 0;
#ifdef	EYE_CANDY
				last_ec_index = -2;
#endif	//EYE_CANDY
			}

			if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_RIGHT))
				right_click++;
			else
				right_click= 0;

			if (SDL_GetMouseState (NULL, NULL) & SDL_BUTTON (SDL_BUTTON_MIDDLE))
				middle_click++;
			else
				middle_click= 0;

			if((left_click==1 || right_click==1))
				if(click_in_windows(mouse_x, mouse_y, 1)>0)
					return (done);

			if(shift_on && ctrl_on && left_click==1 && cur_mode != mode_height){
				get_world_x_y();
				mx=scene_mouse_x;
				my=scene_mouse_y;
				return(done);
			}

			if(minimap_on && left_click==1)
				{
#ifdef	EYE_CANDY
					if ((cur_mode != mode_eye_candy) || (!eye_candy_ready_to_add))
					{
						check_mouse_minimap();
						return(done);
					}
					else
					{
						add_eye_candy_point();
						return(done);
					}
#else	//EYE_CANDY
					check_mouse_minimap();
					return(done);
#endif	//EYE_CANDY
				}
			else
			if(minimap_on && right_click)
				{
					if (cur_mode == mode_tile)
					{
						draw_mouse_minimap();
						return(done);
					}
#ifdef	EYE_CANDY
					else if (cur_mode == mode_eye_candy)
					{
						delete_eye_candy_point();
						return(done);
					}
#endif	//EYE_CANDY
				}

			if(left_click && cur_mode==mode_tile && cur_tool==tool_select && selected_tile!=255  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
			{
				tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=selected_tile;
				if(selected_tile == 0 || selected_tile == 20 || selected_tile == 21)
					kill_height_map_at_texture_tile((int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3);
				return(done);
			}
			
			if(left_click && shift_on && cur_mode==mode_tile && cur_tool==tool_kill && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
			{
				tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=255;
				if(selected_tile == 0 || selected_tile == 20 || selected_tile == 21)
					kill_height_map_at_texture_tile((int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3);
				return(done);
			}
			if(left_click && shift_on && cur_mode==mode_height && cur_tool==tool_kill && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
			{
				//tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=255;
				//if(selected_tile == 0 || selected_tile == 20 || selected_tile == 21)
					//kill_height_map_at_texture_tile((int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3);
				height_map[(int)(scene_mouse_y*2.0f)*tile_map_size_x*6+(int)(scene_mouse_x*2.0f)]=11;
				return(done);
			}
			if(left_click > 1 && cur_mode==mode_height && cur_tool==tool_select && selected_height!=-1  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
			{
				if(alt_on && ctrl_on)
					draw_big_height_tile(5);
				else if(alt_on)
					draw_big_height_tile(3);
				else if(ctrl_on)
					draw_big_height_tile(1);
				else
					height_map[(int)(scene_mouse_y*2)*tile_map_size_x*6+(int)(scene_mouse_x*2)]=selected_height;
			}
			
#ifdef EYE_CANDY
			if (!minimap_on && (mouse_x>=15*32 || mouse_y>=32))
			{
				if (left_click && (cur_mode == mode_eye_candy))
				{
					eye_candy_add_effect();
				}
			}
#endif

			if(check_interface_buttons()==1)tool_bar_click=1;
			if(right_click==1 && cur_tool==tool_select && selected_tile!=255 && cur_mode==mode_tile)selected_tile=255;
			if(right_click==1 && cur_tool==tool_select && selected_height!=-1 && cur_mode==mode_height)selected_height=-1;
			if(right_click==1 && cur_tool==tool_select && selected_2d_object!=-1 && cur_mode==mode_2d)kill_2d_object(selected_2d_object);
			if(right_click==1 && cur_tool==tool_select && selected_3d_object!=-1 && cur_mode==mode_3d)kill_3d_object(selected_3d_object);
#ifdef EYE_CANDY
			if(right_click==1 && cur_tool==tool_select && cur_mode==mode_eye_candy)
			{
				get_3d_object_under_mouse();
				if (selected_3d_object >= MAX_OBJ_3D)
				{
					select_eye_candy_effect(selected_3d_object);
					kill_eye_candy_effect();
					selected_3d_object = -1;
				}
			}
#endif
			if(right_click==1 && cur_tool==tool_select && cur_mode==mode_particles && selected_particles_object!=-1)kill_particles_object(selected_particles_object);
			if(right_click==1 && cur_mode==mode_tile && view_tiles_list)
				{
					view_tiles_list=0;
					cur_tool=tool_select;
					selected_tile=0;
				}

			

			if(!tool_bar_click)
				{
					if(left_click==1)
						{
							if(cur_mode==mode_3d && left_click == 1)
							{
								if(cur_tool==tool_kill)
									{
										get_3d_object_under_mouse();
										if(selected_3d_object!=-1){
											undo_type = mode_3d;
											if(undo_object == NULL)
												free(undo_object);
											undo_object = (object3d *) malloc(sizeof(object3d));
											memcpy(undo_object,objects_list[selected_3d_object],sizeof(object3d));
											if(calhm)
												clear_e3d_heightmap(selected_3d_object);
											kill_3d_object(selected_3d_object);
										}

										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_3d_object_under_mouse();
										if(selected_3d_object!=-1)
					                                            clone_3d_object(selected_3d_object);
										return(done);
									}

								//if we have an object attached to us, drop it
								if(cur_tool==tool_select && selected_3d_object!=-1)
        							{
										///
										if(c1){
											if(c2)
												objects_list[selected_3d_object]->x_rot=randomanglex?((minax + (int)(((double)(maxax-minax+1) * rand()) / (RAND_MAX+1.0)))):(rand()%360);
											if(c3)
												objects_list[selected_3d_object]->y_rot=randomangley?((minay + (int)(((double)(maxay-minay+1) * rand()) / (RAND_MAX+1.0)))):(rand()%360);
											if(c4)
												objects_list[selected_3d_object]->z_rot=randomanglez?((minaz + (int)(((double)(maxaz-minaz+1) * rand()) / (RAND_MAX+1.0)))):(rand()%360);
										}
										if(randomheight){
												objects_list[selected_3d_object]->z_pos=(float)(minh + (int)(((double)(maxh-minh+1) * rand()) / (RAND_MAX+1.0)))/10 ;
										}
										if(ctrl_on)
											clone_3d_object(selected_3d_object);
										else{
											if(calhm)
												add_e3d_heightmap(selected_3d_object, 3);
											selected_3d_object=-1;
										}
									}
								else
									{
										get_3d_object_under_mouse();
										if(selected_3d_object!=-1){
											if (selected_3d_object < MAX_OBJ_3D)
											{
												if(calhm)
													clear_e3d_heightmap(selected_3d_object);
													
												if(alt_on){	
													ew_selected_object=selected_3d_object;
													ew_object_type=0;
													memcpy(&o3t,objects_list[ew_selected_object],sizeof(object3d));
													selected_3d_object=-1;
												}
											}
										}
									}
							}
							//2D objects/////////////////
							if(cur_mode==mode_2d)
							{
								if(cur_tool==tool_kill)
									{
										get_2d_object_under_mouse();
										if(selected_2d_object!=-1){
											undo_type = mode_2d;
											if(undo_object == NULL)
												free(undo_object);
											undo_object = (obj_2d *) malloc(sizeof(obj_2d));
											memcpy(undo_object,obj_2d_list[selected_2d_object],sizeof(obj_2d));
											kill_2d_object(selected_2d_object);
										}
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_2d_object_under_mouse();
										if(selected_2d_object!=-1)
				                                            clone_2d_object(selected_2d_object);
										return(done);
									}

								//if we have an object attached to us, drop it
								if(left_click==1 && cur_tool==tool_select && selected_2d_object!=-1)
                                				    clone_2d_object(selected_2d_object);
								else
								{
									if(selected_2d_object==-1){
										get_2d_object_under_mouse();
										if(alt_on && selected_2d_object!=-1){
											ew_selected_object=selected_2d_object;
											ew_object_type=1;
											memcpy(&o2t,obj_2d_list[ew_selected_object],sizeof(obj_2d));
											selected_2d_object=-1;
										}
									}
								}

							}
							//Particle objects/////////////////
							if(cur_mode==mode_particles)
							{
								if(cur_tool==tool_kill)
									{
										get_particles_object_under_mouse();
										if(selected_particles_object!=-1){
											undo_type = mode_particles;
											if(undo_object == NULL)
												free(undo_object);
											undo_object = (particle_sys *) malloc(sizeof(particle_sys));
											memcpy(undo_object,particles_list[selected_particles_object],sizeof(particle_sys));
											kill_particles_object(selected_particles_object);
										}
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_particles_object_under_mouse();
										if(selected_particles_object!=-1)clone_particles_object(selected_particles_object);
										cur_tool=tool_select;
										return(done);
									}

								//if we have an object attached to us, drop it
								if(left_click==1 && cur_tool==tool_select && selected_particles_object!=-1)selected_particles_object=-1;
								else
								{
									if(selected_particles_object==-1){
										get_particles_object_under_mouse();
										if(alt_on && selected_particles_object!=-1){
											selected_particles_object=-1;
										}
									}
								}

							}
							//Lights/////////////////
							if(cur_mode==mode_light)
							{
								if(cur_tool==tool_kill)
									{
										get_light_under_mouse();
										if(selected_light!=-1 && !lights_list[selected_light]->locked){
											undo_type = mode_light;
											if(undo_object == NULL)
												free(undo_object);
											undo_object = (light *) malloc(sizeof(light));
											memcpy(undo_object,lights_list[selected_light],sizeof(light));
											kill_light(selected_light);
										}
										
										return(done);
									}
								if(cur_tool==tool_clone)
									{
										get_light_under_mouse();
										if(selected_light!=-1 && !lights_list[selected_light]->locked)clone_light(selected_light);
										return(done);
									}

								//if we have a light attached to us, drop it
								if(cur_tool==tool_select && selected_light!=-1)selected_light=-1;
								else
								{
									get_light_under_mouse();
								}
							}
							// tiles /////////////////
							if(cur_mode==mode_tile)
							{
								if(cur_tool==tool_kill && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										undo_type = mode_tile;
										undo_tile = (int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3;
										undo_tile_value = tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3];
										tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=255;
										//kill_height_map_at_texture_tile((int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3);
										return(done);
									}

								if(cur_tool==tool_clone  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										selected_tile=tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3];
										cur_tool=tool_select;
										return(done);
									}

								//if we have a tile attached to us, drop it
								if(cur_tool==tool_select && selected_tile!=255  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3 && scene_mouse_x<tile_map_size_x*3)
									{
										tile_map[(int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3]=selected_tile;
										if(selected_tile == 0 || selected_tile == 20 || selected_tile == 21){
										  kill_height_map_at_texture_tile((int)scene_mouse_y/3*tile_map_size_x+(int)scene_mouse_x/3);
										}
									}
							}
							// heights /////////////////
							if(cur_mode==mode_height)
							{
								if(cur_tool==tool_kill  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
									{
										height_map[(int)(scene_mouse_y*2.0f)*tile_map_size_x*6+(int)(scene_mouse_x*2.0f)]=11;
										return(done);
									}

								if(cur_tool==tool_clone && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
									{
										selected_height=height_map[(int)(scene_mouse_y*2.0f)*tile_map_size_x*6+(int)(scene_mouse_x*2.0f)];
										cur_tool=tool_select;
										return(done);
									}

								//if we have a height attached to us, drop it
								if(cur_tool==tool_select && selected_height!=-1  && scene_mouse_y>0 && scene_mouse_x>0 && scene_mouse_y<tile_map_size_y*3*6 && scene_mouse_x<tile_map_size_x*3*6)
								{
									if (alt_on && ctrl_on)
										draw_big_height_tile(5);
									else if(alt_on)
										draw_big_height_tile(3);
									else if(ctrl_on)
										draw_big_height_tile(1);
									else if(shift_on) 
										map_floodfill ();
									else 
										height_map[(int)(scene_mouse_y*2.0f)*tile_map_size_x*6+(int)(scene_mouse_x*2.0f)]=selected_height;
								}
							}
#ifdef EYE_CANDY
							if (cur_mode == mode_eye_candy)
							{
								get_3d_object_under_mouse();
								if (selected_3d_object >= MAX_OBJ_3D)
								{
									select_eye_candy_effect(selected_3d_object);
									selected_3d_object = -1;
								}
							}
#endif
							

						}
						//no left click==1
						else
						  if(cur_mode==mode_3d && cur_tool==tool_select && selected_3d_object!=-1)
                            move_3d_object(selected_3d_object);
						  else if(cur_mode==mode_2d && cur_tool==tool_select && selected_2d_object!=-1)
                            move_2d_object(selected_2d_object);
						  else if(cur_mode==mode_particles && cur_tool==tool_select && !view_particles_window && selected_particles_object!=-1)
                            move_particles_object(selected_particles_object);
						  else if(cur_mode==mode_light && cur_tool==tool_select && selected_light!=-1 && !lights_list[selected_light]->locked)
                            move_light(selected_light);
						  else if(cur_mode==mode_tile && cur_tool==tool_select && selected_tile!=255)
                            move_tile_a_tile=1;
						  else 
                            move_tile_a_tile=0;

						if(cur_mode==mode_height && cur_tool==tool_select && selected_height!=-1)
                            move_tile_a_height=1;
						else 
                            move_tile_a_height=0;
				}
 		   }


		   
			if((left_click>1))
				if(drag_windows(mouse_x, mouse_y, mouse_delta_x, mouse_delta_y) > 0)
					return done;
			
	return(done);
}
