#include <stdlib.h>
#include "cal.h"
#include "draw_scene.h"
#include "errors.h"
#include "global.h"
#include "load_gl_extensions.h"
#ifdef MISSILES
#include "missiles.h"
#endif
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
#ifdef	NEW_FILE_IO
#include "io/elfilewrapper.h"
#endif	//NEW_FILE_IO
#ifdef NEW_LIGHTING
 #include "lights.h"
#endif

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
		if(	pActor->sitting==1 ){
			//we dont have sittng anim so cancel it
			pActor->sitting=0;
		}
		pActor->stop_animation=0;
		anim.anim_index = actors_defs[pActor->actor_type].cal_idle1_frame.anim_index;
		anim.kind = cycle;
		anim.duration = actors_defs[pActor->actor_type].cal_idle1_frame.duration;
		anim.duration_scale = actors_defs[pActor->actor_type].cal_idle1_frame.duration_scale;
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
	} else {
		CalMixer_ExecuteAction_Stop(mixer,anim.anim_index,delay,0.0);
	}

	pActor->cur_anim=anim;
	pActor->anim_time=0.0;
	pActor->last_anim_update= cur_time;
	pActor->stop_animation = anim.kind;
	
	CalModel_Update(pActor->calmodel,0.0001);//Make changes take effect now

#ifdef MISSILES
	rotate_actor_bones(pActor);
#endif // MISSILES

	if (pActor->cur_anim.anim_index==-1)
		pActor->busy=0;
	pActor->IsOnIdle=0;

#ifdef NEW_SOUND
	// Check if we need a walking sound
	if (pActor->moving && !pActor->fighting)
	{
		handle_walking_sound(pActor, anim.sound);
	}
	else
	{
		// We are going to try letting sounds continue until finished, except looping sounds of course
		if (check_sound_loops(pActor->cur_anim_sound_cookie))
			stop_sound(pActor->cur_anim_sound_cookie);
		pActor->cur_anim_sound_cookie = 0;
		
		if (anim.sound > -1 && !pActor->dead)
		{
			// Found a sound, so add it
			pActor->cur_anim_sound_cookie = add_sound_object_gain(	anim.sound,
																	2*pActor->x_pos,
																	2*pActor->y_pos,
																	pActor->actor_id == yourself ? 1 : 0,
																	anim.sound_scale
																);
		}
	}
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

#ifdef	NEW_ACTOR_ANIMATION
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale, int duration)
	#else
struct cal_anim cal_load_anim(actor_types *act, const char *str, int duration)
	#endif	//NEW_SOUND
#else
	#ifdef NEW_SOUND
struct cal_anim cal_load_anim(actor_types *act, const char *str, const char *sound, const char *sound_scale)
	#else
struct cal_anim cal_load_anim(actor_types *act, const char *str)
	#endif	//NEW_SOUND
#endif
{
	char fname[255]={0};
	struct cal_anim res={-1,cycle,0
#ifdef  NEW_ACTOR_ANIMATION
	,0.0f
#endif
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
		log_error("Bad animation formation: %s", str);
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

#ifdef	NEW_FILE_IO
	res.anim_index=CalCoreModel_ELLoadCoreAnimation(act->coremodel,fname);
#else	//NEW_FILE_IO
	res.anim_index=CalCoreModel_LoadCoreAnimation(act->coremodel,fname);
#endif	//NEW_FILE_IO
	if(res.anim_index == -1) {
		log_error("Cal3d error: %s: %s\n", fname, CalError_GetLastErrorDescription());
		return res;
	}
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);

	if (coreanim) {
		CalCoreAnimation_Scale(coreanim,act->scale);
		res.duration=CalCoreAnimation_GetDuration(coreanim);
#ifdef	NEW_ACTOR_ANIMATION
		if (duration > 0) res.duration_scale = res.duration/(duration*0.001f);
		else res.duration_scale = 1.0f;
#endif
	} else {
		log_error(no_animation_err_str, fname);
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

	glLineWidth(3.0f);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_LINES);

	for(currLine = 0; currLine < nrLines; currLine++) {
    		glVertex3f(lines[currLine][0][0], lines[currLine][0][1], lines[currLine][0][2]);
    		glVertex3f(lines[currLine][1][0], lines[currLine][1][1], lines[currLine][1][2]);
	}

	glEnd();

	glLineWidth(1.0f);

  	// draw the bone points
  	nrPoints = CalSkeleton_GetBonePoints(skel,&points[0][0]);

	glPointSize(4.0f);

	glBegin(GL_POINTS);

	for(currPoint = 0; currPoint < nrPoints; currPoint++) {
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(points[currPoint][0], points[currPoint][1], points[currPoint][2]);
	}

	glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

}


