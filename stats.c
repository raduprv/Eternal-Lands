#include <stdlib.h>
#include <string.h>
#include "stats.h"
#include "asc.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "elwindows.h"
#include "errors.h"
#include "gl_init.h"
#include "hud_statsbar_window.h"
#include "knowledge.h"
#include "session.h"
#include "tabs.h"
#include "counters.h"
#include "eye_candy_wrapper.h"
#include "spells.h"

player_attribs your_info;
player_attribs someone_info;
struct attributes_struct attributes;

static int max_label_width = 0;
static int sep_width = 0;
static int max_lvl_width = 0;
static int max_exp_width = 0;
static int check_grid_y_top = 0;
static int check_grid_x_left = 0;
static int stats_y_step = 0;

int have_stats=0;

struct stats_struct statsinfo[NUM_SKILLS];

#define MAX_NUMBER_OF_FLOATING_MESSAGES 25

typedef struct {
        int actor_id;//The actor it's related to
        char message[50];
        int first_time;
        int active_time;
        int direction;
        short active;
        float color[3];
} floating_message;

floating_message floating_messages[MAX_NUMBER_OF_FLOATING_MESSAGES];

int floatingmessages_enabled = 1;

void floatingmessages_add_level(int actor_id, int level, const unsigned char * skillname);
void floatingmessages_compare_stat(int actor_id, int value, int new_value, const unsigned char *skillname);

