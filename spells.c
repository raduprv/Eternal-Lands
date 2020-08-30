#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spells.h"
#include "asc.h"
#include "cursors.h"
#include "context_menu.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_quickspells_window.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "colors.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "pathfinder.h"
#include "textures.h"
#include "translate.h"
#include "counters.h"
#include "errors.h"
#include "io/elpathwrapper.h"
#include "sound.h"

#define SIGILS_NO 64
#define MAX_SIGILS 6
#define	NUM_SIGILS_LINE	12	// how many sigils per line displayed
#define	NUM_SIGILS_ROW	3	// how many rows of sigils are there?
#define SIGILS_NO 64
#define SPELLS_NO 32
#define GROUPS_NO 8
#define TEXTBUFSIZE 256

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

static sigil_def sigils_list[SIGILS_NO];
int sigils_text;

typedef struct {
	int id;//The spell server id
	char name[60];//The spell name
	char desc[120];//The spell description
	int image;//image_id
	int sigils[MAX_SIGILS];//index of required sigils in sigils_list
	int mana;//required mana
	attrib_16 *lvls[NUM_WATCH_STAT];//pointers to your_info lvls
	int lvls_req[NUM_WATCH_STAT];//minimum lvls requirement
	int reagents_id[4]; //reagents needed image id
	Uint16 reagents_uid[4]; //reagents needed, unique item id
	int reagents_qt[4]; //their quantities
	Uint32 buff;
	int uncastable; //0 if castable, otherwise if something missing
} spell_info;

Uint8 last_spell_str[20];
static int last_spell_len= 0;
int spell_result=0;
int show_poison_count = 0; // elconfig variable
static spell_info spells_list[SPELLS_NO];
static int num_spells=0;
static Uint8 raw_spell_text[TEXTBUFSIZE];
static unsigned char spell_help[TEXTBUFSIZE];
static Sint8 on_cast[MAX_SIGILS];
static int have_error_message=0;
static int we_have_spell=-1; //selected spell
static int on_spell=-1;//mouse over this spell
static int poison_drop_counter = 0;

typedef struct {
	unsigned char desc[120];
	int spells;
	int spells_id[SPELLS_NO];
	int x,y;
} group_def;
static int num_groups=0;
static group_def groups_list[GROUPS_NO];

typedef struct
{
	Sint8 spell;
	Uint32 cast_time;
	Uint32 duration;
#ifdef NEW_SOUND
	unsigned int sound;
#endif
} spell_def;
static spell_def active_spells[NUM_ACTIVE_SPELLS];

//windows related
int start_mini_spells=0; //do we start minimized?
static int sigils_win=-1;
static int spell_win=-1;
static int spell_mini_win=-1;
static int last_win=-1;
static int init_ok=0;
//big window
static int spell_grid_size = 0;
static int spell_border = 0;
static int spell_text_y = 0;
static int spell_engred_y = 0;
static int cast2_button_id = 102;
//sigil window
static int sigil_grid_size = 0;
static int sigil_border = 0;
static int spell_icon_x = 0;
static int spell_icon_y = 0;
static int cast_button_id = 100;
static int clear_button_id = 101;
static int show_last_spell_help=0;
//mini window
static int spell_mini_rows=0;
static int spell_mini_grid_size = 0;
static int spell_mini_border = 0;
//active icons
static int active_spells_size = 32;
static int active_spells_offset = 64;

/* spell duration state */
static Uint16 requested_durations = 0;
static Uint16 last_requested_duration = 0;
static size_t buff_duration_colour_id = 0;

/* mapping of spell buff value from spells.xml to buff bit-masks */
typedef struct buff_buffmask {
	Uint32 buff;
	Uint16 buffmask;
} buff_buffmask;
static buff_buffmask buff_to_buffmask[NUM_BUFFS] = {
		{11, BUFF_INVISIBILITY},
		{3, BUFF_MAGIC_IMMUNITY},
		{1, BUFF_MAGIC_PROTECTION},
		{23, BUFF_COLD_SHIELD},
		{24, BUFF_HEAT_SHIELD},
		{25, BUFF_RADIATION_SHIELD},
		{0, BUFF_SHIELD},
		{7, BUFF_TRUE_SIGHT},
		{5, BUFF_ACCURACY},
		{6, BUFF_EVASION},
		{0xFFFFFFFF, BUFF_DOUBLE_SPEED}
	};

/* display debug information about buff durations */
#if defined(BUFF_DURATION_DEBUG)
static void duration_debug(int buff, int duration, const char*message)
{
	size_t i;
	char buf[128];
	const char *buff_name = "Unknown";
	if (buff == 5)
		buff_name = "Accuracy";
	else if (buff == 6)
		buff_name = "Evasion";
	else
		for (i=0; i<SPELLS_NO; i++)
			if (spells_list[i].buff == buff)
			{
				buff_name = spells_list[i].name;
				break;
			}
	safe_snprintf(buf, sizeof(buf), "Debug: Buff [%s] %s: %d seconds", buff_name, message, duration, message);
	LOG_TO_CONSOLE (c_red1, buf);
}
#endif

/* Called when the client receives SEND_BUFF_DURATION from server.
 * Set the duration and start the time out for the buff duration.
*/
void here_is_a_buff_duration(Uint8 duration)
{
	/* check the request is on the queue */
	if (requested_durations & last_requested_duration)
	{
		size_t i;
		Uint32 buff = 0xFFFFFFFF;

		/* get the spell / buff value from the bit-mask we used */
		for (i=0; i<NUM_BUFFS; i++)
			if (last_requested_duration == buff_to_buffmask[i].buffmask)
			{
				buff = buff_to_buffmask[i].buff;
				break;
			}

		/* if we have a matching spell, set the duration information */
		for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
		{
			if ((active_spells[i].spell != -1) && (buff == active_spells[i].spell))
			{
				active_spells[i].cast_time = get_game_time_sec();
				active_spells[i].duration = (Uint32)duration;
#if defined(BUFF_DURATION_DEBUG)
				duration_debug(buff, active_spells[i].duration, "duration from server");
#endif
				break;
			}
		}

		/* clear request */
		requested_durations &= ~last_requested_duration;
		last_requested_duration = 0;
	}

	/* to save waiting, process others in the queue now */
	check_then_do_buff_duration_request();
}


