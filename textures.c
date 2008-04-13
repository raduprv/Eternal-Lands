#include <stdlib.h>
#include <string.h>
#ifdef ZLIB
#include <zlib.h>
#endif
#include <SDL_image.h>
#include "textures.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "gl_init.h"
#include "init.h"
#include "lights.h"
#include "load_gl_extensions.h"
#include "map.h"
#include "io/elfilewrapper.h"

#ifdef NEW_LIGHTING
extern GLfloat day_ambient[4];
extern GLfloat day_diffuse[4];
extern GLfloat day_specular[4];
extern GLfloat dawn_ambient[4];
extern GLfloat dawn_diffuse[4];
extern GLfloat dawn_specular[4];
extern GLfloat night_ambient[4];
extern GLfloat night_diffuse[4];
extern GLfloat night_specular[4];

xmlDocPtr metadata_xml_doc;
#endif

#ifdef NEW_CURSOR

//Some textures just can't be compressed (written for custom cursors)
static int compression_enabled = 1;

void enable_compression ()
{
	compression_enabled = 1;
}

void disable_compression ()
{
	compression_enabled = 0;
}

#endif // NEW_CURSOR

__inline__ static void set_texture_filter(texture_filter filter, float anisotropic_filter)
{
	switch (filter)
	{
		case TF_POINT:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case TF_BILINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case TF_TRILINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			break;
		case TF_ANISOTROPIC:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				anisotropic_filter);
			break;
		case TF_ANISOTROPIC_AND_MIPMAPS:
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				anisotropic_filter);
			break;
		default:
			LOG_ERROR("Unsupported texture filter (%d)", filter);
			break;
	}
}

__inline__ static void set_texture_filter_parameter()
{
	if (poor_man)
	{
		set_texture_filter(TF_POINT, 0.0f);
	}
	else
	{
		if (use_mipmaps)
		{
			glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
			if (anisotropic_filter > 1.0f)
			{
				set_texture_filter (TF_ANISOTROPIC_AND_MIPMAPS, anisotropic_filter);
			}
			else
			{
				set_texture_filter(TF_TRILINEAR, 0.0f);
			}
		}		
		else
		{
			if (anisotropic_filter > 1.0f)
			{
				set_texture_filter(TF_ANISOTROPIC, anisotropic_filter);
			}
			else
			{
				set_texture_filter(TF_BILINEAR, 0.0f);
			}
		}
	}
}

#if defined NEW_LIGHTING || defined NIGHT_TEXTURES
void do_night_shift_texture(const char * filename, GLubyte * texture_mem, int x_size, int y_size)
{
	int i;
	float percent_grey, average;
	
	if (night_shift_textures)
	{
		// If nighttime, use a nighttime texture.
		if ((!dungeon) &&
			(strncmp(filename, "./textures", 10)) &&	// Exclude the textures dir, which contains buttons and the like.
			(strncmp(filename, "./maps", 6)))		// Also exclude maps
		{
			if ((game_minute > 230) || (game_minute < 10))
			{
				percent_grey = 0.6f;
			}
			else
			{
				if ((game_minute < 40))
				{
					percent_grey = 0.6f * (1.0f - (game_minute - 10.0f) / 30.0f);
				}
				else
				{
					if (game_minute > 200)
					{
						percent_grey = 0.6f * (game_minute - 200) / 30.0f;
					}
					else
					{
						percent_grey = 0.0f;
					}
				}
			}
			for (i = 0; i < x_size * y_size * 4; i += 4)
			{
				average = texture_mem[i + 0];
				average += texture_mem[i + 1];
				average += texture_mem[i + 2];
				average *= 0.33333333f;
				texture_mem[i + 0] = (Uint8)((texture_mem[i + 0] * (1.0 - percent_grey) + average * percent_grey) * (percent_grey == 0.0f ? 1.0f : 0.7f)); // red
				texture_mem[i + 1] = (Uint8)((texture_mem[i + 1] * (1.0 - percent_grey) + average * percent_grey) * (percent_grey == 0.0f ? 1.0f : 0.85f)); // green
				texture_mem[i + 2] = (Uint8)((texture_mem[i + 2] * (1.0 - percent_grey) + average * percent_grey)); // blue
			}
		}
	}
}
#endif
#ifdef NEW_LIGHTING
void load_material_metadata(const char * file_name)
{
	FILE* file;
	
	file = fopen(file_name, "r");
	if (file == NULL)
	{
		log_error("Fatal: Could not open material metadata file.");
		metadata_xml_doc = NULL;
		return;
	}
	fclose(file);

	metadata_xml_doc = xmlReadFile(file_name, NULL, 0);
	if (metadata_xml_doc == NULL)
	{
		log_error("Fatal: Could not read material metadata file.");
		metadata_xml_doc = NULL;
		return;
	}
}

