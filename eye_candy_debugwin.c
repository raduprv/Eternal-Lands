#ifdef ECDEBUGWIN

#include "eye_candy_debugwin.h"

#include "actors.h"
#include "client_serv.h"
#include "eye_candy_wrapper.h"
#include "gamewin.h"
#include "hud.h"
#include "init.h"

#ifdef MINES
#include "mines.h"
#endif

int ecdebug_win = -1;

int ecdw_restoration_button_id = 100;
int ecdw_shield_button_id = 101;
int ecdw_coldshield_button_id = 102;
int ecdw_heatshield_button_id = 103;
int ecdw_radiationshield_button_id = 104;
int ecdw_heal_button_id = 105;
int ecdw_b2g_button_id = 106;
int ecdw_magic_immunity_button_id = 107;
int ecdw_remote_heal_button_id = 108;
int ecdw_poison_button_id = 109;
int ecdw_harm_button_id = 110;
int ecdw_mana_drain_button_id = 111;
int ecdw_alert_button_id = 112;
int ecdw_harv_rare_stone_button_id = 113;
int ecdw_mine_detonate_button_id = 114;
int ecdw_harv_goldbag_button_id = 115;
int ecdw_harv_bee_button_id = 116;

int ecdw_restoration_handler();
int ecdw_shield_handler();
int ecdw_coldshield_handler();
int ecdw_heatshield_handler();
int ecdw_radiationshield_handler();
int ecdw_heal_handler();
int ecdw_b2g_handler();
int ecdw_magic_immunity_handler();
int ecdw_remote_heal_handler();
int ecdw_poison_handler();
int ecdw_harm_handler();
int ecdw_mana_drain_handler();
int ecdw_alert_handler();
int ecdw_harv_rare_stone_handler();
int ecdw_mine_detonate_handler();
int ecdw_harv_goldbag_handler();
int ecdw_harv_bee_handler();

