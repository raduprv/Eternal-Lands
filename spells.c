#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spells.h"
#include "asc.h"
#include "cursors.h"
#include "context_menu.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "stats.h"
#include "colors.h"
#include "multiplayer.h"
#include "pathfinder.h"
#include "textures.h"
#include "translate.h"
#include "counters.h"
#include "errors.h"
#include "io/elpathwrapper.h"
#include "sound.h"

#define SIGILS_NO 64
#define	NUM_SIGILS_LINE	12	// how many sigils per line displayed
#define	NUM_SIGILS_ROW	3	// how many rows of sigils are there?
#define SIGILS_NO 64
#define SPELLS_NO 32
#define GROUPS_NO 8

#define UNCASTABLE_REAGENTS 1
#define UNCASTABLE_SIGILS 2
#define UNCASTABLE_MANA 4
#define UNCASTABLE_LVLS 8

#define UNCASTABLE_SIGILS_STR "(missing sigils)"
#define UNCASTABLE_REAGENTS_STR "(not enough reagents)"
#define UNCASTABLE_MANA_STR "(not enough mana)"
#define UNCASTABLE_LVLS_STR "(not enough levels)"

#define GET_UNCASTABLE_STR(cast) (  (cast&UNCASTABLE_SIGILS) ? (UNCASTABLE_SIGILS_STR):( (cast&UNCASTABLE_LVLS) ? (UNCASTABLE_LVLS_STR):(  (cast&UNCASTABLE_MANA) ? (UNCASTABLE_MANA_STR):( (cast&UNCASTABLE_REAGENTS) ? (UNCASTABLE_REAGENTS_STR):("")   )          )        )            )

#define SPELLS_ALIGN_X 7
#define MAX(a,b) (((a)>(b))?(a):(b))

#define SET_COLOR(x) glColor4f((float) colors_list[x].r1 / 255.0f,(float) colors_list[x].g1 / 255.0f,(float) colors_list[x].b1 / 255.0f,1.0f)
typedef struct
{
	int sigil_img;
	char name[32];
	char description[64];
	int have_sigil;
}sigil_def;

sigil_def sigils_list[SIGILS_NO];
int sigils_text;
int sigils_we_have;


typedef struct {
	int id;//The spell server id
	char name[60];//The spell name
	char desc[120];//The spell description
	int image;//image_id
	int sigils[6];//index of required sigils in sigils_list
	int mana;//required mana
	attrib_16 *lvls[NUM_WATCH_STAT];//pointers to your_info lvls
	int lvls_req[NUM_WATCH_STAT];//minimum lvls requirement
	int reagents_id[4]; //reagents needed image id
	Uint16 reagents_uid[4]; //reagents needed, unique item id
	int reagents_qt[4]; //their quantities
	Uint32 duration;
	Uint32 buff;
	int uncastable; //0 if castable, otherwise if something missing
} spell_info;

spell_info spells_list[SPELLS_NO];
int num_spells=0;
Uint8 spell_text[256];
unsigned char spell_help[256];
Sint8 on_cast[6];
Uint8 last_spell_str[20];
int last_spell_len= 0;
int spell_result=0;
int have_error_message=0;
int we_have_spell=-1; //selected spell
int on_spell=-1;//mouse over this spell
int show_poison_count = 0; // elconfig variable
static int poison_drop_counter = 0;

typedef struct {
	unsigned char desc[120];
	int spells;
	int spells_id[SPELLS_NO];
	int x,y;
} group_def;
int num_groups=0;
group_def groups_list[GROUPS_NO];

typedef struct
{
	Sint8 spell;
	Uint32 cast_time;
	Uint32 duration;
#ifdef NEW_SOUND
	unsigned int sound;
#endif
} spell_def;
spell_def active_spells[NUM_ACTIVE_SPELLS];

//windows related
int sigil_win=-1; //this is referred externally so we will change it when we switch windows
int sigils_win=-1;
int spell_win=-1;
int spell_mini_win=-1;
int last_win=-1;
int start_mini_spells=0; //do we start minimized?
int init_ok=0;
int sigil_menu_x=10;
int sigil_menu_y=20;
//big window
int spell_x_len=0;
int spell_y_len=0;
int spell_y_len_ext=0;
//sigil window
int sigil_x_len=NUM_SIGILS_LINE*33+20;
int sigil_y_len=(3+NUM_SIGILS_ROW)*33;
//mini window
int spell_mini_x_len=0;
int spell_mini_y_len=0;
int spell_mini_rows=0;

typedef struct {
	char spell_name[60];//The spell_name
	Sint8 spell_image;//image_id
	Sint8 spell_id;
	Uint8 spell_str[30];
	//to be difficult, we will store the entire string ready
	//to be sent to the server, including CAST_SPELL and len bytes, len will be byte 2
} mqbdata;

//QUICKSPELLS
int clear_mouseover=0;
int cast_mouseover=0;
mqbdata * mqb_data[MAX_QUICKBAR_SLOTS+1]={NULL};//mqb_data will hold the magic quickbar name, image, pos.
int quickspell_size=20;//size of displayed icons in pixels
int quickspell_x_len=26;
int quickspell_x=60;
int quickspell_y=64;
int quickspells_loaded = 0;


int cast_handler();
int prepare_for_cast();
void draw_spell_icon(int id,int x_start, int y_start, int gridsize, int alpha, int grayed);
void set_spell_help_text(int spell);
void init_sigils();

size_t cm_quickspells_id = CM_INIT_VALUE;
void cm_update_quickspells(void);


void repeat_spell(){
	if(last_spell_len > 0)
		my_tcp_send(my_socket, last_spell_str, last_spell_len);
}

//returns a node with tagname, starts searching from the_node
xmlNode *get_XML_node(xmlNode *the_node, char *tagname){
	xmlNode *node=the_node;

	while(node) {
		if(node->type==XML_ELEMENT_NODE && xmlStrcasecmp (node->name, (xmlChar*)tagname) == 0) return node;
		else node=node->next;
	}
	return node;	
}


attrib_16 *get_skill_address(const char *skillname)
{
	if(strcmp(skillname,(char*)attributes.manufacturing_skill.shortname)==0) return &your_info.manufacturing_skill;
	if(strcmp(skillname,(char*)attributes.alchemy_skill.shortname)==0) return &your_info.alchemy_skill;
	if(strcmp(skillname,(char*)attributes.magic_skill.shortname)==0) return &your_info.magic_skill;
	if(strcmp(skillname,(char*)attributes.summoning_skill.shortname)==0) return &your_info.summoning_skill;
	if(strcmp(skillname,(char*)attributes.attack_skill.shortname)==0) return &your_info.attack_skill;
	if(strcmp(skillname,(char*)attributes.defense_skill.shortname)==0) return &your_info.defense_skill;
	if(strcmp(skillname,(char*)attributes.crafting_skill.shortname)==0) return &your_info.crafting_skill;
	if(strcmp(skillname,(char*)attributes.engineering_skill.shortname)==0) return &your_info.engineering_skill;
	if(strcmp(skillname,(char*)attributes.potion_skill.shortname)==0) return &your_info.potion_skill;
	if(strcmp(skillname,(char*)attributes.tailoring_skill.shortname)==0) return &your_info.tailoring_skill;
	if(strcmp(skillname,(char*)attributes.ranging_skill.shortname)==0) return &your_info.ranging_skill;
	if(strcmp(skillname,(char*)attributes.overall_skill.shortname)==0) return &your_info.overall_skill;
	if(strcmp(skillname,(char*)attributes.harvesting_skill.shortname)==0) return &your_info.harvesting_skill;
	return NULL;
}

int put_on_cast(){
	if(we_have_spell>=0){
		int i;
		for(i=0;i<6;i++) 
			if(spells_list[we_have_spell].sigils[i]>=0)
				if(!sigils_list[spells_list[we_have_spell].sigils[i]].have_sigil) {
					//we miss at least a sigil, clear on_cast
					int j;
					for(j=0;j<6;j++) on_cast[j]=-1;
					return 0;
				}
		for(i=0;i<6;i++) on_cast[i]=spells_list[we_have_spell].sigils[i];
		return 1;
	}
	return 0;
}

