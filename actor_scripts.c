#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

// Element type and dictionaries for actor definitions
typedef struct {
	char *desc;
	int index;
} dict_elem;

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
	  { "gnome male"            , gnome_male             },
	  { "orchan female"         , orchan_female          },
	  { "orchan male"           , orchan_male            },
	  { "draegoni female"       , draegoni_female        },
	  { "draegoni male"         , draegoni_male          },
	  { NULL                    , -1                     }
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
	  { "undefined armor"     , SHIRT_ARMOR_6              },
	  { "fur"                 , SHIRT_FUR                  }, 
	  { NULL                  , -1                         }
	};

const dict_elem skin_color_dict[] = 
	{ { "brown" , SKIN_BROWN  }, 
	  { "normal", SKIN_NORMAL }, 
	  { "pale"  , SKIN_PALE   }, 
	  { "tan"   , SKIN_TAN    },
	  { NULL    , -1          }
	};

const dict_elem hair_color_dict[] = 
	{ { "black" , HAIR_BLACK  },
	  { "blond" , HAIR_BLOND  },
	  { "brown" , HAIR_BROWN  },
	  { "grey"  , HAIR_GRAY   },
	  { "red"   , HAIR_RED    },
	  { "white" , HAIR_WHITE  },
	  { "blue"  , HAIR_BLUE   },
	  { "green" , HAIR_GREEN  },
	  { "purple", HAIR_PURPLE },
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
	  { "moonshadow", CAPE_MOONSHADOW },
	  { "ravenod"   , CAPE_RAVENOD    },
	  { "rogue"     , CAPE_ROGUE      },
	  { "wytter"    , CAPE_WYTTER     },
	  { "quell"     , CAPE_QUELL      },
	  { "none"      , CAPE_NONE       },
	  { NULL        , -1              }
	};

const dict_elem shield_type_dict[] =
	{ { "wood"         , SHIELD_WOOD          },
	  { "wood enhanced", SHIELD_WOOD_ENHANCED },
	  { "iron"         , SHIELD_IRON          },
	  { "steel"        , SHIELD_STEEL         },
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


void move_to_next_frame()
{
	int i,l,k;
	char frame_name[16];
	char frame_number[3];
	int frame_no;
	int numFrames;
	char frame_exists;

	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i]!=0)
				{
					//clear the strings
					for(k=0;k<16;k++)frame_name[k]=0;
					for(k=0;k<3;k++)frame_number[k]=0;

					//now see if we can find that frame
					if(!actors_list[i]->is_enhanced_model)
						numFrames=actors_list[i]->model_data->numFrames;
					else
						numFrames=actors_list[i]->body_parts->head->numFrames;

					//first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
					if(actors_list[i]->damage_ms)
						{
							actors_list[i]->damage_ms-=80;
							if(actors_list[i]->damage_ms<0)actors_list[i]->damage_ms=0;
						}
					//get the frame number out of the frame name
					l=strlen(actors_list[i]->cur_frame);
					if(l<2)//Perhaps this is the bug we've been looking for all along?
#ifndef EXTRA_DEBUG
						continue;
#else
						{
							continue;
							ERR();
						}
#endif
					frame_no=atoi(&actors_list[i]->cur_frame[l-2]);
					//get the frame name
					for(k=0;k<l-2;k++)frame_name[k]=actors_list[i]->cur_frame[k];
					//increment the frame_no
					frame_no++;
					//9 frames, not moving, and another command is queued farther on (based on how long we've done this action)
					if(frame_no > 9 && !actors_list[i]->moving && !actors_list[i]->rotating)
						{
							//if(actors_list[i]->que[0]!=nothing)
							if(actors_list[i]->que[(frame_no<35?7-(frame_no/5):0)]!=nothing)
							//(actors_list[i]->que[(frame_no<63?7-(frame_no/9):0)]!=nothing)
							{
								actors_list[i]->stop_animation=1;	//force stopping, not looping
								actors_list[i]->busy=0;	//ok, take the next command
							}
						}

					//transform back into string
					frame_number[0]=(unsigned int)48+frame_no/10;
					frame_number[1]=(unsigned int)48+frame_no%10;
					//create the name of the next frame to look for
					my_strcat(frame_name,frame_number);

					frame_exists=0;
					for(k=0;k<numFrames;k++)
						{
							if(!actors_list[i]->is_enhanced_model)
								{
									if(strcmp(frame_name,actors_list[i]->model_data->offsetFrames[k].name)==0)
										{
											frame_exists=1;
											break;
										}
								}
							else
								{
									if(strcmp(frame_name,actors_list[i]->body_parts->head->offsetFrames[k].name)==0)
										{
											frame_exists=1;
											break;
										}
								}
						}

					if(!frame_exists)//frame doesn't exist, move at the beginning of animation
						{
							if(actors_list[i]->stop_animation)
								{
									actors_list[i]->busy=0;//ok, take the next command
									continue;//we are done with this guy
								}
							else
								{
									//frame_name has 2 extra numbers, at this point, due to the previous
									//strcat. So, remove those 2 extra numbers
									l=strlen(frame_name);
									if(l<2)continue;
									frame_name[l-2]=0;
									my_strcat(frame_name,"01");
								}
						}
					
					sprintf(actors_list[i]->cur_frame, "%s",frame_name);
				}
		}
	UNLOCK_ACTORS_LISTS();
}

