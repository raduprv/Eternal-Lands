#ifdef ECDEBUGWIN

#include "actors_list.h"
#include "cal.h"
#include "client_serv.h"
#include "eye_candy_wrapper.h"
#include "elwindows.h"
#include "gamewin.h"
#include "hud.h"
#include "elconfig.h"
#include "mines.h"
#include "missiles.h"
#include "skeletons.h"

static int ecdw_restoration_handler(void);
static int ecdw_shield_handler(void);
static int ecdw_coldshield_handler(void);
static int ecdw_heatshield_handler(void);
static int ecdw_radiationshield_handler(void);
static int ecdw_heal_handler(void);
static int ecdw_b2g_handler(void);
static int ecdw_magic_immunity_handler(void);
static int ecdw_remote_heal_handler(void);
static int ecdw_poison_handler(void);
static int ecdw_harm_handler(void);
static int ecdw_mana_drain_handler(void);
static int ecdw_alert_handler(void);
static int ecdw_harv_rare_stone_handler(void);
static int ecdw_mine_high_exp_detonate_handler(void);
static int ecdw_harv_goldbag_handler(void);
static int ecdw_harv_bee_handler(void);
static int ecdw_level_up_oa_handler(void);
static int ecdw_magic_protection_handler(void);
static int ecdw_tptpr_handler(void);
static int ecdw_breathe_fire_handler(void);
static int ecdw_breathe_ice_handler(void);
static int ecdw_breathe_poison_handler(void);
static int ecdw_breathe_magic_handler(void);
static int ecdw_breathe_lightning_handler(void);
static int ecdw_breathe_wind_handler(void);
static int ecdw_life_drain_handler(void);
static int ecdw_tptr_handler(void);
static int ecdw_harv_radon_handler(void);
static int ecdw_harv_cavern_wall_handler(void);
static int ecdw_harv_mother_nature_handler(void);
static int ecdw_harv_queen_handler(void);
static int ecdw_summon_rabbit_handler(void);
static int ecdw_summon_rat_handler(void);
static int ecdw_summon_beaver_handler(void);
static int ecdw_summon_skunk_handler(void);
static int ecdw_summon_racoon_handler(void);
static int ecdw_summon_deer_handler(void);
static int ecdw_summon_green_snake_handler(void);
static int ecdw_summon_red_snake_handler(void);
static int ecdw_summon_brown_snake_handler(void);
static int ecdw_summon_fox_handler(void);
static int ecdw_summon_boar_handler(void);
static int ecdw_summon_wolf_handler(void);
static int ecdw_summon_skeleton_handler(void);
static int ecdw_summon_small_garg_handler(void);
static int ecdw_summon_medium_garg_handler(void);
static int ecdw_summon_large_garg_handler(void);
static int ecdw_summon_puma_handler(void);
static int ecdw_summon_fem_gob_handler(void);
static int ecdw_summon_polar_bear_handler(void);
static int ecdw_summon_bear_handler(void);
static int ecdw_summon_armed_male_gob_handler(void);
static int ecdw_summon_armed_skeleton_handler(void);
static int ecdw_summon_fem_orc_handler(void);
static int ecdw_summon_male_orc_handler(void);
static int ecdw_summon_armed_fem_orc_handler(void);
static int ecdw_summon_armed_male_orc_handler(void);
static int ecdw_summon_cyclops_handler(void);
static int ecdw_summon_fluffy_handler(void);
static int ecdw_summon_phantom_warrior_handler(void);
static int ecdw_summon_mchim_handler(void);
static int ecdw_summon_yeti_handler(void);
static int ecdw_summon_achim_handler(void);
static int ecdw_summon_giant_handler(void);
static int ecdw_summon_giant_snake_handler(void);
static int ecdw_summon_spider_handler(void);
static int ecdw_summon_tiger_handler(void);
static int ecdw_level_up_att_handler(void);
static int ecdw_level_up_def_handler(void);
static int ecdw_mine_small_detonate_handler(void);
static int ecdw_mine_medium_detonate_handler(void);
static int ecdw_mine_trap_detonate_handler(void);
static int ecdw_mine_caltrop_detonate_handler(void);
static int ecdw_mine_poisoned_caltrop_detonate_handler(void);
static int ecdw_mine_mana_drainer_detonate_handler(void);
static int ecdw_mine_mana_burner_detonate_handler(void);
static int ecdw_mine_uninvisibilizer_detonate_handler(void);
static int ecdw_mine_magic_immunity_removal_detonate_handler(void);
static int ecdw_remote_smite_summons_handler(void);
static int ecdw_ongoing_clear_handler(void);
static int ecdw_ongoing_magic_immunity_handler(void);
static int ecdw_ongoing_magic_protection_handler(void);
static int ecdw_ongoing_poison_handler(void);
static int ecdw_ongoing_shield_handler(void);
static int ecdw_ongoing_harvesting_handler(void);
static int ecdw_level_up_har_handler(void);
static int ecdw_level_up_alc_handler(void);
static int ecdw_level_up_mag_handler(void);
static int ecdw_level_up_pot_handler(void);
static int ecdw_level_up_sum_handler(void);
static int ecdw_level_up_man_handler(void);
static int ecdw_level_up_cra_handler(void);
static int ecdw_level_up_eng_handler(void);
static int ecdw_level_up_tai_handler(void);
static int ecdw_level_up_ran_handler(void);
static int ecdw_normal_arrow_handler(int type);
static int ecdw_magic_arrow_handler(int type);
static int ecdw_fire_arrow_handler(int type);
static int ecdw_ice_arrow_handler(int type);
static int ecdw_explosive_arrow_handler(int type);
static int ecdw_harv_tool_break_handler(void);
static int ecdw_wind_leaves_handler(void);
static int ecdw_clouds_handler(void);