int init_spells ()
{
	int i,j;
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;
	char *fname="./spells.xml";

	//init textures and structs
#ifdef	NEW_TEXTURES
	sigils_text = load_texture_cached("textures/sigils.dds", tt_gui);
#else	/* NEW_TEXTURES */
	sigils_text = load_texture_cache ("./textures/sigils.bmp", 0);
#endif	/* NEW_TEXTURES */
	for (i = 0; i < SIGILS_NO; i++)
		sigils_list[i].have_sigil = 0;
	for (i = 0; i < SPELLS_NO; i++){
		spells_list[i].image = -1;
		for(j=0;j<6;j++)
			spells_list[i].sigils[j] =-1;
		for(j=0;j<4;j++) {
			spells_list[i].reagents_id[j] = -1;
			spells_list[i].reagents_uid[j] = unset_item_uid;
		}
		for(j=0;j<NUM_WATCH_STAT;j++)
			spells_list[i].lvls[j] = NULL;
		spells_list[i].uncastable=0;
	}
	for (i = 0; i < GROUPS_NO; i++){
		groups_list[i].spells = 0;
		for(j=0;j<SPELLS_NO;j++) groups_list[i].spells_id[j]=-1;
	}


	spell_text[0]=spell_help[0]=0;
	i = 0;
	//parse xml
	doc = xmlReadFile(fname, NULL, 0);
	if (doc == 0) {
		LOG_ERROR("Unable to read spells definition file %s: %s", fname, strerror(errno));
		ok = 0;
	}

	root = xmlDocGetRootElement (doc);
	if (root == 0)
	{
		LOG_ERROR("Unable to parse spells definition file %s", fname);
		ok = 0;
	}
	else if (xmlStrcasecmp (root->name, (xmlChar*)"Magic") != 0)
	{
		LOG_ERROR("Unknown key \"%s\" (\"Magic\" expected).", root->name);
		ok = 0;
	}
	else
	{
		xmlNode *node;
		xmlNode *data;
		char tmp[200];
		char name[200];
		i = 0;
		//parse spells
		node = get_XML_node(root->children, "Spell_list");
		node = get_XML_node(node->children, "spell");
			
		while (node)
		{
			int j;

			memset(name, 0, sizeof(name));

			data=get_XML_node(node->children,"name");		

			if (data == 0)
			{
				LOG_ERROR("No name for %d spell", i);
			}

			get_string_value(name, sizeof(name), data);	
			safe_strncpy(spells_list[i].name, name,
				sizeof(spells_list[i].name));

			data=get_XML_node(node->children, "desc");			

			if (data == 0)
			{
				LOG_ERROR("No desc for spell '%s'[%d]",
					name, i);
			}

			get_string_value(tmp, sizeof(tmp), data);				
			safe_strncpy(spells_list[i].desc, tmp,
				sizeof(spells_list[i].desc));

			data=get_XML_node(node->children, "id");			

			if (data == 0)
			{
				LOG_ERROR("No id for spell '%s'[%d]",
					name, i);
			}

			spells_list[i].id=get_int_value(data);				

			data=get_XML_node(node->children,"icon");			

			if (data == 0)
			{
				LOG_ERROR("No icon for spell '%s'[%d]",
					name, i);
			}

			spells_list[i].image = get_int_value(data);				

			data=get_XML_node(node->children, "mana");			

			if (data == 0)
			{
				LOG_ERROR("No mana for spell '%s'[%d]",
					name, i);
			}

			spells_list[i].mana = get_int_value(data);				

			data=get_XML_node(node->children,"lvl");			

			if (data == 0)
			{
				LOG_ERROR("No lvl for spell '%s'[%d]",
					name, i);
			}

			j = 0;
			while (data)
			{
				const char *skill = get_string_property(data,"skill");
				spells_list[i].lvls_req[j] = get_int_value(data);
				spells_list[i].lvls[j] = get_skill_address(skill);
				j++;
				data = get_XML_node(data->next,"lvl");
			}

			data = get_XML_node(node->children,"group");

			if (data == 0)
			{
				LOG_ERROR("No group for spell '%s'[%d]",
					name, i);
			}

			while (data)
			{
				int g;

				g = get_int_value(data);				
				groups_list[g].spells_id[groups_list[g].spells] = i;
				groups_list[g].spells++;
				data = get_XML_node(data->next, "group");				
			}			

			data = get_XML_node(node->children, "sigil");

			if (data == 0)
			{
				LOG_ERROR("No sigil for spell '%s'[%d]",
					name, i);
			}

			j = 0;
			while (data)
			{
				spells_list[i].sigils[j] = get_int_value(data);
				j++;
				data = get_XML_node(data->next, "sigil");				
			}

			data=get_XML_node(node->children, "reagent");			

			if (data == 0)
			{
				LOG_ERROR("No reagent for spell '%s'[%d]",
					name, i);
			}

			j = 0;
			while (data)
			{
				int tmpval = -1;
				spells_list[i].reagents_id[j] =
					get_int_property(data, "id");
				if ((tmpval = get_int_property(data, "uid")) >= 0)
					spells_list[i].reagents_uid[j] = (Uint16)tmpval;
				spells_list[i].reagents_qt[j] =
					get_int_value(data);
				j++;
				data = get_XML_node(data->next, "reagent");				
			}

			data = get_XML_node(node->children, "duration");

			if (data != 0)
			{
				spells_list[i].duration = get_int_value(data);
			}
			else
			{
				spells_list[i].duration = 0;
			}

			data = get_XML_node(node->children, "buff");

			if (data != 0)
			{
				spells_list[i].buff = get_int_value(data);
			}
			else
			{
				spells_list[i].buff = 0;
			}

			node = get_XML_node(node->next, "spell");			
			i++;
		}
		num_spells = i;

		//parse sigils
		node = get_XML_node(root->children, "Sigil_list");
		node = get_XML_node(node->children, "sigil");
		while (node)
		{
			int k;
			k = get_int_property(node, "id");
			sigils_list[k].sigil_img = k;
			get_string_value(sigils_list[k].description,
				sizeof(sigils_list[k].description), node);				
			safe_strncpy((char*)sigils_list[k].name,
				get_string_property(node, "name"),
				sizeof(sigils_list[k].name));
			sigils_list[k].have_sigil = 1;
			node = get_XML_node(node->next, "sigil");
		}

		//parse groups
		num_groups = 0;	
		node = get_XML_node(root->children,"Groups");
		node = get_XML_node(node->children,"group");
		while (node)
		{
			int k;
			k = get_int_property(node, "id");
			get_string_value(tmp, sizeof(tmp), node);
			safe_strncpy((char*)groups_list[k].desc, tmp,
				sizeof(groups_list[k].desc));
			num_groups++;
			node = get_XML_node(node->next, "group");
		}
	}

	xmlFreeDoc (doc);

	//init arrays
	for (i = 0; i < 6; i++)
	{
		on_cast[i] = -1;
	}
	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		active_spells[i].spell = -1;
		active_spells[i].cast_time = 0;
		active_spells[i].duration = 0;
#ifdef NEW_SOUND
		if (active_spells[i].sound > 0)
		{
			stop_sound(active_spells[i].sound);
		}
#endif // NEW_SOUND
	}

	if (!ok) //xml failed, init sigils manually
	{
		init_sigils();
	}

	init_ok = ok;

	return ok;
}

void check_castability()
{
	int i,j,k,l;

	for(i=0;i<num_spells;i++){
		spells_list[i].uncastable=0;
		//Check Mana		
		if (have_stats && your_info.ethereal_points.cur<spells_list[i].mana) spells_list[i].uncastable|=UNCASTABLE_MANA;
		//Check Sigils		
		for(j=0;j<6;j++){
			k=spells_list[i].sigils[j];
			if(k>=0 && !sigils_list[k].have_sigil) spells_list[i].uncastable|=UNCASTABLE_SIGILS;
		}
		//Check Reagents		
		for(j=0;j<4&&spells_list[i].reagents_id[j]>=0;j++){
			l=0;
			for(k=0;k<ITEM_WEAR_START;k++) {
				if ((item_list[k].quantity > 0) &&
					(item_list[k].image_id == spells_list[i].reagents_id[j]) &&
					((item_list[k].id == unset_item_uid) ||
						(spells_list[i].reagents_uid[j] == unset_item_uid) ||
						(item_list[k].id == spells_list[i].reagents_uid[j])) ) {
					l=1;
					if(item_list[k].quantity<spells_list[i].reagents_qt[j]) {
						spells_list[i].uncastable|=UNCASTABLE_REAGENTS;
						break;
					}
				}
			}
			if(!l){
				//no reagent j found			
				spells_list[i].uncastable|=UNCASTABLE_REAGENTS;
			}	
		}
		//Check Levels
		for(j=0;j<NUM_WATCH_STAT&&spells_list[i].lvls[j];j++)
			if(spells_list[i].lvls[j])
				if(spells_list[i].lvls[j]->cur<spells_list[i].lvls_req[j]) spells_list[i].uncastable|=UNCASTABLE_LVLS;
		
	}
	//when castabilitychanges, update spell_help
	set_spell_help_text(we_have_spell);
}

/* called each time we get poisoned - perhaps */
void increment_poison_incidence(void)
{
	poison_drop_counter++;
}

/* called from display_game_handler() so we are in a position to draw text */
void draw_spell_icon_strings(void)
{
	size_t i;
	int x_start = 0;
	int y_start = window_height-hud_y-64-SMALL_FONT_Y_LEN;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		unsigned char str[20];
		/* handle the poison count */
		if ((poison_drop_counter > 0) && (active_spells[i].spell == 2) && show_poison_count)
		{
			safe_snprintf((char*)str, sizeof(str), "%d", poison_drop_counter );
			draw_string_small_shadowed(x_start+(33-strlen((char *)str)*SMALL_FONT_X_LEN)/2, y_start, str, 1, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		}
		/* other strings on spell icons, timers perhaps .....*/
		x_start += 33;
	}

}