void set_scene_metadata(const char * file_name)
{
	xmlNode* a_node, *section_node, *attribute_node;
	
	day_ambient[0] = 0.4;
	day_ambient[1] = 0.4;
	day_ambient[2] = 0.45;
	day_diffuse[0] = 2.5;
	day_diffuse[1] = 2.5;
	day_diffuse[2] = 2.5;
	day_specular[0] = 2.7;
	day_specular[1] = 2.5;
	day_specular[2] = 2.3;
	dawn_ambient[0] = 0.45;
	dawn_ambient[1] = 0.3;
	dawn_ambient[2] = 0.2;
	dawn_diffuse[0] = 2.4;
	dawn_diffuse[1] = 1.9;
	dawn_diffuse[2] = 1.3;
	dawn_specular[0] = 2.5;
	dawn_specular[1] = 1.9;
	dawn_specular[2] = 1.2;
	night_ambient[0] = 0.06;
	night_ambient[1] = 0.06;
	night_ambient[2] = 0.08;
	night_diffuse[0] = 0.36;
	night_diffuse[1] = 0.36;
	night_diffuse[2] = 0.45;
	night_specular[0] = 0.36;
	night_specular[1] = 0.36;
	night_specular[2] = 0.45;

	day_ambient[3] = 1.0;
	day_diffuse[3] = 1.0;
	day_specular[3] = 1.0;
	dawn_ambient[3] = 1.0;
	dawn_diffuse[3] = 1.0;
	dawn_specular[3] = 1.0;
	night_ambient[3] = 1.0;
	night_diffuse[3] = 1.0;
	night_specular[3] = 1.0;
	
	if (!metadata_xml_doc)
	  return;
	  
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Scene"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Map"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)file_name))
							{
								break;
							}
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DayAmbient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(day_ambient[0]), &(day_ambient[1]), &(day_ambient[2]), &(day_ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DayDiffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(day_diffuse[0]), &(day_diffuse[1]), &(day_diffuse[2]), &(day_diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DaySpecular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(day_specular[0]), &(day_specular[1]), &(day_specular[2]), &(day_specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DawnAmbient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(dawn_ambient[0]), &(dawn_ambient[1]), &(dawn_ambient[2]), &(dawn_ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DawnDiffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(dawn_diffuse[0]), &(dawn_diffuse[1]), &(dawn_diffuse[2]), &(dawn_diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"DawnSpecular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(dawn_specular[0]), &(dawn_specular[1]), &(dawn_specular[2]), &(dawn_specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"NightAmbient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(night_ambient[0]), &(night_ambient[1]), &(night_ambient[2]), &(night_ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"NightDiffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(night_diffuse[0]), &(night_diffuse[1]), &(night_diffuse[2]), &(night_diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"NightSpecular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(night_specular[0]), &(night_specular[1]), &(night_specular[2]), &(night_specular[3]));
			}
		}
	}
/*
	printf("x Filename: %s\n", file_name);
	printf("x DayAmbient:  %f, %f, %f\n", day_ambient[0], day_ambient[1], day_ambient[2]);
	printf("x DayDiffuse:  %f, %f, %f\n", day_diffuse[0], day_diffuse[1], day_diffuse[2]);
	printf("x DaySpecular: %f, %f, %f\n", day_specular[0], day_specular[1], day_specular[2]);
	printf("x DawnAmbient:  %f, %f, %f\n", dawn_ambient[0], dawn_ambient[1], dawn_ambient[2]);
	printf("x DawnDiffuse:  %f, %f, %f\n", dawn_diffuse[0], dawn_diffuse[1], dawn_diffuse[2]);
	printf("x DawnSpecular: %f, %f, %f\n", dawn_specular[0], dawn_specular[1], dawn_specular[2]);
	printf("x NightAmbient:  %f, %f, %f\n", night_ambient[0], night_ambient[1], night_ambient[2]);
	printf("x NightDiffuse:  %f, %f, %f\n", night_diffuse[0], night_diffuse[1], night_diffuse[2]);
	printf("x NightSpecular: %f, %f, %f\n", night_specular[0], night_specular[1], night_specular[2]);
*/
	build_global_light_table();

	return;
}

texture_struct *set_texture_metadata(const char * file_name, texture_struct *tex)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		tex->ambient[i] = 0.2;
		tex->diffuse[i] = 1.0;
		tex->specular[i] = 0.0;
		tex->emission[i] = 0.0;
	}
	tex->ambient[3] = 1.0;
	tex->diffuse[3] = 1.0;
	tex->specular[3] = 1.0;
	tex->emission[3] = 1.0;
	tex->shininess = 30.0;
	tex->material_set = 0;
	
	if (!metadata_xml_doc)
	  return tex;
	  
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)file_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		tex->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(tex->ambient[0]), &(tex->ambient[1]), &(tex->ambient[2]), &(tex->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(tex->diffuse[0]), &(tex->diffuse[1]), &(tex->diffuse[2]), &(tex->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(tex->specular[0]), &(tex->specular[1]), &(tex->specular[2]), &(tex->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(tex->emission[0]), &(tex->emission[1]), &(tex->emission[2]), &(tex->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(tex->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", file_name);
	printf("x Ambient:  %f, %f, %f\n", tex->ambient[0], tex->ambient[1], tex->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", tex->diffuse[0], tex->diffuse[1], tex->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", tex->specular[0], tex->specular[1], tex->specular[2]);
	printf("x Emission: %f, %f, %f\n", tex->emission[0], tex->emission[1], tex->emission[2]);
	printf("x Shininess: %f\n\n", tex->shininess);
*/
	return tex;
}

object3d *set_model_metadata(object3d * obj)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		obj->ambient[i] = 0.2;
		obj->diffuse[i] = 1.0;
		obj->specular[i] = 0.0;
		obj->emission[i] = 0.0;
	}
	obj->ambient[3] = 1.0;
	obj->diffuse[3] = 1.0;
	obj->specular[3] = 1.0;
	obj->emission[3] = 1.0;
	obj->shininess = 30.0;
	obj->material_set = 0;
	
	if (!metadata_xml_doc)
	  return obj;
	  
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Model"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)obj->file_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		obj->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(obj->ambient[0]), &(obj->ambient[1]), &(obj->ambient[2]), &(obj->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(obj->diffuse[0]), &(obj->diffuse[1]), &(obj->diffuse[2]), &(obj->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(obj->specular[0]), &(obj->specular[1]), &(obj->specular[2]), &(obj->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(obj->emission[0]), &(obj->emission[1]), &(obj->emission[2]), &(obj->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(obj->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", obj->file_name);
	printf("x Ambient:  %f, %f, %f\n", obj->ambient[0], obj->ambient[1], obj->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", obj->diffuse[0], obj->diffuse[1], obj->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", obj->specular[0], obj->specular[1], obj->specular[2]);
	printf("x Emission: %f, %f, %f\n", obj->emission[0], obj->emission[1], obj->emission[2]);
	printf("x Shininess: %f\n\n", obj->shininess);
*/
	return obj;
}

void set_part_metadata(body_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Mesh"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->model_name))
								break;
						}
						else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->skin_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_weapon_metadata(weapon_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Mesh"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->model_name))
								break;
						}
						else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->skin_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_shirt_metadata(shirt_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Mesh"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->model_name))
								break;
						}
						else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->arms_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->torso_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_skin_metadata(skin_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->hands_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->head_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->arms_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->body_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->legs_name))
								break;
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->feet_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_hair_metadata(hair_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->hair_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_boots_metadata(boots_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->boots_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

void set_legs_metadata(legs_part * part)
{
	xmlNode* a_node, *section_node, *attribute_node;
	int i;
	
	for (i = 0; i < 3; i++)
	{
		part->ambient[i] = 0.2;
		part->diffuse[i] = 1.0;
		part->specular[i] = 0.0;
		part->emission[i] = 0.0;
	}
	part->ambient[3] = 1.0;
	part->diffuse[3] = 1.0;
	part->specular[3] = 1.0;
	part->emission[3] = 1.0;
	part->shininess = 30.0;
	part->material_set = 0;
	
	if (!metadata_xml_doc)
	  return;
	
	a_node = xmlDocGetRootElement(metadata_xml_doc)->children;
	for (section_node = a_node; section_node; section_node = section_node->next)
	{
		if (section_node->type==XML_ELEMENT_NODE)
		{
			if (!xmlStrcasecmp(section_node->name,(xmlChar*)"Material"))
			{
				for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
				{
					if (attribute_node->type==XML_ELEMENT_NODE)
					{
						if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Mesh"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->model_name))
								break;
						}
						else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Texture"))
						{
							if (!xmlStrcasecmp(attribute_node->children->content,(xmlChar*)part->legs_name))
								break;
						}
					}
				}
				if (attribute_node)
					break;
			}
		}
	}

	if (section_node)
	{
		part->material_set = 1;
		for (attribute_node = section_node->children; attribute_node; attribute_node = attribute_node->next)
		{
			if (attribute_node->type==XML_ELEMENT_NODE)
			{
				if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Ambient"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->ambient[0]), &(part->ambient[1]), &(part->ambient[2]), &(part->ambient[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Diffuse"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->diffuse[0]), &(part->diffuse[1]), &(part->diffuse[2]), &(part->diffuse[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Specular"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->specular[0]), &(part->specular[1]), &(part->specular[2]), &(part->specular[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Emission"))
					sscanf((char*)attribute_node->children->content, "%f,%f,%f,%f", &(part->emission[0]), &(part->emission[1]), &(part->emission[2]), &(part->emission[3]));
				else if (!xmlStrcasecmp(attribute_node->name,(xmlChar*)"Shininess"))
					sscanf((char*)attribute_node->children->content, "%f", &(part->shininess));
			}
		}
	}
/*
	printf("x Filename: %s\n", part->model_name);
	printf("x Ambient:  %f, %f, %f\n", part->ambient[0], part->ambient[1], part->ambient[2]);
	printf("x Diffuse:  %f, %f, %f\n", part->diffuse[0], part->diffuse[1], part->diffuse[2]);
	printf("x Specular: %f, %f, %f\n", part->specular[0], part->specular[1], part->specular[2]);
	printf("x Emission: %f, %f, %f\n", part->emission[0], part->emission[1], part->emission[2]);
	printf("x Shininess: %f\n\n", part->shininess);
*/
	return;
}

