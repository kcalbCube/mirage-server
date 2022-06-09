#pragma once
#include "../client.h"
#include <core/processing.h>


class ClientAuthorization : 
	public mirage::ecs::Component<ClientAuthorization>,
	public mirage::ecs::Processing<ClientAuthorization>	
{	
public:
	mirage::ecs::ComponentWrapper<mirage::server::Client> client;
	struct AuthorizationProcess : Process<AuthorizationProcess, unsigned>
	{
		mirage::ecs::ComponentWrapper<ClientAuthorization> parent;
		AuthorizationProcess(entt::entity entity)
			: parent(entity) {}

		int counter = 0;
		void update(unsigned delta, void*);
		void failed(void);
	};

	void onPacket(mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>&);

	void initialize(mirage::server::ClientAuthorizationRequestEvent&);
	void lateInitialize(void);
};
MIRAGE_CREATE_WITH_EVENT(
	mirage::server::ClientAuthorizationRequestEvent, 
	ClientAuthorization);