void animate_actors()
{
	int i;
	// lock the actors_list so that nothing can interere with this look
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->moving)
						{
							actors_list[i]->movement_frames_left--;
							if(!actors_list[i]->movement_frames_left)//we moved all the way
								{
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


								}
							else
								{
									actors_list[i]->x_pos+=actors_list[i]->move_x_speed;
									actors_list[i]->y_pos+=actors_list[i]->move_y_speed;
									actors_list[i]->z_pos+=actors_list[i]->move_z_speed;
								}
						}
					else //not moving
						{
							if(actors_list[i]->after_move_frames_left)
								{
									actors_list[i]->after_move_frames_left--;
									if(!actors_list[i]->after_move_frames_left)actors_list[i]->busy=0;

								}
						}

					if(actors_list[i]->rotating)
						{
							actors_list[i]->rotate_frames_left--;
							if(!actors_list[i]->rotate_frames_left)//we rotated all the way
								actors_list[i]->rotating=0;//don't rotate next time, ok?

							actors_list[i]->x_rot+=actors_list[i]->rotate_x_speed;
							actors_list[i]->y_rot+=actors_list[i]->rotate_y_speed;
							actors_list[i]->z_rot+=actors_list[i]->rotate_z_speed;
						}
				}
		}
	// unlock the actors_list since we are done now
	UNLOCK_ACTORS_LISTS();
}




