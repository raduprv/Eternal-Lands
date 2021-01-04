#if defined _el_included
  #endinput
#endif
#define _el_included

#include "client_serv.inc"

native set_object_rotation (id, Float: rot_x, Float: rot_y, Float: rot_z);
native rotate_object (id, Float: rot_x, Float: rot_y, Float: rot_z);
native rotate_object_add (id, Float: rot_x, Float: rot_y, Float: rot_z);

native set_object_position (id, Float: rot_x, Float: rot_y, Float: rot_z);
native translate_object (id, Float: rot_x, Float: rot_y, Float: rot_z);

native add_timer (offset, name[], interval=0);
native clear_timers ();

#if defined NEW_SOUND
native add_sound_object (sound_type, x, y, me=0);
#else
native add_sound_object (sound_type, x, y, positional=true, loops=false);
#endif

native get_position (&x, &y);
native get_actor_from_name (const name[]);
native add_local_actor_command (id, actor_commands: command);

native log_to_console (const format[], {Float,Fixed,_}:...);