void get_the_stats(Sint16 *stats, size_t len_in_bytes)
{
        have_stats=1;

        memset(&your_info, 0, sizeof(your_info));       // failsafe incase structure changes

        //initiate the function pointers
        init_attribf();

        if (len_in_bytes < 112 * sizeof(Sint16))
        {
            LOG_ERROR("Got incomplete stats data from the server");
            return;
        }

        your_info.phy.cur=SDL_SwapLE16(stats[0]);
        your_info.phy.base=SDL_SwapLE16(stats[1]);
        your_info.coo.cur=SDL_SwapLE16(stats[2]);
        your_info.coo.base=SDL_SwapLE16(stats[3]);
        your_info.rea.cur=SDL_SwapLE16(stats[4]);
        your_info.rea.base=SDL_SwapLE16(stats[5]);
        your_info.wil.cur=SDL_SwapLE16(stats[6]);
        your_info.wil.base=SDL_SwapLE16(stats[7]);
        your_info.ins.cur=SDL_SwapLE16(stats[8]);
        your_info.ins.base=SDL_SwapLE16(stats[9]);
        your_info.vit.cur=SDL_SwapLE16(stats[10]);
        your_info.vit.base=SDL_SwapLE16(stats[11]);

        your_info.human_nex.cur=SDL_SwapLE16(stats[12]);
        your_info.human_nex.base=SDL_SwapLE16(stats[13]);
        your_info.animal_nex.cur=SDL_SwapLE16(stats[14]);
        your_info.animal_nex.base=SDL_SwapLE16(stats[15]);
        your_info.vegetal_nex.cur=SDL_SwapLE16(stats[16]);
        your_info.vegetal_nex.base=SDL_SwapLE16(stats[17]);
        your_info.inorganic_nex.cur=SDL_SwapLE16(stats[18]);
        your_info.inorganic_nex.base=SDL_SwapLE16(stats[19]);
        your_info.artificial_nex.cur=SDL_SwapLE16(stats[20]);
        your_info.artificial_nex.base=SDL_SwapLE16(stats[21]);
        your_info.magic_nex.cur=SDL_SwapLE16(stats[22]);
        your_info.magic_nex.base=SDL_SwapLE16(stats[23]);

        your_info.manufacturing_skill.cur=SDL_SwapLE16(stats[24]);
        your_info.manufacturing_skill.base=SDL_SwapLE16(stats[25]);
        your_info.harvesting_skill.cur=SDL_SwapLE16(stats[26]);
        your_info.harvesting_skill.base=SDL_SwapLE16(stats[27]);
        your_info.alchemy_skill.cur=SDL_SwapLE16(stats[28]);
        your_info.alchemy_skill.base=SDL_SwapLE16(stats[29]);
        your_info.overall_skill.cur=SDL_SwapLE16(stats[30]);
        your_info.overall_skill.base=SDL_SwapLE16(stats[31]);
        your_info.attack_skill.cur=SDL_SwapLE16(stats[32]);
        your_info.attack_skill.base=SDL_SwapLE16(stats[33]);
        your_info.defense_skill.cur=SDL_SwapLE16(stats[34]);
        your_info.defense_skill.base=SDL_SwapLE16(stats[35]);
        your_info.magic_skill.cur=SDL_SwapLE16(stats[36]);
        your_info.magic_skill.base=SDL_SwapLE16(stats[37]);
        your_info.potion_skill.cur=SDL_SwapLE16(stats[38]);
        your_info.potion_skill.base=SDL_SwapLE16(stats[39]);
        your_info.carry_capacity.cur=SDL_SwapLE16(stats[40]);
        your_info.carry_capacity.base=SDL_SwapLE16(stats[41]);
        your_info.material_points.cur=SDL_SwapLE16(stats[42]);
        your_info.material_points.base=SDL_SwapLE16(stats[43]);
        your_info.ethereal_points.cur=SDL_SwapLE16(stats[44]);
        your_info.ethereal_points.base=SDL_SwapLE16(stats[45]);
        your_info.food_level=SDL_SwapLE16(stats[46]);

        your_info.manufacturing_exp=SDL_SwapLE32(*((Uint32 *)(stats+49)));
        your_info.manufacturing_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+51)));
        your_info.harvesting_exp=SDL_SwapLE32(*((Uint32 *)(stats+53)));
        your_info.harvesting_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+55)));
        your_info.alchemy_exp=SDL_SwapLE32(*((Uint32 *)(stats+57)));
        your_info.alchemy_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+59)));
        your_info.overall_exp=SDL_SwapLE32(*((Uint32 *)(stats+61)));
        your_info.overall_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+63)));
        your_info.attack_exp=SDL_SwapLE32(*((Uint32 *)(stats+65)));
        your_info.attack_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+67)));
        your_info.defense_exp=SDL_SwapLE32(*((Uint32 *)(stats+69)));
        your_info.defense_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+71)));
        your_info.magic_exp=SDL_SwapLE32(*((Uint32 *)(stats+73)));
        your_info.magic_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+75)));
        your_info.potion_exp=SDL_SwapLE32(*((Uint32 *)(stats+77)));
        your_info.potion_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+79)));

        your_info.summoning_skill.cur=SDL_SwapLE16(stats[83]);
        your_info.summoning_skill.base=SDL_SwapLE16(stats[84]);
        your_info.summoning_exp=SDL_SwapLE32(*((Uint32 *)(stats+85)));
        your_info.summoning_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+87)));
        your_info.crafting_skill.cur=SDL_SwapLE16(stats[89]);
        your_info.crafting_skill.base=SDL_SwapLE16(stats[90]);
        your_info.crafting_exp=SDL_SwapLE32(*((Uint32 *)(stats+91)));
        your_info.crafting_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+93)));
        your_info.engineering_skill.cur=SDL_SwapLE16(stats[95]);
        your_info.engineering_skill.base=SDL_SwapLE16(stats[96]);
        your_info.engineering_exp=SDL_SwapLE32(*((Uint32 *)(stats+97)));
        your_info.engineering_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+99)));
        your_info.tailoring_skill.cur=SDL_SwapLE16(stats[101]);
        your_info.tailoring_skill.base=SDL_SwapLE16(stats[102]);
        your_info.tailoring_exp=SDL_SwapLE32(*((Uint32 *)(stats+103)));
        your_info.tailoring_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+105)));
        your_info.ranging_skill.cur=SDL_SwapLE16(stats[107]);
        your_info.ranging_skill.base=SDL_SwapLE16(stats[108]);
        your_info.ranging_exp=SDL_SwapLE32(*((Uint32 *)(stats+109)));
        your_info.ranging_exp_next_lev=SDL_SwapLE32(*((Uint32 *)(stats+111)));

        your_info.research_completed=SDL_SwapLE16(stats[47]);
        your_info.researching=SDL_SwapLE16(stats[81]);
        your_info.research_total=SDL_SwapLE16(stats[82]);
        check_book_known();

        // can be removed test when we change protocol number for 1.9.2
        if (len_in_bytes <= 2*114)
        {
                your_info.action_points.cur=0;
                your_info.action_points.base=0;
        }
        else
        {
                your_info.action_points.cur=SDL_SwapLE16(stats[113]);
                your_info.action_points.base=SDL_SwapLE16(stats[114]);
        }

        init_session();
        check_castability();
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
                        floatingmessages_add_level(yourself, value, attributes.manufacturing_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_man_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_man_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.manufacturing_skill.base=value;break;
                case HARV_S_CUR:
                        your_info.harvesting_skill.cur=value;break;
                case HARV_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.harvesting_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_har(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.harvesting_skill.base=value;break;
                case ALCH_S_CUR:
                        your_info.alchemy_skill.cur=value;break;
                case ALCH_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.alchemy_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_alc_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_alc_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.alchemy_skill.base=value;break;
                case OVRL_S_CUR:
                        your_info.overall_skill.cur=value;break;
                case OVRL_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.overall_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_oa(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.overall_skill.base=value;break;
                case ATT_S_CUR:
                        your_info.attack_skill.cur=value;break;
                case ATT_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.attack_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_att(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.attack_skill.base=value;break;
                case DEF_S_CUR:
                        your_info.defense_skill.cur=value;break;
                case DEF_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.defense_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_def(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.defense_skill.base=value;break;
                case MAG_S_CUR:
                        your_info.magic_skill.cur=value;break;
                case MAG_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.magic_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_mag(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.magic_skill.base=value;break;
                case POT_S_CUR:
                        your_info.potion_skill.cur=value;break;
                case POT_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.potion_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_pot_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_pot_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
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
                        {
                                char str[5];

                                safe_snprintf(str, sizeof(str), "%d", value-your_info.ethereal_points.cur);
                                add_floating_message(yourself, str, FLOATINGMESSAGE_MIDDLE, 0.3, 0.3, 1.0, 1500);
                                your_info.ethereal_points.cur=value;
                                break;
                        }
                case ETH_POINT_BASE:
                        your_info.ethereal_points.base=value;break;
                case ACTION_POINTS_CUR:
                        {
                                char str[5];
                                safe_snprintf(str, sizeof(str), "%d", value-your_info.action_points.cur);
                                add_floating_message(yourself, str, FLOATINGMESSAGE_MIDDLE, 1.0, 0.0, 1.0, 2500);
                                your_info.action_points.cur=value;
                                break;
                        }
                case ACTION_POINTS_BASE:
                        your_info.action_points.base=value;break;
                case FOOD_LEV:
                        your_info.food_level=value;break;
                case MAN_EXP:
                        floatingmessages_compare_stat(yourself, your_info.manufacturing_exp, value, attributes.manufacturing_skill.shortname);
                        set_last_skill_exp(SI_MAN, value-your_info.manufacturing_exp);
                        increment_manufacturing_counter();
                        your_info.manufacturing_exp=value;
                        break;
                case MAN_EXP_NEXT:
                        your_info.manufacturing_exp_next_lev=value;break;
                case HARV_EXP:
                        set_last_skill_exp(SI_HAR, value-your_info.harvesting_exp);
						floatingmessages_compare_stat(yourself, your_info.harvesting_exp, value, attributes.harvesting_skill.shortname);
                        your_info.harvesting_exp=value;
                        break;
                case HARV_EXP_NEXT:
                        your_info.harvesting_exp_next_lev=value;break;
                case ALCH_EXP:
                        set_last_skill_exp(SI_ALC, value-your_info.alchemy_exp);
                        floatingmessages_compare_stat(yourself, your_info.alchemy_exp, value, attributes.alchemy_skill.shortname);
                        increment_alchemy_counter();
                        your_info.alchemy_exp=value;
                        break;
                case ALCH_EXP_NEXT:
                        your_info.alchemy_exp_next_lev=value;break;
                case OVRL_EXP:
                        set_last_skill_exp(SI_ALL, value-your_info.overall_exp);
                        your_info.overall_exp=value;break;
                case OVRL_EXP_NEXT:
                        your_info.overall_exp_next_lev=value;break;
                case DEF_EXP:
                        set_last_skill_exp(SI_DEF, value-your_info.defense_exp);
                        floatingmessages_compare_stat(yourself, your_info.defense_exp, value, attributes.defense_skill.shortname);
                        your_info.defense_exp=value;
                        break;
                case DEF_EXP_NEXT:
                        your_info.defense_exp_next_lev=value;break;
                case ATT_EXP:
                        set_last_skill_exp(SI_ATT, value-your_info.attack_exp);
                        floatingmessages_compare_stat(yourself, your_info.attack_exp, value, attributes.attack_skill.shortname);
                        your_info.attack_exp=value;
                        break;
                case ATT_EXP_NEXT:
                        your_info.attack_exp_next_lev=value;break;
                case MAG_EXP:
                        set_last_skill_exp(SI_MAG, value-your_info.magic_exp);
                        floatingmessages_compare_stat(yourself, your_info.magic_exp, value, attributes.magic_skill.shortname);
                        your_info.magic_exp=value;
                        break;
                case MAG_EXP_NEXT:
                        your_info.magic_exp_next_lev=value;break;
                case POT_EXP:
                        set_last_skill_exp(SI_POT, value-your_info.potion_exp);
                        floatingmessages_compare_stat(yourself, your_info.potion_exp, value, attributes.potion_skill.shortname);
                        increment_potions_counter();
                        your_info.potion_exp=value;
                        break;
                case POT_EXP_NEXT:
                        your_info.potion_exp_next_lev=value;break;
                case SUM_EXP:
                        set_last_skill_exp(SI_SUM, value-your_info.summoning_exp);
                        floatingmessages_compare_stat(yourself, your_info.summoning_exp, value, attributes.summoning_skill.shortname);
                        increment_summon_manu_counter();
                        your_info.summoning_exp=value;
                        break;
                case SUM_EXP_NEXT:
                        your_info.summoning_exp_next_lev=value;break;
                case SUM_S_CUR:
                        your_info.summoning_skill.cur=value;break;
                case SUM_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.summoning_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_sum(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.summoning_skill.base=value;break;
                case CRA_EXP:
                        set_last_skill_exp(SI_CRA, value-your_info.crafting_exp);
                        floatingmessages_compare_stat(yourself, your_info.crafting_exp, value, attributes.crafting_skill.shortname);
                        increment_crafting_counter();
                        your_info.crafting_exp=value;
                        break;
                case CRA_EXP_NEXT:
                        your_info.crafting_exp_next_lev=value;break;
                case CRA_S_CUR:
                        your_info.crafting_skill.cur=value;break;
                case CRA_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.crafting_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_cra_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_cra_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.crafting_skill.base=value;break;
                case ENG_EXP:
                        set_last_skill_exp(SI_ENG, value-your_info.engineering_exp);
                        floatingmessages_compare_stat(yourself, your_info.engineering_exp, value, attributes.engineering_skill.shortname);
                        increment_engineering_counter();
                        your_info.engineering_exp=value;
                        break;
                case ENG_EXP_NEXT:
                        your_info.engineering_exp_next_lev=value;break;
                case ENG_S_CUR:
                        your_info.engineering_skill.cur=value;break;
                case ENG_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.engineering_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_eng_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_eng_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.engineering_skill.base=value;break;
                case TAIL_EXP:
                        set_last_skill_exp(SI_TAI, value-your_info.tailoring_exp);
                        floatingmessages_compare_stat(yourself, your_info.tailoring_exp, value, attributes.tailoring_skill.shortname);
                        increment_tailoring_counter();
                        your_info.tailoring_exp=value;
                        break;
                case TAIL_EXP_NEXT:
                        your_info.tailoring_exp_next_lev=value;break;
                case TAIL_S_CUR:
                        your_info.tailoring_skill.cur=value;break;
                case TAIL_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.tailoring_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_tai_left(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_tai_right(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.tailoring_skill.base=value;break;

                case RANG_EXP:
                        set_last_skill_exp(SI_RAN, value-your_info.ranging_exp);
                        floatingmessages_compare_stat(yourself, your_info.ranging_exp, value, attributes.ranging_skill.shortname);
                        your_info.ranging_exp=value;
                        break;
                case RANG_EXP_NEXT:
                        your_info.ranging_exp_next_lev=value;break;
                case RANG_S_CUR:
                        your_info.ranging_skill.cur=value;break;
                case RANG_S_BASE:
                        floatingmessages_add_level(yourself, value, attributes.ranging_skill.name);
                        {
                            actor *_actor = get_actor_ptr_from_id(yourself);
                            if (use_eye_candy == 1 && _actor != NULL) {
                  	          ec_create_glow_level_up_default(_actor, (poor_man ? 6 : 10));
                  	          ec_create_glow_level_up_ran(_actor, (poor_man ? 6 : 10));
                            }
                        }
                        your_info.ranging_skill.base=value;break;
                case RESEARCHING:
                        your_info.researching=value; check_book_known(); break;
                case RESEARCH_COMPLETED:
                        your_info.research_completed=value; check_book_known(); break;
                case RESEARCH_TOTAL:
                        your_info.research_total=value; check_book_known(); break;
                default:
                        LOG_ERROR("Server sent invalid stat number\n");
                }
                //update spells
                //this must be here, atm spells depend on mana, magic level and alchemy (bones to gold)
                //but in the future they could involve other attributes/skills
                check_castability();

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
Sint16 get_cur_dext() { return (your_info.coo.cur+your_info.rea.cur)/2;}

Sint16 get_base_eth() { return (your_info.wil.base+your_info.vit.base)/2;}
Sint16 get_cur_eth() { return (your_info.wil.cur+your_info.vit.cur)/2;}


/* store references to the skills info in an easy to use array */
void init_statsinfo_array(void)
{
	statsinfo[SI_ATT].exp = &your_info.attack_exp;
	statsinfo[SI_ATT].next_lev = &your_info.attack_exp_next_lev;
	statsinfo[SI_ATT].skillattr = &your_info.attack_skill;
	statsinfo[SI_ATT].skillnames = &attributes.attack_skill;

	statsinfo[SI_DEF].exp = &your_info.defense_exp;
	statsinfo[SI_DEF].next_lev = &your_info.defense_exp_next_lev;
	statsinfo[SI_DEF].skillattr = &your_info.defense_skill;
	statsinfo[SI_DEF].skillnames = &attributes.defense_skill;

	statsinfo[SI_HAR].exp = &your_info.harvesting_exp;
	statsinfo[SI_HAR].next_lev = &your_info.harvesting_exp_next_lev;
	statsinfo[SI_HAR].skillattr = &your_info.harvesting_skill;
	statsinfo[SI_HAR].skillnames = &attributes.harvesting_skill;

	statsinfo[SI_ALC].exp = &your_info.alchemy_exp;
	statsinfo[SI_ALC].next_lev = &your_info.alchemy_exp_next_lev;
	statsinfo[SI_ALC].skillattr = &your_info.alchemy_skill;
	statsinfo[SI_ALC].skillnames = &attributes.alchemy_skill;

	statsinfo[SI_MAG].exp = &your_info.magic_exp;
	statsinfo[SI_MAG].next_lev = &your_info.magic_exp_next_lev;
	statsinfo[SI_MAG].skillattr = &your_info.magic_skill;
	statsinfo[SI_MAG].skillnames = &attributes.magic_skill;

	statsinfo[SI_POT].exp = &your_info.potion_exp;
	statsinfo[SI_POT].next_lev = &your_info.potion_exp_next_lev;
	statsinfo[SI_POT].skillattr = &your_info.potion_skill;
	statsinfo[SI_POT].skillnames = &attributes.potion_skill;

	statsinfo[SI_SUM].exp = &your_info.summoning_exp;
	statsinfo[SI_SUM].next_lev = &your_info.summoning_exp_next_lev;
	statsinfo[SI_SUM].skillattr = &your_info.summoning_skill;
	statsinfo[SI_SUM].skillnames = &attributes.summoning_skill;

	statsinfo[SI_MAN].exp = &your_info.manufacturing_exp;
	statsinfo[SI_MAN].next_lev = &your_info.manufacturing_exp_next_lev;
	statsinfo[SI_MAN].skillattr = &your_info.manufacturing_skill;
	statsinfo[SI_MAN].skillnames = &attributes.manufacturing_skill;

	statsinfo[SI_CRA].exp = &your_info.crafting_exp;
	statsinfo[SI_CRA].next_lev = &your_info.crafting_exp_next_lev;
	statsinfo[SI_CRA].skillattr = &your_info.crafting_skill;
	statsinfo[SI_CRA].skillnames = &attributes.crafting_skill;

	statsinfo[SI_ENG].exp = &your_info.engineering_exp;
	statsinfo[SI_ENG].next_lev = &your_info.engineering_exp_next_lev;
	statsinfo[SI_ENG].skillattr = &your_info.engineering_skill;
	statsinfo[SI_ENG].skillnames = &attributes.engineering_skill;

	statsinfo[SI_TAI].exp = &your_info.tailoring_exp;
	statsinfo[SI_TAI].next_lev = &your_info.tailoring_exp_next_lev;
	statsinfo[SI_TAI].skillattr = &your_info.tailoring_skill;
	statsinfo[SI_TAI].skillnames = &attributes.tailoring_skill;

	statsinfo[SI_RAN].exp = &your_info.ranging_exp;
	statsinfo[SI_RAN].next_lev = &your_info.ranging_exp_next_lev;
	statsinfo[SI_RAN].skillattr = &your_info.ranging_skill;
	statsinfo[SI_RAN].skillnames = &attributes.ranging_skill;

	/* always make last as special case for skills modifiers - and best displayed last anyway */
	statsinfo[SI_ALL].exp = &your_info.overall_exp;
	statsinfo[SI_ALL].next_lev = &your_info.overall_exp_next_lev;
	statsinfo[SI_ALL].skillattr = &your_info.overall_skill;
	statsinfo[SI_ALL].skillnames = &attributes.overall_skill;
}


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

static void draw_stat(const window_info *win, int x, int y, const attrib_16 *var,
	const names *name)
{
	int x_mid_var = x + max_label_width + sep_width + max_lvl_width / 2;
	char buf[10];
	draw_string_small_zoomed(x, y, name->name, 1, win->current_scale);
	safe_snprintf(buf, sizeof(buf), "%3d / %d", var->cur, var->base);
	draw_string_small_zoomed_centered_around(x_mid_var, y, (const unsigned char*)buf,
		4, win->current_scale);
}

static void draw_skill(const window_info *win, int x, int y, const attrib_16 *lvl,
	const names *name, Uint32 exp, Uint32 exp_next)
{
	int x_mid_lvl = x + max_label_width + sep_width + max_lvl_width / 2;
	int x_mid_exp = x + max_label_width + sep_width + max_lvl_width + sep_width + max_exp_width / 2;
	char buf[25];

	draw_string_small_zoomed(x, y, name->name, 1, win->current_scale);
	safe_snprintf(buf, sizeof(buf), "%3d / %d", lvl->cur, lvl->base);
	draw_string_small_zoomed_centered_around(x_mid_lvl, y, (const unsigned char*)buf,
		4, win->current_scale);
	safe_snprintf(buf, sizeof(buf), "%10u / %u", exp, exp_next);
	draw_string_small_zoomed_centered_around(x_mid_exp, y, (const unsigned char*)buf,
		11, win->current_scale);
}

static void draw_statf(const window_info *win, int x, int y, const attrib_16f *var,
	const names *name)
{
	int x_mid_var = x + max_label_width + sep_width + max_lvl_width / 2;
	char buf[12];

	draw_string_small_zoomed(x, y, name->name, 1, win->current_scale);
	safe_snprintf(buf, sizeof(buf),"%3d / %d", var->cur(), var->base());
	draw_string_small_zoomed_centered_around(x_mid_var, y, (const unsigned char*)buf,
		4, win->current_scale);
}

static void draw_stat_single(const window_info *win, int x, int y,
	const unsigned char* name, const char* value)
{
	int x_mid = x + max_label_width + sep_width + max_lvl_width / 2;
	draw_string_small_zoomed(x, y, name, 1, win->current_scale);
	draw_string_small_zoomed_centered(x_mid, y, (const unsigned char*) value, 1, win->current_scale);
}

static void set_content_widths(window_info *win)
{
	int width;

	max_label_width = 0;
	width = get_string_width_zoom(attributes.phy.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.coo.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.rea.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.wil.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.ins.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.vit.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.might.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.matter.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.tough.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.charm.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.react.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.perc.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.ration.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.dext.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.eth.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.material_points.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.ethereal_points.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.action_points.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.food.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.human_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.animal_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.vegetal_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.inorganic_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.artificial_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.magic_nex.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.attack_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.defense_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.harvesting_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.alchemy_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.magic_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.potion_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.summoning_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.manufacturing_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.crafting_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.engineering_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.tailoring_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.ranging_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.overall_skill.name, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);
	width = get_string_width_zoom(attributes.pickpoints, win->font_category,
		win->current_scale_small);
	max_label_width = max2i(max_label_width, width);

	sep_width = 2 * win->small_font_max_len_x;
	max_lvl_width = get_string_width_zoom((const unsigned char*)"888 / 888",
		win->font_category, win->current_scale_small);
	max_exp_width = get_string_width_zoom((const unsigned char*)"888888888 / 888888888",
		win->font_category, win->current_scale_small);

	win->min_len_x = 1.5 * win->small_font_max_len_x + 2 * max_label_width + 2 * max_lvl_width
		+ max_exp_width + 5 * sep_width;
	win->min_len_y = 24 * win->small_font_len_y;
}

int display_stats_handler(window_info *win)
{
        player_attribs cur_stats = your_info;
        char str[10];
        int x = win->small_font_max_len_x / 2;
        int c2_x_offset = max_label_width + max_lvl_width + 3 * sep_width;
        int start_y = (int)(0.5 + win->small_font_len_y / 2);
        int y = start_y;
        int y_gap_step = (int)(0.5 + win->small_font_len_y * 1.5);

        stats_y_step = win->small_font_len_y;

        draw_string_small_zoomed(x,y,attributes.base,1, win->current_scale);
        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.phy),&(attributes.phy));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.coo),&(attributes.coo));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.rea),&(attributes.rea));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.wil),&(attributes.wil));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.ins),&(attributes.ins));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.vit),&(attributes.vit));

        //cross attributes
        glColor3f(1.0f,1.0f,0.0f);
        y+=y_gap_step;

        draw_string_small_zoomed(x,y,attributes.cross,1, win->current_scale);
        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.might),&(attributes.might));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.matter),&(attributes.matter));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.tough),&(attributes.tough));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.charm),&(attributes.charm));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.react),&(attributes.react));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.perc),&(attributes.perc));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.ration),&(attributes.ration));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.dext),&(attributes.dext));

        y+=stats_y_step;
        draw_statf(win,x,y,&(cur_stats.eth),&(attributes.eth));

        // draw the point attributes
        glColor3f(0.5f,0.5f,1.0f);

        y+=y_gap_step;
        draw_stat(win,x,y,&(cur_stats.material_points),&(attributes.material_points));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.ethereal_points),&(attributes.ethereal_points));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.action_points),&(attributes.action_points));

        //other info
        y = win->len_y - win->small_font_len_y * 1.25;
        safe_snprintf(str, sizeof(str), "%3i",cur_stats.food_level);
        draw_stat_single(win,x,y,attributes.food.name,str);

        safe_snprintf(str, sizeof(str), "%3i",cur_stats.overall_skill.base-cur_stats.overall_skill.cur);
        draw_stat_single(win,x+c2_x_offset,y,attributes.pickpoints,str);

        //nexuses here
        glColor3f(1.0f,1.0f,1.0f);
        x+=c2_x_offset;
        y=start_y;

        draw_string_small_zoomed(x,y,attributes.nexus,1, win->current_scale);

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.human_nex),&(attributes.human_nex));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.animal_nex),&(attributes.animal_nex));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.vegetal_nex),&(attributes.vegetal_nex));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.inorganic_nex),&(attributes.inorganic_nex));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.artificial_nex),&(attributes.artificial_nex));

        y+=stats_y_step;
        draw_stat(win,x,y,&(cur_stats.magic_nex),&(attributes.magic_nex));

        y+=y_gap_step;
        //skills
        glColor3f(1.0f,0.5f,0.2f);
        draw_string_small_zoomed(x,y,attributes.skills,1, win->current_scale);

        y+=stats_y_step;

        check_grid_x_left=x;
        check_grid_y_top=y;

        statsinfo[0].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.attack_skill),&(attributes.attack_skill),cur_stats.attack_exp,cur_stats.attack_exp_next_lev);

        y+=stats_y_step;
        statsinfo[1].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.defense_skill),&(attributes.defense_skill),cur_stats.defense_exp,cur_stats.defense_exp_next_lev);

        y+=stats_y_step;
        statsinfo[2].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.harvesting_skill),&(attributes.harvesting_skill),cur_stats.harvesting_exp,cur_stats.harvesting_exp_next_lev);

        y+=stats_y_step;
        statsinfo[3].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.alchemy_skill),&(attributes.alchemy_skill),cur_stats.alchemy_exp,cur_stats.alchemy_exp_next_lev);

        y+=stats_y_step;
        statsinfo[4].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.magic_skill),&(attributes.magic_skill),cur_stats.magic_exp,cur_stats.magic_exp_next_lev);

        y+=stats_y_step;
        statsinfo[5].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.potion_skill),&(attributes.potion_skill),cur_stats.potion_exp,cur_stats.potion_exp_next_lev);

        y+=stats_y_step;
        statsinfo[6].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.summoning_skill),&(attributes.summoning_skill),cur_stats.summoning_exp,cur_stats.summoning_exp_next_lev);

        y+=stats_y_step;
        statsinfo[7].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.manufacturing_skill),&(attributes.manufacturing_skill),cur_stats.manufacturing_exp,cur_stats.manufacturing_exp_next_lev);

        y+=stats_y_step;
        statsinfo[8].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.crafting_skill),&(attributes.crafting_skill),cur_stats.crafting_exp,cur_stats.crafting_exp_next_lev);

        y+=stats_y_step;
        statsinfo[9].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.engineering_skill),&(attributes.engineering_skill),cur_stats.engineering_exp,cur_stats.engineering_exp_next_lev);

        y+=stats_y_step;
        statsinfo[10].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.tailoring_skill),&(attributes.tailoring_skill),cur_stats.tailoring_exp,cur_stats.tailoring_exp_next_lev);

        y+=stats_y_step;
        statsinfo[11].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.ranging_skill),&(attributes.ranging_skill),cur_stats.ranging_exp,cur_stats.ranging_exp_next_lev);

        y+=stats_y_step;
        statsinfo[12].is_selected==1?glColor3f(1.0f,0.5f,0.5f):glColor3f(1.0f,0.5f,0.2f);
        draw_skill(win,x,y,&(cur_stats.overall_skill),&(attributes.overall_skill),cur_stats.overall_exp,cur_stats.overall_exp_next_lev);

        return 1;
}