#endif	

texture_struct *load_texture(const char * file_name, texture_struct *tex, Uint8 alpha)
{
	SDL_Surface *texture_surface;
	GLubyte* data;
	int texture_width, texture_height, idx;
	int pixel, temp, r, g, b, a;
	int bpp, i, j, index, x_padding;

	el_file_ptr file;

	file = el_open_custom(file_name);

	if (file == NULL)
	{
		return NULL;
	}

	texture_surface = IMG_Load_RW(SDL_RWFromMem(el_get_pointer(file), el_get_size(file)), 1);

	el_close(file);

	if (texture_surface == 0)
	{
		LOG_ERROR("load_texture() error: [%s]", IMG_GetError());
		return NULL;
	}

	// at this point, theTextureSurface contains some type of pixel data.
	// SDL requires us to lock the surface before using the pixel data:
	SDL_LockSurface(texture_surface);
	tex->has_alpha = 0;
	tex->x_size = texture_surface->w;
	tex->y_size = texture_surface->h;

	texture_width = texture_surface->w;
	texture_height = texture_surface->h;

	x_padding = texture_width % 4;

	if (x_padding)
	{
		x_padding = 4 - x_padding;
	}
	if (texture_width <= x_padding)
	{
		x_padding = 0;
	}

	tex->texture = (GLubyte*) malloc(texture_width * texture_height * 4 * sizeof(GLubyte));
	data = tex->texture;

	idx = 0;
	pixel = 0;
	bpp = texture_surface->format->BytesPerPixel;

	for (i = 0; i < texture_height; i++)
	{
		for (j = 0; j < texture_width; j++)
		{
			if ((texture_surface->format->BitsPerPixel == 8) &&
				(texture_surface->format->palette != 0))
			{
				index = ((Uint8 *)texture_surface->pixels)[idx];
				r = texture_surface->format->palette->colors[index].r;
				g = texture_surface->format->palette->colors[index].g;
				b = texture_surface->format->palette->colors[index].b;
				a = (r + g + b) / 3;
			}
			else
			{
				memcpy(&pixel, &((Uint8 *)texture_surface->pixels)[idx], bpp);
				/* Get Red component */
				temp = pixel & texture_surface->format->Rmask;  /* Isolate red component */
				temp = temp >> texture_surface->format->Rshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Rloss;  /* Expand to a full 8-bit number */
				r = (Uint8)temp;

				/* Get Green component */
				temp = pixel & texture_surface->format->Gmask;  /* Isolate green component */
				temp = temp >> texture_surface->format->Gshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Gloss;  /* Expand to a full 8-bit number */
				g = (Uint8)temp;

				/* Get Blue component */
				temp = pixel & texture_surface->format->Bmask;  /* Isolate blue component */
				temp = temp >> texture_surface->format->Bshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Bloss;  /* Expand to a full 8-bit number */
				b = (Uint8)temp;

				/* Get Alpha component */
				temp = pixel & texture_surface->format->Amask;  /* Isolate alpha component */
				temp = temp >> texture_surface->format->Ashift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Aloss;  /* Expand to a full 8-bit number */
				a = (Uint8)temp;
			}
			if (alpha)
			{
				a = alpha;
			}
			idx += bpp;

			index = (texture_height - i - 1) * texture_width + j;
			data[index * 4 + 0] = r;
			data[index * 4 + 1] = g;
			data[index * 4 + 2] = b;
			data[index * 4 + 3] = a;
		}
		idx += bpp * x_padding;
	}

	SDL_UnlockSurface(texture_surface);
	SDL_FreeSurface(texture_surface);

#if defined NEW_LIGHTING || defined NIGHT_TEXTURES
	do_night_shift_texture(file_name, data, tex->x_size, tex->y_size);
#endif

#ifdef NEW_LIGHTING
	set_texture_metadata(file_name, tex);
#endif

	return tex;
}

