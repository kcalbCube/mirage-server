#include "server.h"
#include <ranges>
#include <core/network.h>
#include <algorithm>

namespace mirage::network::server
{
	NetworkController::NetworkController(unsigned short port_)
		: port{port_}, 
		  socket(ioContext(), 
			boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
	{
	}	

	unsigned short NetworkController::getPort(void) const 
	{ 
		return port; 
	}

	void NetworkController::handleConnect(
			const boost::asio::ip::udp::endpoint& endpoint,
			const InitializeConnection& packet)
	{
		auto username = utils::sanitizeUsername(
				utils::stringView(packet.username, usernameMax));

		if(auto&& con = getConnection(username); con.isValid())
		{
			logi("{} connection refused: already connected", con.username);
			ConnectionResponce cr {.responce = ConnectionResponce::alreadyConnected};
			send(con, AbstractPacket(cr));
			return;
		}
		auto& con = connections.emplace_back(endpoint, username);

		const auto& ccon = con;
		ConnectionResponce cr;
		bool shouldErase = false;

		if(signal::isAny<bool>(newConnectionUnavailable, true, ccon))	
		{
			logi("{} connection refused: unavailable", con.username);

			cr.responce = ConnectionResponce::unavailable;
			shouldErase = true;
		}
		else if(signal::isAny<bool>(newConnectionBanned, true, ccon))
		{
			logi("{} connection refused: banned", con.username);
			cr.responce = ConnectionResponce::banned;
			shouldErase = true;
		}
		else
		{
			logi("{} connected ({})", con.username, con.endpoint.port());
			cr.responce = ConnectionResponce::success;
		}

		if(shouldErase)
			disconnectForce(ccon);	
		else
			event::enqueueEvent<NewConnectionEvent>(con.username);	

		send(con, AbstractPacket(cr));
			
	}

	void NetworkController::handlePacketRaw(
			const boost::asio::ip::udp::endpoint& endpoint,
			const AbstractPacket& packet)
	{
		logi("received packet, c {}, id {}", packet.packet->constant, packet.packet->id);
		if(packet.packet->constant != packetConstant)
			return;
		if(packet.packet->id == PacketId::connect)
		{
			handleConnect(endpoint,
					packetCast<InitializeConnection>(packet));
			return;
		}
		const auto connection = getConnection(endpoint);
		if(!connection.isValid())
			return;

		switch(packet.packet->id)
		{
			case PacketId::message:
				event::enqueueEvent<PacketReceivedEvent<MessageSent>>(
						connection.username,
						packetCast<MessageSent>(packet));
				break;
			case PacketId::resource:
				event::enqueueEvent<PacketReceivedEvent<ResourceRequest>>(
						connection.username,
						packetCast<ResourceRequest>(packet));
				break;
			default:
			{
				event::enqueueEvent<PacketReceivedEvent<AbstractPacket>>(
						connection.username,
						packet);
			}
		};
	}
	void NetworkController::handleReceiveFrom(
			const boost::system::error_code& ec, 
			size_t size)
	{
		if(ec && ec != boost::asio::error::message_size)
			return;
		handlePacketRaw(endpoint, AbstractPacket(&packet, size));
		startReceive();
	}

	void NetworkController::disconnectForce(const Connection& connection)
	{
		std::erase_if(connections,
				[&connection](const auto& a) -> bool
				{
					return a.username == connection.username;
				});
	}

	void NetworkController::disconnect(const Connection& connection)
	{
		disconnectForce(connection);
	}

	const Connection& NetworkController::getConnection(std::string_view username) const
	{
		const auto&& it = std::ranges::find_if(connections,
				[&username](auto&& a) -> bool
				{
					return a.username == username;
				});
		if(it == connections.end())
			return invalidConnection;
		return *it;

	}

	const Connection& NetworkController::getConnection(
			const boost::asio::ip::udp::endpoint& endpoint) const
	{
		const auto&& it = std::ranges::find_if(connections,
				[&endpoint](auto&& a) -> bool
				{
					return a.endpoint == endpoint;
				});
		if(it == connections.end())
			return invalidConnection;
		return *it;
	}

	void NetworkController::startReceive(void)
	{
		socket.async_receive_from(
				boost::asio::buffer(data), endpoint,
				boost::bind(&NetworkController::handleReceiveFrom,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}

	void NetworkController::start(void)
	{
		startReceive();
	}

	void NetworkController::send(
			const Connection& connection,
			const boost::asio::const_buffer& buffer)
	{
		socket.async_send_to(buffer, connection.endpoint,
			boost::bind(&NetworkController::handleSend, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
}
