#include "platform.h"
#include "load_gl_extensions.h"
#include "actors.h"
#include "gl_init.h"
#include "misc.h"
#include "cursors.h"
#include "draw_scene.h"
#include "interface.h"
#include "hud.h"
#include <SDL.h>
#include <vector>
#include <map>
#include <cassert>
#include "exceptions/extendedexception.hpp"
#ifdef	USE_BOOST
#include <boost/foreach.hpp>
#endif	/* USE_BOOST */

#include "2d_objects.h"
#include "3d_objects.h"
#include "e3d.h"
#include "actor_init.h"

struct SelectionData
{
	Uint32 type;
	Uint32 id;
};

std::vector<SelectionData> selections;

typedef std::map<Uint32, float> IndexMap;

bool read_selection = false;
bool add_selection = false;

const Uint32 select_size = 5;
const Uint32 select_offset = 2;
const float select_scale = 2.0f;
const float select_scale_max = 16.0f;
Uint32 use_new_selection = 1;

static inline void update_color(Uint8 *color, GLfloat *colorf, const Uint32 index, const bool selectable)
{
	Uint32 value;

	value = index & 0x3FFF;

	color[0] = (value / 128) * 2;
	color[1] = (value % 128) * 2;

	if (selectable)
	{
		color[2] = 127;
	}
	else
	{
		color[2] = 0;
	}
	color[3] = 255;

	colorf[0] = color[0] / 255.0f;
	colorf[1] = color[1] / 255.0f;
	colorf[2] = color[2] / 255.0f;
	colorf[3] = color[3] / 255.0f;
}

static inline float selection_type_scale(const Sint32 type)
{
	switch (type)
	{
		case UNDER_MOUSE_NO_CHANGE:
			LOG_ERROR("Invalid selection type!");
			return 0.0f;
		case UNDER_MOUSE_PLAYER:
		case UNDER_MOUSE_NPC:
		case UNDER_MOUSE_ANIMAL:
			return 1.0f;
		case UNDER_MOUSE_3D_OBJ:
			return 1.0f;
		default:
			LOG_ERROR("Invalid selection type!");
			return 0.0f;
	};
}

static inline void update_selection(Uint8 *color)
{
	Uint32 i, j, idx, index;
	float t, count;
	IndexMap indices;
#ifndef	USE_BOOST
	IndexMap::const_iterator it;
#endif	/* USE_BOOST */

	idx = 0;
	for (i = 0; i < select_size; i++)
	{
		for (j = 0; j < select_size; j++)
		{
			if (((color[idx * 4 + 0] % 2) == 0) && ((color[idx * 4 + 1] % 2) == 0) &&
				(color[idx * 4 + 2] == 127))
			{
				index = color[idx * 4 + 0] * 64 + color[idx * 4 + 1] / 2;
				if (index < selections.size())
				{
					t = (select_scale - i) * (select_scale - i);
					t *= (select_scale - j) * (select_scale - j);
					indices[index] += std::max(0.0f, select_scale_max - t) *
						selection_type_scale(selections[index].type);
				}
			}
			idx++;
		}
	}
	index = 0;
	count = 0;
	for (const auto& it: indices)
	{
		if (it.second > count)
		{
			index = it.first;
			count = it.second;
		}
	}
	if (count > 0)
	{
		thing_under_the_mouse = selections[index].type;
		switch (selections[index].type)
		{
			case UNDER_MOUSE_NO_CHANGE:
				LOG_ERROR("Invalid selection type!");
				break;
			case UNDER_MOUSE_3D_OBJ:
				actor_under_mouse = 0;
				object_under_mouse = selections[index].id;
				break;
			case UNDER_MOUSE_PLAYER:
			case UNDER_MOUSE_NPC:
			case UNDER_MOUSE_ANIMAL:
				if (actors_list[selections[index].id])
				{
					actor_under_mouse = actors_list[selections[index].id];
					object_under_mouse = actors_list[selections[index].id]->actor_id;
				}
				break;
			default:
				LOG_ERROR("Invalid selection type!");
				break;
		}
	}
	selections.clear();
}

Uint8 last_pixel_color[4];

static inline void old_reset_under_the_mouse()
{
	if (!read_mouse_now)
	{
		return;
	}
	last_pixel_color[0] = 0;
	last_pixel_color[1] = 0;
	last_pixel_color[2] = 0;
	object_under_mouse = -1;
	thing_under_the_mouse = UNDER_MOUSE_NOTHING;
}

static inline int old_anything_under_the_mouse(int object_id, int object_type)
{
	char pixels[16] = { 0 };

	if (!read_mouse_now)
	{
		return 0;
	}
	glReadBuffer(GL_BACK);
	glReadPixels(mouse_x, window_height - mouse_y, 1, 1, GL_RGB, GL_BYTE, &pixels);
	if ((pixels[0] != last_pixel_color[0]) || (pixels[1] != last_pixel_color[1])
		|| (pixels[2] != last_pixel_color[2]))
	{
		last_pixel_color[0] = pixels[0];
		last_pixel_color[1] = pixels[1];
		last_pixel_color[2] = pixels[2];

		if (object_type == UNDER_MOUSE_NO_CHANGE)
		{
			return 0;
		}

		if ((object_type == UNDER_MOUSE_PLAYER) || (object_type==UNDER_MOUSE_NPC) ||
			(object_type==UNDER_MOUSE_ANIMAL))
		{
			actor_under_mouse = actors_list[object_id];
			object_id = actors_list[object_id]->actor_id;
		}
		else
		{
			actor_under_mouse = NULL;
		}
		object_under_mouse = object_id;

		thing_under_the_mouse = object_type;
		return 1;//there is something
	}
	return 0;//no collision, sorry
}

