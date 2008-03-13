#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "actor_scripts.h"
#include "actors.h"
#include "asc.h"
#include "cal.h"
#include "cal3d_wrapper.h"
#include "counters.h"
#include "cursors.h"
#include "draw_scene.h"
#include "errors.h"
#include "global.h"
#include "init.h"
#include "interface.h"
#ifdef MISSILES
#include "missiles.h"
#include "new_actors.h"
#endif // MISSILES
#include "multiplayer.h"
#include "new_character.h"
#include "particles.h"
#include "pathfinder.h"
#include "platform.h"
#include "skeletons.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif // NEW_SOUND
#include "tiles.h"
#include "timers.h"
#include "translate.h"
 #include "eye_candy_wrapper.h"
#include "minimap.h"
#include "io/elfilewrapper.h"
#ifdef NEW_LIGHTING
 #include "textures.h"
#endif
#include "actor_init.h"

// mainy of these lists are being phased out by using the id's in XML instead, here as defaults for now
// NOTE: with the new XML standards being used, these are being phased out in preference to the numeric's in the XML
const dict_elem actor_type_dict[] =
	{ { "human female"          , human_female           },
	  { "human male"            , human_male             },
	  { "elf female"            , elf_female             },
	  { "elf male"              , elf_male               },
	  { "dwarf female"          , dwarf_female           },
	  { "dwarf male"            , dwarf_male             },
	  { "wraith"                , wraith                 },
	  { "cyclops"               , cyclops                },
	  { "beaver"                , beaver                 },
	  { "rat"                   , rat                    },
	  { "goblin male 2"         , goblin_male_2          },
	  { "goblin female 1"       , goblin_female_1        },
	  { "town folk 4"           , town_folk4             },
	  { "town folk 5"           , town_folk5             },
	  { "shop girl 3"           , shop_girl3             },
	  { "deer"                  , deer                   },
	  { "bear 1"                , bear                   },
	  { "wolf"                  , wolf                   },
	  { "white rabbit"          , white_rabbit           },
	  { "brown rabbit"          , brown_rabbit           },
	  { "boar"                  , boar                   },
	  { "bear 2"                , bear2                  },
	  { "snake 1"               , snake1                 },
	  { "snake 2"               , snake2                 },
	  { "snake 3"               , snake3                 },
	  { "fox"                   , fox                    },
	  { "puma"                  , puma                   },
	  { "ogre male 1"           , ogre_male_1            },
	  { "goblin male 1"         , goblin_male_1          },
	  { "orc male 1"            , orc_male_1             },
	  { "orc female 1"          , orc_female_1           },
	  { "skeleton"              , skeleton               },
	  { "gargoyle 1"            , gargoyle1              },
	  { "gargoyle 2"            , gargoyle2              },
	  { "gargoyle 3"            , gargoyle3              },
	  { "troll"                 , troll                  },
	  { "chimeran mountain wolf", chimeran_wolf_mountain },
	  { "gnome female"          , gnome_female           },
	  { "gnome male"			, gnome_male             },
	  { "orchan female"			, orchan_female          },
	  { "orchan male"			, orchan_male            },
	  { "draegoni female"		, draegoni_female        },
	  { "draegoni male"			, draegoni_male          },
	  { "skunk 1"				, skunk_1	             },
	  { "racoon 1"				, racoon_1	             },
	  { "unicorn 1"				, unicorn_1	             },
	  { "chimeran desert wolf"	, chimeran_wolf_desert   },
	  { "chimeran forest wolf"	, chimeran_wolf_forest   },
	  { "bear 3"			, bear_3		     },
	  { "bear 4"			, bear_4		     },
	  { "panther"			, panther		     },
	  { "feran"				, feran		     },
	  { "leopard 1"			, leopard_1		     },
	  { "leopard 2"			, leopard_2		     },
	  { "chimeran arctic wolf"	, chimeran_wolf_arctic   },
	  { "tiger 1"				, tiger_1		     },
	  { "tiger 2"				, tiger_2		     },
	  { "armed female orc"		, armed_female_orc		},
	  { "armed male orc"		, armed_male_orc		},
	  { "armed skeleton"		, armed_skeleton		},
	  { "phantom warrior"		, phantom_warrior		},
	  { "imp"					, imp					},
	  { "brownie"				, brownie				},
	  { "leprechaun"			, leprechaun			},
	  { "spider big 1"			, spider_l_1			},
	  { "spider big 2"			, spider_l_2			},
	  { "spider big 3"			, spider_l_3			},
	  { "spider small 1"		, spider_s_1			},
	  { "spider small 2"		, spider_s_2			},
	  { "spider small 3"		, spider_s_3			},
	  { "wood sprite"			, wood_sprite			},
	  { "spider big 4"			, spider_l_4			},
	  { "spider small 4"		, spider_s_4			},
	  { "giant 1"				, giant_1               },
	  { "hobgoblin"				, hobgoblin             },
	  { "yeti"					, yeti                  },
	  { "hobgoblin 1"			, hobgoblin             },
	  { "yeti 1"				, yeti                  },

	  { NULL                    , -1					}
	};

const dict_elem shirt_color_dict[] =
	{ { "black"               , SHIRT_BLACK                },
	  { "blue"                , SHIRT_BLUE                 },
	  { "brown"               , SHIRT_BROWN                },
	  { "grey"                , SHIRT_GREY                 },
	  { "green"               , SHIRT_GREEN                },
	  { "light brown"         , SHIRT_LIGHTBROWN           },
	  { "orange"              , SHIRT_ORANGE               },
	  { "pink"                , SHIRT_PINK                 },
	  { "purple"              , SHIRT_PURPLE               },
	  { "red"                 , SHIRT_RED                  },
	  { "white"               , SHIRT_WHITE                },
	  { "yellow"              , SHIRT_YELLOW               },
	  { "leather armor"       , SHIRT_LEATHER_ARMOR        },
	  { "chain armor"         , SHIRT_CHAIN_ARMOR          },
	  { "steel chain armor"   , SHIRT_STEEL_CHAIN_ARMOR    },
	  { "titanium chain armor", SHIRT_TITANIUM_CHAIN_ARMOR },
	  { "iron plate armor"    , SHIRT_IRON_PLATE_ARMOR     },
	  { "steel plate armor"   , SHIRT_STEEL_PLATE_ARMOR    },
	  { "titanium plate armor", SHIRT_TITANIUM_PLATE_ARMOR },
	  { "augmented leather"   , SHIRT_AUGMENTED_LEATHER_ARMOR },
	  { "fur"                 , SHIRT_FUR                  },
	  { NULL                  , -1                         }
	};

const dict_elem skin_color_dict[] =
	{ { "brown"	, SKIN_BROWN	},
	  { "normal", SKIN_NORMAL	},
	  { "pale"	, SKIN_PALE		},
	  { "tan"	, SKIN_TAN		},
	  { "darkblue", SKIN_DARK_BLUE },	// Elf's only
	  { "dark_blue", SKIN_DARK_BLUE },	// Elf's only, synonym
	  { "white" , SKIN_WHITE    },		// Draegoni only
	  { NULL	, -1			}
	};

const dict_elem hair_color_dict[] =
	{ { "black"	, HAIR_BLACK  },
	  { "blond" , HAIR_BLOND  },
	  { "brown" , HAIR_BROWN  },
	  { "grey"  , HAIR_GRAY   },
	  { "red"   , HAIR_RED    },
	  { "white" , HAIR_WHITE  },
	  { "blue"  , HAIR_BLUE   },	// Draegoni only
	  { "green" , HAIR_GREEN  },	// Draegoni only
	  { "purple", HAIR_PURPLE },	// Draegoni only
	  { "dark_brown",   HAIR_DARK_BROWN},
	  { "strawberry",	HAIR_STRAWBERRY},
	  { "light_blond",	HAIR_LIGHT_BLOND},
	  { "dirty_blond",	HAIR_DIRTY_BLOND},
	  { "brown_gray",	HAIR_BROWN_GRAY},
	  { "dark_gray"	,	HAIR_DARK_GRAY},
	  { "dark_red"	,	HAIR_DARK_RED},
	  { NULL    , -1          }
	};

const dict_elem boots_color_dict[] =
	{ { "black"       , BOOTS_BLACK       },
	  { "brown"       , BOOTS_BROWN       },
	  { "dark brown"  , BOOTS_DARKBROWN   },
	  { "dull brown"  , BOOTS_DULLBROWN   },
	  { "light brown" , BOOTS_LIGHTBROWN  },
	  { "orange"      , BOOTS_ORANGE      },
	  { "leather"     , BOOTS_LEATHER     },
	  { "fur"         , BOOTS_FUR         },
	  { "iron greaves", BOOTS_IRON_GREAVE },
	  { "steel greaves", BOOTS_STEEL_GREAVE },
	  { "titanium greaves", BOOTS_TITANIUM_GREAVE },
	  { "bronze greaves", BOOTS_BRONZE_GREAVE },
	  { "augmented leather", BOOTS_AUGMENTED_LEATHER_GREAVE },
	  { NULL          , -1                }
	};

const dict_elem legs_color_dict[] =
	{ { "black"       , PANTS_BLACK        },
	  { "blue"        , PANTS_BLUE         },
	  { "brown"       , PANTS_BROWN        },
	  { "dark brown"  , PANTS_DARKBROWN    },
	  { "grey"        , PANTS_GREY         },
	  { "green"       , PANTS_GREEN        },
	  { "light brown" , PANTS_LIGHTBROWN   },
	  { "red"         , PANTS_RED          },
	  { "white"       , PANTS_WHITE        },
	  { "leather"     , PANTS_LEATHER      },
	  { "iron cuisses", PANTS_IRON_CUISSES },
	  { "steel cuisses"       , PANTS_STEEL_CUISSES        },
	  { "titanium cuisses"     , PANTS_TITANIUM_CUISSES      },
	  { "bronze cuisses", PANTS_BRONZE_CUISSES },
	  { "augmented leather", PANTS_AUGMENTED_LEATHER_CUISSES },
	  { "fur"         , PANTS_FUR          },
	  { NULL          , -1                 }
	};

const dict_elem cape_color_dict[] =
	{ { "black"     , CAPE_BLACK      },
	  { "blue"      , CAPE_BLUE       },
	  { "blue grey" , CAPE_BLUEGRAY   },
	  { "brown"     , CAPE_BROWN      },
	  { "brown grey", CAPE_BROWNGRAY  },
	  { "grey"      , CAPE_GRAY       },
	  { "green"     , CAPE_GREEN      },
	  { "green grey", CAPE_GREENGRAY  },
	  { "purple"    , CAPE_PURPLE     },
	  { "white"     , CAPE_WHITE      },
	  { "fur"       , CAPE_FUR        },
	  { "gold"      , CAPE_GOLD       },
	  { "red"       , CAPE_RED        },
	  { "orange"    , CAPE_ORANGE     },
	  { "mod"       , CAPE_MOD        },
	  { "derin"     , CAPE_DERIN      },
	  { "ravenod"   , CAPE_RAVENOD    },
	  { "placid"    , CAPE_PLACID     },
	  { "lordvermor",CAPE_LORD_VERMOR},
	  { "aislinn"   , CAPE_AISLINN    },
	  { "soldus"    , CAPE_SOLDUS     },
	  { "lotharion" , CAPE_LOTHARION  },
	  { "learner"   , CAPE_LEARNER    },
//	  { "moonshadow", CAPE_MOONSHADOW },
//	  { "rogue"     , CAPE_ROGUE      },
//	  { "wytter"    , CAPE_WYTTER     },
//	  { "quell"     , CAPE_QUELL      },
	  { "none"      , CAPE_NONE       },
	  { NULL        , -1              }
	};

const dict_elem shield_type_dict[] =
	{ { "wood"         , SHIELD_WOOD          },
	  { "wood enhanced", SHIELD_WOOD_ENHANCED },
	  { "iron"         , SHIELD_IRON          },
	  { "steel"        , SHIELD_STEEL         },
	  { "titanium"     , SHIELD_TITANIUM      },
	  { "bronze"       , SHIELD_BRONZE        },
	  { "none"         , SHIELD_NONE          },
	  { "quiver arrows", QUIVER_ARROWS        },
	  { "quiver bolts" , QUIVER_BOLTS         },
	  { NULL           , -1                   }
	};