// set a default alpha value except where it's black, make that transparent
void	texture_set_alpha(texture_struct *tex, Uint8 alpha, int cutoff)
{
	int	y;
	Uint8 * texture_mem= tex->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;
			if((texture_mem[texture_offset] + texture_mem[texture_offset+1] + texture_mem[texture_offset+2])/3 > cutoff){
				texture_mem[texture_offset+3]= alpha;
			} else {
				texture_mem[texture_offset+3]= 0;
			}
		}
	}
}

//TODO: add alpha mask at location
// warning, textures need to be the same size
// set the alpha on the texture from an alpha mask, results go into the first texture
void	texture_alpha_mask(texture_struct *tex, texture_struct *mask)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= mask->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;
			texture_mem[texture_offset+3]= btexture_mem[texture_offset+3];
		}
	}
}


//TODO: blend overlay at location
// warning, textures need to be the same size
// alpha blend two textures, results go into the first texture
void	texture_alpha_blend(texture_struct *tex, texture_struct *blend)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= blend->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int alpha;
			int texture_offset= (y_offset+x)*4;

			alpha= btexture_mem[texture_offset+3];
			if(alpha > 0){
				int	beta= 255-alpha;

				texture_mem[texture_offset]= (beta*texture_mem[texture_offset] + alpha*btexture_mem[texture_offset])/255;
				texture_mem[texture_offset+1]= (beta*texture_mem[texture_offset+1] + alpha*btexture_mem[texture_offset+1])/255;
				texture_mem[texture_offset+2]= (beta*texture_mem[texture_offset+2] + alpha*btexture_mem[texture_offset+2])/255;
				// calc a new alpha just to be complete
				texture_mem[texture_offset+3]= (texture_mem[texture_offset]+texture_mem[texture_offset+1]+texture_mem[texture_offset+2])/3;
			}
		}
	}
}

//TODO: overlay at location
// warning, textures need to be the same size
// overlay one texture on another is there is any alpha, results go into the first texture
void	texture_overlay(texture_struct *tex, texture_struct *blend)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= blend->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			unsigned char alpha;
			int texture_offset= (y_offset+x)*4;

			alpha= btexture_mem[texture_offset+3];
			if(alpha > 0){
				texture_mem[texture_offset]= texture_mem[texture_offset];
				texture_mem[texture_offset+1]= texture_mem[texture_offset+1];
				texture_mem[texture_offset+2]= texture_mem[texture_offset+2];
				texture_mem[texture_offset+3]= texture_mem[texture_offset+3];
			}
		}
	}
}

