#include "global.h"
#include "cal.h"


void cal_actor_set_anim(int id,struct cal_anim anim)
{
	struct CalMixer *mixer;
	int i;
	if (actors_defs[actors_list[id]->actor_type].coremodel==NULL) return;
	if (actors_list[id]->cur_anim.anim_index==anim.anim_index) return;
	mixer=CalModel_GetMixer(actors_list[id]->calmodel);

	//Stop previous animation if needed
	if (actors_list[id]->IsOnIdle!=1) {
		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==0)) {
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_anim.anim_index,0.05);
		}

		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==1)) {
			CalMixer_RemoveAction(mixer,actors_list[id]->cur_anim.anim_index);
		}
	}

	if (actors_list[id]->IsOnIdle==1) {
		for (i=0;i<actors_defs[actors_list[id]->actor_type].group_count;++i) {
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_idle_anims[i].anim_index,0.05);
		}
	}

	if (anim.kind==0) 
		CalMixer_BlendCycle(mixer,anim.anim_index,1.0,0.05);
	else
		CalMixer_ExecuteAction_Stop(mixer,anim.anim_index,0.0,0.0);
	
	actors_list[id]->cur_anim=anim;
	actors_list[id]->anim_time=0.0;
	CalModel_Update(actors_list[id]->calmodel,0.0001);//Make changes take effect now
	
	if (actors_list[id]->cur_anim.anim_index==-1) actors_list[id]->busy=0;
	actors_list[id]->IsOnIdle=0;
}


struct cal_anim cal_load_anim(actor_types *act, char *str)
{
	char fname[255];
	struct cal_anim res;
	char temp[255];
	struct CalCoreAnimation *coreanim;
	
	sscanf(str,"%s%d",fname,&res.kind);
	res.anim_index=CalCoreModel_LoadCoreAnimation(act->coremodel,fname);
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);
	
	if (coreanim) {
		CalCoreAnimation_Scale(coreanim,act->scale);
		res.duration=CalCoreAnimation_GetDuration(coreanim);
	} else {
		sprintf(temp,"No Anim: %s\n",fname);
		log_error(temp);
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
}

