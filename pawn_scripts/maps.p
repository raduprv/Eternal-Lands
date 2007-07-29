public turn_derzelas ()
{
	new x, y
	 
	get_position (x, y)

	if (350 <= x <= 364 && 308 <= y <= 325)
	{
		// we're in the right room
		new id = get_actor_from_name ("Derzelas")
		if (id >= 0)
		{
			add_local_actor_command (id, turn_left)
			add_local_actor_command (id, turn_left)
			add_local_actor_command (id, turn_left)
		}
	}
}

initialize_irsis_insides ()
{
	// If we get in the room with the potion seller Derzelas, we want him 
	// to towards us. We have the distinct problem however, that at this
	// point we don't have any actors, not even ourselves, and so
	// a) we don't now where on the map we are
	// b) we don't know the ID of Derzelas
	// So we schedule a function to run in two seconds from now, and hope
	// that that's enough time for our actors to arrive
	add_timer (2000, "turn_derzelas")
}

public turn_tree ()
{
	rotate_object (1129, 0.0, 0.0, 6.0)
}

initialize_isla_prima ()
{
	add_timer (2000, "turn_tree", 2000)
}

get_basename (dest[], size, const fname[])
{
	new i, start, end
	
	start = 0
	end = strlen (fname)
	for (i = end - 1; i >= 0; i--)
	{
		if (fname[i] == '.')
		{
			end = i
		}
		else if (fname[i] == '/')
		{
			start = i+1
			break
		}
	}
	
	strmid (dest, fname, start, end, size)
}

public change_map (const fname[])
{
	new map_name[256]
	
	get_basename (map_name, sizeof map_name, fname)
	clear_timers ()
	if (strequal (map_name, "startmap"))
		initialize_isla_prima ()
	else if (strequal (map_name, "cont2map7_insides"))
		initialize_irsis_insides ()
}