const dict_elem weapon_type_dict[] =
	{ { "none"                        , WEAPON_NONE              },
	  { "sword 1"                     , SWORD_1                  },
	  { "sword 2"                     , SWORD_2                  },
	  { "sword 3"                     , SWORD_3                  },
	  { "sword 4"                     , SWORD_4                  },
	  { "sword 5"                     , SWORD_5                  },
	  { "sword 6"                     , SWORD_6                  },
	  { "sword 7"                     , SWORD_7                  },
	  { "staff 1"                     , STAFF_1                  },
	  { "staff 2"                     , STAFF_2                  },
	  { "staff 3"                     , STAFF_3                  },
	  { "staff 4"                     , STAFF_4                  },
	  { "hammer 1"                    , HAMMER_1                 },
	  { "hammer 2"                    , HAMMER_2                 },
	  { "pickaxe"                     , PICKAX                   },
	  { "sword 1 of fire"             , SWORD_1_FIRE             },
	  { "sword 2 of fire"             , SWORD_2_FIRE             },
	  { "sword 2 of ice"              , SWORD_2_COLD             },
	  { "sword 3 of fire"             , SWORD_3_FIRE             },
	  { "sword 3 of ice"              , SWORD_3_COLD             },
	  { "sword 3 of magic"            , SWORD_3_MAGIC            },
	  { "sword 4 of fire"             , SWORD_4_FIRE             },
	  { "sword 4 of ice"              , SWORD_4_COLD             },
	  { "sword 4 of magic"            , SWORD_4_MAGIC            },
	  { "thermal sword 4"             , SWORD_4_THERMAL          },
	  { "sword 5 of fire"             , SWORD_5_FIRE             },
	  { "sword 5 of ice"              , SWORD_5_COLD             },
	  { "sword 5 of magic"            , SWORD_5_MAGIC            },
	  { "thermal sword 5"             , SWORD_5_THERMAL          },
	  { "sword 6 of fire"             , SWORD_6_FIRE             },
	  { "sword 6 of ice"              , SWORD_6_COLD             },
	  { "sword 6 of magic"            , SWORD_6_MAGIC            },
	  { "thermal sword 6"             , SWORD_6_THERMAL          },
	  { "sword 7 of fire"             , SWORD_7_FIRE             },
	  { "sword 7 of ice"              , SWORD_7_COLD             },
	  { "sword 7 of magic"            , SWORD_7_MAGIC            },
	  { "thermal sword 7"             , SWORD_7_THERMAL          },
	  { "pickaxe of magic"            , PICKAX_MAGIC             },
	  { "iron battle axe"             , BATTLEAXE_IRON           },
	  { "steel battle axe"            , BATTLEAXE_STEEL          },
	  { "titanium battle axe"         , BATTLEAXE_TITANIUM       },
	  { "iron battle axe of fire"     , BATTLEAXE_IRON_FIRE      },
	  { "steel battle axe of ice"     , BATTLEAXE_STEEL_COLD     },
	  { "steel battle axe of fire"    , BATTLEAXE_STEEL_FIRE     },
	  { "titanium battle axe of ice"  , BATTLEAXE_TITANIUM_COLD  },
	  { "titanium battle axe of fire" , BATTLEAXE_TITANIUM_FIRE  },
	  { "titanium battle axe of magic", BATTLEAXE_TITANIUM_MAGIC },
	  { "fur gloves"                  , GLOVE_FUR                },
	  { "leather gloves"              , GLOVE_LEATHER            },
	  { "bone 1"              	      , BONE_1	                 },
	  { "stick 1"              	      , STICK_1	                 },
	  { "sword 8"                     , SWORD_EMERALD_CLAYMORE   },
	  { "sword 9"                     , SWORD_CUTLASS            },
	  { "sword 10"                    , SWORD_SUNBREAKER         },
	  { "sword 11"                    , SWORD_ORC_SLAYER         },
	  { "sword 12"                    , SWORD_EAGLE_WING         },
	  { "sword 13"                    , SWORD_RAPIER             },
	  { "sword 14"                    , SWORD_JAGGED_SABER       },
	  { "bronze sword"                , SWORD_BRONZE             },
	  { NULL                          , -1                       }
	};

const dict_elem glow_mode_dict[] =
	{ { "none"   , GLOW_NONE    },
	  { "fire"   , GLOW_FIRE    },
	  { "ice"    , GLOW_COLD    },
	  { "thermal", GLOW_THERMAL },
	  { "magic"  , GLOW_MAGIC   },
	  { NULL     , -1           }
	};

const dict_elem helmet_type_dict[] =
	{ { "iron"   , HELMET_IRON    },
	  { "fur"    , HELMET_FUR     },
	  { "leather", HELMET_LEATHER },
	  { "racoon" , HELMET_RACOON  },
	  { "skunk"  , HELMET_SKUNK   },
	  { "crown_mana"  , HELMET_CROWN_OF_MANA   },
	  { "crown_life"  , HELMET_CROWN_OF_LIFE   },
	  { "steel"  , HELMET_STEEL   },
	  { "titanium",HELMET_TITANIUM},
	  { "bronze" , HELMET_BRONZE  },
	  { "none"   , HELMET_NONE    },
	  { NULL     , -1             }
	};

const dict_elem head_number_dict[] =
	{ { "1" ,  HEAD_1 },
	  { "2" ,  HEAD_2 },
	  { "3" ,  HEAD_3 },
	  { "4" ,  HEAD_4 },
	  { "5" ,  HEAD_5 },
	  { NULL, -1      }
	};

//Forward declarations
int cal_load_weapon_mesh (actor_types *act, const char *fn, const char *kind);
int cal_load_mesh (actor_types *act, const char *fn, const char *kind);
#ifdef NEW_SOUND
int parse_actor_sounds (actor_types *act, xmlNode *cfg);
#endif	//NEW_SOUND

void cal_actor_set_random_idle(int id)
{
	struct CalMixer *mixer;
	int i;
	int random_anim;
	int random_anim_index;

	if (actors_list[id]->calmodel==NULL) return;
	//LOG_TO_CONSOLE(c_green2,"Randomizing");
	//if (actors_list[id]->cur_anim.anim_index==anim.anim_index) return;
	srand( (unsigned)time( NULL ) );
	mixer=CalModel_GetMixer(actors_list[id]->calmodel);
	//Stop previous animation if needed
	if (actors_list[id]->IsOnIdle!=1){
		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==0)) {
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_anim.anim_index,0.05);
		}
		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==1)) {
			CalMixer_RemoveAction(mixer,actors_list[id]->cur_anim.anim_index);
		}
	}

	for (i=0;i<actors_defs[actors_list[id]->actor_type].group_count;++i) {
		random_anim=rand()%(actors_defs[actors_list[id]->actor_type].idle_group[i].count+1);
		if (random_anim<actors_defs[actors_list[id]->actor_type].idle_group[i].count) random_anim_index=actors_defs[actors_list[id]->actor_type].idle_group[i].anim[random_anim].anim_index;
		else random_anim_index=-1;
		if (actors_list[id]->IsOnIdle==1) {
			if (actors_list[id]->cur_idle_anims[i].anim_index!=random_anim_index)
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_idle_anims[i].anim_index,2.0);
		}
		if (actors_list[id]->cur_idle_anims[i].anim_index!=random_anim_index)
			if (random_anim_index>=0) CalMixer_BlendCycle(mixer,random_anim_index,0.5,0.05);
		//safe_snprintf(str, sizeof(str),"%d",random_anim);
		//LOG_TO_CONSOLE(c_green2,str);
		actors_list[id]->cur_idle_anims[i].anim_index=random_anim_index;
		//anim.anim_index,1.0,0.05);else
	}

	//if (anim.kind==0) CalMixer_BlendCycle(mixer,anim.anim_index,1.0,0.05);else
	//CalMixer_ExecuteAction(mixer,anim.anim_index,0.0,0.0);
	//actors_list[id]->cur_anim=anim;
	//actors_list[id]->anim_time=0.0;
	CalModel_Update(actors_list[id]->calmodel,0.0001);//Make changes take effect now
	build_actor_bounding_box(actors_list[id]);
	if (use_animation_program)
	{
		set_transformation_buffers(actors_list[id]);
	}
	actors_list[id]->IsOnIdle= 1;
	actors_list[id]->cur_anim.duration= 0;
	actors_list[id]->anim_time= 0.0;
	actors_list[id]->last_anim_update= cur_time;
	actors_list[id]->cur_anim.anim_index= -1;
#ifdef NEW_SOUND
	if (check_sound_loops(actors_list[id]->cur_anim_sound_cookie))
		stop_sound(actors_list[id]->cur_anim_sound_cookie);
#endif // NEW_SOUND
	actors_list[id]->cur_anim_sound_cookie= 0;
	//if (actors_list[id]->cur_anim.anim_index==-1) actors_list[id]->busy=0;
}


float unwindAngle_Degrees( float fAngle )
{
	fAngle -= 360.0f * (int)( fAngle / 360.0f );
	if( fAngle < 0.0f )
		{
			fAngle += 360.0f;
		}
	return fAngle;
}


float get_rotation_vector( float fStartAngle, float fEndAngle )
{
	float ccw = unwindAngle_Degrees( fStartAngle - fEndAngle );
	float cw = unwindAngle_Degrees( fEndAngle - fStartAngle );
	if(cw<ccw)return cw;
	else return -ccw;
}

int get_motion_vector(int move_cmd, int *dx, int *dy)
{
	int result = 1;
    switch(move_cmd) {
    case move_n:
    case run_n:
        *dx = 0;
        *dy = 1;
        break;
    case move_s:
    case run_s:
        *dx = 0;
        *dy = -1;
        break;
    case move_e:
    case run_e:
        *dx = 1;
        *dy = 0;
        break;
    case move_w:
    case run_w:
        *dx = -1;
        *dy = 0;
        break;
    case move_ne:
    case run_ne:
        *dx = 1;
        *dy = 1;
        break;
    case move_se:
    case run_se:
        *dx = 1;
        *dy = -1;
        break;
    case move_sw:
    case run_sw:
        *dx = -1;
        *dy = -1;
        break;
    case move_nw:
    case run_nw:
        *dx = -1;
        *dy = 1;
        break;

    default:
        *dx = 0;
        *dy = 0;
		result = 0;
        break;
    }
	return result;
}

