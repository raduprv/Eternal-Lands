#include <string.h>
#include "global.h"

int view_self_stats=0;
player_attribs your_info;
player_attribs someone_info;
int attrib_menu_x=100;
int attrib_menu_y=20;
int attrib_menu_x_len=516;
int attrib_menu_y_len=348;
int attrib_menu_dragged=0;

void get_the_stats(Sint16 *stats)
{
	memset(&your_info, 0, sizeof(your_info));	// failsafe incase structure changes

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
	your_info.overall_skill.cur=stats[30];
	your_info.overall_skill.base=stats[31];
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

	your_info.manufacturing_exp=*((Uint32 *)(stats+49));
	your_info.manufacturing_exp_next_lev=*((Uint32 *)(stats+51));
	your_info.harvesting_exp=*((Uint32 *)(stats+53));
	your_info.harvesting_exp_next_lev=*((Uint32 *)(stats+55));
	your_info.alchemy_exp=*((Uint32 *)(stats+57));
	your_info.alchemy_exp_next_lev=*((Uint32 *)(stats+59));
	your_info.overall_exp=*((Uint32 *)(stats+61));
	your_info.overall_exp_next_lev=*((Uint32 *)(stats+63));
	your_info.attack_exp=*((Uint32 *)(stats+65));
	your_info.attack_exp_next_lev=*((Uint32 *)(stats+67));
	your_info.defense_exp=*((Uint32 *)(stats+69));
	your_info.defense_exp_next_lev=*((Uint32 *)(stats+71));
	your_info.magic_exp=*((Uint32 *)(stats+73));
	your_info.magic_exp_next_lev=*((Uint32 *)(stats+75));
	your_info.potion_exp=*((Uint32 *)(stats+77));
	your_info.potion_exp_next_lev=*((Uint32 *)(stats+79));

	your_info.summoning_skill.cur=stats[83];
	your_info.summoning_skill.base=stats[84];
	your_info.summoning_exp=*((Uint32 *)(stats+85));
	your_info.summoning_exp_next_lev=*((Uint32 *)(stats+87));
	your_info.crafting_skill.cur=stats[89];
	your_info.crafting_skill.base=stats[90];
	your_info.crafting_exp=*((Uint32 *)(stats+91));
	your_info.crafting_exp_next_lev=*((Uint32 *)(stats+93));

	your_info.research_completed=stats[47];
	your_info.researching=stats[81];
	your_info.research_total=stats[82];
}

