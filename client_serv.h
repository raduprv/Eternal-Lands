/*!
 * \file
 * \ingroup protocol
 * \brief this file defines the core client server protocol.
 */
#ifndef __CLIENT_SERV_H__
#define __CLIENT_SERV_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Actor types
 */
/*! @{ */
typedef enum actor_types_type
{
	human_female = 0,
	human_male = 1,
	elf_female = 2,
	elf_male = 3,
	dwarf_female = 4,
	dwarf_male = 5,
	wraith = 6,
	cyclops = 7,
	beaver = 8,
	rat = 9,
	goblin_male_2 = 10,
	goblin_female_1 = 11,
	town_folk4 = 12,
	town_folk5 = 13,
	shop_girl3 = 14,
	deer = 15,
	bear = 16,
	wolf = 17,
	white_rabbit = 18,
	brown_rabbit = 19,
	boar = 20,
	bear2 = 21,
	snake1 = 22,
	snake2 = 23,
	snake3 = 24,
	fox = 25,
	puma = 26,
	ogre_male_1 = 27,
	goblin_male_1 = 28,
	orc_male_1 = 29,
	orc_female_1 = 30,
	skeleton = 31,
	gargoyle1 = 32,
	gargoyle2 = 33,
	gargoyle3 = 34,
	troll = 35,
	chimeran_wolf_mountain = 36,
	gnome_female = 37,
	gnome_male = 38,
	orchan_female = 39,
	orchan_male = 40,
	draegoni_female = 41,
	draegoni_male = 42,
	skunk_1 = 43,
	racoon_1 = 44,
	unicorn_1 = 45,
	chimeran_wolf_desert = 46,
	chimeran_wolf_forest = 47,
	bear_3 = 48,
	bear_4 = 49,
	panther = 50,
	feran = 51,
	leopard_1 = 52,
	leopard_2 = 53,
	chimeran_wolf_arctic = 54,
	tiger_1 = 55,
	tiger_2 = 56,
	armed_female_orc = 57,
	armed_male_orc = 58,
	armed_skeleton = 59,
	phantom_warrior = 60,
	imp = 61,
	brownie = 62,
	leprechaun = 63,
	spider_s_1 = 64,
	spider_s_2 = 65,
	spider_s_3 = 66,
	spider_l_1 = 67,
	spider_l_2 = 68,
	spider_l_3 = 69,
	wood_sprite = 70,
	spider_l_4 = 71,
	spider_s_4 = 72,
	giant_1 = 73,
	hobgoblin = 74,
	yeti = 75,
	snake4 = 76,
	feros = 77,
	dragon1 = 78
} actor_types_type;
/*! @} */

/*!
 * \name Skin colors
 */
/*! @{ */
#define SKIN_BROWN		0
#define SKIN_NORMAL		1
#define SKIN_PALE		2
#define SKIN_TAN		3
#define SKIN_DARK_BLUE	4	// for Elf
#define SKIN_WHITE		5	// for Draegoni
/*! @} */

/*!
 * \name Shirt colors
 */
/*! @{ */
#define SHIRT_BLACK 0
#define SHIRT_BLUE 1
#define SHIRT_BROWN 2
#define SHIRT_GREY 3
#define SHIRT_GREEN 4
#define SHIRT_LIGHTBROWN 5
#define SHIRT_ORANGE 6
#define SHIRT_PINK 7
#define SHIRT_PURPLE 8
#define SHIRT_RED 9
#define SHIRT_WHITE 10
#define SHIRT_YELLOW 11
#define SHIRT_LEATHER_ARMOR 12
#define SHIRT_CHAIN_ARMOR 13
#define SHIRT_STEEL_CHAIN_ARMOR 14
#define SHIRT_TITANIUM_CHAIN_ARMOR 15
#define SHIRT_IRON_PLATE_ARMOR 16
#define SHIRT_AUGMENTED_LEATHER_ARMOR 17
#define SHIRT_FUR 18
#define SHIRT_STEEL_PLATE_ARMOR 19
#define SHIRT_TITANIUM_PLATE_ARMOR 20
#define SHIRT_BRONZE_PLATE_ARMOR 21

/*! @} */

/*!
 * \name No armor flags
 */
/*! @{ */
#define NO_BODY_ARMOR 0
#define NO_PANTS_ARMOR 0
#define NO_BOOTS_ARMOR 0
/*! @} */

/*!
 * \name Hair colors
 */
/*! @{ */
#define HAIR_BLACK	0
#define HAIR_BLOND	1
#define HAIR_BROWN	2
#define HAIR_GRAY	3
#define HAIR_RED	4
#define HAIR_WHITE	5
#define HAIR_BLUE	6	// for Draegoni
#define HAIR_GREEN	7	// for Draegoni
#define HAIR_PURPLE 8	// for Draegoni
#define HAIR_DARK_BROWN	9
#define HAIR_STRAWBERRY	10
#define HAIR_LIGHT_BLOND	11
#define HAIR_DIRTY_BLOND	12
#define HAIR_BROWN_GRAY	13
#define HAIR_DARK_GRAY	14
#define HAIR_DARK_RED	15
/*! @} */