// warning, textures need to be the same size
// do three texture masking to combine three textures, the result goes in the first texture
void	texture_mask2(texture_struct *texR, texture_struct *texG, texture_struct *mask)
{
	int	y;
	Uint8 * textureR;
	Uint8 * textureG;
	Uint8 * textureM;

	if(texG == NULL)	return;	// NOP
	if(mask == NULL)	return;	// NOP
	textureR= texR->texture;
	textureG= texG->texture;
	textureM= mask->texture;

	for(y=0; y<texR->y_size; y++)
	{
		int	x;
		int	y_offset= y*texR->x_size;

		for(x=0; x<texR->x_size; x++)
		{
			Uint8 r,g;
			Uint8 * tex;
			int texture_offset= (y_offset+x)*4;

			r= textureM[texture_offset+0];
			g= textureM[texture_offset+1];
			// decide which one to use
			if(r == 0 && g == 0){
				//tex= NULL;	// no copy needed
				continue;
			} else if(r >= g){
				//tex= NULL;	// no copy needed
				continue;
			} else if(g >= r){
				tex= textureG;
			} else {
				//tex= NULL;	// no copy needed
				continue;
			}

			//if(tex){
				textureR[texture_offset]= tex[texture_offset];
				textureR[texture_offset+1]= tex[texture_offset+1];
				textureR[texture_offset+2]= tex[texture_offset+2];
				textureR[texture_offset+3]= tex[texture_offset+3];
			//}
		}
	}
}

// warning, textures need to be the same size
// do three texture masking to combine three textures, the result goes in the first texture
void	texture_mask3(texture_struct *texR, texture_struct *texG, texture_struct *texB, texture_struct *mask)
{
	int	y;
	Uint8 * textureR;
	Uint8 * textureG;
	Uint8 * textureB;
	Uint8 * textureM;

	if(mask == NULL)	return;	// nothing to do
	// watch for a NULL value meaning use the texture before it
	if(texG == NULL)	texG= texR;
	if(texB == NULL)	texB= texG;
	textureR= texR->texture;
	textureG= texG->texture;
	textureB= texB->texture;
	textureM= mask->texture;

	for(y=0; y<texR->y_size; y++)
	{
		int	x;
		int	y_offset= y*texR->x_size;

		for(x=0; x<texR->x_size; x++)
		{
			Uint8 r,b,g;
			Uint8 * tex;
			int texture_offset= (y_offset+x)*4;

			r= textureM[texture_offset+0];
			g= textureM[texture_offset+1];
			b= textureM[texture_offset+2];
			// decide which one to use
			if(r == 0 && g == 0 && b == 0){
				//tex= NULL;	// no copy needed
				continue;
			} else if(r >= g && r >= b){
				//tex= NULL;	// no copy needed
				continue;
			} else if(g >= r && g >= b){
				tex= textureG;
			} else if(b >= r && b >= g){
				tex= textureB;
			} else {
				//tex= NULL;	// no copy needed
				continue;
			}

			//if(tex){
				textureR[texture_offset]= tex[texture_offset];
				textureR[texture_offset+1]= tex[texture_offset+1];
				textureR[texture_offset+2]= tex[texture_offset+2];
				textureR[texture_offset+3]= tex[texture_offset+3];
			//}
		}
	}
}


// get a texture id out of the texture cache
// if null and we haven't failed to load it before, then reload it 
// (means it was previously freed)

int get_texture_id(int i)
{
	int new_texture_id;
	int alpha;
	
	if(!texture_cache[i].texture_id && !texture_cache[i].load_err)
	{
		// we need the alpha to know how to load it
		alpha= texture_cache[i].alpha;
		// our texture was freed, we have to reload it
		if(alpha <= 0) {
			new_texture_id= load_bmp8_color_key(&(texture_cache[i]), alpha);
		} else {
			new_texture_id= load_bmp8_fixed_alpha(&(texture_cache[i]), alpha);
		}
		texture_cache[i].texture_id= new_texture_id;
		if (!new_texture_id)
			texture_cache[i].load_err = 1;
	}
	return texture_cache[i].texture_id;
}

#ifndef	USE_INLINE
void	bind_texture_id(int texture_id)
{
	if(last_texture!=texture_id)
	{
		last_texture=texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
}

int get_and_set_texture_id(int i)
{
	int	texture_id;

#ifdef	DEBUG
	if(i<0||i>TEXTURE_CACHE_MAX) {
		log_error("We tried binding a texture ID of %d\n", i);
		return 0;
	}
#endif	//DEBUG

	// do we need to make a hard load or do we already have it?
	if(!texture_cache[i].texture_id && !texture_cache[i].load_err)
	{
		texture_id= get_texture_id(i);
	} else {
		texture_id= texture_cache[i].texture_id;
	}
	bind_texture_id(texture_id);

#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
                GLfloat temp_ambient[4];
                int j;

		if (lighting_contrast == 0.5)
		{
			glMaterialfv(GL_FRONT, GL_AMBIENT, texture_cache[i].ambient);
		}
		else if (lighting_contrast < 0.5)
		{
			for (j = 0; j < 3; j++)
				temp_ambient[j] = texture_cache[i].ambient[j] * 2 * lighting_contrast;
			temp_ambient[3] = 1.0;
			glMaterialfv(GL_FRONT, GL_AMBIENT, temp_ambient);
		}
		else
		{
			for (j = 0; j < 3; j++)
				temp_ambient[j] = texture_cache[i].ambient[j] + texture_cache[i].diffuse[j] * (lighting_contrast - 0.5) * 2;
			temp_ambient[3] = 1.0;
			glMaterialfv(GL_FRONT, GL_AMBIENT, temp_ambient);
		}

		glMaterialfv(GL_FRONT, GL_DIFFUSE, texture_cache[i].diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, texture_cache[i].specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, texture_cache[i].emission);
		glMaterialf(GL_FRONT, GL_SHININESS, texture_cache[i].shininess);
	}
#endif

	return(texture_id);
}
#endif	//USE_INLINE

