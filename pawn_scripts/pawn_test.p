public pawn_test ()
{
	new i

	printf "Hello Eternal Lands!\n"
	for (i = 0; i < 3; i++)
		log_to_console "Hello EL, from Pawn %f!", floatsqroot (float (i))
}

public play_with_object_pos (id)
{
	translate_object_relative (id, 0.0, 0.0, 0.3)
	rotate_object_relative (id, 12.0, 0.0, 0.0)
}