/*!
 * \name Boots colors
 */
/*! @{ */
#define BOOTS_BLACK 0
#define BOOTS_BROWN 1
#define BOOTS_DARKBROWN 2
#define BOOTS_DULLBROWN 3
#define BOOTS_LIGHTBROWN 4
#define BOOTS_ORANGE 5
#define BOOTS_LEATHER 6
#define BOOTS_FUR 7
#define BOOTS_IRON_GREAVE 8
#define BOOTS_STEEL_GREAVE 9
#define BOOTS_TITANIUM_GREAVE 10
#define BOOTS_BRONZE_GREAVE 11
#define BOOTS_AUGMENTED_LEATHER_GREAVE 12
/*! @} */

/*!
 * \name Pants colors
 */
/*! @{ */
#define PANTS_BLACK 0
#define PANTS_BLUE 1
#define PANTS_BROWN 2
#define PANTS_DARKBROWN 3
#define PANTS_GREY 4
#define PANTS_GREEN 5
#define PANTS_LIGHTBROWN 6
#define PANTS_RED 7
#define PANTS_WHITE 8
#define PANTS_LEATHER 9
#define PANTS_IRON_CUISSES 10
#define PANTS_FUR 11
#define PANTS_STEEL_CUISSES 12
#define PANTS_TITANIUM_CUISSES 13
#define PANTS_BRONZE_CUISSES 14
#define PANTS_AUGMENTED_LEATHER_CUISSES 15
/*! @} */

/*!
 * \name Capes
 */
/*! @{ */
#define CAPE_BLACK 0
#define CAPE_BLUE 1
#define CAPE_BLUEGRAY 2
#define CAPE_BROWN 3
#define CAPE_BROWNGRAY 4
#define CAPE_GRAY 5
#define CAPE_GREEN 6
#define CAPE_GREENGRAY 7
#define CAPE_PURPLE 8
#define CAPE_WHITE 9
#define CAPE_FUR 10
#define CAPE_GOLD 11
#define CAPE_RED 12
#define CAPE_ORANGE 13
#define CAPE_MOD 14
#define CAPE_DERIN 15
#define CAPE_RAVENOD 16
#define CAPE_PLACID 17
#define CAPE_LORD_VERMOR 18
#define CAPE_AISLINN 19
#define CAPE_SOLDUS 20
#define CAPE_LOTHARION 21
#define CAPE_LEARNER 22
#define CAPE_NONE 30
/*! @} */

/*!
 * \name Heads
 */
/*! @{ */
#define HEAD_1 0
#define HEAD_2 1
#define HEAD_3 2
#define HEAD_4 3
#define HEAD_5 4
/*! @} */

/*!
 * \name Types of wearable items
 */
/*! @{ */
#define KIND_OF_WEAPON 0
#define KIND_OF_SHIELD 1
#define KIND_OF_CAPE 2
#define KIND_OF_HELMET 3
#define KIND_OF_LEG_ARMOR 4
#define KIND_OF_BODY_ARMOR 5
#define KIND_OF_BOOT_ARMOR 6
#define KIND_OF_NECK 7

//NECK ITEMS CODES
#define NECK_NONE 0


/*! @} */



/*!
 * \name Helmets
 */
/*! @{ */
#define HELMET_IRON 0
#define HELMET_FUR 1
#define HELMET_LEATHER 2
#define HELMET_RACOON 3
#define HELMET_SKUNK 4
#define HELMET_CROWN_OF_MANA 5
#define HELMET_CROWN_OF_LIFE 6
#define HELMET_STEEL 7
#define HELMET_TITANIUM 8
#define HELMET_BRONZE 9
#define HELMET_NONE 20
/*! @} */

/*!
 * \name Shields
 */
/*! @{ */
#define SHIELD_WOOD 0
#define SHIELD_WOOD_ENHANCED 1
#define SHIELD_IRON 2
#define SHIELD_STEEL 3
#define SHIELD_TITANIUM 4
#define SHIELD_BRONZE 5
#define QUIVER_ARROWS 7
#define SHIELD_NONE 11
#define QUIVER_BOLTS 13
/*! @} */

/*!
 * \name Weapons
 */
