#ifndef __cal3d_wrap_h__
#define __cal3d_wrap_h__


class Viewer
{
// member variables
protected:
  int m_width;
  int m_height;
  bool m_bFullscreen;
  int m_mouseX;
  int m_mouseY;
  float m_tiltAngle;
  float m_twistAngle;
  float m_distance;
  bool m_bLeftMouseButtonDown;
  bool m_bRightMouseButtonDown;
  unsigned int m_lastTick;
  bool m_bPaused;
  CalCoreModel m_calCoreModel;
  CalModel m_calModel;
  float m_scale;
  int m_currentAnimationId;
  float m_leftAnimationTime;
  float m_blendTime;
  float m_lodLevel;
  int m_vertexCount;
  int m_faceCount;

// constructors/destructor
public:
	Viewer();
	virtual ~Viewer();
	
// member functions
public:
  void onIdle();
  bool onInit();
  bool onCreate();
  GLuint loadTexture(const std::string& strFilename);
  bool parseModelConfiguration(const std::string& strFilename);
  void renderModel();
};

extern Viewer theViewer;

#endif
