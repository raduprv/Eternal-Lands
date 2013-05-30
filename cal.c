#include <stdlib.h>
#include "actors.h"
#include "actor_init.h"
#include "cal.h"
#include "draw_scene.h"
#include "errors.h"
#include "font.h"
#include "global.h"
#include "load_gl_extensions.h"
#include "missiles.h"
#include "shadows.h"
#include "translate.h"
#ifdef NEW_SOUND
#include "asc.h"
#include "sound.h"
#include "tiles.h"
#endif /* NEW_SOUND */
#ifdef DEBUG
#include "init.h"
#endif /* DEBUG */
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif /* OPENGL_TRADE */
#include "io/elfilewrapper.h"
#include "io/cal3d_io_wrapper.h"

#ifdef MORE_EMOTES
void start_transition(actor *act, int to, int msec){

	if (act->startIdle==to) return;
	act->startIdle=act->cur_anim.anim_index;
	act->endIdle=to;
	act->idleTime=cur_time;
	act->idleDuration=msec;
	act->busy=1;
	printf("transition started from %i to %i starting at %i, lasting %i\n",act->startIdle,to,act->idleTime,msec);
}

int do_transition(actor *act){
	struct CalMixer *mixer;
	float k;

	if (act==NULL) return 0;
	if (act->calmodel==NULL) return 0;
	if (act->startIdle==act->endIdle) return 0;

	printf("doing transition from %i to %i at %i of %i\n",act->startIdle,act->endIdle,cur_time-act->idleTime,act->idleDuration);

	mixer=CalModel_GetMixer(act->calmodel);	
	k=(float)(cur_time-act->idleTime)/act->idleDuration;
	if(k>1.0) k=1.0;
	CalMixer_BlendCycle(mixer,act->startIdle,1.0-k, 0.0);
	CalMixer_BlendCycle(mixer,act->endIdle,k, 0.0);

	if(k>=1.0) {
		printf("transition done\n");
		CalMixer_ClearCycle(mixer,act->startIdle, 0.0);
		act->startIdle=act->endIdle;
		act->idleTime=0;
		act->busy=0;
		return 1;
	}
	return 0;
}
#endif


#ifdef NEW_SOUND
void cal_play_anim_sound(actor *pActor, struct cal_anim anim, int is_emote){

	unsigned int *cookie;

	cookie = (is_emote) ? (&pActor->cur_emote_sound_cookie):(&pActor->cur_anim_sound_cookie);
	// Check if we need a walking sound
	if (pActor->moving && !pActor->fighting &&!is_emote){
		handle_walking_sound(pActor, anim.sound);
	} else {
		if (check_sound_loops(*cookie))
				stop_sound(*cookie);
		*cookie = 0;
		
		if (anim.sound > -1 && !pActor->dead){
			// We are going to try letting sounds continue until finished, except looping sounds of course
			// Found a sound, so add it
			*cookie = add_sound_object_gain(anim.sound,
							 2*pActor->x_pos,
							 2*pActor->y_pos,
							 pActor->actor_id == yourself ? 1 : 0,
							 anim.sound_scale);
		}
	}

}
#endif

void cal_reset_emote_anims(actor *pActor, int cycles_too){

	struct CalMixer *mixer;
	emote_anim *cur_emote;

	if (pActor==NULL)
		return;

	if (pActor->calmodel==NULL)
		return;

	mixer=CalModel_GetMixer(pActor->calmodel);	
	cur_emote=&pActor->cur_emote;

	//remove emote idle
	if(cur_emote->idle.anim_index>=0&&cycles_too) {
		//printf("CAL_reset cycle %i\n",cur_emote->idle.anim_index);
		CalMixer_ClearCycle(mixer,cur_emote->idle.anim_index, cal_cycle_blending_delay);
		cur_emote->idle.anim_index=-1;
	}
	if(cur_emote->active) {
		int i;
		//remove actions
		for(i=0;i<cur_emote->nframes;i++){
			if(cur_emote->frames[i].anim_index>=0&&cur_emote->frames[i].kind==action){
				//printf("CAL_reset action %i\n",cur_emote->frames[i].anim_index);
				CalMixer_RemoveAction(mixer,cur_emote->frames[i].anim_index);
				cur_emote->frames[i].anim_index=-1;
			}
		}
	}
	cur_emote->active=0;

#ifdef NEW_SOUND
	if(pActor->cur_emote_sound_cookie)
		stop_sound(pActor->cur_emote_sound_cookie);
#endif
	
}


