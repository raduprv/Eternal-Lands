/**
 * @file
 * @ingroup io
 * @brief file i/o opject with support for zip and gzip files
 */

#ifndef	_ELDATASOURCE_HPP_
#define	_ELDATASOURCE_HPP_


#include "../platform.h"
#include <SDL/SDL_types.h>
#include <SDL/SDL_endian.h>
#include <cal3d/cal3d.h>
#include "elfile.hpp"

namespace eternal_lands
{

	class el_data_source: public el_file, public CalDataSource
	{
		public:
			el_data_source(const std::string &file_name): el_file(file_name, true)
			{
			}

			virtual bool ok() const
			{
				return true;
			}

			virtual void setError() const
			{
			}

			virtual bool readBytes(void* pBuffer, int length)
			{
				return read(length, pBuffer) == length;
			}

			virtual bool readFloat(float &value)
			{
				float tmp;
				int length;

				length = read(sizeof(float), &tmp);

				value = SwapLEFloat(tmp);

				return length == sizeof(float);
			}

			virtual bool readInteger(int &value)
			{
				Sint32 tmp;
				int length;

				length = read(sizeof(Sint32), &tmp);

				value = SDL_SwapLE32(tmp);

				return length == sizeof(Sint32);
			}

			virtual bool readString(std::string &strValue)
			{
				int length;

				if (readInteger(length))
				{
					if (length >= 0)
					{
						strValue = std::string(reinterpret_cast
							<const char*>(get_current_pointer()),
							length);

						seek(length, SEEK_CUR);

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

			virtual ~el_data_source()
			{
			}
	};

}


#endif	// _ELDATASOURCE_HPP_
