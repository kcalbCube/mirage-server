#pragma once
#include <core/graphics.h>
#include <core/utility.h>
#include <core/ecs.h>
#include <core/processing.h>
#include "server.h"
#include <map>

namespace mirage::graphics
{
	struct IconLoader
	{
		using result_type = std::shared_ptr<IconResource>;	

		result_type operator()(const std::string& path, entt::id_type);	
		result_type operator()(SDL_Surface* surface, entt::id_type);
	};

	using IconCache = entt::resource_cache<IconResource, IconLoader>;
	MIRAGE_COFU(IconCache, iconCache);

	entt::resource<IconResource> load(entt::id_type id, const std::string& path);	
	entt::resource<IconResource> load(entt::id_type id, SDL_Surface* surface);

	struct IconRequestsResponder : 
		ecs::Component<IconRequestsResponder>,
		ecs::Processing<IconRequestsResponder>
	{
		struct ResponderProcess : Process<ResponderProcess>
		{
			void update(float delta);
		};

		std::map<std::string, std::vector<Icon>> queue;

		void onRequest(network::server::PacketReceivedEvent<network::ResourceRequest>&);
		void initialize(void);
		void lateInitialize(void);
	};

	MIRAGE_CREATE_ON_STARTUP(IconRequestsResponder, responder);
}