/*! @{ */
#define WEAPON_NONE 0
#define SWORD_1 1
#define SWORD_2 2
#define SWORD_3 3
#define SWORD_4 4
#define SWORD_5 5
#define SWORD_6 6
#define SWORD_7 7
#define STAFF_1 8
#define STAFF_2 9
#define STAFF_3 10
#define STAFF_4 11
#define HAMMER_1 12
#define HAMMER_2 13
#define PICKAX 14
#define SWORD_1_FIRE 15
#define SWORD_2_FIRE 16
#define SWORD_2_COLD 17
#define SWORD_3_FIRE 18
#define SWORD_3_COLD 19
#define SWORD_3_MAGIC 20
#define SWORD_4_FIRE 21
#define SWORD_4_COLD 22
#define SWORD_4_MAGIC 23
#define SWORD_4_THERMAL 24
#define SWORD_5_FIRE 25
#define SWORD_5_COLD 26
#define SWORD_5_MAGIC 27
#define SWORD_5_THERMAL 28
#define SWORD_6_FIRE 29
#define SWORD_6_COLD 30
#define SWORD_6_MAGIC 31
#define SWORD_6_THERMAL 32
#define SWORD_7_FIRE 33
#define SWORD_7_COLD 34
#define SWORD_7_MAGIC 35
#define SWORD_7_THERMAL 36
#define PICKAX_MAGIC 37
#define BATTLEAXE_IRON 38
#define BATTLEAXE_STEEL 39
#define BATTLEAXE_TITANIUM 40
#define BATTLEAXE_IRON_FIRE 41
#define BATTLEAXE_STEEL_COLD 42
#define BATTLEAXE_STEEL_FIRE 43
#define BATTLEAXE_TITANIUM_COLD 44
#define BATTLEAXE_TITANIUM_FIRE 45
#define BATTLEAXE_TITANIUM_MAGIC 46
#define GLOVE_FUR 47
#define GLOVE_LEATHER 48
#define BONE_1 49
#define STICK_1 50
#define SWORD_EMERALD_CLAYMORE 51
#define SWORD_CUTLASS 52
#define SWORD_SUNBREAKER 53
#define SWORD_ORC_SLAYER 54
#define SWORD_EAGLE_WING 55
#define SWORD_RAPIER 56
#define SWORD_JAGGED_SABER 57
#define SWORD_BRONZE 58
#define BOW_LONG 64
#define BOW_SHORT 65
#define BOW_RECURVE 66
#define BOW_ELVEN 67
#define BOW_CROSS 68

/*! @} */

/*!
 * \name Frames
 */
/*! @{ */
#define frame_walk 0
#define frame_run 1
#define frame_die1 2
#define frame_die2 3
#define frame_pain1 4
#define frame_pain2 11
#define frame_pick 5
#define frame_drop 6
#define frame_idle 7
#define frame_harvest 8
#define frame_cast 9
#define frame_ranged 10
#define frame_sit 12
#define frame_stand 13
#define frame_sit_idle 14
#define frame_combat_idle 15
#define frame_in_combat 16
#define frame_out_combat 17
#define frame_attack_up_1 18
#define frame_attack_up_2 19
#define frame_attack_up_3 20
#define frame_attack_up_4 21
#define frame_attack_down_1 22
#define frame_attack_down_2 23
#define frame_attack_down_3 24
#define frame_attack_down_4 25 
#define frame_attack_down_5 26 
#define frame_attack_down_6 27 
#define frame_attack_down_7 28 
#define frame_attack_down_8 29 
#define frame_attack_down_9 30 
#define frame_attack_down_10 31 
#define frame_attack_up_5 32 
#define frame_attack_up_6 33 
#define frame_attack_up_7 34 
#define frame_attack_up_8 35 
#define frame_attack_up_9 36 
#define frame_attack_up_10 37
//frame values for poses (40 different poses)
#define frame_poses_start 100
#define frame_poses_end 140
/*! @} */

/*!
 * \name Colors
 */
/*! @{ */
#define c_lbound 0
#define c_red1 0
#define c_red2 7
#define c_red3 14
#define c_red4 21
#define c_orange1 1
#define c_orange2 8
#define c_orange3 15
#define c_orange4 22
#define c_yellow1 2
#define c_yellow2 9
#define c_yellow3 16
#define c_yellow4 23
#define c_green1 3
#define c_green2 10
#define c_green3 17
#define c_green4 24
#define c_blue1 4
#define c_blue2 11
#define c_blue3 18
#define c_blue4 25
#define c_purple1 5
#define c_purple2 12
#define c_purple3 19
#define c_purple4 26
#define c_grey1 6
#define c_grey2 13
#define c_grey3 20
#define c_grey4 27
#define c_ubound 27
/*! @} */

/*!
 * \name Foreign chars
 */
/*! @{ */
#define SPECIALCHAR_LBOUND 180
#define EACUTE 181
#define ACIRC 182
#define AGRAVE 183
#define CCEDIL 184
#define ECIRC 185
#define EUML 186
#define EGRAVE 187
#define IUML 188
#define OCIRC 189
#define uGRAVE 190
#define aUMLAUT 191
#define oUMLAUT 192
#define uUMLAUT 193
#define AUMLAUT 194
#define OUMLAUT 195
#define UUMLAUT 196
#define DOUBLES 197
#define aELIG 198
#define oSLASH 199
#define aRING 200
#define AELIG 201
#define OSLASH 202
#define ARING 203
#define EnyE 204
#define ENYE 205
#define aACCENT 206
#define AACCENT 207
#define EACCENT 208
#define iACCENT 209
#define IACCENT 210
#define oACCENT 211
#define OACCENT 212
#define uACCENT 213
#define UACCENT 214
#define SPECIALCHAR_UBOUND 214