void cal_actor_set_emote_anim(actor *pActor, emote_frame *anims){
	struct CalMixer *mixer;
	struct cal_anim *action;
	hash_entry *he;
	emote_anim *cur_emote;
	int i;
	float md=0;		
		
	if (pActor==NULL||!anims)
		return;

	if (pActor->calmodel==NULL)
		return;
	mixer=CalModel_GetMixer(pActor->calmodel);
	cur_emote=&pActor->cur_emote;
	cur_emote->start_time=cur_time;
	cur_emote->active=1;
	cur_emote->nframes=anims->nframes;
	cur_emote->flow=anims;

	for(i=0;i<anims->nframes;i++) {
		//printf("adding frame %i: %i\n",i,anims->ids[i]);
		he=hash_get(actors_defs[pActor->actor_type].emote_frames,(void*)(NULL+anims->ids[i]));
		if(!he) continue;
		action = (struct cal_anim*) he->item;
		//printf("duration scale %f\n",action->duration_scale);
		cur_emote->frames[i]=*action;
		if (action->kind==cycle){
			//handle cycles:
			//removes previous emote cycles
			if(cur_emote->idle.anim_index>=0)
				CalMixer_ClearCycle(mixer,cur_emote->idle.anim_index, cal_cycle_blending_delay);
			//removes previous idle cycles
			if(pActor->cur_anim.kind==cycle) {
				CalMixer_ClearCycle(mixer,pActor->cur_anim.anim_index, cal_cycle_blending_delay);
				pActor->cur_anim.anim_index=-1;
			}
			CalMixer_BlendCycle(mixer,action->anim_index,1.0f, cal_cycle_blending_delay);
			CalMixer_SetAnimationTime(mixer, 0.0f);
			cur_emote->idle=*action;
		} else {
			CalMixer_ExecuteAction_Stop(mixer,action->anim_index,0,0);
			md=(action->duration/action->duration_scale>md) ? (action->duration*1000.0/action->duration_scale):(md);
#ifdef NEW_SOUND
			if (action->sound>-1) cal_play_anim_sound(pActor, *action,1);
#endif
		}
	}
	cur_emote->max_duration=(int)md;
	//printf("EMOTE: start: %i, end: %i, dur: %i\n", cur_emote->start_time, cur_emote->start_time+cur_emote->max_duration,cur_emote->max_duration);

}

void handle_cur_emote(actor *pActor){
	emote_anim *cur_emote;

	if (pActor==NULL)
		return;

	if (pActor->calmodel==NULL)
		return;
	
	cur_emote = &pActor->cur_emote;	

	if(cur_emote->active&&cur_emote->start_time+cur_emote->max_duration<cur_time){
		//all anims are finished, see if more frames are linked
		//printf("reset current frame\n");
		cal_reset_emote_anims(pActor,0);
		//printf("Remove EMOTE: %i\n",cur_time);
		if(cur_emote->flow) {
			cur_emote->flow=cur_emote->flow->next;
			//printf("starting next emote frame %p\n",cur_emote->flow);
			cal_actor_set_emote_anim(pActor, cur_emote->flow);
		}
	}
}





