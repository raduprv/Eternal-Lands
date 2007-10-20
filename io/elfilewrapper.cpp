#ifdef NEW_FILE_IO

#include "elfilewrapper.h"
#include "elfile.hpp"
#include "eldatasource.hpp"
#include "../errors.h"

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
			el_file::add_path(std::string(get_path_config()) + std::string("custom_updates/"));
			el_file::add_path(std::string(get_path_config()) + std::string("updates/"));
			el_file::add_path(std::string(get_path_config()));
			el_file::add_path(std::string(datadir) + std::string("custom_updates/"));
			el_file::add_path(std::string(datadir) + std::string("updates/"));
			el_file::add_path(datadir);
			el_file::add_path("./");
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

			return explicitIncRef(self->loadCoreAnimation(file).get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" CalCoreMaterial *CalLoader_ELLoadCoreMaterial(CalLoader *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			return explicitIncRef(self->loadCoreMaterial(file).get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
	}

	extern "C" CalCoreMesh *CalLoader_ELLoadCoreMesh(CalLoader *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			return explicitIncRef(self->loadCoreMesh(file).get());
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

	extern "C" int CalCoreModel_ELLoadCoreAnimation(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreAnimationPtr pCoreAnimation = CalLoader::loadCoreAnimation(file, self->getCoreSkeleton());

			if(!pCoreAnimation)
			{
				return -1;
			}

			return self->addCoreAnimation(pCoreAnimation.get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int CalCoreModel_ELLoadCoreMaterial(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreMaterialPtr pCoreMaterial = CalLoader::loadCoreMaterial(file);

			if (!pCoreMaterial)
			{
				return -1;
			}

			return self->addCoreMaterial(pCoreMaterial.get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" int CalCoreModel_ELLoadCoreMesh(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreMeshPtr pCoreMesh = CalLoader::loadCoreMesh(file);

			if (!pCoreMesh)
			{
				return -1;
			}

			return self->addCoreMesh(pCoreMesh.get());
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
	}

	extern "C" CalBoolean CalCoreModel_ELLoadCoreSkeleton(CalCoreModel *self, const char *strFilename)
	{
		assert(self);
		try
		{
			el_data_source file(strFilename);

			CalCoreSkeletonPtr pCoreSkeleton = CalLoader::loadCoreSkeleton(file);

			if (!pCoreSkeleton)
			{
				return False;
			}

			self->setCoreSkeleton(pCoreSkeleton.get());

			return True;
		}
		CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(False);
	}

}

#endif //NEW_FILE_IO