/* Called periodically from the main loop
 * Time out any old requests.
 * If no request is pending but we have one in the queue, ask the server for the duration.
*/
void check_then_do_buff_duration_request(void)
{
	static Uint32 last_request_time = 0;

	/* wait until the client knows the game time fully */
	if (!is_real_game_second_valid())
		return;

	/* stop waiting for server response after 10 seconds, clear all other requests */
	if (last_requested_duration && (SDL_GetTicks() - last_request_time) > 10000)
	{
		last_requested_duration = 0;
		requested_durations = 0;
	}

	/* else if there is no active request but we have one queued, make the server request */
	else if (!last_requested_duration && requested_durations)
	{
		Uint8 str[4];

		last_requested_duration = 1;
		while (!(requested_durations & last_requested_duration))
			last_requested_duration <<= 1;
		last_request_time = SDL_GetTicks();

		str[0] = GET_BUFF_DURATION;
		*((Uint16 *)(str+1)) = SDL_SwapLE16(last_requested_duration);
		my_tcp_send (my_socket, str, 3);
	}
}

/*	Called when we receive notification that a spell is active.
 * 	If the spell is in the buff bit-mask array, queue the duration request.
*/
static void request_buff_duration(Uint32 buff)
{
	size_t i;
	for (i=0; i<NUM_BUFFS; i++)
		if (buff == buff_to_buffmask[i].buff)
		{
			requested_durations |= buff_to_buffmask[i].buffmask;
			check_then_do_buff_duration_request();
			return;
		}
}





static int cast_handler(void);
static int prepare_for_cast(void);
static void set_spell_help_text(int spell);
static void init_sigils(void);


void repeat_spell(void){
	if(last_spell_len > 0)
		my_tcp_send(my_socket, last_spell_str, last_spell_len);
}

//returns a node with tagname, starts searching from the_node
static xmlNode *get_XML_node(xmlNode *the_node, char *tagname){
	xmlNode *node=the_node;

	while(node) {
		if(node->type==XML_ELEMENT_NODE && xmlStrcasecmp (node->name, (xmlChar*)tagname) == 0) return node;
		else node=node->next;
	}
	return node;
}


static attrib_16 *get_skill_address(const char *skillname)
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

static int put_on_cast(void){
	if(we_have_spell>=0){
		int i;
		for(i=0;i<MAX_SIGILS;i++)
			if(spells_list[we_have_spell].sigils[i]>=0)
				if(!sigils_list[spells_list[we_have_spell].sigils[i]].have_sigil) {
					//we miss at least a sigil, clear on_cast
					int j;
					for(j=0;j<MAX_SIGILS;j++) on_cast[j]=-1;
					return 0;
				}
		for(i=0;i<MAX_SIGILS;i++) on_cast[i]=spells_list[we_have_spell].sigils[i];
		return 1;
	}
	return 0;
}