//ACTIVE SPELLS
void get_active_spell(int pos, int spell)
{
	Uint32 i;

	active_spells[pos].spell = spell;
	active_spells[pos].cast_time = SDL_GetTicks();
#ifdef NEW_SOUND
	active_spells[pos].sound = add_spell_sound(spell);
#endif // NEW_SOUND

	for (i = 0; i < SPELLS_NO; i++)
	{
		if (spell == spells_list[i].buff)
		{
			active_spells[pos].duration = spells_list[i].duration;
			return;
		}
	}
}

void remove_active_spell(int pos)
{
	if (active_spells[pos].spell == 2)
		poison_drop_counter = 0;
	active_spells[pos].spell = -1;
	active_spells[pos].cast_time = 0;
	active_spells[pos].duration = 0;
#ifdef NEW_SOUND
	if (active_spells[pos].sound > 0)
		stop_sound(active_spells[pos].sound);
#endif // NEW_SOUND
}

void get_active_spell_list(const Uint8 *my_spell_list)
{
	Uint32 i, j;
	int cur_spell;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		active_spells[i].spell = my_spell_list[i];
		active_spells[i].cast_time = 0xFFFFFFFF;

		cur_spell = my_spell_list[i];

		for (j = 0; j < SPELLS_NO; j++)
		{
			if (cur_spell == spells_list[j].buff)
			{
				active_spells[i].duration =
					spells_list[j].duration;
				break;
			}
		}
#ifdef NEW_SOUND
		active_spells[i].sound = add_spell_sound(active_spells[i].spell);
#endif // NEW_SOUND
		if (active_spells[i].spell == 2)
			increment_poison_incidence();
	}
}

#ifdef NEW_SOUND
void restart_active_spell_sounds(void)
{
	Uint32 i;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		if (active_spells[i].sound > 0)
		{
			stop_sound(active_spells[i].sound);
		}
		if (active_spells[i].spell != -1)
		{
			active_spells[i].sound = add_spell_sound(active_spells[i].spell);
		}
	}
}
#endif // NEW_SOUND

int we_are_poisoned()
{
	Uint32 i;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		if (active_spells[i].spell == 2)
		{
			return 1;
		}
	}
	return 0;
}

void time_out(const float x_start, const float y_start, const float gridsize,
	const float progress)
{
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.0f, 0.7f, 0.0f);

	glBegin(GL_QUADS);
		glVertex2f(x_start, y_start + gridsize * progress);
		glVertex2f(x_start + gridsize, y_start + gridsize * progress);
		glVertex2f(x_start + gridsize, y_start + gridsize);
		glVertex2f(x_start, y_start + gridsize);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void display_spells_we_have()
{
	Uint32 i;
	float scale, duration, cur_time;

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	cur_time = SDL_GetTicks();

	//ok, now let's draw the objects...
	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		if (active_spells[i].spell != -1)
		{
			int cur_spell,cur_pos;
			int x_start,y_start;

			//get the UV coordinates.
			cur_spell = active_spells[i].spell + 32;	//the first 32 icons are the sigils

			//get the x and y
			cur_pos=i;

			x_start=33*cur_pos;
			y_start=window_height-hud_y-64;

			duration = active_spells[i].duration;

			if (duration > 0.0)
			{
				scale = (cur_time - active_spells[i].cast_time)
					/ duration;

				if ((scale >= 0.0) && (scale <= 1.0))
				{
					time_out(x_start, y_start, 32, scale);
				}
			}

			glEnable(GL_BLEND);
			draw_spell_icon(cur_spell,x_start,y_start,32,0,0);
			glDisable(GL_BLEND);
		}
	}
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int show_last_spell_help=0;



//DISPLAY HANDLERS

int draw_switcher(window_info *win){

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	
	//Draw switcher spells <-> sigils
	glBegin(GL_LINES);
		glVertex3i(win->len_x-20,40,0);
		glVertex3i(win->len_x,40,0);
		glVertex3i(win->len_x-20,40,0);
		glVertex3i(win->len_x-20,20,0);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3i(win->len_x-15,35,0);
		glVertex3i(win->len_x-5,35,0);
		glVertex3i(win->len_x-5,25,0);
		glVertex3i(win->len_x-15,25,0);	
	glEnd();

	if (sigil_win==spell_win || sigil_win==spell_mini_win) {
		//Draw switcher spells <-> mini
		glBegin(GL_LINES);
			glVertex3i(win->len_x-20,60,0);
			glVertex3i(win->len_x,60,0);
			glVertex3i(win->len_x-20,60,0);
			glVertex3i(win->len_x-20,40,0);
			if(sigil_win==spell_win) {
			//arrow down		
					glVertex3i(win->len_x-15,45,0);
					glVertex3i(win->len_x-10,55,0);
					glVertex3i(win->len_x-10,55,0);
					glVertex3i(win->len_x-5,45,0);	
			} else {
			//arrow up
					glVertex3i(win->len_x-15,55,0);
					glVertex3i(win->len_x-10,45,0);
					glVertex3i(win->len_x-10,45,0);
					glVertex3i(win->len_x-5,55,0);	
			}
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	return 1;
}

void draw_spell_icon(int id,int x_start, int y_start, int gridsize, int alpha, int grayed){

	float u_start,v_start,u_end,v_end;

#ifdef	NEW_TEXTURES
	u_start = 0.125f * (id % 8);
	v_start = 0.125f * (id / 8);
	u_end = u_start + 0.125f;
	v_end = v_start + 0.125f;

	bind_texture(sigils_text);
#else	/* NEW_TEXTURES */
	u_start=0.125f*(id%8);
	v_start=1.0f-((float)32/256*(id/8));
	u_end=u_start+0.125f;
	v_end=v_start-0.125f;

	get_and_set_texture_id(sigils_text);
#endif	/* NEW_TEXTURES */
	if(alpha) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.05f);
		glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end, x_start,y_start,x_start+gridsize,y_start+gridsize);
		glEnd();
		glDisable(GL_ALPHA_TEST);
	} else {
		glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end, x_start,y_start,x_start+gridsize,y_start+gridsize);
		glEnd();
	}

	if(grayed) gray_out(x_start,y_start,gridsize);
	
}

void draw_current_spell(int x, int y, int sigils_too){

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	if(we_have_spell>=0){
		int i,j;
		unsigned char str[4];
		//we have a current spell (cliked or casted) !!mqb_data[0] can still be null!!
		j=we_have_spell;
		draw_spell_icon(spells_list[j].image,x,y,32,1,0);

		if(sigils_too){
			//draw sigils	
			x+=33*2;
			for(i=0;i<6;i++){ 
				if (spells_list[j].sigils[i]<0) break;				
				draw_spell_icon(spells_list[j].sigils[i],x+33*i,y,32,0,spells_list[j].uncastable&UNCASTABLE_SIGILS);
				if(spells_list[j].uncastable&UNCASTABLE_SIGILS&&!sigils_list[spells_list[j].sigils[i]].have_sigil) gray_out(x+33*i,y,32);
			}
		}

		//draw reagents
		x+= (sigils_too) ? (33*6+33):(33+16);
		for(i=0;spells_list[j].reagents_id[i]>0;i++) { 	
			draw_item(spells_list[j].reagents_id[i],x+33*i,y,33);
			safe_snprintf((char *)str, sizeof(str), "%i",spells_list[j].reagents_qt[i]);
			draw_string_small_shadowed(x+33*i, y+21, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f);
			if(spells_list[j].uncastable&UNCASTABLE_REAGENTS) gray_out(x+33*i,y+1,32);
		}
		//draw mana
		x+=(sigils_too) ? (33*5):(33*4+17);
		safe_snprintf((char *)str, sizeof(str), "%i",spells_list[j].mana);
		if (spells_list[j].uncastable&UNCASTABLE_MANA) glColor3f(1.0f,0.0f,0.0f);
		else glColor3f(0.0,1.0,0.0);
		i=(33-get_string_width(str))/2;
		j=(33-get_char_width(str[0]))/2;
		draw_string(x+i,y+j,str,1);
	}
	
	//draw strings	
	x=20;
	glColor3f(0.77f,0.57f,0.39f);
	if(sigils_too) { 
		x+=33*2; 
		draw_string_small(x,y-15,(unsigned char*)"Sigils",1);
		x+=33*6+33;
	} else x+=33+16;

	draw_string_small(x,y-15,(unsigned char*)"Reagents",1);
	x+=33*4+((sigils_too) ? (33):(17));
	draw_string_small(x,y-15,(unsigned char*)"Mana",1);

	//draw grids
	glDisable(GL_TEXTURE_2D);
	x=20;
	if(sigils_too) { 
		x+=33*2; 
		rendergrid (6, 1, x, y, 33, 33);
		x+=33*6+33;
	} else x+=33+16;
	rendergrid (4, 1, x, y, 33, 33);
	x+=33*4+((sigils_too) ? (33):(17));
	rendergrid (1, 1, x, y, 33, 33);
}

