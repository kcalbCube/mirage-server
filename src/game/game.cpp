#include "game.h"

void game::SimpleGame::sendFrame(void)
{
	mirage::network::GraphicFrame::SerializedT frame{};
	mirage::graphics::VerticeGroup vg;
	auto w = client->windowWidth;
	auto h = client->windowHeight;	

	auto scaleX = static_cast<float>(w) / (width * 64);
	auto scaleY = static_cast<float>(h) / (height * 64);

	auto cropX = static_cast<unsigned>(w - 64 * scaleX * width);
	auto cropY = static_cast<unsigned>(h - 64 * scaleY * height);

	vg.filters.emplace_back(mirage::graphics::Scale(
		static_cast<unsigned>(scaleX), static_cast<unsigned>(scaleY)));
	auto icon = mirage::graphics::load("1.png"_hs, "resource/1.png");		

	{
		auto tile = mirage::graphics::load("tiles/tile.png"_hs, "resource/tiles/tile.png");
		auto dx = static_cast<unsigned>(64 * scaleX);
		auto dy = static_cast<unsigned>(64 * scaleY);
	
		auto sx = cropX / 2;
		auto sy = cropY / 2;

		for(uint16_t x = 0; x < width; ++x)
			for(uint16_t y = 0; y < height; ++y)
				vg.vertices.emplace_back(
					sx + x * dx + dx / 2, 
					sy + y * dy + dy / 2, 
					tile->id);
	}

	vg.vertices.emplace_back(x, y, icon->id, 1, 1, 1);


	frame.push_back(vg);	

	mirage::network::GraphicFrame framePacket;
	auto serialized = mirage::utils::serialize(frame);
	memcpy(framePacket.serialized, serialized.c_str(), serialized.size());
	mirage::network::server::networkController().send(
		client->getConnection(), 
		mirage::network::AbstractPacket{framePacket});
}

void game::SimpleGame::onInput(mirage::network::server::PacketReceivedEvent<mirage::network::Input>& packet)
{
	if(packet.username != client->getUsername())
		return;
	auto deserialized = mirage::utils::deserialize<mirage::network::Input::SerializedT>(
		std::string{mirage::utils::span(packet.packet.serialized)});

	bool isUpdated = false;
	for(auto&& key : deserialized)
	{
		switch(key)
		{
		case SDL_SCANCODE_W:
			y -= 32;
			break;
		case SDL_SCANCODE_S:
			y += 32;
			break;
		case SDL_SCANCODE_D:
			x += 32;
			break;
		case SDL_SCANCODE_A:
			x -= 32;
			break;
		default: continue;
		}

		isUpdated = true;
	}

	if(isUpdated)
		sendFrame();
}

void game::SimpleGame::initialize(mirage::server::ClientAuthorizedEvent& event)
{
	client = event.client;
}

void game::SimpleGame::lateInitialize(void)
{
	bindEvent<mirage::network::server::PacketReceivedEvent<mirage::network::Input>, &SimpleGame::onInput>();
}
