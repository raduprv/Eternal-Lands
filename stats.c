#include <string.h>
#include "global.h"
#include "elwindows.h"

int	stats_win= 0;
player_attribs your_info;
player_attribs someone_info;
int attrib_menu_x=100;
int attrib_menu_y=20;
int attrib_menu_x_len=576;
int attrib_menu_y_len=348;
//int attrib_menu_dragged=0;

int watch_this_stat=10;  // default to watching overall
int check_grid_y_top=0;
int check_grid_x_left=0;

void get_the_stats(Sint16 *stats)
{
	memset(&your_info, 0, sizeof(your_info));	// failsafe incase structure changes
	
	//initiate the function pointers
	init_attribf();
	
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

Sint16 get_base_might() { return (your_info.phy.base+your_info.coo.base)/2;}
Sint16 get_cur_might() { return (your_info.phy.cur+your_info.coo.cur)/2;}

Sint16 get_base_matter() { return (your_info.phy.base+your_info.wil.base)/2;}
Sint16 get_cur_matter() { return (your_info.phy.cur+your_info.wil.cur)/2;}

Sint16 get_base_tough() { return (your_info.phy.base+your_info.vit.base)/2;}
Sint16 get_cur_tough() { return (your_info.phy.cur+your_info.vit.cur)/2;}

Sint16 get_base_charm() { return (your_info.ins.base+your_info.vit.base)/2;}
Sint16 get_cur_charm() { return (your_info.ins.cur+your_info.vit.cur)/2;}

Sint16 get_base_react() { return (your_info.ins.base+your_info.coo.base)/2;}
Sint16 get_cur_react() { return (your_info.ins.cur+your_info.coo.cur)/2;}

Sint16 get_base_perc() { return (your_info.ins.base+your_info.rea.base)/2;}
Sint16 get_cur_perc() { return (your_info.ins.cur+your_info.rea.cur)/2;}

Sint16 get_base_rat() { return (your_info.wil.base+your_info.rea.base)/2;}
Sint16 get_cur_rat() { return (your_info.wil.cur+your_info.rea.cur)/2;}

Sint16 get_base_dext() { return (your_info.coo.base+your_info.rea.base)/2;}
Sint16 get_cur_dext() { return (your_info.coo.cur+your_info.rea.base)/2;}

Sint16 get_base_eth() { return (your_info.wil.base+your_info.vit.base)/2;}
Sint16 get_cur_eth() { return (your_info.wil.cur+your_info.vit.cur)/2;}

void init_attribf()
{
	your_info.might.base=get_base_might;
	your_info.might.cur=get_cur_might;
	your_info.matter.base=get_base_matter;
	your_info.matter.cur=get_cur_matter;
	your_info.tough.base=get_base_tough;
	your_info.tough.cur=get_cur_tough;
	your_info.charm.base=get_base_charm;
	your_info.charm.cur=get_cur_charm;
	your_info.react.base=get_base_react;
	your_info.react.cur=get_cur_react;
	your_info.perc.base=get_base_perc;
	your_info.perc.cur=get_cur_perc;
	your_info.ration.base=get_base_rat;
	your_info.ration.cur=get_cur_rat;
	your_info.dext.base=get_base_dext;
	your_info.dext.cur=get_cur_dext;
	your_info.eth.base=get_base_eth;
	your_info.eth.cur=get_cur_eth;
}

void draw_stat_final(int len, int x, int y, char * name, char * value);

void draw_stat(int len, int x, int y, attrib_16 * var, names * name)
{
	char str[8];
	snprintf(str,8,"%2i/%-2i",var->cur,var->base);
	draw_stat_final(len,x,y,name->name,str);
}

void draw_skill(int len, int x, int y, attrib_16 * lvl, names * name, int exp, int exp_next)
{
	char str[36];
	char lvlstr[8];
	char expstr[24];
	int offset;
	snprintf(lvlstr,8,"%2i/%-2i",lvl->cur,lvl->base);
	snprintf(expstr,24,"[%2i/%-2i]",exp, exp_next);
	offset=strlen(str);
	snprintf(str,36,"%-7s %-22s",lvlstr,expstr);
	draw_stat_final(len,x,y,name->name,str);
}

void draw_statf(int len, int x, int y, attrib_16f * var, names * name)
{
	char str[8];
	snprintf(str,8,"%2i/%-2i",var->cur(),var->base());
	draw_stat_final(len,x,y,name->name,str);
}

void draw_stat_final(int len, int x, int y, char * name, char * value)
{
	char str[80];
	snprintf(str,len,"%-15s %s",name,value);
	draw_string_small(x,y,str,1);
}

int display_stats_handler(window_info *win)
{
	player_attribs cur_stats = your_info;
	Uint8 str[10];
	int x,y;
	
	x=5;
	y=5;

	draw_string_small(x,y,attributes.base,1);
	y+=14;
	draw_stat(24,x,y,&(cur_stats.phy),&(attributes.phy));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.coo),&(attributes.coo));
	
	y+=14;
	draw_stat(24,x,y,&(cur_stats.rea),&(attributes.rea));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.wil),&(attributes.wil));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.ins),&(attributes.ins));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.vit),&(attributes.vit));

	//cross attributes
	glColor3f(1.0f,1.0f,0.0f);
	y+=20;

	draw_string_small(x,y,attributes.cross,1);
	y+=14;
	draw_statf(24,x,y,&(cur_stats.might),&(attributes.might));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.matter),&(attributes.matter));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.tough),&(attributes.tough));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.charm),&(attributes.charm));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.react),&(attributes.react));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.perc),&(attributes.perc));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.ration),&(attributes.ration));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.dext),&(attributes.dext));

	y+=14;
	draw_statf(24,x,y,&(cur_stats.eth),&(attributes.eth));

	glColor3f(0.5f,0.5f,1.0f);
	y+=14;	// blank lines for spacing
	y+=14;	// blank lines for spacing

	//other attribs
	y+=20;
	sprintf(str,"%i",cur_stats.food_level);
	draw_stat_final(24,x,y,attributes.food.name,str);
	
	y+=14;
	draw_stat(24,x,y,&(cur_stats.material_points),&(attributes.material_points));
	
	y+=14;
	draw_stat(24,x,y,&(cur_stats.ethereal_points),&(attributes.ethereal_points));

	//other info
	y-=28;
	sprintf(str,"%i",cur_stats.overall_skill.base-cur_stats.overall_skill.cur);
	draw_stat_final(24,205,y,attributes.pickpoints,str);

	//nexuses here
	glColor3f(1.0f,1.0f,1.0f);
	x+=200;
	y=5;

	draw_string_small(x,y,attributes.nexus,1);
	
	y+=14;
	draw_stat(24,x,y,&(cur_stats.human_nex),&(attributes.human_nex));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.animal_nex),&(attributes.animal_nex));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.vegetal_nex),&(attributes.vegetal_nex));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.inorganic_nex),&(attributes.inorganic_nex));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.artificial_nex),&(attributes.artificial_nex));

	y+=14;
	draw_stat(24,x,y,&(cur_stats.magic_nex),&(attributes.magic_nex));

	y+=20;
	//skills
	glColor3f(1.0f,0.5f,0.2f);
	draw_string_small(x,y,attributes.skills,1);

	y+=14;

	check_grid_x_left=x;
	check_grid_y_top=y;

	watch_this_stat==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.attack_skill),&(attributes.attack_skill),cur_stats.attack_exp,cur_stats.attack_exp_next_lev);

	y+=14;
	watch_this_stat==2?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.defense_skill),&(attributes.defense_skill),cur_stats.defense_exp,cur_stats.defense_exp_next_lev);

	y+=14;
	watch_this_stat==3?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.harvesting_skill),&(attributes.harvesting_skill),cur_stats.harvesting_exp,cur_stats.harvesting_exp_next_lev);

	y+=14;
	watch_this_stat==4?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.alchemy_skill),&(attributes.alchemy_skill),cur_stats.alchemy_exp,cur_stats.alchemy_exp_next_lev);

	y+=14;
	watch_this_stat==5?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.magic_skill),&(attributes.magic_skill),cur_stats.magic_exp,cur_stats.magic_exp_next_lev);

	y+=14;
	watch_this_stat==6?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.potion_skill),&(attributes.potion_skill),cur_stats.potion_exp,cur_stats.potion_exp_next_lev);

	y+=14;
	watch_this_stat==7?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.summoning_skill),&(attributes.summoning_skill),cur_stats.summoning_exp,cur_stats.summoning_exp_next_lev);

	y+=14;
	watch_this_stat==8?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.manufacturing_skill),&(attributes.manufacturing_skill),cur_stats.manufacturing_exp,cur_stats.manufacturing_exp_next_lev);

	y+=14;
	watch_this_stat==9?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.crafting_skill),&(attributes.crafting_skill),cur_stats.crafting_exp,cur_stats.crafting_exp_next_lev);

	y+=14;
	watch_this_stat==10?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
	draw_skill(46,x,y,&(cur_stats.overall_skill),&(attributes.overall_skill),cur_stats.overall_exp,cur_stats.overall_exp_next_lev);

	return 1;
}

int click_stats_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int	i;

	if(mx > check_grid_x_left && mx < check_grid_x_left+105 &&
		my > check_grid_y_top && my < check_grid_y_top+140){
		// we don't care which click did the select
		i = 1+(my - check_grid_y_top)/14;
		if (i < NUM_WATCH_STAT){
			watch_this_stat = i;
		}
		return 1;
	}
	return 0;
}

void display_stats(player_attribs cur_stats)	// cur_stats is ignored for this test
{
	if(stats_win <= 0){
		stats_win= create_window("Stats", 0, 0, attrib_menu_x, attrib_menu_y, attrib_menu_x_len, attrib_menu_y_len, ELW_WIN_DEFAULT);

		//set_window_color(stats_win, ELW_COLOR_BORDER, 0.0f, 1.0f, 0.0f, 0.0f);
		set_window_handler(stats_win, ELW_HANDLER_DISPLAY, &display_stats_handler );
		set_window_handler(stats_win, ELW_HANDLER_CLICK, &click_stats_handler );
	} else {
		show_window(stats_win);
		select_window(stats_win);
	}
	display_window(stats_win);
}

