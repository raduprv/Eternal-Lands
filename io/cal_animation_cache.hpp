#ifndef _CAL_ANIMATION_CACHE_
#define _CAL_ANIMATION_CACHE_

#include <map>
#include <cal3d/global.h>
#include <iostream>

namespace eternal_lands
{

	class CalAnimationCache
	{
	private:
		typedef std::pair<std::string,float> AnimationKey;
		typedef std::map<AnimationKey, CalCoreAnimationPtr> AnimationsMap;
		
		CalAnimationCache()
			: _animations()
		{
		}
		
		static CalAnimationCache & instance()
		{
			static CalAnimationCache _cache;
			return _cache;
		}
		
	public:
		
		static CalCoreAnimationPtr loadAnimation(const std::string &fileName, float scale)
		{
			AnimationsMap &anims = instance()._animations;
			AnimationKey key(fileName, scale);
			AnimationsMap::iterator it = anims.find(key);
			if (it != anims.end())
				return it->second;
			else {
				el_data_source file(fileName);
				
				CalCoreAnimationPtr anim_ptr = CalLoader::loadCoreAnimation(file, NULL);
				if (anim_ptr)
				{
					CalCoreAnimation_Scale(anim_ptr.get(), scale);
					anim_ptr->setFilename(fileName);
				}
				
				anims[key] = anim_ptr;
				return anim_ptr;
			}
		}
		
	private:
		AnimationsMap _animations;
	};
	
} // namespace eternal_lands

#endif // _CAL_ANIMATION_CACHE_
