#ifdef PAWN

#include <string.h>
#include <math.h>
#include "../actors.h"
#include "../actor_scripts.h"
#include "../asc.h"
#include "../bbox_tree.h"
#include "../client_serv.h"
#include "../e3d.h"
#include "../eye_candy_wrapper.h"
#include "../font.h"
#include "../platform.h"
#include "../sound.h"
#include "../text.h"
#include "amxel.h"
#include "amxcons.h"
#include "elpawn.h"

#define MAX_LOG_MSG_SIZE 256

void update_object_pos_and_rot (object3d* obj)
{
	e3d_object* e3d = obj->e3d_data;
// 	unsigned int ground;
	
	calc_rotation_and_translation_matrix (obj->matrix, obj->x_pos, obj->y_pos, obj->z_pos, obj->x_rot, obj->y_rot, obj->z_rot);
	
// 	ground = !has_normal (e3d->vertex_options);

	// FIXME: TODO: update the bounding boxes for this object. Will probably
	// require a new type of bbox storage.

	ec_remove_obstruction_by_object3d (obj);
	ec_add_object_obstruction (obj, e3d, 2.0);
}

static cell AMX_NATIVE_CALL n_set_object_rotation (AMX *amx, const cell *params)
{
	int id = (int) params[1];
	object3d *obj;
	
	if (id < 0 || id >= MAX_OBJ_3D || objects_list[id] == NULL)
		// invalid object ID
		return 1;
	
	obj = objects_list[id];
	
	obj->x_rot = *((REAL*)(params+2));
	obj->y_rot = *((REAL*)(params+3));
	obj->z_rot = *((REAL*)(params+4));

	update_object_pos_and_rot (obj);

	return 0;
}

static cell AMX_NATIVE_CALL n_rotate_object (AMX *amx, const cell *params)
{
	int id = (int) params[1];
	object3d *obj;
	MATRIX4x4D matrix;

	if (id < 0 || id >= MAX_OBJ_3D || objects_list[id] == NULL)
		// invalid object ID
		return 1;
	
	obj = objects_list[id];
	
	// Trying to compute the new angles directly seems pretty horrible,
	// so we'll do this the lazy way, and compute the angles from the
	// rotation matrix
	glPushMatrix ();
	glLoadIdentity ();
	glRotatef (*((REAL*)params+4), 0.0f, 0.0f, 1.0f);
	glRotatef (*((REAL*)params+2), 1.0f, 0.0f, 0.0f);
	glRotatef (*((REAL*)params+3), 0.0f, 1.0f, 0.0f);
	glRotatef (obj->z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef (obj->x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef (obj->y_rot, 0.0f, 1.0f, 0.0f);
	glGetDoublev (GL_MODELVIEW_MATRIX, matrix);
	glPopMatrix ();
	
	obj->x_rot = 180 * asin (matrix[6]) / M_PI;
	obj->y_rot = 180 * atan2 (-matrix[2], matrix[10]) / M_PI;
	obj->z_rot = 180 * atan2 (-matrix[4], matrix[5]) / M_PI;

	update_object_pos_and_rot (obj);

	return 0;
}


static cell AMX_NATIVE_CALL n_rotate_object_add (AMX *amx, const cell *params)
{
	int id = (int) params[1];
	object3d *obj;

	if (id < 0 || id >= MAX_OBJ_3D || objects_list[id] == NULL)
		// invalid object ID
		return 1;
	
	obj = objects_list[id];
	
	obj->x_rot += *((REAL*)(params+2));
	if (obj->x_rot < -180.0)
		obj->x_rot += 360.0;
	else if (obj->x_rot > 180.0)
		obj->x_rot -= 360.0;
	obj->y_rot += *((REAL*)(params+3));
	if (obj->y_rot < -180.0)
		obj->y_rot += 360.0;
	else if (obj->y_rot > 180.0)
		obj->y_rot -= 360.0;
	obj->z_rot += *((REAL*)(params+4));
	if (obj->z_rot < -180.0)
		obj->z_rot += 360.0;
	else if (obj->z_rot > 180.0)
		obj->z_rot -= 360.0;

	update_object_pos_and_rot (obj);

	return 0;
}

static cell AMX_NATIVE_CALL n_set_object_position (AMX *amx, const cell *params)
{
	int id = (int) params[1];
	object3d *obj;
	
	if (id < 0 || id >= MAX_OBJ_3D || objects_list[id] == NULL)
		// invalid object ID
		return 1;
	
	obj = objects_list[id];
	
	obj->x_pos = *((REAL*)(params+2));
	obj->y_pos = *((REAL*)(params+3));
	obj->z_pos = *((REAL*)(params+4));

	update_object_pos_and_rot (obj);

	return 0;
}

static cell AMX_NATIVE_CALL n_translate_object (AMX *amx, const cell *params)
{
	int id = (int) params[1];
	object3d *obj;
	
	if (id < 0 || id >= MAX_OBJ_3D || objects_list[id] == NULL)
		// invalid object ID
		return 1;
	
	obj = objects_list[id];
	
	obj->x_pos += *((REAL*)(params+2));
	obj->y_pos += *((REAL*)(params+3));
	obj->z_pos += *((REAL*)(params+4));

	update_object_pos_and_rot (obj);

	return 0;
}

static cell AMX_NATIVE_CALL n_add_sound_object (AMX *amx, const cell *params)
{
#ifdef NEW_SOUND
	add_sound_object (params[1], params[2], params[3], 0);
#else
	add_sound_object (params[1], params[2], params[3], params[4], params[5]);
#endif
	
	return 0;
}

static cell AMX_NATIVE_CALL n_add_timer (AMX *amx, const cell *params)
{
	char name[256];
	cell *pstr;
	
	amx_GetAddr (amx, params[2], &pstr);
	amx_GetString (name, pstr, 0, sizeof (name));

	add_map_timer (params[1], name, params[3]);
	
	return 0;
}

static cell AMX_NATIVE_CALL n_clear_timers (AMX *amx, const cell *params)
{
	clear_map_timers ();
	return 0;
}

static cell AMX_NATIVE_CALL n_get_position (AMX *amx, const cell *params)
{
	cell *x_addr, *y_addr;
	actor *me = get_our_actor ();
	
	if (!me)
		// Uh oh, we don't exist!
		return 1;
	
	amx_GetAddr (amx, params[1], &x_addr);
	amx_GetAddr (amx, params[2], &y_addr);
	
	*x_addr = (cell) me->x_tile_pos;
	*y_addr = (cell) me->y_tile_pos;
	
	return 0;
}

static cell AMX_NATIVE_CALL n_get_actor_from_name (AMX *amx, const cell* params)
{
	char name[256];
	cell *pstr;
	int i, size, max_size;

	amx_GetAddr (amx, params[1], &pstr);
	amx_GetString (name, pstr, 0, sizeof (name));

	max_size = sizeof (((actor *)NULL)->actor_name);
	size = strlen (name);
	if (size > max_size)
		size = max_size;

	LOCK_ACTORS_LISTS ();
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] 
		    && strncasecmp (actors_list[i]->actor_name, name, size) == 0 
		    && (size == max_size || !is_printable (actors_list[i]->actor_name[size]))
		   )
			break;
	}
	if (i >= max_actors)
		i = -1;
	UNLOCK_ACTORS_LISTS ();

	return (cell) i;
}

