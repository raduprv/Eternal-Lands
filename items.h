#ifndef __ITEMS_H__
#define __ITEMS_H__

#define ITEM_MONEY 20
#define ITEM_BREAD 21
#define ITEM_MEAD 22
#define ITEM_WINE 23
#define ITEM_ALE 24
#define ITEM_GOLRIC_BOOK 26
#define ITEM_QUARTZ 13
#define ITEM_ROSE_QUARTZ 14
#define ITEM_BLUE_QUARTZ 15
#define ITEM_SULPHUR 17
#define ITEM_MERCURY 16

#define ITEM_VIAL 25
#define ITEM_EMPTY_PARCHMENT 29
#define ITEM_BAKART_PARCHMENT 30

#define ITEM_FIRE_ESSENCE 31
#define ITEM_WATER_ESSENCE 32
#define ITEM_EARTH_ESSENCE 33
#define ITEM_AIR_ESSENCE 34
#define ITEM_SPIRIT_ESSENCE 35
#define ITEM_MATTER_ESSENCE 36
#define ITEM_ENERGY_ESSENCE 37
#define ITEM_LIFE_ESSENCE 38
#define ITEM_DEATH_ESSENCE 39
#define ITEM_HEALTH_ESSENCE 40
#define ITEM_MAGIC_ESSENCE 41

#define ITEM_VEGETABLES 42
#define ITEM_FRUITS 43
#define ITEM_WATERSKIN 44

#define ITEM_IRON_ORE 46
#define ITEM_SILVER_ORE 18
#define ITEM_GOLD_ORE 45
#define ITEM_TITANIUM_ORE 78
#define ITEM_COAL 47
#define ITEM_EMERALD 48
#define ITEM_SAPPHIRE 49
#define ITEM_RUBY 50
#define ITEM_DIAMOND 51
#define ITEM_MORTOR 52
#define ITEM_CACTUS 53
#define ITEM_BONES 54

#define ITEM_BEAR_SKIN 56
#define ITEM_DEER_SKIN 57
#define ITEM_WOLF_SKIN 58
#define ITEM_BEAR_FUR 59
#define ITEM_DEER_FUR 60
#define ITEM_WOLF_FUR 61
#define ITEM_DEER_ANTLERS 62
#define ITEM_WHITE_RABBIT_FUR 66
#define ITEM_BROWN_RABBIT_FUR 67
#define ITEM_GREEN_SNAKE_SKIN 82
#define ITEM_RED_SNAKE_SKIN 83
#define ITEM_BROWN_SNAKE_SKIN 84
#define ITEM_FOX_FUR 85
#define ITEM_PUMA_FUR 86

#define ITEM_POTION_OF_WILDERNESS 68
#define ITEM_POTION_OF_REASONING 69
#define ITEM_POTION_OF_WILL 70
#define ITEM_POTION_OF_PHYSIQUE 71
#define ITEM_POTION_OF_HARVESTING 72
#define ITEM_POTION_OF_ATTACK 73
#define ITEM_POTION_OF_DEFENSE 74
#define ITEM_POTION_OF_MINOR_HEALING 75
#define ITEM_POTION_OF_MANA 76
#define ITEM_POTION_OF_BODY_RESTAURATION 27
#define ITEM_POTION_OF_MIND_RESTAURATION 28
#define ITEM_POTION_OF_FEASTING 77
#define ITEM_POTION_OF_COORDINATION 64
#define ITEM_POTION_OF_VITALITY 65
#define ITEM_POTION_OF_COMBAT 80
#define ITEM_POTION_OF_MANUFACTURING 147
#define ITEM_POTION_OF_SUMMONING 79

#define ITEM_MEAT 55
#define ITEM_COOKED_MEAT 81

#define ITEM_BONE_POWDER 63

#define ITEM_NEEDLE 87
#define ITEM_THREAD 88
#define ITEM_WARM_FUR_GLOVES 89
#define ITEM_FOX_SCARF 90
#define ITEM_FUR_HAT 91
#define ITEM_FUR_CLOAK 92

#define ITEM_LEATHER_GLOVES 93
#define ITEM_ENHANCED_LEATHER_GLOVES 94
#define ITEM_CHAIN_GLOVES 95
#define ITEM_STEEL_GLOVES 96
#define ITEM_LEATHER_ARMOR 97
#define ITEM_IRON_CHAIN_ARMOR 98

#define ITEM_RING_WHITE_STONE 99
#define ITEM_RING_ISLA_PRIMA 100
#define ITEM_RING_DESERT_PINES 101
#define ITEM_RING_PORTLAND 102
#define ITEM_RING_VALEY_OF_DWARVES 103

#define ITEM_RING_OF_DISSENGAGEMENT 127
#define ITEM_RING_OF_DAMAGE 128

#define ITEM_UNICORN_MEDALION 104
#define ITEM_SUN_MEDALION 105
#define ITEM_MOON_MEDALION 106
#define ITEM_STARS_MEDALION 107

#define ITEM_IRON 108
#define ITEM_SILVER 109
#define ITEM_GOLD 110
#define ITEM_STEEL 111
#define ITEM_TITANIUM 112

#define ITEM_PICKAXE 113
#define ITEM_PLATINUM_COINS 114

