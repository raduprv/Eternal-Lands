#ifndef __STATS_H__
#define __STATS_H__

#include "global.h"

extern int	stats_win;

typedef struct
{
	unsigned char name[21];
#ifdef WRITE_XML
	int saved_name;
#endif
	unsigned char shortname[6];
#ifdef WRITE_XML
	int saved_shortname;
#endif
} names;

typedef struct
{
	Sint16 base;
	Sint16 cur;
} attrib_16;

typedef struct
{
	Sint16 (*base)(void);
	Sint16 (*cur)(void);
} attrib_16f;

struct attributes_struct
{
	unsigned char base[30];
	
	names phy;
	names coo;

	names rea;
	names wil;
	
	names ins;
	names vit;
	
	unsigned char cross[30];
	names might;
	names matter;
	names tough;
	names react;
	names charm;
	names perc;
	names ration;
	names dext;
	names eth;
	
	unsigned char nexus[30];
	names human_nex;
	names animal_nex;
	names vegetal_nex;
	names inorganic_nex;
	names artificial_nex;
	names magic_nex;
	
	unsigned char skills[30];
	names manufacturing_skill;
	names harvesting_skill;
	names alchemy_skill;
	names overall_skill;
	names attack_skill;
	names defense_skill;
	names magic_skill;
	names potion_skill;
	names summoning_skill;
	names crafting_skill;
	
	names food;
	unsigned char pickpoints[30];
	names material_points;
	names ethereal_points;

	names carry_capacity;
};

struct attributes_struct attributes;

typedef struct
{
	unsigned char name[20];
	
	attrib_16 phy;
	attrib_16 coo;

	attrib_16 rea;
	attrib_16 wil;

	attrib_16 ins;
	attrib_16 vit;

	attrib_16f might;
	attrib_16f matter;
	attrib_16f tough;
	attrib_16f charm;
	attrib_16f react;
	attrib_16f perc;
	attrib_16f ration;
	attrib_16f dext;
	attrib_16f eth;
	
	attrib_16 human_nex;
	attrib_16 animal_nex;
	attrib_16 vegetal_nex;
	attrib_16 inorganic_nex;
	attrib_16 artificial_nex;
	attrib_16 magic_nex;

	attrib_16 material_points;
	attrib_16 ethereal_points;

	attrib_16 manufacturing_skill;
	attrib_16 harvesting_skill;
	attrib_16 alchemy_skill;
	attrib_16 overall_skill;
	attrib_16 attack_skill;
	attrib_16 defense_skill;
	attrib_16 magic_skill;
	attrib_16 potion_skill;
	attrib_16 summoning_skill;
	attrib_16 crafting_skill;

	attrib_16 carry_capacity;
	
	Sint8 food_level;

	Uint32 manufacturing_exp;
	Uint32 manufacturing_exp_next_lev;
	Uint32 harvesting_exp;
	Uint32 harvesting_exp_next_lev;
	Uint32 alchemy_exp;
	Uint32 alchemy_exp_next_lev;
	Uint32 overall_exp;
	Uint32 overall_exp_next_lev;
	Uint32 attack_exp;
	Uint32 attack_exp_next_lev;
	Uint32 defense_exp;
	Uint32 defense_exp_next_lev;
	Uint32 magic_exp;
	Uint32 magic_exp_next_lev;
	Uint32 potion_exp;
	Uint32 potion_exp_next_lev;
	Uint32 summoning_exp;
	Uint32 summoning_exp_next_lev;
	Uint32 crafting_exp;
	Uint32 crafting_exp_next_lev;

	Uint16 researching;
	Uint16 research_completed;
	Uint16 research_total;
} player_attribs;
#define	NUM_WATCH_STAT	11	// allow watching stats 0-10

extern int attrib_menu_x;
extern int attrib_menu_y;
extern int attrib_menu_x_len;
extern int attrib_menu_y_len;
//extern int attrib_menu_dragged;

extern int watch_this_stat;

player_attribs your_info;
player_attribs someone_info;

void get_the_stats(Sint16 *stats);
void get_partial_stat(unsigned char name,Sint32 value);
void display_stats(player_attribs cur_stats);
void init_attribf(void);

#endif

