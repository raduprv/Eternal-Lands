#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"
#include <time.h>
#include "actors.h"

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
	  { "hydrogenium greaves", BOOTS_HYDROGENIUM_GREAVE },
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
	  { "hydrogenium cuisses", PANTS_HYDROGENIUM_CUISSES },
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
	  { "titanium"     , SHIELD_TITANIUM         },
	  { "hydrogenium"  , SHIELD_HYDROGENIUM         },
	  { "none"         , SHIELD_NONE          },
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
	  { "titanium"  , HELMET_TITANIUM   },
	  { "hydrogenium"  , HELMET_HYDROGENIUM   },
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
		//sprintf(str,"%d",random_anim);
		//LOG_TO_CONSOLE(c_green2,str);
		actors_list[id]->cur_idle_anims[i].anim_index=random_anim_index;
		//anim.anim_index,1.0,0.05);else
	}

	//if (anim.kind==0) CalMixer_BlendCycle(mixer,anim.anim_index,1.0,0.05);else
	//CalMixer_ExecuteAction(mixer,anim.anim_index,0.0,0.0);
	//actors_list[id]->cur_anim=anim;
	//actors_list[id]->anim_time=0.0;
	CalModel_Update(actors_list[id]->calmodel,0.0001);//Make changes take effect now
	actors_list[id]->IsOnIdle=1;
	actors_list[id]->cur_anim.duration=0;
	actors_list[id]->anim_time=0.0;
	actors_list[id]->cur_anim.anim_index=-1;
#ifdef NEW_SOUND
	stop_sound(actors_list[id]->cur_anim_sound_cookie);
#endif	//NEW_SOUND
	actors_list[id]->cur_anim_sound_cookie = 0;
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

void animate_actors()
{
	int i;
	static int last_update=0;
	char str[255];
	// lock the actors_list so that nothing can interere with this look
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]) {
			if(actors_list[i]->moving) {
				actors_list[i]->movement_frames_left--;
				if(!actors_list[i]->movement_frames_left){//we moved all the way
					Uint8 last_command;
					actors_list[i]->moving=0;//don't move next time, ok?
					actors_list[i]->after_move_frames_left=3;//this is done to prevent going to idle imediatelly
					//now, we need to update the x/y_tile_pos, and round off
					//the x/y_pos according to x/y_tile_pos
					last_command=actors_list[i]->last_command;
					switch(last_command) {
						case move_n:
						case run_n:
							actors_list[i]->y_tile_pos++;break;
						case move_s:
						case run_s:
							actors_list[i]->y_tile_pos--;break;
						case move_e:
						case run_e:
							actors_list[i]->x_tile_pos++;break;
						case move_w:
						case run_w:
							actors_list[i]->x_tile_pos--;break;
						case move_ne:
						case run_ne:
							actors_list[i]->x_tile_pos++;
							actors_list[i]->y_tile_pos++;
							break;
						case move_se:
						case run_se:
							actors_list[i]->x_tile_pos++;
							actors_list[i]->y_tile_pos--;
							break;
						case move_sw:
						case run_sw:
							actors_list[i]->x_tile_pos--;
							actors_list[i]->y_tile_pos--;
							break;
						case move_nw:
						case run_nw:
							actors_list[i]->x_tile_pos--;
							actors_list[i]->y_tile_pos++;
						break;
					}

					//ok, now update the x/y_pos
					actors_list[i]->x_pos=actors_list[i]->x_tile_pos*0.5;
					actors_list[i]->y_pos=actors_list[i]->y_tile_pos*0.5;
#ifdef  MINIMAP
					// and update the minimap if we need to
					if(actors_list[i]->actor_id == yourself){
						update_exploration_map();
					}
#endif  //MINIMAP
				} else {
					actors_list[i]->x_pos+=actors_list[i]->move_x_speed;
					actors_list[i]->y_pos+=actors_list[i]->move_y_speed;
					actors_list[i]->z_pos+=actors_list[i]->move_z_speed;
				}
			} else {//Not moving
				if(actors_list[i]->after_move_frames_left){
					actors_list[i]->after_move_frames_left--;
					if (actors_list[i]->actor_id==yourself)  {
						snprintf(str,sizeof(str),"Left: %d",actors_list[i]->after_move_frames_left);
					}
					if(!actors_list[i]->after_move_frames_left){
						//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Free");
						actors_list[i]->busy=0;
					}
				}
			}

			if(actors_list[i]->rotating) {
				actors_list[i]->rotate_frames_left--;
				if(!actors_list[i]->rotate_frames_left)//we rotated all the way
					actors_list[i]->rotating=0;//don't rotate next time, ok?
				actors_list[i]->x_rot+=actors_list[i]->rotate_x_speed;
				actors_list[i]->y_rot+=actors_list[i]->rotate_y_speed;
				actors_list[i]->z_rot+=actors_list[i]->rotate_z_speed;
				if(actors_list[i]->z_rot >= 360) {
					actors_list[i]->z_rot -= 360;
				} else if (actors_list[i]->z_rot <= 0) {
					actors_list[i]->z_rot += 360;
				}
			}

			if (actors_list[i]->calmodel!=NULL){
#ifdef	NEW_ACTOR_ANIMATION
				actors_list[i]->anim_time=actors_list[i]->anim_time+(((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0);
				CalModel_Update(actors_list[i]->calmodel, (((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0));
#else
				actors_list[i]->anim_time=actors_list[i]->anim_time+(cur_time-last_update)/1000.0;
				CalModel_Update(actors_list[i]->calmodel,((cur_time-last_update)/1000.0));
#endif
			}
		}
	}
	// unlock the actors_list since we are done now
	UNLOCK_ACTORS_LISTS();

	last_update=cur_time;
}




int coun=0;
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
				}
			}

			if ((actors_list[i]->IsOnIdle)&&(actors_list[i]->anim_time>=5.0)&&(actors_list[i]->stop_animation!=1)) {
				cal_actor_set_random_idle(i);
			}

			if (actors_list[i]->cur_anim.anim_index==-1) actors_list[i]->busy=0;
			// XXX (Grum): this is weird. Either the closing brace shouldn't be there,
			// in which case we can forget about the whole if statement, or it should
			// and then we're dereferencing a NULL pointer.
			//} else actors_list[i]->busy=0;

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