int display_sigils_handler(window_info *win)
{
	int i;
	int x_start,y_start;

	if (init_ok) draw_switcher(win);	

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);

	//let's add the new spell icon if we have one
	x_start=350;
	y_start=112;
	if(mqb_data[0] && mqb_data[0]->spell_id!=-1) draw_spell_icon(mqb_data[0]->spell_image,x_start,y_start,32,1,0);

	//ok, now let's draw the objects...
	for(i=0;i<SIGILS_NO;i++){
		if(sigils_list[i].have_sigil){
			//get the x and y
			x_start=33*(i%NUM_SIGILS_LINE)+1;
			y_start=33*(i/NUM_SIGILS_LINE);
			draw_spell_icon(sigils_list[i].sigil_img,x_start,y_start,32,0,0);
		}
	}

	//ok, now let's draw the sigils on the list
	for(i=0;i<6;i++)
	{
		if(on_cast[i]!=-1)
		{
			//get the x and y
			x_start=33*(i%6)+5;
			y_start=sigil_y_len-37;
			draw_spell_icon(on_cast[i],x_start,y_start,32,0,0);
		}
	}

	//now, draw the inventory text, if any.
	draw_string_small(4,sigil_y_len-90,spell_text,4);

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	rendergrid (NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, 33, 33);
	rendergrid (6, 1, 5, sigil_y_len-37, 33, 33);

	glEnable(GL_TEXTURE_2D);

	if(show_last_spell_help && mqb_data[0] && mqb_data[0]->spell_id!=-1)show_help(mqb_data[0]->spell_name,350-8*strlen(mqb_data[0]->spell_name),120);
	show_last_spell_help=0;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}



int display_spells_handler(window_info *win){

	int i,j,k,x,y;

	draw_switcher(win);	
	
	//Draw spell groups
	for(i=0;i<num_groups;i++){
		x=groups_list[i].x;
		y=groups_list[i].y;
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f,1.0f,1.0f);
		draw_string_small(x,y-15,groups_list[i].desc,1);		
		for(k=0,j=0;j<groups_list[i].spells;j++){
			draw_spell_icon(spells_list[groups_list[i].spells_id[j]].image,
					x+33*(k%SPELLS_ALIGN_X),
					y+33*(k/SPELLS_ALIGN_X),32,
					0,spells_list[groups_list[i].spells_id[j]].uncastable);
			k++;
		}
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.77f,0.57f,0.39f);
		rendergrid(SPELLS_ALIGN_X,groups_list[i].spells/(SPELLS_ALIGN_X+1)+1,x,y,33,33);
	}

	glEnable(GL_TEXTURE_2D);

	//draw spell text & help
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(20,spell_y_len-100,spell_text,4);
	draw_string_small(20,spell_y_len+5,spell_help,2);

	//draw the bottom bar
	draw_current_spell(20,spell_y_len-37,1);
	if(we_have_spell>=0&&spells_list[we_have_spell].uncastable){
		//not castable
		glColor3f(1.0f,0.0f,0.0f);
		rendergrid(1,1,20,spell_y_len-37,33,33);
		
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


int display_spells_mini_handler(window_info *win){

	int i,j,x,y,cg,cs;

	draw_switcher(win);	

	glEnable(GL_TEXTURE_2D);
	x=20;y=10;
	glColor3f(1.0f,1.0f,1.0f);
	for (i=0,cs=0,cg=0;i<spell_mini_rows;i++)
		for (j=0;j<SPELLS_ALIGN_X;j++){
			if (cs==groups_list[cg].spells) {cs=0; cg++; break;}
			draw_spell_icon(spells_list[groups_list[cg].spells_id[cs]].image,
					x+j*33,y+33*i,32,
					0,spells_list[groups_list[i].spells_id[j]].uncastable);
			cs++;
		}
	
	//draw spell help
	if(on_spell==-2) {
		//mouse over the bottom-left selected spell icon, show uncastability
		int l=(int)(get_string_width((unsigned char*)GET_UNCASTABLE_STR(spells_list[we_have_spell].uncastable))*(float)DEFAULT_SMALL_RATIO);
		SET_COLOR(c_red2);
		draw_string_small(20+(33*SPELLS_ALIGN_X-l)/2,spell_mini_y_len-37-35,(unsigned char*)GET_UNCASTABLE_STR(spells_list[we_have_spell].uncastable),1);		
	} else {
		i=(on_spell>=0) ? (on_spell):(we_have_spell);
		if(i>=0){
			int l=(int)(get_string_width((unsigned char*)spells_list[i].name)*(float)DEFAULT_SMALL_RATIO);
			if (on_spell>=0) SET_COLOR(c_grey1);
			else SET_COLOR(c_green3);
			draw_string_small(20+(33*SPELLS_ALIGN_X-l)/2,spell_mini_y_len-37-35,(unsigned char*)spells_list[i].name,1);
		}
	}
	

	//draw the current spell
	draw_current_spell(x,spell_mini_y_len-37,0);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	rendergrid(SPELLS_ALIGN_X,spell_mini_rows,x,y,33,33);
	
	if(we_have_spell>=0&&spells_list[we_have_spell].uncastable){
		//not castable, red grid
		glColor3f(1.0f,0.0f,0.0f);
		rendergrid(1,1,20,spell_mini_y_len-37,33,33);
	}

	glEnable(GL_TEXTURE_2D);
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}



//CLICK HANDLERS
int switch_handler(int new_win){
	window_info *win;
	int this_win;

	last_win=sigil_win;
	this_win=new_win;

	win=&windows_list.window[last_win];
	windows_list.window[this_win].opaque=windows_list.window[last_win].opaque;
	hide_window(last_win);
	move_window(this_win, win->pos_id, win->pos_loc, win->pos_x, win->pos_y);
	show_window(this_win);
	select_window(this_win);			
	sigil_win=this_win;
	start_mini_spells=(sigil_win==spell_mini_win)? 1:0;

	return 1;
}


int click_switcher_handler(window_info *win, int mx, int my, Uint32 flags){

	if (mx>=win->len_x-20&&my>=20&&my<=40) {
		do_click_sound();
		switch_handler((sigil_win==sigils_win) ? (last_win):(sigils_win));				
	} else if(sigil_win==spell_win || sigil_win==spell_mini_win){
		if (mx>=win->len_x-20&&my>=40&&my<=60) {
			do_click_sound();
			switch_handler((sigil_win==spell_win) ? (spell_mini_win):(spell_win));
		}						
	}
	return 0;
}


int click_sigils_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// only handle real clicks, not scroll wheel moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0 ) {
		return 0;
	} else if(mx>=350 && mx<=381 && my>=112 && my<=143&&mqb_data[0] && mqb_data[0]->spell_id!=-1) {
		add_spell_to_quickbar();
		return 1;
	} else if(mx>0 && mx<NUM_SIGILS_LINE*33 && my>0 && my<NUM_SIGILS_ROW*33) {
		int pos=get_mouse_pos_in_grid(mx,my, NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, 33, 33);

		if (pos >= 0 && sigils_list[pos].have_sigil) {
			int j;
			int image_id=sigils_list[pos].sigil_img;

			//see if it is already on the list
			for(j=0;j<6;j++) {
				if(on_cast[j]==image_id) {
					return 1;
				}
			}

			for(j=0;j<6;j++) {
				if(on_cast[j]==-1) {
					on_cast[j]=image_id;
					return 1;
				}
			}
			return 1;
		}
	} else if(mx>5 && mx<6*33+5 && my>sigil_y_len-37 && my<sigil_y_len-5) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, sigil_y_len-37, 33, 33);

		if (pos >= 0) {
			on_cast[pos]=-1;
		}
	}
	if (init_ok) click_switcher_handler(win,mx,my,flags);
	return 0;
}

