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

		float x = 0, y = 0;

		void sendFrame(void)
		{
			mirage::network::GraphicFrame::SerializedT frame{};
			mirage::graphics::VerticeGroup vg;
			auto w = client->windowWidth;
			auto h = client->windowHeight;
			vg.filters.emplace_back(mirage::graphics::Scale(w / (13 * 64), h / (9 * 64)));
			auto icon = mirage::graphics::load("1.png"_hs, "resource/1.png");	

			vg.vertices.emplace_back(static_cast<uint16_t>(x * UINT16_MAX), static_cast<uint16_t>(y * UINT16_MAX), icon->id);

			{
				auto tile = mirage::graphics::load("tiles/tile.png"_hs, "resource/tiles/tile.png");
				auto dx = UINT16_MAX / 13;
				auto dy = UINT16_MAX / 9;

				for(uint16_t x = 0; x < 13; ++x)
					for(uint16_t y = 0; y < 9; ++y)
						vg.vertices.emplace_back(x * dx + dx / 2, y * dy + dy / 2, tile->id);
			}


			frame.push_back(vg);	

			mirage::network::GraphicFrame framePacket;
			auto serialized = mirage::utils::serialize(frame);
			memcpy(framePacket.serialized, serialized.c_str(), serialized.size());
			mirage::network::server::networkController().send(
				client->getConnection(), 
				mirage::network::AbstractPacket{framePacket});
		}

		void onInput(mirage::network::server::PacketReceivedEvent<mirage::network::Input>& packet)
		{
			if(packet.username != client->getUsername())
				return;
			auto deserialized = mirage::utils::deserialize<mirage::network::Input::SerializedT>(
				std::string{mirage::utils::span(packet.packet.serialized)});

			bool isUpdated = false;
			for(auto&& key : deserialized)
			{
				switch(key)
				{
				case SDL_SCANCODE_W:
					y -= 0.1f;
					break;
				case SDL_SCANCODE_S:
					y += 0.1f;
					break;
				case SDL_SCANCODE_D:
					x += 0.1f;
					break;
				case SDL_SCANCODE_A:
					x -= 0.1f;
					break;
				default: continue;
				}

				isUpdated = true;
			}

			if(isUpdated)
				sendFrame();
		}

		void initialize(mirage::server::ClientAuthorizedEvent& event)
		{
			client = event.client;
		}

		void lateInitialize(void)
		{
			bindEvent<mirage::network::server::PacketReceivedEvent<mirage::network::Input>, &SimpleGame::onInput>();
		}
	};
	MIRAGE_CREATE_WITH_EVENT(
		mirage::server::ClientAuthorizedEvent,
		SimpleGame);
}