void cal_actor_set_anim_delay(int id, struct cal_anim anim, float delay)
{
	actor *pActor = actors_list[id];
	struct CalMixer *mixer;
	int i;

	//char str[255];
	//sprintf(str, "actor:%d anim:%d type:%d delay:%f\0",id,anim.anim_index,anim.kind,delay);
	//LOG_TO_CONSOLE(c_green2,str);
	
	if (pActor==NULL)
		return;

	if (pActor->calmodel==NULL)
		return;

	if (pActor->cur_anim.anim_index==anim.anim_index)
		return;
	
	//this shouldnt happend but its happends if actor doesnt have
	//animation so we add this workaround to prevent "freezing"
	if(anim.anim_index==-1){
        attachment_props *att_props;
		if(	pActor->sitting==1 ){
			//we dont have sitting anim so cancel it
			pActor->sitting=0;
		}
		pActor->stop_animation=0;
		att_props = get_attachment_props_if_held(pActor);
		if (att_props)
		{
			anim.anim_index = att_props->cal_frames[cal_attached_idle_frame].anim_index;
			anim.duration = 0.5; // we're not really idle, so only play a short version as placeholder
			anim.duration_scale = att_props->cal_frames[cal_attached_idle_frame].duration_scale;
		}
		else
		{
			anim.anim_index = actors_defs[pActor->actor_type].cal_frames[cal_actor_idle1_frame].anim_index;
			anim.duration = 0.5; // we're not really idle, so only play a short version as placeholder
			anim.duration_scale = actors_defs[pActor->actor_type].cal_frames[cal_actor_idle1_frame].duration_scale;
		}
		anim.kind = cycle;
	}

	mixer=CalModel_GetMixer(pActor->calmodel);

	//Stop previous animation if needed
	if (pActor->IsOnIdle!=1 && (pActor->cur_anim.anim_index!=-1)) {
		if (pActor->cur_anim.kind==cycle) {
			//little more smooth
			delay+=cal_cycle_blending_delay;
			CalMixer_ClearCycle(mixer,pActor->cur_anim.anim_index, delay);
		}
		if (pActor->cur_anim.kind==action) {
			CalMixer_RemoveAction(mixer,pActor->cur_anim.anim_index);
			//change from action to new action or cycle should be smooth
			if(anim.duration>0.0f)
				delay=anim.duration;
			else
				delay+=cal_action_blending_delay;
		}
	}else{
		//we starting from unkown state (prev anim == -1)
		//so we add some delay to blend into new state
		if(anim.duration>0.0f)
			delay=anim.duration;
		else
			delay+=cal_action_blending_delay;
	}

	//seems to be unusable - groups are always empty???
	if (pActor->IsOnIdle==1) {
		for (i=0;i<actors_defs[pActor->actor_type].group_count;++i) {
			CalMixer_ClearCycle(mixer,pActor->cur_idle_anims[i].anim_index, delay);
		}
	}
	
	if (anim.kind==cycle){
		CalMixer_BlendCycle(mixer,anim.anim_index,1.0f, delay);
		CalMixer_SetAnimationTime(mixer, 0.0f);	//always start at the beginning of a cycling animation
		//if an emote is cycling, stop it
		if(pActor->cur_emote.idle.anim_index!=-1) {
				//printf("stopping idle emote %i\n", pActor->cur_emote.idle.anim_index);
				CalMixer_ClearCycle(mixer,pActor->cur_emote.idle.anim_index,0);
				pActor->cur_emote.idle.anim_index=-1;
		}
/*		//if an emote is playing, stop it
		if(pActor->cur_emote.active) {
				printf("stopping emote of actor %i\n", pActor->actor_id);
				cal_reset_emote_anims(pActor,0);
				pActor->cur_emote.flow=NULL;
		}
*/
	} else {
		CalMixer_ExecuteAction_Stop(mixer,anim.anim_index,delay,0.0);
		//if an emote is playing, stop it
/*		if(pActor->cur_emote.active) {
				printf("stopping emote of actor %i\n", pActor->actor_id);
				cal_reset_emote_anims(pActor,0);
				pActor->cur_emote.flow=NULL;
		}
*/
	}

	pActor->cur_anim=anim;
	pActor->anim_time=0.0;
	pActor->last_anim_update= cur_time;
	pActor->stop_animation = anim.kind;
	
	CalModel_Update(pActor->calmodel,0.0001);//Make changes take effect now
	build_actor_bounding_box(pActor);

	missiles_rotate_actor_bones(pActor);

	if (use_animation_program)
	{
		set_transformation_buffers(pActor);
	}

	if (pActor->cur_anim.anim_index==-1)
		pActor->busy=0;
	pActor->IsOnIdle=0;
#ifdef NEW_SOUND
		cal_play_anim_sound(pActor, pActor->cur_anim,0);
#endif

}

