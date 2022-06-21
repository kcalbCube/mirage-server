#include "graphics.h"
#include "core/packet.h"
#include "core/processing.h"
#include "core/utility.h"
#include "entt/core/fwd.hpp"
#include "src/server.h"
#include <cstring>
#include <core/network.h>
mirage::graphics::IconLoader::result_type 
	mirage::graphics::IconLoader::operator()(const std::string& path, entt::id_type id)
{	
	return operator()(SDL_LoadBMP(path.c_str()), id);
}

mirage::graphics::IconLoader::result_type 
	mirage::graphics::IconLoader::operator()(SDL_Surface* surface, entt::id_type id)
{
	return std::make_shared<IconResource>(surface, 1.f, id);	
}

mirage::graphics::IconRequestsResponder::ResponderProcess::ResponderProcess(entt::entity entity)
	: parent{entity} {}

void mirage::graphics::IconRequestsResponder::ResponderProcess::update(unsigned int, void *)
{
	decltype(parent->queue) mqueue;
	mqueue.merge(parent->queue);

	for(auto&& [username, icons] : mqueue)
	{
		network::ResourceUpdate ru;
		network::ResourceUpdate::SerializedT st;
		for(auto&& icon : icons)
			if(entt::resource<IconResource> res = iconCache()[icon])
				st.push_back(*res);
			else
				loge("unknown icon requested: {}", icon);

		auto serialized = utils::serialize(st);
		memcpy(ru.serialized, serialized.c_str(), std::min(sizeof(ru.serialized), serialized.size()));

		network::server::networkController().sendTcp(
			*network::server::networkController().getConnection(username),
			network::AbstractPacket{ru}
		);
	}
}

void mirage::graphics::IconRequestsResponder::onRequest(
	network::server::PacketReceivedEvent<network::ResourceRequest>& request)
{
	auto deserialized = utils::deserialize<network::ResourceRequest::SerializedT>
		(std::string{std::string_view(request.packet.serialized, std::size(request.packet.serialized))});
	if(!queue.contains(request.username))
		queue[request.username].reserve(deserialized.size());
	auto&& dest = queue[request.username];

	std::ranges::copy(deserialized, std::back_inserter(dest));
}

void mirage::graphics::IconRequestsResponder::initialize(void)
{
	startProcess<ResponderProcess>(mirage::ecs::processing::PeriodMS<5>::getInstance(), entity);
}

void mirage::graphics::IconRequestsResponder::lateInitialize(void)
{
	bindEvent<network::server::PacketReceivedEvent<network::ResourceRequest>, 
			  &IconRequestsResponder::onRequest>();
}