void cal_render_actor(actor *act)
{
	struct CalRenderer *pCalRenderer;
	int meshCount,meshId,submeshCount,submeshId,vertexCount;
	int textureCoordinateCount,faceCount;
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
	int boneid=-1;
	float reverse_scale;
	char str[255];

	skel=CalModel_GetSkeleton(act->calmodel);

	//glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	//glScalef(1.2,1.2,1.2);
	// get the renderer of the model

	if (render_mesh) {
  		pCalRenderer = CalModel_GetRenderer(act->calmodel);
  		// begin the rendering loop
  		if(CalRenderer_BeginRendering(pCalRenderer)){
    			// set global OpenGL states

    			// we will use vertex arrays, so enable them
    			glEnableClientState(GL_VERTEX_ARRAY);
    			glEnableClientState(GL_NORMAL_ARRAY);

    			// get the number of meshes
    			meshCount = CalRenderer_GetMeshCount(pCalRenderer);

    			// render all meshes of the model
    			for(meshId = 0; meshId < meshCount; meshId++){
				// get the number of submeshes
      				submeshCount = CalRenderer_GetSubmeshCount(pCalRenderer,meshId);
				glPushMatrix();

				//Special treatment for weapons and shields only for enhanced models
				boneid=-1;
				if (act->is_enhanced_model) {
					_mesh=CalModel_GetAttachedMesh(act->calmodel,meshId);//Get current rendered mesh
					_coremesh=CalMesh_GetCoreMesh(_mesh);//Get the coremesh
					_weaponmesh=CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,actors_defs[act->actor_type].weapon[act->cur_weapon].mesh_index);
					_shieldmesh=CalCoreModel_GetCoreMesh(actors_defs[act->actor_type].coremodel,act->body_parts->shield_meshindex);
				
					if (_coremesh==_weaponmesh) boneid=26;//If it's a weapon snap to WeaponR bone
					if (_coremesh==_shieldmesh) boneid=21;//If it's a shield snap to WeaponL bone
					if (boneid!=-1) {
						reverse_scale=1.0/actors_defs[act->actor_type].skel_scale;
						CalSkeleton_GetBonePoints(skel,&points[0][0]);
						
						glTranslatef(points[boneid][0],points[boneid][1],points[boneid][2]);
						glScalef(reverse_scale,reverse_scale,reverse_scale);
						glTranslatef(-points[boneid][0],-points[boneid][1],-points[boneid][2]);
					}
				}
		
				// render all submeshes of the mesh
				for(submeshId = 0; submeshId < submeshCount; submeshId++) {
					// select mesh and submesh for further data access
					if(CalRenderer_SelectMeshSubmesh(pCalRenderer,meshId, submeshId)) 
                    
                    {
						// get the transformed vertices of the submesh
						vertexCount = CalRenderer_GetVertices(pCalRenderer,&meshVertices[0][0]);
						
						// get the transformed normals of the submesh
						CalRenderer_GetNormals(pCalRenderer,&meshNormals[0][0]);
	
	  	        			// get the texture coordinates of the submesh
       		  				textureCoordinateCount = CalRenderer_GetTextureCoordinates(pCalRenderer,0,&meshTextureCoordinates[0][0]);
       				 	
						// get the faces of the submesh
        	  				faceCount = CalRenderer_GetFaces(pCalRenderer,&meshFaces[0][0]);
	
	          				// set the vertex and normal buffers
	          				glVertexPointer(3, GL_FLOAT, 0, &meshVertices[0][0]);
	          				glNormalPointer(GL_FLOAT, 0, &meshNormals[0][0]);
	
    	   	 	  			// set the texture coordinate buffer and state if necessary
   						/*
						if((pCalRenderer->getMapCount() > 0) && (textureCoordinateCount > 0)) {
							glEnable(GL_TEXTURE_2D);
							glEnable(GL_COLOR_MATERIAL);
							// set the texture id we stored in the map user data
							glBindTexture(GL_TEXTURE_2D, (GLuint)pCalRenderer->getMapUserData(0));
							// set the texture coordinate buffer
           	 
            						glColor3f(1.0f, 1.0f, 1.0f);
          					}
			 			*/
	
        		  			// draw the submesh
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				  		glTexCoordPointer(2, GL_FLOAT, 0, &meshTextureCoordinates[0][0]);
						if(sizeof(CalIndex)==2)
        		    				glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_SHORT, &meshFaces[0][0]);
        		  			else
            						glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, &meshFaces[0][0]);
	
        	  				// disable the texture coordinate state if necessary
				  		/*
          					if((pCalRenderer->getMapCount() > 0) && (textureCoordinateCount > 0)) {
          	  					glDisable(GL_COLOR_MATERIAL);
            						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            						glDisable(GL_TEXTURE_2D);
          					}
						*/

          					// adjust the vertex and face counter
          					//m_vertexCount += vertexCount;
          					//m_faceCount += faceCount;
					}
				}
				glPopMatrix();
			}
			// clear vertex array state
    			glDisableClientState(GL_NORMAL_ARRAY);
    			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    			// end the rendering
    			CalRenderer_EndRendering(pCalRenderer);
		}
	}

  	glDisable(GL_LIGHTING);
  	glDisable(GL_DEPTH_TEST);
  	glDisable(GL_TEXTURE_2D);
  	
	glColor3f(1,1,1);
  	if (render_skeleton) cal_render_bones(act);
  	glEnable(GL_LIGHTING);
  	glEnable(GL_DEPTH_TEST);
  	glEnable(GL_TEXTURE_2D);
  	glPopMatrix();
	//glEnable(GL_TEXTURE_2D);
}