void cal_actor_set_anim(int id,struct cal_anim anim)
{
	cal_actor_set_anim_delay(id, anim, 0.05f);
}

#ifdef NEW_SOUND
void cal_set_anim_sound(struct cal_anim *my_cal_anim, const char *sound, const char *sound_scale)
{
	if (sound)
		my_cal_anim->sound = get_index_for_sound_type_name(sound);
	else
		my_cal_anim->sound = -1;

	if (sound_scale && strcasecmp(sound_scale, ""))
		my_cal_anim->sound_scale = atof(sound_scale);
	else
		my_cal_anim->sound_scale = 1.0f;
}
#endif // NEW_SOUND


#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale, int duration)
#else
struct cal_anim cal_load_anim(actor_types *act, const char *str, int duration)
#endif	//NEW_SOUND
{
	char fname[255]={0};
	struct cal_anim res={-1,cycle,0,0.0f
#ifdef NEW_SOUND
	,-1
#endif  //NEW_SOUND
	};
	struct CalCoreAnimation *coreanim;
#ifdef NEW_SOUND
	int i;
#endif  //NEW_SOUND

	if (sscanf (str, "%254s %d", fname, (int*)(&res.kind)) != 2)
	{
		LOG_ERROR("Bad animation formation: %s", str);
		return res;
	}

#ifdef NEW_SOUND
	if (have_sound_config && sound && strcasecmp(sound, ""))
	{
		i = get_index_for_sound_type_name(sound);
		if (i == -1)
			LOG_ERROR("Unknown sound (%s) in actor def: %s", sound, act->actor_name);
		else
			res.sound = i;
	}
	else
		res.sound = -1;

	if (sound_scale && strcasecmp(sound_scale, ""))
		res.sound_scale = atof(sound_scale);
	else
		res.sound_scale = 1.0f;
#endif	//NEW_SOUND

	res.anim_index=CalCoreModel_ELLoadCoreAnimation(act->coremodel,fname,act->scale);
	if(res.anim_index == -1) {
		LOG_ERROR("Cal3d error: %s: %s\n", fname, CalError_GetLastErrorDescription());
		return res;
	}
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);

	if (coreanim) {
		res.duration=CalCoreAnimation_GetDuration(coreanim);
		if (duration > 0) res.duration_scale = res.duration/(duration*0.001f);
		else res.duration_scale = 1.0f;
	} else {
		LOG_ERROR(no_animation_err_str, fname);
	}
	return res;
}



void cal_render_bones(actor *act)
{
	float lines[1024][2][3];
	float points[1024][3];
	int nrLines;
	int nrPoints;
	int currLine;
	int currPoint;
	struct CalSkeleton *skel;

	skel=CalModel_GetSkeleton(act->calmodel);
	nrLines = CalSkeleton_GetBoneLines(skel,&lines[0][0][0]);

	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 1.0f);

	glLineStipple(1, 0x3030);
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);

	for(currLine = 0; currLine < nrLines; currLine++) {
    		glVertex3f(lines[currLine][0][0], lines[currLine][0][1], lines[currLine][0][2]);
    		glVertex3f(lines[currLine][1][0], lines[currLine][1][1], lines[currLine][1][2]);
	}

	glEnd();
	glDisable(GL_LINE_STIPPLE);

  	// draw the bone points
  	nrPoints = CalSkeleton_GetBonePoints(skel,&points[0][0]);

	glPointSize(4.0f);
	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_POINTS);
	for(currPoint = 0; currPoint < nrPoints; currPoint++) {
		glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
	}
	glEnd();