#define ITEM_IRON_SWORD 115
#define ITEM_IRON_BROAD_SWORD 116
#define ITEM_STEEL_LONG_SWORD 117
#define ITEM_STEEL_TWO_EDGED_SWORD 118
#define ITEM_TITANIUM_STEEL_SHORT_SWORD 119
#define ITEM_TITANIUM_STEEL_SWORD 120
#define ITEM_TITANIUM_SERPENT_SWORD 121

#define ITEM_BATTLE_HAMMER 122
#define ITEM_IRON_BATTLE_HAMMER 139
#define ITEM_WOOD_STAFF 123
#define ITEM_QUATER_STAFF 124

#define ITEM_HAMMER 125
#define ITEM_LEATHER 126

#define ITEM_WOODEN_SHIELD 129
#define ITEM_ENHANCED_WOODEN_SHIELD 130
#define ITEM_IRON_SHIELD 131
#define ITEM_STEEL_SHIELD 132

#define ITEM_MAGE_CLOAK 134
#define ITEM_FIGHTER_CLOAK 135
#define ITEM_HARVESTER_CLOAK 136
#define ITEM_ALCHEMIST_CLOAK 137
#define ITEM_POTION_MAKER_CLOAK 138
#define ITEM_IRON_HELM 140
#define ITEM_STEEL_CHAIN_ARMOR 141
#define ITEM_LEATHER_PANTS 142
#define ITEM_LEATHER_BOOTS 143
#define ITEM_TITANIUM_CHAIN_ARMOR 144

#define ITEM_BEAVER_FUR 145
#define ITEM_RAT_TAIL 146

#define ITEM_DOOM_CLOAK 148
#define ITEM_MASTERY_CLOAK 149
#define ITEM_SERPENT_STONE 150

typedef enum {
	ITEM_ON 			  = 1,
	ITEM_ON_TRADE         = 2,
	ITEM_IN_BANK          = 4,
} item_flags;

typedef enum {
	ITEM_REAGENT           = 1,//can be used in magic
	ITEM_RESOURCE          = 2,//can be used to manufacture
	ITEM_STACKABLE         = 4,
	ITEM_INVENTORY_USABLE  = 8,
	ITEM_TILE_USABLE       = 16,
	ITEM_PLAYER_USABLE     = 32,
	ITEM_OBJECT_USABLE     = 64,
	ITEM_ON_OFF            = 128,
	ITEM_WEARABLE          = 256,//can be worn
} item_definition_flags;

#define NO_WEAR 0
#define LEFT_HAND 1
#define RIGHT_HAND 2
#define BOTH_HANDS 3
#define LEGS 4
#define BOOTS 5
#define HEAD 6
#define BODY 7
#define RING 8
#define AMULET 9
#define SHOULDERS 10

typedef struct {
	char name[64];
	char description[200];
	Uint8 image_id;
	int base_cost;
	int base_quality;
	int base_quantity;
	short weight;
	int flags;

	int teleport_x;
	int teleport_y;
	int teleport_map;

	int increase_attack_on_use;
	int increase_defense_on_use;
	int increase_combat_on_use;
	int increase_manufacturing_on_use;
	int increase_harvesting_on_use;
	int increase_alchemy_on_use;
	int increase_magic_on_use;
	int increase_potion_on_use;
	int increase_armor_on_use;
	int increase_summoning_on_use;

	int increase_physique_on_use;
	int increase_coordination_on_use;
	int increase_reasoning_on_use;
	int increase_will_on_use;
	int increase_instinct_on_use;
	int increase_vitality_on_use;

	int increase_human_nexus_on_use;
	int increase_animal_nexus_on_use;
	int increase_vegetal_nexus_on_use;
	int increase_inorganic_nexus_on_use;
	int increase_artificial_nexus_on_use;
	int increase_magic_nexus_on_use;

	int increase_material_points_on_use;
	int increase_ethereal_points_on_use;
	int increase_karma_on_use;
	int increase_food_on_use;

	int increase_armor_on_wear;

	int damage;
	int accuracy;

	int give_back_item;
	int wearable;
	int what_kind_wearable;
	int what_wearable_id;

	int is_quest_item;

}item_definition;

extern item_definition items[512];

typedef struct {
	short item_category;//a pointer in the array of item definitions
	Uint32 quantity;
}inventory_item;

typedef struct {
	short item_category;//a pointer in the array of item definitions
	int quantity;
}ground_item;

typedef struct {
	short item_category;//a pointer in the array of item definitions
	int quantity;
}trade_item;

#define MAX_SPAWN_POINTS 100

typedef struct {
	int spawn_chance;
	int map_id;
	int x;
	int y;
	int x_dev;
	int y_dev;
	int item_1_id;
	int item_1_min_quantity;
	int item_1_max_quantity;
	int item_1_chance;

	int item_2_id;
	int item_2_min_quantity;
	int item_2_max_quantity;
	int item_2_chance;

	int item_3_id;
	int item_3_min_quantity;
	int item_3_max_quantity;
	int item_3_chance;

	int item_4_id;
	int item_4_min_quantity;
	int item_4_max_quantity;
	int item_4_chance;

	int item_5_id;
	int item_5_min_quantity;
	int item_5_max_quantity;
	int item_5_chance;
}spawn_point;

spawn_point spawn_points[MAX_SPAWN_POINTS];

#endif
