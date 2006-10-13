#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "highlight.h"

#ifdef SFX
//much of this is based on the highlight.c code
#define SPECIAL_EFFECT_LIFESPAN	(500)
#define SPECIAL_EFFECT_HEAL_LIFESPAN (2000)
#define NUMBER_OF_SPECIAL_EFFECTS	(20)	// 20 active in one area should be enough, right?

typedef struct {
	short x;		// used to store x_tile_pos and y_tile_pos
	short y;		// will probably need a z too eventually
	actor *owner;	// will be NULL for stationary effects
	int timeleft;
	int lifespan;	// total lifespan of effect
	int type;		// type of effect / spell that was cast
	int active;	
	int caster;		//is this caster or target
} special_effect;

special_effect sfx_markers[NUMBER_OF_SPECIAL_EFFECTS];

int sfx_enabled = 1;

const static float dx = (TILESIZE_X / 6);
const static float dy = (TILESIZE_Y / 6);

//Allocate a spot for a new special effect
special_effect *get_free_special_effect() {
	int i;
	//find the first free slot
	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (!sfx_markers[i].active) {
			return &sfx_markers[i];
		}
	}
	return NULL;	// all memory for special effects has been taken
}

// Initialize a new special effect
void add_sfx(int effect, Uint16 playerid, int caster)
{
	Uint8 str[70];
	actor *this_actor = get_actor_ptr_from_id(playerid);
	special_effect *m = get_free_special_effect();
	if (m == NULL) 
	{
		snprintf (str, sizeof (str), "Could not add special effect.  Increase NUMBER_OF_SPECIAL_EFFECTS.");	
		LOG_TO_CONSOLE (c_purple2, str);
		return;
	}
	if (this_actor == NULL ) return;
		
	switch (effect)
	{
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:
		case SPECIAL_EFFECT_HEAL_SUMMONED:
		case SPECIAL_EFFECT_INVASION_BEAMING:
		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			m->x = this_actor->x_tile_pos;		//static effects will not store a actor by convention
			m->y = this_actor->y_tile_pos;		// but we need to know where they were cast
			break;						
		default:								// all others are movable effects
			m->owner = this_actor;				//let sfx_marker know who is target of effect
			m->x = m->owner->x_tile_pos;
			m->y = m->owner->y_tile_pos;
			break;
	}

	m->type = effect;
	if (effect == SPECIAL_EFFECT_HEAL)
	{
		m->timeleft = SPECIAL_EFFECT_HEAL_LIFESPAN;
		m->lifespan = SPECIAL_EFFECT_HEAL_LIFESPAN;
	}
	else
	{
		m->timeleft = SPECIAL_EFFECT_LIFESPAN;
		m->lifespan = SPECIAL_EFFECT_LIFESPAN;
	}
	m->active = 1;
	m->caster = caster;							// should = 1 if caster of spell, 0 otherwise
}