int click_stats_handler(window_info *win, int mx, int my, Uint32 flags)
{
        int     i;
        int is_button = flags & ELW_MOUSE_BUTTON;

        if(is_button && mx > check_grid_x_left && mx < win->len_x && my > check_grid_y_top && my < check_grid_y_top+stats_y_step*(NUM_WATCH_STAT-1))
        {
                // we don't care which click did the select
                // Grum: as long as it's not a wheel move
                i = 1+(my - check_grid_y_top)/stats_y_step;
                if (i < NUM_WATCH_STAT)
                {
                        handle_stats_selection(i, flags);
                }
                return 1;
        }

        return 0;
}

static int ui_scale_stats_handler(window_info* win)
{
	set_content_widths(win);
	return 1;
}

static int change_stats_font_handler(window_info *win, font_cat font)
{
	if (font != UI_FONT)
		return 0;
	set_content_widths(win);
	return 1;
}

void fill_stats_win (int window_id)
{
	//set_window_color(window_id, ELW_COLOR_BORDER, 0.0f, 1.0f, 0.0f, 0.0f);
	set_window_custom_scale(window_id, MW_STATS);
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_stats_handler );
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_stats_handler );
	set_window_handler(window_id, ELW_HANDLER_UI_SCALE, &ui_scale_stats_handler);
	set_window_handler(window_id, ELW_HANDLER_FONT_CHANGE, &change_stats_font_handler);

	if (window_id >= 0 && window_id < windows_list.num_windows)
		set_content_widths(&windows_list.window[window_id]);
}

