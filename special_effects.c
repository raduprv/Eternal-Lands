#include <stdlib.h>
#include "global.h"

#ifdef SFX
//effect on player casting spell for all events *except* 
void add_special_effect_caster(int effect, Uint16 playerid)
{
	//temporary code just to show an effect
	actor *this_actor = get_actor_ptr_from_id(playerid);
	if (effect == SPECIAL_EFFECT_SMITE_SUMMONINGS)
	add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_SMITE_SUMMONINGS);
	else
	add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_HEAL_SUMMONED);
}

//second half of Player:Player casting
void add_special_effect_target(int effect, Uint16 playerid)
{	
	//temporary code just to show an effect
	actor *this_actor = get_actor_ptr_from_id(playerid);
	add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_SMITE_SUMMONINGS);
}

//special effects targeted to a particular location
void add_special_effect_location(int effect, Uint16 x, Uint16 y)
{
	//temporary code just to show an effect
	add_highlight(x,y, HIGHLIGHT_TYPE_WALKING_DESTINATION);
}

//send server data packet to appropriate method depending on desired effect
void parse_special_effect(int sfx, const Uint16 *data)
{
	Uint8 str[100];
	int offset = 0;
	Uint16 var_a, var_b =0;
	
	switch(sfx){
		//player only
		case	SPECIAL_EFFECT_SHIELD:
		case	SPECIAL_EFFECT_RESTORATION:
		case	SPECIAL_EFFECT_SMITE_SUMMONINGS:
		case	SPECIAL_EFFECT_CLOAK:
		case	SPECIAL_EFFECT_DECLOAK:
		case	SPECIAL_EFFECT_HEAL_SUMMONED:
		case	SPECIAL_EFFECT_HEAL:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				snprintf (str, sizeof (str), "effect %d,  player id= %d",sfx,var_a);
				add_special_effect_caster(sfx,var_a);
			}
			break;
		//player to player, var_a is caster, var_b is recipient/target
		case	SPECIAL_EFFECT_POISON:
		case	SPECIAL_EFFECT_REMOTE_HEAL:
		case	SPECIAL_EFFECT_HARM:
		case	SPECIAL_EFFECT_MANA_DRAIN:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				var_b = SDL_SwapLE16 (*((Uint16 *)(&data[offset+1])));
				snprintf (str, sizeof (str), "effect %d,  caster id=%d, target id=%d",sfx,var_a,var_b);
				add_special_effect_caster(sfx,var_a);
				add_special_effect_target(sfx,var_b);
			}
			break;
		//location (a&b variable are not known until implemented by server)
		case	SPECIAL_EFFECT_INVASION_BEAMING:
		case	SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				var_b = SDL_SwapLE16 (*((Uint16 *)(&data[offset+1])));
				snprintf (str, sizeof (str), "effect %d,  x pos=%d, y pos=%d",sfx,var_a,var_b);			
				add_special_effect_location(sfx,var_a,var_b);
			}
			break;
		default:
			snprintf (str, sizeof (str), " SPECIAL_EFFECT_unknown:%d",sfx);
			break;
	}
	LOG_TO_CONSOLE (c_purple2, str);
}
#endif //SFX