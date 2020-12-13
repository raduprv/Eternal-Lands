#include "default.inc"
#include "maps.p"

public pawn_test (const msg[]) 
{
	new x, y, i

	get_position (x, y)
	printf "Hello Eternal Lands @ %d, %d!\n", x, y
	for (i = 0; i < 3; i++)
		log_to_console "%s %f!", msg, floatsqroot (float (i))

	add_sound_object (1, x+3, y+3);
	add_sound_object (2, x-3, y-3);
}

public play_with_object_pos (id, add)
{
	//translate_object_relative (id, 0.0, 0.0, 0.3)
	if (add)
		rotate_object_add (id, 0.0, 12.0, 0.0)
	else
		rotate_object (id, 0.0, 12.0, 0.0)
}