#ifdef DEBUG
	// draw the bones orientation
	if (render_bones_orientation) {
		float shift[3], pos[3];
		
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		for (currPoint = nrPoints; currPoint--;) {
			shift[0] = 0.1; shift[1] = 0.0; shift[2] = 0.0;
			cal_get_actor_bone_local_position(act, currPoint, shift, pos);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
			glVertex3fv(pos);
			
			shift[0] = 0.0; shift[1] = 0.1; shift[2] = 0.0;
			cal_get_actor_bone_local_position(act, currPoint, shift, pos);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
			glVertex3fv(pos);
			
			shift[0] = 0.0; shift[1] = 0.0; shift[2] = 0.1;
			cal_get_actor_bone_local_position(act, currPoint, shift, pos);
			glColor3f(0.0, 0.0, 1.0);
			glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
			glVertex3fv(pos);
		}
		glEnd();
	}

	// draw bones id
	if (render_bones_id) {
		GLdouble model[16], proj[16];
		GLint view[4];
		GLdouble px,py,pz;
		unsigned char buf[16];
		float font_size_x = SMALL_INGAME_FONT_X_LEN/ALT_INGAME_FONT_X_LEN;
		float font_size_y = SMALL_INGAME_FONT_Y_LEN/ALT_INGAME_FONT_X_LEN;

		glGetDoublev(GL_MODELVIEW_MATRIX, model);
		glGetDoublev(GL_PROJECTION_MATRIX, proj);
		glGetIntegerv(GL_VIEWPORT, view);
		
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glOrtho(view[0],view[2]+view[0],view[1],view[3]+view[1],0.0f,-1.0f);
		
		glPushAttrib(GL_ENABLE_BIT);
		
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		glColor4f(1.0, 0.0, 1.0, 1.0);
		
		for (currPoint = nrPoints; currPoint--;) {
			struct CalBone *bone;
			bone = CalSkeleton_GetBone(skel, currPoint);
			
			sprintf((char*)buf, "%d", currPoint);
			gluProject(points[currPoint][0], points[currPoint][1], points[currPoint][2], model, proj, view, &px, &py, &pz);
			draw_ortho_ingame_string(px, py, pz, buf, 1, font_size_x, font_size_y);
		}
		
		glPopAttrib();
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
#endif // DEBUG

	glLineWidth(1.0f);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

}


