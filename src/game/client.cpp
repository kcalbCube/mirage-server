#include "client.h"

void ClientAuthorization::AuthorizationProcess::update(float delta)
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

void ClientAuthorization::AuthorizationProcess::onFail(void)
{
	if(parent && parent->client)
	{
		mirage::event::triggerEvent<mirage::server::ClientAuthorizationConfirmedEvent>
			(std::string(parent->client->getUsername()));
		parent->client->sendMessage("Authorization failed!");
		parent->destroy();
	}
}

void ClientAuthorization::onPacket(
		mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>& packet)
{	
	if(packet.username != client->getUsername())
		return;

	if(mirage::utils::stringView(packet.packet.message) == "kcalbCubinho")
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
			mirage::ecs::processing::PeriodMS<1000>::getInstance());
	client = request.client;
}

void ClientAuthorization::lateInitialize(void)
{
	bindEvent<mirage::network::server::PacketReceivedEvent<mirage::network::MessageSent>, &ClientAuthorization::onPacket>();
}
