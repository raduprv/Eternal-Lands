#include "global.h"
#include <cal3d/cal3d.h>
#include "cal3dwrap.h"


/*******************************************************
    WARNING: This cannot be used in its current form
             because it was ripped directly from GPL code
***********************************************************/

Viewer theViewer;

Viewer::Viewer()
{
  m_width = 640;
  m_height = 480;
  m_bFullscreen = false;
  m_mouseX = 0;
  m_mouseY = 0;
  m_tiltAngle = -70.0f;
  m_twistAngle = -45.0f;
  m_distance = 270.0f;
  m_bLeftMouseButtonDown = false;
  m_bRightMouseButtonDown = false;
  m_lastTick = 0;
  m_bPaused = false;
  m_scale = 1.0f;
  m_blendTime = 0.3f;
  m_lodLevel = 1.0f;
  m_vertexCount = 0;
  m_faceCount = 0;
}

Viewer::~Viewer()
{
}

extern "C" {
	void cal3d_init() {
		theViewer.onCreate();
		theViewer.onInit();
	}

	void cal3d_render() {
		theViewer.renderModel();
	}

	void cal3d_tick() {
		theViewer.onIdle();
	}
}

bool Viewer::onCreate()
{

	// parse the model configuration file
	if(!parseModelConfiguration("cal3d/test.cfg")) return false;


  // make one material thread for each material
  // NOTE: this is not the right way to do it, but this viewer can't do the right
  // mapping without further information on the model etc., so this is the only
  // thing we can do here.
  int materialId;
  for(materialId = 0; materialId < m_calCoreModel.getCoreMaterialCount(); materialId++)
  {
    // create the a material thread
    m_calCoreModel.createCoreMaterialThread(materialId);

    // initialize the material thread
    m_calCoreModel.setCoreMaterialId(materialId, 0, materialId);
  }

  // create the model instance from the loaded core model
  if(!m_calModel.create(&m_calCoreModel))
  {
    CalError::printLastError();
    return false;
  }

  return true;
}

bool Viewer::onInit()
{
  // load all textures and store the opengl texture id in the corresponding map in the material
  int materialId;
  for(materialId = 0; materialId < m_calCoreModel.getCoreMaterialCount(); materialId++)
  {
    // get the core material
    CalCoreMaterial *pCoreMaterial;
    pCoreMaterial = m_calCoreModel.getCoreMaterial(materialId);
	
    // loop through all maps of the core material
    int mapId;
    for(mapId = 0; mapId < pCoreMaterial->getMapCount(); mapId++)
    {
      // get the filename of the texture
      std::string strFilename;
      strFilename = pCoreMaterial->getMapFilename(mapId);

      // load the texture from the file
      GLuint textureId;
      textureId = loadTexture(strFilename);

      // store the opengl texture id in the user data of the map
      pCoreMaterial->setMapUserData(mapId, (Cal::UserData)textureId);
    }
  }
  // attach all meshes to the model
  int meshId;
  for(meshId = 0; meshId < m_calCoreModel.getCoreMeshCount(); meshId++)
  {
    m_calModel.attachMesh(meshId);
  }

  // set the material set of the whole model
  m_calModel.setMaterialSet(0);

  // set initial animation state
  if(m_calCoreModel.getCoreAnimationCount() > 0)
  {
    m_currentAnimationId = 0;
    m_leftAnimationTime = m_calCoreModel.getCoreAnimation(m_currentAnimationId)->getDuration() - m_blendTime;
    if(m_calCoreModel.getCoreAnimationCount() > 1)
    {
      m_calModel.getMixer()->executeAction(m_currentAnimationId, 0.0f, m_blendTime);
    }
    else
    {
      m_calModel.getMixer()->blendCycle(m_currentAnimationId, 1.0f, 0.0f);
    }
  }
  else
  {
    m_currentAnimationId = -1;
    m_leftAnimationTime = -1.0f;
  }

  // we're done
  std::cout << "Initialization done." << std::endl;
  std::cout << std::endl;
  std::cout << "Quit the viewer by pressing 'q' or ESC" << std::endl;
  std::cout << std::endl;

  m_lastTick = SDL_GetTicks();

  return true;
}

