#include "server.h"
#include "core/event.h"
#include "core/mirage.h"
#include <ranges>
#include <core/network.h>
#include <algorithm>

namespace mirage::network::server
{
	NetworkController::NetworkController(unsigned short port_, unsigned short tcpPort_)
		: port{port_},
		  tcpPort{tcpPort_},
		  socket(ioContext(), 
			boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),
		  acceptor{ioContext(),
		  	boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), tcpPort)},
		  buffer{},
		  tcpBuffer{}
	{
	}	

	unsigned short NetworkController::getPort(void) const 
	{ 
		return port; 
	}

	void NetworkController::handleTcpReceiveFrom(
			boost::shared_ptr<Connection> connection,
			const boost::system::error_code& ec, 
			size_t size)
	{
		if ((ec == boost::asio::error::eof) || (ec == boost::asio::error::connection_reset))
			return; // FIXME: add disconnect handling
		handlePacketRaw(endpoint, AbstractPacket(&tcpBuffer.packet, size));
		startTcpReceive(connection);
	}

	void NetworkController::startTcpReceive(boost::shared_ptr<Connection> connection)
	{
		socket.async_receive(
			boost::asio::buffer(tcpBuffer.data),
			boost::bind(&NetworkController::handleTcpReceiveFrom,
				this,
				connection,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void NetworkController::handleAccept(
			boost::shared_ptr<Connection> connection, 
			const boost::system::error_code&)
	{
		auto& socket = connection->tcpConnection->socket;
		logi("{}:{} TCP connected", connection->username, socket.remote_endpoint().port());
		startTcpReceive(connection);

		event::enqueueEvent<NewConnectionEvent>(connection->username);

		connection->activated = true;
	}


	void NetworkController::handleConnect(
			const boost::asio::ip::udp::endpoint& endpoint,
			const InitializeConnection& packet)
	{
		auto username = utils::sanitizeUsername(
				std::string(utils::stringView(packet.username)));

		if(auto&& connectionPtr = getConnection(username); connectionPtr)
		{
			auto& con = *connectionPtr;
			logi("{} connection refused: already connected", con.username);
			ConnectionResponce cr {.responce = ConnectionResponce::alreadyConnected};
			send(con, AbstractPacket(cr));
			return;
		}

		auto& ptr = connections.emplace_back(
			std::move(
				boost::make_shared<Connection>(
					boost::make_shared<TCPConnection>(ioContext()),
					endpoint,
					username,
					false
				)
			)
		);
			
		auto& con = *ptr;

		ConnectionResponce cr;

		if(signal::isAny<bool>(newConnectionUnavailable, true, ptr))	
		{
			logi("{} connection refused: unavailable", con.username);
			cr.responce = ConnectionResponce::unavailable;
		}
		else if(signal::isAny<bool>(newConnectionBanned, true, ptr))
		{
			logi("{} connection refused: banned", con.username);
			cr.responce = ConnectionResponce::banned;
		}
		else
		{
			logi("{}:{} UDP connected, waiting TCP connect.", con.username, con.endpoint.port());
			acceptor.async_accept(con.tcpConnection->socket, 
				boost::bind(&NetworkController::handleAccept, this, ptr, boost::asio::placeholders::error));	
			return;
		}

		send(con, AbstractPacket{cr});
		disconnectForce(con);
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
		if(!connection || !connection->isValid() || !connection->activated)
			return;
		std::string username = connection->username; // copy

		switch(packet.packet->id)
		{
			case PacketId::message:
				event::enqueueEvent<PacketReceivedEvent<MessageSent>>(
						std::move(username),
						packetCast<MessageSent>(packet));
				break;
			case PacketId::resource:
				event::enqueueEvent<PacketReceivedEvent<ResourceRequest>>(
						std::move(username),
						packetCast<ResourceRequest>(packet));
				break;
			default:
			{
				event::enqueueEvent<PacketReceivedEvent<AbstractPacket>>(
						std::move(username),
						packet);
				break;
			}
		};
	}
	void NetworkController::handleReceiveFrom(
			const boost::system::error_code& ec, 
			size_t size)
	{
		if(ec && ec != boost::asio::error::message_size)
			return;
		handlePacketRaw(endpoint, AbstractPacket(&buffer.packet, size));
		startReceive();
	}

	void NetworkController::disconnectForce(const Connection& connection)
	{
		std::erase_if(connections,
				[&connection](const auto& a) -> bool
				{
					return a->username == connection.username;
				});
	}

	void NetworkController::disconnect(const Connection& connection)
	{
		disconnectForce(connection);
	}

	boost::shared_ptr<Connection> NetworkController::getConnection(std::string_view username) const
	{
		const auto&& it = std::ranges::find_if(connections,
				[&username](auto&& a) -> bool
				{
					return a->username == username;
				});
		if(it == connections.end())
			return nullptr;
		return *it;

	}

	boost::shared_ptr<Connection> NetworkController::getConnection(
			const boost::asio::ip::udp::endpoint& endpoint) const
	{
		const auto&& it = std::ranges::find_if(connections,
				[&endpoint](auto&& a) -> bool
				{
					return a->endpoint == endpoint;
				});
		if(it == connections.end())
			return nullptr;
		return *it;
	}

	void NetworkController::startReceive(void)
	{
		socket.async_receive_from(
				boost::asio::buffer(buffer.data), endpoint,
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

	void NetworkController::sendTcp(
			const Connection& connection,
			const boost::asio::const_buffer& buffer)
	{
		connection.tcpConnection->socket.async_send(buffer, 
			boost::bind(&NetworkController::handleSend, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}
}