/*! @} */

/*!
 * \name Windows
 */
/*! @{ */
#define RULE_WIN 1
#define RULE_INTERFACE 2
#define NEW_CHAR_INTERFACE 3
/*! @} */

/*!
 * \name Actor commands
 * 
 * Note to other developers: #defines are generally *bad form*.  They
 * interfere with innocent calls, like my call to eye_candy.idle(), in very
 * confusing ways (in my case, "expected unqualified-id before numeric
 * constant").  This is what enums are for; please use them.  I'm fixing this
 * one for you as an example.
 *
 *  - Karen (meme@daughtersoftiresias.org)
 */
/*! @{ */
typedef enum actor_commands
{
  nothing = 0,
  kill_me = 1,
  die1 = 3,
  die2 = 4,
  pain1 = 5,
  pain2 = 17,
  pick = 6,
  drop = 7,
  idle = 8,
  harvest = 9,
  cast = 10,
  ranged = 11,
  meele = 12,
  sit_down = 13,
  stand_up = 14,
  turn_left = 15,
  turn_right = 16,
  enter_combat = 18,
  leave_combat = 19,

  move_n = 20,
  move_ne = 21,
  move_e = 22,
  move_se = 23,
  move_s = 24,
  move_sw = 25,
  move_w = 26,
  move_nw = 27,


  run_n = 30,
  run_ne = 31,
  run_e = 32,
  run_se = 33,
  run_s = 34,
  run_sw = 35,
  run_w = 36,
  run_nw = 37,

  turn_n = 38,
  turn_ne = 39,
  turn_e = 40,
  turn_se = 41,
  turn_s = 42,
  turn_sw = 43,
  turn_w = 44,
  turn_nw = 45,

  attack_up_1 = 46,
  attack_up_2 = 47,
  attack_up_3 = 48,
  attack_up_4 = 49,
  attack_down_1 = 50,
  attack_down_2 = 51,

  enter_aim_mode = 52,
  leave_aim_mode = 53,
  aim_mode_reload = 54,
  aim_mode_fire = 55,
  missile_miss = 56,
  //unwear_bow = 57,
  //unwear_quiver = 58,
  missile_critical = 59,

  attack_down_3 = 60,
  attack_down_4 = 61,
  attack_down_5 = 62,
  attack_down_6 = 63,
  attack_down_7 = 64,
  attack_down_8 = 65,
  attack_down_9 = 66,
  attack_down_10 = 67,
  
  attack_up_5 = 68,
  attack_up_6 = 69,
  attack_up_7 = 70,
  attack_up_8 = 71,
  attack_up_9 = 72,
  attack_up_10 = 73,

  emote_cmd = 100,

  //from 100 to 255, commands are reserved for emotes
//#ifdef MORE_ATTACHED_ACTORS
  wait_cmd=256, //to synch actor/horse (not sent by server)
//#endif
} actor_commands;

/*! @} */


/*!
 * \name Weather types
 */
/*! @{ */
typedef enum
{
	//precipitation types MUST come first, in the same order as in weather.c . same indexen.
	weather_effect_rain = 1,
	weather_effect_snow = 2,
	weather_effect_hail = 3,
	weather_effect_sand = 4,
	weather_effect_dust = 5,
	weather_effect_lava = 6,
	//other effects can be put at the end
	weather_effect_wind = 20,
	weather_effect_leaves = 21
} weather_type;

/*! @} */


/*!
 * \name To server commands
 */
/*! @{ */
#define MOVE_TO 1
#define SEND_PM 2
#define GET_PLAYER_INFO 5
#define RUN_TO 6
#define SIT_DOWN 7
#define SEND_ME_MY_ACTORS 8
#define SEND_OPENING_SCREEN 9
#define SEND_VERSION 10
#define TURN_LEFT 11
#define TURN_RIGHT 12
#define PING 13
#define HEART_BEAT 14
#define LOCATE_ME 15
#define USE_MAP_OBJECT 16
#define SEND_MY_STATS 17
#define SEND_MY_INVENTORY 18
#define LOOK_AT_INVENTORY_ITEM 19
#define MOVE_INVENTORY_ITEM 20
#define HARVEST 21
#define DROP_ITEM 22
#define PICK_UP_ITEM 23
#define LOOK_AT_GROUND_ITEM 24
#define INSPECT_BAG 25
#define S_CLOSE_BAG 26
#define LOOK_AT_MAP_OBJECT 27
#define TOUCH_PLAYER 28
#define RESPOND_TO_NPC 29
#define MANUFACTURE_THIS 30
#define USE_INVENTORY_ITEM 31
#define TRADE_WITH 32
#define ACCEPT_TRADE 33
#define REJECT_TRADE 34
#define EXIT_TRADE 35
#define PUT_OBJECT_ON_TRADE 36
#define REMOVE_OBJECT_FROM_TRADE 37
#define LOOK_AT_TRADE_ITEM 38
#define CAST_SPELL 39
#define ATTACK_SOMEONE 40
#define GET_KNOWLEDGE_INFO 41
#define ITEM_ON_ITEM 42
#define SEND_BOOK 43
#define GET_STORAGE_CATEGORY 44
#define DEPOSITE_ITEM 45
#define WITHDRAW_ITEM 46
#define LOOK_AT_STORAGE_ITEM 47
#define SPELL_NAME 48
#define SEND_VIDEO_INFO 49
#define POPUP_REPLY 50
#define FIRE_MISSILE_AT_OBJECT 51