void animate_actors()
{
	int i;
	static int last_update= 0;
#ifdef NEW_ACTOR_MOVEMENT
    int time_diff = cur_time-last_update;
    int tmp_time_diff;
#endif

	// lock the actors_list so that nothing can interere with this look
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0; i<max_actors; i++) {
		if(actors_list[i]) {
			if(actors_list[i]->moving) {
#ifndef NEW_ACTOR_MOVEMENT
				actors_list[i]->movement_frames_left--;
				if(!actors_list[i]->movement_frames_left){	//we moved all the way
#else // NEW_ACTOR_MOVEMENT
				if (time_diff <= actors_list[i]->movement_time_left+40) {
					actors_list[i]->x_pos += actors_list[i]->move_x_speed*time_diff;
					actors_list[i]->y_pos += actors_list[i]->move_y_speed*time_diff;
					actors_list[i]->z_pos += actors_list[i]->move_z_speed*time_diff;
				}
				else {
					actors_list[i]->x_pos += actors_list[i]->move_x_speed*actors_list[i]->movement_time_left;
					actors_list[i]->y_pos += actors_list[i]->move_y_speed*actors_list[i]->movement_time_left;
					actors_list[i]->z_pos += actors_list[i]->move_z_speed*actors_list[i]->movement_time_left;
				}
                actors_list[i]->movement_time_left -= time_diff;
				if(actors_list[i]->movement_time_left <= 0){	//we moved all the way
#endif // NEW_ACTOR_MOVEMENT
					Uint8 last_command;
                    int dx, dy;

					actors_list[i]->moving= 0;	//don't move next time, ok?
#ifndef NEW_ACTOR_MOVEMENT
					actors_list[i]->after_move_frames_left= 3;	//this is done to prevent going to idle imediatelly
#endif // NEW_ACTOR_MOVEMENT
					//now, we need to update the x/y_tile_pos, and round off
					//the x/y_pos according to x/y_tile_pos
					last_command= actors_list[i]->last_command;
                    if (get_motion_vector(last_command, &dx, &dy)) {
						actors_list[i]->x_tile_pos += dx;
						actors_list[i]->y_tile_pos += dy;

#ifndef NEW_ACTOR_MOVEMENT
						//ok, now update the x/y_pos
						actors_list[i]->x_pos= actors_list[i]->x_tile_pos*0.5;
						actors_list[i]->y_pos= actors_list[i]->y_tile_pos*0.5;
#endif // NEW_ACTOR_MOVEMENT

						// and update the minimap if we need to
						if(actors_list[i]->actor_id == yourself){
							update_exploration_map();
						}
						minimap_touch();
#ifdef NEW_ACTOR_MOVEMENT
						actors_list[i]->busy = 0;
						if (actors_list[i]->que[0] >= move_n &&
							actors_list[i]->que[0] <= move_nw) {
							next_command();
						}
						else {
							actors_list[i]->x_pos= actors_list[i]->x_tile_pos*0.5;
							actors_list[i]->y_pos= actors_list[i]->y_tile_pos*0.5;
							actors_list[i]->z_pos= get_actor_z(actors_list[i]);
						}
#endif // NEW_ACTOR_MOVEMENT
					}
#ifdef NEW_ACTOR_MOVEMENT
					else {
						actors_list[i]->busy = 0;
					}
#endif // NEW_ACTOR_MOVEMENT
				}
#ifndef NEW_ACTOR_MOVEMENT
				else {
					actors_list[i]->x_pos+= actors_list[i]->move_x_speed;
					actors_list[i]->y_pos+= actors_list[i]->move_y_speed;
					actors_list[i]->z_pos+= actors_list[i]->move_z_speed;
				}
#endif // NEW_ACTOR_MOVEMENT
			}
#ifndef NEW_ACTOR_MOVEMENT
            else {//Not moving
				if(actors_list[i]->after_move_frames_left){
					actors_list[i]->after_move_frames_left--;
					if (actors_list[i]->actor_id == yourself)  {
                        char str[255];
						safe_snprintf(str,sizeof(str),"Left: %d",actors_list[i]->after_move_frames_left);
					}
					if(!actors_list[i]->after_move_frames_left){
						//if (actors_list[i]->actor_id == yourself) LOG_TO_CONSOLE(c_green2,"Free");
						actors_list[i]->busy= 0;
					}
				}
			}
#endif // NEW_ACTOR_MOVEMENT

			if(actors_list[i]->rotating) {
#ifndef NEW_ACTOR_MOVEMENT
				actors_list[i]->rotate_frames_left--;
				if(!actors_list[i]->rotate_frames_left)//we rotated all the way
					actors_list[i]->rotating= 0;//don't rotate next time, ok?
				actors_list[i]->x_rot+= actors_list[i]->rotate_x_speed;
				actors_list[i]->y_rot+= actors_list[i]->rotate_y_speed;
				actors_list[i]->z_rot+= actors_list[i]->rotate_z_speed;
#else // NEW_ACTOR_MOVEMENT
				actors_list[i]->rotate_time_left -= time_diff;
				if (actors_list[i]->rotate_time_left <= 0) { //we rotated all the way
					actors_list[i]->rotating= 0;//don't rotate next time, ok?
                    tmp_time_diff = time_diff + actors_list[i]->rotate_time_left;
                }
                else {
                    tmp_time_diff = time_diff;
                }
				actors_list[i]->x_rot+= actors_list[i]->rotate_x_speed*tmp_time_diff;
				actors_list[i]->y_rot+= actors_list[i]->rotate_y_speed*tmp_time_diff;
				actors_list[i]->z_rot+= actors_list[i]->rotate_z_speed*tmp_time_diff;
#endif // NEW_ACTOR_MOVEMENT
				if(actors_list[i]->z_rot >= 360) {
					actors_list[i]->z_rot -= 360;
				} else if (actors_list[i]->z_rot <= 0) {
					actors_list[i]->z_rot += 360;
				}
			}

#ifdef	NEW_ACTOR_ANIMATION
			actors_list[i]->anim_time+= ((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0;
#else
			actors_list[i]->anim_time+= (cur_time-last_update)/1000.0;
#endif
#ifndef	DYNAMIC_ANIMATIONS
			if (actors_list[i]->calmodel!=NULL){
#ifdef	NEW_ACTOR_ANIMATION
				CalModel_Update(actors_list[i]->calmodel, (((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0));
#else
				CalModel_Update(actors_list[i]->calmodel,((cur_time-last_update)/1000.0));
#endif
				build_actor_bounding_box(actors_list[i]);
#ifdef MISSILES
				missiles_rotate_actor_bones(actors_list[i]);
#endif
				if (use_animation_program)
				{
					set_transformation_buffers(actors_list[i]);
				}
			}
#endif	//DYNAMIC_ANIMATIONS
		}
	}
	// unlock the actors_list since we are done now
	UNLOCK_ACTORS_LISTS();

	last_update = cur_time;
}




int coun= 0;
void move_to_next_frame()
{
	int i;
	//int numFrames=0;
	//char frame_exists;
	//struct CalMixer *mixer;
	//char str[255];

	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]!=NULL) {
			if (actors_list[i]->calmodel!=NULL) {
				if ((actors_list[i]->stop_animation==1)&&(actors_list[i]->anim_time>=actors_list[i]->cur_anim.duration)){
					actors_list[i]->busy=0;
#ifdef MISSILES
					if (!actors_list[i]->in_aim_mode &&
						actors_list[i]->unwear_item_type_after_animation >= 0) {
						int unwear_item_type = actors_list[i]->unwear_item_type_after_animation;
						missiles_log_message("%s (%d): unwearing item type %d now\n",
                                             actors_list[i]->actor_name,
                                             actors_list[i]->actor_id,
                                             unwear_item_type);
						actors_list[i]->unwear_item_type_after_animation = -1;
						unwear_item_from_actor(actors_list[i]->actor_id, unwear_item_type);
						if (actors_list[i]->wear_item_type_after_animation >= 0 &&
							actors_list[i]->wear_item_id_after_animation >= 0) {
							int wear_item_type = actors_list[i]->wear_item_type_after_animation;
							int wear_item_id = actors_list[i]->wear_item_id_after_animation;
							missiles_log_message("%s (%d): wearing item type %d now\n",
												 actors_list[i]->actor_name,
                                                 actors_list[i]->actor_id,
                                                 wear_item_type);
							actors_list[i]->wear_item_type_after_animation = -1;
							actors_list[i]->wear_item_id_after_animation = -1;
							actor_wear_item(actors_list[i]->actor_id, wear_item_type, wear_item_id);
						}
					}
#endif // MISSILES
				}
			}

			if ((actors_list[i]->IsOnIdle)&&(actors_list[i]->anim_time>=5.0)&&(actors_list[i]->stop_animation!=1)) {
				cal_actor_set_random_idle(i);
			} else if(!actors_list[i]->IsOnIdle && actors_list[i]->stand_idle && actors_list[i]->anim_time>=5.0){
				// lets see if we want to change the idle animation
				// make sure we have at least two idles, and add a randomizer to continue
				if(actors_defs[actors_list[i]->actor_type].cal_idle2_frame.anim_index != -1 && RAND(0,50) == 0){
					// pick what we want the next idle to be
					// 75% chance to do idle1
					if(RAND(0, 3) == 0){
						// and check to see if we are changing the animation or not
						if(actors_list[i]->cur_anim.anim_index != actors_defs[actors_list[i]->actor_type].cal_idle2_frame.anim_index){
							cal_actor_set_anim_delay(i, actors_defs[actors_list[i]->actor_type].cal_idle2_frame, 0.5f); // normal idle
						}
					} else {
						// and check to see if we are changing the animation or not
						if(actors_list[i]->cur_anim.anim_index != actors_defs[actors_list[i]->actor_type].cal_idle1_frame.anim_index){
							cal_actor_set_anim_delay(i, actors_defs[actors_list[i]->actor_type].cal_idle1_frame, 0.5f); // normal idle
						}
					}
				}
			}

			if (actors_list[i]->cur_anim.anim_index==-1) actors_list[i]->busy=0;

			//first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
			if(actors_list[i]->damage_ms) {
				actors_list[i]->damage_ms-=80;
				if(actors_list[i]->damage_ms<0)actors_list[i]->damage_ms=0;
			}

			//9 frames, not moving, and another command is queued farther on (based on how long we've done this action)
			if(!actors_list[i]->moving && !actors_list[i]->rotating){
				/*
					actors_list[i]->stop_animation=1;	//force stopping, not looping
					actors_list[i]->busy=0;	//ok, take the next command
					LOG_TO_CONSOLE(c_green2,"FREE");
					//Idle here?
				*/
			}

			if(actors_list[i]->stop_animation) {

				//we are done with this guy
				//Should we go into idle here?
			}
		}
	}
	UNLOCK_ACTORS_LISTS();
}

void set_on_idle(int actor_idx)
{
    actor *a = actors_list[actor_idx];
    if(!a->dead) {
        a->stop_animation=0;
        
        if(a->fighting){
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_combat_idle_frame);
        }
#ifdef MISSILES
        else if(a->in_aim_mode){
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].weapon[a->cur_weapon].cal_range_idle_frame);
        }
#endif // MISSILES
        else if(!a->sitting) {
            // we are standing, see if we can activate a stand idle
            if(!a->stand_idle){
                if (actors_defs[a->actor_type].group_count == 0)
                {
                    // 75% chance to do idle1
                    if (actors_defs[a->actor_type].cal_idle2_frame.anim_index != -1 && RAND(0, 3) == 0){
                        cal_actor_set_anim(actor_idx, actors_defs[a->actor_type].cal_idle2_frame); // normal idle
                    } else {
                        cal_actor_set_anim(actor_idx, actors_defs[a->actor_type].cal_idle1_frame); // normal idle
                    }
                    
                }
                else
                {
                    cal_actor_set_random_idle(actor_idx);
                    a->IsOnIdle=1;
                }
                
                a->stand_idle=1;
            }
        } else	{
            // we are sitting, see if we can activate the sit idle
            if(!a->sit_idle) {
                cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_idle_sit_frame);
                a->sit_idle=1;
            }
        }
    }
}

//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i;
	int max_queue=0;

#ifndef NEW_ACTOR_MOVEMENT
	LOCK_ACTORS_LISTS();
#endif // NEW_ACTOR_MOVEMENT
	for(i=0;i<max_actors;i++){
		if(!actors_list[i])continue;//actor exists?
		if(!actors_list[i]->busy
#ifndef NEW_ACTOR_MOVEMENT
           || (actors_list[i]->busy &&
               actors_list[i]->after_move_frames_left &&
               (actors_list[i]->que[0]>=move_n &&
                actors_list[i]->que[0]<=move_nw))
#endif // NEW_ACTOR_MOVEMENT
           ){//Are we busy?
			if(actors_list[i]->que[0]==nothing){//Is the queue empty?
				//if que is empty, set on idle
                set_on_idle(i);
				actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
			} else {
				int actor_type;
				int last_command=actors_list[i]->last_command;
				float z_rot=actors_list[i]->z_rot;
				float targeted_z_rot;
				int k;
				int no_action = 0;

				actors_list[i]->sit_idle=0;
				actors_list[i]->stand_idle=0;

				actor_type=actors_list[i]->actor_type;

				switch(actors_list[i]->que[0]) {
					case kill_me:
/*						if(actors_list[i]->remapped_colors)
						glDeleteTextures(1,&actors_list[i]->texture_id);
						ec_actor_delete(actors_list[i]);
						free(actors_list[i]);
						actors_list[i]=0;*/ //Obsolete
						break;
					case die1:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_die1_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->dead=1;
						break;
					case die2:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_die2_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->dead=1;
						break;
					case pain1:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_pain1_frame);
						actors_list[i]->stop_animation=1;
						break;
					case pain2:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_pain2_frame);
						actors_list[i]->stop_animation=1;
						break;
					case pick:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_pick_frame);
						actors_list[i]->stop_animation=1;
						break;
					case drop:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_drop_frame);
						actors_list[i]->stop_animation=1;
						break;
					case harvest:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_harvest_frame);
						actors_list[i]->stop_animation=1;
						LOG_TO_CONSOLE(c_green2,"Harvesting!");
						break;
					case cast:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_attack_cast_frame);
						actors_list[i]->stop_animation=1;
						break;
					case ranged:
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_attack_ranged_frame);
						actors_list[i]->stop_animation=1;
						break;
					case sit_down:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_sit_down_frame);
						//cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_1_frame);
						//cal_actor_set_anim(i,actors_defs[actor_type].cal_in_combat_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->sitting=1;
						if(actors_list[i]->actor_id==yourself)
							you_sit_down();
						break;
					case stand_up:
						//LOG_TO_CONSOLE(c_green2,"stand_up");
						cal_actor_set_anim(i,actors_defs[actor_type].cal_stand_up_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->sitting=0;
						if(actors_list[i]->actor_id==yourself)
							you_stand_up();
						break;
					case enter_combat:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_in_combat_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;
						//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Enter Combat");
						break;
					case leave_combat:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_out_combat_frame);
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=0;
						break;
					case attack_up_1:
						if(actors_list[i]->is_enhanced_model){
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_1_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_1_frame);
						}
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case attack_up_2:
						if(actors_list[i]->is_enhanced_model){
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_1_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_2_frame);
						}
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case attack_up_3:
						if(actors_list[i]->is_enhanced_model){
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_3_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case attack_up_4:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_4_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case attack_down_1:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_down_1_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_down_1_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case attack_down_2:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_down_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_down_2_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case turn_left:
						//LOG_TO_CONSOLE(c_green2,"turn left");
#ifndef NEW_ACTOR_MOVEMENT
						actors_list[i]->rotate_z_speed=45.0/27.0;
						actors_list[i]->rotate_frames_left=27;
#else // NEW_ACTOR_MOVEMENT
						actors_list[i]->rotate_z_speed=45.0/540.0;
						actors_list[i]->rotate_time_left=540;
#endif // NEW_ACTOR_MOVEMENT
						actors_list[i]->rotating=1;
						//generate a fake movement, so we will know when to make the actor
						//not busy
						actors_list[i]->move_x_speed=0;
						actors_list[i]->move_y_speed=0;
						actors_list[i]->move_z_speed=0;
#ifndef NEW_ACTOR_MOVEMENT
						actors_list[i]->movement_frames_left=27;
#else // NEW_ACTOR_MOVEMENT
						actors_list[i]->movement_time_left=540;
#endif // NEW_ACTOR_MOVEMENT
						actors_list[i]->moving=1;
						//test
						if(!actors_list[i]->fighting){
							cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_walk_frame);
						}
						actors_list[i]->stop_animation=0;
						break;
					case turn_right:
					//LOG_TO_CONSOLE(c_green2,"turn right");
