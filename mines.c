#ifdef MINES

#include <stdlib.h>
#include <string.h>
#include "mines.h"
#include "3d_objects.h"
#include "asc.h"
#include "elwindows.h"
#include "errors.h"
#include "init.h"
#include "translate.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif // NEW_SOUND
#include "tiles.h"
#ifdef EYE_CANDY
#include "eye_candy_wrapper.h"
#endif

#define MAX_MINE_DEFS 30

typedef struct
{
	int id;
	char file[50];
} mine_types;

mine_types mine_defs[MAX_MINE_DEFS];
mine mine_list[NUM_MINES];
int num_mine_defs = 0;

char * get_mine_e3d(mine_type)
{
	int i;
	// Search the array for the required mine type
	for (i = 0; i < num_mine_defs; i++)
	{
		if (mine_defs[i].id == mine_type)
			return mine_defs[i].file;
	}
	LOG_ERROR("Invalid mine type was requested!\n");
	return "";
}

void put_mine_on_ground(int mine_x, int mine_y, int mine_type, int mine_id)
{
	float x, y, z;
	int obj_3d_id;
	
	printf("Adding mine: X: %i, Y: %i, Type: %i, ID: %i\n", mine_x, mine_y, mine_type, mine_id);

	// Now, get the Z position
	if (mine_y * tile_map_size_x * 6 + mine_x > tile_map_size_x * tile_map_size_y * 6 * 6)
	{
		//Warn about this error:
		log_error("A mine was placed OUTSIDE the map!\n");
		return;
	}
	
	z = -2.2f + height_map[mine_y * tile_map_size_x * 6 + mine_x] * 0.2f;
	// Convert from height values to meters
	x = (float)mine_x / 2;
	y = (float)mine_y / 2;
	// Center the object
	x = x + 0.25f;
	y = y + 0.25f;

	obj_3d_id = add_e3d(get_mine_e3d(mine_type), x, y, z, 0, 0, 0, 1, 0, 1.0f, 1.0f, 1.0f, 1);
	
	// Now, find a place into the mines list, so we can destroy the mine properly
	mine_list[mine_id].x = mine_x;
	mine_list[mine_id].y = mine_y;
	mine_list[mine_id].type = mine_type;
	mine_list[mine_id].obj_3d_id = obj_3d_id;
}

void add_mines_from_list (const Uint8 *data)
{
	Uint16 mines_no;
	int i;
	int mine_x, mine_y, mine_type, my_offset;
	float x, y, z;
	int obj_3d_id, mine_id;
	
	mines_no = data[0];
	printf("Adding %d mines.\n", mines_no);

	if (mines_no > NUM_MINES)
	{
		return;		// Something nasty happened
	}
	
	for (i = 0; i < mines_no; i++)
	{
		my_offset = i * 6 + 1;
		mine_x = SDL_SwapLE16(*((Uint16 *)(data + my_offset)));
		mine_y = SDL_SwapLE16(*((Uint16 *)(data + my_offset + 2)));
		mine_id = *((Uint8 *)(data + my_offset + 4));
		mine_type = *((Uint8 *)(data + my_offset + 5));
		printf("Adding mine: X: %i, Y: %i, Type: %i, ID: %i\n", mine_x, mine_y, mine_type, mine_id);
		if (mine_id >= NUM_MINES)
		{
			continue;
		}
		// Now, get the Z position
		if (mine_y * tile_map_size_x * 6 + mine_x > tile_map_size_x * tile_map_size_y * 6 * 6)
		{
			// Warn about this error!
			log_error("A mine was located OUTSIDE the map!\n");
			continue;
		}
		
		z = -2.2f + height_map[mine_y * tile_map_size_x * 6 + mine_x] * 0.2f;
		// Convert from height values to meters
		x = (float)mine_x / 2;
		y = (float)mine_y / 2;
		// Center the object
		x = x + 0.25f;
		y = y + 0.25f;
	
		// Now, find the place into the mines list, so we can destroy the mine properly
		if (mine_list[mine_id].obj_3d_id != -1)
		{
			char buf[256];

			// Oops, slot already taken!
			safe_snprintf(buf, sizeof(buf), "Oops, trying to add an existing mine! id=%d\n", mine_id);
			LOG_ERROR(buf);
			return;
		}

		obj_3d_id = add_e3d(get_mine_e3d(mine_type), x, y, z, 0, 0, 0, 1, 0, 1.0f, 1.0f, 1.0f, 1);
		mine_list[mine_id].x = mine_x;
		mine_list[mine_id].y = mine_y;
		mine_list[mine_id].type = mine_type;
		mine_list[mine_id].obj_3d_id = obj_3d_id;
	}
}

