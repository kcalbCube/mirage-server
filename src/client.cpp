#include "client.h"
#include <core/ecs.h>
#include <core/network.h>
#include "core/event.h"
#include "server.h"
#include <boost/bind.hpp>
namespace mirage::server
{
	bool Client::isAuthorized(void) const
	{
		return authorized;
	}

	const network::server::Connection& Client::getConnection(void) const
	{
		return connection;
	}

	std::string_view Client::getUsername(void) const
	{
		return connection.username;
	}

	void Client::staticInitialize(void)
	{
	}

	void Client::onDestroy(void)
	{
		network::server::networkController().disconnect(connection);
	}

	void Client::authorizationBlocked(ClientAuthorizationBlockedEvent& event)
	{
		if(!authorized && event.username == getUsername())
			destroy();
	}

	void Client::authorizationConfirmed(ClientAuthorizationConfirmedEvent& event)
	{
		if(authorized || event.username != getUsername())
			return;

		authorized = true;	
		event::triggerEvent<ClientAuthorizedEvent>(entity);
	}

	void Client::initialize(network::server::NewConnectionEvent& event)
	{
		connection = network::server::networkController()
			.getConnection(event.username);
	
		event::triggerEvent<ClientAuthorizationRequestEvent>(entity);
	}

	void Client::lateInitialize(void)
	{
		bindEvent<ClientAuthorizationBlockedEvent, &Client::authorizationBlocked>();
		bindEvent<ClientAuthorizationConfirmedEvent, &Client::authorizationConfirmed>();
	}

	void Client::sendMessage(std::string_view str)
	{
		network::MessageSent msg;
		memcpy(msg.message, str.data(), str.size());
		
		network::server::networkController().send(
			getConnection(), 
			network::AbstractPacket(msg));
	}

}
