#ifndef __CLIENT_SERV_H__
#define __CLIENT_SERV_H__

#define human_female 0
#define human_male 1
#define elf_female 2
#define elf_male 3
#define dwarf_female 4
#define dwarf_male 5
#define wraith 6
#define cyclops 7
#define beaver 8
#define rat 9
#define goblin_male_2 10
#define goblin_female_1 11
#define town_folk4 12
#define town_folk5 13
#define shop_girl3 14
#define deer 15
#define bear 16
#define wolf 17
#define white_rabbit 18
#define brown_rabbit 19
#define boar 20
#define bear2 21
#define snake1 22
#define snake2 23
#define snake3 24
#define fox 25
#define puma 26
#define ogre_male_1 27
#define goblin_male_1 28
#define orc_male_1 29
#define orc_female_1 30
#define skeleton 31
#define gargoyle1 32
#define gargoyle2 33
#define gargoyle3 34
#define troll 35
#define chimeran_wolf_mountain 36
#define gnome_female 37
#define gnome_male 38
#define orchan_female 39
#define orchan_male 40
#define draegoni_female 41
#define draegoni_male 42

//skin colors
#define SKIN_BROWN 0
#define SKIN_NORMAL 1
#define SKIN_PALE 2
#define SKIN_TAN 3

//shirt colors
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
#define SHIRT_ARMOR_6 17
#define SHIRT_FUR 18

#define NO_BODY_ARMOR 0
#define NO_PANTS_ARMOR 0
#define NO_BOOTS_ARMOR 0

//hair
#define HAIR_BLACK 0
#define HAIR_BLOND 1
#define HAIR_BROWN 2
#define HAIR_GRAY 3
#define HAIR_RED 4
#define HAIR_WHITE 5
#define HAIR_BLUE 6		// for Draegoni
#define HAIR_GREEN 7	// for Draegoni
#define HAIR_PURPLE 8	// for Draegoni

//boots color
#define BOOTS_BLACK 0
#define BOOTS_BROWN 1
#define BOOTS_DARKBROWN 2
#define BOOTS_DULLBROWN 3
#define BOOTS_LIGHTBROWN 4
#define BOOTS_ORANGE 5
#define BOOTS_LEATHER 6
#define BOOTS_FUR 7
#define BOOTS_IRON_GREAVE 8

//pants
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

//capes
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
#define CAPE_MOONSHADOW 15
#define CAPE_RAVENOD 16
#define CAPE_ROGUE 17
#define CAPE_WYTTER 18
#define CAPE_QUELL 19
#define CAPE_NONE 30

//heads
#define HEAD_1 0
#define HEAD_2 1
#define HEAD_3 2
#define HEAD_4 3
#define HEAD_5 4


#define KIND_OF_WEAPON 0
#define KIND_OF_SHIELD 1
#define KIND_OF_CAPE 2
#define KIND_OF_HELMET 3
#define KIND_OF_LEG_ARMOR 4
#define KIND_OF_BODY_ARMOR 5
#define KIND_OF_BOOT_ARMOR 6

//helmets
#define HELMET_IRON 0
#define HELMET_FUR 1
#define HELMET_LEATHER 2
#define HELMET_NONE 20


//shields
#define SHIELD_WOOD 0
#define SHIELD_WOOD_ENHANCED 1
#define SHIELD_IRON 2
#define SHIELD_STEEL 3
#define SHIELD_NONE 11

//weapons
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

//colors
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

//foreign chars
#define UUML 180
#define EACUTE 181
#define ACIRC 182
#define AGRAVE 183
#define CCEDIL 184
#define ECIRC 185
#define EUML 186
#define EGRAVE 187
#define IUML 188
#define OCIRC 189
#define UGRAVE 190
#define aUMLAUT 191
#define oUMLAUT 192
#define uUMLAUT 192
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

//Windows
#define RULE_WIN 1
#define RULE_INTERFACE 2
#define NEW_CHAR_INTERFACE 3

//actor commands
#define nothing 0
#define kill_me 1
#define die1 3
#define die2 4
#define pain1 5
#define pain2 17
#define pick 6
#define drop 7
#define idle 8
#define harvest 9
#define cast 10
#define ranged 11
#define meele 12
#define sit_down 13
#define stand_up 14
#define turn_left 15
#define turn_right 16
#define enter_combat 18
#define leave_combat 19

#define move_n 20
#define move_ne 21
#define move_e 22
#define move_se 23
#define move_s 24
#define move_sw 25
#define move_w 26
#define move_nw 27


#define run_n 30
#define run_ne 31
#define run_e 32
#define run_se 33
#define run_s 34
#define run_sw 35
#define run_w 36
#define run_nw 37

#define turn_n 38
#define turn_ne 39
#define turn_e 40
#define turn_se 41
#define turn_s 42
#define turn_sw 43
#define turn_w 44
#define turn_nw 45

#define attack_up_1 46
#define attack_up_2 47
#define attack_up_3 48
#define attack_up_4 49
#define attack_down_1 50
#define attack_down_2 51

//to server commands
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
#define PING_RESPONSE 60

#define GET_DATE 230
#define GET_TIME 231
#define SERVER_STATS 232
#define ORIGINAL_IP 233
#define LOG_IN 140
#define CREATE_CHAR 141

//to client commands
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
#define START_RAIN 15
#define STOP_RAIN 16
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

#define UPGRADE_NEW_VERSION 240
#define UPGRADE_TOO_OLD 241
#define REDEFINE_YOUR_COLORS 248
#define YOU_DONT_EXIST 249
#define LOG_IN_OK 250
#define LOG_IN_NOT_OK 251
#define CREATE_CHAR_OK 252
#define CREATE_CHAR_NOT_OK 253

//common (both to the server and client)
#define RAW_TEXT 0
#define BYE 255

//protocol places
#define PROTOCOL 0 //is an unsigned char

//STATS
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

//SOUND
#define snd_rain     0
#define snd_tele_in  1
#define snd_tele_out 2
#define snd_teleprtr 3
#define snd_thndr_1  4
#define snd_thndr_2  5
#define snd_thndr_3  6
#define snd_thndr_4  7
#define snd_thndr_5  8

#endif