int click_spells_handler(window_info *win, int mx, int my, Uint32 flags){
	int pos,i,the_group=-1,the_spell=-1;
	static int last_clicked=0;
	static int last_pos=-1;	

	if (!(flags & ELW_MOUSE_BUTTON)) return 0;

	for(i=0;i<num_groups;i++){
		pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, groups_list[i].spells/(SPELLS_ALIGN_X+1)+1, groups_list[i].x, groups_list[i].y, 33, 33);
		if(pos>=0&&pos<groups_list[i].spells) {
			the_group=i;
			the_spell=pos;
			break;
		}
	}
	
	if (the_spell!=-1){
		//a spell has been clicked
		int code_pos=(the_group*256+the_spell);
		we_have_spell=groups_list[the_group].spells_id[the_spell];
		put_on_cast();
		//handle double click && cast spell
		if ( ((SDL_GetTicks() - last_clicked) < 400)&&last_pos==code_pos) cast_handler();
		else have_error_message=0; //if not double click, clear server msg
		last_pos=code_pos;
	} else {
		last_pos=-1;
		//check spell icon
		if(we_have_spell>=0&&mx>20&&mx<53&&my>spell_y_len-37&&my<spell_y_len-4) {
			if(flags & ELW_LEFT_MOUSE) {
				//cast spell
				if (put_on_cast()) cast_handler();
			} else if (flags & ELW_RIGHT_MOUSE) {
				//add to quickbar
				if(put_on_cast()) {
					prepare_for_cast();
					add_spell_to_quickbar();
				}
			}
		} else click_switcher_handler(win,mx,my,flags);
	}
	last_clicked = SDL_GetTicks();	
	return 0;
}
int click_spells_mini_handler(window_info *win, int mx, int my, Uint32 flags){

	int pos;
	static int last_clicked=0;
	static int last_pos=-1;

	if (!(flags & ELW_MOUSE_BUTTON)) return 0;

	pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, spell_mini_rows, 20, 10, 33, 33);	
	if (pos>=0){
		int i,j,cs,cg,the_spell=-1,the_group=-1,the_pos=pos;		
		//find the spell clicked
		for (i=0,cs=0,cg=0;i<spell_mini_rows&&the_pos>=0;i++) {
			for (j=0;j<SPELLS_ALIGN_X;j++){
				the_pos--;
				if (the_pos==-1) { the_spell=cs; the_group=cg;}
				else if(the_pos<-1) break;
				if (cs==groups_list[cg].spells-1) {cs=0; cg++; the_pos-=(SPELLS_ALIGN_X-j-1); break;}
				else cs++;
			}
		}
		//put it on the cast bar
		if(the_spell!=-1){
			we_have_spell=groups_list[the_group].spells_id[the_spell];
			put_on_cast();
			//handle double click
			if ( ((SDL_GetTicks() - last_clicked) < 400)&&last_pos==pos) cast_handler();
		}
	} else {
		//check if clicked on the spell icon
		if(we_have_spell>=0&&mx>=20&&mx<=53&&my>=spell_mini_y_len-37&&my<=spell_mini_y_len-4) {
			if(flags & ELW_LEFT_MOUSE) {
				if (put_on_cast()) cast_handler();
			} else if (flags & ELW_RIGHT_MOUSE) {
				//add to quickbar
				if(put_on_cast()){
					prepare_for_cast();
					add_spell_to_quickbar();
				}
			}
		} else click_switcher_handler(win,mx,my,flags);
	}
	last_pos=pos;
	last_clicked = SDL_GetTicks();
	return 0;
}



//MOUSEOVER HANDLERS
int mouseover_sigils_handler(window_info *win, int mx, int my)
{
	if(!have_error_message) {
		spell_text[0] = 0;
	}

	if(mx>=350 && mx<=381 && my>=112 && my<=143&&mqb_data[0] &&mqb_data[0]->spell_name[0]) {
		show_last_spell_help = 1;
	}

	//see if we clicked on any sigil in the main category
	if(mx>0 && mx<NUM_SIGILS_LINE*33 && my>0 && my<NUM_SIGILS_ROW*33) {
		int pos=get_mouse_pos_in_grid(mx,my, NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, 33, 33);

		if (pos >= 0 && sigils_list[pos].have_sigil)
		{
			my_strcp((char*)spell_text,sigils_list[pos].name);
			have_error_message=0;
		}
		return 0;
	}

	//see if we clicked on any sigil from "on cast"
	if(mx>5 && mx<6*33+5 && my>sigil_y_len-37 && my<sigil_y_len-5) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, sigil_y_len-37, 33, 33);

		if (pos >= 0 && on_cast[pos]!=-1){
			my_strcp((char*)spell_text,sigils_list[on_cast[pos]].name);
			have_error_message=0;
		}
		return 0;
	}

	if(mx>=350 && mx<=381 && my>=112 && my<=143 && mqb_data[0] && mqb_data[0]->spell_id != -1) {
		safe_snprintf((char*)spell_text, sizeof(spell_text), "Click to add the spell to the quickbar");
		return 0;
	}

	return 0;
}


void set_spell_help_text(int spell){

	char clr[4];

	if (spell<0) {
		spell_help[0]=0;
		return;
	}

	//Set spell name color
	if (spell==we_have_spell) spell_help[0]=127+c_green3;
	else spell_help[0]=127+c_orange2;
	spell_help[1]=0;

	//Set spell name
	safe_strcat((char*)spell_help,spells_list[spell].name,sizeof(spell_help));

	//Set uncastable message
	if(spells_list[spell].uncastable){
		clr[0]=127+c_red2;
		clr[1]=clr[2]=' ';
		clr[3]=0;
		safe_strcat((char*)spell_help,clr,sizeof(spell_help));		
		safe_strcat((char*)spell_help,GET_UNCASTABLE_STR(spells_list[spell].uncastable),sizeof(spell_help));
	}
	safe_strcat((char*)spell_help,"\n",sizeof(spell_help));
	clr[0]=127+c_grey1;
	clr[1]=0;
	safe_strcat((char*)spell_help,clr,sizeof(spell_help));
	safe_strcat((char*)spell_help,spells_list[spell].desc,sizeof(spell_help));

}

int mouseover_spells_handler(window_info *win, int mx, int my){
	int i,pos;
	
	if(!have_error_message) {
		spell_text[0] = 0;
	}

	on_spell=-1;
	for(i=0;i<num_groups;i++){
		pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, groups_list[i].spells/(SPELLS_ALIGN_X+1)+1, groups_list[i].x, groups_list[i].y, 33, 33);
		if(pos>=0&&pos<groups_list[i].spells) {
			on_spell=groups_list[i].spells_id[pos];
			set_spell_help_text(on_spell);
			return 0;				
		}
	}
	set_spell_help_text(we_have_spell);
	//check spell icon
	if(mx>20&&mx<53&&my>spell_y_len-37&&my<spell_y_len-4&&we_have_spell>=0) {
		safe_snprintf((char*)spell_text, sizeof(spell_text), "Left click to cast\nRight click to add the spell to the quickbar");
		have_error_message=0;		
	}
	return 0;
}
int mouseover_spells_mini_handler(window_info *win, int mx, int my){

	int pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, spell_mini_rows, 20, 10, 33, 33);	
	on_spell=-1;
	if (pos>=0){
		int i,j,cs,cg,the_spell=-1,the_group=-1,the_pos=pos;		
		//find the spell clicked
		for (i=0,cs=0,cg=0;i<spell_mini_rows&&the_pos>=0;i++) {
			for (j=0;j<SPELLS_ALIGN_X;j++){
				the_pos--;
				if (the_pos==-1) { the_spell=cs; the_group=cg;}
				else if(the_pos<-1) break;
				if (cs==groups_list[cg].spells-1) {cs=0; cg++; the_pos-=(SPELLS_ALIGN_X-j-1); break;}
				else cs++;
			}
		}
		if(the_spell!=-1) on_spell=groups_list[the_group].spells_id[the_spell];
	} else if(mx>20&&mx<53&&my>spell_mini_y_len-37&&my<spell_mini_y_len-4&&we_have_spell>=0) {
		//check spell icon
		on_spell=-2; //draw uncastability reason
	}
	return 0;
}




//MISC FUNCTIONS
void get_sigils_we_have(Uint32 sigils_we_have, Uint32 sigils2)
{
	int i;
	int po2=1;

	// the first 32 sigils
	for(i=0;i<32;i++)
		{
			if((sigils_we_have&po2))sigils_list[i].have_sigil=1;
			else sigils_list[i].have_sigil=0;
			po2*=2;
		}

	// the next optional sigils
	po2= 1;
	for(i=32;i<SIGILS_NO;i++)
		{
			if((sigils2&po2))sigils_list[i].have_sigil=1;
			else sigils_list[i].have_sigil=0;
			po2*=2;
		}
	check_castability();
}


int have_spell_name(int spell_id)
{
	int i;

	for(i=1;i<MAX_QUICKBAR_SLOTS+1;i++){
		if(mqb_data[i] && mqb_data[i]->spell_id==spell_id && mqb_data[i]->spell_name[0]){
			if(mqb_data[0])
				safe_snprintf(mqb_data[0]->spell_name, sizeof(mqb_data[0]->spell_name), "%s", mqb_data[i]->spell_name);
			return 1;
		}
	}
	return 0;
}


void set_spell_name (int id, const char *data, int len)
{
	int i;

	if (len >= 60) return;

	counters_set_spell_name(id, (char *)data, len);

	for (i = 0; i < MAX_QUICKBAR_SLOTS+1; i++)
	{
		if (mqb_data[i] != NULL && mqb_data[i]->spell_id==id)
		{
			safe_snprintf (mqb_data[i]->spell_name, sizeof(mqb_data[i]->spell_name), "%.*s", len, data);
		}
	}

}