static __inline__ void render_submesh(int meshId, int submeshCount, struct CalRenderer * pCalRenderer, float meshVertices[30000][3], float meshNormals[30000][3], float meshTextureCoordinates[30000][2], CalIndex meshFaces[50000][3], Uint32 use_lightning, Uint32 use_textures)
{
	int submeshId;
	int faceCount=0;

	for(submeshId = 0; submeshId < submeshCount; submeshId++) {
		// select mesh and submesh for further data access
		if(CalRenderer_SelectMeshSubmesh(pCalRenderer,meshId, submeshId)) {
			// get the transformed vertices of the submesh
			CalRenderer_GetVertices(pCalRenderer,&meshVertices[0][0]);

			// get the transformed normals of the submesh
			if (use_lightning)
			{
				CalRenderer_GetNormals(pCalRenderer,&meshNormals[0][0]);
			}

			// get the texture coordinates of the submesh
			if (use_textures)
			{
				CalRenderer_GetTextureCoordinates(pCalRenderer,0,&meshTextureCoordinates[0][0]);
			}

			// get the faces of the submesh
			faceCount = CalRenderer_GetFaces(pCalRenderer, &meshFaces[0][0]);

			// set the vertex and normal buffers
			glVertexPointer(3, GL_FLOAT, 0, &meshVertices[0][0]);
			if (use_lightning)
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, 0, &meshNormals[0][0]);
			}
			else
			{
				glDisableClientState(GL_NORMAL_ARRAY);
			}

			// draw the submesh
			if (use_textures)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, 0, &meshTextureCoordinates[0][0]);
			}
			else
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}

			if(sizeof(CalIndex)==2)
				glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_SHORT, &meshFaces[0][0]);
			else
				glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, &meshFaces[0][0]);
		}
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void cal_render_actor(actor *act, Uint32 use_lightning, Uint32 use_textures, Uint32 use_glow)
{
	struct CalRenderer *pCalRenderer;
	int meshCount,meshId,submeshCount/*,submeshId, vertexCount*/;
	float points[1024][3];
	static float meshVertices[30000][3];
	static float meshNormals[30000][3];
	static float meshTextureCoordinates[30000][2];
	static CalIndex meshFaces[50000][3];
	struct CalSkeleton *skel;
	struct CalMesh *_mesh;
	struct CalCoreMesh *_coremesh;
	struct CalCoreMesh *_weaponmesh;
	struct CalCoreMesh *_shieldmesh;
	//int boneid=-1;
	float reverse_scale;
	//int glow=-1;
	
	if(act->calmodel==NULL) {
		return;//Wtf!?
	}
	skel=CalModel_GetSkeleton(act->calmodel);

	glPushMatrix();
	// actor model rescaling
	if(actors_defs[act->actor_type].actor_scale != 1.0){
		glScalef(actors_defs[act->actor_type].actor_scale, actors_defs[act->actor_type].actor_scale, actors_defs[act->actor_type].actor_scale);
	}
	// the dynamic scaling
	if(act->scale != 1.0f){
		glScalef(act->scale,act->scale,act->scale);
	}

#ifdef	DYNAMIC_ANIMATIONS
	if(act->last_anim_update < cur_time)
		if(act->cur_anim.duration_scale > 0.0f)
			CalModel_Update(act->calmodel, (((cur_time-act->last_anim_update)*act->cur_anim.duration_scale)/1000.0));
	build_actor_bounding_box(act);
	missiles_rotate_actor_bones(act);
	if (use_animation_program)
	{
		set_transformation_buffers(act);
	}
	act->last_anim_update= cur_time;
#endif	//DYNAMIC_ANIMATIONS

	// get the renderer of the model
#ifdef DEBUG
	if (render_mesh) {
#endif
		pCalRenderer = CalModel_GetRenderer(act->calmodel);
		// begin the rendering loop
		if(CalRenderer_BeginRendering(pCalRenderer)){
			// set global OpenGL states
			if(!act->ghost && act->has_alpha){
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER,0.06f);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				//glDisable(GL_CULL_FACE);
			}

			// will use vertex arrays, so enable them
			glEnableClientState(GL_VERTEX_ARRAY);

			// get the number of meshes
			meshCount = CalRenderer_GetMeshCount(pCalRenderer);

			// check for weapons or shields being worn
			if (act->is_enhanced_model) {
				if(actors_defs[act->actor_type].weapon[act->cur_weapon].mesh_index!=-1) _weaponmesh=CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].weapon[act->cur_weapon].mesh_index);
				else _weaponmesh=NULL;
				if(act->body_parts->shield_meshindex!=-1) _shieldmesh=CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,act->body_parts->shield_meshindex);
				else _shieldmesh=NULL;
			} else {
				// non-enhanced never have weapon or shields
				_weaponmesh=NULL;
				_shieldmesh=NULL;
			}
			
			// render all meshes of the model
			for(meshId = 0; meshId < meshCount; meshId++){
				// get the number of submeshes
   				submeshCount = CalRenderer_GetSubmeshCount(pCalRenderer,meshId);

				_mesh=CalModel_GetAttachedMesh(act->calmodel,meshId);//Get current rendered mesh
				_coremesh=CalMesh_GetCoreMesh(_mesh);//Get the coremesh
				
				if(act->is_enhanced_model && (_weaponmesh || _shieldmesh)) {
					//Special treatment for weapons and shields only for enhanced models
					int glow=-1;
					int boneid=-1;
					
					if (_coremesh==_weaponmesh) boneid=26;//If it's a weapon snap to WeaponR bone
					else if (_coremesh==_shieldmesh) boneid=21;//If it's a shield snap to WeaponL bone
					if (boneid!=-1) {
						glPushMatrix();
						reverse_scale= 1.0/actors_defs[act->actor_type].skel_scale;
						CalSkeleton_GetBonePoints(skel,&points[0][0]);

						glTranslatef(points[boneid][0],points[boneid][1],points[boneid][2]);
						glScalef(reverse_scale,reverse_scale,reverse_scale);
						glTranslatef(-points[boneid][0],-points[boneid][1],-points[boneid][2]);

						// find the proper place to bind this object to
						switch(boneid){
							case 26:
								if(actors_defs[act->actor_type].weapon[act->cur_weapon].glow>0){
									glow=actors_defs[act->actor_type].weapon[act->cur_weapon].glow;
								}
								break;
							case 21:
								if(actors_defs[act->actor_type].shield[act->cur_shield].glow>0){
									glow=actors_defs[act->actor_type].shield[act->cur_shield].glow;
								}
							default:
								break;
						}
					}

					// now check for a glowing weapon
					if(glow>0 && use_glow){
						glEnable(GL_COLOR_MATERIAL);
						glBlendFunc(GL_ONE,GL_SRC_ALPHA);
						if(!act->ghost && !(act->buffs & BUFF_INVISIBILITY)) {
							glEnable(GL_BLEND);
							glDisable(GL_LIGHTING);
						}

						if(use_shadow_mapping){
							glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
							ELglActiveTextureARB(shadow_unit);
							glDisable(depth_texture_target);
							disable_texgen();
							ELglActiveTextureARB(GL_TEXTURE0);
						}

						glColor4f(glow_colors[glow].r, glow_colors[glow].g, glow_colors[glow].b, 0.5f);
						glPushMatrix();
						glScalef(0.99f, 0.99f, 0.99f);
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces, 0, use_textures);
						glPopMatrix();

						glColor4f(glow_colors[glow].r, glow_colors[glow].g, glow_colors[glow].b, 0.85f);
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces, 0, use_textures);
						glColor4f(glow_colors[glow].r, glow_colors[glow].g, glow_colors[glow].b, 0.99f);
						glPushMatrix();
						glScalef(1.01f, 1.01f, 1.01f);
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces, 0, use_textures);
						glPopMatrix();

						if(use_shadow_mapping){
							glPopAttrib();
						}
						glColor3f(1.0f, 1.0f, 1.0f);
						glDisable(GL_COLOR_MATERIAL);
						if(!act->ghost && !(act->buffs & BUFF_INVISIBILITY)) {
							glDisable(GL_BLEND);
							glEnable(GL_LIGHTING);
						} else {
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
							if((act->buffs & BUFF_INVISIBILITY)) {
								glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
							}
						}
					} else {
						// enhanced actors without glowing items
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces, use_lightning, use_textures);
					}
					if(boneid >= 0){
						//if this was a weapon or shield, restore the transformation matrix
						glPopMatrix();
					}
				} else {
					// non-enhanced actors, or enhanced without attached meshes
					render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces, use_lightning, use_textures);
				}
			}

			// clear vertex array state
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			if(!act->ghost && act->has_alpha){
				glDisable(GL_ALPHA_TEST);
				//glEnable(GL_CULL_FACE);
				glDisable(GL_BLEND);
			}

			// end the rendering
			CalRenderer_EndRendering(pCalRenderer);
		}