//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i;
	int max_queue=0;

	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++)
		{
			if(!actors_list[i])continue;//actor exists?
			if(!actors_list[i]->busy || (actors_list[i]->busy && actors_list[i]->after_move_frames_left && (actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw)))//is it not busy?
				{
					if(actors_list[i]->que[0]==nothing)//do we have something in the que?
						{
							//if que is empty, set on idle
							if(!actors_list[i]->dead)
								{
									actors_list[i]->stop_animation=0;

									if(actors_list[i]->fighting)
										{
											my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].combat_idle_frame);
										}

									else if(!actors_list[i]->sitting)
										{
											if(!actors_list[i]->sit_idle)
												{
													my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].idle_frame);
													actors_list[i]->sit_idle=1;
												}
										}
									else
										{
											if(!actors_list[i]->stand_idle)
												{
													my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].idle_sit_frame);
													actors_list[i]->stand_idle=1;
												}
										}

								}

							actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
						}
					else
						{
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
/*								if(actors_list[i]->remapped_colors)
									glDeleteTextures(1,&actors_list[i]->texture_id);
								free(actors_list[i]);
								actors_list[i]=0;*/ //Obsolete
								break;			
							case die1:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].die1_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->dead=1;
								break;
							case die2:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].die2_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->dead=1;
								break;
							case pain1:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].pain1_frame);
								actors_list[i]->stop_animation=1;
								break;
							case pain2:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].pain2_frame);
								actors_list[i]->stop_animation=1;
								break;
							case pick:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].pick_frame);
								actors_list[i]->stop_animation=1;
								break;
							case drop:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].drop_frame);
								actors_list[i]->stop_animation=1;
								break;
							case harvest:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].harvest_frame);
								actors_list[i]->stop_animation=1;
								break;
							case cast:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_cast_frame);
								actors_list[i]->stop_animation=1;
								break;
							case ranged:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_ranged_frame);
								actors_list[i]->stop_animation=1;
								break;
							case sit_down:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].sit_down_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->sitting=1;
								if(actors_list[i]->actor_id==yourself)
									you_sit_down();
								break;
							case stand_up:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].stand_up_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->sitting=0;
								if(actors_list[i]->actor_id==yourself)
									you_stand_up();
								break;
							case enter_combat:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].in_combat_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case leave_combat:
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].out_combat_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=0;
								break;
							case attack_up_1:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up1);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_1_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case attack_up_2:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up1);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_2_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case attack_up_3:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up2);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_3_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case attack_up_4:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up2);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_4_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case attack_down_1:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_down1);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_down_1_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case attack_down_2:
								if(actors_list[i]->is_enhanced_model)
									my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_down2);
								else my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].attack_down_2_frame);
								actors_list[i]->stop_animation=1;
								actors_list[i]->fighting=1;
								break;
							case turn_left:
								actors_list[i]->rotate_z_speed=45.0/9.0;
								actors_list[i]->rotate_frames_left=9;
								actors_list[i]->rotating=1;
								//generate a fake movement, so we will know when to make the actor
								//not busy
								actors_list[i]->move_x_speed=0;
								actors_list[i]->move_y_speed=0;
								actors_list[i]->movement_frames_left=9;
								actors_list[i]->moving=1;
								//test
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
								actors_list[i]->stop_animation=1;
								break;
							case turn_right:
								actors_list[i]->rotate_z_speed=-45.0/9.0;
								actors_list[i]->rotate_frames_left=9;
								actors_list[i]->rotating=1;
								//generate a fake movement, so we will know when to make the actor
								//not busy
								actors_list[i]->move_x_speed=0;
								actors_list[i]->move_y_speed=0;
								actors_list[i]->movement_frames_left=9;
								actors_list[i]->moving=1;
								//test
								my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
								actors_list[i]->stop_animation=1;
								break;
							//ok, now the movement, this is the tricky part
							default:
								if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw)
									{
									float rotation_angle;

									if(last_command<move_n || last_command>move_nw)//update the frame name too
										my_strcp(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
									actors_list[i]->stop_animation=0;
									if(last_command!=actors_list[i]->que[0])//we need to calculate the rotation...
										{
											targeted_z_rot=(actors_list[i]->que[0]-move_n)*45.0f;
											rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
											actors_list[i]->rotate_z_speed=rotation_angle/6;
											if(auto_camera)
												if(actors_list[i]->actor_id==yourself)
													{
														camera_rotation_speed=rotation_angle/18;
														camera_rotation_frames=18;
													}

											actors_list[i]->rotate_frames_left=6;
											actors_list[i]->rotating=1;
										}
									else targeted_z_rot=z_rot;
									//ok, now calculate the motion vector...
									actors_list[i]->move_x_speed=actors_defs[actor_type].walk_speed*sin(targeted_z_rot*3.1415926/180.0);
									actors_list[i]->move_y_speed=actors_defs[actor_type].walk_speed*cos(targeted_z_rot*3.1415926/180.0);
									actors_list[i]->movement_frames_left=18/4;
									actors_list[i]->after_move_frames_left=0;
									actors_list[i]->moving=1;
									//test to see if we have a diagonal movement, and if we do, adjust the speeds

									if((actors_list[i]->move_x_speed>0.01f || actors_list[i]->move_x_speed<-0.01f)
									   && (actors_list[i]->move_y_speed>0.01f || actors_list[i]->move_y_speed<-0.01f))
										{
											actors_list[i]->move_x_speed*=1.4142315;
											actors_list[i]->move_y_speed*=1.4142315;
										}
								}
							else if(actors_list[i]->que[0]>=turn_n && actors_list[i]->que[0]<=turn_nw)
								{
									float rotation_angle;
									targeted_z_rot=(actors_list[i]->que[0]-turn_n)*45.0f;
									rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
									actors_list[i]->rotate_z_speed=rotation_angle/6.0f;
									actors_list[i]->rotate_frames_left=6;
									actors_list[i]->rotating=1;
									actors_list[i]->stop_animation=1;
								}
							}

							//mark the actor as being busy
							actors_list[i]->busy=1;
							//save the last command. It is especially good for run and walk
							actors_list[i]->last_command=actors_list[i]->que[0];
							//move que down with one command
							for(k=0;k<10-1;k++)
								{
									if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
									actors_list[i]->que[k]=actors_list[i]->que[k+1];
								}
							actors_list[i]->que[k]=nothing;
						}
				}
		}
	UNLOCK_ACTORS_LISTS();
	if(max_queue >= 4)my_timer_adjust+=6+(max_queue-4);	//speed up the timer clock if we are building up too much
}


