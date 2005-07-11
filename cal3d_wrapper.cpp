#include <cal3d/cal3d.h>
#include <cal3d/cal3d_wrapper.h>

extern "C" CAL3D_WRAPPER_API CalMesh *CalModel_GetAttachedMesh(CalModel *self,int i)
{
	return self->getVectorMesh()[i];
}

extern "C" CAL3D_WRAPPER_API void CalCoreSkeleton_Scale(CalCoreSkeleton *self,float factor)
{
	self->scale(factor);
}


extern "C" CAL3D_WRAPPER_API enum Boolean CalMixer_ExecuteAction_Stop(CalMixer *self, int id, float delayIn, float delayOut)
{
	return self->executeAction(id, delayIn, delayOut, 1.0f,true) ? True : False;
}

extern "C" CAL3D_WRAPPER_API void CalMixer_RemoveAction(CalMixer *self,int id)
{
	self->removeAction(id);
}

extern "C" CAL3D_WRAPPER_API void CalCoreAnimation_Scale(CalCoreAnimation *self, float factor)
{
	self->scale(factor);
}

extern "C" CAL3D_WRAPPER_API void CalCoreMesh_Scale(CalCoreMesh *self,float factor)
{
	self->scale(factor);
}

