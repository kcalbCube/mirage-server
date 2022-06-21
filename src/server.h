#pragma once

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <core/mirage.h>
#include <core/signal.h>
#include <core/event.h>
#include <core/packet.h>
#include <boost/asio/buffer.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

namespace mirage::network::server
{
	struct TCPConnection	
	{
		boost::asio::ip::tcp::socket socket;

		TCPConnection(auto& context)
			: socket{context} {}
	};

	template<size_t Size>
	union Buffer
	{
		PacketVoid packet;
		uint8_t data[Size];
	};

	class Connection
	{
		public:
		boost::shared_ptr<TCPConnection> tcpConnection;
		boost::asio::ip::udp::endpoint endpoint;	
		std::string username;

		bool activated;

		inline bool isValid(void) const 
		{
			return (!username.empty()) && activated;
		}
	};

	class NetworkController
	{
		unsigned short 
			port = 5000,
			tcpPort = 5001;

		std::list<boost::shared_ptr<Connection>> connections;	
		boost::asio::ip::udp::endpoint endpoint;
		boost::asio::ip::udp::socket socket;
		boost::asio::ip::tcp::acceptor acceptor;
		std::mutex mutex;

		Buffer<maxPacketSize> buffer;
		Buffer<maxTcpPacketSize> tcpBuffer;

		void handleConnect(
				const boost::asio::ip::udp::endpoint&,
				const InitializeConnection&);
		void handlePacketRaw(
				const boost::asio::ip::udp::endpoint&,
				const AbstractPacket&);

		void handleReceiveFrom(const boost::system::error_code&, size_t);
		void handleTcpReceiveFrom(boost::shared_ptr<Connection>, const boost::system::error_code&, size_t);

		void startTcpReceive(boost::shared_ptr<Connection>);
		void startReceive(void);

		bool started = false;
		void handleSend(const boost::system::error_code&,
				size_t) {}
		void handleAccept(boost::shared_ptr<Connection>, const boost::system::error_code&);
	public:
		NetworkController(unsigned short port, unsigned short tcpPort);

		unsigned short getPort(void) const;
		unsigned short getTcpPort(void) const;

		boost::shared_ptr<Connection> getConnection(std::string_view username) const;
		boost::shared_ptr<Connection> getConnection(const boost::asio::ip::udp::endpoint&) const;
		void disconnect(std::string_view username);
		void disconnect(const Connection&);
		/*
		 * Disconnect without replying.
		 */
		void disconnectForce(const Connection&);
	
		entt::sigh<bool(boost::shared_ptr<Connection>)> newConnectionUnavailable;
		entt::sigh<bool(boost::shared_ptr<Connection>)> newConnectionBanned;	

		void start(void);
		void send(const Connection&, const boost::asio::const_buffer&);
		void sendTcp(const Connection&, const boost::asio::const_buffer&);
	};	

	inline NetworkController& networkController(void)
	{
		static NetworkController instance(5000, 5001);
		return instance;
	}

	struct NewConnectionEvent
	{
		std::string username;
	};
	struct ConnectionDisconnectEvent
	{
		std::string username;
	};

	template<typename T>
	struct PacketReceivedEvent
	{
		std::string username;
		T packet;
	};
}