int load_alphamap(const char * FileName, Uint8 * texture_mem, int orig_x_size, int orig_y_size)
{
	int x_size, y_size;
	texture_struct	texture;
	texture_struct	ttexture;
	texture_struct	*tex;
	char filename[1024];//Create a buffer...
	char * name;

	/* copy (maybe truncating) FileName into a buffer */
	safe_snprintf(filename, sizeof(filename), "%s", FileName);
	/* find last dot */
	name = strrchr(filename, '.');
	if (name == NULL)
	{
		name = filename;
	}
	/* terminate filename before last dot */
	*name = '\0';

	/* safely add '_alpha.bmp' to the string */
	safe_strcat (filename, "_alpha", sizeof (filename));
	name = strrchr(FileName, '.');
	if (name == NULL)
	{
		name = ".bmp";
	}
	safe_strcat (filename, name, sizeof (filename));

	// check for a file
	if (!el_file_exists(filename))
	{
		return 0;	// no file
	}
	// read in the texture
	tex = load_texture(filename, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;

	if(x_size != orig_x_size || y_size != orig_y_size){
		log_error("The alphamap for %s was not the same size as the original - we didn't load the alphamap...", FileName);
		free(tex->texture);
		return 0;
	}

	// now add this alpha to the original texture
	texture.x_size= x_size;
	texture.y_size= y_size;
	texture.texture= texture_mem;
	texture_alpha_mask(&texture, tex);

	free(tex->texture);
	return 1;
}

//load a bmp texture, in respect to the color key
GLuint load_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha)
{
	int x_size, y_size;
#ifdef NEW_LIGHTING
	int i;
#endif
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;
	GLuint texture;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(tex_cache_entry->file_name, texture_mem, x_size, y_size) && alpha < 0){
#ifdef	NEW_ALPHA
		// no texture alpha found, use the constant
		if(alpha < -1){
			// a specific alpha threshold was mentioned
			texture_set_alpha(tex, 255, -alpha);
		} else {
			texture_set_alpha(tex, 255, 15);
	}
#endif	//NEW_ALPHA
	} else {
		tex->has_alpha++;
	}

    tex_cache_entry->has_alpha = tex->has_alpha;

	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	CHECK_GL_ERRORS();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();

	free(tex->texture);
#ifdef NEW_LIGHTING
	for (i = 0; i < 4; i++)
	{
		tex_cache_entry->ambient[i] = tex->ambient[i];
		tex_cache_entry->diffuse[i] = tex->diffuse[i];
		tex_cache_entry->specular[i] = tex->specular[i];
		tex_cache_entry->emission[i] = tex->emission[i];
	}
	tex_cache_entry->shininess = tex->shininess;
	tex_cache_entry->material_set = tex->material_set;
	
/*
	printf("x Filename: %s\n", tex_cache_entry->file_name);
	printf("x Ambient:  %f, %f, %f, %f\n", tex_cache_entry->ambient[0], tex_cache_entry->ambient[1], tex_cache_entry->ambient[2], tex_cache_entry->ambient[3]);
	printf("x Diffuse:  %f, %f, %f, %f\n", tex_cache_entry->diffuse[0], tex_cache_entry->diffuse[1], tex_cache_entry->diffuse[2], tex_cache_entry->diffuse[3]);
	printf("x Specular: %f, %f, %f, %f\n", tex_cache_entry->specular[0], tex_cache_entry->specular[1], tex_cache_entry->specular[2], tex_cache_entry->specular[3]);
	printf("x Emission: %f, %f, %f, %f\n", tex_cache_entry->emission[0], tex_cache_entry->emission[1], tex_cache_entry->emission[2], tex_cache_entry->emission[3]);
	printf("x Shininess: %f\n\n", tex_cache_entry->shininess);
*/
#endif
	return texture;
}

//load a bmp texture, with the specified global alpha
GLuint load_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a)
{
	int x_size, y_size;
#ifdef NEW_LIGHTING
	int i;
#endif
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;
	GLuint texture;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, a);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	CHECK_GL_ERRORS();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if(have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();

	free(tex->texture);
#ifdef NEW_LIGHTING
	for (i = 0; i < 4; i++)
	{
		tex_cache_entry->ambient[i] = tex->ambient[i];
		tex_cache_entry->diffuse[i] = tex->diffuse[i];
		tex_cache_entry->specular[i] = tex->specular[i];
		tex_cache_entry->emission[i] = tex->emission[i];
	}
	tex_cache_entry->shininess = tex->shininess;
#endif
	return texture;
}