//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i;
	int max_queue=0;

	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++){
		if(!actors_list[i])continue;//actor exists?
		if(!actors_list[i]->busy || (actors_list[i]->busy && actors_list[i]->after_move_frames_left && (actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw))){//Are we busy?
			if(actors_list[i]->que[0]==nothing){//Is the queue empty?
				//if que is empty, set on idle
				if(!actors_list[i]->dead) {
					actors_list[i]->stop_animation=0;

					if(actors_list[i]->fighting){
						cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_combat_idle_frame);
					} else if(!actors_list[i]->sitting) {
						if(!actors_list[i]->sit_idle){
							if (actors_defs[actors_list[i]->actor_type].group_count == 0)
							{
								if (actors_defs[actors_list[i]->actor_type].cal_idle2_frame.anim_index != -1 && RAND (0, 1))
									cal_actor_set_anim (i, actors_defs[actors_list[i]->actor_type].cal_idle2_frame); // normal idle
								else
									cal_actor_set_anim (i, actors_defs[actors_list[i]->actor_type].cal_idle1_frame); // normal idle

							}
							else
							{
								cal_actor_set_random_idle(i);
								actors_list[i]->IsOnIdle=1;
							}

							actors_list[i]->sit_idle=1;
						}
					} else	{
						if(!actors_list[i]->stand_idle) {
							cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_idle_sit_frame);
							actors_list[i]->stand_idle=1;
						}
					}
				}

				actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
			} else {
				int actor_type;
				int last_command=actors_list[i]->last_command;
				float z_rot=actors_list[i]->z_rot;
				float targeted_z_rot;
				int k;

				actors_list[i]->sit_idle=0;
				actors_list[i]->stand_idle=0;

				actor_type=actors_list[i]->actor_type;

				switch(actors_list[i]->que[0]) {
					case kill_me:
/*						if(actors_list[i]->remapped_colors)
						glDeleteTextures(1,&actors_list[i]->texture_id);
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

						break;
					case attack_up_2:
						if(actors_list[i]->is_enhanced_model){
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_1_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_2_frame);
						}
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

						break;
					case attack_up_3:
						if(actors_list[i]->is_enhanced_model){
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_3_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

						break;
					case attack_up_4:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_up_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_up_4_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

						break;
					case attack_down_1:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_down_1_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_down_1_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

						break;
					case attack_down_2:
						if(actors_list[i]->is_enhanced_model) {
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_attack_down_2_frame);
						} else {
							cal_actor_set_anim(i,actors_defs[actor_type].cal_attack_down_2_frame);
						}

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

						break;
					case turn_left:
						//LOG_TO_CONSOLE(c_green2,"turn left");
						actors_list[i]->rotate_z_speed=45.0/27.0;
						actors_list[i]->rotate_frames_left=27;
						actors_list[i]->rotating=1;
						//generate a fake movement, so we will know when to make the actor
						//not busy
						actors_list[i]->move_x_speed=0;
						actors_list[i]->move_y_speed=0;
						actors_list[i]->movement_frames_left=27;
						actors_list[i]->moving=1;
						//test
						if(!actors_list[i]->fighting){
							cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_walk_frame);
						}
						actors_list[i]->stop_animation=0;
						break;
					case turn_right:
					//LOG_TO_CONSOLE(c_green2,"turn right");
						actors_list[i]->rotate_z_speed=-45.0/27.0;
						actors_list[i]->rotate_frames_left=27;
						actors_list[i]->rotating=1;
						//generate a fake movement, so we will know when to make the actor
						//not busy
						actors_list[i]->move_x_speed=0;
						actors_list[i]->move_y_speed=0;
						actors_list[i]->movement_frames_left=27;
						actors_list[i]->moving=1;
						//test
						if(!actors_list[i]->fighting){
							cal_actor_set_anim(i,actors_defs[actors_list[i]->actor_type].cal_walk_frame);
						}
						actors_list[i]->stop_animation=0;
						break;
					//ok, now the movement, this is the tricky part
					default:
						if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw) {
							float rotation_angle;

							if(last_command<move_n || last_command>move_nw){//update the frame name too
								cal_actor_set_anim(i,actors_defs[actor_type].cal_walk_frame);
								actors_list[i]->stop_animation=0;
							}

							if(last_command!=actors_list[i]->que[0]){ //Calculate the rotation
								targeted_z_rot=(actors_list[i]->que[0]-move_n)*45.0f;
								rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
								actors_list[i]->rotate_z_speed=rotation_angle/18;
								if(auto_camera && actors_list[i]->actor_id==yourself){
									camera_rotation_speed=rotation_angle/54;
									camera_rotation_frames=54;
								}

								actors_list[i]->rotate_frames_left=18;
								actors_list[i]->rotating=1;
							} else targeted_z_rot=z_rot;

							//ok, now calculate the motion vector...
							actors_list[i]->move_x_speed=(actors_defs[actor_type].walk_speed/3.0f)*sin(targeted_z_rot*M_PI/180.0);
							actors_list[i]->move_y_speed=(actors_defs[actor_type].walk_speed/3.0f)*cos(targeted_z_rot*M_PI/180.0);
							actors_list[i]->movement_frames_left=54/4;
							actors_list[i]->after_move_frames_left=0;
							actors_list[i]->moving=1;
							//test to see if we have a diagonal movement, and if we do, adjust the speeds
							if((actors_list[i]->move_x_speed>0.01f || actors_list[i]->move_x_speed<-0.01f)
							   && (actors_list[i]->move_y_speed>0.01f || actors_list[i]->move_y_speed<-0.01f)) {
								actors_list[i]->move_x_speed*=1.4142315;
								actors_list[i]->move_y_speed*=1.4142315;
							}

							actors_list[i]->fighting=0;
						} else if(actors_list[i]->que[0]>=turn_n && actors_list[i]->que[0]<=turn_nw) {
							float rotation_angle;

							targeted_z_rot=(actors_list[i]->que[0]-turn_n)*45.0f;
							rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
							actors_list[i]->rotate_z_speed=rotation_angle/18.0f;
							actors_list[i]->rotate_frames_left=18;
							actors_list[i]->rotating=1;
							actors_list[i]->stop_animation=1;
						}
					}

					//mark the actor as being busy
					actors_list[i]->busy=1;
					//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Busy");
					//save the last command. It is especially good for run and walk
					actors_list[i]->last_command=actors_list[i]->que[0];
					//move que down with one command
					for(k=0;k<MAX_CMD_QUEUE-1;k++) {
						if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
						actors_list[i]->que[k]=actors_list[i]->que[k+1];
					}
					actors_list[i]->que[k]=nothing;
				}
			}
		}
	UNLOCK_ACTORS_LISTS();
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
				if(actors_list[i] == your_actor) your_actor = NULL;
				if(actors_list[i]->calmodel!=NULL)
					CalModel_Delete(actors_list[i]->calmodel);
				if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
				if(actors_list[i]->is_enhanced_model){
					glDeleteTextures(1,&actors_list[i]->texture_id);
					if(actors_list[i]->body_parts)free(actors_list[i]->body_parts);
				}
#ifdef NEW_SOUND
				stop_sound(actors_list[i]->cur_anim_sound_cookie);
				actors_list[i]->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
				free(actors_list[i]);
				actors_list[i]=NULL;
				if(i==max_actors-1)max_actors--;
				else {
					//copy the last one down and fill in the hole
					max_actors--;
					actors_list[i]=actors_list[max_actors];
					actors_list[max_actors]=NULL;
				}

				UNLOCK_ACTORS_LISTS();
				break;
			}
	}
}

void destroy_all_actors()
{
	int i=0;
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	your_actor = NULL;
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]){
			if(actors_list[i]->calmodel!=NULL)
				CalModel_Delete(actors_list[i]->calmodel);
			if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
			if(actors_list[i]->is_enhanced_model){
				glDeleteTextures(1,&actors_list[i]->texture_id);
				if(actors_list[i]->body_parts)free(actors_list[i]->body_parts);
			}
#ifdef NEW_SOUND
			stop_sound(actors_list[i]->cur_anim_sound_cookie);
			actors_list[i]->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
			free(actors_list[i]);
			actors_list[i]=NULL;
		}
	}
	max_actors= 0;
	my_timer_adjust= 0;
	UNLOCK_ACTORS_LISTS();	//unlock it since we are done
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

void add_command_to_actor(int actor_id, char command)
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

		if(command==leave_combat||command==enter_combat||command==die1||command==die2){
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

			if(act->last_command == nothing){
				//We may be on idle, update the actor so we can reduce the rendering lag
				CalModel_Update(act->calmodel, 5.0f);
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

#ifdef COUNTERS
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
#endif

		UNLOCK_ACTORS_LISTS();

		if(k>MAX_CMD_QUEUE-2){
			update_all_actors();
		}
	}
}

void get_actor_damage(int actor_id, int damage)
{
	//int i=0;
	actor * act;

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

#ifdef COUNTERS
		if (act->cur_health <= 0) {
			increment_death_counter(act);
		}
#endif
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

	actor *me=pf_get_our_actor();

	if(!me)return;//Wtf!?

	x=me->tmp.x_tile_pos;
	y=me->tmp.y_tile_pos;
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

	move_to (tx, ty);
}


void    actor_check_string(actor_types *act, const char *section, const char *type, const char *value){
	char	str[256];
	
	if(value == NULL || *value=='\0'){
		sprintf(str, "Data Error in %s(%d): Missing %s.%s",
			act->actor_name, act->actor_type,
			section, type
		);
		log_error(str);
	}
}

void    actor_check_int(actor_types *act, const char *section, const char *type, int value){
	char    str[256];

	if(value < 0){
		sprintf(str, "Data Error in %s(%d): Missing %s.%s",
		    act->actor_name, act->actor_type,
		    section, type
		);
		log_error(str);
	}
}

xmlNode *get_default_node(xmlNode *cfg, xmlNode *defaults) {
	xmlNode	*item;
	char	*group;
	
	// first, check for errors
	if(defaults == NULL || cfg == NULL){
        return NULL;
	}
	
	//lets find out what group to look for
	group= get_string_property(cfg, "group");
	
	// look for defaul entries with the same name
	for(item=defaults->children; item; item=item->next){
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, cfg->name) == 0){
				char	*item_group;
			
				item_group= get_string_property(item, "group");
				// either both have no group, or both groups match
				if(xmlStrcasecmp(item_group, group) == 0){
					// this is the default entry we want then!
					return item;
				}
			}
		}
	}
	
	// if we got here, there is no default node that matches
	return NULL;
}

int parse_actor_shirt (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	shirt= &(act->shirt[col_idx]);
	ok= 1;
	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp (item->name, "arms") == 0) {
				get_string_value(shirt->arms_name, sizeof(shirt->arms_name), item);
			} else if(xmlStrcasecmp(item->name, "mesh") == 0) {
				get_string_value(shirt->model_name, sizeof(shirt->model_name), item);
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			} else if(xmlStrcasecmp(item->name, "torso") == 0) {
				get_string_value(shirt->torso_name, sizeof(shirt->torso_name), item);
			} else {
				LOG_ERROR("unknown shirt property \"%s\"", item->name);
				ok= 0;
			}
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(shirt->arms_name==NULL || *shirt->arms_name=='\0')
				get_item_string_value(shirt->arms_name, sizeof(shirt->arms_name), default_node, "arms");
			if(shirt->model_name==NULL || *shirt->model_name=='\0'){
				get_item_string_value(shirt->model_name, sizeof(shirt->model_name), default_node, "mesh");
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			}
			if(shirt->torso_name==NULL || *shirt->torso_name=='\0')
				get_item_string_value(shirt->torso_name, sizeof(shirt->torso_name), default_node, "torso");
		}
	}
#endif	//USE_ACTOR_DEFAULTS
	
	// check the critical information
	actor_check_string(act, "shirt", "arms", shirt->arms_name);
	actor_check_string(act, "shirt", "model", shirt->model_name);
	actor_check_int(act, "shirt", "mesh", shirt->mesh_index);
	actor_check_string(act, "shirt", "torso", shirt->torso_name);

	return ok;
}

int parse_actor_skin (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	skin = &(act->skin[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "hands") == 0) {
				get_string_value (skin->hands_name, sizeof (skin->hands_name), item);
			} else if (xmlStrcasecmp (item->name, "head") == 0) {
				get_string_value (skin->head_name, sizeof (skin->head_name), item);
			} else {
				LOG_ERROR("unknown skin property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(skin->hands_name==NULL || *skin->hands_name=='\0')
				get_item_string_value(skin->hands_name, sizeof(skin->hands_name), default_node, "hands");
			if(skin->head_name==NULL || *skin->head_name=='\0')
				get_item_string_value(skin->head_name, sizeof(skin->head_name), default_node, "head");
		}
	}
#endif	//USE_ACTOR_DEFAULTS
	
	// check the critical information
	actor_check_string(act, "skin", "hands", skin->hands_name);
	actor_check_string(act, "skin", "head", skin->head_name);

	return ok;
}

int parse_actor_legs (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	legs = &(act->legs[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (legs->legs_name, sizeof (legs->legs_name), item);
			} else if (xmlStrcasecmp (item->name, "mesh") == 0) {
				get_string_value (legs->model_name, sizeof (legs->model_name), item);
				legs->mesh_index = cal_load_mesh (act, legs->model_name, "legs");
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				legs->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(legs->legs_name==NULL || *legs->legs_name=='\0')
				get_item_string_value(legs->legs_name, sizeof(legs->legs_name), default_node, "skin");
			if(legs->model_name==NULL || *legs->model_name=='\0'){
				get_item_string_value(legs->model_name, sizeof(legs->model_name), default_node, "mesh");
				legs->mesh_index= cal_load_mesh(act, legs->model_name, "legs");
			}
		}
	}
#endif	//USE_ACTOR_DEFAULTS

	// check the critical information
	actor_check_string(act, "legs", "skin", legs->legs_name);
	actor_check_string(act, "legs", "model", legs->model_name);
	actor_check_int(act, "legs", "mesh", legs->mesh_index);

	return ok;
}

int parse_actor_weapon (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
	xmlNode *item;
	char str[255];
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

	weapon = &(act->weapon[type_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "mesh") == 0) {
				get_string_value (weapon->model_name, sizeof (weapon->model_name), item);
				weapon->mesh_index = cal_load_weapon_mesh (act, weapon->model_name, "weapon");
			} else if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (weapon->skin_name, sizeof (weapon->skin_name), item);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up1") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_up_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up2") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_up_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_down1") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_down_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_down2") == 0) {
				get_string_value (str,sizeof(str),item);
     			weapon->cal_attack_down_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				weapon->glow = mode;
			} else {
				LOG_ERROR("unknown weapon property \"%s\"", item->name);
				ok = 0;
			}
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_weapon (act, item->children, defaults);
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(weapon->skin_name==NULL || *weapon->skin_name=='\0')
				get_item_string_value(weapon->skin_name, sizeof(weapon->skin_name), default_node, "skin");
			if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
				if(weapon->model_name==NULL || *weapon->model_name=='\0'){
					get_item_string_value(weapon->model_name, sizeof(weapon->model_name), default_node, "mesh");
					weapon->mesh_index= cal_load_weapon_mesh(act, weapon->model_name, "weapon");
				}
			}
			// TODO: combat animations
		}
	}
#endif	//USE_ACTOR_DEFAULTS

	// check the critical information
	if(type_idx!=WEAPON_NONE){   // no weapon doesn't have a skin/model
		actor_check_string(act, "weapon", "skin", weapon->skin_name);
		if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
			actor_check_string(act, "weapon", "model", weapon->model_name);
			actor_check_int(act, "weapon.mesh", weapon->model_name, weapon->mesh_index);
		}
		// TODO: check combat animations
	}

	return ok;
}

int parse_actor_body_part (actor_types *act, body_part *part, xmlNode *cfg, const char *part_name, xmlNode *default_node) {
	xmlNode *item;
	int ok = 1;

	if(cfg == NULL) return 0;

	for(item=cfg; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, "mesh") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
				if(strcmp("shield",part_name)==0)
					part->mesh_index = cal_load_weapon_mesh (act, part->model_name, part_name);
				else
					part->mesh_index = cal_load_mesh (act, part->model_name, part_name);
			} else if(xmlStrcasecmp(item->name, "skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if(xmlStrcasecmp(item->name, "glow") == 0) {
				int mode= find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if(mode < 0) mode = GLOW_NONE;
				part->glow= mode;
			} else {
				LOG_ERROR("unknown %s property \"%s\"", part_name, item->name);
				ok = 0;
			}
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(default_node){
		if(part->skin_name==NULL || *part->skin_name=='\0')
			if(strcmp(part_name, "head")){ // heads don't have seperate skins here
				get_item_string_value(part->skin_name, sizeof(part->skin_name), default_node, "skin");
			}
		if(part->model_name==NULL || *part->model_name=='\0'){
			get_item_string_value(part->model_name, sizeof(part->model_name), default_node, "mesh");
			if(strcmp("shield",part_name)==0)
				part->mesh_index= cal_load_weapon_mesh(act, part->model_name, part_name);
			else
				part->mesh_index= cal_load_mesh(act, part->model_name, part_name);
		}
	}
#endif	//USE_ACTOR_DEFAULTS

	// check the critical information
	if(strcmp(part_name, "head")){ // heads don't have seperate skins here
		actor_check_string(act, part_name, "skin", part->skin_name);
	}
	actor_check_string(act, part_name, "model", part->model_name);
	actor_check_int(act, part_name, "mesh", part->mesh_index);

	return ok;
}

int parse_actor_helmet (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	helmet= &(act->helmet[type_idx]);
	return parse_actor_body_part(act,helmet, cfg->children, "helmet", default_node);
}

int parse_actor_cape (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	cape= &(act->cape[type_idx]);
	return parse_actor_body_part(act,cape, cfg->children, "cape", default_node);
}

int parse_actor_head (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	head= &(act->head[type_idx]);
	return parse_actor_body_part(act, head, cfg->children, "head", default_node);
}

int parse_actor_shield (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
	xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *shield;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "shield type", shield_type_dict);
	}
	if(type_idx < 0 || type_idx >= ACTOR_SHIELD_SIZE){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	shield= &(act->shield[type_idx]);
	return parse_actor_body_part(act,shield, cfg->children, "shield", default_node);
}

int parse_actor_hair (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	buf= act->hair[col_idx].hair_name;
	len= sizeof (act->hair[col_idx].hair_name);
	get_string_value(buf, len, cfg);
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
	strncpy(act->idle_group[res].name, name, sizeof(act->idle_group[res].name));
	++act->group_count;

	return res;
}

struct cal_anim cal_load_idle(actor_types *act, char *str)
{
	struct cal_anim res = {-1,0,0};
	struct CalCoreAnimation *coreanim;

	res.anim_index=CalCoreModel_LoadCoreAnimation(act->coremodel,str);
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

void parse_idle_group(actor_types *act,char *str)
{
	char gname[255]={0};
	char fname[255]={0};
	//char temp[255];
	int gindex;

	if(sscanf(str,"%s %s",gname,fname)!=2)return;

	gindex=cal_get_idle_group(act,gname);
	cal_group_addanim(act,gindex,fname);
	//sprintf(temp,"%d",gindex);
	//LOG_TO_CONSOLE(c_green2,gname);
	//LOG_TO_CONSOLE(c_green2,fname);
	//LOG_TO_CONSOLE(c_green2,temp);
}

int parse_actor_frames (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
	xmlNode *item;
	char str[255];
	//char fname[255];
	//char temp[255];
	//int i;

	int ok = 1;
	if (cfg == NULL) return 0;

	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {

			if (xmlStrcasecmp (item->name, "CAL_IDLE_GROUP") == 0) {
				get_string_value (str,sizeof(str),item);
     				//act->cal_walk_frame=cal_load_anim(act,str);
				//LOG_TO_CONSOLE(c_green2,str);
				parse_idle_group(act,str);
				//Not functional!
			} else if (xmlStrcasecmp (item->name, "CAL_walk") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_walk_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_run") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_run_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_die1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_die1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_die2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_die2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_pain1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pain1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_pain2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pain2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_pick") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_pick_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_drop") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_drop_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_idle2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_idle_sit") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_idle_sit_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
 			} else if (xmlStrcasecmp (item->name, "CAL_harvest") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_harvest_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_cast") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_cast_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_sit_down") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_sit_down_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_stand_up") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_stand_up_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_in_combat") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_in_combat_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_out_combat") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_out_combat_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_combat_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_combat_idle_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up_1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up_2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up_3") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_3_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_up_4") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_up_4_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_down_1") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_down_1_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
#endif	//NEW_SOUND
#ifdef	NEW_ACTOR_ANIMATION
					, get_int_property(item, "duration")
#endif	//NEW_ACTOR_ANIMATION
					);
			} else if (xmlStrcasecmp (item->name, "CAL_attack_down_2") == 0) {
				get_string_value (str,sizeof(str),item);
     			act->cal_attack_down_2_frame=cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item,"sound")
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

int parse_actor_boots (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
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

	boots = &(act->boots[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (boots->boots_name, sizeof (boots->boots_name), item);
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				boots->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

#ifdef	USE_ACTOR_DEFAULTS
	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);
		
		if(default_node){
			if(boots->boots_name==NULL || *boots->boots_name=='\0')
				get_item_string_value(boots->boots_name, sizeof(boots->boots_name), default_node, "skin");
		}
	}
#endif	//USE_ACTOR_DEFAULTS
	
	// check the critical information
	actor_check_string(act, "boots", "boots", boots->boots_name);

	return ok;
}

void MD2_to_CMF(char *str)
{
	int l;
	l=strlen(str);

	if (l<3) return;
	str[l-3]='c';
	str[l-2]='m';
	str[l-1]='f';
}

//Searches if a mesh is already loaded- TODO:MAKE THIS BETTER
int cal_search_mesh (actor_types *act, const char *fn, const char *kind)
{
	int i;

	if (kind == NULL)
	{
		return -1;
	}
	else if (strcmp (kind, "head") == 0)
	{
		for (i = 0; i < ACTOR_HEAD_SIZE; i++)
			if (strcmp (fn, act->head[i].model_name) == 0 && act->head[i].mesh_index != -1)
				return act->head[i].mesh_index;
	}
	else if (strcmp (kind, "shirt") == 0)
	{
		for (i = 0; i < ACTOR_SHIRT_SIZE; i++)
		{
			if (strcmp (fn, act->shirt[i].model_name) == 0 && act->shirt[i].mesh_index != -1)
				return act->shirt[i].mesh_index;
		}
	}
	else if (strcmp (kind, "legs") == 0)
	{
		for (i = 0; i < ACTOR_LEGS_SIZE; i++)
		{
			if (strcmp (fn, act->legs[i].model_name) == 0 && act->legs[i].mesh_index != -1)
				return act->legs[i].mesh_index;
		}
	}
	else if (strcmp (kind, "cape") == 0)
	{
		for (i = 0; i < ACTOR_CAPE_SIZE; i++)
		{
			if (strcmp (fn, act->cape[i].model_name) == 0 && act->cape[i].mesh_index != -1)
				return act->cape[i].mesh_index;
		}
	}
	else if (strcmp (kind, "helmet") == 0)
	{
		for (i = 0; i < ACTOR_HELMET_SIZE; i++)
		{
			if (strcmp (fn, act->cape[i].model_name) == 0 && act->helmet[i].mesh_index != -1)
				return act->helmet[i].mesh_index;
		}
	}
	else if (strcmp (kind, "shield") == 0)
	{
		for (i = 0; i < ACTOR_SHIELD_SIZE; i++)
		{
			if (strcmp (fn, act->shield[i].model_name) == 0 && act->shield[i].mesh_index != -1)
				return act->shield[i].mesh_index;
		}
	}
	else if (strcmp (kind, "weapon") == 0)
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
	//sscanf(fn,"./md2/%s",temp);
	//MD2_to_CMF(temp);
	//sprintf(fname,"./Meshes/%s",temp);
	//LOG_TO_CONSOLE(c_green2,fn);

	//Load coremesh
	res=CalCoreModel_LoadCoreMesh(act->coremodel,fn);

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

	//sscanf(fn,"./md2/%s",temp);
	//MD2_to_CMF(temp);
	//sprintf(fname,"./Meshes/%s",temp);
	//LOG_TO_CONSOLE(c_green2,fn);

	//Load coremesh
	res=CalCoreModel_LoadCoreMesh(act->coremodel,fn);

	//Scale coremesh
	if (res>=0) {
		mesh=CalCoreModel_GetCoreMesh(act->coremodel,res);
		if ((mesh)&&(act->skel_scale!=1.0)) CalCoreMesh_Scale(mesh,act->skel_scale);
	} else {
		log_error("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}

	return res;
}

int	parse_actor_nodes (actor_types *act, xmlNode *cfg, xmlNode *defaults) {
	xmlNode	*item;
	int	ok= 1;
	
	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, "ghost") == 0) {
				act->ghost= get_bool_value (item);
			} else if(xmlStrcasecmp(item->name, "skin") == 0) {
				get_string_value(act->skin_name, sizeof (act->skin_name), item);
			} else if(xmlStrcasecmp(item->name, "mesh") == 0) {
				get_string_value(act->file_name, sizeof (act->file_name), item);
			} else if(xmlStrcasecmp(item->name, "scale")==0) {
				act->scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, "mesh_scale")==0) {
				act->mesh_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, "bone_scale")==0) {
				act->skel_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, "skeleton")==0) {
				get_string_value(act->skeleton_name, sizeof (act->skeleton_name), item);
				act->coremodel= CalCoreModel_New("Model");
				if(!CalCoreModel_LoadCoreSkeleton(act->coremodel, act->skeleton_name)) {
					log_error("Cal3d error: %s: %s\n", act->skeleton_name, CalError_GetLastErrorDescription());
				}
			} else if(xmlStrcasecmp(item->name, "walk_speed") == 0) {
				act->walk_speed= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, "run_speed") == 0) {
				act->run_speed= get_float_value(item);

			} else if(xmlStrcasecmp(item->name, "defaults") == 0) {
				defaults= item;
			} else if(xmlStrcasecmp(item->name, "frames") == 0) {
				ok &= parse_actor_frames(act, item->children, defaults);
			} else if(xmlStrcasecmp(item->name, "shirt") == 0) {
				ok &= parse_actor_shirt(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "hskin") == 0) {
				ok &= parse_actor_skin(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "hair") == 0) {
				ok &= parse_actor_hair(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "boots") == 0) {
				ok &= parse_actor_boots(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "legs") == 0) {
				ok &= parse_actor_legs(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "cape") == 0) {
				ok &= parse_actor_cape(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "head") == 0) {
				ok &= parse_actor_head(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "shield") == 0) {
				ok &= parse_actor_shield(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "weapon") == 0) {
				ok &= parse_actor_weapon(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, "helmet") == 0) {
				ok &= parse_actor_helmet(act, item, defaults);
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

int parse_actor_script (xmlNode *cfg) {
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

		strcpy(name,get_string_property(cfg, "type"));
		sprintf(str, "Data Error in %s(%d): Actor ID out of range %d",
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

		strcpy(name,get_string_property(cfg, "type"));
		sprintf(str, "Data Error in %s(%d): Already loaded %s(%d)",
			name, act_idx, act->actor_name, act->actor_type
		);
		log_error(str);
	}
	ok= 1;
	act->actor_type= act_idx;	// memorize the ID & name to help in debugging
	strcpy(act->actor_name, get_string_property(cfg, "type"));
	actor_check_string(act, "actor", "name", act->actor_name);

	//Initialize Cal3D settings
	act->coremodel= NULL;
	act->scale= 1.0;
	act->mesh_scale= 1.0;
	act->skel_scale= 1.0;
	act->group_count= 0;
	for (i=0; i<16; ++i) {
		strncpy(act->idle_group[i].name, "", sizeof(act->idle_group[i].name));
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

	for (i=0; i<80; ++i){
		act->weapon[i].cal_attack_up_1_frame.anim_index=-1;
		act->weapon[i].cal_attack_up_2_frame.anim_index=-1;
		act->weapon[i].cal_attack_down_1_frame.anim_index=-1;
		act->weapon[i].cal_attack_down_2_frame.anim_index=-1;
		act->weapon[i].mesh_index = -1;
	}

	//Init head meshes
	for(i=0; i<ACTOR_HEAD_SIZE; i++)
		act->head[i].mesh_index= -1;
	//Init shield meshes
	for(i=0; i<ACTOR_SHIELD_SIZE; i++)
		act->shield[i].mesh_index= -1;
	//Init cape meshes
	for(i=0; i<ACTOR_CAPE_SIZE; i++)
		act->cape[i].mesh_index= -1;
	//Init helmet meshes
	for(i=0; i<ACTOR_HELMET_SIZE; i++)
		act->helmet[i].mesh_index= -1;
	//Init torso meshes
	for(i=0; i<ACTOR_SHIRT_SIZE; i++)
		act->shirt[i].mesh_index= -1;
	//Init legs meshes
	for(i=0; i<ACTOR_LEGS_SIZE; i++)
		act->legs[i].mesh_index= -1;

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
		if(strcmp (act->head[0].model_name, "") == 0)
			act->shirt[0].mesh_index= cal_load_mesh(act, act->file_name, NULL); //save the single meshindex as torso
	}

	return ok;
}

int parse_actor_defs (xmlNode *node) {
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp (def->name, "actor") == 0) {
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

int read_actor_defs (const char *dir, const char *index) {
	xmlNode *root;
	xmlDoc *doc;
	char fname[120];
	int ok = 1;

	snprintf (fname, sizeof(fname), "%s/%s", dir, index);

	doc = xmlReadFile (fname, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read actor definition file %s", fname);
		return 0;
	}

	root = xmlDocGetRootElement (doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse actor definition file %s", fname);
		ok = 0;
	} else if (xmlStrcasecmp (root->name, "actors") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"actors\" expected).", root->name);
		ok = 0;
	} else {
		ok = parse_actor_defs (root);
	}

	xmlFreeDoc (doc);
	return ok;
}

void init_actor_defs () {
	const char *dirname = "actor_defs", *idxname = "actor_defs.xml";
	char defdir[256];
	int ok;

	// initialize the whole thing to zero
	memset (actors_defs, 0, sizeof (actors_defs));

#ifndef WINDOWS
	snprintf (defdir, sizeof (defdir), "%s/%s", datadir, dirname);
#else
	my_strcp (defdir, dirname);
#endif

	ok = read_actor_defs (defdir, idxname);
}
