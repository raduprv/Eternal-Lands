public pawn_test (const msg[]) 
{
	new i

	printf "Hello Eternal Lands %s!\n", msg
	for (i = 0; i < 3; i++)
		log_to_console "Hello EL, from Pawn %f!", floatsqroot (float (i))
}

public play_with_object_pos (id, add)
{
	//translate_object_relative (id, 0.0, 0.0, 0.3)
	if (add)
		rotate_object_add (id, 0.0, 12.0, 0.0)
	else
		rotate_object (id, 0.0, 12.0, 0.0)
}

