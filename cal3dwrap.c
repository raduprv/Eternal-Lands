#include "global.h"
#include <cal3d/cal3d.h>


CalCoreModel testcore;
CalModel testmodel;
int testanim;
int testmesh;
int testmaterial;
int lasttick;

const int TEST_MATERIAL_THREAD = 0;
const int TEST_MATERIAL_SET = 0;

extern "C" {
	
void create_cal3d_model() {

	if(!testcore.create("test model")) {
		printf("error creating test core model\n");
	}
	
	if(!testcore.loadCoreSkeleton("cal3d/test.csf")) {
		printf("error loading skeleton\n");
	}

	testanim = testcore.loadCoreAnimation("cal3d/testbaked.caf");

	if(testanim == -1) {
		printf("error loading animation\n");
	}

	testmesh = testcore.loadCoreMesh("cal3d/testWolf.cmf");

	if(testmesh == -1) {
		printf("error loading mesh\n");
	}

	testmaterial = testcore.loadCoreMaterial("cal3d/testchimeran.crf");

	if(testmaterial == -1) {
		printf("error loading material\n");
	}

	testcore.createCoreMaterialThread(TEST_MATERIAL_THREAD);

	testcore.setCoreMaterialId(TEST_MATERIAL_THREAD, TEST_MATERIAL_SET, testmaterial);

	//I would load textures, but we don't have any?
}

void destroy_cal3d_model() {
	testmodel.destroy();
	testcore.destroy();
}

void init_cal3d_model() {
	if(!testmodel.create(&testcore)) {
		printf("error creating test model\n");
	}

	if(!testmodel.attachMesh(testmesh)) {
		printf("error attaching mesh\n");
	}

	testmodel.setLodLevel(0.5f);

	testmodel.setMaterialSet(TEST_MATERIAL_SET);

	testmodel.getMixer()->blendCycle(testanim, 1.0f, 1.0f);

	lasttick = SDL_GetTicks();
}

void render_cal3d_model(actor * actor_id) {
	CalRenderer *pCalRenderer;
	pCalRenderer = testmodel.getRenderer();

	if(!pCalRenderer->beginRendering()) {
		printf("rendering broke?\n");
		return;
	}
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glScalef(0.20,0.20,0.20);

	// we will use vertex arrays, so enable them
	//glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
  
	// get the number of meshes
	int meshCount;
	meshCount = pCalRenderer->getMeshCount();
  
	// render all meshes of the model
	int meshId;
	for(meshId = 0; meshId < meshCount; meshId++)
		{
			// get the number of submeshes
			int submeshCount;
			submeshCount = pCalRenderer->getSubmeshCount(meshId);
			
			// render all submeshes of the mesh
			int submeshId;
			for(submeshId = 0; submeshId < submeshCount; submeshId++) {
				// select mesh and submesh for further data access
				if(pCalRenderer->selectMeshSubmesh(meshId, submeshId)) {
					unsigned char meshColor[4];
					GLfloat materialColor[4];
				  
					// set the material ambient color
					pCalRenderer->getAmbientColor(&meshColor[0]);
					materialColor[0] = meshColor[0] / 255.0f;  materialColor[1] = meshColor[1] / 255.0f; materialColor[2] = meshColor[2] / 255.0f; materialColor[3] = meshColor[3] / 255.0f;
					glMaterialfv(GL_FRONT, GL_AMBIENT, materialColor);
				  
					// set the material diffuse color
					pCalRenderer->getDiffuseColor(&meshColor[0]);
					materialColor[0] = meshColor[0] / 255.0f;  materialColor[1] = meshColor[1] / 255.0f; materialColor[2] = meshColor[2] / 255.0f; materialColor[3] = meshColor[3] / 255.0f;
					glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);
				  
					// set the material specular color
					pCalRenderer->getSpecularColor(&meshColor[0]);
					materialColor[0] = meshColor[0] / 255.0f;  materialColor[1] = meshColor[1] / 255.0f; materialColor[2] = meshColor[2] / 255.0f; materialColor[3] = meshColor[3] / 255.0f;
					glMaterialfv(GL_FRONT, GL_SPECULAR, materialColor);
				  
					// set the material shininess factor
					float shininess;
					shininess = pCalRenderer->getShininess();
					glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
				  
					// get the transformed vertices of the submesh
					static float meshVertices[30000][3];
					int vertexCount;
					vertexCount = pCalRenderer->getVertices(&meshVertices[0][0]);
				  
					// get the transformed normals of the submesh
					static float meshNormals[30000][3];
					pCalRenderer->getNormals(&meshNormals[0][0]);
				  
					// get the texture coordinates of the submesh
					static float meshTextureCoordinates[30000][2];
					int textureCoordinateCount;
					textureCoordinateCount = pCalRenderer->getTextureCoordinates(0, &meshTextureCoordinates[0][0]);

					// get the faces of the submesh
					static CalIndex meshFaces[50000][3];
					int faceCount;
					faceCount = pCalRenderer->getFaces(&meshFaces[0][0]);

					// set the vertex and normal buffers
					glVertexPointer(3, GL_FLOAT, 0, &meshVertices[0][0]);
					glNormalPointer(GL_FLOAT, 0, &meshNormals[0][0]);
				  
					// draw the submesh
				  
					if(sizeof(CalIndex)==2)
						glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_SHORT, &meshFaces[0][0]);
					else
						glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, &meshFaces[0][0]);
				}
			}
		}

	// clear vertex array state
	glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_VERTEX_ARRAY);
	//fix the md2 models
	glNormal3f(0.0f,0.0f,1.0f);
  
	// end the rendering
	pCalRenderer->endRendering();
	glPopMatrix();
}

void update_cal3d_model() {
	unsigned int tick;
	tick = SDL_GetTicks();
	
	float elapsedSeconds;
	elapsedSeconds = (float)(tick - lasttick) / 1000.0f;
  
	testmodel.update(elapsedSeconds);
	
	lasttick = tick;
}

}
