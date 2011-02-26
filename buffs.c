/*
 * buffs.c
 *
 *  Created on: 14.11.2008
 *      Author: superfloh
 */

#include "buffs.h"

#include "client_serv.h"
#include "eye_candy_wrapper.h"
#include "font.h" // for ALT_INGAME_FONT_X_LEN
#include "gl_init.h"
#include "init.h" // for poor_man
#include "interface.h" // for view_names
#include "platform.h"
#include "spells.h" // for the sigils texture
#include "textures.h"

int view_buffs = 1;
int buff_icon_size = 32;

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
		if (in_buffs & BUFF_DOUBLE_SPEED)
			act->step_duration = actors_defs[act->actor_type].step_duration / 2;
		else
			act->step_duration = actors_defs[act->actor_type].step_duration;
		if (act->attached_actor >= 0)
		{
			actors_list[act->attached_actor]->buffs = in_buffs & BUFF_DOUBLE_SPEED;
			actors_list[act->attached_actor]->step_duration = act->step_duration;
		}
		act->buffs = in_buffs;
#ifdef BUFF_DEBUG
		{
			int i, num_buffs = 0;
			for (i = 0; i < NUM_BUFFS; i++)
			{
				if (act->buffs & ((Uint32)pow(2, i))) {
					num_buffs++;
				}
			}
			printf("update_actor_buffs: name %s id %i num buffs: %i\n", act->actor_name, act->actor_id, num_buffs);
		}
		if (in_buffs & BUFF_INVISIBILITY) {
			printf(" invisibility ON\n");
		}
//		else {
//			printf(" invisibility off\n");
//		}
		if (in_buffs & BUFF_MAGIC_IMMUNITY) {
			printf(" magic immunity ON\n");
		}
//		else {
//			printf(" magic immunity off\n");
//		}
		if (in_buffs & BUFF_MAGIC_PROTECTION) {
			printf(" magic protection ON\n");
		}
//		else {
//			printf(" magic protection off\n");
//		}
		if (in_buffs & BUFF_COLD_SHIELD) {
			printf(" cold shield ON\n");
		}
//		else {
//			printf(" cold shield off\n");
//		}
		if (in_buffs & BUFF_HEAT_SHIELD) {
			printf(" heat shield ON\n");
		}
//		else {
//			printf(" heat shield off\n");
//		}
		if (in_buffs & BUFF_RADIATION_SHIELD) {
			printf(" radiation shield ON\n");
		}
//		else {
//			printf(" radiation shield off\n");
//		}
		if (in_buffs & BUFF_SHIELD) {
			printf(" shield ON\n");
		}
//		else {
//			printf(" shield off\n");
//		}
		if (in_buffs & BUFF_TRUE_SIGHT) {
			printf(" true sight ON\n");
		}
//		else {
//			printf(" true sight off\n");
//		}
		if (in_buffs & BUFF_ACCURACY) {
			printf(" accuracy ON\n");
		}
//		else {
//			printf(" accuracy off\n");
//		}
		if (in_buffs & BUFF_EVASION) {
			printf(" evasion ON\n");
		}
//		else {
//			printf(" evasion off\n");
//		}
		if (in_buffs & BUFF_DOUBLE_SPEED) {
			printf(" double speed ON\n");
		}
//		else {
//			printf(" double speed off\n");
//		}
#endif // BUFF_DEBUG
		update_buff_eye_candy(actor_id);
	}
}

void update_buff_eye_candy(int actor_id) {
	actor *act;
	act = get_actor_ptr_from_id(actor_id);
	if (act) {
		int i = 0; // loop index
        // turn on eye candy effects
		if (act->buffs & BUFF_SHIELD && act->ec_buff_reference[((Uint32)(log(BUFF_SHIELD)/log(2)))] == NULL) {
			act->ec_buff_reference[((Uint32)(log(BUFF_SHIELD)/log(2)))] = ec_create_ongoing_shield2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		}
		if (act->buffs & BUFF_MAGIC_PROTECTION && act->ec_buff_reference[((Uint32)(log(BUFF_MAGIC_PROTECTION)/log(2)))] == NULL) {
			act->ec_buff_reference[((Uint32)(log(BUFF_MAGIC_PROTECTION)/log(2)))] = ec_create_ongoing_magic_protection2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		}
/*
 * not yet implemented
		if (act->buffs & BUFF_POISONED && act->ec_buff_reference[((Uint32)(log(BUFF_POISONED)/log(2)))] == NULL) {
			act->ec_buff_reference[((Uint32)(log(BUFF_POISONED)/log(2)))] = ec_create_ongoing_poison2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
		}
*/
// removed by Roja's request
//		if (act->buffs & BUFF_MAGIC_IMMUNITY && act->ec_buff_reference[((Uint32)(log(BUFF_MAGIC_IMMUNITY)/log(2)))] == NULL) {
//			act->ec_buff_reference[((Uint32)(log(BUFF_MAGIC_IMMUNITY)/log(2)))] = ec_create_ongoing_magic_immunity2(act, 1.0, 1.0, (poor_man ? 6 : 10), 1.0);
//		}
		// turn off effects
		for (i = 0; i < NUM_BUFFS; i++) {
			if (act->ec_buff_reference[i] != NULL && !(act->buffs & ((Uint32)pow(2, i)))) {
				ec_recall_effect(act->ec_buff_reference[i]);
				act->ec_buff_reference[i] = NULL;
			}
		}
	}
}