int init_spells (void)
{
	int i,j;
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;
	char *fname="./spells.xml";

	buff_duration_colour_id = elglGetColourId("buff.duration.background");

	//init textures and structs
	sigils_text = load_texture_cached("textures/sigils.dds", tt_gui);
	for (i = 0; i < SIGILS_NO; i++)
		sigils_list[i].have_sigil = 0;
	for (i = 0; i < SPELLS_NO; i++){
		spells_list[i].image = -1;
		for(j=0;j<MAX_SIGILS;j++)
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

	raw_spell_text[0]=spell_help[0]=0;
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
		const int expected_version = 1;
		int actual_version = -1;
		i = 0;

		if ((actual_version = xmlGetInt(root, "version")) < expected_version)
		{
			safe_snprintf(tmp, sizeof(tmp), "Warning: %s file is out of date expecting %d, actual %d.", fname, expected_version, actual_version);
			LOG_TO_CONSOLE (c_red1, tmp);
		}

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

			data = get_XML_node(node->children, "buff");

			if (data != 0)
			{
				spells_list[i].buff = get_int_value(data);
			}
			else
			{
				spells_list[i].buff = 0xFFFFFFFF;
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
	for (i = 0; i < MAX_SIGILS; i++)
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

void check_castability(void)
{
	int i,j,k,l;

	for(i=0;i<num_spells;i++){
		spells_list[i].uncastable=0;
		//Check Mana
		if (have_stats && your_info.ethereal_points.cur<spells_list[i].mana) spells_list[i].uncastable|=UNCASTABLE_MANA;
		//Check Sigils
		for(j=0;j<MAX_SIGILS;j++){
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
void draw_spell_icon_strings(window_info *win)
{
	size_t i;
	int x_start = 0;
	int x_sep = (int)(0.5 + win->current_scale * 33);
	int y_start = 0;

	// these are used when drawing the ative icons too
	active_spells_size = (int)(0.5 + win->current_scale * 32);
	active_spells_offset = (int)(0.5 + win->current_scale * 64);

	y_start = window_height - hud_y - active_spells_offset - win->small_font_len_y;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		unsigned char str[20];
		/* handle the poison count */
		if ((poison_drop_counter > 0) && (active_spells[i].spell == 2) && show_poison_count)
		{
			safe_snprintf((char*)str, sizeof(str), "%d", poison_drop_counter );
			draw_text(x_start+x_sep/2, y_start, str, strlen((const char*)str), win->font_category,
				TDO_SHADOW, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0,
				TDO_ZOOM, win->current_scale_small, TDO_ALIGNMENT, CENTER, TDO_END);
		}
		/* other strings on spell icons, timers perhaps .....*/
		x_start += x_sep;
	}

}

//ACTIVE SPELLS
void get_active_spell(int pos, int spell)
{
	active_spells[pos].spell = spell;
	active_spells[pos].cast_time = 0;
	request_buff_duration(spell);
#ifdef NEW_SOUND
	active_spells[pos].sound = add_spell_sound(spell);
#endif // NEW_SOUND
}

void remove_active_spell(int pos)
{
#if defined(BUFF_DURATION_DEBUG)
	if (active_spells[pos].duration > 0)
		duration_debug(active_spells[pos].spell, diff_game_time_sec(active_spells[pos].cast_time), "actual duration");
#endif
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

static void rerequest_durations(void)
{
	size_t i;
	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		if (active_spells[i].spell >= 0)
			request_buff_duration(active_spells[i].spell);
	}
}

#if defined(BUFF_DURATION_DEBUG)
int command_buff_duration(char *text, int len)
{
	LOG_TO_CONSOLE(c_green1, "Request buff durations");
	rerequest_durations();
	return 1;
}
#endif

void get_active_spell_list(const Uint8 *my_spell_list)
{
	size_t i;

	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		active_spells[i].spell = my_spell_list[i];
		active_spells[i].duration = active_spells[i].cast_time = 0;
		if (active_spells[i].spell >= 0)
			request_buff_duration(active_spells[i].spell);
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

int we_are_poisoned(void)
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

static void time_out(const float x_start, const float y_start, const float gridsize,
	const float progress)
{
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	elglColourI(buff_duration_colour_id);

	glBegin(GL_QUADS);
		glVertex2f(x_start, y_start + gridsize * progress);
		glVertex2f(x_start + gridsize, y_start + gridsize * progress);
		glVertex2f(x_start + gridsize, y_start + gridsize);
		glVertex2f(x_start, y_start + gridsize);
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void display_spells_we_have(void)
{
	Uint32 i;
	float scale, duration;

	if (your_actor != NULL)
	{
		static int last_actor_type = -1;
		if (last_actor_type < 0)
			last_actor_type = your_actor->actor_type;
		if (last_actor_type != your_actor->actor_type)
		{
			last_actor_type = your_actor->actor_type;
			rerequest_durations();
		}
	}

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

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

			x_start = (active_spells_size + 1) * cur_pos;
			y_start = window_height - hud_y - active_spells_offset;

			duration = active_spells[i].duration;

			if (duration > 0.0)
			{
				scale = diff_game_time_sec(active_spells[i].cast_time) / duration;

				if ((scale >= 0.0) && (scale <= 1.0))
				{
					time_out(x_start, y_start, active_spells_size, scale);
				}
			}

			glEnable(GL_BLEND);
			draw_spell_icon(cur_spell, x_start, y_start, active_spells_size, 0, 0);
			glDisable(GL_BLEND);
		}
	}
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}



//DISPLAY HANDLERS

static int draw_switcher(window_info *win){

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	//Draw switcher spells <-> sigils
	glBegin(GL_LINES);
		glVertex3i(win->len_x-win->box_size,2*win->box_size,0);
		glVertex3i(win->len_x,2*win->box_size,0);
		glVertex3i(win->len_x-win->box_size,2*win->box_size,0);
		glVertex3i(win->len_x-win->box_size,win->box_size,0);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3i(win->len_x-0.75*win->box_size,1.75*win->box_size,0);
		glVertex3i(win->len_x-0.25*win->box_size,1.75*win->box_size,0);
		glVertex3i(win->len_x-0.25*win->box_size,1.25*win->box_size,0);
		glVertex3i(win->len_x-0.75*win->box_size,1.25*win->box_size,0);
	glEnd();

	if (get_id_MW(MW_SPELLS) == spell_win || get_id_MW(MW_SPELLS) == spell_mini_win) {
		//Draw switcher spells <-> mini
		glBegin(GL_LINES);
			glVertex3i(win->len_x-win->box_size,3*win->box_size,0);
			glVertex3i(win->len_x,3*win->box_size,0);
			glVertex3i(win->len_x-win->box_size,3*win->box_size,0);
			glVertex3i(win->len_x-win->box_size,2*win->box_size,0);
			if(get_id_MW(MW_SPELLS) == spell_win) {
			//arrow down
					glVertex3i(win->len_x-0.75*win->box_size,2.25*win->box_size,0);
					glVertex3i(win->len_x-0.5*win->box_size,2.75*win->box_size,0);
					glVertex3i(win->len_x-0.5*win->box_size,2.75*win->box_size,0);
					glVertex3i(win->len_x-0.25*win->box_size,2.25*win->box_size,0);
			} else {
			//arrow up
					glVertex3i(win->len_x-0.75*win->box_size,2.75*win->box_size,0);
					glVertex3i(win->len_x-0.5*win->box_size,2.25*win->box_size,0);
					glVertex3i(win->len_x-0.5*win->box_size,2.25*win->box_size,0);
					glVertex3i(win->len_x-0.25*win->box_size,2.75*win->box_size,0);
			}
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	return 1;
}

void draw_spell_icon(int id,int x_start, int y_start, int gridsize, int alpha, int grayed){

	float u_start,v_start,u_end,v_end;

	u_start = 0.125f * (id % 8);
	v_start = 0.125f * (id / 8);
	u_end = u_start + 0.125f;
	v_end = v_start + 0.125f;

	bind_texture(sigils_text);
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

static void draw_current_spell(window_info *win, int x, int y, int sigils_too, int grid_size){

	int start_x = x;
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	if(we_have_spell>=0){
		int i,j;
		unsigned char str[4];
		//we have a current spell (cliked or casted) !!mqb_data[0] can still be null!!
		j=we_have_spell;
		draw_spell_icon(spells_list[j].image,x,y,grid_size-1,1,0);

		if(sigils_too){
			//draw sigils
			x+=grid_size*2;
			for(i=0;i<MAX_SIGILS;i++){
				if (spells_list[j].sigils[i]<0) break;
				draw_spell_icon(spells_list[j].sigils[i],x+grid_size*i,y,grid_size-1,0,spells_list[j].uncastable&UNCASTABLE_SIGILS);
				if(spells_list[j].uncastable&UNCASTABLE_SIGILS&&!sigils_list[spells_list[j].sigils[i]].have_sigil) gray_out(x+grid_size*i,y,grid_size-1);
			}
		}

		//draw reagents
		x+= (sigils_too) ? (grid_size*MAX_SIGILS+grid_size):(grid_size*1.5);
		for(i=0;spells_list[j].reagents_id[i]>0;i++) {
			draw_item(spells_list[j].reagents_id[i],x+grid_size*i,y,grid_size);
			safe_snprintf((char *)str, sizeof(str), "%i",spells_list[j].reagents_qt[i]);
			draw_string_small_shadowed_zoomed(x+grid_size*i, y+grid_size*0.5, (unsigned char*)str, 1,1.0f,1.0f,1.0f, 0.0f, 0.0f, 0.0f, win->current_scale);
			if(spells_list[j].uncastable&UNCASTABLE_REAGENTS) gray_out(x+grid_size*i,y+1,grid_size-1);
		}
		//draw mana
		x+=(sigils_too) ? (grid_size*5):(grid_size*4+grid_size*0.5);
		safe_snprintf((char *)str, sizeof(str), "%i",spells_list[j].mana);
		if (spells_list[j].uncastable&UNCASTABLE_MANA) glColor3f(1.0f,0.0f,0.0f);
		else glColor3f(0.0,1.0,0.0);
		j = (grid_size - win->default_font_len_y)/2;
		draw_string_zoomed_centered(x+grid_size/2,y+j,str,1, win->current_scale);
	}

	//draw strings
	x=start_x;
	glColor3f(0.77f,0.57f,0.39f);
	if(sigils_too) {
		x+=grid_size*2;
		draw_string_small_zoomed(x, y - win->small_font_len_y, (unsigned char*)"Sigils", 1, win->current_scale);
		x+=grid_size*MAX_SIGILS+grid_size;
	} else x += grid_size * 1.5;

	draw_string_small_zoomed(x, y - win->small_font_len_y, (unsigned char*)"Reagents", 1, win->current_scale);
	x+=grid_size*4+((sigils_too) ? (grid_size):(grid_size*0.5));
	draw_string_small_zoomed(x, y - win->small_font_len_y, (unsigned char*)"Mana", 1, win->current_scale);

	//draw grids
	glDisable(GL_TEXTURE_2D);
	x=start_x;
	if(sigils_too) {
		x+=grid_size*2;
		rendergrid (MAX_SIGILS, 1, x, y, grid_size, grid_size);
		x+=grid_size*MAX_SIGILS+grid_size;
	} else x+=grid_size*1.5;
	rendergrid (4, 1, x, y, grid_size, grid_size);
	x+=grid_size*4+((sigils_too) ? (grid_size):(grid_size*0.5));
	rendergrid (1, 1, x, y, grid_size, grid_size);
}

static int display_sigils_handler(window_info *win)
{
	int i;
	int x_start,y_start;
	Uint8 spell_text_buf[TEXTBUFSIZE];

	if (init_ok) draw_switcher(win);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);

	//let's add the new spell icon if we have one
	if(mqb_data[0] && mqb_data[0]->spell_id!=-1)
		draw_spell_icon(mqb_data[0]->spell_image, spell_icon_x, spell_icon_y, sigil_grid_size - 1, 1, 0);

	//ok, now let's draw the objects...
	for(i=0;i<SIGILS_NO;i++){
		if(sigils_list[i].have_sigil){
			//get the x and y
			x_start=sigil_grid_size*(i%NUM_SIGILS_LINE)+1;
			y_start=sigil_grid_size*(i/NUM_SIGILS_LINE);
			draw_spell_icon(sigils_list[i].sigil_img,x_start,y_start,sigil_grid_size-1,0,0);
		}
	}

	//ok, now let's draw the sigils on the list
	for(i=0;i<MAX_SIGILS;i++)
	{
		if(on_cast[i]!=-1)
		{
			//get the x and y
			x_start = sigil_grid_size * (i % MAX_SIGILS) + sigil_border;
			y_start = win->len_y - sigil_grid_size - sigil_border - 1;
			draw_spell_icon(on_cast[i],x_start,y_start,sigil_grid_size-1,0,0);
		}
	}

	//now, draw the inventory text, if any.
	put_small_text_in_box_zoomed(raw_spell_text, strlen((char *)raw_spell_text), win->len_x-2*sigil_border, spell_text_buf, win->current_scale);
	draw_string_small_zoomed(sigil_border, NUM_SIGILS_ROW * sigil_grid_size + win->small_font_len_y / 2, spell_text_buf, 4, win->current_scale);

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	rendergrid (NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, sigil_grid_size, sigil_grid_size);
	rendergrid (MAX_SIGILS, 1, sigil_border, win->len_y - sigil_grid_size - sigil_border - 1, sigil_grid_size, sigil_grid_size);

	glEnable(GL_TEXTURE_2D);

	if (show_last_spell_help && mqb_data[0] && mqb_data[0]->spell_id!=-1)
	{
		show_help_colored_scaled_right((const unsigned char*)mqb_data[0]->spell_name,
			spell_icon_x, spell_icon_y + (sigil_grid_size - win->small_font_len_y) / 2,
			1.0f, 1.0f, 1.0f, win->current_scale_small);
	}
	show_last_spell_help=0;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}



static int display_spells_handler(window_info *win){

	int i,j,k,x,y;
	Uint8 spell_text_buf[TEXTBUFSIZE];

	draw_switcher(win);

	//Draw spell groups
	for(i=0;i<num_groups;i++){
		x=groups_list[i].x;
		y=groups_list[i].y;
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f,1.0f,1.0f);
		draw_string_small_zoomed(x, y-win->small_font_len_y, groups_list[i].desc, 1, win->current_scale);
		for(k=0,j=0;j<groups_list[i].spells;j++){
			draw_spell_icon(spells_list[groups_list[i].spells_id[j]].image,
					x+spell_grid_size*(k%SPELLS_ALIGN_X),
					y+spell_grid_size*(k/SPELLS_ALIGN_X),spell_grid_size-1,
					0,spells_list[groups_list[i].spells_id[j]].uncastable);
			k++;
		}
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.77f,0.57f,0.39f);
		rendergrid(SPELLS_ALIGN_X,groups_list[i].spells/(SPELLS_ALIGN_X+1)+1,x,y,spell_grid_size,spell_grid_size);
	}

	glEnable(GL_TEXTURE_2D);

	//draw spell text & help
	glColor3f(1.0f,1.0f,1.0f);
	put_small_text_in_box_zoomed(raw_spell_text, strlen((char *)raw_spell_text), win->len_x-2*spell_border, spell_text_buf, win->current_scale);
	draw_string_small_zoomed(spell_border, spell_text_y, spell_text_buf, 3, win->current_scale);
	draw_string_small_zoomed(spell_border, spell_engred_y + spell_grid_size + spell_border, spell_help, 2, win->current_scale);

	//draw the bottom bar
	draw_current_spell(win, spell_border, spell_engred_y, 1, spell_grid_size);
	if(we_have_spell>=0&&spells_list[we_have_spell].uncastable){
		//not castable
		glColor3f(1.0f,0.0f,0.0f);
		rendergrid(1, 1, spell_border, spell_engred_y, spell_grid_size, spell_grid_size);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


static int display_spells_mini_handler(window_info *win)
{
	int i,j,cg,cs;
	int x = spell_mini_border;
	int y = spell_mini_border;

	draw_switcher(win);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	for (i=0,cs=0,cg=0;i<spell_mini_rows;i++)
		for (j=0;j<SPELLS_ALIGN_X;j++){
			if (cs==groups_list[cg].spells) {cs=0; cg++; break;}
			draw_spell_icon(spells_list[groups_list[cg].spells_id[cs]].image,
					x + j * spell_mini_grid_size, y + spell_mini_grid_size * i, spell_mini_grid_size - 1,
					0,spells_list[groups_list[i].spells_id[j]].uncastable);
			cs++;
		}

	//draw spell help
	if(on_spell==-2 && spells_list[we_have_spell].uncastable) {
		//mouse over the bottom-left selected spell icon, show uncastability
		int l = get_string_width_zoom((const unsigned char*)GET_UNCASTABLE_STR(spells_list[we_have_spell].uncastable),
			win->font_category, win->current_scale_small);
		SET_COLOR(c_red2);
		draw_string_small_zoomed(spell_mini_border + (spell_mini_grid_size * SPELLS_ALIGN_X - l) / 2,
			win->len_y - spell_mini_grid_size - spell_mini_border  - 2.5 * win->small_font_len_y,
			(unsigned char*)GET_UNCASTABLE_STR(spells_list[we_have_spell].uncastable),1, win->current_scale);
	} else {
		i=(on_spell>=0) ? (on_spell):(we_have_spell);
		if(i>=0){
			int l = get_string_width_zoom((unsigned char*)spells_list[i].name,
				win->font_category, win->current_scale_small);
			if (on_spell>=0) SET_COLOR(c_grey1);
			else SET_COLOR(c_green3);
			draw_string_small_zoomed(spell_mini_border + (spell_mini_grid_size * SPELLS_ALIGN_X - l) / 2,
				win->len_y - spell_mini_grid_size - spell_mini_border - 2.5 * win->small_font_len_y,
				(unsigned char*)spells_list[i].name, 1, win->current_scale);
		}
	}

	//draw the current spell
	draw_current_spell(win, x, win->len_y - spell_mini_grid_size - spell_mini_border, 0, spell_mini_grid_size);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	rendergrid(SPELLS_ALIGN_X, spell_mini_rows, x, y, spell_mini_grid_size, spell_mini_grid_size);

	if(we_have_spell>=0&&spells_list[we_have_spell].uncastable){
		//not castable, red grid
		glColor3f(1.0f,0.0f,0.0f);
		rendergrid(1, 1, spell_mini_border, win->len_y - spell_mini_grid_size - spell_mini_border, spell_mini_grid_size, spell_mini_grid_size);
	}

	glEnable(GL_TEXTURE_2D);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}



//CLICK HANDLERS
static int switch_handler(int new_win){
	window_info *win;
	int this_win;

	last_win = get_id_MW(MW_SPELLS);
	this_win=new_win;

	win=&windows_list.window[last_win];
	windows_list.window[this_win].opaque=windows_list.window[last_win].opaque;
	hide_window(last_win);
	move_window(this_win, win->pos_id, win->pos_loc, win->pos_x, win->pos_y);
	show_window(this_win);
	select_window(this_win);
	set_id_MW(MW_SPELLS, this_win);
	start_mini_spells=(this_win == spell_mini_win)? 1:0;

	return 1;
}


static int click_switcher_handler(window_info *win, int mx, int my, Uint32 flags){
	int sigil_win = get_id_MW(MW_SPELLS);
	if (mx>=win->len_x-win->box_size&&my>=win->box_size&&my<=2*win->box_size) {
		do_click_sound();
		switch_handler((sigil_win==sigils_win) ? (last_win):(sigils_win));
	} else if(sigil_win==spell_win || sigil_win==spell_mini_win){
		if (mx>=win->len_x-win->box_size&&my>=2*win->box_size&&my<=3*win->box_size) {
			do_click_sound();
			switch_handler((sigil_win==spell_win) ? (spell_mini_win):(spell_win));
		}
	}
	return 0;
}


static int click_sigils_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// only handle real clicks, not scroll wheel moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0 ) {
		return 0;
	} else if(mx>=spell_icon_x && mx<=spell_icon_x+sigil_grid_size &&
			my>=spell_icon_y && my<=spell_icon_y+sigil_grid_size && mqb_data[0] && mqb_data[0]->spell_id!=-1) {
		add_quickspell();
		return 1;
	} else if(mx>0 && mx<NUM_SIGILS_LINE*sigil_grid_size && my>0 && my<NUM_SIGILS_ROW*sigil_grid_size) {
		int pos=get_mouse_pos_in_grid(mx,my, NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, sigil_grid_size, sigil_grid_size);

		if (pos >= 0 && sigils_list[pos].have_sigil) {
			int j;
			int image_id=sigils_list[pos].sigil_img;

			//see if it is already on the list
			for(j=0;j<MAX_SIGILS;j++) {
				if(on_cast[j]==image_id) {
					return 1;
				}
			}

			for(j=0;j<MAX_SIGILS;j++) {
				if(on_cast[j]==-1) {
					on_cast[j]=image_id;
					return 1;
				}
			}
			return 1;
		}
	} else if(mx>sigil_border && mx<MAX_SIGILS*sigil_grid_size+sigil_border && my>win->len_y-sigil_grid_size-sigil_border-1 && my<win->len_y-sigil_border) {
		int pos=get_mouse_pos_in_grid(mx, my, MAX_SIGILS, 1, sigil_border, win->len_y-sigil_grid_size-sigil_border-1, sigil_grid_size, sigil_grid_size);

		if (pos >= 0) {
			on_cast[pos]=-1;
		}
	}
	if (init_ok) click_switcher_handler(win,mx,my,flags);
	return 0;
}

static int click_spells_handler(window_info *win, int mx, int my, Uint32 flags){
	int pos,i,the_group=-1,the_spell=-1;
	static int last_clicked=0;
	static int last_pos=-1;

	if (!(flags & ELW_MOUSE_BUTTON)) return 0;

	for(i=0;i<num_groups;i++){
		pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, groups_list[i].spells/(SPELLS_ALIGN_X+1)+1,
			groups_list[i].x, groups_list[i].y, spell_grid_size, spell_grid_size);
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
		if(we_have_spell >= 0 && mx > spell_border && mx < spell_border + spell_grid_size &&
				my > spell_engred_y && my < spell_engred_y + spell_grid_size) {
			if(flags & ELW_LEFT_MOUSE) {
				//cast spell
				if (put_on_cast()) cast_handler();
			} else if (flags & ELW_RIGHT_MOUSE) {
				//add to quickspells
				if(put_on_cast()) {
					prepare_for_cast();
					add_quickspell();
				}
			}
		} else click_switcher_handler(win,mx,my,flags);
	}
	last_clicked = SDL_GetTicks();
	return 0;
}


static int click_spells_mini_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	static int last_clicked=0;
	static int last_pos=-1;

	if (!(flags & ELW_MOUSE_BUTTON)) return 0;

	pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, spell_mini_rows, spell_mini_border, spell_mini_border, spell_mini_grid_size, spell_mini_grid_size);
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
		if(we_have_spell>=0 && mx>=spell_mini_border && mx<=spell_mini_border+spell_mini_grid_size &&
				my>=win->len_y-spell_mini_grid_size-spell_mini_border && my<=win->len_y-spell_mini_border) {
			if(flags & ELW_LEFT_MOUSE) {
				if (put_on_cast()) cast_handler();
			} else if (flags & ELW_RIGHT_MOUSE) {
				//add to quickspells
				if(put_on_cast()){
					prepare_for_cast();
					add_quickspell();
				}
			}
		} else click_switcher_handler(win,mx,my,flags);
	}
	last_pos=pos;
	last_clicked = SDL_GetTicks();
	return 0;
}