__inline__ void render_submesh(int meshId, int submeshCount, struct CalRenderer * pCalRenderer, float meshVertices[30000][3], float meshNormals[30000][3], float meshTextureCoordinates[30000][2], CalIndex meshFaces[50000][3])
{
	int submeshId;
	int vertexCount=0;
	int textureCoordinateCount=0;
	int faceCount=0;

	for(submeshId = 0; submeshId < submeshCount; submeshId++) {
		// select mesh and submesh for further data access
		if(CalRenderer_SelectMeshSubmesh(pCalRenderer,meshId, submeshId)) {
			// get the transformed vertices of the submesh
			vertexCount = CalRenderer_GetVertices(pCalRenderer,&meshVertices[0][0]);

			// get the transformed normals of the submesh
			CalRenderer_GetNormals(pCalRenderer,&meshNormals[0][0]);

			// get the texture coordinates of the submesh
			textureCoordinateCount = CalRenderer_GetTextureCoordinates(pCalRenderer,0,&meshTextureCoordinates[0][0]);

			// get the faces of the submesh
			faceCount = CalRenderer_GetFaces(pCalRenderer, &meshFaces[0][0]);

			// set the vertex and normal buffers
			glVertexPointer(3, GL_FLOAT, 0, &meshVertices[0][0]);
			glNormalPointer(GL_FLOAT, 0, &meshNormals[0][0]);

			// draw the submesh
			glTexCoordPointer(2, GL_FLOAT, 0, &meshTextureCoordinates[0][0]);

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


void cal_render_actor(actor *act)
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
#ifdef	NEW_ACTOR_SCALE
	// actor model rescaling
	if(actors_defs[act->actor_type].actor_scale != 1.0){
		glScalef(actors_defs[act->actor_type].actor_scale, actors_defs[act->actor_type].actor_scale, actors_defs[act->actor_type].actor_scale);
	}
#endif	//NEW_ACTOR_SCALE
	// the dynamic scaling
	if(act->scale != 1.0f){
		glScalef(act->scale,act->scale,act->scale);
	}

#ifdef	DYNAMIC_ANIMATIONS
	if(act->last_anim_update < cur_time)
#ifdef	NEW_ACTOR_ANIMATION
		if(act->cur_anim.duration_scale > 0.0f)
			CalModel_Update(act->calmodel, (((cur_time-act->last_anim_update)*act->cur_anim.duration_scale)/1000.0));
#else
		CalModel_Update(act->calmodel,((cur_time-act->last_anim_update)/1000.0));
#endif
#ifdef MISSILES
	rotate_actor_bones(pActor);
#endif // MISSILES
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
#ifdef	ALPHA_ACTORS
			if(!act->ghost && act->has_alpha){
				//glEnable(GL_ALPHA_TEST);
				//glAlphaFunc(GL_GREATER,0.06f);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				//glDisable(GL_CULL_FACE);
			}
#endif	//ALPHA_ACTORS

			// will use vertex arrays, so enable them
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
				
#ifdef NEW_LIGHTING
				if (use_new_lighting)
				{
					if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].weapon[act->cur_weapon].mesh_index))
					{
//						printf("1\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].weapon[act->cur_weapon].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].weapon[act->cur_weapon].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].weapon[act->cur_weapon].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].weapon[act->cur_weapon].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].weapon[act->cur_weapon].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].shield[act->cur_shield].mesh_index))
					{
//						printf("2\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].shield[act->cur_shield].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].shield[act->cur_shield].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].shield[act->cur_shield].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].shield[act->cur_shield].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].shield[act->cur_shield].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].head[act->head].mesh_index))
					{
//						printf("3\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].head[act->head].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].head[act->head].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].head[act->head].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].head[act->head].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].head[act->head].shininess);
					}
/*
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].legs[act->legs].mesh_index))
					{
						printf("4\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].legs[act->legs].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].legs[act->legs].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].legs[act->legs].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].legs[act->legs].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].legs[act->legs].shininess);
					}
*/
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].shirt[act->shirt].mesh_index))
					{
//						printf("5: %s,%s\n  %d: %f,%f,%f / %f,%f,%f / %f,%f,%f\n", act->actor_name, act->skin_name, &(actors_defs[act->actor_type].shirt[act->shirt]), actors_defs[act->actor_type].shirt[act->shirt].ambient[0], actors_defs[act->actor_type].shirt[act->shirt].ambient[1], actors_defs[act->actor_type].shirt[act->shirt].ambient[2], actors_defs[act->actor_type].shirt[act->shirt].diffuse[0], actors_defs[act->actor_type].shirt[act->shirt].diffuse[1], actors_defs[act->actor_type].shirt[act->shirt].diffuse[2], actors_defs[act->actor_type].shirt[act->shirt].specular[0], actors_defs[act->actor_type].shirt[act->shirt].specular[1], actors_defs[act->actor_type].shirt[act->shirt].specular[2]);
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].shirt[act->shirt].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].shirt[act->shirt].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].shirt[act->shirt].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].shirt[act->shirt].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].shirt[act->shirt].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].legs[act->pants].mesh_index))
					{
//						printf("6\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].legs[act->pants].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].legs[act->pants].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].legs[act->pants].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].legs[act->pants].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].legs[act->pants].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].skin[act->skin].mesh_index))
					{
//						printf("7\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].skin[act->skin].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].skin[act->skin].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].skin[act->skin].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].skin[act->skin].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].skin[act->skin].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].hair[act->hair].mesh_index))
					{
//						printf("8\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].hair[act->hair].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].hair[act->hair].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].hair[act->hair].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].hair[act->hair].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].hair[act->hair].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].boots[act->boots].mesh_index))
					{
//						printf("9\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].boots[act->boots].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].boots[act->boots].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].boots[act->boots].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].boots[act->boots].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].boots[act->boots].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].helmet[act->helmet].mesh_index))
					{
//						printf("10\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].helmet[act->helmet].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].helmet[act->helmet].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].helmet[act->helmet].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].helmet[act->helmet].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].helmet[act->helmet].shininess);
					}
					else if (_coremesh == CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].cape[act->cape].mesh_index))
					{
//						printf("11\n");					
						glMaterialfv(GL_FRONT, GL_AMBIENT, actors_defs[act->actor_type].cape[act->cape].ambient);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, actors_defs[act->actor_type].cape[act->cape].diffuse);
						glMaterialfv(GL_FRONT, GL_SPECULAR, actors_defs[act->actor_type].cape[act->cape].specular);
						glMaterialfv(GL_FRONT, GL_EMISSION, actors_defs[act->actor_type].cape[act->cape].emission);
						glMaterialf(GL_FRONT, GL_SHININESS, actors_defs[act->actor_type].cape[act->cape].shininess);
					}
					else
					{
						set_material_defaults();
					}
				}
