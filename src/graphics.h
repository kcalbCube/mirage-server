#pragma once
#include <entt/entt.hpp>
#include <core/graphics.h>
#include <core/utility.h>

namespace mirage::graphics
{
	struct IconLoader
	{
		using result_type = std::shared_ptr<IconResource>;

		result_type operator()(const std::string& path)
		{	
			return operator()(SDL_LoadBMP(path.c_str()));
		}

		result_type operator()(SDL_Surface* surface)
		{
			return std::make_shared<IconResource>(surface, 1.f);	
		}
	};

	using IconCache = entt::resource_cache<IconResource, IconLoader>;
	MIRAGE_COFU(IconCache, iconCache);
}
