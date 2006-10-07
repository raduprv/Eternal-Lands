#include <stdlib.h>
#include "global.h"
#include "highlight.h"

#ifdef SFX
//much of this is based on the highlight.c code
#define SPECIAL_EFFECT_LIFESPAN	(500)
#define NUMBER_OF_SPECIAL_EFFECTS	(10)

typedef struct {
	short x;
	short y;
	int timeleft;
	int type;
	int active;
	int caster;		//is this caster or target
} special_effect;

special_effect sfx_markers[NUMBER_OF_SPECIAL_EFFECTS];

int sfx_enabled = 1;

special_effect *get_free_special_effect(short x, short y) {
	int i;
	//first try to find one that already occupies this tile
	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (sfx_markers[i].active && sfx_markers[i].x == x && sfx_markers[i].y == y) {
			return &sfx_markers[i];
		}
	}
	//otherwise, find the first free slot
	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (!sfx_markers[i].active) {
			return &sfx_markers[i];
		}
	}
	return NULL;
}

//effect on player casting spell for all events *except* 
void add_sfx(int effect, Uint16 playerid, int caster)
{
	actor *this_actor = get_actor_ptr_from_id(playerid);
	special_effect *m = get_free_special_effect(this_actor->x_tile_pos, this_actor->y_tile_pos);
	if (m == NULL) return;
	
	m->x = this_actor->x_tile_pos;
	m->y = this_actor->y_tile_pos;
	m->type = effect;
	m->timeleft = SPECIAL_EFFECT_LIFESPAN;
	m->active = 1;
	m->caster = caster;
}