static void spell_cast(const Uint8 id)
{
	Uint32 i, spell;

	spell = 0xFFFFFFFF;

	for (i = 0; i < SPELLS_NO; i++)
	{
		if (spells_list[i].id == id)
		{
			spell = spells_list[i].buff;
			break;
		}
	}

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		if (active_spells[i].spell == spell)
		{
			active_spells[i].cast_time = SDL_GetTicks();
			return;
		}
	}
}

void process_network_spell (const char *data, int len)
{
	last_spell_name[0] = '\0';
	switch (data[0])
	{
		case S_INVALID:
			spell_result=0;
			LOG_TO_CONSOLE(c_red1, invalid_spell_str);
			return;
		case S_NAME:
			set_spell_name (data[1], &data[2], len-2);//Will set the spell name of the given ID
			return;;
		case S_SELECT_TARGET://spell_result==3
			spell_result=3;
			action_mode=ACTION_WAND;
			break;
		case S_SELECT_TELE_LOCATION://spell_result==2
			// we're about to teleport, don't let the pathfinder
			// interfere with our destination
			if (pf_follow_path) pf_destroy_path ();
			spell_result=2;
			action_mode=ACTION_WAND;
			break;
		case S_SUCCES://spell_result==1
			spell_result=1;
			action_mode=ACTION_WALK;
			spell_cast(data[1]);
			break;
		case S_FAILED:
			spell_result=0;
			action_mode=ACTION_WALK;
			return;
	}

	if(!mqb_data[0]){
		mqb_data[0]=(mqbdata*)calloc(1,sizeof(mqbdata));
		mqb_data[0]->spell_id=-1;
	}

	if(mqb_data[0]->spell_id!=data[1]){
		if(!have_spell_name(data[1])){
			Uint8 str[2];

			str[0]=SPELL_NAME;
			str[1]=data[1];
			my_tcp_send(my_socket, str, 2);
		}

		mqb_data[0]->spell_id=data[1];
		mqb_data[0]->spell_image=data[2];
	}
}



/****** QUICKSPELLS FUNCTIONS *****/

//Quickspell I/O start


void add_spell_to_quickbar()
{
	int i;

	if(!mqb_data[0])
		return;

	for(i=1;i<num_quickbar_slots+1;i++) {
		if(mqb_data[i] && mqb_data[0]->spell_id==mqb_data[i]->spell_id) {
			return;
		}
	}

	for (i = 1; i < num_quickbar_slots+1; i++)
	{
		if (mqb_data[i] == NULL)
		{
			// Free slot
			mqb_data[i] = calloc(1, sizeof (mqbdata));
			break;
		}
	}

	if (i >= num_quickbar_slots+1)
		// No free slot, overwrite the last entry
		i = num_quickbar_slots;

	memcpy (mqb_data[i], mqb_data[0], sizeof (mqbdata));
	save_quickspells();
	cm_update_quickspells();
}

void remove_spell_from_quickbar (int pos)
{
	int i;

	if (pos < 1 || pos > num_quickbar_slots || mqb_data[pos] == NULL) {
		return;
	}

	// remove the spell
	free (mqb_data[pos]);

	// move the other spells one up
	for (i = pos; i < MAX_QUICKBAR_SLOTS; i++) {
		mqb_data[i] = mqb_data[i+1];
	}
	mqb_data[MAX_QUICKBAR_SLOTS] = NULL;
	save_quickspells();
	cm_update_quickspells();
}


void move_spell_on_quickbar (int pos, int direction)
{
	int i=pos;
	mqbdata * mqb_temp;
	if (pos < 1 || pos > num_quickbar_slots || mqb_data[pos] == NULL) return;
	if ((pos ==1 && direction==0)||(pos==num_quickbar_slots && direction==1)) return;
	if (direction==0){
		mqb_temp=mqb_data[i-1];
		mqb_data[i-1]=mqb_data[i]; //move it up
		mqb_data[i]=mqb_temp; //move it up
		save_quickspells();
	}
	else if(direction==1){
		if(mqb_data[pos+1] == NULL) return;
		mqb_temp=mqb_data[i+1];
		mqb_data[i+1]=mqb_data[i]; //move it down
		mqb_data[i]=mqb_temp; //move it down
		save_quickspells();
	}
}

static mqbdata* build_quickspell_data(const Uint32 spell_id)
{
	Uint8 str[20];
	mqbdata* result;
	Uint32 i, count, index, len, size;

	index = 0xFFFFFFFF;

	for (i = 0; i < SPELLS_NO; i++)
	{
		if (spells_list[i].id == spell_id)
		{
			index = i;
			break;
		}
	}

	if (index == 0xFFFFFFFF)
	{
		LOG_WARNING("Invalid spell id %d", spell_id);

		return 0;
	}

	memset(str, 0, sizeof(str));

	count = 0;

	for (i = 0; i < 6; i++)
	{
		if (spells_list[index].sigils[i] != -1)
		{
			str[count + 2] = spells_list[index].sigils[i];
			count++;
		}
	}

	str[0] = CAST_SPELL;
	str[1] = count;

	result = calloc(1, sizeof(mqbdata));

	if (result == 0)
	{
		LOG_WARNING("Can't allocate memory for spell");

		return 0;
	}

	result->spell_id = spells_list[index].id;
	result->spell_image = spells_list[index].image;

	size = sizeof(result->spell_name);

	len = strlen(spells_list[index].name);

	if (size > len)
	{
		size = len;
	}
	else
	{
		size -= 1;
	}

	memset(result->spell_name, 0, size);
	memset(result->spell_str, 0, sizeof(result->spell_str));
	memcpy(result->spell_name, spells_list[index].name, len);
	memcpy(result->spell_str, str, count + 2);

	return result;
}

void load_quickspells ()
{
	char fname[128];
	Uint8 num_spells;
	FILE *fp;
	Uint32 i, index;

	// Grum: move this over here instead of at the end of the function,
	// so that quickspells are always saved when the player logs in.
	// (We're only interested in if this function is called, not if it
	// succeeds)
	quickspells_loaded = 1;

	//open the data file
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",username_str);
	my_tolower(fname);

	/* sliently ignore non existing file */
	if (file_exists_config(fname)!=1)
		return;

	fp = open_file_config(fname,"rb");

	if (fp == NULL)
	{
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file,
			fname, strerror(errno));
		return;
	}

	if (fread(&num_spells, sizeof(num_spells), 1, fp) != 1)
	{
		LOG_ERROR("%s() read failed for [%s] \n", __FUNCTION__, fname);
		fclose (fp);
		return;
	}

	ENTER_DEBUG_MARK("load spells");

	if (num_spells > 0)
	{
		num_spells--;
	}

	if (num_spells > MAX_QUICKBAR_SLOTS)
	{
		LOG_WARNING("Too many spells (%d), only %d spells allowed",
			num_spells, MAX_QUICKBAR_SLOTS);

		num_spells = MAX_QUICKBAR_SLOTS;
	}

	memset(mqb_data, 0, sizeof (mqb_data));

	LOG_DEBUG("Reading %d spells from file '%s'", num_spells, fname);

	index = 1;

	for (i = 0; i < num_spells; i++)
	{
		mqbdata tmp;

		if (fread(&tmp, sizeof(mqbdata), 1, fp) != 1)
		{
			LOG_ERROR("Failed reading spell %d from file '%s'", i,
				fname);
			continue;
		}

		mqb_data[index] = build_quickspell_data(tmp.spell_id);

		if (mqb_data[index] == 0)
		{
			continue;
		}

		LOG_DEBUG("Added quickspell %d '%s' at index %d", i,
			mqb_data[index]->spell_name, index);

		index++;
	}
	fclose (fp);

	cm_update_quickspells();

	LEAVE_DEBUG_MARK("load spells");
}

void save_quickspells()
{
	char fname[128];
	FILE *fp;
	Uint8 i;

	if (!quickspells_loaded)
		return;

	//write to the data file, to ensure data integrity, we will write all the information
	safe_snprintf(fname, sizeof(fname), "spells_%s.dat",username_str);
	my_tolower(fname);
	fp=open_file_config(fname,"wb");
	if(fp == NULL){
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
		return;
	}

	for (i = 1; i < MAX_QUICKBAR_SLOTS+1; i++)
	{
		if (mqb_data[i] == NULL)
			break;
	}

	ENTER_DEBUG_MARK("save spells");

	// write the number of spells + 1
	fwrite(&i, sizeof(i), 1, fp);

	LOG_DEBUG("Writing %d spells to file '%s'", i, fname);

	for (i = 1; i < (MAX_QUICKBAR_SLOTS + 1); i++)
	{
		if (mqb_data[i] == 0)
		{
			break;
		}

		if (fwrite(mqb_data[i], sizeof(mqbdata), 1, fp) != 1)
		{
			LOG_ERROR("Failed writing spell '%s' to file '%s'",
				mqb_data[i]->spell_name, fname);
			break;
		}
		
		LOG_DEBUG("Wrote spell '%s' to file '%s'",
			mqb_data[i]->spell_name, fname);
	}

	fclose(fp);

	LEAVE_DEBUG_MARK("save spells");
}