extern "C" void reset_under_the_mouse()
{
	Uint8 color[4];
	GLfloat colorf[4];
	Uint8 buffer[4 * select_size * select_size];
	Uint32 i;
	Sint32 j;
	Sint32 x, y;

	memset(color, 0, sizeof(color));
	memset(colorf, 0, sizeof(colorf));
	memset(buffer, 0, sizeof(buffer));

	if (use_new_selection)
	{
		read_selection = add_selection;
		add_selection = read_mouse_now;

		if (read_selection)
		{
			object_under_mouse = -1;
			thing_under_the_mouse = UNDER_MOUSE_NOTHING;
			actor_under_mouse = 0;

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			draw_tile_map();
			display_2d_objects();

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glEnableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			ELglClientActiveTextureARB(GL_TEXTURE1);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_Q);
			glDisable(GL_TEXTURE_GEN_R);

			ELglClientActiveTextureARB(GL_TEXTURE0);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_Q);
			glDisable(GL_TEXTURE_GEN_R);
			glReadBuffer(GL_BACK);

			glDisable(GL_FOG);
		  	glDisable(GL_LIGHTING);

			ELglActiveTexture(GL_TEXTURE1);
		  	glDisable(GL_TEXTURE_2D);
			ELglActiveTexture(GL_TEXTURE0);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colorf);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);

			glAlphaFunc(GL_GREATER, 0.06f);

			for (i = 0; i < selections.size(); i++)
			{
				if ((selections[i].type == UNDER_MOUSE_3D_OBJ) && objects_list[selections[i].id])
				{
					update_color(color, colorf, i, true);
					glColor4ubv(color);
					glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colorf);
					for (j = 0; j < objects_list[selections[i].id]->e3d_data->material_no; j++)
					{
						if (objects_list[selections[i].id]->e3d_data->materials[j].options)
						{
							glEnable(GL_ALPHA_TEST);
						  	glEnable(GL_TEXTURE_2D);
							glDisable(GL_CULL_FACE);
							draw_3d_object_detail(objects_list[selections[i].id],
								j, 0, 1, 0);
						}
						else
						{
							glDisable(GL_ALPHA_TEST);
						  	glDisable(GL_TEXTURE_2D);
							glEnable(GL_CULL_FACE);
							draw_3d_object_detail(objects_list[selections[i].id],
								j, 0, 0, 0);
						}
					}
				}
			}

			glDisable(GL_ALPHA_TEST);
		  	glDisable(GL_TEXTURE_2D);

			disable_buffer_arrays();

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			if (use_animation_program)
			{
				set_actor_animation_program(SELECTION_RENDER_PASS, 0);
			}
			for (i = 0; i < selections.size(); i++)
			{
				switch (selections[i].type)
				{
					case UNDER_MOUSE_PLAYER:
					case UNDER_MOUSE_NPC:
					case UNDER_MOUSE_ANIMAL:
						update_color(color, colorf, i, true);
						if (use_animation_program)
						{
							ELglVertexAttrib4Nubv(4, color);
						}
						else
						{
							glColor4ubv(color);
						}
						if (actors_list[selections[i].id])
						{
							if (actors_list[selections[i].id]->has_alpha)
							{
								glEnable(GL_ALPHA_TEST);
							  	glEnable(GL_TEXTURE_2D);
							}
							else
							{
								glDisable(GL_ALPHA_TEST);
							  	glDisable(GL_TEXTURE_2D);
							}
							glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colorf);
#ifdef OSX
							// Fedora's patch to fix NEW_SELECTION on PPC Macs
							glPushAttrib(GL_ALL_ATTRIB_BITS);
							glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
							draw_actor_without_banner(actors_list[selections[i].id], 0, actors_list[selections[i].id]->has_alpha, 0);
							glPopClientAttrib();
							glPopAttrib();
#else
							draw_actor_without_banner(actors_list[selections[i].id], 0, actors_list[selections[i].id]->has_alpha, 0);
#endif
						}
						break;
					case UNDER_MOUSE_3D_OBJ:
						break;
					default:
						LOG_ERROR("Invalid selection type (%d)!", selections[i].type);
						break;
				}
			}
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			if (use_animation_program)
			{
				ELglVertexAttrib4f(4, 1.0f, 1.0f, 1.0f, 1.0f);
				disable_actor_animation_program();
			}
			x = mouse_x - select_offset;
			y = window_height - mouse_y - select_offset;

			glReadPixels(x, y, select_size, select_size, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			update_selection(buffer);

			glPopAttrib();
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			ELglActiveTextureARB(detail_unit);
			glDisable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBindTexture(GL_TEXTURE_2D, 0);

			ELglClientActiveTextureARB(base_unit);
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
			last_texture = -1;
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
	}
	else
	{
		old_reset_under_the_mouse();
	}
}

extern "C" int anything_under_the_mouse(int object_id, int object_type)
{
	SelectionData data;

	if (use_new_selection)
	{
		if (add_selection && (object_type != UNDER_MOUSE_NOTHING) &&
			(object_type != UNDER_MOUSE_NO_CHANGE))
		{
			data.id = object_id;
			data.type = object_type;

			selections.push_back(data);
		}

		return 0;
	}
	else
	{
		return old_anything_under_the_mouse(object_id, object_type);
	}
}