// reload a bmp texture, in respect to the color key
GLuint reload_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha, GLuint texture)
{
	int x_size, y_size;
#ifdef NEW_LIGHTING
	int i;
#endif
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(tex_cache_entry->file_name, texture_mem, x_size, y_size) && alpha < 0){
#ifdef	NEW_ALPHA
		// no texture alpha found, use the constant
		if(alpha < -1){
			// a specific alpha threshold was mentioned
			texture_set_alpha(tex, 255, -alpha);
		} else {
			texture_set_alpha(tex, 255, 15);
	}
#endif	//NEW_ALPHA
	} else {
		tex->has_alpha++;
	}

    tex_cache_entry->has_alpha = tex->has_alpha;

	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	CHECK_GL_ERRORS();
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();

	free(tex->texture);
#ifdef NEW_LIGHTING
	for (i = 0; i < 4; i++)
	{
		tex_cache_entry->ambient[i] = tex->ambient[i];
		tex_cache_entry->diffuse[i] = tex->diffuse[i];
		tex_cache_entry->specular[i] = tex->specular[i];
		tex_cache_entry->emission[i] = tex->emission[i];
	}
	tex_cache_entry->shininess = tex->shininess;
#endif
	return texture;
}

//reload a bmp texture, with the specified global alpha
GLuint reload_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a, GLuint texture)
{
	int x_size, y_size;
#ifdef NEW_LIGHTING
	int i;
#endif
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, a);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	CHECK_GL_ERRORS();
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();

	free(tex->texture);
#ifdef NEW_LIGHTING
	for (i = 0; i < 4; i++)
	{
		tex_cache_entry->ambient[i] = tex->ambient[i];
		tex_cache_entry->diffuse[i] = tex->diffuse[i];
		tex_cache_entry->specular[i] = tex->specular[i];
		tex_cache_entry->emission[i] = tex->emission[i];
	}
	tex_cache_entry->shininess = tex->shininess;
#endif
	return texture;
}


//Tests to see if a texture is already loaded. If it is, return the handle.
//If not, load it, and return the handle
int load_texture_cache (const char * file_name, int alpha)
{
	int slot = load_texture_cache_deferred(file_name, alpha);
	get_and_set_texture_id(slot);
	return slot;
}

int load_texture_cache_deferred (const char * file_name, int alpha)
{
	int i;
	int file_name_length;
	int texture_slot= -1;

	file_name_length=strlen(file_name);

	for(i=0;i<TEXTURE_CACHE_MAX;i++)
	{
		if(texture_cache[i].file_name[0])
		{
			if(!strcasecmp(texture_cache[i].file_name, file_name))
			{
				// already loaded, use existing texture
				return i;
			}
		}
		else
		{
			// remember the first open slot we have
			if(texture_slot < 0)
			{
				texture_slot= i;
			}
		}
	}

	if(texture_slot >= 0 && !texture_cache[texture_slot].file_name[0]) {//we found a place to store it
		safe_snprintf(texture_cache[texture_slot].file_name, sizeof(texture_cache[texture_slot].file_name), "%s", file_name);
		texture_cache[texture_slot].texture_id= 0;
		//if(texture_slot == 0) texture_cache[texture_slot].texture_id= 1;
		texture_cache[texture_slot].alpha= alpha;
        texture_cache[texture_slot].has_alpha = 0;
		return texture_slot;
	} else {	
		log_error("Error: out of texture space\n");
		return 0;	// ERROR!
	}
}


#ifndef MAP_EDITOR2
void copy_bmp8_to_coordinates (texture_struct *tex, Uint8 *texture_space, int x_pos, int y_pos)
{
	int x, y, x_size, y_size;
	Uint8 *texture_mem;

	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	for (y= 0; y<y_size; y++)
	{
		int	y_offset= y*x_size;
		int texture_y= (255 - ((y_size-y-1) + y_pos))*256+x_pos;

		for (x = 0; x < x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;

			texture_space[(texture_y+x)*4] = texture_mem[texture_offset];
			texture_space[(texture_y+x)*4+1] = texture_mem[texture_offset+1];
			texture_space[(texture_y+x)*4+2] = texture_mem[texture_offset+2];
			texture_space[(texture_y+x)*4+3] = texture_mem[texture_offset+3];
		}
	}
}

texture_struct *load_bmp8_alpha (const char *filename, texture_struct *tex, Uint8 alpha)
{
	int x_size, y_size;
	Uint8 *texture_mem;

	tex = load_texture(filename, tex, alpha);
	if(!tex){	// oops, failed
		return NULL;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(filename, texture_mem, x_size, y_size)){
		// no texture alpha found found, use the constant
		texture_set_alpha(tex, alpha, -1);
	} else {
		tex->has_alpha++;
	}

	return(tex);
}

int load_bmp8_to_coordinates (const char *filename, Uint8 *texture_space, int x_pos, int y_pos, Uint8 alpha)
{
	texture_struct	texture;
	texture_struct	*tex;

	tex= load_bmp8_alpha(filename, &texture, alpha);
	if(!tex || !tex->texture){	// oops, failed
		return 0;
	}
	copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);

	// release the temporary memory
	free(tex->texture);
	return(tex->has_alpha);
}

