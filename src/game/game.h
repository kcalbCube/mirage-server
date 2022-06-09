#pragma once
#include <core/graphics.h>
#include <core/network.h>
#include "../server.h"
#include "../client.h"
#include "../graphics.h"
#include <core/processing.h>

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
			vg.vertices.emplace_back(x, y);	

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
			switch(toupper(packet.packet.view()[0]))
			{
			case 'W':
				y -= 0.1;
				break;
			case 'S':
				y += 0.1;
				break;
			case 'A':
				x -= 0.1;
				break;
			case 'D':
				x += 0.1;
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