//basic shape template that allows for rotation and duplication
void do_shape_spikes(float x, float y, float z, float center_offset_x, float center_offset_y, float base_offset_z, float a)
{
	int i;
	
	//save the world
	glPushMatrix();
		glTranslatef(x,y,z);

		glRotatef(270.0f*a, 0.0f, 0.0f, 1.0f);

		//now create eight copies of the object, each separated by 45 degrees
		for (i = 0; i < 8; i++)
		{
			glRotatef(45.f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glEnd();
		}
	//return to the world
	glPopMatrix();
}

//example halos moving in opposite directions, not yet optimized, and still just an example
void do_double_spikes(float x, float y, float z, float center_offset_x, float center_offset_y, float base_offset_z, float a)
{
	int i;
	
	//save the world
	glPushMatrix();
		glTranslatef(x,y,z);

		glRotatef(270.0f*a, 0.0f, 0.0f, 1.0f);

		//now create eight copies of the object, each separated by 45 degrees
		for (i = 0; i < 8; i++)
		{
			glRotatef(45.f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glEnd();
		}
	//return to the world
	glPopMatrix();
}

void display_special_effect(special_effect *marker) {
	
	// (a) varies from 1..0 depending on the age of this marker
	const float a = ((float)marker->timeleft) / ((float)marker->lifespan);

	float x,y;
	
	// place x,y in the center of the actor's tile
	switch (marker->type)
	{
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:			// group "static" tile-based effects
		case SPECIAL_EFFECT_HEAL_SUMMONED:
		case SPECIAL_EFFECT_INVASION_BEAMING:
		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			x= (float)marker->x/2 + (TILESIZE_X / 2);	// "static" tile based effects
			y= (float)marker->y/2 + (TILESIZE_Y / 2);	// "static" tile based effects
			break;						
		default:										// all others are movable effects
			x = marker->owner->x_pos + (TILESIZE_X / 2);	// movable effects need current position
			y = marker->owner->y_pos + (TILESIZE_X / 2);
			break;
	}

	// height of terrain at the effect's location
	float z = get_tile_display_height(marker->x, marker->y);
	
	//	center_offset_x&y are for radial distance from actor in ground plane
	//	base_offset_z is for height off the ground (z)
	float center_offset_x, center_offset_y, base_offset_z;
	
	switch (marker->type) {
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));	//fast expanding
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			base_offset_z = z + a*0.3f;						//drop toward ground
			glColor4f(1.0f, 0.0f, 0.0f, a);
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_HEAL_SUMMONED:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			base_offset_z = z + a*0.3f;						//drop toward ground
			glColor4f(0.0f, 0.0f, 1.0f, a);
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_INVASION_BEAMING:
		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			break;
		case SPECIAL_EFFECT_HEAL:
			center_offset_x = (TILESIZE_X / 2);				//constant radius
			center_offset_y = (TILESIZE_X / 2);
			if (a > 0.5f) {
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);			//keep solid at the start
				base_offset_z = z + 3.0f/(2.0f*a) - 1.0f;	//move up quickly
				}
			else {
				glColor4f(1.0f, 1.0f, 1.0f, 2.0f*a);		//fade out
				base_offset_z = z + 2.0f;					//pause over location
				}
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_RESTORATION:
			center_offset_x = (TILESIZE_X / 2.5);				//constant radius
			center_offset_y = (TILESIZE_X / 2.5);
			if (a > 0) base_offset_z = z + 1.5/(a+.5) - 1;	//beam up effect
			glColor4f(1.0f-a, 0.0f, 0.0f+a, a);				//color gradient
			do_double_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		//this is an example using the marker->caster for PvP effects
		case SPECIAL_EFFECT_REMOTE_HEAL:
			center_offset_x = ((TILESIZE_X / 2) * (a*a));
			center_offset_y = ((TILESIZE_X / 2) * (a*a));
			if (a > 0) base_offset_z = z + 1.5/(a+.5) - 1;	//beam up effect
			if (marker->caster)
				glColor4f(0.0f, 0.0f, 1.0f, a);				//caster
			else
				glColor4f(0.0f, 1.0f, 0.0f, a);				//recipient
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_SHIELD:
			center_offset_x = ((TILESIZE_X / 2) / (a));		//relatively slow expanding
			center_offset_y = ((TILESIZE_X / 2) / (a));	
			base_offset_z = z + a*0.3f;						//drop toward ground
			glColor4f(1.0f-a/2, 1.0f-a/2, 1.0f, a);			//color change effect
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		default: // for all the spells we have not gotten to yet
			break;
	}

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
				LOG_TO_CONSOLE (c_purple2, str);
				//need good function here when implemented
			}
			break;
		default:
			snprintf (str, sizeof (str), " SPECIAL_EFFECT_unknown:%d",sfx);
			LOG_TO_CONSOLE (c_purple2, str);
			break;
	}
}

#endif //SFX
