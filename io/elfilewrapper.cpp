
#include "elfilewrapper.h"
#include "elfile.hpp"
#include "eldatasource.hpp"
#include "../elc_private.h"
#include "../errors.h"
#include "cal_animation_cache.hpp"

namespace eternal_lands
{

	extern "C" void add_zip_archive(const char* file_name, const char* path, int replace)
	{
		try
		{
			el_file::add_zip_archive(file_name, path, replace == 1);
		}
		CATCH_AND_LOG_EXCEPTIONS
	}

	extern "C" void add_paths()
	{
		try
		{
//			el_file::add_path(std::string(get_path_config()) + std::string("custom/"));		// We don't want all searches looking in the custom dir
			el_file::add_path(std::string(get_path_updates()));
			el_file::add_path(datadir);
		}
		CATCH_AND_LOG_EXCEPTIONS
	}

	extern "C" el_file* el_open(const char* file_name)
	{
		el_file* file;

		try
		{
			file = new el_file(file_name, true);

			return file;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" el_file* el_open_custom(const char* file_name)
	{
		el_file* file;

		try
		{
			file = new el_file(file_name, true, get_path_config_base());		// The /custom/ is already on the front

			return file;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" el_file* el_open_anywhere(const char* file_name)
	{
		el_file* file;

		try
		{
			file = new el_file (file_name, true, get_path_config());

			return file;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" el_file* el_open_no_decompress(const char* file_name)
	{
		el_file* file;

		try
		{
			file = new el_file(file_name, false);

			return file;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" int el_read(el_file* file, int size, void* buffer)
	{
		if (file == 0)
		{
			return -1;
		}
		try
		{
			return file->read(size, buffer);
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int el_seek(el_file* file, int offset, int seek_type)
	{
		if (file == 0)
		{
			return -1;
		}
		try
		{
			return file->seek(offset, seek_type);
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int el_tell(el_file* file)
	{
		if (file == 0)
		{
			return -1;
		}
		try
		{
			return file->tell();
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int el_get_size(el_file* file)
	{
		if (file == 0)
		{
			return -1;
		}
		try
		{
			return file->get_size();
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" void el_close(el_file* file)
	{
		if (file == 0)
		{
			return;
		}
		delete file;
		file = 0;
	}

	extern "C" void* el_get_pointer(el_file* file)
	{
		if (file == 0)
		{
			return 0;
		}
		try
		{
			return file->get_pointer();
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" int el_file_exists(const char* file_name)
	{
		try
		{
			return el_file::file_exists (file_name);
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" int el_custom_file_exists(const char* file_name)
	{
		try
		{
			return el_file::file_exists (file_name, get_path_config_base());		// The /custom/ is already on the front
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" int el_file_exists_anywhere(const char* file_name)
	{
		try
		{
			return el_file::file_exists(file_name, get_path_config());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

//****************************************************************************//
// CalLoader wrapper functions definition                                     //
//****************************************************************************//

	extern "C" CalCoreAnimation *CalLoader_ELLoadCoreAnimation(CalLoader *self,
		const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreAnimation *core_animation = explicitIncRef(self->loadCoreAnimation(file).get());

			if (core_animation)
			{
				core_animation->setFilename(strFilename);
			}
			
			return core_animation;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" CalCoreMaterial *CalLoader_ELLoadCoreMaterial(CalLoader *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreMaterial *core_material = explicitIncRef(self->loadCoreMaterial(file).get());

			if (core_material)
			{
				core_material->setFilename(strFilename);
			}
			
			return core_material;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" CalCoreMesh *CalLoader_ELLoadCoreMesh(CalLoader *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreMesh *core_mesh = explicitIncRef(self->loadCoreMesh(file).get());

			if (core_mesh)
			{
				core_mesh->setFilename(strFilename);
			}
			
			return core_mesh;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" CalCoreSkeleton *CalLoader_ELLoadCoreSkeleton(CalLoader *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			return explicitIncRef(self->loadCoreSkeleton(file).get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

#ifndef CACHE_ANIMATIONS
	extern "C" int CalCoreModel_ELLoadCoreAnimation(CalCoreModel *self, const char *strFilename)
#else // CACHE_ANIMATIONS
	extern "C" int CalCoreModel_ELLoadCoreAnimation(CalCoreModel *self, const char *strFilename, float scale)
#endif // CACHE_ANIMATIONS
	{
		assert(self);
		try
		{
#ifndef CACHE_ANIMATIONS
			el_data_source file(strFilename);

			CalCoreAnimationPtr core_animation = CalLoader::loadCoreAnimation(file, self->getCoreSkeleton());

			if (core_animation)
			{
				core_animation->setFilename(strFilename);
			}
			else
#else // CACHE_ANIMATIONS
 			CalCoreAnimationPtr core_animation = CalAnimationCache::loadAnimation(strFilename, scale);

			if (!core_animation)
#endif // CACHE_ANIMATIONS
			{
				return -1;
			}

			return self->addCoreAnimation(core_animation.get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int CalCoreModel_ELLoadCoreMaterial(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

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
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int CalCoreModel_ELLoadCoreMesh(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

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
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" CalBoolean CalCoreModel_ELLoadCoreSkeleton(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreSkeletonPtr core_skeleton = CalLoader::loadCoreSkeleton(file);

			if (!core_skeleton)
			{
				return False;
			}

			self->setCoreSkeleton(core_skeleton.get());

			return True;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(False);
	}

}