#define PING_RESPONSE 60
#define SET_ACTIVE_CHANNEL 61

/* send: 16 bit quest id, request the server to supply quest title using HERE_IS_QUEST_ID */
#define WHAT_QUEST_IS_THIS_ID 63

#define DO_EMOTE 70

#define LOG_IN 140
#define CREATE_CHAR 141

#define GET_DATE 230
#define GET_TIME 231
#define SERVER_STATS 232
#define ORIGINAL_IP 233
/*! @} */

/*!
 * \name To client commands
 */
/*! @{ */
#define ADD_NEW_ACTOR 1
#define ADD_ACTOR_COMMAND 2
#define YOU_ARE 3
#define SYNC_CLOCK 4
#define NEW_MINUTE 5
#define REMOVE_ACTOR 6
#define CHANGE_MAP 7
#define COMBAT_MODE 8
#define KILL_ALL_ACTORS 9
#define GET_TELEPORTERS_LIST 10
#define PONG 11
#define TELEPORT_IN 12
#define TELEPORT_OUT 13
#define PLAY_SOUND 14
#define START_RAIN 15	//delete later on
#define STOP_RAIN 16	//delete later on
#define THUNDER 17
#define HERE_YOUR_STATS 18
#define HERE_YOUR_INVENTORY 19
#define INVENTORY_ITEM_TEXT 20
#define GET_NEW_INVENTORY_ITEM 21
#define REMOVE_ITEM_FROM_INVENTORY 22
#define HERE_YOUR_GROUND_ITEMS 23
#define GET_NEW_GROUND_ITEM 24
#define REMOVE_ITEM_FROM_GROUND 25
#define CLOSE_BAG 26
#define GET_NEW_BAG 27
#define GET_BAGS_LIST 28
#define DESTROY_BAG 29
#define NPC_TEXT 30
#define NPC_OPTIONS_LIST 31
#define CLOSE_NPC_MENU 32
#define SEND_NPC_INFO 33
#define GET_TRADE_INFO 34//delete later on
#define GET_TRADE_OBJECT 35
#define GET_TRADE_ACCEPT 36
#define GET_TRADE_REJECT 37
#define GET_TRADE_EXIT 38
#define REMOVE_TRADE_OBJECT 39
#define GET_YOUR_TRADEOBJECTS 40
#define GET_TRADE_PARTNER_NAME 41
#define GET_YOUR_SIGILS 42
#define SPELL_ITEM_TEXT 43
#define GET_ACTIVE_SPELL 44
#define GET_ACTIVE_SPELL_LIST 45
#define REMOVE_ACTIVE_SPELL 46
#define GET_ACTOR_DAMAGE 47
#define GET_ACTOR_HEAL 48
#define SEND_PARTIAL_STAT 49
#define SPAWN_BAG_PARTICLES 50
#define ADD_NEW_ENHANCED_ACTOR 51
#define ACTOR_WEAR_ITEM 52
#define ACTOR_UNWEAR_ITEM 53
#define PLAY_MUSIC 54
#define GET_KNOWLEDGE_LIST 55
#define GET_NEW_KNOWLEDGE 56
#define GET_KNOWLEDGE_TEXT 57
#define BUDDY_EVENT 59
#define PING_REQUEST 60
#define FIRE_PARTICLES 61
#define REMOVE_FIRE_AT 62
#define DISPLAY_CLIENT_WINDOW 63
#define OPEN_BOOK 64
#define READ_BOOK 65
#define CLOSE_BOOK 66
#define STORAGE_LIST 67
#define STORAGE_ITEMS 68
#define STORAGE_TEXT 69
#define SPELL_CAST 70
#define GET_ACTIVE_CHANNELS 71
#define MAP_FLAGS 72
#define GET_ACTOR_HEALTH 73
#define GET_3D_OBJ_LIST 74
#define GET_3D_OBJ 75
#define REMOVE_3D_OBJ 76
#define GET_ITEMS_COOLDOWN 77
#define SEND_BUFFS 78
#define SEND_SPECIAL_EFFECT 79
#define REMOVE_MINE 80
#define GET_NEW_MINE 81
#define GET_MINES_LIST 82
#define DISPLAY_POPUP 83
#define MISSILE_AIM_A_AT_B 84
#define MISSILE_AIM_A_AT_XYZ 85
#define MISSILE_FIRE_A_TO_B 86
#define MISSILE_FIRE_A_TO_XYZ 87
#define MISSILE_FIRE_XYZ_TO_B 88
#define ADD_ACTOR_ANIMATION 89
#define SEND_MAP_MARKER 90
#define REMOVE_MAP_MARKER 91
/* sent: 16 bit quest id for the next npc message */
#define NEXT_NPC_MESSAGE_IS_QUEST 92
/* sent: non null terminated string giving the title of the quest */
#define HERE_IS_QUEST_ID 93
/* sent: 16 bit quest id, this quest should be shown as finished */
#define QUEST_FINISHED 94
/* sent: 5 x 32 bit integers, each active bit is an achievement the last "You see: name" player has */
#define SEND_ACHIEVEMENTS 95

