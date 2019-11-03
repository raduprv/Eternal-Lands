#include <stdlib.h>
#include "cursors.h"
#include "elwindows.h"
#include <SDL_mouse.h>
#include <errno.h>
#include "errors.h"
#include "translate.h"
#include "io/elfilewrapper.h"
#ifdef FASTER_MAP_LOAD
#include "io/elpathwrapper.h"
#include "asc.h"
#endif

#define OBJ_NAME_SIZE           80
#define MAX_CURSORS		(CURSOR_TEXT + 1)
#define MAX_HARVESTABLE_OBJECTS 300
#define MAX_ENTRABLE_OBJECTS    300

actor *actor_under_mouse;
int object_under_mouse;
int thing_under_the_mouse;
int current_cursor = -1;
int read_mouse_now=0;
int elwin_mouse=-1;
int cursor_scale_factor = 1;
// max_cursor_scale_factor
// works OK on Linux 1.2.15 SDL up to 4 then flickers for larger values
// does not work above 2 on windows build that is using 1.2.8 SDL, crashing in SDL_SetCursor
// if I can find a solution or a reliable way to limit, I will do so.... pjbroad
int max_cursor_scale_factor = 2;

/*!
 * A cursors_struct contains a Hot Spot and a pointer to the actual cursor.
 */
struct cursors_struct
{
	int hot_x; /*!< x coordinate of the hot spot point. */
	int hot_y; /*!< y coordinate of the hot spot point. */
	Uint8 *cursor_pointer; /*!< pointer to the actual cursor */
};

static struct cursors_struct cursors_array[MAX_CURSORS];

#ifdef FASTER_MAP_LOAD
static char harvestable_objects[MAX_HARVESTABLE_OBJECTS][OBJ_NAME_SIZE];
static int nr_harvestable_objects;
static char entrable_objects[MAX_ENTRABLE_OBJECTS][OBJ_NAME_SIZE];
static int nr_entrable_objects;
#else
char harvestable_objects[300][80];
char entrable_objects[300][80];
#endif

static Uint8 *cursors_mem = NULL;
static int cursors_x_length;
static int cursors_y_length;

