
#include "cal3d_io_wrapper.h"
#include "../elc_private.h"
#include "../errors.h"
#include <cal3d/global.h>
#include <cal3d/cal3d.h>
#include "cal3d/coretrack.h"
#include <iostream>
#include "elfilewrapper.h"

//****************************************************************************//
// CalLoader wrapper functions definition                                     //
//****************************************************************************//

class ElDataSource: public CalDataSource
{
	private:
		el_file_ptr m_file;

	public:
		ElDataSource(const std::string &file_name)
		{
			m_file = el_open(file_name.c_str());
		}

		virtual ~ElDataSource()
		{
			el_close(m_file);
		}

		virtual bool ok() const
		{
			return m_file != 0;
		}

		virtual void setError() const
		{
		}

		virtual bool readBytes(void* pBuffer, int length)
		{
			return el_read(m_file, length, pBuffer) == length;
		}

		virtual bool readShort(short& value)
		{
			Sint16 tmp;
			int length;

			length = el_read(m_file, sizeof(Sint16), &tmp);

			value = SDL_SwapLE16(tmp);

			return length == sizeof(Sint16);
		}

		virtual bool readFloat(float &value)
		{
#ifdef FASTER_STARTUP
			return el_read_float(m_file, &value);
#else
			float tmp;
			int length;

			length = el_read(m_file, sizeof(float), &tmp);

			value = SwapLEFloat(tmp);

			return length == sizeof(float);
#endif
		}

		virtual bool readInteger(int &value)
		{
#ifdef FASTER_STARTUP
			return el_read_int(m_file, &value);
#else
			Sint32 tmp;
			int length;

			length = el_read(m_file, sizeof(Sint32), &tmp);

			value = SDL_SwapLE32(tmp);

			return length == sizeof(Sint32);
#endif
		}

		virtual bool readString(std::string &strValue)
		{
			char* str;
			int length;

			if (readInteger(length))
			{
				if (length >= 0)
				{

					str = new char[length];

					el_read(m_file, length, str);

					strValue = str;

					delete [] str;

					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

};

class CalAnimationCache
{
	private:
		typedef std::pair<std::string,float> AnimationKey;
		typedef std::map<AnimationKey, CalCoreAnimationPtr> AnimationsMap;
		
		AnimationsMap m_animations;

		CalAnimationCache()
		{
		}


		//The m_animations map will free itself when the CalAnimationCache singleton goes out of 
		//scope. However, for some reason the tracks/keyframes of that animation are not managed with
		//reference-counted objects (like everything else in Cal3D). So, we have to explicitly delete them.
		//At that point, it's probably best not to leave stale animation objects in the map, so we remove them as well.
		~CalAnimationCache()
		{
			while (!m_animations.empty()) {
				free_and_remove_animation(m_animations.begin());
			}
		}

		//This function should work externally, too, although we never delete items from the animations cache.
		void free_and_remove_animation(const AnimationsMap::iterator& animIt) {
			//Remove all keyframes (via destroy()) from each track individually.
			std::list<CalCoreTrack*>& trackList = animIt->second->getListCoreTrack();
			for (auto track: trackList)
			{
				track->destroy();
				delete track;
			}

			//Clear the track list too.
			trackList.clear();

			//Now eject this item from the map.
			m_animations.erase(animIt);
		}
		
		static CalAnimationCache & instance()
		{
			static CalAnimationCache cache;

			return cache;
		}
		
	public:
		
		static CalCoreAnimationPtr loadAnimation(const std::string &fileName, float scale)
		{
			AnimationsMap &anims = instance().m_animations;
			AnimationKey key(fileName, scale);
			AnimationsMap::iterator it = anims.find(key);

			if (it != anims.end())
			{
				return it->second;
			}
			else
			{
				ElDataSource file(fileName);
				
				CalCoreAnimationPtr anim_ptr = CalLoader::loadCoreAnimation(file, 0);

				if (anim_ptr)
				{
					CalCoreAnimation_Scale(anim_ptr.get(), scale);
					anim_ptr->setFilename(fileName);
				}
				
				anims[key] = anim_ptr;

				return anim_ptr;
			}
		}
		
};

extern "C" CalCoreAnimation *CalLoader_ELLoadCoreAnimation(CalLoader *self,
	const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreAnimation *core_animation = explicitIncRef(self->loadCoreAnimation(file).get());

	if (core_animation)
	{
		core_animation->setFilename(strFilename);
	}
			
	return core_animation;
}

extern "C" CalCoreMaterial *CalLoader_ELLoadCoreMaterial(CalLoader *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreMaterial *core_material = explicitIncRef(self->loadCoreMaterial(file).get());

	if (core_material)
	{
		core_material->setFilename(strFilename);
	}
			
	return core_material;
}

extern "C" CalCoreMesh *CalLoader_ELLoadCoreMesh(CalLoader *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreMesh *core_mesh = explicitIncRef(self->loadCoreMesh(file).get());

	if (core_mesh)
	{
		core_mesh->setFilename(strFilename);
	}
			
	return core_mesh;
}

extern "C" CalCoreSkeleton *CalLoader_ELLoadCoreSkeleton(CalLoader *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	return explicitIncRef(self->loadCoreSkeleton(file).get());
}

extern "C" int CalCoreModel_ELLoadCoreAnimation(CalCoreModel *self, const char *strFilename, float scale)
{
	assert(self);

	CalCoreAnimationPtr core_animation = CalAnimationCache::loadAnimation(strFilename, scale);

	if (!core_animation)
	{
		return -1;
	}

	return self->addCoreAnimation(core_animation.get());
}

extern "C" int CalCoreModel_ELLoadCoreMaterial(CalCoreModel *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreMaterialPtr core_material = CalLoader::loadCoreMaterial(file);

	if (!core_material)
	{
		return -1;
	}
	else
	{
		core_material->setFilename(strFilename);
	}

	return self->addCoreMaterial(core_material.get());
}

extern "C" int CalCoreModel_ELLoadCoreMesh(CalCoreModel *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreMeshPtr core_mesh = CalLoader::loadCoreMesh(file);

	if (!core_mesh)
	{
		return -1;
	}
	else
	{
		core_mesh->setFilename(strFilename);
	}

	return self->addCoreMesh(core_mesh.get());
}

extern "C" CalBoolean CalCoreModel_ELLoadCoreSkeleton(CalCoreModel *self, const char *strFilename)
{
	assert(self);

	ElDataSource file(strFilename);

	CalCoreSkeletonPtr core_skeleton = CalLoader::loadCoreSkeleton(file);

	if (!core_skeleton)
	{
		return False;
	}

	self->setCoreSkeleton(core_skeleton.get());

	return True;
}

extern "C" void set_invert_v_coord()
{
	CalLoader::setLoadingMode(LOADER_INVERT_V_COORD);
}