//MOUSEOVER HANDLERS
static int mouseover_sigils_handler(window_info *win, int mx, int my)
{
	if(!have_error_message) {
		raw_spell_text[0] = 0;
	}

	if(mx>=spell_icon_x && mx<=spell_icon_x+sigil_grid_size &&
		my>=spell_icon_y && my<=spell_icon_y+sigil_grid_size &&mqb_data[0] &&mqb_data[0]->spell_name[0]) {
		show_last_spell_help = 1;
	}

	//see if we clicked on any sigil in the main category
	if(mx>0 && mx<NUM_SIGILS_LINE*sigil_grid_size && my>0 && my<NUM_SIGILS_ROW*sigil_grid_size) {
		int pos=get_mouse_pos_in_grid(mx,my, NUM_SIGILS_LINE, NUM_SIGILS_ROW, 0, 0, sigil_grid_size, sigil_grid_size);

		if (pos >= 0 && sigils_list[pos].have_sigil)
		{
			my_strcp((char*)raw_spell_text,sigils_list[pos].name);
			have_error_message=0;
		}
		return 0;
	}

	//see if we clicked on any sigil from "on cast"
	if(mx>sigil_border && mx<MAX_SIGILS*sigil_grid_size+sigil_border && my>win->len_y-sigil_grid_size-sigil_border-1 && my<win->len_y-sigil_border) {
		int pos=get_mouse_pos_in_grid(mx, my, MAX_SIGILS, 1, sigil_border, win->len_y-sigil_grid_size-sigil_border-1, sigil_grid_size, sigil_grid_size);

		if (pos >= 0 && on_cast[pos]!=-1){
			my_strcp((char*)raw_spell_text,sigils_list[on_cast[pos]].name);
			have_error_message=0;
		}
		return 0;
	}

	if(mx>=spell_icon_x && mx<=spell_icon_x+sigil_grid_size &&
		my>=spell_icon_y && my<=spell_icon_y+sigil_grid_size && mqb_data[0] && mqb_data[0]->spell_id != -1) {
		safe_snprintf((char*)raw_spell_text, sizeof(raw_spell_text), "Click to add the quick spells bar");
		return 0;
	}

	return 0;
}


