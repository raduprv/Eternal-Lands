#include "global.h"

void get_the_stats(Sint16 *stats)
{
	your_info.phy.cur=stats[0];
	your_info.phy.base=stats[1];
	your_info.coo.cur=stats[2];
	your_info.coo.base=stats[3];
	your_info.rea.cur=stats[4];
	your_info.rea.base=stats[5];
	your_info.wil.cur=stats[6];
	your_info.wil.base=stats[7];
	your_info.ins.cur=stats[8];
	your_info.ins.base=stats[9];
	your_info.vit.cur=stats[10];
	your_info.vit.base=stats[11];

	your_info.human_nex.cur=stats[12];
	your_info.human_nex.base=stats[13];
	your_info.animal_nex.cur=stats[14];
	your_info.animal_nex.base=stats[15];
	your_info.vegetal_nex.cur=stats[16];
	your_info.vegetal_nex.base=stats[17];
	your_info.inorganic_nex.cur=stats[18];
	your_info.inorganic_nex.base=stats[19];
	your_info.artificial_nex.cur=stats[20];
	your_info.artificial_nex.base=stats[21];
	your_info.magic_nex.cur=stats[22];
	your_info.magic_nex.base=stats[23];

	your_info.manufacturing_skill.cur=stats[24];
	your_info.manufacturing_skill.base=stats[25];
	your_info.harvesting_skill.cur=stats[26];
	your_info.harvesting_skill.base=stats[27];
	your_info.alchemy_skill.cur=stats[28];
	your_info.alchemy_skill.base=stats[29];
	your_info.combat_skill.cur=stats[30];
	your_info.combat_skill.base=stats[31];
	your_info.attack_skill.cur=stats[32];
	your_info.attack_skill.base=stats[33];
	your_info.defense_skill.cur=stats[34];
	your_info.defense_skill.base=stats[35];
	your_info.magic_skill.cur=stats[36];
	your_info.magic_skill.base=stats[37];
	your_info.potion_skill.cur=stats[38];
	your_info.potion_skill.base=stats[39];
	your_info.carry_capacity.cur=stats[40];
	your_info.carry_capacity.base=stats[41];
	your_info.material_points.cur=stats[42];
	your_info.material_points.base=stats[43];
	your_info.ethereal_points.cur=stats[44];
	your_info.ethereal_points.base=stats[45];
	your_info.food_level=stats[46];
	your_info.armor=stats[47];
	your_info.magic_resistence=stats[48];

	your_info.manufacturing_exp=*((Uint32 *)(stats+49));
	your_info.manufacturing_exp_next_lev=*((Uint32 *)(stats+51));
	your_info.harvesting_exp=*((Uint32 *)(stats+53));
	your_info.harvesting_exp_next_lev=*((Uint32 *)(stats+55));
	your_info.alchemy_exp=*((Uint32 *)(stats+57));
	your_info.alchemy_exp_next_lev=*((Uint32 *)(stats+59));
	your_info.combat_exp=*((Uint32 *)(stats+61));
	your_info.combat_exp_next_lev=*((Uint32 *)(stats+63));
	your_info.attack_exp=*((Uint32 *)(stats+65));
	your_info.attack_exp_next_lev=*((Uint32 *)(stats+67));
	your_info.defense_exp=*((Uint32 *)(stats+69));
	your_info.defense_exp_next_lev=*((Uint32 *)(stats+71));
	your_info.magic_exp=*((Uint32 *)(stats+73));
	your_info.magic_exp_next_lev=*((Uint32 *)(stats+75));
	your_info.potion_exp=*((Uint32 *)(stats+77));
	your_info.potion_exp_next_lev=*((Uint32 *)(stats+79));

	your_info.accuracy=stats[81];
	your_info.damage=stats[82];
	your_info.summoning_skill.cur=stats[83];
	your_info.summoning_skill.base=stats[84];
	your_info.summoning_exp=*((Uint32 *)(stats+85));
	your_info.summoning_exp_next_lev=*((Uint32 *)(stats+87));

}