#define SEND_WEATHER 100

// reserved for future expansion 220-229, not being used in the server yet
#define MAP_SET_OBJECTS 220
#define MAP_STATE_OBJECTS 221

#define UPGRADE_NEW_VERSION 240	// TODO: Consider combining all this into one packet followed by one byte (plus optional text)
#define UPGRADE_TOO_OLD 241
#define REDEFINE_YOUR_COLORS 248
#define YOU_DONT_EXIST 249
#define LOG_IN_OK 250
#define LOG_IN_NOT_OK 251
#define CREATE_CHAR_OK 252
#define CREATE_CHAR_NOT_OK 253
/*! @} */

/*!
 * \name Common (both to the server and client) commands
 */
/*! @{ */
#define RAW_TEXT 0
#define PROXY 254	// reserved for advanced PROXY support
#define BYE 255
/*! @} */


/*!
 * \name Protocol places
 */
#define PROTOCOL 0 /*!< is an unsigned char */

/*!
 * \name Stats
 */
/*! @{ */
#define PHY_CUR 0
#define PHY_BASE 1
#define COO_CUR 2
#define COO_BASE 3
#define REAS_CUR 4
#define REAS_BASE 5
#define WILL_CUR 6
#define WILL_BASE 7
#define INST_CUR 8
#define INST_BASE 9
#define VIT_CUR 10
#define VIT_BASE 11
#define HUMAN_CUR 12
#define HUMAN_BASE 13
#define ANIMAL_CUR 14
#define ANIMAL_BASE 15
#define VEGETAL_CUR 16
#define VEGETAL_BASE 17
#define INORG_CUR 18
#define INORG_BASE 19
#define ARTIF_CUR 20
#define ARTIF_BASE 21
#define MAGIC_CUR 22
#define MAGIC_BASE 23
#define MAN_S_CUR 24
#define MAN_S_BASE 25
#define HARV_S_CUR 26
#define HARV_S_BASE 27
#define ALCH_S_CUR 28
#define ALCH_S_BASE 29
#define OVRL_S_CUR 30
#define OVRL_S_BASE 31
#define DEF_S_CUR 32
#define DEF_S_BASE 33
#define ATT_S_CUR 34
#define ATT_S_BASE 35
#define MAG_S_CUR 36
#define MAG_S_BASE 37
#define POT_S_CUR 38
#define POT_S_BASE 39
#define CARRY_WGHT_CUR 40
#define CARRY_WGHT_BASE 41
#define MAT_POINT_CUR 42
#define MAT_POINT_BASE 43
#define ETH_POINT_CUR 44
#define ETH_POINT_BASE 45
#define FOOD_LEV 46
#define RESEARCHING 47
#define MAG_RES 48
#define MAN_EXP 49
#define MAN_EXP_NEXT 50
#define HARV_EXP 51
#define HARV_EXP_NEXT 52
#define ALCH_EXP 53
#define ALCH_EXP_NEXT 54
#define OVRL_EXP 55
#define OVRL_EXP_NEXT 56
#define DEF_EXP 57
#define DEF_EXP_NEXT 58
#define ATT_EXP 59
#define ATT_EXP_NEXT 60
#define MAG_EXP 61
#define MAG_EXP_NEXT 62
#define POT_EXP 63
#define POT_EXP_NEXT 64
#define RESEARCH_COMPLETED 65
#define RESEARCH_TOTAL 66
#define SUM_EXP 67
#define SUM_EXP_NEXT 68
#define SUM_S_CUR 69
#define SUM_S_BASE 70
#define CRA_EXP 71
#define CRA_EXP_NEXT 72
#define CRA_S_CUR 73
#define CRA_S_BASE 74
#define ENG_EXP 75
#define ENG_EXP_NEXT 76
#define ENG_S_CUR 77
#define ENG_S_BASE 78
#define RANG_EXP 79
#define RANG_EXP_NEXT 80
#define RANG_S_CUR 81
#define RANG_S_BASE 82
#define TAIL_EXP 83
#define TAIL_EXP_NEXT 84
#define TAIL_S_CUR 85
#define TAIL_S_BASE 86
#define ACTION_POINTS_CUR 87
#define ACTION_POINTS_BASE 88
/*! @} */

/*!
 * \name Sound
 */
