#pragma once
#include <core/ecs.h>
#include <core/event.h>
#include "server.h"
#include <functional>
#include <string>

namespace mirage::server
{
	struct ClientAuthorizationRequestEvent	
	{
		entt::entity client;
	};

	struct ClientAuthorizationBlockedEvent	
	{
		std::string username;
	};

	struct ClientAuthorizationConfirmedEvent	
	{
		std::string username;
	};

	struct ClientAuthorizedEvent	
	{
		entt::entity client;
	};

	class Client : 
		public mirage::ecs::Component<Client>	
	{
		network::server::Connection connection;
		bool authorized = false;

		void authorizationBlocked(ClientAuthorizationBlockedEvent&);
		void authorizationConfirmed(ClientAuthorizationConfirmedEvent&);	
	public:
		const network::server::Connection& getConnection(void) const;
		std::string_view getUsername(void) const;

		bool isAuthorized(void) const;

		void onDestroy(void);
		
		static void staticInitialize(void);
		void initialize(network::server::NewConnectionEvent&);
		void lateInitialize(void);

		void sendMessage(std::string_view);
	};

	MIRAGE_CREATE_WITH_EVENT(network::server::NewConnectionEvent, Client);
}