void remove_mine(int which_mine)
{
/*
#ifdef EYE_CANDY
	float x, y, z;
#endif
#ifdef NEW_SOUND
	int snd;
#endif // NEW_SOUND
*/
	
	printf("Removing mine: %i\n", which_mine);

	if (which_mine == -1 || which_mine >= NUM_MINES) return;

	if (mine_list[which_mine].obj_3d_id == -1) {
		// Oops, no mine in that slot!
		LOG_ERROR("Oops, double-removal of mine!\n");
		return;
	}

/* Mine removal effects will happen with the SEND_SPECIAL_EFFECT command if the server deems them nessessary
 * This code can stay here just in-case that needs to be changed.

 #ifdef EYE_CANDY
	x = mine_list[which_mine].x;
	y = mine_list[which_mine].y;
	z = -2.2f + height_map[mine_list[which_mine].y * tile_map_size_x * 6 + mine_list[which_mine].x] * 0.2f;
	printf("X: %f, Y: %f, Type: %i\n", x, y, mine_list[which_mine].type);
	//convert from height values to meters
	x /= 2;
	y /= 2;
	//center the object
	x = x + 0.25f;
	y = y + 0.25f;
	if (use_eye_candy) ec_create_mine_remove(x, y, z, mine_list[which_mine].type, (poor_man ? 6 : 10));
 #else // EYE_CANDY
  #ifdef SFX
	add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
  #endif
 #endif // EYE_CANDY
#ifdef NEW_SOUND
	if (sound_on)
	{
		snd = get_sound_index_for_particle_file_name("./particles/mine_out.part");
		if (snd >= 0)
		{
			add_sound_object (snd, mine_list[which_mine].x, mine_list[which_mine].y, 0);
		}
	}
#endif // NEW_SOUND
*/

	destroy_3d_object(mine_list[which_mine].obj_3d_id);
	mine_list[which_mine].obj_3d_id = -1;
}

void remove_all_mines()
{
	int i;

	for (i = 0; i < NUM_MINES; i++){    // Clear the mines list!
		mine_list[i].obj_3d_id = -1;
	}
}

int parse_mine_defs(xmlNode *node)
{
	xmlNode *def;
	mine_types * mine_def = NULL;
	char content[100] = "";
	int ok = 1;

	for (def = node->children; def; def = def->next)
	{
		if (def->type == XML_ELEMENT_NODE)
		{
			if (xmlStrcasecmp (def->name, (xmlChar*)"mine") == 0)
			{
				if (num_mine_defs >= MAX_MINE_DEFS)
				{
					LOG_ERROR("%s: Maximum mine types reached: %d", mines_config_error, MAX_MINE_DEFS);
					ok = 0;
				}
				mine_def = &mine_defs[num_mine_defs++];
				mine_def->id = get_int_property(def, "id");
				get_string_value(content, sizeof(content), def);				
				safe_strncpy(mine_def->file, content, sizeof(mine_def->file));
			}
			else
			{
				LOG_ERROR("%s: Unknown element found: %s", mines_config_error, def->name);
				ok = 0;
			}
		}
		else if (def->type == XML_ENTITY_REF_NODE)
		{
			ok &= parse_mine_defs (def->children);
		}
	}

	return ok;
}

void load_mines_config()
{
	xmlDoc *doc;
	xmlNode *root = NULL;
	char *file = "mines.xml";

#ifdef	NEW_FILE_IO
	if ((doc = xmlReadFile(file, NULL, 0)) == NULL)
#else	// NEW_FILE_IO
	char path[1024];

#ifndef WINDOWS
	safe_snprintf(path, sizeof(path), "%s/%s", datadir, file);
#else
	safe_snprintf(path, sizeof(path), "%s", file);
#endif // !WINDOWS

	// Can we open the file as xml?
	if ((doc = xmlReadFile(path, NULL, 0)) == NULL)
#endif	// NEW_FILE_IO
	{
		char str[200];
		safe_snprintf(str, sizeof(str), mines_config_open_err_str, file);
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red1,str);
	}
	// Can we find a root element
	else if ((root = xmlDocGetRootElement(doc)) == NULL)
	{
		LOG_ERROR("%s: No XML root element found in '%s'", mines_config_error, file);
	}
	// Is the root the right type?
	else if (xmlStrcmp(root->name, (xmlChar*)"mines"))
	{
		LOG_ERROR("%s: Invalid root element '%s' in '%s'", mines_config_error, root->name, file);
	}
	// We've found our expected root, now parse the children
	else
	{
		parse_mine_defs(root);
	}

	xmlFree(doc);
}

#endif // MINES