/*! @{ */
#define snd_rain     0
#define snd_tele_in  1
#define snd_tele_out 2
#define snd_teleprtr 3
#define snd_thndr_1  4
#define snd_thndr_2  5
#define snd_thndr_3  6
#define snd_thndr_4  7
#define snd_thndr_5  8
#define snd_fire     9
/*! @} */

/*!
 * \name Text channels
 */
/*! @{ */
#define CHAT_LOCAL	0
#define CHAT_PERSONAL	1
#define CHAT_GM		2
#define CHAT_SERVER	3
#define CHAT_MOD	4
#define CHAT_CHANNEL1	5
#define CHAT_CHANNEL2	6
#define CHAT_CHANNEL3	7
#define CHAT_MODPM	8
#define CHAT_POPUP 0xFF
/*! @} */

/*!
 * \name Actor scaling constants
 */
/*! @{ */
#define	ACTOR_SCALE_BASE	0x4000
#define	ACTOR_SCALE_MAX		0x7FFF
/*! @} */

/*!
 * \name Special spell effects
 */
/*! @{ */
typedef enum {
	//when one player uses the poison spell on another one. Player to Player
	SPECIAL_EFFECT_POISON = 0,	

	//when one player heals another. Player to Player
	SPECIAL_EFFECT_REMOTE_HEAL = 1,	

	//when one player harms another. Player to Player
	SPECIAL_EFFECT_HARM = 2,	

	//when one player casts shield on himself. Player
	SPECIAL_EFFECT_SHIELD = 3,	

	//when one player casts restoration. Player
	SPECIAL_EFFECT_RESTORATION = 4,	

	//when one player casts a smite summonings. Player
	SPECIAL_EFFECT_SMITE_SUMMONINGS = 5,	

	//when a player goes invisible. Player
	SPECIAL_EFFECT_CLOAK = 6,	

	//when a player becomes visible. Player
	SPECIAL_EFFECT_DECLOAK = 7,	

	//when an invasion starts. Location
	SPECIAL_EFFECT_INVASION_BEAMING = 8,	

	//when a player casts heal summoned. Player
	SPECIAL_EFFECT_HEAL_SUMMONED = 9,	

	//When a player casts mana drain. Player to Player.
	SPECIAL_EFFECT_MANA_DRAIN = 10,	

	//when a player teleports to range. Player, Location
	SPECIAL_EFFECT_TELEPORT_TO_RANGE = 11,	

	//when a player teleports to range. Player, Location
	SPECIAL_EFFECT_HEAL = 12,	

	//when a player finds a rare stone
	SPECIAL_EFFECT_HARVEST_RARE_STONE = 13,	

	//when a player is blessed by MN with exp
	SPECIAL_EFFECT_HARVEST_MN_EXP_BLESSING = 14,	

	//when a player is blessed by MN with money
	SPECIAL_EFFECT_HARVEST_MN_MONEY_BLESSING = 15,	

	//when a wall colapses over a player
	SPECIAL_EFFECT_HARVEST_WALL_COLLAPSE = 16,	

	//when a bees sting a player
	SPECIAL_EFFECT_HARVEST_BEES = 17,	

	//when a radeon hits
	SPECIAL_EFFECT_HARVEST_RADON = 18,	

	//when a tool breaks
	SPECIAL_EFFECT_HARVEST_TOOL_BREAKS = 19,	

	//when teleport nexus actor_id,x1,y1,x2,y2
	SPECIAL_EFFECT_HARVEST_TELEPORT_NEXUS = 20,	

	//when MN takes your health
	SPECIAL_EFFECT_HARVEST_MOTHER_NATURE_PISSED = 21,	

	//when a manufacture tool breaks
	SPECIAL_EFFECT_MANUFACTURE_TOOL_BREAKS = 22,	

	//when a special item is created
	SPECIAL_EFFECT_MANUFACTURE_RARE_ITEM = 23,	

	//when a "who doesn't see me" spell is cast
	SPECIAL_EFFECT_MAKE_PLAYER_GLOW = 24,
	
	//Summoning stuff
	SPECIAL_EFFECT_SUMMON_RABBIT = 25,
	SPECIAL_EFFECT_SUMMON_RAT = 26,
	SPECIAL_EFFECT_SUMMON_BEAVER = 27,
	SPECIAL_EFFECT_SUMMON_SKUNK = 28,
	SPECIAL_EFFECT_SUMMON_RACOON = 29,
	SPECIAL_EFFECT_SUMMON_DEER = 30,
	SPECIAL_EFFECT_SUMMON_GREEN_SNAKE = 31,
	SPECIAL_EFFECT_SUMMON_RED_SNAKE = 32,
	SPECIAL_EFFECT_SUMMON_BROWN_SNAKE = 33,
	SPECIAL_EFFECT_SUMMON_FOX = 34,
	SPECIAL_EFFECT_SUMMON_BOAR = 35,
	SPECIAL_EFFECT_SUMMON_WOLF = 36,
	SPECIAL_EFFECT_SUMMON_SKELETON = 37,
	SPECIAL_EFFECT_SUMMON_SMAL_GARG = 38,
	SPECIAL_EFFECT_SUMMON_MEDIUM_GARG = 39,
	SPECIAL_EFFECT_SUMMON_BIG_GARG = 40,
	SPECIAL_EFFECT_SUMMON_PUMA = 41,
	SPECIAL_EFFECT_SUMMON_FEM_GOBLIN = 42,
	SPECIAL_EFFECT_SUMMON_POLAR_BEAR = 43,
	SPECIAL_EFFECT_SUMMON_BEAR = 44,
	SPECIAL_EFFECT_SUMMON_ARMED_MALE_GOB = 45,
	SPECIAL_EFFECT_SUMMON_ARMED_SKELETON = 46,
	SPECIAL_EFFECT_SUMMON_FEMALE_ORC = 47,
	SPECIAL_EFFECT_SUMMON_MALE_ORC = 48,
	SPECIAL_EFFECT_SUMMON_ARMED_FEM_ORC = 49,
	SPECIAL_EFFECT_SUMMON_ARMED_MALE_ORC = 50,
	SPECIAL_EFFECT_SUMMON_CYCLOP = 51,
	SPECIAL_EFFECT_SUMMON_FLUFFY_RABBIT = 52,
	SPECIAL_EFFECT_SUMMON_PHANTOM_WARRIOR = 53,
	SPECIAL_EFFECT_SUMMON_MOUNTAIN_CHIM = 54,
	SPECIAL_EFFECT_SUMMON_YETI = 55,
	SPECIAL_EFFECT_SUMMON_ARCTIC_CHIM = 56,
	SPECIAL_EFFECT_SUMMON_GIANT = 57,
	SPECIAL_EFFECT_SUMMON_GIANT_SNAKE = 58,
	SPECIAL_EFFECT_SUMMON_SPIDER = 59,
	SPECIAL_EFFECT_SUMMON_TIGER = 60,
	
	// Mines stuff
	SPECIAL_EFFECT_SMALL_MINE_GOES_BOOM = 61,
	SPECIAL_EFFECT_MEDIUM_MINE_GOES_BOOM = 62,
	SPECIAL_EFFECT_HIGH_EXPLOSIVE_MINE_GOES_BOOM = 63,
	SPECIAL_EFFECT_SNARE_GOES_BOOM = 64,
	SPECIAL_EFFECT_CALTROP_GOES_BOOM = 65,
	SPECIAL_EFFECT_POISONED_CALTROP_GOES_BOOM = 66,
	SPECIAL_EFFECT_MANA_DRAINER_GOES_BOOM = 67,
	SPECIAL_EFFECT_MANA_BURNER_GOES_BOOM = 68,
	SPECIAL_EFFECT_UNINVIZIBILIZER_GOES_BOOM = 69,
	SPECIAL_EFFECT_MAGIC_IMMUNITY_REMOVAL_GOES_BOOM = 70,

	// Heal allies spell	
	SPECIAL_EFFECT_HEAL_ALLIES = 71,

	// when one player casts heat shield on himself
	SPECIAL_EFFECT_HEATSHIELD = 72, 
	// when one player casts cold shield on himself
	SPECIAL_EFFECT_COLDSHIELD = 73, 
	// when one player casts radiation shield on himself
	SPECIAL_EFFECT_RADIATIONSHIELD = 74,
	// magic immunity spell
	SPECIAL_EFFECT_MAGIC_IMMUNITY = 75,
	// magic protection spell
	SPECIAL_EFFECT_MAGIC_PROTECTION = 76
} special_effect_enum;
	/*! @} */

