#pragma once
#include <core/graphics.h>
#include <core/network.h>
#include "../server.h"
#include "../client.h"
#include "../graphics.h"
#include "core/utility.h"
#include <core/processing.h>

using namespace entt::literals;

namespace game
{
	class SimpleGame :
		public mirage::ecs::Component<SimpleGame>	
	{
	public:
		mirage::ecs::ComponentWrapper<mirage::server::Client> client;

		uint16_t x = 0, y = 0;
		unsigned width = 30,
			 height = 16;

		void sendFrame(void);

		void onInput(mirage::network::server::PacketReceivedEvent<mirage::network::Input>& packet);	

		void initialize(mirage::server::ClientAuthorizedEvent& event);	
		void lateInitialize(void);
	};
	MIRAGE_CREATE_WITH_EVENT(
		mirage::server::ClientAuthorizedEvent,
		SimpleGame);
}