// Quickspell window start

int quickspell_over=-1;

// get the quickbar length - it depends on the numbe rof slots active
int get_quickspell_y_len(void)
{
	return num_quickbar_slots*30;
}

/*	returns the y coord position of the active base
	of the quickspell window.  If spell slots are unused
	the base is higher */
int get_quickspell_y_base()
{
	int active_len = quickspell_y + get_quickspell_y_len();
	int i;

	if (!quickspells_loaded)
		return quickspell_y;

	for (i = num_quickbar_slots; i > 0; i--)
	{
		if (mqb_data[i] == NULL)
			active_len -= 30;
		else
			break;
	}
	return active_len;
}


int display_quickspell_handler(window_info *win)
{
	int x,y,width,i;
	static int last_num_quickbar_slots = -1;

	// Check for a change of the number of quickbar slots
	if (last_num_quickbar_slots == -1)
		last_num_quickbar_slots = num_quickbar_slots;
	else if (last_num_quickbar_slots != num_quickbar_slots)
	{
		last_num_quickbar_slots = num_quickbar_slots;
		init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, win->len_x, get_quickspell_y_len());
		cm_update_quickspells();
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.20f);
	glEnable(GL_BLEND);	// Turn Blending On
	glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);

	for(i=1;i<num_quickbar_slots+1;i++) {
		if(mqb_data[i] && mqb_data[i]->spell_name[0]){
			x=quickspell_size/2;
			y=(i-1)*30+15;
			width=quickspell_size/2;
			
			if(quickspell_over==i){	//highlight if we are hovering over
				glColor4f(1.0f,1.0f,1.0f,1.0f);
			} else {	//otherwise shade it a bit
				glColor4f(1.0f,1.0f,1.0f,0.6f);
			}

			draw_spell_icon(mqb_data[i]->spell_image,x-width,y-width,quickspell_size,0,0);
		}
	}

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glDisable(GL_BLEND);	// Turn Blending Off
	glDisable(GL_ALPHA_TEST);

	if(quickspell_over!=-1 && mqb_data[quickspell_over])
		show_help(mqb_data[quickspell_over]->spell_name,-10-strlen(mqb_data[quickspell_over]->spell_name)*8,(quickspell_over-1)*30+10);
	quickspell_over=-1;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int mouseover_quickspell_handler(window_info *win, int mx, int my)
{
	int pos;

	pos=my/30+1;
	if(pos<num_quickbar_slots+1 && pos>=1 && mqb_data[pos] && mqb_data[pos]->spell_name[0]) {
		quickspell_over=pos;
		elwin_mouse=CURSOR_WAND;
		return 1;
	}
	return 0;
}

int click_quickspell_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;

	pos=my/30+1;

	if(pos<num_quickbar_slots+1 && pos>=1 && mqb_data[pos])
	{
		if ((flags & ELW_LEFT_MOUSE)&&(flags & ELW_SHIFT))
		{
			move_spell_on_quickbar (pos,0);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & ELW_SHIFT))
		{
			move_spell_on_quickbar (pos,1);
			return 1;
		}
		else if (flags & ELW_LEFT_MOUSE && mqb_data[pos]->spell_str[0])
		{
			send_spell(mqb_data[pos]->spell_str, mqb_data[pos]->spell_str[1]+2);
			return 1;
		}
		else if ((flags & ELW_RIGHT_MOUSE)&&(flags & ELW_CTRL))
		{
			remove_spell_from_quickbar (pos);
			return 1;
		}
	}
	return 0;
}

static int context_quickspell_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	int pos=my/30+1;
	if(pos<num_quickbar_slots+1 && pos>=1 && mqb_data[pos])
	{
		switch (option)
		{
			case 0: move_spell_on_quickbar (pos,0); break;
			case 1: move_spell_on_quickbar (pos,1); break;
			case 2: remove_spell_from_quickbar (pos); break;
		}
	}
	return 1;
}

void cm_update_quickspells(void)
{
	int active_y_len = 0, i;
	if (quickspell_win < 0)
		return;
	for (i = num_quickbar_slots; i > 0; i--)
	{
		if (mqb_data[i] != NULL)
			active_y_len += 30;
	}
	cm_remove_regions(quickspell_win);
	cm_add_region(cm_quickspells_id, quickspell_win, 0, 0, quickspell_x_len, active_y_len);
}