#ifdef FASTER_MAP_LOAD
void load_harvestable_list()
{
	FILE *f = NULL;
	char strLine[255];

	memset(harvestable_objects, 0, sizeof(harvestable_objects));
	nr_harvestable_objects = 0;

	f = open_file_data("harvestable.lst", "rb");
	if (f == NULL)
	{
		LOG_ERROR("%s: %s \"harvestable.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while (nr_harvestable_objects < MAX_HARVESTABLE_OBJECTS)
	{
		if (fscanf(f, "%254s", strLine) != 1)
			break;
		my_tolower(strLine);
		my_strncp(harvestable_objects[nr_harvestable_objects], strLine,
			OBJ_NAME_SIZE);
		nr_harvestable_objects++;
		if (!fgets(strLine, sizeof(strLine), f))
			break;

	}
	fclose(f);

	// Sort the list so we can use binary search
	qsort(harvestable_objects, nr_harvestable_objects, OBJ_NAME_SIZE,
		(int(*)(const void*,const void*))strcmp);
}

int is_harvestable(const char* fname)
{
	return bsearch(fname, harvestable_objects, nr_harvestable_objects,
		OBJ_NAME_SIZE, (int(*)(const void*,const void*))strcmp) != NULL;
}

static void cursors_init(void)
{
	size_t i;
	for (i=0; i<MAX_CURSORS; i++)
		cursors_array[i].cursor_pointer = NULL;
	cursors_mem = NULL;
}

void load_entrable_list(void)
{
	FILE *f = NULL;
	char strLine[255];
	int off;

	memset(entrable_objects, 0, sizeof(entrable_objects));
	nr_entrable_objects = 0;
	f=open_file_data("entrable.lst", "rb");
	if(f == NULL)
	{
		LOG_ERROR("%s: %s \"entrable.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while (nr_entrable_objects < MAX_ENTRABLE_OBJECTS)
	{
		if (fscanf(f, "%254s", strLine) != 1)
			break;
		my_tolower(strLine);
		off = *strLine == '/' ? 1 : 0;
		my_strncp(entrable_objects[nr_entrable_objects], strLine+off,
			OBJ_NAME_SIZE);
		nr_entrable_objects++;
		if (!fgets(strLine, sizeof(strLine), f))
			break;
	}
	fclose(f);

	// Sort the list so we can use binary search
	qsort(entrable_objects, nr_entrable_objects, OBJ_NAME_SIZE,
		(int(*)(const void*,const void*))strcmp);
}

int is_entrable(const char* fname)
{
	return bsearch(fname, entrable_objects, nr_entrable_objects,
		OBJ_NAME_SIZE, (int(*)(const void*,const void*))strcmp) != NULL;
}
#endif // FASTER_MAP_LOAD

void load_cursors(void)
{
	int cursors_colors_no, x, y, i;
	Uint8 * cursors_mem_bmp;
	Uint8 cur_color;
	el_file_ptr file;

	file = el_open("textures/cursors.bmp");

	if (file == NULL)
	{
		LOG_ERROR("%s: %s [%s]\n", reg_error_str, cursors_file_str, "textures/cursors.bmp");
		return;
	}

	if ((cursors_mem_bmp = el_get_pointer(file)) == NULL)
	{
		el_close(file);
		LOG_ERROR("%s: %s (read) [%s]\n", reg_error_str, cursors_file_str, "textures/cursors.bmp");
		return;
	}

	cursors_mem_bmp += 18;		//x length is at offset+18
	cursors_x_length = SDL_SwapLE32(*((int *) cursors_mem_bmp));
	cursors_mem_bmp += 4;		//y length is at offset+22
	cursors_y_length = SDL_SwapLE32(*((int *) cursors_mem_bmp));
	cursors_mem_bmp += 46 - 22;
	cursors_colors_no = SDL_SwapLE32(*((int *) cursors_mem_bmp));
	cursors_mem_bmp += 54 - 46 + cursors_colors_no * 4;

	//ok, now transform the bitmap in cursors info
	if(cursors_mem) free(cursors_mem);
	cursors_init();
	cursors_mem = calloc (cursors_x_length*cursors_y_length*2, sizeof(Uint8));

	for(y=cursors_y_length-1;y>=0;y--)
		{
			i=(cursors_y_length-y-1)*cursors_x_length;
			for(x=0;x<cursors_x_length;x++)
				{
					cur_color=*(cursors_mem_bmp+y*cursors_x_length+x);
					switch(cur_color) {
					case 0: //transparent
						*(cursors_mem+(i+x)*2)=0;
						*(cursors_mem+(i+x)*2+1)=0;
						break;
					case 1: //white
						*(cursors_mem+(i+x)*2)=0;
						*(cursors_mem+(i+x)*2+1)=1;
						break;
					case 2: //black
						*(cursors_mem+(i+x)*2)=1;
						*(cursors_mem+(i+x)*2+1)=1;
						break;
					case 3: //reverse
						*(cursors_mem+(i+x)*2)=1;
						*(cursors_mem+(i+x)*2+1)=0;
						break;
					}
				}

		}
	el_close(file);
}

void cursors_cleanup(void)
{
	int i;
	for (i=0; i<MAX_CURSORS; i++)
	{
		if(cursors_array[i].cursor_pointer != NULL)
			SDL_FreeCursor((SDL_Cursor*)cursors_array[i].cursor_pointer);
	}
	if(cursors_mem != NULL) {
		free(cursors_mem);
	}
	cursors_init();
}

static void assign_cursor(size_t cursor_id)
{
	int hot_x,hot_y,x,y,i,cur_color,cur_byte,cur_bit;
	int cursor_bmp_size = 16;
	int cursor_size = cursor_scale_factor * cursor_bmp_size;
	int cursor_mem_size = cursor_size * cursor_size;
	Uint8 cur_mask=0;
	Uint8 *cursor_data = calloc(cursor_mem_size / 8, sizeof(Uint8));
	Uint8 *cursor_mask = calloc(cursor_mem_size / 8, sizeof(Uint8));
	Uint8 *cur_cursor_mem = calloc(cursor_mem_size * 2, sizeof(Uint8));

	i=0;
	for(y=0; y<cursors_y_length; y++)
		{
		for(x=cursor_id * cursor_bmp_size; x<(cursor_id * cursor_bmp_size + cursor_bmp_size); x++)
			{
				int pr, pc;
				cur_color = *(cursors_mem + (y * cursors_x_length + x) * 2);
				for (pr=0; pr<cursor_scale_factor; pr++)
					for (pc=0; pc<cursor_scale_factor; pc++)
						*(cur_cursor_mem + i + pc + pr * cursor_size) = cur_color;//data
				cur_color = *(cursors_mem + (y * cursors_x_length + x) * 2 + 1);
				for (pr=0; pr<cursor_scale_factor; pr++)
					for (pc=0; pc<cursor_scale_factor; pc++)
						*(cur_cursor_mem + i + +cursor_mem_size + pc + pr * cursor_size) = cur_color;//data
				i += cursor_scale_factor;
			}
			i += (cursor_scale_factor -1 ) * cursor_size;
		}
	//ok, now put the data into the bit data and bit mask
	for(i=0; i<cursor_mem_size; i++)
		{
			cur_color=*(cur_cursor_mem+i);
			cur_byte=i/8;
			cur_bit=i%8;
			if(cur_color)//if it is 0, let it alone, no point in setting it
				{
					switch(cur_bit) {
					case 0:
						cur_mask=128;break;
					case 1:
						cur_mask=64;break;
					case 2:
						cur_mask=32;break;
					case 3:
						cur_mask=16;break;
					case 4:
						cur_mask=8;break;
					case 5:
						cur_mask=4;break;
					case 6:
						cur_mask=2;break;
					case 7:
						cur_mask=1;break;
					}
					cursor_data[cur_byte]|=cur_mask;
				}

		}
	for(i=0;i<cursor_mem_size;i++)
		{
			cur_color = *(cur_cursor_mem + i + cursor_mem_size);
			cur_byte=i/8;
			cur_bit=i%8;
			if(cur_color)//if it is 0, let it alone, no point in setting it
				{
					if(cur_bit==0)cur_mask=128;
					else if(cur_bit==1)cur_mask=64;
					else if(cur_bit==2)cur_mask=32;
					else if(cur_bit==3)cur_mask=16;
					else if(cur_bit==4)cur_mask=8;
					else if(cur_bit==5)cur_mask=4;
					else if(cur_bit==6)cur_mask=2;
					else if(cur_bit==7)cur_mask=1;
					cursor_mask[cur_byte]|=cur_mask;
				}
		}

	hot_x=cursors_array[cursor_id].hot_x;
	hot_y=cursors_array[cursor_id].hot_y;
	if (cursors_array[cursor_id].cursor_pointer != NULL)
		SDL_FreeCursor((SDL_Cursor*)cursors_array[cursor_id].cursor_pointer);
	cursors_array[cursor_id].cursor_pointer=(Uint8 *)SDL_CreateCursor(cursor_data, cursor_mask, cursor_size, cursor_size, hot_x, hot_y);
    free(cur_cursor_mem);
    free(cursor_mask);
    free(cursor_data);
}

void change_cursor(int cursor_id)
{
	if ((cursor_id >= 0) && (cursor_id < MAX_CURSORS) && cursors_array[cursor_id].cursor_pointer != NULL)
	{
		SDL_SetCursor((SDL_Cursor*)cursors_array[cursor_id].cursor_pointer);
		current_cursor=cursor_id;
	}
}

void build_cursors(void)
{
	int hot_x[MAX_CURSORS] = {7, 7, 3, 8, 2, 4, 7, 3, 7, 7, 1, 3, 8};
	int hot_y[MAX_CURSORS] = {7, 7, 4, 7, 15, 4, 3, 0, 7, 7, 5, 4, 7};
	size_t i;

	for (i=0; i<MAX_CURSORS; i++)
	{
		cursors_array[i].hot_x = cursor_scale_factor * hot_x[i];
		cursors_array[i].hot_y = cursor_scale_factor * hot_y[i];
		assign_cursor(i);
	}
}