void get_partial_stat(Uint8 name,Sint32 value)
{
	switch(name)
		{
		case PHY_CUR:
			your_info.phy.cur=value;break;
		case PHY_BASE:
			your_info.phy.base=value;break;
		case COO_CUR:
			your_info.coo.cur=value;break;
		case COO_BASE:
			your_info.coo.base=value;break;
		case REAS_CUR:
			your_info.rea.cur=value;break;
		case REAS_BASE:
			your_info.rea.base=value;break;
		case WILL_CUR:
			your_info.wil.cur=value;break;
		case WILL_BASE:
			your_info.wil.base=value;break;
		case INST_CUR:
			your_info.ins.cur=value;break;
		case INST_BASE:
			your_info.ins.base=value;break;
		case VIT_CUR:
			your_info.vit.cur=value;break;
		case VIT_BASE:
			your_info.vit.base=value;break;
		case HUMAN_CUR:
			your_info.human_nex.cur=value;break;
		case HUMAN_BASE:
			your_info.human_nex.base=value;break;
		case ANIMAL_CUR:
			your_info.animal_nex.cur=value;break;
		case ANIMAL_BASE:
			your_info.animal_nex.base=value;break;
		case VEGETAL_CUR:
			your_info.vegetal_nex.cur=value;break;
		case VEGETAL_BASE:
			your_info.vegetal_nex.base=value;break;
		case INORG_CUR:
			your_info.inorganic_nex.cur=value;break;
		case INORG_BASE:
			your_info.inorganic_nex.base=value;break;
		case ARTIF_CUR:
			your_info.artificial_nex.cur=value;break;
		case ARTIF_BASE:
			your_info.artificial_nex.base=value;break;
		case MAGIC_CUR:
			your_info.magic_nex.cur=value;break;
		case MAGIC_BASE:
			your_info.magic_nex.base=value;break;
		case MAN_S_CUR:
			your_info.manufacturing_skill.cur=value;break;
		case MAN_S_BASE:
			your_info.manufacturing_skill.base=value;break;
		case HARV_S_CUR:
			your_info.harvesting_skill.cur=value;break;
		case HARV_S_BASE:
			your_info.harvesting_skill.base=value;break;
		case ALCH_S_CUR:
			your_info.alchemy_skill.cur=value;break;
		case ALCH_S_BASE:
			your_info.alchemy_skill.base=value;break;
		case OVRL_S_CUR:
			your_info.overall_skill.cur=value;break;
		case OVRL_S_BASE:
			your_info.overall_skill.base=value;break;
		case ATT_S_CUR:
			your_info.attack_skill.cur=value;break;
		case ATT_S_BASE:
			your_info.attack_skill.base=value;break;
		case DEF_S_CUR:
			your_info.defense_skill.cur=value;break;
		case DEF_S_BASE:
			your_info.defense_skill.base=value;break;
		case MAG_S_CUR:
			your_info.magic_skill.cur=value;break;
		case MAG_S_BASE:
			your_info.magic_skill.base=value;break;
		case POT_S_CUR:
			your_info.potion_skill.cur=value;break;
		case POT_S_BASE:
			your_info.potion_skill.base=value;break;
		case CARRY_WGHT_CUR:
			your_info.carry_capacity.cur=value;break;
		case CARRY_WGHT_BASE:
			your_info.carry_capacity.base=value;break;
		case MAT_POINT_CUR:
			your_info.material_points.cur=value;break;
		case MAT_POINT_BASE:
			your_info.material_points.base=value;break;
		case ETH_POINT_CUR:
			your_info.ethereal_points.cur=value;break;
		case ETH_POINT_BASE:
			your_info.ethereal_points.base=value;break;
		case FOOD_LEV:
			your_info.food_level=value;break;
		case MAN_EXP:
			your_info.manufacturing_exp=value;break;
		case MAN_EXP_NEXT:
			your_info.manufacturing_exp_next_lev=value;break;
		case HARV_EXP:
			your_info.harvesting_exp=value;break;
		case HARV_EXP_NEXT:
			your_info.harvesting_exp_next_lev=value;break;
		case ALCH_EXP:
			your_info.alchemy_exp=value;break;
		case ALCH_EXP_NEXT:
			your_info.alchemy_exp_next_lev=value;break;
		case OVRL_EXP:
			your_info.overall_exp=value;break;
		case OVRL_EXP_NEXT:
			your_info.overall_exp_next_lev=value;break;
		case DEF_EXP:
			your_info.defense_exp=value;break;
		case DEF_EXP_NEXT:
			your_info.defense_exp_next_lev=value;break;
		case ATT_EXP:
			your_info.attack_exp=value;break;
		case ATT_EXP_NEXT:
			your_info.attack_exp_next_lev=value;break;
		case MAG_EXP:
			your_info.magic_exp=value;break;
		case MAG_EXP_NEXT:
			your_info.magic_exp_next_lev=value;break;
		case POT_EXP:
			your_info.potion_exp=value;break;
		case POT_EXP_NEXT:
			your_info.potion_exp_next_lev=value;break;
		case SUM_EXP:
			your_info.summoning_exp=value;break;
		case SUM_EXP_NEXT:
			your_info.summoning_exp_next_lev=value;break;
		case SUM_S_CUR:
			your_info.summoning_skill.cur=value;break;
		case SUM_S_BASE:
			your_info.summoning_skill.base=value;break;
		case CRA_EXP:
			your_info.crafting_exp=value;break;
		case CRA_EXP_NEXT:
			your_info.crafting_exp_next_lev=value;break;
		case CRA_S_CUR:
			your_info.crafting_skill.cur=value;break;
		case CRA_S_BASE:
			your_info.crafting_skill.base=value;break;
		case RESEARCHING:
			your_info.researching=value;break;
		case RESEARCH_COMPLETED:
			your_info.research_completed=value;break;
		case RESEARCH_TOTAL:
			your_info.research_total=value;break;
		default:
			log_error("Server sent invalid stat number\n");
		}
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
	y+=14;	// blank lines for spacing
	y+=14;	// blank lines for spacing

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

	//other info
	y-=28;
	sprintf(str,"Pickpoints: %i",cur_stats.overall_skill.base - cur_stats.overall_skill.cur);
	draw_string_small(attrib_menu_x+190,y,str,1);


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
	sprintf(str,"Attack:      %2i/%-2i [%2i/%-2i]",cur_stats.attack_skill.cur,cur_stats.attack_skill.base,
			cur_stats.attack_exp,cur_stats.attack_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Defense:     %2i/%-2i [%2i/%-2i]",cur_stats.defense_skill.cur,cur_stats.defense_skill.base,
			cur_stats.defense_exp,cur_stats.defense_exp_next_lev);
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

	y+=14;
	sprintf(str,"Manufacture: %2i/%-2i [%2i/%-2i]",cur_stats.manufacturing_skill.cur,cur_stats.manufacturing_skill.base,
			cur_stats.manufacturing_exp,cur_stats.manufacturing_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Crafting:    %2i/%-2i [%2i/%-2i]",cur_stats.crafting_skill.cur,cur_stats.crafting_skill.base,
			cur_stats.crafting_exp,cur_stats.crafting_exp_next_lev);
	draw_string_small(x,y,str,1);

	y+=14;
	sprintf(str,"Overall:     %2i/%-2i [%2i/%-2i]",cur_stats.overall_skill.cur,cur_stats.overall_skill.base,
			cur_stats.overall_exp,cur_stats.overall_exp_next_lev);
	draw_string_small(x,y,str,1);

}


int check_stats_interface()
{
	if(!view_self_stats || mouse_x>attrib_menu_x+attrib_menu_x_len || mouse_x<attrib_menu_x
	   || mouse_y<attrib_menu_y || mouse_y>attrib_menu_y+attrib_menu_y_len)return 0;
	return 1;
}