#endif
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
					if(glow>0){
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
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces);
						glPopMatrix();

						glColor4f(glow_colors[glow].r, glow_colors[glow].g, glow_colors[glow].b, 0.85f);
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces);
						glColor4f(glow_colors[glow].r, glow_colors[glow].g, glow_colors[glow].b, 0.99f);
						glPushMatrix();
						glScalef(1.01f, 1.01f, 1.01f);
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces);
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
						render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces);
					}
					if(boneid >= 0){
						//if this was a weapon or shield, restore the transformation matrix
						glPopMatrix();
					}
				} else {
					// non-enhanced actors, or enhanced without attached meshes
					render_submesh(meshId, submeshCount, pCalRenderer, meshVertices, meshNormals, meshTextureCoordinates, meshFaces);
				}
			}

			// clear vertex array state
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#ifdef	ALPHA_ACTORS
			if(!act->ghost && act->has_alpha){
				//glDisable(GL_ALPHA_TEST);
				//glEnable(GL_CULL_FACE);
				glDisable(GL_BLEND);
			}
#endif	//ALPHA_ACTORS

			// end the rendering
			CalRenderer_EndRendering(pCalRenderer);
		}
#ifdef DEBUG
	}
#endif

	glColor3f(1,1,1);

#ifdef DEBUG
  	glDisable(GL_LIGHTING);
  	glDisable(GL_DEPTH_TEST);
  	glDisable(GL_TEXTURE_2D);

  	if(render_skeleton) cal_render_bones(act);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
#endif
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