static void set_spell_help_text(int spell){

	unsigned char clr[4];

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
		safe_strcat((char*)spell_help,(char*)clr,sizeof(spell_help));
		safe_strcat((char*)spell_help,GET_UNCASTABLE_STR(spells_list[spell].uncastable),sizeof(spell_help));
	}
	safe_strcat((char*)spell_help,"\n",sizeof(spell_help));
	clr[0]=127+c_grey1;
	clr[1]=0;
	safe_strcat((char*)spell_help,(char*)clr,sizeof(spell_help));
	safe_strcat((char*)spell_help,spells_list[spell].desc,sizeof(spell_help));

}

static int mouseover_spells_handler(window_info *win, int mx, int my){
	int i,pos;

	if(!have_error_message) {
		raw_spell_text[0] = 0;
	}

	on_spell=-1;
	for(i=0;i<num_groups;i++){
		pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, groups_list[i].spells/(SPELLS_ALIGN_X+1)+1,
			groups_list[i].x, groups_list[i].y, spell_grid_size, spell_grid_size);
		if(pos>=0&&pos<groups_list[i].spells) {
			on_spell=groups_list[i].spells_id[pos];
			set_spell_help_text(on_spell);
			return 0;
		}
	}
	set_spell_help_text(we_have_spell);
	//check spell icon
	if(mx > spell_border && mx < spell_border + spell_grid_size &&
			my > spell_engred_y && my < spell_engred_y + spell_grid_size && we_have_spell >= 0) {
		safe_snprintf((char*)raw_spell_text, sizeof(raw_spell_text), "Left click to cast\nRight click to add the quick spells bar");
		elwin_mouse = CURSOR_WAND;
		have_error_message=0;
		return 1;
	}
	return 0;
}


