#ifndef __STATS_H__
#define __STATS_H__

typedef struct
{
	Sint16 base;
	Sint16 cur;
} attrib_16;

typedef struct
{
	char name[20];
	attrib_16 phy;
	attrib_16 coo;

	attrib_16 rea;
	attrib_16 wil;

	attrib_16 ins;
	attrib_16 vit;

	attrib_16 human_nex;
	attrib_16 animal_nex;
	attrib_16 vegetal_nex;
	attrib_16 inorganic_nex;
	attrib_16 artificial_nex;
	attrib_16 magic_nex;

	attrib_16 karma;
	attrib_16 material_points;
	attrib_16 ethereal_points;

	attrib_16 manufacturing_skill;
	attrib_16 harvesting_skill;
	attrib_16 alchemy_skill;
	attrib_16 combat_skill;
	attrib_16 attack_skill;
	attrib_16 defense_skill;
	attrib_16 magic_skill;
	attrib_16 potion_skill;
	attrib_16 summoning_skill;
	attrib_16 crafting_skill;

	attrib_16 carry_capacity;
	Sint8 food_level;
	Sint8 armor;
	Sint8 damage;
	Sint8 accuracy;
	Sint8 magic_resistence;

	Uint32 manufacturing_exp;
	Uint32 manufacturing_exp_next_lev;
	Uint32 harvesting_exp;
	Uint32 harvesting_exp_next_lev;
	Uint32 alchemy_exp;
	Uint32 alchemy_exp_next_lev;
	Uint32 combat_exp;
	Uint32 combat_exp_next_lev;
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

} player_attribs;

extern int attrib_menu_x;
extern int attrib_menu_y;

extern int attrib_menu_x_len;
extern int attrib_menu_y_len;
extern int attrib_menu_dragged;


player_attribs your_info;
player_attribs someone_info;

void get_the_stats(Sint16 *stats);
void get_partial_stat(Uint8 name,Sint32 value);
void display_stats(player_attribs cur_stats);

#endif

