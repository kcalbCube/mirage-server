#include "client.h"

void ClientAuthorization::AuthorizationProcess::update(unsigned int delta, void*)
{
	if(!parent || !parent->client)
	{
		fail();
		return;
	}
	if(counter++ > 10)
	{
		fail();
		return;
	}
	if(counter % 2)
	{
		parent->client.get().sendMessage("Enter password!");
	}
}

void ClientAuthorization::AuthorizationProcess::failed(void)
{
	if(parent && parent->client)
	{
		mirage::event::triggerEvent<mirage::server::ClientAuthorizationConfirmedEvent>
			(std::string(parent->client->getUsername()));
		return;
		parent->client->sendMessage("Authorization failed!");
		parent->destroy();
	}
}

void ClientAuthorization::onPacket(
		mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>& packet)
{	
	if(packet.username != client->getUsername())
		return;

	if(packet.packet.view() == "kcalbCubinho")
	{
		client->sendMessage("Authorized.");	
		mirage::event::triggerEvent<mirage::server::ClientAuthorizationConfirmedEvent>
			(std::string(client->getUsername()));
		destroy();
	}
	else
		client->sendMessage("Wrong password!");
}
void ClientAuthorization::initialize( 
		mirage::server::ClientAuthorizationRequestEvent& request)
{
	startProcess<AuthorizationProcess>(
			mirage::ecs::processing::PeriodMS<1000>::getInstance(), 
			entity);
	client = request.client;
}

void ClientAuthorization::lateInitialize(void)
{
	bindEvent<mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>, &ClientAuthorization::onPacket>();
}