void display_special_effect(const special_effect *marker) {
	// (a) varies from 1..0 depending on the age of this marker
	const float a = ((float)marker->timeleft) / SPECIAL_EFFECT_LIFESPAN;
	
	//the -11 is a fudge factor to take into account the height of actor occupying
	// the tile.  This may need to be adjusted for non-elves.
	float z = get_tile_display_height(marker->x, marker->y) - 11;
	
	float x = (float)marker->x/2;
	float y = (float)marker->y/2;
	
	/*
		For more details on the below effect, look at 
		display_highlight_marker in highlight.c
		*/
	
	const float dx = (TILESIZE_X / 6);
	const float dy = (TILESIZE_Y / 6);
	
	float center_offset_x, center_offset_y;

	// place x,y in the center of the actor's tile
	x += (TILESIZE_X / 2);
	y += (TILESIZE_Y / 2);
	
	switch (marker->type) {
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));	//fast expanding
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			z += a*0.3f;									//drop toward ground
			glColor4f(1.0f, 0.0f, 0.0f, a);
			break;
		case SPECIAL_EFFECT_HEAL_SUMMONED:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			z += a*0.3f;									//drop toward ground
			glColor4f(0.0f, 0.0f, 1.0f, a);
			break;
		case SPECIAL_EFFECT_HEAL:
			center_offset_x = ((TILESIZE_X / 2) * (a*a));	//shrinking
			center_offset_y = ((TILESIZE_X / 2) * (a*a));
			if (a > 0) z += 1.5/(a+.5) - 1;					//beam up effect
			glColor4f(1.0f, 1.0f, 1.0f, a);
			break;
		case SPECIAL_EFFECT_REMOTE_HEAL:
			center_offset_x = ((TILESIZE_X / 2) * (a*a));
			center_offset_y = ((TILESIZE_X / 2) * (a*a));
			if (a > 0) z += 1.5/(a+.5) - 1;					//beam up effect
			if (marker->caster)
				glColor4f(0.0f, 0.0f, 1.0f, a);				//caster
			else
				glColor4f(0.0f, 1.0f, 0.0f, a);				//recipient
			break;
		case SPECIAL_EFFECT_SHIELD:
			center_offset_x = ((TILESIZE_X / 2) / (a));		//slow expanding
			center_offset_y = ((TILESIZE_X / 2) / (a));	
			z += a*0.3f;
			glColor4f(1.0f-a/2, 1.0f-a/2, 1.0f, a);			//color change effect
			break;
		default:
			center_offset_x = ((TILESIZE_X / 2) * (a*a));
			center_offset_y = ((TILESIZE_X / 2) * (a*a));
			z += a*0.3f;									//drop toward ground
			glColor4f(0.0f, 1.0f, 1.0f, a);
			break;
	}
	
	glBegin(GL_POLYGON);
	glVertex3f(x - 2*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x - 1*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x - 0*dx - center_offset_x, y - 0*dy - center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y - 1*dy - center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x + 2*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x + 1*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x + 0*dx + center_offset_x, y - 0*dy - center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y - 1*dy - center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x + 2*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x + 1*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x + 0*dx + center_offset_x, y + 0*dy + center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y + 1*dy + center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x - 2*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x - 1*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x - 0*dx - center_offset_x, y + 0*dy + center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y + 1*dy + center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x - 0*dx, y + 2.83*dy + center_offset_y, z);
	glVertex3f(x + .71*dx, y + 2.12*dy + center_offset_y, z);
	glVertex3f(x - 0*dx, y + 0*dy + center_offset_y, z);
	glVertex3f(x - .71*dx, y + 2.12*dy + center_offset_y, z);
	glVertex3f(x - 0*dx, y + 2.83*dy + center_offset_y, z);
	glEnd();		
	glBegin(GL_POLYGON);
	glVertex3f(x - 0*dx, y - 2.83*dy - center_offset_y, z);
	glVertex3f(x - .71*dx, y - 2.12*dy - center_offset_y, z);
	glVertex3f(x - 0*dx, y + 0*dy - center_offset_y, z);
	glVertex3f(x + .71*dx, y - 2.12*dy - center_offset_y, z);
	glVertex3f(x - 0*dx, y - 2.83*dy - center_offset_y, z);
	glEnd();		
	glBegin(GL_POLYGON);
	glVertex3f(x + 2.83*dx + center_offset_x, y + 0*dy, z);
	glVertex3f(x + 2.12*dx + center_offset_x, y - .71*dy, z);
	glVertex3f(x - 0*dx + center_offset_x, y + 0*dy, z);
	glVertex3f(x + 2.12*dx + center_offset_x, y + .71*dy, z);
	glVertex3f(x + 2.83*dx + center_offset_x, y + 0*dy, z);
	glEnd();		
	glBegin(GL_POLYGON);
	glVertex3f(x - 2.83*dx - center_offset_x, y + 0*dy, z);
	glVertex3f(x - 2.12*dx - center_offset_x, y + .71*dy, z);
	glVertex3f(x - 0*dx - center_offset_x, y + 0*dy, z);
	glVertex3f(x - 2.12*dx - center_offset_x, y - .71*dy, z);
	glVertex3f(x - 2.83*dx - center_offset_x, y + 0*dy, z);
	glEnd();		
}

void display_special_effects() {
	int i;
	//probably want to do a config check to turn on/off sfx
	if (!sfx_enabled) return;
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_ALPHA_TEST);

	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (sfx_markers[i].active) {
			sfx_markers[i].timeleft -= (cur_time - last_time);
			if (sfx_markers[i].timeleft > 0) {
				display_special_effect(&sfx_markers[i]);
			} else {
				// This marker has lived long enough now.
				sfx_markers[i].active = 0;
			}
		}
	}

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

//special effects targeted to a particular location
//void add_special_effect_location(int effect, Uint16 x, Uint16 y)
//{
	//temporary code just to show an effect
//	add_sfx(x,y, effect, 0);
//}

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
				add_sfx(sfx,var_a,1);
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
				add_sfx(sfx,var_a,1); //caster
				add_sfx(sfx,var_b,0); //target
			}
			break;
		//location (a&b variable are not known until implemented by server)
		case	SPECIAL_EFFECT_INVASION_BEAMING:
		case	SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				var_b = SDL_SwapLE16 (*((Uint16 *)(&data[offset+1])));
				snprintf (str, sizeof (str), "effect %d,  x pos=%d, y pos=%d",sfx,var_a,var_b);			
				//need good function here when implemented
			}
			break;
		default:
			snprintf (str, sizeof (str), " SPECIAL_EFFECT_unknown:%d",sfx);
			break;
	}
	LOG_TO_CONSOLE (c_purple2, str);
}

#endif //SFX