void draw_floatingmessage(floating_message *message, float healthbar_z) {
        float cut;
        double f, width, y, x, z;
        double model[16],proj[16];
        int view[4];

        if(!message)return;

        cut=message->active_time/4000.0f;
        f = ((float)(message->active_time-(cur_time-message->first_time)))/message->active_time;
        glColor4f(message->color[0], message->color[1], message->color[2], f > cut ? 1.0f : (f / cut));
		width = (float)get_string_width_zoom((unsigned char*)message->message, NAME_FONT,
			INGAME_FONT_X_LEN * 8.0);

        //Figure out where the point just above the actor's head is in the viewport
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, view);
        if (first_person)
        {
			x=window_width/2.0;
			y=window_height/2.0-40.0;
        }
        else
        {
			gluProject(0.0, 0.0, healthbar_z * get_actor_scale(your_actor), model, proj, view, &x, &y, &z);
			y += 50*font_scales[NAME_FONT]; // size of the actor name/bar
        }


        switch(message->direction){
                case FLOATINGMESSAGE_EAST:
                        x+=-width/2.0f-(f*window_width*0.1f);
                        y+=window_height*0.1;
                        break;
                case FLOATINGMESSAGE_SOUTH:
                        x+=-width/2.0f;
                        y+=f*window_height*0.1;
                        break;
                case FLOATINGMESSAGE_WEST:
                        x+=-width/2.0f+(f*window_width*0.1f);
                        y+=window_height*0.1;
                        break;
                case FLOATINGMESSAGE_MIDDLE:
                        x+=-width/2.0f;
                        y+=f*window_height*0.05f;
                        break;
                case FLOATINGMESSAGE_NORTH:
                default:
                        x+=-width/2.0f;
                        y+=(1.0-f)*window_height*0.1;
                        break;
        }

        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(view[0],view[2]+view[0],view[1],view[3]+view[1],0.0f,-1.0f);

        draw_ortho_ingame_string(x, y, 0, (unsigned char*)message->message, 1,
			INGAME_FONT_X_LEN*8.0, INGAME_FONT_X_LEN*8.0);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

}

