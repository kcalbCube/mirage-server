#include "graphics.h"
#include "core/packet.h"
#include "core/processing.h"
#include "core/utility.h"
#include "entt/core/fwd.hpp"
#include "src/server.h"
#include <cstring>
#include <core/network.h>
#include <SDL_image.h>

mirage::graphics::IconLoader::result_type 
	mirage::graphics::IconLoader::operator()(const std::string& path, entt::id_type id)
{
	logi("loading icon {}", path);
	auto img = IMG_Load(path.c_str());

	if(img == nullptr)
	{
		auto error = SDL_GetError();
		loge("Failed loading icon '{}' id {}: {}", path, id, error);

		throw std::runtime_error(error);
	}

	return operator()(img, id);
}

mirage::graphics::IconLoader::result_type 
	mirage::graphics::IconLoader::operator()(SDL_Surface* surface, entt::id_type id)
{
	return std::make_shared<IconResource>(surface, 1.f, id);	
}

entt::resource<mirage::graphics::IconResource> mirage::graphics::load(entt::id_type id, const std::string& path)
{
	return iconCache().load(id, path, id).first->second; 
}

entt::resource<mirage::graphics::IconResource> mirage::graphics::load(entt::id_type id, SDL_Surface* surface)
{
	return iconCache().load(id, surface, id).first->second;
}

void mirage::graphics::IconRequestsResponder::ResponderProcess::update(float delta)
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
	startProcess<ResponderProcess>(mirage::ecs::processing::PeriodMS<5>::getInstance());
}

void mirage::graphics::IconRequestsResponder::lateInitialize(void)
{
	bindEvent<network::server::PacketReceivedEvent<network::ResourceRequest>, 
			  &IconRequestsResponder::onRequest>();
}