int load_bmp8_to_coordinates_mask2 (const char *filename, const char *basename, const char *maskname, Uint8 *texture_space, int x_pos, int y_pos, Uint8 alpha)
{
	//int x_size, y_size;
	texture_struct	texture;
	texture_struct	basetex;
	texture_struct	masktex;
	texture_struct	*tex;
	texture_struct	*base;
	texture_struct	*mask;

	// first, watch for being able to do simplified processing
	// no masking, or both textures are the same file
	if(!basename || !maskname || !*basename || !*maskname || !strcmp(filename, basename)){
		// yes, either the mask or the base is missing, just do a load
		return load_bmp8_to_coordinates(filename, texture_space, x_pos, y_pos, alpha);
	}
//log_error("%s %s %s", filename, basename, maskname);
	tex= load_bmp8_alpha(filename, &texture, alpha);
	if(!tex){	// oops, failed
		return 0;
	}
	//x_size= tex->x_size;
	//y_size= tex->y_size;

	// now load the base in
	base= load_bmp8_alpha(basename, &basetex, alpha);
	if(!base){	// oops, failed
		// just place what we have and free the memory
		copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);
		free(tex->texture);
		return(tex->has_alpha);
	}

	// now load the mask in
	mask= load_bmp8_alpha(maskname, &masktex, alpha);
	if(!mask){	// oops, failed
		// just place what we have and free the memory
		copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);
		free(tex->texture);
		free(base->texture);
		return(tex->has_alpha);
	}

	// lets combine them all together into one
	texture_mask2(tex, base, mask);
	tex->has_alpha+= base->has_alpha;

	// and copy it where we need it
	copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);

	// release the temporary memory
	free(tex->texture);
	free(base->texture);
	free(mask->texture);

	return(tex->has_alpha);
}


#ifdef	ELC
int load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a)
{
	GLuint texture;
	Uint8 * texture_mem;
	int	has_alpha= 0;

	texture_mem=(Uint8*)calloc(1,256*256*4);
	if(this_actor->pants_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->pants_tex,this_actor->legs_base,this_actor->pants_mask,texture_mem,78,175,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->pants_tex,texture_mem,78,175,a);
#endif	//MASKING
	}
	if(this_actor->boots_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->boots_tex,this_actor->boots_base,this_actor->boots_mask,texture_mem,0,175,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->boots_tex,texture_mem,0,175,a);
#endif	//MASKING
	}
#ifdef NEW_TEX
	if(this_actor->torso_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->torso_tex,this_actor->body_base, this_actor->torso_mask, texture_mem,158,149,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->torso_tex,texture_mem,158,149,a);
#endif	//MASKING
	}
#else
	if(this_actor->torso_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->torso_tex,this_actor->torso_base, this_actor->torso_mask, texture_mem,158,156,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->torso_tex,texture_mem,158,156,a);
#endif	//MASKING
	}
#endif
	if(this_actor->arms_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->arms_tex,this_actor->arms_base,this_actor->arms_mask,texture_mem,0,96,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->arms_tex,texture_mem,0,96,a);
#endif	//MASKING
	}
	if(this_actor->hands_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->hands_tex,this_actor->hands_tex_save,this_actor->hands_mask,texture_mem,67,64,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->hands_tex,texture_mem,67,64,a);
#endif	//MASKING
	}
	if(this_actor->head_tex[0]){
#ifdef	MASKING
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->head_tex,this_actor->head_base,this_actor->head_mask,texture_mem,67,0,a);
#else	//MASKING
		has_alpha+= load_bmp8_to_coordinates(this_actor->head_tex,texture_mem,67,0,a);
#endif	//MASKING
	}
	if(this_actor->hair_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->hair_tex,texture_mem,0,0,a);
#ifdef NEW_TEX
	if(this_actor->weapon_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,178,77,a);
	if(this_actor->shield_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,100,77,a);
#else
	if(this_actor->weapon_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,158,77,a);
	if(this_actor->shield_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,80,96,a);
#endif
	if(this_actor->helmet_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->helmet_tex,texture_mem,80,149,a);
	if(this_actor->cape_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->cape_tex,texture_mem,131,0,a);

	this_actor->has_alpha= has_alpha;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();
	free(texture_mem);
	return texture;
}
#endif	//ELC
#endif  //MAP_EDITOR2

#ifdef MAP_EDITOR2
/////////////////////////////////////////////////////////////////////////////////////
//load a bmp file, convert it to the rgba format, but don't assign it to any texture object
//// TODO: remove this and use the generic texure routines above
char * load_bmp8_color_key_no_texture_img(char * filename, img_struct * img)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	Uint8 *texture_mem;
#ifdef	ZLIB
	gzFile *f = NULL;
	if(!gzfile_exists(filename))	return 0;	// no file at all
	f= my_gzopen(filename, "rb");
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
	{
		free(file_mem_start);
		fclose (f);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}
	file_mem+=16;

	colors_no=*((int *)file_mem);
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
#ifdef	ZLIB
	gzread(f, file_mem, colors_no*4+4);//header only
#else	//ZLIB
	fread(file_mem, 1, colors_no*4+4, f);//header only
#endif	//ZLIB
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));

	for(y=0;y<y_size;y++)
	{
#ifdef	ZLIB
		gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB

		for(x=0;x<x_size;x++)
		{
			current_pallete_entry=*(read_buffer+x);
			b=*(color_pallete+current_pallete_entry*4);
			g=*(color_pallete+current_pallete_entry*4+1);
			r=*(color_pallete+current_pallete_entry*4+2);
			*(texture_mem+(y*x_size+x)*4)=r;
			*(texture_mem+(y*x_size+x)*4+1)=g;
			*(texture_mem+(y*x_size+x)*4+2)=b;
			a=(r+b+g)/3;
			*(texture_mem+(y*x_size+x)*4+3)=a;
		}

	}
	
	if(img)
	{
		img->x=x_size;
		img->y=y_size;
	}
	
	free(file_mem_start);
	free(read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
	return texture_mem;
}
#endif