void destroy_actor(int actor_id)
{
#ifdef EXTRA_DEBUG
	ERR();
#endif
	int i;

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])//The timer thread doesn't free memory
				if(actors_list[i]->actor_id==actor_id)
					{
						LOCK_ACTORS_LISTS();
						if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
						if(actors_list[i]->is_enhanced_model)
							{
								glDeleteTextures(1,&actors_list[i]->texture_id);
								if(actors_list[i]->body_parts)free(actors_list[i]->body_parts);
							}
						free(actors_list[i]);
						actors_list[i]=0;
						if(i==max_actors-1)max_actors--;
						else
							{
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
	actor *to_free;
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
					if(actors_list[i]->is_enhanced_model)
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							free(actors_list[i]->body_parts);
						}
					to_free = actors_list[i];
					actors_list[i]=NULL;
					free(to_free);
				}
		}
	max_actors=0;
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
	int i=0;
	int k=0;
	int have_actor=0;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	while(i<max_actors)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)//The timer thread can't free so this should be np...
					{
						LOCK_ACTORS_LISTS();
						for(k=0;k<10;k++)
							{
								if(actors_list[i]->que[k]==nothing)
									{
										//we are SEVERLY behind, just update all the actors in range
										if(k>8) break;
										else if(k>6)
											{
												// is the front a sit/stand spam?
												if((actors_list[i]->que[0]==stand_up||actors_list[i]->que[0]==sit_down)
												&&(actors_list[i]->que[1]==stand_up||actors_list[i]->que[1]==sit_down))
													{
														int	j;
														//move que down with one command
														for(j=0;j<=k;j++)
															{
																actors_list[i]->que[j]=actors_list[i]->que[j+1];
															}
														actors_list[i]->que[j]=nothing;
														//backup one entry
														k--;
													}

												// is the end a sit/stand spam?
												else if((command==stand_up||command==sit_down)
												&& (actors_list[i]->que[k-1]==stand_up||actors_list[i]->que[k-1]==sit_down))
													{
														actors_list[i]->que[k-1]=command;
														break;
													}

											}
										actors_list[i]->que[k]=command;
										break;
									}
							}
						UNLOCK_ACTORS_LISTS();
						have_actor=1;
						break;
					}
			i++;
		}

#ifdef EXTRA_DEBUG
	ERR();
#endif

	if(!have_actor)
		{
#ifdef EXTRA_DEBUG
	ERR();
#endif
			//if we got here, it means we don't have this actor, so get it from the server...
			char	str[256];
			sprintf(str, "%s %d - %d\n", cant_add_command, command, actor_id);
			LOG_ERROR(str);
		}
	else if (k>8)
		{
			update_all_actors();
		}
}