void display_ecdebugwin(void)
{
	int ecdebug_win = get_id_MW(MW_ECDEBUG);

	if (ecdebug_win < 0) // create window
	{
		// at least scale to using the current value
		// we don't really need it to be dynamic and that would be a lot of code
		float current_scale = get_global_scale();
		int ecdebug_win_width = (int)(0.5 + current_scale * (512 + ELW_BOX_SIZE));
		int ecdebug_win_height = (int)(0.5 + current_scale * 214);
		int button_width = (int)(0.5 + current_scale * 160);
		int button_x = (int)(0.5 + current_scale * 8);
		int button_y = (int)(0.5 + current_scale * 8);
		int button_x_shift = (int)(0.5 + current_scale * 168);
		int button_y_shift = (int)(0.5 + current_scale * 36);

		// tab ids
		int ecdw_tab_collection = -1;
		int tab_self = 12100;
		int tab_remote = 12101;
		int tab_harv = 12102;
		int tab_lvlup = 12103;
		int tab_misc = 12104;
		int tab_breath = 12105;
		int tab_summon = 12106;
		int tab_summon2 = 12107;
		int tab_summon3 = 12108;
		int tab_mines = 12109;
		int tab_arrows = 12110;

		// widget ids
		int ecdw_restoration_button_id = 11100;
		int ecdw_shield_button_id = 11101;
		int ecdw_coldshield_button_id = 11102;
		int ecdw_heatshield_button_id = 11103;
		int ecdw_radiationshield_button_id = 11104;
		int ecdw_heal_button_id = 11105;
		int ecdw_b2g_button_id = 11106;
		int ecdw_magic_immunity_button_id = 11107;
		int ecdw_remote_heal_button_id = 11108;
		int ecdw_poison_button_id = 11109;
		int ecdw_harm_button_id = 11110;
		int ecdw_mana_drain_button_id = 11111;
		int ecdw_alert_button_id = 11112;
		int ecdw_harv_rare_stone_button_id = 11113;
		int ecdw_mine_high_exp_detonate_button_id = 11114;
		int ecdw_harv_goldbag_button_id = 11115;
		int ecdw_harv_bee_button_id = 11116;
		int ecdw_level_up_oa_button_id = 11117;
		int ecdw_magic_protection_button_id = 11118;
		int ecdw_tptpr_button_id = 11119;
		int ecdw_breathe_fire_button_id = 11120;
		int ecdw_breathe_ice_button_id = 11121;
		int ecdw_breathe_poison_button_id = 11122;
		int ecdw_breathe_magic_button_id = 11123;
		int ecdw_breathe_lightning_button_id = 11124;
		int ecdw_breathe_wind_button_id = 11125;
		int ecdw_life_drain_button_id = 11126;
		int ecdw_tptr_button_id = 11127;
		int ecdw_harv_radon_button_id = 11128;
		int ecdw_harv_cavern_wall_button_id = 11129;
		int ecdw_harv_mother_nature_button_id = 11130;
		int ecdw_harv_queen_button_id = 11131;
		int ecdw_summon_rabbit_button_id = 11132;
		int ecdw_summon_rat_button_id = 11133;
		int ecdw_summon_beaver_button_id = 11134;
		int ecdw_summon_skunk_button_id = 11135;
		int ecdw_summon_racoon_button_id = 11136;
		int ecdw_summon_deer_button_id = 11137;
		int ecdw_summon_green_snake_button_id = 11138;
		int ecdw_summon_red_snake_button_id = 11139;
		int ecdw_summon_brown_snake_button_id = 11140;
		int ecdw_summon_fox_button_id = 11141;
		int ecdw_summon_boar_button_id = 11142;
		int ecdw_summon_wolf_button_id = 11143;
		int ecdw_summon_skeleton_button_id = 11144;
		int ecdw_summon_small_garg_button_id = 11145;
		int ecdw_summon_medium_garg_button_id = 11146;
		int ecdw_summon_large_garg_button_id = 11147;
		int ecdw_summon_puma_button_id = 11148;
		int ecdw_summon_fem_gob_button_id = 11149;
		int ecdw_summon_polar_bear_button_id = 11150;
		int ecdw_summon_bear_button_id = 11151;
		int ecdw_summon_armed_male_gob_button_id = 11152;
		int ecdw_summon_armed_skeleton_button_id = 11153;
		int ecdw_summon_fem_orc_button_id = 11154;
		int ecdw_summon_male_orc_button_id = 11155;
		int ecdw_summon_armed_fem_orc_button_id = 11156;
		int ecdw_summon_armed_male_orc_button_id = 11157;
		int ecdw_summon_cyclops_button_id = 11158;
		int ecdw_summon_fluffy_button_id = 11159;
		int ecdw_summon_phantom_warrior_button_id = 11160;
		int ecdw_summon_mchim_button_id = 11161;
		int ecdw_summon_yeti_button_id = 11162;
		int ecdw_summon_achim_button_id = 11163;
		int ecdw_summon_giant_button_id = 11164;
		int ecdw_summon_giant_snake_button_id = 11165;
		int ecdw_summon_spider_button_id = 11166;
		int ecdw_summon_tiger_button_id = 11167;
		int ecdw_level_up_att_button_id = 11168;
		int ecdw_level_up_def_button_id = 11169;
		int ecdw_mine_small_detonate_button_id = 11170;
		int ecdw_mine_medium_detonate_button_id = 11171;
		int ecdw_mine_trap_detonate_button_id = 11172;
		int ecdw_mine_caltrop_detonate_button_id = 11173;
		int ecdw_mine_poisoned_caltrop_detonate_button_id = 11174;
		int ecdw_mine_mana_drainer_detonate_button_id = 11176;
		int ecdw_mine_mana_burner_detonate_button_id = 11177;
		int ecdw_mine_uninvisibilizer_detonate_button_id = 11178;
		int ecdw_mine_magic_immunity_removal_detonate_button_id = 11179;
		int ecdw_remote_smite_summons_button_id = 11180;
		int ecdw_ongoing_clear_button_id = 11181;
		int ecdw_ongoing_magic_immunity_button_id = 11182;
		int ecdw_ongoing_magic_protection_button_id = 11183;
		int ecdw_ongoing_poison_button_id = 11184;
		int ecdw_ongoing_shield_button_id = 11185;
		int ecdw_ongoing_harvesting_button_id = 11186;
		int ecdw_level_up_har_button_id = 11187;
		int ecdw_level_up_alc_button_id = 11188;
		int ecdw_level_up_mag_button_id = 11189;
		int ecdw_level_up_pot_button_id = 11190;
		int ecdw_level_up_sum_button_id = 11191;
		int ecdw_level_up_man_button_id = 11192;
		int ecdw_level_up_cra_button_id = 11193;
		int ecdw_level_up_eng_button_id = 11194;
		int ecdw_level_up_tai_button_id = 11195;
		int ecdw_level_up_ran_button_id = 11196;
		int ecdw_normal_arrow_button_id = 11197;
		int ecdw_magic_arrow_button_id = 11198;
		int ecdw_fire_arrow_button_id = 11199;
		int ecdw_ice_arrow_button_id = 11200;
		int ecdw_explosive_arrow_button_id = 11201;
		int ecdw_harv_tool_break_button_id = 11202;
		int ecdw_wind_leaves_button_id = 11203;
		int ecdw_clouds_button_id = 12204;
		int ecdw_tabcollection_id = 12205;

		// init window
		ecdebug_win = create_window("Eye Candy DEBUG", (not_on_top_now(MW_ECDEBUG) ?game_root_win : -1),
			0, get_pos_x_MW(MW_ECDEBUG), get_pos_y_MW(MW_ECDEBUG),
			ecdebug_win_width, ecdebug_win_height, ELW_WIN_DEFAULT|ELW_USE_UISCALE);
		if (ecdebug_win < 0 || ecdebug_win >= windows_list.num_windows)
			return;
		set_id_MW(MW_ECDEBUG, ecdebug_win);

		// create tab collection
		// 24 pixels offset down so we don't cover the [X] in the upper right corner
		ecdw_tab_collection = tab_collection_add_extended(ecdebug_win, ecdw_tabcollection_id,
			NULL, 0, 0, ecdebug_win_width - windows_list.window[ecdebug_win].box_size,
			ecdebug_win_height, 0, DEFAULT_SMALL_RATIO * current_scale, 0, ELW_BOX_SIZE);

		// create tabs
		tab_self = tab_add(ecdebug_win, ecdw_tab_collection, "self", 0, 0, 0);
		tab_remote = tab_add(ecdebug_win, ecdw_tab_collection, "remote", 0, 0, 0);
		tab_harv = tab_add(ecdebug_win, ecdw_tab_collection, "harv", 0, 0, 0);
		tab_lvlup = tab_add(ecdebug_win, ecdw_tab_collection, "lvl up", 0, 0, 0);
		tab_mines = tab_add(ecdebug_win, ecdw_tab_collection, "mines", 0, 0, 0);
		tab_breath = tab_add(ecdebug_win, ecdw_tab_collection, "breath", 0, 0, 0);
		tab_summon = tab_add(ecdebug_win, ecdw_tab_collection, "summon1", 0, 0, 0);
		tab_summon2
			= tab_add(ecdebug_win, ecdw_tab_collection, "summon2", 0, 0, 0);
		tab_summon3
			= tab_add(ecdebug_win, ecdw_tab_collection, "summon3", 0, 0, 0);
		tab_misc = tab_add(ecdebug_win, ecdw_tab_collection, "misc", 0, 0, 0);
		tab_arrows = tab_add(ecdebug_win, ecdw_tab_collection, "arrows", 0, 0, 0);

		// create buttons

		// self magic buttons
		ecdw_restoration_button_id = button_add_extended(tab_self,
			ecdw_restoration_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Restoration");
		ecdw_shield_button_id = button_add_extended(tab_self,
			ecdw_shield_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Shield");
		ecdw_coldshield_button_id = button_add_extended(tab_self,
			ecdw_coldshield_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Cold Shield");
		ecdw_heatshield_button_id = button_add_extended(tab_self,
			ecdw_heatshield_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Heat Shield");
		ecdw_radiationshield_button_id = button_add_extended(tab_self,
			ecdw_radiationshield_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Rad. Shield");
		ecdw_heal_button_id = button_add_extended(tab_self,
			ecdw_heal_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Heal");
		ecdw_b2g_button_id = button_add_extended(tab_self, ecdw_b2g_button_id, 
		NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Bones2Gold");
		ecdw_magic_immunity_button_id = button_add_extended(tab_self,
			ecdw_magic_immunity_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Magic Immu.");
		ecdw_magic_protection_button_id = button_add_extended(tab_self,
			ecdw_magic_protection_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Magic Prot.");
		ecdw_tptpr_button_id = button_add_extended(tab_self,
			ecdw_tptpr_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "TP to PR");

		// remote magic buttons
		ecdw_remote_heal_button_id = button_add_extended(tab_remote,
			ecdw_remote_heal_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Remote Heal");
		ecdw_poison_button_id = button_add_extended(tab_remote,
			ecdw_poison_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Poison");
		ecdw_harm_button_id = button_add_extended(tab_remote,
			ecdw_harm_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Harm");
		ecdw_mana_drain_button_id = button_add_extended(tab_remote,
			ecdw_mana_drain_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Drain Mana");
		ecdw_life_drain_button_id = button_add_extended(tab_remote,
			ecdw_life_drain_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Life Drain");
		ecdw_tptr_button_id = button_add_extended(tab_remote,
			ecdw_tptr_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "TP to R");
		ecdw_remote_smite_summons_button_id = button_add_extended(tab_remote,
			ecdw_remote_smite_summons_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Smite Summons");

		// harv effect buttons
		ecdw_harv_rare_stone_button_id = button_add_extended(tab_harv,
			ecdw_harv_rare_stone_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Rare Stone");
		ecdw_harv_goldbag_button_id = button_add_extended(tab_harv,
			ecdw_harv_goldbag_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Gold Bag");
		ecdw_harv_bee_button_id = button_add_extended(tab_harv,
			ecdw_harv_bee_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Bees");
		ecdw_harv_radon_button_id = button_add_extended(tab_harv,
			ecdw_harv_radon_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Radon");
		ecdw_harv_cavern_wall_button_id = button_add_extended(tab_harv,
			ecdw_harv_cavern_wall_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Cavern Wall");
		ecdw_harv_mother_nature_button_id = button_add_extended(tab_harv,
			ecdw_harv_mother_nature_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Mother Nature");
		ecdw_harv_queen_button_id = button_add_extended(tab_harv,
			ecdw_harv_queen_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Queen");
		ecdw_harv_tool_break_button_id = button_add_extended(tab_harv,
			ecdw_harv_tool_break_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Tool break");

		// level up effect buttons
		ecdw_level_up_oa_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_oa_button_id, 
			NULL, button_x + button_x_shift / 2 * 0, button_y + button_y_shift
				* 0, button_width / 2, 0, 0, current_scale, "OA");
		ecdw_level_up_att_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_att_button_id, 
			NULL, button_x + button_x_shift / 2 * 0, button_y + button_y_shift
				* 1, button_width / 2, 0, 0, current_scale, "ATT");
		ecdw_level_up_def_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_def_button_id, 
			NULL, button_x + button_x_shift / 2 * 0, button_y + button_y_shift
				* 2, button_width / 2, 0, 0, current_scale, "DEF");
		ecdw_level_up_har_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_har_button_id, 
			NULL, button_x + button_x_shift / 2 * 0, button_y + button_y_shift
				* 3, button_width / 2, 0, 0, current_scale, "HAR");
		ecdw_level_up_alc_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_alc_button_id, 
			NULL, button_x + button_x_shift / 2 * 1, button_y + button_y_shift
				* 0, button_width / 2, 0, 0, current_scale, "ALC");
		ecdw_level_up_mag_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_mag_button_id, 
			NULL, button_x + button_x_shift / 2 * 1, button_y + button_y_shift
				* 1, button_width / 2, 0, 0, current_scale, "MAG");
		ecdw_level_up_pot_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_pot_button_id, 
			NULL, button_x + button_x_shift / 2 * 1, button_y + button_y_shift
				* 2, button_width / 2, 0, 0, current_scale, "POT");
		ecdw_level_up_sum_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_sum_button_id, 
			NULL, button_x + button_x_shift / 2 * 1, button_y + button_y_shift
				* 3, button_width / 2, 0, 0, current_scale, "SUM");
		ecdw_level_up_man_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_man_button_id, 
			NULL, button_x + button_x_shift / 2 * 2, button_y + button_y_shift
				* 0, button_width / 2, 0, 0, current_scale, "MAN");
		ecdw_level_up_cra_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_cra_button_id, 
			NULL, button_x + button_x_shift / 2 * 2, button_y + button_y_shift
				* 1, button_width / 2, 0, 0, current_scale, "CRA");
		ecdw_level_up_eng_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_eng_button_id, 
			NULL, button_x + button_x_shift / 2 * 2, button_y + button_y_shift
				* 2, button_width / 2, 0, 0, current_scale, "ENG");
		ecdw_level_up_tai_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_tai_button_id, 
			NULL, button_x + button_x_shift / 2 * 2, button_y + button_y_shift
				* 3, button_width / 2, 0, 0, current_scale, "TAI");
		ecdw_level_up_ran_button_id = button_add_extended(tab_lvlup,
			ecdw_level_up_ran_button_id, 
			NULL, button_x + button_x_shift / 2 * 3, button_y + button_y_shift
				* 0, button_width / 2, 0, 0, current_scale, "RAN");

		// mines effect buttons
		ecdw_mine_high_exp_detonate_button_id = button_add_extended(tab_mines,
			ecdw_mine_high_exp_detonate_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "High exp. Mine");
		ecdw_mine_small_detonate_button_id = button_add_extended(tab_mines,
			ecdw_mine_small_detonate_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Small Mine");
		ecdw_mine_medium_detonate_button_id = button_add_extended(tab_mines,
			ecdw_mine_medium_detonate_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Medium Mine");
		ecdw_mine_trap_detonate_button_id = button_add_extended(tab_mines,
			ecdw_mine_trap_detonate_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Trap");
		ecdw_mine_caltrop_detonate_button_id = button_add_extended(tab_mines,
			ecdw_mine_caltrop_detonate_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Caltrop");
		ecdw_mine_poisoned_caltrop_detonate_button_id = button_add_extended(
			tab_mines, ecdw_mine_poisoned_caltrop_detonate_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Pois. Caltrop");
		ecdw_mine_mana_drainer_detonate_button_id = button_add_extended(
			tab_mines, ecdw_mine_mana_drainer_detonate_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Mana Drainer");
		ecdw_mine_mana_burner_detonate_button_id = button_add_extended(
			tab_mines, ecdw_mine_mana_burner_detonate_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Mana Burner");
		ecdw_mine_uninvisibilizer_detonate_button_id = button_add_extended(
			tab_mines, ecdw_mine_uninvisibilizer_detonate_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Uninvis.");
		ecdw_mine_magic_immunity_removal_detonate_button_id
			= button_add_extended(tab_mines,
				ecdw_mine_magic_immunity_removal_detonate_button_id, 
				NULL, button_x + button_x_shift * 2, button_y + button_y_shift
					* 1, button_width, 0, 0, current_scale, "Mag. Immu. Rem.");

		// breath effect buttons
		ecdw_breathe_fire_button_id = button_add_extended(tab_breath,
			ecdw_breathe_fire_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Fire");
		ecdw_breathe_ice_button_id = button_add_extended(tab_breath,
			ecdw_breathe_ice_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Ice");
		ecdw_breathe_magic_button_id = button_add_extended(tab_breath,
			ecdw_breathe_magic_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Magic");
		ecdw_breathe_poison_button_id = button_add_extended(tab_breath,
			ecdw_breathe_poison_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Poison");
		ecdw_breathe_lightning_button_id = button_add_extended(tab_breath,
			ecdw_breathe_lightning_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Lightning");
		ecdw_breathe_wind_button_id = button_add_extended(tab_breath,
			ecdw_breathe_wind_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Wind");

		// summon effect buttons
		ecdw_summon_rabbit_button_id = button_add_extended(tab_summon,
			ecdw_summon_rabbit_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Rabbit");
		ecdw_summon_rat_button_id = button_add_extended(tab_summon,
			ecdw_summon_rat_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Rat");
		ecdw_summon_beaver_button_id = button_add_extended(tab_summon,
			ecdw_summon_beaver_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Beaver");
		ecdw_summon_skunk_button_id = button_add_extended(tab_summon,
			ecdw_summon_skunk_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Skunk");
		ecdw_summon_racoon_button_id = button_add_extended(tab_summon,
			ecdw_summon_racoon_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Racoon");
		ecdw_summon_deer_button_id = button_add_extended(tab_summon,
			ecdw_summon_deer_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Deer");
		ecdw_summon_green_snake_button_id = button_add_extended(tab_summon,
			ecdw_summon_green_snake_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Green Snake");
		ecdw_summon_red_snake_button_id = button_add_extended(tab_summon,
			ecdw_summon_red_snake_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Red Snake");
		ecdw_summon_brown_snake_button_id = button_add_extended(tab_summon,
			ecdw_summon_brown_snake_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Brown Snake");
		ecdw_summon_fox_button_id = button_add_extended(tab_summon,
			ecdw_summon_fox_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Fox");
		ecdw_summon_boar_button_id = button_add_extended(tab_summon,
			ecdw_summon_boar_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Boar");
		ecdw_summon_wolf_button_id = button_add_extended(tab_summon,
			ecdw_summon_wolf_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Wolf");
		// summon effects tab #2
		ecdw_summon_skeleton_button_id = button_add_extended(tab_summon2,
			ecdw_summon_skeleton_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Skeleton");
		ecdw_summon_small_garg_button_id = button_add_extended(tab_summon2,
			ecdw_summon_small_garg_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Small Garg");
		ecdw_summon_medium_garg_button_id = button_add_extended(tab_summon2,
			ecdw_summon_medium_garg_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Medium Garg");
		ecdw_summon_large_garg_button_id = button_add_extended(tab_summon2,
			ecdw_summon_large_garg_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Large Garg");
		ecdw_summon_puma_button_id = button_add_extended(tab_summon2,
			ecdw_summon_puma_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Puma");
		ecdw_summon_fem_gob_button_id = button_add_extended(tab_summon2,
			ecdw_summon_fem_gob_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Fem Gob");
		ecdw_summon_polar_bear_button_id = button_add_extended(tab_summon2,
			ecdw_summon_polar_bear_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Polar Bear");
		ecdw_summon_bear_button_id = button_add_extended(tab_summon2,
			ecdw_summon_bear_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Bear");
		ecdw_summon_armed_male_gob_button_id = button_add_extended(tab_summon2,
			ecdw_summon_armed_male_gob_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Armed M Gob");
		ecdw_summon_armed_skeleton_button_id = button_add_extended(tab_summon2,
			ecdw_summon_armed_skeleton_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Armed Skel");
		ecdw_summon_fem_orc_button_id = button_add_extended(tab_summon2,
			ecdw_summon_fem_orc_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Fem Orc");
		ecdw_summon_male_orc_button_id = button_add_extended(tab_summon2,
			ecdw_summon_male_orc_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Male Orc");
		// summon effects tab #3
		ecdw_summon_armed_fem_orc_button_id = button_add_extended(tab_summon3,
			ecdw_summon_armed_fem_orc_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Armed Fem Orc");
		ecdw_summon_armed_male_orc_button_id = button_add_extended(tab_summon3,
			ecdw_summon_armed_male_orc_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Armed M Orc");
		ecdw_summon_cyclops_button_id = button_add_extended(tab_summon3,
			ecdw_summon_cyclops_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Cyclops");
		ecdw_summon_fluffy_button_id = button_add_extended(tab_summon3,
			ecdw_summon_fluffy_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Fluffy");
		ecdw_summon_phantom_warrior_button_id = button_add_extended(
			tab_summon3, ecdw_summon_phantom_warrior_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Phantom W");
		ecdw_summon_mchim_button_id = button_add_extended(tab_summon3,
			ecdw_summon_mchim_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "M Chim");
		ecdw_summon_yeti_button_id = button_add_extended(tab_summon3,
			ecdw_summon_yeti_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Yeti");
		ecdw_summon_achim_button_id = button_add_extended(tab_summon3,
			ecdw_summon_achim_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "A Chim");
		ecdw_summon_giant_button_id = button_add_extended(tab_summon3,
			ecdw_summon_giant_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Giant");
		ecdw_summon_giant_snake_button_id = button_add_extended(tab_summon3,
			ecdw_summon_giant_snake_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "Sslessar");
		ecdw_summon_spider_button_id = button_add_extended(tab_summon3,
			ecdw_summon_spider_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "Spider");
		ecdw_summon_tiger_button_id = button_add_extended(tab_summon3,
			ecdw_summon_tiger_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "Tiger");

		// misc effect buttons
		ecdw_alert_button_id = button_add_extended(tab_misc,
			ecdw_alert_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "Alert");
		ecdw_wind_leaves_button_id = button_add_extended(tab_misc,
			ecdw_wind_leaves_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "wind leaves");
		ecdw_clouds_button_id = button_add_extended(tab_misc,
			ecdw_clouds_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "clouds");
		ecdw_ongoing_magic_immunity_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_magic_immunity_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "OG Mag Immu");
		ecdw_ongoing_magic_protection_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_magic_protection_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "OG Mag Prot");
		ecdw_ongoing_poison_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_poison_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "OG Poison");
		ecdw_ongoing_shield_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_shield_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "OG Shield");
		ecdw_ongoing_harvesting_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_harvesting_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "OG Harvest");
		ecdw_ongoing_clear_button_id = button_add_extended(tab_misc,
			ecdw_ongoing_clear_button_id, 
			NULL, button_x + button_x_shift * 2, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "clear OG");

		// arrow effect buttons
		ecdw_normal_arrow_button_id = button_add_extended(tab_arrows,
			ecdw_normal_arrow_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "normal");
		ecdw_magic_arrow_button_id = button_add_extended(tab_arrows,
			ecdw_magic_arrow_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 1,
			button_width, 0, 0, current_scale, "magic");
		ecdw_fire_arrow_button_id = button_add_extended(tab_arrows,
			ecdw_fire_arrow_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 2,
			button_width, 0, 0, current_scale, "fire");
		ecdw_ice_arrow_button_id = button_add_extended(tab_arrows,
			ecdw_ice_arrow_button_id, 
			NULL, button_x + button_x_shift * 0, button_y + button_y_shift * 3,
			button_width, 0, 0, current_scale, "ice");
		ecdw_explosive_arrow_button_id = button_add_extended(tab_arrows,
			ecdw_explosive_arrow_button_id, 
			NULL, button_x + button_x_shift * 1, button_y + button_y_shift * 0,
			button_width, 0, 0, current_scale, "explosive");

		// add button handlers

		// self magic handlers
		widget_set_OnClick(tab_self, ecdw_restoration_button_id,
			ecdw_restoration_handler);
		widget_set_OnClick(tab_self, ecdw_shield_button_id, ecdw_shield_handler);
		widget_set_OnClick(tab_self, ecdw_coldshield_button_id,
			ecdw_coldshield_handler);
		widget_set_OnClick(tab_self, ecdw_heatshield_button_id,
			ecdw_heatshield_handler);
		widget_set_OnClick(tab_self, ecdw_radiationshield_button_id,
			ecdw_radiationshield_handler);
		widget_set_OnClick(tab_self, ecdw_heal_button_id, ecdw_heal_handler);
		widget_set_OnClick(tab_self, ecdw_b2g_button_id, ecdw_b2g_handler);
		widget_set_OnClick(tab_self, ecdw_magic_immunity_button_id,
			ecdw_magic_immunity_handler);
		widget_set_OnClick(tab_self, ecdw_magic_protection_button_id,
			ecdw_magic_protection_handler);
		widget_set_OnClick(tab_self, ecdw_tptpr_button_id, ecdw_tptpr_handler);

		// remote magic handlers
		widget_set_OnClick(tab_remote, ecdw_remote_heal_button_id,
			ecdw_remote_heal_handler);
		widget_set_OnClick(tab_remote, ecdw_poison_button_id,
			ecdw_poison_handler);
		widget_set_OnClick(tab_remote, ecdw_harm_button_id, ecdw_harm_handler);
		widget_set_OnClick(tab_remote, ecdw_mana_drain_button_id,
			ecdw_mana_drain_handler);
		widget_set_OnClick(tab_remote, ecdw_life_drain_button_id,
			ecdw_life_drain_handler);
		widget_set_OnClick(tab_remote, ecdw_tptr_button_id, ecdw_tptr_handler);
		widget_set_OnClick(tab_remote, ecdw_remote_smite_summons_button_id,
			ecdw_remote_smite_summons_handler);

		// harv effect handlers
		widget_set_OnClick(tab_harv, ecdw_harv_rare_stone_button_id,
			ecdw_harv_rare_stone_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_goldbag_button_id,
			ecdw_harv_goldbag_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_bee_button_id,
			ecdw_harv_bee_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_radon_button_id,
			ecdw_harv_radon_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_cavern_wall_button_id,
			ecdw_harv_cavern_wall_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_mother_nature_button_id,
			ecdw_harv_mother_nature_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_queen_button_id,
			ecdw_harv_queen_handler);
		widget_set_OnClick(tab_harv, ecdw_harv_tool_break_button_id,
			ecdw_harv_tool_break_handler);

		// level up effect handlers
		widget_set_OnClick(tab_lvlup, ecdw_level_up_oa_button_id,
			ecdw_level_up_oa_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_att_button_id,
			ecdw_level_up_att_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_def_button_id,
			ecdw_level_up_def_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_har_button_id,
			ecdw_level_up_har_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_alc_button_id,
			ecdw_level_up_alc_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_mag_button_id,
			ecdw_level_up_mag_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_pot_button_id,
			ecdw_level_up_pot_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_sum_button_id,
			ecdw_level_up_sum_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_man_button_id,
			ecdw_level_up_man_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_cra_button_id,
			ecdw_level_up_cra_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_eng_button_id,
			ecdw_level_up_eng_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_tai_button_id,
			ecdw_level_up_tai_handler);
		widget_set_OnClick(tab_lvlup, ecdw_level_up_ran_button_id,
			ecdw_level_up_ran_handler);

		// mines effect handlers
		widget_set_OnClick(tab_mines, ecdw_mine_high_exp_detonate_button_id,
			ecdw_mine_high_exp_detonate_handler);
		widget_set_OnClick(tab_mines, ecdw_mine_small_detonate_button_id,
			ecdw_mine_small_detonate_handler);
		widget_set_OnClick(tab_mines, ecdw_mine_medium_detonate_button_id,
			ecdw_mine_medium_detonate_handler);
		widget_set_OnClick(tab_mines, ecdw_mine_trap_detonate_button_id,
			ecdw_mine_trap_detonate_handler);
		widget_set_OnClick(tab_mines, ecdw_mine_caltrop_detonate_button_id,
			ecdw_mine_caltrop_detonate_handler);
		widget_set_OnClick(tab_mines,
			ecdw_mine_poisoned_caltrop_detonate_button_id,
			ecdw_mine_poisoned_caltrop_detonate_handler);
		widget_set_OnClick(tab_mines,
			ecdw_mine_mana_drainer_detonate_button_id,
			ecdw_mine_mana_drainer_detonate_handler);
		widget_set_OnClick(tab_mines, ecdw_mine_mana_burner_detonate_button_id,
			ecdw_mine_mana_burner_detonate_handler);
		widget_set_OnClick(tab_mines,
			ecdw_mine_uninvisibilizer_detonate_button_id,
			ecdw_mine_uninvisibilizer_detonate_handler);
		widget_set_OnClick(tab_mines,
			ecdw_mine_magic_immunity_removal_detonate_button_id,
			ecdw_mine_magic_immunity_removal_detonate_handler);

		// breath effect handlers
		widget_set_OnClick(tab_breath, ecdw_breathe_fire_button_id,
			ecdw_breathe_fire_handler);
		widget_set_OnClick(tab_breath, ecdw_breathe_ice_button_id,
			ecdw_breathe_ice_handler);
		widget_set_OnClick(tab_breath, ecdw_breathe_poison_button_id,
			ecdw_breathe_poison_handler);
		widget_set_OnClick(tab_breath, ecdw_breathe_magic_button_id,
			ecdw_breathe_magic_handler);
		widget_set_OnClick(tab_breath, ecdw_breathe_lightning_button_id,
			ecdw_breathe_lightning_handler);
		widget_set_OnClick(tab_breath, ecdw_breathe_wind_button_id,
			ecdw_breathe_wind_handler);

		// summon effect handlers
		widget_set_OnClick(tab_summon, ecdw_summon_rabbit_button_id,
			ecdw_summon_rabbit_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_rat_button_id,
			ecdw_summon_rat_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_beaver_button_id,
			ecdw_summon_beaver_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_skunk_button_id,
			ecdw_summon_skunk_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_racoon_button_id,
			ecdw_summon_racoon_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_deer_button_id,
			ecdw_summon_deer_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_green_snake_button_id,
			ecdw_summon_green_snake_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_red_snake_button_id,
			ecdw_summon_red_snake_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_brown_snake_button_id,
			ecdw_summon_brown_snake_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_fox_button_id,
			ecdw_summon_fox_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_boar_button_id,
			ecdw_summon_boar_handler);
		widget_set_OnClick(tab_summon, ecdw_summon_wolf_button_id,
			ecdw_summon_wolf_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_skeleton_button_id,
			ecdw_summon_skeleton_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_small_garg_button_id,
			ecdw_summon_small_garg_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_medium_garg_button_id,
			ecdw_summon_medium_garg_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_large_garg_button_id,
			ecdw_summon_large_garg_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_puma_button_id,
			ecdw_summon_puma_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_fem_gob_button_id,
			ecdw_summon_fem_gob_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_polar_bear_button_id,
			ecdw_summon_polar_bear_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_bear_button_id,
			ecdw_summon_bear_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_armed_male_gob_button_id,
			ecdw_summon_armed_male_gob_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_armed_skeleton_button_id,
			ecdw_summon_armed_skeleton_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_fem_orc_button_id,
			ecdw_summon_fem_orc_handler);
		widget_set_OnClick(tab_summon2, ecdw_summon_male_orc_button_id,
			ecdw_summon_male_orc_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_armed_fem_orc_button_id,
			ecdw_summon_armed_fem_orc_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_armed_male_orc_button_id,
			ecdw_summon_armed_male_orc_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_cyclops_button_id,
			ecdw_summon_cyclops_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_fluffy_button_id,
			ecdw_summon_fluffy_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_phantom_warrior_button_id,
			ecdw_summon_phantom_warrior_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_mchim_button_id,
			ecdw_summon_mchim_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_yeti_button_id,
			ecdw_summon_yeti_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_achim_button_id,
			ecdw_summon_achim_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_giant_button_id,
			ecdw_summon_giant_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_giant_snake_button_id,
			ecdw_summon_giant_snake_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_spider_button_id,
			ecdw_summon_spider_handler);
		widget_set_OnClick(tab_summon3, ecdw_summon_tiger_button_id,
			ecdw_summon_tiger_handler);

		// misc effect handlers
		widget_set_OnClick(tab_misc, ecdw_alert_button_id, ecdw_alert_handler);
		widget_set_OnClick(tab_misc, ecdw_wind_leaves_button_id, ecdw_wind_leaves_handler);
		widget_set_OnClick(tab_misc, ecdw_clouds_button_id, ecdw_clouds_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_clear_button_id,
			ecdw_ongoing_clear_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_magic_immunity_button_id,
			ecdw_ongoing_magic_immunity_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_magic_protection_button_id,
			ecdw_ongoing_magic_protection_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_poison_button_id,
			ecdw_ongoing_poison_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_shield_button_id,
			ecdw_ongoing_shield_handler);
		widget_set_OnClick(tab_misc, ecdw_ongoing_harvesting_button_id,
			ecdw_ongoing_harvesting_handler);

		// arrow effect handlers
		widget_set_OnClick(tab_arrows, ecdw_normal_arrow_button_id, ecdw_normal_arrow_handler);
		widget_set_OnClick(tab_arrows, ecdw_magic_arrow_button_id, ecdw_magic_arrow_handler);
		widget_set_OnClick(tab_arrows, ecdw_fire_arrow_button_id, ecdw_fire_arrow_handler);
		widget_set_OnClick(tab_arrows, ecdw_ice_arrow_button_id, ecdw_ice_arrow_handler);
		widget_set_OnClick(tab_arrows, ecdw_explosive_arrow_button_id, ecdw_explosive_arrow_handler);

		check_proportional_move(MW_ECDEBUG);

}
	else // display existing window
	{
		show_window(ecdebug_win);
		select_window(ecdebug_win);
	}
}

static int ecdw_restoration_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_restoration2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_shield_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_shield_generic(me, (poor_man ? 6 : 10), SPECIAL_EFFECT_SHIELD);
		release_actors_list();
	}
	return 1;
}

static int ecdw_coldshield_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_shield_generic(me, (poor_man ? 6 : 10), SPECIAL_EFFECT_COLDSHIELD);
		release_actors_list();
	}
	return 1;
}

static int ecdw_heatshield_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_shield_generic(me, (poor_man ? 6 : 10), SPECIAL_EFFECT_HEATSHIELD);
		release_actors_list();
	}
	return 1;
}

static int ecdw_radiationshield_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_shield_generic(me, (poor_man ? 6 : 10), SPECIAL_EFFECT_RADIATIONSHIELD);
		release_actors_list();
	}
	return 1;
}

static int ecdw_heal_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_heal2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_b2g_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_bones_to_gold2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_magic_immunity_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_magic_immunity2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_remote_heal_handler(void)
{
	actor *target;
	actor * me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_glow_remote_heal(me, (poor_man ? 6 : 10));
		ec_create_targetmagic_remote_heal2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_poison_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_glow_poison(me, (poor_man ? 6 : 10));
		ec_create_targetmagic_poison2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harm_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_glow_harm(me, (poor_man ? 6 : 10));
		ec_create_targetmagic_harm2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mana_drain_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_targetmagic_drain_mana2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_alert_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_alert2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_rare_stone_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_rare_stone2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_high_exp_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_HIGH_EXPLOSIVE_MINE, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_goldbag_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_bag_of_gold2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_bee_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_bees2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_oa_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_oa(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_magic_protection_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_magic_protection2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_tptpr_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_selfmagic_teleport_to_the_portals_room2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_fire_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_fire2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_ice_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_ice2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_poison_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_poison2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_magic_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_magic2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_lightning_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_lightning2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_breathe_wind_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_breath_wind2(me, target, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_life_drain_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_targetmagic_life_drain2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_tptr_handler(void)
{
	actor *target;
	actor *me = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		ec_create_targetmagic_teleport_to_range2(me, target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_radon_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_radon_pouch2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_cavern_wall_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_cavern_wall2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_mother_nature_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_mother_nature2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_queen_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_queen_of_nature2(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_rabbit_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_rabbit2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_rat_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_rat2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_beaver_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_beaver2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_skunk_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_skunk2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_racoon_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_racoon2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_deer_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_deer2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_green_snake_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_green_snake2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_red_snake_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_red_snake2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_brown_snake_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_brown_snake2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_fox_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_fox2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_boar_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_boar2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_wolf_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_wolf2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_skeleton_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_skeleton2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_small_garg_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_small_gargoyle2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_medium_garg_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_medium_gargoyle2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_large_garg_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_large_gargoyle2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_puma_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_puma2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_fem_gob_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_female_goblin2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_polar_bear_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_polar_bear2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_bear_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_bear2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_armed_male_gob_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_armed_male_goblin2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_armed_skeleton_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_armed_skeleton2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_fem_orc_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_female_orc2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_male_orc_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_male_orc2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_armed_fem_orc_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_armed_female_orc2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_armed_male_orc_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_armed_male_orc2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_cyclops_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_cyclops2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_fluffy_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_fluffy2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_phantom_warrior_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_phantom_warrior2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_mchim_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_mountain_chimeran2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_yeti_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_yeti2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_achim_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_arctic_chimeran2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_giant_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_giant2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_giant_snake_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_giant_snake2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_spider_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_spider2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_summon_tiger_handler(void)
{
	actor *target = lock_and_get_target();
	if (target != NULL)
	{
		ec_create_summon_tiger2(target, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_att_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_att(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_def_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_def(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_small_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_SMALL_MINE, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_medium_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_MEDIUM_MINE, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_trap_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_TRAP, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_caltrop_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_CALTROP, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_poisoned_caltrop_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_POISONED_CALTROP, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_mana_drainer_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_MANA_DRAINER, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_mana_burner_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_MANA_BURNER, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_uninvisibilizer_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_UNINVIZIBILIZER, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_mine_magic_immunity_removal_detonate_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_mine_detonate2(me, MINE_TYPE_MAGIC_IMMUNITY_REMOVAL, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

struct remote_smite_summons_info
{
	int target_count;
	ec_reference ref;
};

static void remote_smite_summons_target(actor *act, void* data, locked_list_ptr actors_list)
{
	struct remote_smite_summons_info *info = data;
	if (act != get_self(actors_list))
	{
		ec_add_target(info->ref, act->x_pos, act->y_pos, 1.5);
		++info->target_count;
	}
}

// TODO!
static int ecdw_remote_smite_summons_handler(void)
{
	struct remote_smite_summons_info info = { .target_count = 0, .ref = ec_create_generic() };
	locked_list_ptr actors_list = get_locked_actors_list();

	for_each_actor(actors_list, remote_smite_summons_target, &info);
	if (info.target_count > 0)
	{
		actor *me = get_self(actors_list);
		ec_launch_targetmagic_smite_summoned(info.ref, me->x_pos, me->y_pos, 1.5,
			(poor_man ? 6 : 10));
	}

	release_locked_actors_list(actors_list);
	return 1;
}

static int ecdw_ongoing_magic_protection_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_ongoing_magic_protection2(me, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_ongoing_shield_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_ongoing_shield2(me, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_ongoing_magic_immunity_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_ongoing_magic_immunity2(me, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_ongoing_poison_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_ongoing_poison2(me, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_ongoing_clear_handler(void)
{
	ec_delete_effect_type(EC_ONGOING);
	return 1;
}

static int ecdw_ongoing_harvesting_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_ongoing_harvesting2(me, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_har_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_har(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_alc_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_alc_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_alc_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_mag_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_mag(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_pot_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_pot_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_pot_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_sum_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_sum(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_man_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_man_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_man_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_cra_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_cra_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_cra_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_eng_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_eng_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_eng_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_tai_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_tai_left(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_tai_right(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_level_up_ran_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_glow_level_up_default(me, (poor_man ? 6 : 10));
		ec_create_glow_level_up_ran(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_normal_arrow_handler(int type)
{
	float origin_f[3];
	float target_f[3];
	actor *target;
	actor *origin = lock_and_get_self_and_target(&target);

	if (target != NULL)
	{
		cal_get_actor_bone_absolute_position(origin, get_actor_bone_id(origin, body_top_bone), NULL, origin_f);
		cal_get_actor_bone_absolute_position(target, get_actor_bone_id(target, body_top_bone), NULL, target_f);
		missiles_add(0, origin_f, target_f, 0.0, 0);

		release_actors_list();
	}
	return 1;
}

static int ecdw_magic_arrow_handler(int type)
{
	float origin_f[3];
	float target_f[3];
	actor *target;
	actor *origin = lock_and_get_self_and_target(&target);

	if (target != NULL)
	{
		cal_get_actor_bone_absolute_position(origin, get_actor_bone_id(origin, body_top_bone), NULL, origin_f);
		cal_get_actor_bone_absolute_position(target, get_actor_bone_id(target, body_top_bone), NULL, target_f);
		missiles_add(1, origin_f, target_f, 0.0, 0);

		release_actors_list();
	}
	return 1;
}

static int ecdw_fire_arrow_handler(int type)
{
	float origin_f[3];
	float target_f[3];
	actor *target;
	actor *origin = lock_and_get_self_and_target(&target);

	if (target != NULL)
	{
		cal_get_actor_bone_absolute_position(origin, get_actor_bone_id(origin, body_top_bone), NULL, origin_f);
		cal_get_actor_bone_absolute_position(target, get_actor_bone_id(target, body_top_bone), NULL, target_f);
		missiles_add(2, origin_f, target_f, 0.0, 0);

		release_actors_list();
	}
	return 1;
}

static int ecdw_ice_arrow_handler(int type)
{
	float origin_f[3];
	float target_f[3];
	actor *target;
	actor *origin = lock_and_get_self_and_target(&target);
	if (target != NULL)
	{
		cal_get_actor_bone_absolute_position(origin, get_actor_bone_id(origin, body_top_bone), NULL, origin_f);
		cal_get_actor_bone_absolute_position(target, get_actor_bone_id(target, body_top_bone), NULL, target_f);
		missiles_add(3, origin_f, target_f, 0.0, 0);

		release_actors_list();
	}
	return 1;
}

static int ecdw_explosive_arrow_handler(int type)
{
	float origin_f[3];
	float target_f[3];
	actor *target;
	actor *origin = lock_and_get_self_and_target(&target);

	if (target != NULL)
	{
		cal_get_actor_bone_absolute_position(origin, get_actor_bone_id(origin, body_top_bone), NULL, origin_f);
		cal_get_actor_bone_absolute_position(target, get_actor_bone_id(target, body_top_bone), NULL, target_f);
		missiles_add(4, origin_f, target_f, 0.0, 0);

		release_actors_list();
	}
	return 1;
}

static int ecdw_harv_tool_break_handler(void)
{
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_create_harvesting_tool_break(me, (poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

static int ecdw_wind_leaves_handler(void)
{
	ec_bounds *bounds = ec_create_bounds_list();
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_add_smooth_polygon_bound(bounds, 2.0, 2.5);
		ec_create_wind_leaves(me->x_pos, me->y_pos, me->z_pos, 1.0, 1.0, 1.0, 0.1, bounds,
			me->x_pos + 3.0, me->y_pos + 5.0, me->z_pos);
		release_actors_list();
	}
	return 1;
}

static int ecdw_clouds_handler(void)
{
	ec_bounds *bounds = ec_create_bounds_list();
	actor *me = lock_and_get_self();
	if (me)
	{
		ec_add_smooth_polygon_bound(bounds, 2.0, 2.5);
		ec_create_cloud(me->x_pos, me->y_pos, me->z_pos, 1.0, 1.0, 0.1, bounds,
			(poor_man ? 6 : 10));
		release_actors_list();
	}
	return 1;
}

#endif // ECDEBUGWIN