void display_ecdebugwin()
{
	if(ecdebug_win < 0)
	{
		// init window
		ecdebug_win = create_window("Eye Candy DEBUG", windows_on_top?-1:game_root_win, 0, 16, 16, 400+ELW_BOX_SIZE, 512+1, ELW_WIN_DEFAULT);
		
		// create buttons
		ecdw_restoration_button_id = button_add_extended(ecdebug_win, ecdw_restoration_button_id,
			NULL, 8, 8, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Restoration");
		ecdw_shield_button_id = button_add_extended(ecdebug_win, ecdw_shield_button_id,
			NULL, 8, 40, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Shield");
		ecdw_coldshield_button_id = button_add_extended(ecdebug_win, ecdw_coldshield_button_id,
			NULL, 8, 40+32, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Cold Shield");
		ecdw_heatshield_button_id = button_add_extended(ecdebug_win, ecdw_heatshield_button_id,
			NULL, 8, 40+64, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Heat Shield");
		ecdw_radiationshield_button_id = button_add_extended(ecdebug_win, ecdw_radiationshield_button_id,
			NULL, 8, 40+96, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Rad. Shield");
		ecdw_heal_button_id = button_add_extended(ecdebug_win, ecdw_heal_button_id,
			NULL, 8, 40+128, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Heal");
		ecdw_b2g_button_id = button_add_extended(ecdebug_win, ecdw_b2g_button_id,
			NULL, 8, 40+160, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Bones2Gold");
		ecdw_magic_immunity_button_id = button_add_extended(ecdebug_win, ecdw_magic_immunity_button_id,
			NULL, 8, 40+192, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Mag. Immu.");
		ecdw_remote_heal_button_id = button_add_extended(ecdebug_win, ecdw_remote_heal_button_id,
			NULL, 180, 8, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Remote Heal");
		ecdw_poison_button_id = button_add_extended(ecdebug_win, ecdw_poison_button_id,
			NULL, 180, 40, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Poison");
		ecdw_harm_button_id = button_add_extended(ecdebug_win, ecdw_harm_button_id,
			NULL, 180, 40+32, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Harm");
		ecdw_mana_drain_button_id = button_add_extended(ecdebug_win, ecdw_mana_drain_button_id,
			NULL, 180, 40+64, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Drain Mana");
		ecdw_alert_button_id = button_add_extended(ecdebug_win, ecdw_alert_button_id,
			NULL, 8, 40+224, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Alert");
		ecdw_harv_rare_stone_button_id = button_add_extended(ecdebug_win, ecdw_harv_rare_stone_button_id,
			NULL, 8, 40+256, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "H: Rare Stone");
		ecdw_mine_detonate_button_id = button_add_extended(ecdebug_win, ecdw_mine_detonate_button_id,
			NULL, 8, 40+288, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "Mine Det.");
		ecdw_harv_goldbag_button_id = button_add_extended(ecdebug_win, ecdw_harv_goldbag_button_id,
			NULL, 8, 40+320, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "H: Gold Bag");
		ecdw_harv_bee_button_id = button_add_extended(ecdebug_win, ecdw_harv_bee_button_id,
			NULL, 8, 40+352, 160, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, "H: Bees");
		
		// add handlers
		widget_set_OnClick(ecdebug_win, ecdw_restoration_button_id, ecdw_restoration_handler);
		widget_set_OnClick(ecdebug_win, ecdw_shield_button_id, ecdw_shield_handler);
		widget_set_OnClick(ecdebug_win, ecdw_coldshield_button_id, ecdw_coldshield_handler);
		widget_set_OnClick(ecdebug_win, ecdw_heatshield_button_id, ecdw_heatshield_handler);
		widget_set_OnClick(ecdebug_win, ecdw_radiationshield_button_id, ecdw_radiationshield_handler);
		widget_set_OnClick(ecdebug_win, ecdw_heal_button_id, ecdw_heal_handler);
		widget_set_OnClick(ecdebug_win, ecdw_b2g_button_id, ecdw_b2g_handler);
		widget_set_OnClick(ecdebug_win, ecdw_magic_immunity_button_id, ecdw_magic_immunity_handler);
		widget_set_OnClick(ecdebug_win, ecdw_remote_heal_button_id, ecdw_remote_heal_handler);
		widget_set_OnClick(ecdebug_win, ecdw_poison_button_id, ecdw_poison_handler);
		widget_set_OnClick(ecdebug_win, ecdw_harm_button_id, ecdw_harm_handler);
		widget_set_OnClick(ecdebug_win, ecdw_mana_drain_button_id, ecdw_mana_drain_handler);
		widget_set_OnClick(ecdebug_win, ecdw_alert_button_id, ecdw_alert_handler);
		widget_set_OnClick(ecdebug_win, ecdw_harv_rare_stone_button_id, ecdw_harv_rare_stone_handler);
		widget_set_OnClick(ecdebug_win, ecdw_mine_detonate_button_id, ecdw_mine_detonate_handler);
		widget_set_OnClick(ecdebug_win, ecdw_harv_goldbag_button_id, ecdw_harv_goldbag_handler);
		widget_set_OnClick(ecdebug_win, ecdw_harv_bee_button_id, ecdw_harv_bee_handler);
	} else {
		show_window(ecdebug_win);
		select_window(ecdebug_win);
	}
}

int ecdw_restoration_handler()
{
	ec_create_selfmagic_restoration2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_shield_handler()
{
	ec_create_selfmagic_shield_generic(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10), SPECIAL_EFFECT_SHIELD);
	return 1;
}

int ecdw_coldshield_handler()
{
	ec_create_selfmagic_shield_generic(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10), SPECIAL_EFFECT_COLDSHIELD);
	return 1;
}

int ecdw_heatshield_handler()
{
	ec_create_selfmagic_shield_generic(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10), SPECIAL_EFFECT_HEATSHIELD);
	return 1;
}

int ecdw_radiationshield_handler()
{
	ec_create_selfmagic_shield_generic(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10), SPECIAL_EFFECT_RADIATIONSHIELD);
	return 1;
}

int ecdw_heal_handler()
{
	ec_create_selfmagic_heal2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_b2g_handler()
{
	ec_create_selfmagic_bones_to_gold2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_magic_immunity_handler()
{
	ec_create_selfmagic_magic_immunity2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_remote_heal_handler()
{
    actor *target = NULL;
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] && actors_list[i] != get_actor_ptr_from_id(yourself))
		{
			target = actors_list[i];
		}
	}
	if (target != NULL) {
		ec_create_targetmagic_remote_heal2(get_actor_ptr_from_id(yourself), target, (poor_man ? 6 : 10));
	}
	return 1;
}

int ecdw_poison_handler()
{
    actor *target = NULL;
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] && actors_list[i] != get_actor_ptr_from_id(yourself))
		{
			target = actors_list[i];
		}
	}
	if (target != NULL) {
		ec_create_targetmagic_poison2(get_actor_ptr_from_id(yourself), target, (poor_man ? 6 : 10));
	}
	return 1;
}

int ecdw_harm_handler()
{
    actor *target = NULL;
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] && actors_list[i] != get_actor_ptr_from_id(yourself))
		{
			target = actors_list[i];
		}
	}
	if (target != NULL) {
		ec_create_targetmagic_harm2(get_actor_ptr_from_id(yourself), target, (poor_man ? 6 : 10));
	}
	return 1;
}

int ecdw_mana_drain_handler()
{
    actor *target = NULL;
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] && actors_list[i] != get_actor_ptr_from_id(yourself))
		{
			target = actors_list[i];
		}
	}
	if (target != NULL) {
		ec_create_targetmagic_drain_mana2(get_actor_ptr_from_id(yourself), target, (poor_man ? 6 : 10));
	}
	return 1;
}

int ecdw_alert_handler()
{
	ec_create_alert2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_harv_rare_stone_handler()
{
	ec_create_harvesting_rare_stone2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_mine_detonate_handler()
{
	ec_create_mine_detonate2(get_actor_ptr_from_id(yourself), MINE_TYPE_HIGH_EXPLOSIVE_MINE, (poor_man ? 6 : 10));
	return 1;
}

int ecdw_harv_goldbag_handler()
{
	ec_create_harvesting_bag_of_gold2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

int ecdw_harv_bee_handler()
{
	ec_create_harvesting_bees2(get_actor_ptr_from_id(yourself), (poor_man ? 6 : 10));
	return 1;
}

#endif // ECDEBUGWIN