void get_actor_damage(int actor_id, int damage)
{
	int i=0;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	while(i<max_actors)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						actors_list[i]->damage=damage;
						actors_list[i]->damage_ms=2000;
						actors_list[i]->cur_health-=damage;
						break;
					}
			i++;
		}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void get_actor_heal(int actor_id, int quantity)
{
	int i=0;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	while(i<max_actors)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						actors_list[i]->cur_health+=quantity;
						break;
					}
			i++;
		}
	//if we got here, it means we don't have this actor, so get it from the server...

}


void move_self_forward()
{
	int i,x,y,rot,tx,ty;
	Uint8 str[10];

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i] && actors_list[i]->actor_id==yourself)
				{
					LOCK_ACTORS_LISTS();
					x=actors_list[i]->x_tile_pos;
					y=actors_list[i]->y_tile_pos;
					rot=actors_list[i]->z_rot;
					rot=unwindAngle_Degrees(rot);
					switch(rot) {
					case 0:
						tx=x;
						ty=y+1;
						break;
					case 45:
						tx=x+1;
						ty=y+1;
						break;
					case 90:
						tx=x+1;
						ty=y;
						break;
					case 135:
						tx=x+1;
						ty=y-1;
						break;
					case 180:
						tx=x;
						ty=y-1;
						break;
					case 225:
						tx=x-1;
						ty=y-1;
						break;
					case 270:
						tx=x-1;
						ty=y;
						break;
					case 315:
						tx=x-1;
						ty=y+1;
						break;
					default:
						tx=x;
						ty=y;
					}

					//check to see if the coordinates are OUTSIDE the map
					if(ty<0 || tx<0 || tx>=tile_map_size_x*6 || ty>=tile_map_size_y*6)return;

					if (pf_follow_path) {
						pf_destroy_path();
					}

					str[0]=MOVE_TO;
					*((short *)(str+1))=SDL_SwapLE16((short)tx);
					*((short *)(str+3))=SDL_SwapLE16((short)ty);

					my_tcp_send(my_socket,str,5);
					UNLOCK_ACTORS_LISTS();
					return;
				}
		}
}


int find_description_index (const dict_elem dict[], const char *elem, const char *desc) {
	int idx = 0;
	char errmsg[120];
	char *key;
	
	while ((key = dict[idx].desc) != NULL) {
		if (strcasecmp (key, elem) == 0)
			return dict[idx].index;
		idx++;
	}

	snprintf (errmsg, sizeof (errmsg), "Unknown %s \"%s\"\n", desc, elem);
	LOG_ERROR(errmsg);
	return -1;
}

void get_string_value (char *buf, size_t maxlen, xmlNode *node) {
	if (node->children == NULL)
		buf[0] = '\0';
	else 
		my_strncp (buf, node->children->content, maxlen);
}

int get_bool_value (xmlNode *node) {
	char *tval;
	if (node->children == NULL) return 0;
	tval = node->children->content;
	return (xmlStrcasecmp (tval, "yes") == 0 || xmlStrcasecmp (tval, "true") == 0 || xmlStrcasecmp (tval, "1") == 0);
}

double get_float_value (xmlNode *node) {
	if (node->children == NULL) return 0.0;
	return atof (node->children->content);
}

int get_property (xmlNode *node, const char *prop, const char *desc, const dict_elem dict[]) {
	xmlAttr *attr;
	char errmsg[120];
	
	for (attr = node->properties; attr; attr = attr->next) {
		if (attr->type == XML_ATTRIBUTE_NODE && xmlStrcasecmp (attr->name, prop) == 0) {
			return find_description_index (dict,  attr->children->content, desc);
		}
	}
	
	snprintf (errmsg, sizeof(errmsg), "Unable to find property %s in node %s\n", prop, node->name);
	LOG_ERROR(errmsg);
	return -1;
}

