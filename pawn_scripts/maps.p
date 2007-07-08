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
}