#ifndef NEW_ACTOR_MOVEMENT
						actors_list[i]->rotate_z_speed=-45.0/27.0;
						actors_list[i]->rotate_frames_left=27;
#else // NEW_ACTOR_MOVEMENT
						actors_list[i]->rotate_z_speed=-45.0/540.0;
						actors_list[i]->rotate_time_left=540;
#endif // NEW_ACTOR_MOVEMENT
						actors_list[i]->rotating=1;
						//generate a fake movement, so we will know when to make the actor
						//not busy
						actors_list[i]->move_x_speed=0;
						actors_list[i]->move_y_speed=0;
						actors_list[i]->move_z_speed=0;
#ifndef NEW_ACTOR_MOVEMENT
						actors_list[i]->movement_frames_left=27;
#else // NEW_ACTOR_MOVEMENT
						actors_list[i]->movement_time_left=540;
#endif // NEW_ACTOR_MOVEMENT
						actors_list[i]->moving=1;
						//test
						if(!actors_list[i]->fighting){
							cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_walk_frame);
						}
						actors_list[i]->stop_animation=0;
						break;

#ifdef MISSILES
				case enter_aim_mode:
					if (!actors_list[i]->in_aim_mode) {
						missiles_log_message("%s (%d): enter in aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_range_in_frame);

						actors_list[i]->cal_h_rot_start = 0.0;
						actors_list[i]->cal_v_rot_start = 0.0;
						actors_list[i]->reload[0] = 0;
						actors_list[i]->shot_type[0] = NORMAL_SHOT;
						actors_list[i]->shots_count = 0;
					}
					else {
                        float range_rotation;

						missiles_log_message("%s (%d): aiming again (time=%d)", actors_list[i]->actor_name, actors_list[i]->actor_id, cur_time);
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_range_idle_frame);
						actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_h_rot_end *
														   actors_list[i]->cal_rotation_blend);
						actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_v_rot_end *
														   actors_list[i]->cal_rotation_blend);

						range_rotation = missiles_compute_actor_rotation(&actors_list[i]->cal_h_rot_end,
																		 &actors_list[i]->cal_v_rot_end,
																		 actors_list[i], actors_list[i]->range_target_aim);
						actors_list[i]->cal_rotation_blend = 0.0;
						actors_list[i]->cal_rotation_speed = 1.0/360.0;
                        actors_list[i]->cal_last_rotation_time = cur_time;
						actors_list[i]->are_bones_rotating = 1;
						actors_list[i]->stop_animation = 1;
						
						if (range_rotation != 0.0) {
							missiles_log_message("%s (%d): not facing its target => client side rotation needed",
                                                 actors_list[i]->actor_name, actors_list[i]->actor_id);
#ifndef NEW_ACTOR_MOVEMENT
							if (actors_list[i]->rotating) {
								range_rotation += actors_list[i]->rotate_z_speed * actors_list[i]->rotate_frames_left;
							}
							actors_list[i]->rotate_z_speed = range_rotation/18.0;
							actors_list[i]->rotate_frames_left=18;
#else // NEW_ACTOR_MOVEMENT
							if (actors_list[i]->rotating) {
								range_rotation += actors_list[i]->rotate_z_speed * actors_list[i]->rotate_time_left;
							}
							actors_list[i]->rotate_z_speed = range_rotation/360.0;
							actors_list[i]->rotate_time_left=360;
#endif // NEW_ACTOR_MOVEMENT
							actors_list[i]->rotating=1;
						}
					}
					break;

				case leave_aim_mode:
					if (!actors_list[i]->in_aim_mode) {
						if (actors_list[i]->cal_rotation_blend < 0.0 ||
							(actors_list[i]->cal_h_rot_end == 0.0 &&
							 actors_list[i]->cal_v_rot_end == 0.0)) {
							log_error("next_command: trying to leave range mode while we are not in it => aborting safely...");
							no_action = 1;
							break;
						}
						else {
							log_error("next_command: trying to leave range mode while we are not in it => continuing because of a wrong actor bones rotation!");
						}
					}

					missiles_log_message("%s (%d): leaving aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
					cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_range_out_frame);
					actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_h_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_v_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_h_rot_end = 0.0;
					actors_list[i]->cal_v_rot_end = 0.0;
					actors_list[i]->cal_rotation_blend = 0.0;
					actors_list[i]->cal_rotation_speed = 1.0/360.0;
                    actors_list[i]->cal_last_rotation_time = cur_time;
					actors_list[i]->are_bones_rotating = 1;
					actors_list[i]->in_aim_mode = 0;
					actors_list[i]->stop_animation = 1;
					break;

/* 				case aim_mode_reload: */
/* 					missiles_log_message("%s (%d): reload after next fire", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					actors_list[i]->reload = 1; */
/*  					no_action = 1; */
/* 					break; */

				case aim_mode_fire:
					if (!actors_list[i]->in_aim_mode) {
						log_error("next_command: trying to fire an arrow out of range mode => aborting!");
						no_action = 1;
						break;
					}

					if (actors_list[i]->reload[0]) {
						missiles_log_message("%s (%d): fire and reload", actors_list[i]->actor_name, actors_list[i]->actor_id);
						// launch fire and reload animation
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_range_fire_frame);
						actors_list[i]->in_aim_mode = 1;
					}
					else {
						missiles_log_message("%s (%d): fire and leave aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
						// launch fire and leave aim mode animation
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_range_fire_out_frame);
						actors_list[i]->in_aim_mode = 0;
					}
					
					actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_h_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_v_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_h_rot_end = 0.0;
					actors_list[i]->cal_v_rot_end = 0.0;
					actors_list[i]->cal_rotation_blend = 0.0;
					actors_list[i]->cal_rotation_speed = 1.0/360.0;
                    actors_list[i]->cal_last_rotation_time = cur_time;
					actors_list[i]->are_bones_rotating = 1;
					actors_list[i]->stop_animation = 1;

					/* In case of a missed shot due to a collision with an actor,
					 * the server send the position of the actor with 0.0 for the Z coordinate.
					 * So we have to compute the coordinate of the ground at this position.
					 */
					if (actors_list[i]->shot_type[0] == MISSED_SHOT &&
						actors_list[i]->range_target_fire[0][2] == 0.0) {
						int tile_x = (int)(actors_list[i]->range_target_fire[0][0]*2.0);
						int tile_y = (int)(actors_list[i]->range_target_fire[0][1]*2.0);
						actors_list[i]->range_target_fire[0][2] = height_map[tile_y*tile_map_size_x*6+tile_x]*0.2-2.2;
                        missiles_log_message("missed shot detected: new height computed: %f", actors_list[i]->range_target_fire[0][2]);
					}

#ifdef DEBUG
					{
						float aim_angle = atan2f(actors_list[i]->range_target_aim[1] - actors_list[i]->y_pos,
												 actors_list[i]->range_target_aim[0] - actors_list[i]->x_pos);
						float fire_angle = atan2f(actors_list[i]->range_target_fire[0][1] - actors_list[i]->y_pos,
												 actors_list[i]->range_target_fire[0][0] - actors_list[i]->x_pos);
						if (aim_angle < 0.0) aim_angle += 2*M_PI;
						if (fire_angle < 0.0) fire_angle += 2*M_PI;
						if (fabs(fire_angle - aim_angle) > M_PI/8.0) {
							char msg[512];
							sprintf(msg,
									"%s (%d): WARNING! Target position is too different from aim position: pos=(%f,%f,%f) aim=(%f,%f,%f) target=(%f,%f,%f) aim_angle=%f target_angle=%f",
                                    actors_list[i]->actor_name,
                                    actors_list[i]->actor_id,
									actors_list[i]->x_pos,
									actors_list[i]->y_pos,
									actors_list[i]->z_pos,
									actors_list[i]->range_target_aim[0],
									actors_list[i]->range_target_aim[1],
									actors_list[i]->range_target_aim[2],
									actors_list[i]->range_target_fire[0][0],
									actors_list[i]->range_target_fire[0][1],
									actors_list[i]->range_target_fire[0][2],
									aim_angle, fire_angle);
							LOG_TO_CONSOLE(c_red2, msg);
							missiles_log_message(msg);
						}
					}
#endif // DEBUG

					missiles_fire_arrow(actors_list[i], actors_list[i]->range_target_fire[0], actors_list[i]->shot_type[0]);

					// we remove the current shot from the queue
					if (actors_list[i]->shots_count > 0) {
						int j;
						for (j = 1; j < MAX_SHOTS_QUEUE; ++j) {
							memcpy(actors_list[i]->range_target_fire[j-1],
								   actors_list[i]->range_target_fire[j],
								   3*sizeof(float));
							actors_list[i]->shot_type[j-1] = actors_list[i]->shot_type[j];
						}
						--actors_list[i]->shots_count;
					}
					else {
						log_error("the shots queue is already empty!");
					}
					actors_list[i]->reload[actors_list[i]->shots_count] = 0;
					actors_list[i]->shot_type[actors_list[i]->shots_count] = NORMAL_SHOT;
					break;

/* 				case missile_miss: */
/* 					missiles_log_message("%s (%d): will miss his target", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					if (actors_list[i]->shots_count < MAX_SHOTS_QUEUE) */
/* 						actors_list[i]->shot_type[actors_list[i]->shots_count] = MISSED_SHOT; */
/*  					no_action = 1; */
/* 					break; */

/* 				case missile_critical: */
/* 					missiles_log_message("%s (%d): will do a critical hit", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					if (actors_list[i]->shots_count < MAX_SHOTS_QUEUE) */
/* 						actors_list[i]->shot_type[actors_list[i]->shots_count] = CRITICAL_SHOT; */
/*  					no_action = 1; */
/* 					break; */

/* 				case unwear_bow: */
/* 					unwear_item_from_actor(actors_list[i]->actor_id, KIND_OF_WEAPON); */
/*  					no_action = 1; */
/* 					break; */