void draw_buffs(int actor_id, float x, float y,float z)
{
	actor *act;
	act = get_actor_ptr_from_id(actor_id);
	if (act && act->buffs) {
		// texture coords
		float u_start,v_start,u_end,v_end;
		// current texture
		int cur_tex = 0;
		// loop index
		int i;
		// number of buffs
		int num_buffs = 0;
		// x offset
		int x_off = 0;
		// textures
		int texture_ids[NUM_BUFFS];
		//enable alpha filtering, so we have some alpha key
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER,0.1f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor3f(1.0f,1.0f,1.0f);
#ifdef	NEW_TEXTURES
		bind_texture(sigils_text);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(sigils_text);
#endif	/* NEW_TEXTURES */
		// keep in sync with client_serv.h !!!
		if (act->buffs & BUFF_SHIELD) {
			texture_ids[num_buffs] = 32;
			num_buffs++;
		}
		if (act->buffs & BUFF_MAGIC_PROTECTION) {
			texture_ids[num_buffs] = 33;
			num_buffs++;
		}
		if (act->buffs & BUFF_COLD_SHIELD) {
			texture_ids[num_buffs] = 55;
			num_buffs++;
		}
		if (act->buffs & BUFF_HEAT_SHIELD) {
			texture_ids[num_buffs] = 56;
			num_buffs++;
		}
		if (act->buffs & BUFF_RADIATION_SHIELD) {
			texture_ids[num_buffs] = 57;
			num_buffs++;
		}
/*
 * not yet implemented
		if (act->buffs & BUFF_POISONED) {
			texture_ids[num_buffs] = 34;
			num_buffs++;
		}
*/
		if (act->buffs & BUFF_MAGIC_IMMUNITY) {
			texture_ids[num_buffs] = 35;
			num_buffs++;
		}
		// move icons up by actor name and actor health bar
		y = y + 1.0f/ALT_INGAME_FONT_X_LEN*SMALL_INGAME_FONT_Y_LEN*name_zoom*12.0*view_names // displayed_font_y_size from font.c
		      + ALT_INGAME_FONT_Y_LEN*12.0*name_zoom*1.0f/ALT_INGAME_FONT_X_LEN; // healthbar_y_len from actors.c
		for (i = 0; i < num_buffs; i++)
		{
			cur_tex = texture_ids[i];
			//now get the texture coordinates, copied from spells.c
#ifdef	NEW_TEXTURES
			u_start = 0.125f * (cur_tex % 8);
			u_end = u_start + 0.125f;
			v_start = 0.125f * (cur_tex / 8);
			v_end = v_start + 0.125f;
#else	/* NEW_TEXTURES */
			u_start=0.125f*(cur_tex%8);
			u_end=u_start+0.125f;
			v_start=1.0f-(0.125f*(cur_tex/8));
			v_end=v_start-0.125f;
#endif	/* NEW_TEXTURES */
			x_off = (int)(-1.0 * ((float)num_buffs * buff_icon_size) / 2.0f + (buff_icon_size * i));
			// draw the spell icon
			glBegin(GL_QUADS);
			glTexCoord2f(u_start,v_start);
			glVertex3f(x + x_off, y + buff_icon_size, z);

			glTexCoord2f(u_start,v_end);
			glVertex3f(x + x_off,y,z);

			glTexCoord2f(u_end,v_end);
			glVertex3f(x + buff_icon_size + x_off, y, z);

			glTexCoord2f(u_end,v_start);
			glVertex3f(x + buff_icon_size + x_off, y + buff_icon_size, z);
			glEnd();
		}
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	#ifdef OPENGL_TRACE
		CHECK_GL_ERRORS();
	#endif //OPENGL_TRACE
	}
}
