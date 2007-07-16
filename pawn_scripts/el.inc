native set_object_rotation (id, Float: rot_x, Float: rot_y, Float: rot_z);
native rotate_object (id, Float: rot_x, Float: rot_y, Float: rot_z);
native rotate_object_add (id, Float: rot_x, Float: rot_y, Float: rot_z);

native set_object_position (id, Float: rot_x, Float: rot_y, Float: rot_z);
native translate_object (id, Float: rot_x, Float: rot_y, Float: rot_z);

native add_timer (offset, name[], interval);
native clear_timers ();

#if defined NEW_SOUND
native add_sound_object (sound_type, x, y);
#else
native add_sound_object (sound_type, x, y, positional=true, loops=false);
#endif

native get_position (&x, &y);

native log_to_console (const format[], {Float,Fixed,_}:...);