#ifdef DEBUG
	}
#endif

	glColor3f(1,1,1);

#ifdef DEBUG
	if(render_skeleton)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		
		cal_render_bones(act);
		
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}
#endif
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void cal_get_actor_bone_local_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos)
{
	struct CalSkeleton *skel;
	struct CalBone *bone;
	struct CalVector *point;

    if (in_bone_id < 0) return;

	skel = CalModel_GetSkeleton(in_act->calmodel);

    if (in_bone_id >= CalSkeleton_GetBonesNumber(skel)) return;

	bone = CalSkeleton_GetBone(skel, in_bone_id);
	point = CalBone_GetTranslationAbsolute(bone);

	memcpy(out_pos, CalVector_Get(point), 3*sizeof(float));

	if (in_shift) {
		struct CalQuaternion *rot;
		struct CalVector *vect;
		float *tmp;
		rot = CalBone_GetRotationAbsolute(bone);
		vect = CalVector_New();
		CalVector_Set(vect, in_shift[0], in_shift[1], in_shift[2]);
		CalVector_Transform(vect, rot);
		tmp = CalVector_Get(vect);
		out_pos[0] += tmp[0];
		out_pos[1] += tmp[1];
		out_pos[2] += tmp[2];
		CalVector_Delete(vect);
	}
}

void cal_get_actor_bone_absolute_position(actor *in_act, int in_bone_id, float *in_shift, float *out_pos)
{
	float act_rot[9];
	float pos[3];
	get_actor_rotation_matrix(in_act, act_rot);
	cal_get_actor_bone_local_position(in_act, in_bone_id, in_shift, pos);
	transform_actor_local_position_to_absolute(in_act, pos, act_rot, out_pos);
}