static int mouseover_spells_mini_handler(window_info *win, int mx, int my)
{
	int pos=get_mouse_pos_in_grid(mx,my, SPELLS_ALIGN_X, spell_mini_rows, spell_mini_border, spell_mini_border, spell_mini_grid_size, spell_mini_grid_size);
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
	} else if(mx > spell_mini_border && mx < spell_mini_border + spell_mini_grid_size &&
				my > win->len_y - spell_mini_grid_size - spell_mini_border && my < win->len_y - spell_mini_border && we_have_spell >= 0) {
		//check spell icon
		elwin_mouse = CURSOR_WAND;
		on_spell=-2; //draw uncastability reason
		return 1;
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


static int have_spell_name(int spell_id)
{
	int i;

	for(i=1;i<MAX_QUICKSPELL_SLOTS+1;i++){
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

	for (i = 0; i < MAX_QUICKSPELL_SLOTS+1; i++)
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
			request_buff_duration(spell);
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
			set_gamewin_wand_action();
			break;
		case S_SELECT_TELE_LOCATION://spell_result==2
			// we're about to teleport, don't let the pathfinder
			// interfere with our destination
			if (pf_follow_path) pf_destroy_path ();
			spell_result=2;
			set_gamewin_wand_action();
			break;
		case S_SUCCES://spell_result==1
			spell_result=1;
			clear_gamewin_wand_action();
			spell_cast(data[1]);
			break;
		case S_FAILED:
			spell_result=0;
			clear_gamewin_wand_action();
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

mqbdata* build_quickspell_data(const Uint32 spell_id)
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

	for (i = 0; i < MAX_SIGILS; i++)
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


//CAST FUNCTIONS
static int spell_clear_handler(void)
{
	int i;

	for(i=0;i<MAX_SIGILS;i++) {
		on_cast[i]=-1;
	}

	we_have_spell=-1;
	raw_spell_text[0] = 0;
	return 1;
}

void send_spell(Uint8 *str, int len)
{
	my_tcp_send(my_socket, str, len);
	memcpy(last_spell_str, str, len);
	last_spell_len = len;
}

/* show the last spell name and message bytes */
int command_show_spell(char *text, int len)
{
	int i;
	char out_str[128];
	char mess_str[64];

	/* trap if we have no last spell or other invalid strings */
	if (!*last_spell_name || strlen(last_spell_name)>59 || last_spell_len>30 || last_spell_len<=0)
	{
		LOG_TO_CONSOLE(c_green2, no_spell_to_show_str);
		return 1;
	}

	/* create the message body string, each byte in hex */
	for(i=0; i<last_spell_len; i++)
		sprintf(&mess_str[2*i], "%02x", last_spell_str[i]);
	mess_str[last_spell_len*2] = 0;

	safe_snprintf(out_str, sizeof(out_str), "%s %s", last_spell_name, mess_str );
	LOG_TO_CONSOLE(c_green2, out_str);

	return 1;
}

static int prepare_for_cast(void){
	Uint8 str[20];
	int count=0;
	int sigils_no=0;
	int i;

	for(i=0;i<MAX_SIGILS;i++) {
		if(on_cast[i]!=-1) {
			count++;
		}
	}

	if(count<2) {
		safe_snprintf((char*)raw_spell_text, sizeof(raw_spell_text), "%c%s",127+c_red2,sig_too_few_sigs);
		have_error_message=1;
		return 0;
	}

	str[0]=CAST_SPELL;
	for(i=0;i<MAX_SIGILS;i++) {
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

	if(get_id_MW(MW_SPELLS) != sigils_win && we_have_spell >= 0){
		mqb_data[0]->spell_id=spells_list[we_have_spell].id;
		mqb_data[0]->spell_image=spells_list[we_have_spell].image;
		memcpy(mqb_data[0]->spell_name, spells_list[we_have_spell].name, 60);
	}

	memcpy(mqb_data[0]->spell_str, str, sigils_no+2);//Copy the last spell send to the server
	return sigils_no;
}

static int cast_handler(void)
{
	//Cast?

	int sigils_no=prepare_for_cast();
	//ok, send it to the server...
	if(sigils_no) send_spell(mqb_data[0]->spell_str, sigils_no+2);
	return 1;
}

static int ui_scale_spells_handler(window_info *win)
{
	size_t i;
	int len_x;
	int len_y;
	int y;
	int gy = 0;
	widget_list *w_cast = NULL;

	spell_grid_size = (int)(0.5 + win->current_scale * 33);
	spell_border = (int)(0.5 + win->current_scale * 10);

	//calc spell_win
	for(i=0;i<num_groups;i+=2)
		gy += spell_border + win->small_font_len_y + spell_grid_size *
			MAX((groups_list[i].spells / (SPELLS_ALIGN_X + 1) + 1), (groups_list[i+1].spells / (SPELLS_ALIGN_X + 1) + 1));

	len_x = SPELLS_ALIGN_X * spell_grid_size * 2 + spell_grid_size + 2 * spell_border + win->box_size;
	spell_text_y = gy + spell_border;
	spell_engred_y = spell_text_y + 3 * win->small_font_len_y + spell_border + win->small_font_len_y;
	len_y = spell_engred_y + spell_grid_size + spell_border + 2 * win->small_font_len_y + spell_border;

	y = spell_border;
	for(i=0;i<num_groups;i++){

		groups_list[i].x = spell_border;
		groups_list[i].y = y + win->small_font_len_y;
		if(i == num_groups - 1)
			 //if groups are odd, last one is drawn in the middle
			groups_list[i].x += ((2 * spell_grid_size * SPELLS_ALIGN_X + spell_grid_size) - (spell_grid_size * SPELLS_ALIGN_X)) / 2;
		i++;
		if(i >= num_groups)
			break;
		groups_list[i].x = spell_border + spell_grid_size + spell_grid_size * SPELLS_ALIGN_X;
		groups_list[i].y = y + win->small_font_len_y;
		y += spell_border + win->small_font_len_y + spell_grid_size *
			MAX(groups_list[i-1].spells / (SPELLS_ALIGN_X + 1) + 1, groups_list[i].spells / (SPELLS_ALIGN_X + 1) + 1);
	}

	w_cast = widget_find(spell_win, cast2_button_id);
	button_resize(win->window_id, cast2_button_id, 0, 0, win->current_scale);
	widget_move(win->window_id, cast2_button_id, len_x - spell_border - w_cast->len_x, len_y - w_cast->len_y - spell_border);

	resize_window(win->window_id, len_x, len_y);
	return 1;
}

static int change_spells_font_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	ui_scale_spells_handler(win);
	return 1;
}

static int ui_scale_sigils_handler(window_info *win)
{
	int but_space = 0;
	int len_x, len_y;
	widget_list *w_cast = NULL;
	widget_list *w_clear = NULL;

	sigil_border = (int)(0.5 + win->current_scale * 5);
	sigil_grid_size = (int)(0.5 + win->current_scale * 33);
	len_x = sigil_grid_size * NUM_SIGILS_LINE + win->box_size;
	len_y = sigil_grid_size * (1 + NUM_SIGILS_ROW) + 5 * win->small_font_len_y + sigil_border;
	spell_icon_x = len_x - sigil_grid_size - sigil_border;
	spell_icon_y = (1 + NUM_SIGILS_ROW) * sigil_grid_size;

	button_resize(win->window_id, cast_button_id, 0, 0, win->current_scale);
	button_resize(win->window_id, clear_button_id, 0, 0, win->current_scale);
	w_cast = widget_find(win->window_id, cast_button_id);
	w_clear = widget_find(win->window_id, clear_button_id);
	but_space = (len_x - (sigil_grid_size*MAX_SIGILS+sigil_border) - w_cast->len_x - w_clear->len_x)/3;
	widget_move(win->window_id, cast_button_id, sigil_grid_size*MAX_SIGILS+sigil_border + but_space+sigil_border, len_y - w_cast->len_y - sigil_border -1);
	widget_move(win->window_id, clear_button_id, w_cast->pos_x + w_cast->len_x + but_space, len_y - w_clear->len_y - sigil_border - 1);

	resize_window(win->window_id, len_x, len_y);

	return 1;
}

static int change_sigils_font_handler(window_info *win, font_cat cat)
{
	if (cat != win->font_category)
		return 0;
	ui_scale_sigils_handler(win);
	return 1;
}

static int ui_scale_spells_mini_handler(window_info *win)
{
	size_t i;
	int len_x, len_y;

	//calc spell_mini_win
	spell_mini_rows=0;
	for(i=0;i<num_groups;i++)
		spell_mini_rows+=groups_list[i].spells/(SPELLS_ALIGN_X+1)+1;

	spell_mini_border = (int)(0.5 + win->current_scale * 5);
	spell_mini_grid_size = (int)(0.5 + win->current_scale * 33);
	len_x = SPELLS_ALIGN_X * spell_mini_grid_size + 2 * spell_mini_border + win->box_size;
	len_y = spell_mini_border + spell_mini_grid_size * spell_mini_rows + 3 * win->small_font_len_y + spell_mini_grid_size + spell_mini_border;

	resize_window(win->window_id, len_x, len_y);
	return 1;
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

	if(sigils_win < 0){
		//create sigil win
		sigils_win = create_window(win_sigils, (not_on_top_now(MW_SPELLS) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_SPELLS), get_pos_y_MW(MW_SPELLS), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);

		set_window_custom_scale(sigils_win, MW_SPELLS);
		set_window_handler(sigils_win, ELW_HANDLER_DISPLAY, &display_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_CLICK, &click_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_MOUSEOVER, &mouseover_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_UI_SCALE, &ui_scale_sigils_handler );
		set_window_handler(sigils_win, ELW_HANDLER_FONT_CHANGE, &change_sigils_font_handler);

		cast_button_id=button_add_extended(sigils_win, cast_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, cast_str);
		widget_set_OnClick(sigils_win, cast_button_id, cast_handler);

		clear_button_id=button_add_extended(sigils_win, clear_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, clear_str);
		widget_set_OnClick(sigils_win, clear_button_id, spell_clear_handler);

		if (sigils_win >= 0 && sigils_win < windows_list.num_windows)
			ui_scale_sigils_handler(&windows_list.window[sigils_win]);

		hide_window(sigils_win);
	}

	if(spell_win < 0){
		//create spell win
		spell_win= create_window("Spells", (not_on_top_now(MW_SPELLS) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_SPELLS), get_pos_y_MW(MW_SPELLS), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);

		set_window_custom_scale(spell_win, MW_SPELLS);
		set_window_handler(spell_win, ELW_HANDLER_DISPLAY, &display_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_CLICK, &click_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_MOUSEOVER, &mouseover_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_UI_SCALE, &ui_scale_spells_handler );
		set_window_handler(spell_win, ELW_HANDLER_FONT_CHANGE, &change_spells_font_handler);

		cast2_button_id=button_add_extended(spell_win, cast2_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, cast_str);
		widget_set_OnClick(spell_win, cast2_button_id, cast_handler);

		if (spell_win >= 0 && spell_win < windows_list.num_windows)
			ui_scale_spells_handler(&windows_list.window[spell_win]);

		hide_window(spell_win);
		if(!start_mini_spells) set_id_MW(MW_SPELLS, spell_win);
	}

	if(spell_mini_win < 0){
		//create mini spell win
		spell_mini_win= create_window("Spells", (not_on_top_now(MW_SPELLS) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_SPELLS), get_pos_y_MW(MW_SPELLS), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);

		set_window_custom_scale(spell_mini_win, MW_SPELLS);
		set_window_handler(spell_mini_win, ELW_HANDLER_DISPLAY, &display_spells_mini_handler );
		set_window_handler(spell_mini_win, ELW_HANDLER_CLICK, &click_spells_mini_handler );
		set_window_handler(spell_mini_win, ELW_HANDLER_MOUSEOVER, &mouseover_spells_mini_handler );
		set_window_handler(spell_mini_win, ELW_HANDLER_UI_SCALE, &ui_scale_spells_mini_handler );

		if (spell_mini_win >= 0 && spell_mini_win < windows_list.num_windows)
			ui_scale_spells_mini_handler(&windows_list.window[spell_mini_win]);

		hide_window(spell_mini_win);
		if(start_mini_spells) set_id_MW(MW_SPELLS, spell_mini_win);
	}
	check_castability();
	switch_handler((init_ok) ? (get_id_MW(MW_SPELLS)):(sigils_win));

	check_proportional_move(MW_SPELLS);
}


static void init_sigils(void){
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

void spell_text_from_server(const Uint8 *in_data, int data_length)
{
	safe_strncpy2((char *)raw_spell_text, (const char *)in_data, TEXTBUFSIZE, data_length);
	if(get_id_MW(MW_SPELLS) == -1 || !windows_list.window[get_id_MW(MW_SPELLS)].displayed)
		put_text_in_buffer (CHAT_SERVER, in_data, data_length);
	have_error_message=1;
}