/* 				case unwear_quiver: */
/* 					unwear_item_from_actor(actors_list[i]->actor_id, KIND_OF_SHIELD); */
/*  					no_action = 1; */
/* 					break; */
#endif // MISSILES

					//ok, now the movement, this is the tricky part
					default:
						if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw) {
							float rotation_angle;
#ifdef NEW_ACTOR_MOVEMENT
                            int dx, dy;
#endif // NEW_ACTOR_MOVEMENT

							actors_list[i]->moving=1;
							actors_list[i]->fighting=0;
							if(last_command<move_n || last_command>move_nw){//update the frame name too
								cal_actor_set_anim(i,actors_defs[actor_type].cal_walk_frame);
								actors_list[i]->stop_animation=0;
							}

							if(last_command!=actors_list[i]->que[0]){ //Calculate the rotation
								targeted_z_rot=(actors_list[i]->que[0]-move_n)*45.0f;
								rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
#ifndef NEW_ACTOR_MOVEMENT
								actors_list[i]->rotate_z_speed=rotation_angle/18;
#else // NEW_ACTOR_MOVEMENT
								actors_list[i]->rotate_z_speed=rotation_angle/360.0;
#endif // NEW_ACTOR_MOVEMENT
								if(auto_camera && actors_list[i]->actor_id==yourself){
#ifndef NEW_CAMERA
									camera_rotation_speed=rotation_angle/54;
									camera_rotation_frames=54;
#else // NEW_CAMERA
									camera_rotation_speed=rotation_angle/1000.0;
									camera_rotation_duration=1000;
#endif // NEW_CAMERA
								}

#ifndef NEW_ACTOR_MOVEMENT
								actors_list[i]->rotate_frames_left=18;
#else // NEW_ACTOR_MOVEMENT
								actors_list[i]->rotate_time_left=360;
#endif // NEW_ACTOR_MOVEMENT
								actors_list[i]->rotating=1;
							}
#ifndef NEW_ACTOR_MOVEMENT
                            else targeted_z_rot=z_rot;
#endif // NEW_ACTOR_MOVEMENT

							//ok, now calculate the motion vector...
#ifndef NEW_ACTOR_MOVEMENT
							actors_list[i]->move_x_speed=(actors_defs[actor_type].walk_speed/3.0f)*sin(targeted_z_rot*M_PI/180.0);
							actors_list[i]->move_y_speed=(actors_defs[actor_type].walk_speed/3.0f)*cos(targeted_z_rot*M_PI/180.0);
							actors_list[i]->movement_frames_left=54/4;
							actors_list[i]->after_move_frames_left=0;
							//test to see if we have a diagonal movement, and if we do, adjust the speeds
							if((actors_list[i]->move_x_speed>0.01f || actors_list[i]->move_x_speed<-0.01f)
							   && (actors_list[i]->move_y_speed>0.01f || actors_list[i]->move_y_speed<-0.01f)) {
								actors_list[i]->move_x_speed*=1.4142315;
								actors_list[i]->move_y_speed*=1.4142315;
							}
#else // NEW_ACTOR_MOVEMENT
                            get_motion_vector(actors_list[i]->que[0], &dx, &dy);

							/* if other move commands are waiting in the queue,
							 * we walk at a speed that is close to the server speed
							 * else we walk at a slightly slower speed to wait next
							 * incoming walking commands */
                            if (actors_list[i]->que[1] >= move_n &&
                                actors_list[i]->que[1] <= move_nw) {
                                if (actors_list[i]->que[2] >= move_n &&
                                    actors_list[i]->que[2] <= move_nw) {
									if (actors_list[i]->que[3] >= move_n &&
										actors_list[i]->que[3] <= move_nw)
										actors_list[i]->movement_time_left = 225; // 3 moves
									else
										actors_list[i]->movement_time_left = 250; // 2 moves
								}
                                else
                                    actors_list[i]->movement_time_left = 275; // 1 move
                            }
                            else {
                                actors_list[i]->movement_time_left = 300; // 0 move
                            }
							// if we have a diagonal motion, we slow down the animation a bit
							if (dx != 0 && dy != 0)
								actors_list[i]->movement_time_left = (int)(actors_list[i]->movement_time_left*1.2+0.5);
							
                            // we compute the moving speeds in x, y and z directions
                            actors_list[i]->move_x_speed = 0.5*(dx+actors_list[i]->x_tile_pos)-actors_list[i]->x_pos;
                            actors_list[i]->move_y_speed = 0.5*(dy+actors_list[i]->y_tile_pos)-actors_list[i]->y_pos;
                            actors_list[i]->move_z_speed = -2.2 + height_map[(actors_list[i]->y_tile_pos+dy)*tile_map_size_x*6+actors_list[i]->x_tile_pos+dx]*0.2 - actors_list[i]->z_pos;
                            actors_list[i]->move_x_speed /= (float)actors_list[i]->movement_time_left;
                            actors_list[i]->move_y_speed /= (float)actors_list[i]->movement_time_left;
                            actors_list[i]->move_z_speed /= (float)actors_list[i]->movement_time_left;

                            /* we change the speed of the walking animation according to the walking speed and to the size of the actor
                             * we suppose here that the speed of the walking animation is 2 meters per second (1 tile in 250ms) */
                            actors_list[i]->cur_anim.duration_scale = actors_defs[actor_type].cal_walk_frame.duration_scale;
                            actors_list[i]->cur_anim.duration_scale *= 250.0/(actors_list[i]->movement_time_left*actors_list[i]->scale);
							if (actors_defs[actor_type].actor_scale != 1.0)
								actors_list[i]->cur_anim.duration_scale /= actors_defs[actor_type].actor_scale;
							else
								actors_list[i]->cur_anim.duration_scale /= actors_defs[actor_type].scale;
                            if (dx != 0 && dy != 0)
                                actors_list[i]->cur_anim.duration_scale *= 1.4142315;
#endif // NEW_ACTOR_MOVEMENT
						} else if(actors_list[i]->que[0]>=turn_n && actors_list[i]->que[0]<=turn_nw) {
							float rotation_angle;

							targeted_z_rot=(actors_list[i]->que[0]-turn_n)*45.0f;
							rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
#ifndef NEW_ACTOR_MOVEMENT
							actors_list[i]->rotate_z_speed=rotation_angle/18.0f;
							actors_list[i]->rotate_frames_left=18;
#else // NEW_ACTOR_MOVEMENT
							actors_list[i]->rotate_z_speed=rotation_angle/360.0f;
							actors_list[i]->rotate_time_left=360;
#endif // NEW_ACTOR_MOVEMENT
							actors_list[i]->rotating=1;
							actors_list[i]->stop_animation=1;
#ifdef MISSILES
							missiles_log_message("%s (%d): rotation %d requested", actors_list[i]->actor_name, actors_list[i]->actor_id, actors_list[i]->que[0] - turn_n);
#endif // MISSILES
						}
					}

					//mark the actor as being busy
					if (!no_action)
						actors_list[i]->busy=1;
					//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Busy");
					//save the last command. It is especially good for run and walk
					actors_list[i]->last_command=actors_list[i]->que[0];

#ifdef MISSILES
					/* We do the enter in aim mode in two steps in order the actor have
					 * the time to do the load animation before rotating bones. This is
					 * necessary in the case of cross bows where the actor need to use
					 * his foot to reload. So here, we don't remove the enter aim mode
					 * from the queue in order to treat it again but this time in aim mode.
					 */
					if (actors_list[i]->que[0] == enter_aim_mode && !actors_list[i]->in_aim_mode) {
						actors_list[i]->in_aim_mode = 1;
						continue;
					}
#endif // MISSILES

					//move que down with one command
					for(k=0;k<MAX_CMD_QUEUE-1;k++) {
						if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
						actors_list[i]->que[k]=actors_list[i]->que[k+1];
					}
					actors_list[i]->que[k]=nothing;
				}
			}
		}
#ifndef NEW_ACTOR_MOVEMENT
	UNLOCK_ACTORS_LISTS();
#endif // NEW_ACTOR_MOVEMENT
	if(max_queue >= (MAX_CMD_QUEUE/2))my_timer_adjust+=6+(max_queue-(MAX_CMD_QUEUE/2));	//speed up the timer clock if we are building up too much
}


void destroy_actor(int actor_id)
{
	int i;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	for(i=0;i<max_actors;i++){
		if(actors_list[i])//The timer thread doesn't free memory
			if(actors_list[i]->actor_id==actor_id){
				LOCK_ACTORS_LISTS();
				if (actor_id == yourself)
					set_our_actor (NULL);
				if(actors_list[i]->calmodel!=NULL)
					model_delete(actors_list[i]->calmodel);
				if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
				if(actors_list[i]->is_enhanced_model){
					glDeleteTextures(1,&actors_list[i]->texture_id);
					if(actors_list[i]->body_parts)free(actors_list[i]->body_parts);
				}
#ifdef NEW_SOUND
				stop_sound(actors_list[i]->cur_anim_sound_cookie);
				actors_list[i]->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
				ec_actor_delete(actors_list[i]);
				free(actors_list[i]);
				actors_list[i]=NULL;
				if(i==max_actors-1)max_actors--;
				else {
					//copy the last one down and fill in the hole
					max_actors--;
					actors_list[i]=actors_list[max_actors];
					actors_list[max_actors]=NULL;
				}

				actor_under_mouse = NULL;
				UNLOCK_ACTORS_LISTS();
				break;
			}
	}
	minimap_touch();
}

void destroy_all_actors()
{
	int i=0;
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	set_our_actor (NULL);
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]){
			if(actors_list[i]->calmodel!=NULL)
				model_delete(actors_list[i]->calmodel);
			if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
			if(actors_list[i]->is_enhanced_model){
				glDeleteTextures(1,&actors_list[i]->texture_id);
				if(actors_list[i]->body_parts)free(actors_list[i]->body_parts);
			}
#ifdef NEW_SOUND
			stop_sound(actors_list[i]->cur_anim_sound_cookie);
			actors_list[i]->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
			ec_actor_delete(actors_list[i]);
			free(actors_list[i]);
			actors_list[i]=NULL;
		}
	}
	max_actors= 0;
	actor_under_mouse = NULL;
	my_timer_adjust= 0;
	UNLOCK_ACTORS_LISTS();	//unlock it since we are done
	minimap_touch();
}




void update_all_actors()
{
 	Uint8 str[40];

	//we got a nasty error, log it
	LOG_TO_CONSOLE(c_red2,resync_server);

	destroy_all_actors();
	str[0]=SEND_ME_MY_ACTORS;
	my_tcp_send(my_socket,str,1);
}

void add_command_to_actor(int actor_id, unsigned char command)
{
	//int i=0;
	int k=0;
	//int have_actor=0;
//if ((actor_id==yourself)&&(command==enter_combat)) LOG_TO_CONSOLE(c_green2,"FIGHT!");
	actor * act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act= get_actor_ptr_from_id(actor_id);

	if(!act){
		//Resync
		//if we got here, it means we don't have this actor, so get it from the server...
		LOG_ERROR("%s %d - %d\n", cant_add_command, command, actor_id);
	} else {
		LOCK_ACTORS_LISTS();

#ifdef MISSILES
		if (command == missile_miss) {
			missiles_log_message("%s (%d): will miss his target", act->actor_name, actor_id);
			if (act->shots_count < MAX_SHOTS_QUEUE)
				act->shot_type[act->shots_count] = MISSED_SHOT;
			UNLOCK_ACTORS_LISTS();
			return;
		}
		else if (command == missile_critical) {
			missiles_log_message("%s (%d): will do a critical hit", act->actor_name, actor_id);
			if (act->shots_count < MAX_SHOTS_QUEUE)
				act->shot_type[act->shots_count] = CRITICAL_SHOT;
			UNLOCK_ACTORS_LISTS();
			return;
		}
		else if (command == aim_mode_reload) {
			missiles_log_message("%s (%d): reload after next fire", act->actor_name, actor_id);
			if (act->shots_count < MAX_SHOTS_QUEUE)
				act->reload[act->shots_count] = 1;
			UNLOCK_ACTORS_LISTS();
			return;
		}
#endif // MISSILES

		if(command==leave_combat||command==enter_combat||command==die1||command==die2)
		{
			int j= 0;

			//Strip the queue for attack messages
			for(k=0; k<MAX_CMD_QUEUE; k++){
				switch(act->que[k]){
					case pain1:
					case pain2:
					case attack_up_1:
					case attack_up_2:
					case attack_up_3:
					case attack_up_4:
					case attack_down_1:
					case attack_down_2:
						act->que[k]= nothing;
						break;

					default:
						act->que[j]= act->que[k];
						j++;
						if(j<=k){
							act->que[k]= nothing;
						}
						break;
				}
			}

			if(act->last_command == nothing)
			{
				//We may be on idle, update the actor so we can reduce the rendering lag
				CalModel_Update(act->calmodel, 5.0f);
				build_actor_bounding_box(act);
#ifdef MISSILES
				missiles_rotate_actor_bones(get_actor_ptr_from_id(actor_id));
#endif // MISSILES
				if (use_animation_program)
				{
					set_transformation_buffers(act);
				}
			}
		}

		for(k=0;k<MAX_CMD_QUEUE;k++){
			if(act->que[k]==nothing){
				//if we are SEVERLY behind, just update all the actors in range
				if(k>MAX_CMD_QUEUE-2) break;
				else if(k>MAX_CMD_QUEUE-8){
					// is the front a sit/stand spam?
					if((act->que[0]==stand_up||act->que[0]==sit_down)
  					 &&(act->que[1]==stand_up||act->que[1]==sit_down)){
						int j;
						//move que down with one command
						for(j=0;j<=k;j++){
							act->que[j]=act->que[j+1];
						}
						act->que[j]=nothing;
						//backup one entry
						k--;
					}

					// is the end a sit/stand spam?
					else if((command==stand_up||command==sit_down)
					     && (act->que[k-1]==stand_up||act->que[k-1]==sit_down)) {
						act->que[k-1]=command;
						break;
					}

				}

				act->que[k]=command;
				break;
			}
		}

		switch(command) {
		case enter_combat:
			act->async_fighting= 1;
			break;
		case leave_combat:
			act->async_fighting= 0;
			break;
		case move_n:
		case run_n:
			act->async_y_tile_pos++;
			act->async_z_rot= 0;
			break;
		case move_ne:
		case run_ne:
			act->async_x_tile_pos++;
			act->async_y_tile_pos++;
			act->async_z_rot= 45;
			break;
		case move_e:
		case run_e:
			act->async_x_tile_pos++;
			act->async_z_rot= 90;
			break;
		case move_se:
		case run_se:
			act->async_x_tile_pos++;
			act->async_y_tile_pos--;
			act->async_z_rot= 135;
			break;
		case move_s:
		case run_s:
			act->async_y_tile_pos--;
			act->async_z_rot= 180;
			break;
		case move_sw:
		case run_sw:
			act->async_x_tile_pos--;
			act->async_y_tile_pos--;
			act->async_z_rot= 225;
			break;
		case move_w:
		case run_w:
			act->async_x_tile_pos--;
			act->async_z_rot= 270;
			break;
		case move_nw:
		case run_nw:
			act->async_x_tile_pos--;
			act->async_y_tile_pos++;
			act->async_z_rot= 315;
			break;
		case turn_n:
		case turn_ne:
		case turn_e:
		case turn_se:
		case turn_s:
		case turn_sw:
		case turn_w:
		case turn_nw:
			 act->async_z_rot= (command-turn_n)*45;
			 break;
		}

		UNLOCK_ACTORS_LISTS();

		if(k>MAX_CMD_QUEUE-2){
			int i;
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => resync!",
					  act->actor_id, act->actor_name);
			for (i = 0; i < MAX_CMD_QUEUE; ++i)
				LOG_ERROR("%dth command in the queue: %d", i, (int)act->que[i]);
			update_all_actors();
		}
	}
}