void drawactor_floatingmessages(int actor_id, float healthbar_z) {
        int i;

        if (actor_id < 0) return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glDisable(GL_DEPTH_TEST);
        for(i = 0; i < MAX_NUMBER_OF_FLOATING_MESSAGES; i++) {
                if(floating_messages[i].active){
                        if(floating_messages[i].first_time<=cur_time){
                                if(floating_messages[i].first_time+floating_messages[i].active_time<cur_time){
                                        floating_messages[i].active=0;
                                } else if(floating_messages[i].actor_id==actor_id)
                                        draw_floatingmessage(&floating_messages[i], healthbar_z);
                        }
                }
        }
        glEnable(GL_DEPTH_TEST);

        for(i = 0; i < MAX_NUMBER_OF_FLOATING_MESSAGES; i++) {
                if(floating_messages[i].active){
                        if(floating_messages[i].first_time<=cur_time){
                                if(floating_messages[i].first_time+floating_messages[i].active_time<cur_time){
                                        floating_messages[i].active=0;
                                } else if(floating_messages[i].actor_id==actor_id)
                                        draw_floatingmessage(&floating_messages[i], healthbar_z);
                        }
                }
        }

        glDisable(GL_BLEND);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

floating_message *get_free_floatingmessage() {
        int i;
        for(i = 0; i < MAX_NUMBER_OF_FLOATING_MESSAGES; i++) {
                if (floating_messages[i].active == 0)
                        return &floating_messages[i];
        }
        return NULL;
}

void add_floating_message(int actor_id, char * str, int direction, float r, float g, float b, int active_time)
{
        int time;
        static int last_time_added[5]={0};
        static int last_direction_added[5]={0};
        static int last_actor[5]={0};//Make sure that we don't see too many messages from that actor
        floating_message *m=get_free_floatingmessage();

        if(!m) return;

        m->color[0]=r;
        m->color[1]=g;
        m->color[2]=b;

        safe_snprintf(m->message, sizeof(m->message), "%s", str);

        m->direction=direction;
        m->active_time=active_time;

        if(last_actor[4]==actor_id && last_time_added[4]+550>cur_time && last_direction_added[4]==direction)
                time=m->first_time=last_time_added[4]+550;
        else if(last_actor[3]==actor_id && last_time_added[3]+550>cur_time && last_direction_added[3]==direction)
                time=m->first_time=last_time_added[3]+550;
        else if(last_actor[2]==actor_id && last_time_added[2]+550>cur_time && last_direction_added[2]==direction)
                time=m->first_time=last_time_added[2]+550;
        else if(last_actor[1]==actor_id && last_time_added[1]+550>cur_time && last_direction_added[1]==direction)
                time=m->first_time=last_time_added[1]+550;
        else if(last_actor[0]==actor_id && last_time_added[0]+550>cur_time && last_direction_added[0]==direction)
                time=m->first_time=last_time_added[0]+550;
        else
                time=m->first_time=cur_time;

        m->active=1;
        m->actor_id=actor_id;

        last_actor[0]=last_actor[1];
        last_time_added[0]=last_time_added[1];
        last_direction_added[0]=last_direction_added[1];
        last_actor[1]=last_actor[2];
        last_time_added[1]=last_time_added[2];
        last_direction_added[1]=last_direction_added[2];
        last_actor[2]=last_actor[3];
        last_time_added[2]=last_time_added[3];
        last_direction_added[2]=last_direction_added[3];
        last_actor[3]=last_actor[4];
        last_time_added[3]=last_time_added[4];
        last_direction_added[3]=last_direction_added[4];
        last_actor[4]=actor_id;
        last_time_added[4]=time;
        last_direction_added[4]=direction;
}

void floatingmessages_add_level(int actor_id, int level, const unsigned char * skillname)
{
        char str[50];

        safe_snprintf(str,sizeof(str),"%d %s",level, skillname);
        add_floating_message(actor_id, str, FLOATINGMESSAGE_NORTH, 0.3, 0.3, 1.0, 2000);
}

void floatingmessages_compare_stat(int actor_id, int value, int new_value, const unsigned char *skillname)
{
        char str[50];
        int diff=new_value-value;

        safe_snprintf(str, sizeof(str), "%s: %c%d", skillname, diff<0?' ':'+', diff);

        if(diff<0)
                add_floating_message(actor_id, str, FLOATINGMESSAGE_SOUTH, 1.0, 0.3, 0.3,1500);
        else
                add_floating_message(actor_id, str, FLOATINGMESSAGE_NORTH, 0.3, 1.0, 0.3,1500);
}