void get_partial_stat(Uint8 name,Sint32 value)
	{
		if(name==PHY_CUR)your_info.phy.cur=value;
		else
		if(name==PHY_BASE)your_info.phy.base=value;
		else
		if(name==COO_CUR)your_info.coo.cur=value;
		else
		if(name==COO_BASE)your_info.coo.base=value;
		else
		if(name==REAS_CUR)your_info.rea.cur=value;
		else
		if(name==REAS_BASE)your_info.rea.base=value;
		else
		if(name==WILL_CUR)your_info.wil.cur=value;
		else
		if(name==WILL_BASE)your_info.wil.base=value;
		else
		if(name==INST_CUR)your_info.ins.cur=value;
		else
		if(name==INST_BASE)your_info.ins.base=value;
		else
		if(name==VIT_CUR)your_info.vit.cur=value;
		else
		if(name==VIT_BASE)your_info.vit.base=value;
		else
		if(name==HUMAN_CUR)your_info.human_nex.cur=value;
		else
		if(name==HUMAN_BASE)your_info.human_nex.base=value;
		else
		if(name==ANIMAL_CUR)your_info.animal_nex.cur=value;
		else
		if(name==ANIMAL_BASE)your_info.animal_nex.base=value;
		else
		if(name==VEGETAL_CUR)your_info.vegetal_nex.cur=value;
		else
		if(name==VEGETAL_BASE)your_info.vegetal_nex.base=value;
		else
		if(name==INORG_CUR)your_info.inorganic_nex.cur=value;
		else
		if(name==INORG_BASE)your_info.inorganic_nex.base=value;
		else
		if(name==ARTIF_CUR)your_info.artificial_nex.cur=value;
		else
		if(name==ARTIF_BASE)your_info.artificial_nex.base=value;
		else
		if(name==MAGIC_CUR)your_info.magic_nex.cur=value;
		else
		if(name==MAGIC_BASE)your_info.magic_nex.base=value;
		else
		if(name==MAN_S_CUR)your_info.manufacturing_skill.cur=value;
		else
		if(name==MAN_S_BASE)your_info.manufacturing_skill.base=value;
		else
		if(name==HARV_S_CUR)your_info.harvesting_skill.cur=value;
		else
		if(name==HARV_S_BASE)your_info.harvesting_skill.base=value;
		else
		if(name==ALCH_S_CUR)your_info.alchemy_skill.cur=value;
		else
		if(name==ALCH_S_BASE)your_info.alchemy_skill.base=value;
		else
		if(name==COMB_S_CUR)your_info.combat_skill.cur=value;
		else
		if(name==COMB_S_BASE)your_info.combat_skill.base=value;
		else
		if(name==ATT_S_CUR)your_info.attack_skill.cur=value;
		else
		if(name==ATT_S_BASE)your_info.attack_skill.base=value;
		else
		if(name==DEF_S_CUR)your_info.defense_skill.cur=value;
		else
		if(name==DEF_S_BASE)your_info.defense_skill.base=value;
		else
		if(name==MAG_S_CUR)your_info.magic_skill.cur=value;
		else
		if(name==MAG_S_BASE)your_info.magic_skill.base=value;
		else
		if(name==POT_S_CUR)your_info.potion_skill.cur=value;
		else
		if(name==POT_S_BASE)your_info.potion_skill.base=value;
		else
		if(name==CARRY_WGHT_CUR)your_info.carry_capacity.cur=value;
		else
		if(name==CARRY_WGHT_BASE)your_info.carry_capacity.base=value;
		else
		if(name==MAT_POINT_CUR)your_info.material_points.cur=value;
		else
		if(name==MAT_POINT_BASE)your_info.material_points.base=value;
		else
		if(name==ETH_POINT_CUR)your_info.ethereal_points.cur=value;
		else
		if(name==ETH_POINT_BASE)your_info.ethereal_points.base=value;
		else
		if(name==FOOD_LEV)your_info.food_level=value;
		else
		if(name==ARMOR_STR)your_info.armor=value;
		else
		if(name==MAG_RES)your_info.magic_resistence=value;
		else
		if(name==MAN_EXP)your_info.manufacturing_exp=value;
		else
		if(name==MAN_EXP_NEXT)your_info.manufacturing_exp_next_lev=value;
		else
		if(name==HARV_EXP)your_info.harvesting_exp=value;
		else
		if(name==HARV_EXP_NEXT)your_info.harvesting_exp_next_lev=value;
		else
		if(name==ALCH_EXP)your_info.alchemy_exp=value;
		else
		if(name==ALCH_EXP_NEXT)your_info.alchemy_exp_next_lev=value;
		else
		if(name==COMB_EXP)your_info.combat_exp=value;
		else
		if(name==COMB_EXP_NEXT)your_info.combat_exp_next_lev=value;
		else
		if(name==DEF_EXP)your_info.defense_exp=value;
		else
		if(name==DEF_EXP_NEXT)your_info.defense_exp_next_lev=value;
		else
		if(name==ATT_EXP)your_info.attack_exp=value;
		else
		if(name==ATT_EXP_NEXT)your_info.attack_exp_next_lev=value;
		else
		if(name==MAG_EXP)your_info.magic_exp=value;
		else
		if(name==MAG_EXP_NEXT)your_info.magic_exp_next_lev=value;
		else
		if(name==POT_EXP)your_info.potion_exp=value;
		else
		if(name==POT_EXP_NEXT)your_info.potion_exp_next_lev=value;
		else
		if(name==WEAP_ACCURACY)your_info.accuracy=value;
		else
		if(name==WEAP_DAMAGE)your_info.damage=value;
		else
		if(name==SUM_EXP)your_info.summoning_exp=value;
		else
		if(name==SUM_EXP_NEXT)your_info.summoning_exp_next_lev=value;
		else
		if(name==SUM_S_CUR)your_info.summoning_skill.cur=value;
		else
		if(name==SUM_S_BASE)your_info.summoning_skill.base=value;

	}


