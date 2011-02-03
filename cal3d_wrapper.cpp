#include <cal3d/cal3d.h>
#include <cal3d/coretrack.h>
#include <cal3d/corekeyframe.h>

//#include <cal3d/cal3d_wrapper.h>
#include "cal3d_wrapper.h"

extern "C" CAL3D_WRAPPER_API CalCoreTrack *CalCoreAnimation_GetCoreTrack(CalCoreAnimation *self, int coreBoneId)
{
	std::list<CalCoreTrack *> &track_list = self->getListCoreTrack();
	std::list<CalCoreTrack *>::iterator it;
	for (it = track_list.begin(); it != track_list.end(); ++it)
		if ((*it)->getCoreBoneId() == coreBoneId)
			return *it;
	return NULL;
}

extern "C" CAL3D_WRAPPER_API void CalCoreKeyframe_GetTranslation(CalCoreKeyframe *self, CalVector *outV)
{
	*outV = self->getTranslation();
}

extern "C" CAL3D_WRAPPER_API void CalCoreKeyframe_SetTranslation(CalCoreKeyframe *self, CalVector *pTranslation)
{
	self->setTranslation(*pTranslation);
}

extern "C" CAL3D_WRAPPER_API void CalCoreKeyframe_GetRotation(CalCoreKeyframe *self, CalQuaternion *outQ)
{
	*outQ = self->getRotation();
}

extern "C" CAL3D_WRAPPER_API void CalCoreKeyframe_SetRotation(CalCoreKeyframe *self, CalQuaternion *pRotation)
{
	self->setRotation(*pRotation);
}

extern "C" CAL3D_WRAPPER_API int CalCoreSkeleton_GetCoreBonesNumber(struct CalCoreSkeleton *self)
{
	return self->getVectorCoreBone().size();
}

extern "C" CAL3D_WRAPPER_API int CalCoreTrack_GetCoreKeyframeCount(CalCoreTrack *self)
{
	return self->getCoreKeyframeCount();
}

extern "C" CAL3D_WRAPPER_API CalCoreKeyframe * CalCoreTrack_GetCoreKeyframe(CalCoreTrack *self, int i)
{
	return self->getCoreKeyframe(i);
}

extern "C" CAL3D_WRAPPER_API enum CalBoolean CalMixer_ExecuteAction_Stop(CalMixer *self, int id, float delayIn, float delayOut)
{
	return self->executeAction(id, delayIn, delayOut, 1.0f,true) ? True : False;
}

extern "C" CAL3D_WRAPPER_API enum CalBoolean CalMixer_ExecuteActionExt(CalMixer *self, int id, float delayIn, float delayOut, float weight, int autoLock){

	//Execute Action id
	//starting after delayIn, ending before delayOut
	//with a specified weight
	//and removing the action at the end if autolock is false

	return self->executeAction(id, delayIn, delayOut, weight,(bool) autoLock) ? True : False;
}

extern "C" CAL3D_WRAPPER_API void CalMixer_SetAnimationTime(CalMixer *self, float animationTime)
{
	self->setAnimationTime(animationTime);
}

extern "C" CAL3D_WRAPPER_API CalMesh *CalModel_GetAttachedMesh(CalModel *self,int i)
{
	return self->getVectorMesh()[i];
}

extern "C" CAL3D_WRAPPER_API void CalQuaternion_Invert(CalQuaternion *self)
{
	self->invert();
}

extern "C" CAL3D_WRAPPER_API int CalSkeleton_GetBonesNumber(struct CalSkeleton *self)
{
	return self->getVectorBone().size();
}
