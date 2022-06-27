#pragma once
#include <core/graphics.h>
#include <core/network.h>
#include "../server.h"
#include "../client.h"
#include "../graphics.h"
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
			auto icon = mirage::graphics::load("1.bmp"_hs, "resource/1.bmp");	

			vg.vertices.emplace_back(static_cast<uint16_t>(x * UINT16_MAX), static_cast<uint16_t>(y * UINT16_MAX), icon->id);

			frame.push_back(vg);	

			mirage::network::GraphicFrame framePacket;
			auto serialized = mirage::utils::serialize(frame);
			memcpy(framePacket.serialized, serialized.c_str(), serialized.size());
			mirage::network::server::networkController().send(
				client->getConnection(), 
				mirage::network::AbstractPacket{framePacket});
		}

		void onMessage(mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>& packet)
		{
			if(packet.username != client->getUsername())
				return;
			switch(toupper(mirage::utils::stringView(packet.packet.message)[0]))
			{
			case 'W':
				y -= 0.1f;
				break;
			case 'S':
				y += 0.1f;
				break;
			case 'A':
				x -= 0.1f;
				break;
			case 'D':
				x += 0.1f;
				break;
			default:
				return;
			}

			sendFrame();
		}

		void initialize(mirage::server::ClientAuthorizedEvent& event)
		{
			client = event.client;
		}

		void lateInitialize(void)
		{
			bindEvent<mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>, &SimpleGame::onMessage>();
		}
	};
	MIRAGE_CREATE_WITH_EVENT(
		mirage::server::ClientAuthorizedEvent,
		SimpleGame);
}