void display_stats(player_attribs cur_stats)
{
	Uint8 str[80];
	int x,y;
	//first of all, draw the actual menu.

	draw_menu_title_bar(attrib_menu_x,attrib_menu_y-16,attrib_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(attrib_menu_x,attrib_menu_y+attrib_menu_y_len,0);
	glVertex3i(attrib_menu_x,attrib_menu_y,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y+attrib_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.0f,1.0f,0.0f);
	glBegin(GL_LINES);
	glVertex3i(attrib_menu_x,attrib_menu_y,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y,0);

	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y+attrib_menu_y_len,0);

	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y+attrib_menu_y_len,0);
	glVertex3i(attrib_menu_x,attrib_menu_y+attrib_menu_y_len,0);

	glVertex3i(attrib_menu_x,attrib_menu_y+attrib_menu_y_len,0);
	glVertex3i(attrib_menu_x,attrib_menu_y,0);

	//draw the corner, with the X in
	glVertex3i(attrib_menu_x+attrib_menu_x_len,attrib_menu_y+20,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len-20,attrib_menu_y+20,0);

	glVertex3i(attrib_menu_x+attrib_menu_x_len-20,attrib_menu_y+20,0);
	glVertex3i(attrib_menu_x+attrib_menu_x_len-20,attrib_menu_y,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	draw_string_small(attrib_menu_x+attrib_menu_x_len-16,attrib_menu_y+2,"X",1);

	x=attrib_menu_x+5;
	y=attrib_menu_y+5;

	draw_string_small(x,y,"Basic Attributes",1);
	y+=14;
	sprintf(str,"Physique:     %2i/%-2i",cur_stats.phy.cur,cur_stats.phy.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Coordination: %2i/%-2i",cur_stats.coo.cur,cur_stats.coo.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Reasoning:    %2i/%-2i",cur_stats.rea.cur,cur_stats.rea.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Will:         %2i/%-2i",cur_stats.wil.cur,cur_stats.wil.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Instinct:     %2i/%-2i",cur_stats.ins.cur,cur_stats.ins.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Vitality:     %2i/%-2i",cur_stats.vit.cur,cur_stats.vit.base);
	draw_string_small(x,y,str,1);

	//cross attributes
	glColor3f(1.0f,1.0f,0.0f);
	y+=20;

	draw_string_small(x,y,"Cross Attributes",1);
	y+=14;
	sprintf(str,"Might:        %2i/%-2i",(cur_stats.phy.cur+cur_stats.coo.cur)/2,(cur_stats.phy.base+cur_stats.coo.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Matter:       %2i/%-2i",(cur_stats.phy.cur+cur_stats.wil.cur)/2,(cur_stats.phy.base+cur_stats.wil.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Toughness:    %2i/%-2i",(cur_stats.phy.cur+cur_stats.vit.cur)/2,(cur_stats.phy.base+cur_stats.vit.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Charm:        %2i/%-2i",(cur_stats.ins.cur+cur_stats.vit.cur)/2,(cur_stats.ins.base+cur_stats.vit.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Reaction:     %2i/%-2i",(cur_stats.ins.cur+cur_stats.coo.cur)/2,(cur_stats.ins.base+cur_stats.coo.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Perception:   %2i/%-2i",(cur_stats.ins.cur+cur_stats.rea.cur)/2,(cur_stats.ins.base+cur_stats.rea.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Rationality:  %2i/%-2i",(cur_stats.wil.cur+cur_stats.rea.cur)/2,(cur_stats.wil.base+cur_stats.rea.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Dexterity:    %2i/%-2i",(cur_stats.coo.cur+cur_stats.rea.cur)/2,(cur_stats.coo.base+cur_stats.rea.base)/2);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Ethereality:  %2i/%-2i",(cur_stats.wil.cur+cur_stats.vit.cur)/2,(cur_stats.wil.base+cur_stats.vit.base)/2);
	draw_string_small(x,y,str,1);

	glColor3f(0.5f,0.5f,1.0f);

	//other attribs
	y+=20;
	sprintf(str,"Food level:      %i",cur_stats.food_level);
	draw_string_small(x,y,str,1);
	y+=14;
	sprintf(str,"Material Points: %2i/%-2i",cur_stats.material_points.cur,cur_stats.material_points.base);
	draw_string_small(x,y,str,1);
	y+=14;
	sprintf(str,"Ethereal Points: %2i/%-2i",cur_stats.ethereal_points.cur,cur_stats.ethereal_points.base);
	draw_string_small(x,y,str,1);

	//armor and magic protection
	y-=28;
	sprintf(str,"Armor: %i",cur_stats.armor);
	draw_string_small(attrib_menu_x+230,y,str,1);
	y+=14;
	sprintf(str,"Weapon Accuracy:  %i",cur_stats.accuracy);
	draw_string_small(attrib_menu_x+230,y,str,1);
	y+=14;
	if(cur_stats.damage<3)cur_stats.damage=3;
	sprintf(str,"Weapon Damage:    %i",cur_stats.damage);
	draw_string_small(attrib_menu_x+230,y,str,1);
	y+=14;
	sprintf(str,"Magic Protection: %i",cur_stats.magic_resistence);
	draw_string_small(attrib_menu_x+230,y,str,1);


	//nexuses here
	glColor3f(1.0f,1.0f,1.0f);
	x+=170;
	y=attrib_menu_y+5;

	draw_string_small(x,y,"Nexuses",1);
	y+=14;

	sprintf(str,"Human:       %2i/%-2i",cur_stats.human_nex.cur,cur_stats.human_nex.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Animal:      %2i/%-2i",cur_stats.animal_nex.cur,cur_stats.animal_nex.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Vegetal:     %2i/%-2i",cur_stats.vegetal_nex.cur,cur_stats.vegetal_nex.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Inorganic:   %2i/%-2i",cur_stats.inorganic_nex.cur,cur_stats.inorganic_nex.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Artificial:  %2i/%-2i",cur_stats.artificial_nex.cur,cur_stats.artificial_nex.base);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Magic:       %2i/%-2i",cur_stats.magic_nex.cur,cur_stats.magic_nex.base);
	draw_string_small(x,y,str,1);

	y+=20;
	//skills
	glColor3f(1.0f,0.5f,0.2f);
	draw_string_small(x,y,"Skills",1);
	y+=14;

	sprintf(str,"Manufacture: %2i/%-2i [%2i/%-2i]",cur_stats.manufacturing_skill.cur,cur_stats.manufacturing_skill.base,
	cur_stats.manufacturing_exp,cur_stats.manufacturing_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Harvest:     %2i/%-2i [%2i/%-2i]",cur_stats.harvesting_skill.cur,cur_stats.harvesting_skill.base,
	cur_stats.harvesting_exp,cur_stats.harvesting_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Alchemy:     %2i/%-2i [%2i/%-2i]",cur_stats.alchemy_skill.cur,cur_stats.alchemy_skill.base,
	cur_stats.alchemy_exp,cur_stats.alchemy_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Combat:      %2i/%-2i [%2i/%-2i]",cur_stats.combat_skill.cur,cur_stats.combat_skill.base,
	cur_stats.combat_exp,cur_stats.combat_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Attack:      %2i/%-2i [%2i/%-2i]",cur_stats.attack_skill.cur,cur_stats.attack_skill.base,
	cur_stats.attack_exp,cur_stats.attack_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Defense:     %2i/%-2i [%2i/%-2i]",cur_stats.defense_skill.cur,cur_stats.defense_skill.base,
	cur_stats.defense_exp,cur_stats.defense_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Magic:       %2i/%-2i [%2i/%-2i]",cur_stats.magic_skill.cur,cur_stats.magic_skill.base,
	cur_stats.magic_exp,cur_stats.magic_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Potion:      %2i/%-2i [%2i/%-2i]",cur_stats.potion_skill.cur,cur_stats.potion_skill.base,
	cur_stats.potion_exp,cur_stats.potion_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Summoning:   %2i/%-2i [%2i/%-2i]",cur_stats.summoning_skill.cur,cur_stats.summoning_skill.base,
	cur_stats.summoning_exp,cur_stats.summoning_exp_next_lev);
	draw_string_small(x,y,str,1);
}