int parse_actor_shirt (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, col_idx;
	shirt_part *shirt;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	col_idx = get_property (cfg, "color", "shirt color", shirt_color_dict);
	if (col_idx < 0) return 0;
	
	shirt = &(act->shirt[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "arms") == 0) {
				get_string_value (shirt->arms_name, sizeof (shirt->arms_name), item);
			} else if (xmlStrcasecmp (item->name, "model") == 0) {
				get_string_value (shirt->model_name, sizeof (shirt->model_name), item);
			} else if (xmlStrcasecmp (item->name, "torso") == 0) {
				get_string_value (shirt->torso_name, sizeof (shirt->torso_name), item);
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown shirt property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_skin (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, col_idx;
	skin_part *skin;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	col_idx = get_property (cfg, "color", "skin color", skin_color_dict);
	if (col_idx < 0) return 0;
	
	skin = &(act->skin[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "hands") == 0) {
				get_string_value (skin->hands_name, sizeof (skin->hands_name), item);
			} else if (xmlStrcasecmp (item->name, "head") == 0) {
				get_string_value (skin->head_name, sizeof (skin->head_name), item);
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown skin property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_legs (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, col_idx;
	legs_part *legs;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	col_idx = get_property (cfg, "color", "legs color", legs_color_dict);
	if (col_idx < 0) return 0;
	
	legs = &(act->legs[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (legs->legs_name, sizeof (legs->legs_name), item);
			} else if (xmlStrcasecmp (item->name, "model") == 0) {
				get_string_value (legs->model_name, sizeof (legs->model_name), item);
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				legs->glow = mode;
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown legs property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_weapon (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, type_idx;
	weapon_part *weapon;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	type_idx = get_property (cfg, "type", "weapon type", weapon_type_dict);
	if (type_idx < 0) return 0;
	
	weapon = &(act->weapon[type_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "model") == 0) {
				get_string_value (weapon->model_name, sizeof (weapon->model_name), item);
			} else if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (weapon->skin_name, sizeof (weapon->skin_name), item);
			} else if (xmlStrcasecmp (item->name, "attack_up1") == 0) {
				get_string_value (weapon->attack_up1, sizeof (weapon->attack_up1), item);
			} else if (xmlStrcasecmp (item->name, "attack_up2") == 0) {
				get_string_value (weapon->attack_up2, sizeof (weapon->attack_up2), item);
			} else if (xmlStrcasecmp (item->name, "attack_down1") == 0) {
				get_string_value (weapon->attack_down1, sizeof (weapon->attack_down1), item);
			} else if (xmlStrcasecmp (item->name, "attack_down2") == 0) {
				get_string_value (weapon->attack_down2, sizeof (weapon->attack_down2), item);
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				weapon->glow = mode;
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown weapon property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_body_part (body_part *part, xmlNode *cfg, const char *part_name) {
	xmlNode *item;
	char errmsg[120];
	int ok = 1;

	if (cfg == NULL) return 0;
	
	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "model") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
			} else if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if (xmlStrcasecmp (item->name, "glow") == 0) {
				int mode = find_description_index (glow_mode_dict, item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				part->glow = mode;
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown %s property \"%s\"", part_name, item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_helmet (actor_types *act, xmlNode *cfg) {
	int type_idx;
	body_part *helmet;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	type_idx = get_property (cfg, "type", "helmet type", helmet_type_dict);
	if (type_idx < 0) return 0;
	
	helmet = &(act->helmet[type_idx]);
	return parse_actor_body_part (helmet, cfg->children, "helmet");
}

int parse_actor_cape (actor_types *act, xmlNode *cfg) {
	int type_idx;
	body_part *cape;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	type_idx = get_property (cfg, "color", "cape color", cape_color_dict);
	if (type_idx < 0) return 0;
	
	cape = &(act->cape[type_idx]);
	return parse_actor_body_part (cape, cfg->children, "cape");
}

int parse_actor_head (actor_types *act, xmlNode *cfg) {
	int idx;
	body_part *head;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	idx = get_property (cfg, "number", "head number", head_number_dict);
	if (idx < 0) return 0;
	
	head = &(act->head[idx]);
	return parse_actor_body_part (head, cfg->children, "head");
}

int parse_actor_shield (actor_types *act, xmlNode *cfg) {
	int type_idx;
	body_part *shield;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	type_idx = get_property (cfg, "type", "shield type", shield_type_dict);
	if (type_idx < 0) return 0;
	
	shield = &(act->shield[type_idx]);
	return parse_actor_body_part (shield, cfg->children, "shield");
}

int parse_actor_hair (actor_types *act, xmlNode *cfg) {
	int col_idx;
	size_t len;
	char *buf;
	
	if (cfg == NULL || cfg->children == NULL) return 0;
	
	col_idx = get_property (cfg, "color", "hair color", hair_color_dict);
	if (col_idx < 0) return 0;
	
	buf = act->hair[col_idx].hair_name;
	len = sizeof (act->hair[col_idx].hair_name);
	get_string_value (buf, len, cfg);
	return 1;
}

int parse_actor_frames (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok = 1;

	if (cfg == NULL) return 0;
	
	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "walk") == 0) {
				get_string_value (act->walk_frame, sizeof (act->walk_frame), item);
			} else if (xmlStrcasecmp (item->name, "run") == 0) {
				get_string_value (act->run_frame, sizeof (act->run_frame), item);
			} else if (xmlStrcasecmp (item->name, "die1") == 0) {
				get_string_value (act->die1_frame, sizeof (act->die1_frame), item);
			} else if (xmlStrcasecmp (item->name, "die2") == 0) {
				get_string_value (act->die2_frame, sizeof (act->die2_frame), item);
			} else if (xmlStrcasecmp (item->name, "pain1") == 0) {
				get_string_value (act->pain1_frame, sizeof (act->pain1_frame), item);
			} else if (xmlStrcasecmp (item->name, "pain2") == 0) {
				get_string_value (act->pain2_frame, sizeof (act->pain2_frame), item);
			} else if (xmlStrcasecmp (item->name, "pick") == 0) {
				get_string_value (act->pick_frame, sizeof (act->pick_frame), item);
			} else if (xmlStrcasecmp (item->name, "drop") == 0) {
				get_string_value (act->drop_frame, sizeof (act->drop_frame), item);
			} else if (xmlStrcasecmp (item->name, "idle") == 0) {
				get_string_value (act->idle_frame, sizeof (act->idle_frame), item);
			} else if (xmlStrcasecmp (item->name, "idle_sit") == 0) {
				get_string_value (act->idle_sit_frame, sizeof (act->idle_sit_frame), item);
			} else if (xmlStrcasecmp (item->name, "harvest") == 0) {
				get_string_value (act->harvest_frame, sizeof (act->harvest_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_cast") == 0) {
				get_string_value (act->attack_cast_frame, sizeof (act->attack_cast_frame), item);
			} else if (xmlStrcasecmp (item->name, "sit_down") == 0) {
				get_string_value (act->sit_down_frame, sizeof (act->sit_down_frame), item);
			} else if (xmlStrcasecmp (item->name, "stand_up") == 0) {
				get_string_value (act->stand_up_frame, sizeof (act->stand_up_frame), item);
			} else if (xmlStrcasecmp (item->name, "in_combat") == 0) {
				get_string_value (act->in_combat_frame, sizeof (act->in_combat_frame), item);
			} else if (xmlStrcasecmp (item->name, "out_combat") == 0) {
				get_string_value (act->out_combat_frame, sizeof (act->out_combat_frame), item);
			} else if (xmlStrcasecmp (item->name, "combat_idle") == 0) {
				get_string_value (act->combat_idle_frame, sizeof (act->combat_idle_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_up_1") == 0) {
				get_string_value (act->attack_up_1_frame, sizeof (act->attack_up_1_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_up_2") == 0) {
				get_string_value (act->attack_up_2_frame, sizeof (act->attack_up_2_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_up_3") == 0) {
				get_string_value (act->attack_up_3_frame, sizeof (act->attack_up_3_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_up_4") == 0) {
				get_string_value (act->attack_up_4_frame, sizeof (act->attack_up_4_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_down_1") == 0) {
				get_string_value (act->attack_down_1_frame, sizeof (act->attack_down_1_frame), item);
			} else if (xmlStrcasecmp (item->name, "attack_down_2") == 0) {
				get_string_value (act->attack_down_2_frame, sizeof (act->attack_down_2_frame), item);
			} else {
				snprintf (errmsg, sizeof (errmsg), "unknown frame property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_boots (actor_types *act, xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, col_idx;
	boots_part *boots;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	col_idx = get_property (cfg, "color", "boots color", boots_color_dict);
	if (col_idx < 0) return 0;
	
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
				snprintf (errmsg, sizeof (errmsg), "unknown legs property \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
	}
	
	return ok;
}

int parse_actor_script (xmlNode *cfg) {
	xmlNode *item;
	char errmsg[120];
	int ok, act_idx;
	actor_types *act;

	if (cfg == NULL || cfg->children == NULL) return 0;
	
	act_idx = get_property (cfg, "type", "actor type", actor_type_dict);
	if (act_idx < 0) return 0;
	
	act = &(actors_defs[act_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, "ghost") == 0) {
				act->ghost = get_bool_value (item);
			} else if (xmlStrcasecmp (item->name, "skin") == 0) {
				get_string_value (act->skin_name, sizeof (act->skin_name), item);
			} else if (xmlStrcasecmp (item->name, "model") == 0) {
				get_string_value (act->file_name, sizeof (act->file_name), item);
			} else if (xmlStrcasecmp (item->name, "frames") == 0) {
				ok &= parse_actor_frames (act, item->children);
			} else if (xmlStrcasecmp (item->name, "shirt") == 0) {
				ok &= parse_actor_shirt (act, item);
			} else if (xmlStrcasecmp (item->name, "hskin") == 0) {
				ok &= parse_actor_skin (act, item);
			} else if (xmlStrcasecmp (item->name, "hair") == 0) {
				ok &= parse_actor_hair (act, item);
			} else if (xmlStrcasecmp (item->name, "boots") == 0) {
				ok &= parse_actor_boots (act, item);
			} else if (xmlStrcasecmp (item->name, "legs") == 0) {
				ok &= parse_actor_legs (act, item);
			} else if (xmlStrcasecmp (item->name, "cape") == 0) {
				ok &= parse_actor_cape (act, item);
			} else if (xmlStrcasecmp (item->name, "head") == 0) {
				ok &= parse_actor_head (act, item);
			} else if (xmlStrcasecmp (item->name, "shield") == 0) {
				ok &= parse_actor_shield (act, item);
			} else if (xmlStrcasecmp (item->name, "weapon") == 0) {
				ok &= parse_actor_weapon (act, item);
			} else if (xmlStrcasecmp (item->name, "helmet") == 0) {
				ok &= parse_actor_helmet (act, item);
			} else if (xmlStrcasecmp (item->name, "walk_speed") == 0) {
				act->walk_speed = get_float_value (item);
			} else if (xmlStrcasecmp (item->name, "run_speed") == 0) {
				act->run_speed = get_float_value (item);
			} else {
				snprintf (errmsg, sizeof (errmsg), "Unknown actor attribute \"%s\"", item->name);
				LOG_ERROR(errmsg);
				ok = 0;
			}
		}
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
	char errmsg[120], fname[120];
	int ok = 1;
	
	snprintf (fname, 120, "%s/%s", dir, index);
	
	doc = xmlReadFile (fname, NULL, 0);
	if (doc == NULL) {
		snprintf (errmsg, sizeof (errmsg), "Unable to read actor definition file %s", fname);
		LOG_ERROR(errmsg);
		return 0;
	}
	
	root = xmlDocGetRootElement (doc);
	if (root == NULL) {
		snprintf (errmsg, sizeof (errmsg), "Unable to parse actor definition file %s", fname);
		LOG_ERROR(errmsg);
		ok = 0;
	} else if (xmlStrcasecmp (root->name, "actors") != 0) {
		snprintf (errmsg, sizeof (errmsg), "Unknown key \"%s\" (\"actors\" expected).", root->name);
		LOG_ERROR(errmsg);
		ok = 0;
	} else {
		ok = parse_actor_defs (root);
	}
	
	xmlFreeDoc (doc);
	return ok;
}

void init_actor_defs () {
	const char *dirname = "actor_defs", *idxname = "actor_defs.xml";
	char defdir[120];
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