void get_actor_damage(int actor_id, int damage)
{
	//int i=0;
	actor * act;
	float blood_level;
        float bone_list[1024][3];
        int total_bones;
        int bone;
        float bone_x, bone_y, bone_z;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
		if(floatingmessages_enabled){
			act->last_health_loss=cur_time;
		}

		act->damage=damage;
		act->damage_ms=2000;
		act->cur_health-=damage;

		if (act->cur_health <= 0) {
#ifdef NEW_SOUND
			add_death_sound(act);
#endif // NEW_SOUND
			increment_death_counter(act);
		}
		act->last_range_attacker_id = -1;

		if (use_eye_candy && enable_blood)
		{
			if (strcmp(act->actor_name, "Gargoyle") && strcmp(act->actor_name, "Skeleton") && strcmp(act->actor_name, "Phantom Warrior"))	//Ideally, we'd also check to see if it was a player or not, but since this is just cosmetic...
			{
				blood_level=(int)powf(damage / powf(act->max_health, 0.5), 0.75) + 0.5;
				total_bones = CalSkeleton_GetBonePoints(CalModel_GetSkeleton(act->calmodel), &bone_list[0][0]);
				bone = rand() % total_bones;
				bone_x = bone_list[bone][0] + act->x_pos + 0.25;
				bone_y = bone_list[bone][1] + act->y_pos + 0.25;
				bone_z = bone_list[bone][2] + ec_get_z(act);
//				printf("ec_create_impact_blood((%f %f, %f), (%f, %f, %f), %d, %f);", bone_x, bone_y, bone_z, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, (poor_man ? 6 : 10), blood_level);
				ec_create_impact_blood(bone_x, bone_y, bone_z, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, (poor_man ? 6 : 10), blood_level);
			}
		}
	}
}

void get_actor_heal(int actor_id, int quantity)
{
	//int i=0;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
		if(floatingmessages_enabled){
			act->damage=-quantity;
			act->damage_ms=2000;
			act->last_health_loss=cur_time;
		}

		act->cur_health+=quantity;
	}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void get_actor_health(int actor_id, int quantity)
{
	//int i=0;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
//		if(floatingmessages_enabled){
			//act->damage=-quantity;
			//act->damage_ms=2000;
			//act->last_health_loss=cur_time;
//		}

		act->max_health=quantity;
	}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void update_actor_buffs(int actor_id, Uint32 in_buffs)
{
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
		act->buffs = in_buffs;
	}
}

void move_self_forward()
{
	int x,y,rot,tx,ty;

	actor *me = get_our_actor ();

	if(!me)return;//Wtf!?

#ifndef NEW_ACTOR_MOVEMENT
	x=me->tmp.x_tile_pos;
	y=me->tmp.y_tile_pos;
#else // NEW_ACTOR_MOVEMENT
	x=me->x_tile_pos;
	y=me->y_tile_pos;
#endif // NEW_ACTOR_MOVEMENT
	rot=(int)rint(me->z_rot/45.0f);
	if (rot < 0) rot += 8;
	switch(rot) {
		case 8: //360
		case 0: //0
			tx=x;
			ty=y+1;
			break;
		case 1: //45
			tx=x+1;
			ty=y+1;
			break;
		case 2: //90
			tx=x+1;
			ty=y;
			break;
		case 3: //135
			tx=x+1;
			ty=y-1;
			break;
		case 4: //180
			tx=x;
			ty=y-1;
			break;
		case 5: //225
			tx=x-1;
			ty=y-1;
			break;
		case 6: //270
			tx=x-1;
			ty=y;
			break;
		case 7: //315
			tx=x-1;
			ty=y+1;
			break;
		default:
			tx=x;
			ty=y;
	}

	//check to see if the coordinates are OUTSIDE the map
	if(ty<0 || tx<0 || tx>=tile_map_size_x*6 || ty>=tile_map_size_y*6) {
		return;
	}
	if (pf_follow_path) {
		pf_destroy_path();
	}

	move_to (tx, ty, 0);
}


void actor_check_string(actor_types *act, const char *section, const char *type, const char *value)
{
	if (value == NULL || *value=='\0')
	{
#ifdef DEBUG
		LOG_ERROR("Data Error in %s(%d): Missing %s.%s", act->actor_name, act->actor_type, section, type);
#endif // DEBUG
	}
}

void actor_check_int(actor_types *act, const char *section, const char *type, int value)
{
	if (value < 0)
	{
#ifdef DEBUG
		LOG_ERROR("Data Error in %s(%d): Missing %s.%s", act->actor_name, act->actor_type, section, type);
#endif // DEBUG
	}
}

xmlNode *get_default_node(xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	char *group;
	
	// first, check for errors
	if(defaults == NULL || cfg == NULL){
        return NULL;
	}
	
	//lets find out what group to look for
	group = get_string_property(cfg, "group");
	
	// look for defaul entries with the same name
	for(item=defaults->children; item; item=item->next){
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, cfg->name) == 0){
				char *item_group;
			
				item_group = get_string_property(item, "group");
				// either both have no group, or both groups match
				if(xmlStrcasecmp((xmlChar*)item_group, (xmlChar*)group) == 0){
					// this is the default entry we want then!
					return item;
				}
			}
		}
	}
	
	// if we got here, there is no default node that matches
	return NULL;
}

int parse_actor_shirt (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	int ok, col_idx;
	shirt_part *shirt;

	if(cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "shirt color", shirt_color_dict);
	}
	if(col_idx < 0 || col_idx >= ACTOR_SHIRT_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->shirt == NULL) {
		int i;
		act->shirt = (shirt_part*)calloc(ACTOR_SHIRT_SIZE, sizeof(shirt_part));
		for (i = ACTOR_SHIRT_SIZE; i--;) act->shirt[i].mesh_index= -1;
	}

	shirt= &(act->shirt[col_idx]);
	ok= 1;
	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp (item->name, (xmlChar*)"arms") == 0) {
				get_string_value(shirt->arms_name, sizeof(shirt->arms_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value(shirt->model_name, sizeof(shirt->model_name), item);
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"torso") == 0) {
				get_string_value(shirt->torso_name, sizeof(shirt->torso_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"armsmask") == 0) {
				get_string_value(shirt->arms_mask, sizeof(shirt->arms_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"torsomask") == 0) {
				get_string_value(shirt->torso_mask, sizeof(shirt->torso_mask), item);
			} else {
				LOG_ERROR("unknown shirt property \"%s\"", item->name);
				ok= 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(shirt->arms_name==NULL || *shirt->arms_name=='\0')
				get_item_string_value(shirt->arms_name, sizeof(shirt->arms_name), default_node, (xmlChar*)"arms");
			if(shirt->model_name==NULL || *shirt->model_name=='\0'){
				get_item_string_value(shirt->model_name, sizeof(shirt->model_name), default_node, (xmlChar*)"mesh");
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			}
			if(shirt->torso_name==NULL || *shirt->torso_name=='\0')
				get_item_string_value(shirt->torso_name, sizeof(shirt->torso_name), default_node, (xmlChar*)"torso");
		}
	}
	
	// check the critical information
	actor_check_string(act, "shirt", "arms", shirt->arms_name);
	actor_check_string(act, "shirt", "model", shirt->model_name);
	actor_check_int(act, "shirt", "mesh", shirt->mesh_index);
	actor_check_string(act, "shirt", "torso", shirt->torso_name);

#if NEW_LIGHTING
	set_shirt_metadata(shirt);
#endif
	return ok;
}

int parse_actor_skin (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	int ok, col_idx;
	skin_part *skin;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "skin color", skin_color_dict);
	}
	if(col_idx < 0 || col_idx >= ACTOR_SKIN_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->skin == NULL) {
		int i;
		act->skin = (skin_part*)calloc(ACTOR_SKIN_SIZE, sizeof(skin_part));
		for (i = ACTOR_SKIN_SIZE; i--;) act->skin[i].mesh_index= -1;
	}

	skin = &(act->skin[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"hands") == 0) {
				get_string_value (skin->hands_name, sizeof (skin->hands_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"head") == 0) {
				get_string_value (skin->head_name, sizeof (skin->head_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"torso") == 0) {
				get_string_value (skin->body_name, sizeof (skin->body_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"arms") == 0) {
				get_string_value (skin->arms_name, sizeof (skin->arms_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"legs") == 0) {
				get_string_value (skin->legs_name, sizeof (skin->legs_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"feet") == 0) {
				get_string_value (skin->feet_name, sizeof (skin->feet_name), item);
			} else {
				LOG_ERROR("unknown skin property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(skin->hands_name==NULL || *skin->hands_name=='\0')
				get_item_string_value(skin->hands_name, sizeof(skin->hands_name), default_node, (xmlChar*)"hands");
			if(skin->head_name==NULL || *skin->head_name=='\0')
				get_item_string_value(skin->head_name, sizeof(skin->head_name), default_node, (xmlChar*)"head");
		}
	}
	
	// check the critical information
	actor_check_string(act, "skin", "hands", skin->hands_name);
	actor_check_string(act, "skin", "head", skin->head_name);

#if NEW_LIGHTING
	set_skin_metadata(skin);
#endif

	return ok;
}

int parse_actor_legs (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	int ok, col_idx;
	legs_part *legs;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "legs color", legs_color_dict);
	}
	if(col_idx < 0 || col_idx >= ACTOR_LEGS_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->legs == NULL) {
		int i;
		act->legs = (legs_part*)calloc(ACTOR_LEGS_SIZE, sizeof(legs_part));
		for (i = ACTOR_LEGS_SIZE; i--;) act->legs[i].mesh_index= -1;
	}

	legs = &(act->legs[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (legs->legs_name, sizeof (legs->legs_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (legs->model_name, sizeof (legs->model_name), item);
				legs->mesh_index = cal_load_mesh (act, legs->model_name, "legs");
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"legsmask") == 0) {
				get_string_value (legs->legs_mask, sizeof (legs->legs_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				legs->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(legs->legs_name==NULL || *legs->legs_name=='\0')
				get_item_string_value(legs->legs_name, sizeof(legs->legs_name), default_node, (xmlChar*)"skin");
			if(legs->model_name==NULL || *legs->model_name=='\0'){
				get_item_string_value(legs->model_name, sizeof(legs->model_name), default_node, (xmlChar*)"mesh");
				legs->mesh_index= cal_load_mesh(act, legs->model_name, "legs");
			}
		}
	}

	// check the critical information
	actor_check_string(act, "legs", "skin", legs->legs_name);
	actor_check_string(act, "legs", "model", legs->model_name);
	actor_check_int(act, "legs", "mesh", legs->mesh_index);

#if NEW_LIGHTING
	set_legs_metadata(legs);
#endif

	return ok;
}

int parse_actor_weapon_detail (actor_types *act, weapon_part *weapon, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	char str[255];
	int ok;

	if (cfg == NULL || cfg->children == NULL) return 0;

	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (weapon->model_name, sizeof (weapon->model_name), item);
				weapon->mesh_index = cal_load_weapon_mesh (act, weapon->model_name, "weapon");
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (weapon->skin_name, sizeof (weapon->skin_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (weapon->skin_mask, sizeof (weapon->skin_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up1") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_up_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up2") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_up_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down1") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_down_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down2") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_down_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
#ifdef MISSILES
			else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_fire") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_range_fire_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
			else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_fire_out") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_range_fire_out_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
			else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_range_idle_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
			else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_in") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_range_in_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
			else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_out") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_range_out_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			}
#endif // MISSILES
			else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				weapon->glow = mode;
#ifdef NEW_SOUND
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up1") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_attack_up_1_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up2") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_attack_up_2_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down1") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_attack_down_1_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down2") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_attack_down_2_frame, str, get_string_property(item, "sound_scale"));
#endif	//NEW_SOUND
			} else {
				LOG_ERROR("unknown weapon property \"%s\"", item->name);
				ok = 0;
			}
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_weapon_detail (act, weapon, item->children, defaults);
		}
	}


	return ok;
}

int parse_actor_weapon (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	int ok, type_idx;
	weapon_part *weapon;

	if (cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "weapon type", weapon_type_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_WEAPON_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->weapon == NULL) {
		int i;
		act->weapon = (weapon_part*)calloc(ACTOR_WEAPON_SIZE, sizeof(weapon_part));
		for (i = ACTOR_WEAPON_SIZE; i--;) {
#ifdef MISSILES
			act->weapon[i].cal_range_in_frame.anim_index=-1;
			act->weapon[i].cal_range_out_frame.anim_index=-1;
			act->weapon[i].cal_range_idle_frame.anim_index=-1;
			act->weapon[i].cal_range_fire_frame.anim_index=-1;
			act->weapon[i].cal_range_fire_out_frame.anim_index=-1;
#endif // MISSILES
			act->weapon[i].cal_attack_up_1_frame.anim_index=-1;
			act->weapon[i].cal_attack_up_2_frame.anim_index=-1;
			act->weapon[i].cal_attack_down_1_frame.anim_index=-1;
			act->weapon[i].cal_attack_down_2_frame.anim_index=-1;
			act->weapon[i].mesh_index = -1;
		}
	}

	weapon = &(act->weapon[type_idx]);
	ok= parse_actor_weapon_detail(act, weapon, cfg, defaults);

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(weapon->skin_name==NULL || *weapon->skin_name=='\0')
				get_item_string_value(weapon->skin_name, sizeof(weapon->skin_name), default_node, (xmlChar*)"skin");
			if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
				if(weapon->model_name==NULL || *weapon->model_name=='\0'){
					get_item_string_value(weapon->model_name, sizeof(weapon->model_name), default_node, (xmlChar*)"mesh");
					weapon->mesh_index= cal_load_weapon_mesh(act, weapon->model_name, "weapon");
				}
			}
			// TODO: combat animations
		}
	}

	// check the critical information
	if(type_idx!=WEAPON_NONE){   // no weapon doesn't have a skin/model
		actor_check_string(act, "weapon", "skin", weapon->skin_name);
		if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
			actor_check_string(act, "weapon", "model", weapon->model_name);
			actor_check_int(act, "weapon.mesh", weapon->model_name, weapon->mesh_index);
		}
		// TODO: check combat animations
	}