void init_quickspell()
{
	if (quickspell_win < 0){
		quickspell_win = create_window ("Quickspell", -1, 0, window_width - quickspell_x, quickspell_y, quickspell_x_len, get_quickspell_y_len(), ELW_CLICK_TRANSPARENT|ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(quickspell_win, ELW_HANDLER_DISPLAY, &display_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_CLICK, &click_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickspell_handler );
		cm_quickspells_id = cm_create(cm_quickspell_menu_str, &context_quickspell_handler);
		cm_update_quickspells();
	} else {
		show_window (quickspell_win);
		move_window (quickspell_win, -1, 0, window_width - quickspell_x, quickspell_y);
	}
}
// Quickspells end



//CAST FUNCTIONS
int spell_clear_handler()
{
	int i;

	for(i=0;i<6;i++) {
		on_cast[i]=-1;
	}

	we_have_spell=-1;
	spell_text[0]=0;
	return 1;
}

void send_spell(Uint8 *str, int len)
{
	my_tcp_send(my_socket, str, len);
	memcpy(last_spell_str, str, len);
	last_spell_len = len;
}

int action_spell_keys(Uint32 key)
{
	size_t i;
	Uint32 keys[] = {K_SPELL1, K_SPELL2, K_SPELL3, K_SPELL4, K_SPELL5, K_SPELL6,
					 K_SPELL7, K_SPELL8, K_SPELL9, K_SPELL10, K_SPELL11, K_SPELL12 };
	for (i=0; (i<sizeof(keys)/sizeof(Uint32)) & (i < num_quickbar_slots); i++)
		if(key == keys[i])
		{
			if(mqb_data[i+1] && mqb_data[i+1]->spell_str[0])
				send_spell(mqb_data[i+1]->spell_str, mqb_data[i+1]->spell_str[1]+2);
			return 1;
		}
	return 0;
}

int prepare_for_cast(){
	Uint8 str[20];
	int count=0;
	int sigils_no=0;
	int i;

	for(i=0;i<6;i++) {
		if(on_cast[i]!=-1) {
			count++;
		}
	}

	if(count<2) {
		safe_snprintf((char*)spell_text, sizeof(spell_text), "%c%s",127+c_red2,sig_too_few_sigs);
		have_error_message=1;
		return 0;
	}

	str[0]=CAST_SPELL;
	for(i=0;i<6;i++) {
		if(on_cast[i]!=-1){
			str[sigils_no+2]=on_cast[i];
			sigils_no++;
		}
	}

	str[1]=sigils_no;

	if(!mqb_data[0]) {
		mqb_data[0]=(mqbdata*)calloc(1,sizeof(mqbdata));
		mqb_data[0]->spell_id=-1;
	}

	if(sigil_win!=sigils_win&&we_have_spell>=0){
		mqb_data[0]->spell_id=spells_list[we_have_spell].id;
		mqb_data[0]->spell_image=spells_list[we_have_spell].image;
		memcpy(mqb_data[0]->spell_name, spells_list[we_have_spell].name, 60);
	}

	memcpy(mqb_data[0]->spell_str, str, sigils_no+2);//Copy the last spell send to the server
	return sigils_no;
}

int cast_handler()
{
	//Cast?

	int sigils_no=prepare_for_cast();
	//ok, send it to the server...
	if(sigils_no) send_spell(mqb_data[0]->spell_str, sigils_no+2);
	return 1;
}



//Calc windows size based on xml data
void calc_spell_windows(){
	
	int i,gy=0,y;
	//calc spell_win
	for(i=0;i<num_groups;i+=2)
		gy+=MAX(33*(groups_list[i].spells/(SPELLS_ALIGN_X+1)+1)+20,33*(groups_list[i+1].spells/(SPELLS_ALIGN_X+1)+1)+20);
	spell_x_len = SPELLS_ALIGN_X*33*2+33+50;
	spell_y_len = 10+gy+50+15+37;
	spell_y_len_ext = spell_y_len+35;

	y=10;
	for(i=0;i<num_groups;i++){

		groups_list[i].x=20;
		groups_list[i].y=y+15;
		if(i==num_groups-1) groups_list[i].x+=((2*33*SPELLS_ALIGN_X+33)-(33*SPELLS_ALIGN_X))/2; //if groups are odd, last one is drawn in the middle
		
		i++;
		if(i>=num_groups) break;
		groups_list[i].x+=20+33+33*SPELLS_ALIGN_X;
		groups_list[i].y=y+15;
		y+=20+33*MAX(groups_list[i-1].spells/(SPELLS_ALIGN_X+1)+1,groups_list[i].spells/(SPELLS_ALIGN_X+1)+1);
	}		

	//calc spell_mini_win
	spell_mini_rows=0;
	for(i=0;i<num_groups;i++)
		spell_mini_rows+=groups_list[i].spells/(SPELLS_ALIGN_X+1)+1;
	spell_mini_x_len=SPELLS_ALIGN_X*33+50;
	spell_mini_y_len=10+33*spell_mini_rows+20+30+37;
}


//Create and show/hide our windows
void display_sigils_menu()
{
	static int checked_reagents = 0;
	if (!checked_reagents) {
		if (item_info_available()) {
			int i, j;
			// check item ids/uid all give unique items
			for (i = 0; i < SPELLS_NO; i++)
				for(j=0;j<4;j++)
					if (spells_list[i].reagents_id[j] >= 0)
						if (get_item_count(spells_list[i].reagents_uid[j], spells_list[i].reagents_id[j]) != 1)
							LOG_ERROR("Invalid spell.xml reagents spells_list[%d].reagents_uid[%d]=%d spells_list[%d].reagents_id[%d]=%d\n",
								i, j, spells_list[i].reagents_uid[j], i, j, spells_list[i].reagents_id[j]);
		}
		checked_reagents = 1;
	}

	calc_spell_windows();
	if(sigils_win < 0){
		//create sigil win
		static int cast_button_id=100;
		static int clear_button_id=101;
		widget_list *w_cast = NULL;
		widget_list *w_clear = NULL;
		int but_space = 0;

		int our_root_win = -1;

		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		sigils_win= create_window(win_sigils, our_root_win, 0, sigil_menu_x, sigil_menu_y, sigil_x_len, sigil_y_len, ELW_WIN_DEFAULT);

		set_window_handler(sigils_win, ELW_HANDLER_DISPLAY, &display_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_CLICK, &click_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_MOUSEOVER, &mouseover_sigils_handler );

		cast_button_id=button_add_extended(sigils_win, cast_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, cast_str);
		widget_set_OnClick(sigils_win, cast_button_id, cast_handler);

		clear_button_id=button_add_extended(sigils_win, clear_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, clear_str);
		widget_set_OnClick(sigils_win, clear_button_id, spell_clear_handler);

		w_cast = widget_find(sigils_win, cast_button_id);
		w_clear = widget_find(sigils_win, clear_button_id);
		but_space = (sigil_x_len - (33*6+5) - w_cast->len_x - w_clear->len_x)/3;
		widget_move(sigils_win, cast_button_id, 33*6+5 + but_space+5, sigil_y_len - w_cast->len_y - 4);
		widget_move(sigils_win, clear_button_id, w_cast->pos_x + w_cast->len_x + but_space, sigil_y_len - w_clear->len_y - 4);
		hide_window(sigils_win);	
	}

	if(spell_win < 0){
		//create spell win		
		static int cast2_button_id=102;
		widget_list *w_cast = NULL;
		int our_root_win = -1;

		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		spell_win= create_window("Spells", our_root_win, 0, sigil_menu_x, sigil_menu_y, spell_x_len, spell_y_len_ext, ELW_WIN_DEFAULT);

		set_window_handler(spell_win, ELW_HANDLER_DISPLAY, &display_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_CLICK, &click_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_MOUSEOVER, &mouseover_spells_handler );

		
		cast2_button_id=button_add_extended(spell_win, cast2_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, cast_str);
		widget_set_OnClick(spell_win, cast2_button_id, cast_handler);	
		w_cast = widget_find(spell_win, cast2_button_id);
		widget_move(spell_win, cast2_button_id, spell_x_len-20-10-w_cast->len_x , spell_y_len_ext - w_cast->len_y - 4);

		hide_window(spell_win);
		if(!start_mini_spells) sigil_win=spell_win;
	} 

	if(spell_mini_win < 0){
		//create mini spell win		
		int our_root_win = -1;

		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		spell_mini_win= create_window("Spells", our_root_win, 0, sigil_menu_x, sigil_menu_y, spell_mini_x_len, spell_mini_y_len, ELW_WIN_DEFAULT);

		set_window_handler(spell_mini_win, ELW_HANDLER_DISPLAY, &display_spells_mini_handler );
		set_window_handler(spell_mini_win, ELW_HANDLER_CLICK, &click_spells_mini_handler );
		set_window_handler(spell_mini_win, ELW_HANDLER_MOUSEOVER, &mouseover_spells_mini_handler );

		hide_window(spell_mini_win);	
		if(start_mini_spells) sigil_win=spell_mini_win;
	} 
	check_castability();
	switch_handler((init_ok) ? (sigil_win):(sigils_win));
}


void init_sigils(){
	int i;

	i=0;

	// TODO: load this data from a file
	sigils_list[i].sigil_img=0;
	my_strcp(sigils_list[i].name,(char*)sig_change.str);
	my_strcp(sigils_list[i].description,(char*)sig_change.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=1;
	my_strcp(sigils_list[i].name,(char*)sig_restore.str);
	my_strcp(sigils_list[i].description,(char*)sig_restore.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=2;
	my_strcp(sigils_list[i].name,(char*)sig_space.str);
	my_strcp(sigils_list[i].description,(char*)sig_space.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=3;
	my_strcp(sigils_list[i].name,(char*)sig_increase.str);
	my_strcp(sigils_list[i].description,(char*)sig_increase.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=4;
	my_strcp(sigils_list[i].name,(char*)sig_decrease.str);
	my_strcp(sigils_list[i].description,(char*)sig_decrease.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=5;
	my_strcp(sigils_list[i].name,(char*)sig_temp.str);
	my_strcp(sigils_list[i].description,(char*)sig_temp.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=6;
	my_strcp(sigils_list[i].name,(char*)sig_perm.str);
	my_strcp(sigils_list[i].description,(char*)sig_perm.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=7;
	my_strcp(sigils_list[i].name,(char*)sig_move.str);
	my_strcp(sigils_list[i].description,(char*)sig_move.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=8;
	my_strcp(sigils_list[i].name,(char*)sig_local.str);
	my_strcp(sigils_list[i].description,(char*)sig_local.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=9;
	my_strcp(sigils_list[i].name,(char*)sig_global.str);
	my_strcp(sigils_list[i].description,(char*)sig_global.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=10;
	my_strcp(sigils_list[i].name,(char*)sig_fire.str);
	my_strcp(sigils_list[i].description,(char*)sig_fire.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=11;
	my_strcp(sigils_list[i].name,(char*)sig_water.str);
	my_strcp(sigils_list[i].description,(char*)sig_water.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=12;
	my_strcp(sigils_list[i].name,(char*)sig_air.str);
	my_strcp(sigils_list[i].description,(char*)sig_air.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=13;
	my_strcp(sigils_list[i].name,(char*)sig_earth.str);
	my_strcp(sigils_list[i].description,(char*)sig_earth.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=14;
	my_strcp(sigils_list[i].name,(char*)sig_spirit.str);
	my_strcp(sigils_list[i].description,(char*)sig_spirit.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=15;
	my_strcp(sigils_list[i].name,(char*)sig_matter.str);
	my_strcp(sigils_list[i].description,(char*)sig_matter.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=16;
	my_strcp(sigils_list[i].name,(char*)sig_energy.str);
	my_strcp(sigils_list[i].description,(char*)sig_energy.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=17;
	my_strcp(sigils_list[i].name,(char*)sig_magic.str);
	my_strcp(sigils_list[i].description,(char*)sig_magic.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=18;
	my_strcp(sigils_list[i].name,(char*)sig_destroy.str);
	my_strcp(sigils_list[i].description,(char*)sig_destroy.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=19;
	my_strcp(sigils_list[i].name,(char*)sig_create.str);
	my_strcp(sigils_list[i].description,(char*)sig_create.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=20;
	my_strcp(sigils_list[i].name,(char*)sig_knowledge.str);
	my_strcp(sigils_list[i].description,(char*)sig_knowledge.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=21;
	my_strcp(sigils_list[i].name,(char*)sig_protection.str);
	my_strcp(sigils_list[i].description,(char*)sig_protection.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=22;
	my_strcp(sigils_list[i].name,(char*)sig_remove.str);
	my_strcp(sigils_list[i].description,(char*)sig_remove.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=23;
	my_strcp(sigils_list[i].name,(char*)sig_health.str);
	my_strcp(sigils_list[i].description,(char*)sig_health.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=24;
	my_strcp(sigils_list[i].name,(char*)sig_life.str);
	my_strcp(sigils_list[i].description,(char*)sig_life.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=25;
	my_strcp(sigils_list[i].name,(char*)sig_death.str);
	my_strcp(sigils_list[i].description,(char*)sig_death.desc);
	sigils_list[i].have_sigil=1;

}