static cell AMX_NATIVE_CALL n_add_local_actor_command (AMX *amx, const cell* params)
{
	int idx = params[1];
	unsigned char cmd = params[2];

	if (idx < 0 || idx >= max_actors || !actors_list[idx])
		return -1;

	add_command_to_actor (actors_list[idx]->actor_id, cmd);
	return 0;
}

static int format_append_char (void *dest, char ch)
{
	char* str = dest;
	size_t len = strlen (dest);
	if (len + 1 < MAX_LOG_MSG_SIZE)
	{
		str[len++] = ch; 
		str[len] = '\0';
	}
	return 0;
}

static int format_append_string (void *dest, const char* src)
{
	char* str = dest;
	size_t len = strlen (dest);
	if (len + strlen (src) < MAX_LOG_MSG_SIZE)
	{
		strcpy (str+len, src);
	}
	else if (len + 1 < MAX_LOG_MSG_SIZE)
	{
		// we have space for at least one more character
		strncpy (str+len, src, MAX_LOG_MSG_SIZE-len-1);
		str[MAX_LOG_MSG_SIZE-1] = '\0';
	}
	return 0;
}

const char* format_log_message (AMX *amx, const cell fmt, const cell* params, int nr_params)
{
	static char msg[MAX_LOG_MSG_SIZE];
	AMX_FMTINFO info;
	cell *cstr;
 
	msg[0] = '\0';
	
	memset (&info, 0, sizeof (info));
	info.params = params;
	info.numparams = nr_params;
	info.skip = 0;
	info.length = MAX_LOG_MSG_SIZE; 
	info.f_putstr = format_append_string;
	info.f_putchar = format_append_char;
	info.user = msg;

	amx_GetAddr (amx, fmt, &cstr);
	amx_printstring (amx, cstr, &info);
	
	return msg;
}

static cell AMX_NATIVE_CALL n_log_to_console (AMX *amx, const cell *params)
{
	int nr_params = params[0] / sizeof (cell) - 1;
	const char* msg = format_log_message (amx, params[1], params+2, nr_params);

	LOG_TO_CONSOLE (c_red1, msg);

	return 0;
}

#ifdef __cplusplus
extern "C"
#endif

const AMX_NATIVE_INFO el_Natives[] = {
	{ "log_to_console",          n_log_to_console          },
	/* object position manipulation */
	{ "set_object_rotation",     n_set_object_rotation     },
	{ "rotate_object",           n_rotate_object           },
	{ "rotate_object_add",       n_rotate_object_add       },
	{ "set_object_position",     n_set_object_position     },
	{ "translate_object",        n_translate_object        },
	/* playing sounds */
	{ "add_sound_object",        n_add_sound_object        },
	/* scheduling of functions calls */
	{ "add_timer",               n_add_timer               },
	{ "clear_timers",            n_clear_timers            },
	/* actor information */
	{ "get_position",            n_get_position            },
	{ "get_actor_from_name",     n_get_actor_from_name     },
	{ "add_local_actor_command", n_add_local_actor_command },
	/* terminator */
 	{ NULL,                      NULL                      }
};

int AMXEXPORT amx_ElInit (AMX *amx)
{
	return amx_Register (amx, el_Natives, -1);
}

int AMXEXPORT amx_ElCleanup (AMX *amx)
{
	(void) amx;
	return AMX_ERR_NONE;
}

#endif // PAWN