#if NEW_LIGHTING
	set_weapon_metadata(weapon);
#endif

	return ok;
}

int parse_actor_body_part (actor_types *act, body_part *part, xmlNode *cfg, const char *part_name, xmlNode *default_node)
{
	xmlNode *item;
	int ok = 1;

	if(cfg == NULL) return 0;

	for(item=cfg; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
				if(strcmp("shield",part_name)==0)
					part->mesh_index = cal_load_weapon_mesh (act, part->model_name, part_name);
				else
					part->mesh_index = cal_load_mesh (act, part->model_name, part_name);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (part->skin_mask, sizeof (part->skin_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if(mode < 0) mode = GLOW_NONE;
				part->glow= mode;
			} else {
				LOG_ERROR("unknown %s property \"%s\"", part_name, item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(default_node){
		if(part->skin_name==NULL || *part->skin_name=='\0')
			if(strcmp(part_name, "head")){ // heads don't have separate skins here
				get_item_string_value(part->skin_name, sizeof(part->skin_name), default_node, (xmlChar*)"skin");
			}
		if(part->model_name==NULL || *part->model_name=='\0'){
			get_item_string_value(part->model_name, sizeof(part->model_name), default_node, (xmlChar*)"mesh");
			if(strcmp("shield",part_name)==0)
				part->mesh_index= cal_load_weapon_mesh(act, part->model_name, part_name);
			else
				part->mesh_index= cal_load_mesh(act, part->model_name, part_name);
		}
	}

	// check the critical information
	if(strcmp(part_name, "head")){ // heads don't have separate skins here
		actor_check_string(act, part_name, "skin", part->skin_name);
	}
	actor_check_string(act, part_name, "model", part->model_name);
	actor_check_int(act, part_name, "mesh", part->mesh_index);

#if NEW_LIGHTING
	set_part_metadata(part);
#endif

	return ok;
}

int parse_actor_helmet (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *helmet;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "helmet type", helmet_type_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_HELMET_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->helmet == NULL) {
		int i;
		act->helmet = (body_part*)calloc(ACTOR_HELMET_SIZE, sizeof(body_part));
		for (i = ACTOR_HELMET_SIZE; i--;) act->helmet[i].mesh_index= -1;
	}

	helmet= &(act->helmet[type_idx]);

#if NEW_LIGHTING
	set_part_metadata(helmet);
#endif

	return parse_actor_body_part(act,helmet, cfg->children, "helmet", default_node);
}

#ifdef NEW_SOUND
int parse_actor_sounds (actor_types *act, xmlNode *cfg)
{
	xmlNode *item;
	char str[255];
	int ok;
	int i;

	if (cfg == NULL) return 0;
	if (!have_sound_config) return 0;

	ok = 1;
	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			get_string_value (str,sizeof(str),item);
			if (xmlStrcasecmp (item->name, (xmlChar*)"walk") == 0) {
				cal_set_anim_sound(&act->cal_walk_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"die1") == 0) {
				cal_set_anim_sound(&act->cal_die1_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"die2") == 0) {
				cal_set_anim_sound(&act->cal_die2_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pain1") == 0) {
				cal_set_anim_sound(&act->cal_pain1_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pain2") == 0) {
				cal_set_anim_sound(&act->cal_pain2_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pick") == 0) {
				cal_set_anim_sound(&act->cal_pick_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"drop") == 0) {
				cal_set_anim_sound(&act->cal_drop_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"harvest") == 0) {
				cal_set_anim_sound(&act->cal_harvest_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"attack_cast") == 0) {
				cal_set_anim_sound(&act->cal_attack_cast_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"attack_ranged") == 0) {
				cal_set_anim_sound(&act->cal_attack_ranged_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"sit_down") == 0) {
				cal_set_anim_sound(&act->cal_sit_down_frame, str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"stand_up") == 0) {
				cal_set_anim_sound(&act->cal_stand_up_frame, str, get_string_property(item, "sound_scale"));
			// These sounds are only found in the <sounds> block as they aren't tied to an animation
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"battlecry") == 0) {
				i = get_index_for_sound_type_name(str);
				if (i == -1)
					LOG_ERROR("Unknown battlecry sound (%s) in actor def: %s", str, act->actor_name);
				else
				{
					act->battlecry.sound = i;
					safe_strncpy(str, get_string_property(item, "sound_scale"), sizeof(str));
					if (strcasecmp(str, ""))
						act->battlecry.scale = atof(str);
					else
						act->battlecry.scale = 1.0f;
				}
			} else {
				LOG_ERROR("Unknown sound \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}
#endif	//NEW_SOUND

int parse_actor_cape (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *cape;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "color", "cape color", cape_color_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_CAPE_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->cape == NULL) {
		int i;
		act->cape = (body_part*)calloc(ACTOR_CAPE_SIZE, sizeof(body_part));
		for (i = ACTOR_CAPE_SIZE; i--;) act->cape[i].mesh_index= -1;
	}

	cape= &(act->cape[type_idx]);

#if NEW_LIGHTING
	set_part_metadata(cape);
#endif

	return parse_actor_body_part(act,cape, cfg->children, "cape", default_node);
}

int parse_actor_head (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *head;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "number", "head number", head_number_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_HEAD_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->head == NULL) {
		int i;
		act->head = (body_part*)calloc(ACTOR_HEAD_SIZE, sizeof(body_part));
		for (i = ACTOR_HEAD_SIZE; i--;) act->head[i].mesh_index= -1;
	}

	head= &(act->head[type_idx]);


#if NEW_LIGHTING
	set_part_metadata(head);
#endif

	return parse_actor_body_part(act, head, cfg->children, "head", default_node);
}

int parse_actor_shield_part (actor_types *act, shield_part *part, xmlNode *cfg, xmlNode *default_node)
{
	xmlNode *item;
	int ok = 1;

	if(cfg == NULL) return 0;

	for(item=cfg; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
				part->mesh_index = cal_load_weapon_mesh (act, part->model_name, "shield");
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (part->skin_mask, sizeof (part->skin_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if(mode < 0) mode = GLOW_NONE;
				part->glow= mode;
#ifdef MISSILES
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"missile") == 0) {
				part->missile_type = get_int_value(item);
#endif // MISSILES
			} else {
				LOG_ERROR("unknown shield property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(default_node){
		if(part->model_name==NULL || *part->model_name=='\0'){
			get_item_string_value(part->model_name, sizeof(part->model_name), default_node, (xmlChar*)"mesh");
			part->mesh_index= cal_load_weapon_mesh(act, part->model_name, "shield");
		}
	}

	// check the critical information
	actor_check_string(act, "shield", "model", part->model_name);
	actor_check_int(act, "shield", "mesh", part->mesh_index);

#if NEW_LIGHTING
	set_part_metadata(part);
#endif

	return ok;
}

int parse_actor_shield (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	shield_part *shield;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "shield type", shield_type_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_SHIELD_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->shield == NULL) {
		int i;
		act->shield = (shield_part*)calloc(ACTOR_SHIELD_SIZE, sizeof(shield_part));
		for (i = ACTOR_SHIELD_SIZE; i--;) {
			act->shield[i].mesh_index = -1;
#ifdef MISSILES
			act->shield[i].missile_type = -1;
#endif // MISSILES
		}
	}

	shield= &(act->shield[type_idx]);

#if NEW_LIGHTING
	set_part_metadata(shield);
#endif

	return parse_actor_shield_part(act, shield, cfg->children, default_node);
}

int parse_actor_hair (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	int col_idx;
	size_t len;
	char *buf;

	if(cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "hair color", hair_color_dict);
	}
	if(col_idx < 0 || col_idx >= ACTOR_HAIR_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->hair == NULL) {
		int i;
		act->hair = (hair_part*)calloc(ACTOR_HAIR_SIZE, sizeof(hair_part));
		for (i = ACTOR_HAIR_SIZE; i--;) act->hair[i].mesh_index= -1;
	}

	buf= act->hair[col_idx].hair_name;
	len= sizeof (act->hair[col_idx].hair_name);
	get_string_value(buf, len, cfg);

#if NEW_LIGHTING
	set_hair_metadata(act->hair);
#endif

	return 1;
}

int cal_get_idle_group(actor_types *act,char *name)
{
	int i;
	int res=-1;

	for (i=0;i<act->group_count;++i) {
		if (strcmp(name,act->idle_group[i].name)==0) res=i;
	}

	if (res>=0) return res;//Found it, return

	//Create a new named group
	res=act->group_count;
	safe_strncpy(act->idle_group[res].name, name, sizeof(act->idle_group[res].name));
	++act->group_count;

	return res;
}

struct cal_anim cal_load_idle(actor_types *act, char *str)
{
	struct cal_anim res={-1,0,0
#ifdef  NEW_ACTOR_ANIMATION
	,0.0f
#endif
#ifdef NEW_SOUND
	,-1
	,0.0f
#endif  //NEW_SOUND
	};
	struct CalCoreAnimation *coreanim;

	res.anim_index=CalCoreModel_ELLoadCoreAnimation(act->coremodel,str);
	if(res.anim_index == -1) {
		log_error("Cal3d error: %s: %s\n", str, CalError_GetLastErrorDescription());
		return res;
	}
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);

	if (coreanim) {
		CalCoreAnimation_Scale(coreanim,act->scale);
		res.duration=CalCoreAnimation_GetDuration(coreanim);
	} else {
		log_error("No Anim: %s\n",str);
	}

	return res;
}

void cal_group_addanim(actor_types *act,int gindex, char *fanim)
{
	int i;

	i=act->idle_group[gindex].count;
	act->idle_group[gindex].anim[i]=cal_load_idle(act,fanim);
	//LOG_TO_CONSOLE(c_green2,fanim);
	++act->idle_group[gindex].count;
}

void parse_idle_group (actor_types *act, const char *str)
{
	char gname[255]={0};
	char fname[255]={0};
	//char temp[255];
	int gindex;

	if (sscanf (str, "%254s %254s", gname, fname) != 2) return;

	gindex=cal_get_idle_group(act,gname);
	cal_group_addanim(act,gindex,fname);
	//safe_snprintf(temp, sizeof(temp), "%d",gindex);
	//LOG_TO_CONSOLE(c_green2,gname);
	//LOG_TO_CONSOLE(c_green2,fname);
	//LOG_TO_CONSOLE(c_green2,temp);
}

int parse_actor_frames (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	char str[255];
	//char fname[255];
	//char temp[255];
	//int i;

	int ok = 1;
	if (cfg == NULL) return 0;

	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {

			if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_IDLE_GROUP") == 0) {
				get_string_value (str,sizeof(str),item);
     				//act->cal_walk_frame=cal_load_anim(act,str);
				//LOG_TO_CONSOLE(c_green2,str);
				parse_idle_group(act,str);
				//Not functional!
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_walk") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_walk_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_run") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_run_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_die1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_die1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_die2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_die2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pain1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pain1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pain2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pain2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pick") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pick_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_drop") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_drop_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle_sit") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle_sit_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
 			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_harvest") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_harvest_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_cast") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_cast_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_sit_down") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_sit_down_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_stand_up") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_stand_up_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_in_combat") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_in_combat_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_out_combat") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_out_combat_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_combat_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_combat_idle_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_3") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_3_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_4") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_4_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_down_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_down_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else {
				LOG_ERROR("unknown frame property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}

int parse_actor_boots (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	int ok, col_idx;
	boots_part *boots;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx = get_property (cfg, "color", "boots color", boots_color_dict);
	}
	if(col_idx < 0 || col_idx >= ACTOR_BOOTS_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->boots == NULL) {
		int i;
		act->boots = (boots_part*)calloc(ACTOR_BOOTS_SIZE, sizeof(boots_part));
		for (i = ACTOR_BOOTS_SIZE; i--;) act->boots[i].mesh_index= -1;
	}

	boots = &(act->boots[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (boots->boots_name, sizeof (boots->boots_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"bootsmask") == 0) {
				get_string_value (boots->boots_mask, sizeof (boots->boots_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				boots->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(boots->boots_name==NULL || *boots->boots_name=='\0')
				get_item_string_value(boots->boots_name, sizeof(boots->boots_name), default_node, (xmlChar*)"skin");
		}
	}
	
	// check the critical information
	actor_check_string(act, "boots", "boots", boots->boots_name);

	return ok;
}

//Searches if a mesh is already loaded- TODO:MAKE THIS BETTER
int cal_search_mesh (actor_types *act, const char *fn, const char *kind)
{
	int i;

	if (kind == NULL)
	{
		return -1;
	}
	else if (act->head && strcmp (kind, "head") == 0)
	{
		for (i = 0; i < ACTOR_HEAD_SIZE; i++)
			if (strcmp (fn, act->head[i].model_name) == 0 && act->head[i].mesh_index != -1)
				return act->head[i].mesh_index;
	}
	else if (act->shirt && strcmp (kind, "shirt") == 0)
	{
		for (i = 0; i < ACTOR_SHIRT_SIZE; i++)
		{
			if (strcmp (fn, act->shirt[i].model_name) == 0 && act->shirt[i].mesh_index != -1)
				return act->shirt[i].mesh_index;
		}
	}
	else if (act->legs && strcmp (kind, "legs") == 0)
	{
		for (i = 0; i < ACTOR_LEGS_SIZE; i++)
		{
			if (strcmp (fn, act->legs[i].model_name) == 0 && act->legs[i].mesh_index != -1)
				return act->legs[i].mesh_index;
		}
	}
	else if (act->cape && strcmp (kind, "cape") == 0)
	{
		for (i = 0; i < ACTOR_CAPE_SIZE; i++)
		{
			if (strcmp (fn, act->cape[i].model_name) == 0 && act->cape[i].mesh_index != -1)
				return act->cape[i].mesh_index;
		}
	}
	else if (act->helmet && strcmp (kind, "helmet") == 0)
	{
		for (i = 0; i < ACTOR_HELMET_SIZE; i++)
		{
			if (strcmp (fn, act->helmet[i].model_name) == 0 && act->helmet[i].mesh_index != -1)
				return act->helmet[i].mesh_index;
		}
	}
	else if (act->shield && strcmp (kind, "shield") == 0)
	{
		for (i = 0; i < ACTOR_SHIELD_SIZE; i++)
		{
			if (strcmp (fn, act->shield[i].model_name) == 0 && act->shield[i].mesh_index != -1)
				return act->shield[i].mesh_index;
		}
	}
	else if (act->weapon && strcmp (kind, "weapon") == 0)
	{
		for (i = 0; i < ACTOR_WEAPON_SIZE; i++)
		{
			if (strcmp (fn, act->weapon[i].model_name) == 0 && act->weapon[i].mesh_index != -1)
				return act->weapon[i].mesh_index;
		}
	}

	return -1;
}

//Loads a Cal3D mesh
int cal_load_mesh (actor_types *act, const char *fn, const char *kind)
{
	//int i;
	//int meshindex=-1;
	//char fname[255];
	//char temp[255];
	int res;
	struct CalCoreMesh *mesh;

	if (fn==0) return -1;
	if (strlen(fn)==0) return -1;
	if (act->coremodel==NULL) return -1;
	if (kind != NULL)
	{
		res = cal_search_mesh (act, fn, kind);
		if (res != -1) return res;
	}

	//Load coremesh
	res=CalCoreModel_ELLoadCoreMesh(act->coremodel,fn);

	//Scale coremesh
	if (res >= 0) {
		mesh=CalCoreModel_GetCoreMesh(act->coremodel,res);
		if ((mesh)&&(act->mesh_scale!=1.0)) CalCoreMesh_Scale(mesh,act->mesh_scale);
	} else {
		log_error("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}
	
	return res;
}

int cal_load_weapon_mesh (actor_types *act, const char *fn, const char *kind)
{
	//int i;
	//int meshindex=-1;
	//char fname[255];
	//char temp[255];
	int res;
	struct CalCoreMesh *mesh;

	if (fn==0) return -1;
	if (strlen(fn)==0) return -1;
	if (act->coremodel==NULL) return -1;

	if (kind != NULL)
	{
		res = cal_search_mesh (act, fn, kind);
		if (res!=-1) return res;
	}

	//Load coremesh
	res=CalCoreModel_ELLoadCoreMesh(act->coremodel,fn);

	//Scale coremesh
	if (res>=0) {
		mesh=CalCoreModel_GetCoreMesh(act->coremodel,res);
		if ((mesh)&&(act->skel_scale!=1.0)) CalCoreMesh_Scale(mesh,act->skel_scale);
	} else {
		log_error("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}

	return res;
}

int	parse_actor_nodes (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode	*item;
	int	ok= 1;
	
	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"ghost") == 0) {
				act->ghost= get_bool_value (item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value(act->skin_name, sizeof (act->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value(act->file_name, sizeof (act->file_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"actor_scale")==0) {
				act->actor_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"scale")==0) {
				act->scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh_scale")==0) {
				act->mesh_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"bone_scale")==0) {
				act->skel_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skeleton")==0) {
				char skeleton_name[MAX_FILE_PATH];
				get_string_value(skeleton_name, sizeof(skeleton_name), item);
				act->coremodel= CalCoreModel_New("Model");
				if(!CalCoreModel_ELLoadCoreSkeleton(act->coremodel, skeleton_name)) {
					log_error("Cal3d error: %s: %s\n", skeleton_name, CalError_GetLastErrorDescription());
					act->skeleton_type = -1;
				}
				else {
					act->skeleton_type = get_skeleton(act->coremodel, skeleton_name);
				}
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"walk_speed") == 0) {
				act->walk_speed= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"run_speed") == 0) {
				act->run_speed= get_float_value(item);

			} else if(xmlStrcasecmp(item->name, (xmlChar*)"defaults") == 0) {
				defaults= item;
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"frames") == 0) {
				ok &= parse_actor_frames(act, item->children, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"shirt") == 0) {
				ok &= parse_actor_shirt(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"hskin") == 0) {
				ok &= parse_actor_skin(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"hair") == 0) {
				ok &= parse_actor_hair(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"boots") == 0) {
				ok &= parse_actor_boots(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"legs") == 0) {
				ok &= parse_actor_legs(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"cape") == 0) {
				ok &= parse_actor_cape(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"head") == 0) {
				ok &= parse_actor_head(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"shield") == 0) {
				ok &= parse_actor_shield(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"weapon") == 0) {
				ok &= parse_actor_weapon(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"helmet") == 0) {
				ok &= parse_actor_helmet(act, item, defaults);
#ifdef NEW_SOUND
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"sounds") == 0) {
				ok &= parse_actor_sounds(act, item->children);
#endif	//NEW_SOUND
			} else {
				LOG_ERROR("Unknown actor attribute \"%s\"", item->name);
				ok= 0;
			}
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_nodes(act, item->children, defaults);
		}
	}
	return ok;
}

int parse_actor_script (xmlNode *cfg)
{
	int ok, act_idx, i;
	actor_types *act;
	struct CalCoreSkeleton *skel;

	if(cfg == NULL || cfg->children == NULL) return 0;

	act_idx= get_int_property(cfg, "id");
	if(act_idx < 0){
		act_idx= get_property(cfg, "type", "actor type", actor_type_dict);
	}
	if(act_idx < 0 || act_idx >= MAX_ACTOR_DEFS){
		char	str[256];
		char    name[256];

		safe_strncpy(name, get_string_property(cfg, "type"), sizeof(name));
		safe_snprintf(str, sizeof(str), "Data Error in %s(%d): Actor ID out of range %d",
			name, act_idx, act_idx
		);
		log_error(str);
		return 0;
	}

	act= &(actors_defs[act_idx]);
	// watch for loading an actor more then once
	if(act->actor_type > 0 || *act->actor_name){
		char	str[256];
		char    name[256];

		safe_strncpy(name, get_string_property(cfg, "type"), sizeof(name));
		safe_snprintf(str, sizeof(str), "Data Error in %s(%d): Already loaded %s(%d)",
			name, act_idx, act->actor_name, act->actor_type
		);
		log_error(str);
	}
	ok= 1;
	act->actor_type= act_idx;	// memorize the ID & name to help in debugging
	safe_strncpy(act->actor_name, get_string_property(cfg, "type"), sizeof(act->actor_name));
	actor_check_string(act, "actor", "name", act->actor_name);

	//Initialize Cal3D settings
	act->coremodel= NULL;
	act->actor_scale= 1.0;
	act->scale= 1.0;
	act->mesh_scale= 1.0;
	act->skel_scale= 1.0;
	act->group_count= 0;
	for (i=0; i<16; ++i) {
		safe_strncpy(act->idle_group[i].name, "", sizeof(act->idle_group[i].name));
		act->idle_group[i].count= 0;
	}

	act->cal_walk_frame.anim_index= -1;
	act->cal_run_frame.anim_index= -1;
	act->cal_die1_frame.anim_index= -1;
	act->cal_die2_frame.anim_index= -1;
	act->cal_pain1_frame.anim_index= -1;
	act->cal_pain2_frame.anim_index= -1;
	act->cal_pick_frame.anim_index= -1;
	act->cal_drop_frame.anim_index= -1;
	act->cal_idle1_frame.anim_index= -1;
	act->cal_idle2_frame.anim_index= -1;
	act->cal_idle_sit_frame.anim_index= -1;
	act->cal_harvest_frame.anim_index= -1;
	act->cal_attack_cast_frame.anim_index= -1;
	act->cal_attack_ranged_frame.anim_index= -1;
	act->cal_sit_down_frame.anim_index= -1;
	act->cal_stand_up_frame.anim_index= -1;
	act->cal_in_combat_frame.anim_index= -1;
	act->cal_out_combat_frame.anim_index= -1;
	act->cal_combat_idle_frame.anim_index= -1;
	act->cal_attack_up_1_frame.anim_index= -1;
	act->cal_attack_up_2_frame.anim_index= -1;
	act->cal_attack_up_3_frame.anim_index= -1;
	act->cal_attack_up_4_frame.anim_index= -1;
	act->cal_attack_down_1_frame.anim_index= -1;
	act->cal_attack_down_2_frame.anim_index= -1;

#ifdef NEW_SOUND
	act->cal_walk_frame.sound= -1;
	act->cal_run_frame.sound= -1;
	act->cal_die1_frame.sound= -1;
	act->cal_die2_frame.sound= -1;
	act->cal_pain1_frame.sound= -1;
	act->cal_pain2_frame.sound= -1;
	act->cal_pick_frame.sound= -1;
	act->cal_drop_frame.sound= -1;
	act->cal_idle1_frame.sound= -1;
	act->cal_idle2_frame.sound= -1;
	act->cal_idle_sit_frame.sound= -1;
	act->cal_harvest_frame.sound= -1;
	act->cal_attack_cast_frame.sound= -1;
	act->cal_attack_ranged_frame.sound= -1;
	act->cal_sit_down_frame.sound= -1;
	act->cal_stand_up_frame.sound= -1;
	act->cal_in_combat_frame.sound= -1;
	act->cal_out_combat_frame.sound= -1;
	act->cal_combat_idle_frame.sound= -1;
	act->cal_attack_up_1_frame.sound= -1;
	act->cal_attack_up_2_frame.sound= -1;
	act->cal_attack_up_3_frame.sound= -1;
	act->cal_attack_up_4_frame.sound= -1;
	act->cal_attack_down_1_frame.sound= -1;
	act->cal_attack_down_2_frame.sound= -1;
	act->battlecry.sound = -1;
	act->battlecry.scale = 1.0f;
#endif // NEW_SOUND

	ok= parse_actor_nodes(act, cfg, NULL);
		
	// TODO: add error checking for missing actor information

	//Actor def parsed, now setup the coremodel
	if (act->coremodel!=NULL)
	{
		skel=CalCoreModel_GetCoreSkeleton(act->coremodel);
		if(skel){
			CalCoreSkeleton_Scale(skel,act->skel_scale);
		}

		// If this not an enhanced actor, load the single mesh and exit
		if(!act->head || strcmp (act->head[0].model_name, "") == 0)
		{
			act->shirt = (shirt_part*)calloc(ACTOR_SHIRT_SIZE, sizeof(shirt_part));
#ifdef NEW_LIGHTING
			strncpy(act->shirt[0].model_name, act->file_name, sizeof(act->shirt[0].model_name));
#endif
			act->shirt[0].mesh_index= cal_load_mesh(act, act->file_name, NULL); //save the single meshindex as torso
#ifdef NEW_LIGHTING
			set_shirt_metadata(&act->shirt[0]);
#endif
		}
		if (use_animation_program)
		{
			build_buffers(act);
		}
	}

	return ok;
}

int parse_actor_defs (xmlNode *node)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp (def->name, (xmlChar*)"actor") == 0) {
				ok &= parse_actor_script (def);
			} else {
				LOG_ERROR("parse error: actor or include expected");
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_defs (def->children);
		}
	}

	return ok;
}

int read_actor_defs (const char *dir, const char *index)
{
	xmlNode *root;
	xmlDoc *doc;
	char fname[120];
	int ok = 1;

	safe_snprintf (fname, sizeof(fname), "%s/%s", dir, index);

	doc = xmlReadFile (fname, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read actor definition file %s", fname);
		return 0;
	}

	root = xmlDocGetRootElement (doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse actor definition file %s", fname);
		ok = 0;
	} else if (xmlStrcasecmp (root->name, (xmlChar*)"actors") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"actors\" expected).", root->name);
		ok = 0;
	} else {
		ok = parse_actor_defs (root);
	}

	xmlFreeDoc (doc);
	return ok;
}

void init_actor_defs()
{

	// initialize the whole thing to zero
	memset (actors_defs, 0, sizeof (actors_defs));

	read_actor_defs ("actor_defs", "actor_defs.xml");
}