/*!
 * \name Mine types
 */
/*! @{ */
#define MINE_TYPE_SMALL_MINE 0
#define MINE_TYPE_MEDIUM_MINE 1
#define MINE_TYPE_HIGH_EXPLOSIVE_MINE 2
#define MINE_TYPE_TRAP 3
#define MINE_TYPE_CALTROP 4
#define MINE_TYPE_POISONED_CALTROP 5
#define MINE_TYPE_BARRICADE 6
#define MINE_TYPE_MANA_DRAINER 7
#define MINE_TYPE_MANA_BURNER 8
#define MINE_TYPE_UNINVIZIBILIZER 9
#define MINE_TYPE_MAGIC_IMMUNITY_REMOVAL 10
/*! @} */

/*!
 * \name Actor buffs constants
 */
typedef enum {
	BUFF_INVISIBILITY = 1,
	BUFF_MAGIC_IMMUNITY = 2,
	BUFF_MAGIC_PROTECTION = 4,
	BUFF_COLD_SHIELD = 8,
	BUFF_HEAT_SHIELD = 16,
	BUFF_RADIATION_SHIELD = 32,
	BUFF_SHIELD = 64,
	BUFF_TRUE_SIGHT = 128,
	BUFF_ACCURACY = 256,
	BUFF_EVASION = 512,
	BUFF_DOUBLE_SPEED = 1024
} buffs;

#ifdef __cplusplus
} // extern "C"
#endif
	
#endif