GLuint Viewer::loadTexture(const std::string& strFilename)
{
  GLuint textureId=0;
  if(stricmp(strrchr(strFilename.c_str(),'.'),".raw")==0)
  {

     // open the texture file
     std::ifstream file;
     file.open(strFilename.c_str(), std::ios::in | std::ios::binary);
     if(!file)
     {
       std::cerr << "Texture file '" << strFilename << "' not found." << std::endl;
       return 0;
     }

     // load the dimension of the texture
     int width;
     file.read((char *)&width, 4);
     int height;
     file.read((char *)&height, 4);
     int depth;
     file.read((char *)&depth, 4);

     // allocate a temporary buffer to load the texture to
     unsigned char *pBuffer;
     pBuffer = new unsigned char[2 * width * height * depth];
     if(pBuffer == 0)
     {
       std::cerr << "Memory allocation for texture '" << strFilename << "' failed." << std::endl;
       return 0;
     }

     // load the texture
     file.read((char *)pBuffer, width * height * depth);

     // explicitely close the file
     file.close();

     // flip texture around y-axis (-> opengl-style)
     int y;
     for(y = 0; y < height; y++)
     {
       memcpy(&pBuffer[(height + y) * width * depth], &pBuffer[(height - y - 1) * width * depth], width * depth);
     }
     
     // generate texture
     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

     glGenTextures(1, &textureId);
     glBindTexture(GL_TEXTURE_2D, textureId);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     
	 glTexImage2D(GL_TEXTURE_2D, 0, (depth == 3) ? GL_RGB : GL_RGBA, width, height, 0, (depth == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, &pBuffer[width * height * depth]);
     // free the allocated memory

     delete [] pBuffer;  
  }
  /*
  else if(stricmp(strrchr(strFilename.c_str(),'.'),".tga")==0)  
  {
	  CTga *Tga;
	  Tga = new CTga();
	  if(Tga->ReadFile(strFilename.c_str())==0)
	  {
		  Tga->Release();
		  return 0;
	  }

	  if(Tga->Bpp()!=32)
	  {
		  Tga->Release();
		  return 0;
	  }
     //Flip texture
     int width = Tga->GetSizeX();
     int height = Tga->GetSizeY();
     int depth = Tga->Bpp() / 8;

     char* texData = new char[width * height * depth];

     for (int y = 0; y < height; ++y)
     {
       memcpy(&texData[y * width * depth], 
              &((char*)Tga->GetPointer())[(height - y - 1) * width * depth], width * depth);
     }
    

     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

     glGenTextures(1, &textureId);

     glBindTexture(GL_TEXTURE_2D, textureId);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA , GL_UNSIGNED_BYTE, texData );

	  Tga->Release();

     delete [] texData;

  }
  */
  

  return textureId;
}



bool Viewer::parseModelConfiguration(const std::string& strFilename)
{
  // open the model configuration file
  std::ifstream file;
  file.open(strFilename.c_str(), std::ios::in | std::ios::binary);
  if(!file)
  {
    std::cerr << "Failed to open model configuration file '" << strFilename << "'." << std::endl;
    return false;
  }

  // create a core model instance
  if(!m_calCoreModel.create("dummy"))
  {
    CalError::printLastError();
    return false;
  }

  // parse all lines from the model configuration file
  int line;
  for(line = 1; ; line++)
  {
    // read the next model configuration line
    std::string strBuffer;
    std::getline(file, strBuffer);

    // stop if we reached the end of file
    if(file.eof()) break;

    // check if an error happend while reading from the file
    if(!file)
    {
      std::cerr << "Error while reading from the model configuration file '" << strFilename << "'." << std::endl;
      return false;
    }

    // find the first non-whitespace character
    std::string::size_type pos;
    pos = strBuffer.find_first_not_of(" \t");

    // check for empty lines
    if((pos == std::string::npos) || (strBuffer[pos] == '\n') || (strBuffer[pos] == '\r') || (strBuffer[pos] == 0)) continue;

    // check for comment lines
    if(strBuffer[pos] == '#') continue;

    // get the key
    std::string strKey;
    strKey = strBuffer.substr(pos, strBuffer.find_first_of(" =\t\n\r", pos) - pos);
    pos += strKey.size();

    // get the '=' character
    pos = strBuffer.find_first_not_of(" \t", pos);
    if((pos == std::string::npos) || (strBuffer[pos] != '='))
    {
      std::cerr << strFilename << "(" << line << "): Invalid syntax." << std::endl;
      return false;
    }

    // find the first non-whitespace character after the '=' character
    pos = strBuffer.find_first_not_of(" \t", pos + 1);

    // get the data
    std::string strData;
    strData = strBuffer.substr(pos, strBuffer.find_first_of("\n\r", pos) - pos);

    // handle the model creation
    if(strKey == "scale")
    {
      // set rendering scale factor
      m_scale = atof(strData.c_str());
    }
    else if(strKey == "skeleton")
    {
      // load core skeleton
      std::cout << "Loading skeleton '" << strData << "'..." << std::endl;
      if(!m_calCoreModel.loadCoreSkeleton(strData))
      {
        CalError::printLastError();
        return false;
      }
    }
    else if(strKey == "animation")
    {
      // load core animation
      std::cout << "Loading animation '" << strData << "'..." << std::endl;
      if(m_calCoreModel.loadCoreAnimation(strData) == -1)
      {
        CalError::printLastError();
        return false;
      }
    }
    else if(strKey == "mesh")
    {
      // load core mesh
      std::cout << "Loading mesh '" << strData << "'..." << std::endl;
      if(m_calCoreModel.loadCoreMesh(strData) == -1)
      {
        CalError::printLastError();
        return false;
      }
    }
    else if(strKey == "material")
    {
      // load core material
      std::cout << "Loading material '" << strData << "'..." << std::endl;
      if(m_calCoreModel.loadCoreMaterial(strData) == -1)
      {
        CalError::printLastError();
        return false;
      }
    }
    else
    {
      // everything else triggers an error message, but is ignored
      std::cerr << strFilename << "(" << line << "): Invalid syntax." << std::endl;
    }
  }

  // explicitely close the file
  file.close();

  return true;
}


void Viewer::renderModel()
{
  // get the renderer of the model
  CalRenderer *pCalRenderer;
  pCalRenderer = m_calModel.getRenderer();

  // begin the rendering loop
  if(pCalRenderer->beginRendering())
  {
    // set global OpenGL states
    //glEnable(GL_DEPTH_TEST);
    //glShadeModel(GL_SMOOTH);
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

    // we will use vertex arrays, so enable them
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_NORMAL_ARRAY);

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
      for(submeshId = 0; submeshId < submeshCount; submeshId++)
      {
        // select mesh and submesh for further data access
        if(pCalRenderer->selectMeshSubmesh(meshId, submeshId))
        {
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
          shininess = 50.0f; //pCalRenderer->getShininess();
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

          // set the texture coordinate buffer and state if necessary
          if((pCalRenderer->getMapCount() > 0) && (textureCoordinateCount > 0))
          {
			  //glEnable(GL_TEXTURE_2D);
			  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			  //glEnable(GL_COLOR_MATERIAL);

            // set the texture id we stored in the map user data
            glBindTexture(GL_TEXTURE_2D, (GLuint)pCalRenderer->getMapUserData(0));

            // set the texture coordinate buffer
            glTexCoordPointer(2, GL_FLOAT, 0, &meshTextureCoordinates[0][0]);
            glColor3f(1.0f, 1.0f, 1.0f);
          }

          // draw the submesh

		  if(sizeof(CalIndex)==2)
			  glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_SHORT, &meshFaces[0][0]);
		  else
			  glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, &meshFaces[0][0]);

          // disable the texture coordinate state if necessary
          if((pCalRenderer->getMapCount() > 0) && (textureCoordinateCount > 0))
          {
			  //glDisable(GL_COLOR_MATERIAL);
			  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			  //glDisable(GL_TEXTURE_2D);
          }

          // adjust the vertex and face counter
          m_vertexCount += vertexCount;
          m_faceCount += faceCount;
		}
      }
    }

    // clear vertex array state
    //glDisableClientState(GL_NORMAL_ARRAY);
    //glDisableClientState(GL_VERTEX_ARRAY);

    // clear light
    //glDisable(GL_LIGHTING);
    //glDisable(GL_LIGHT0);
    //glDisable(GL_DEPTH_TEST);

    // end the rendering
    pCalRenderer->endRendering();
  }

}

void Viewer::onIdle()
{
  // get the current tick value
  unsigned int tick;
  tick = SDL_GetTicks();

  // calculate the amount of elapsed seconds
  float elapsedSeconds;
  elapsedSeconds = (float)(tick - m_lastTick) / 1000.0f;

  // update the model if not paused
  if(!m_bPaused)
  {
    // check if the time has come to blend to the next animation
    if(m_calCoreModel.getCoreAnimationCount() > 1)
    {
      m_leftAnimationTime -= elapsedSeconds;
      if(m_leftAnimationTime <= m_blendTime)
      {
        // get the next animation
        m_currentAnimationId = (m_currentAnimationId + 1) % m_calCoreModel.getCoreAnimationCount();

        // fade in the new animation
        m_calModel.getMixer()->executeAction(m_currentAnimationId, m_leftAnimationTime, m_blendTime);

        // adjust the animation time left until next animation flip
        m_leftAnimationTime = m_calCoreModel.getCoreAnimation(m_currentAnimationId)->getDuration() - m_blendTime;
      }
    }

    m_calModel.update(elapsedSeconds);
  }

  // current tick will be last tick next round
  m_lastTick = tick;

  // update the screen
  //glutPostRedisplay();
}
