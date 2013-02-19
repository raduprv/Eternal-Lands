#include <errno.h>
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
#include "eye_candy_wrapper.h"

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
		if (mine_defs[i].id == mine_type && strcasecmp(mine_defs[i].file, ""))
			return mine_defs[i].file;
	}
	LOG_ERROR("Invalid mine type was requested!\n");
	return "";
}

void put_mine_on_ground(int mine_x, int mine_y, int mine_type, int mine_id)
{
	float x, y, z;
	int obj_3d_id;
	
	// Now, get the Z position
	if (!get_tile_valid(mine_x, mine_y))
	{
		//Warn about this error:
		LOG_ERROR("A mine was placed OUTSIDE the map!\n");
		return;
	}
	
	z = get_tile_height(mine_x, mine_y);
	// Convert from height values to meters
	x = (float)mine_x / 2;
	y = (float)mine_y / 2;
	// Center the object
	x = x + 0.25f;
	y = y + 0.25f;

	// Now, find a place into the mines list, so we can destroy the mine properly
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

void add_mines_from_list (const Uint8 *data)
{
	Uint16 mines_no;
	int i;
	int mine_x, mine_y, mine_type, my_offset;
	float x, y, z;
	int obj_3d_id, mine_id;

	mines_no = data[0];

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
		if (mine_id >= NUM_MINES)
		{
			continue;
		}
		// Now, get the Z position
		if (!get_tile_valid(mine_x, mine_y))
		{
			// Warn about this error!
			LOG_ERROR("A mine was located OUTSIDE the map!\n");
			continue;
		}
		
		z = get_tile_height(mine_x, mine_y);
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
	if (which_mine == -1 || which_mine >= NUM_MINES) return;

	if (mine_list[which_mine].obj_3d_id == -1) {
		// Oops, no mine in that slot!
		LOG_ERROR("Oops, double-removal of mine!\n");
		return;
	}

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

	if ((doc = xmlReadFile(file, NULL, 0)) == NULL)
	{
		char str[200];
		safe_snprintf(str, sizeof(str), "%s: %s: %s", mines_config_open_err_str, file, strerror(errno));
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

	xmlFreeDoc(doc);
}

