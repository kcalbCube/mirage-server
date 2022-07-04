#include <core/mirage.h>
#include <core/json.h>
#include <boost/json/src.hpp>
#include "SDL_image.h"
#include "config.h"
#include "server.h"
#include "client.h"
#include "game/game.h"
#include <SDL_image.h>

void readConfig(void)
{
	//auto&& jv = mirage::parseJson(mirage::serverConfigFile);
	//mirage::serverConfig = boost::json::value_to<mirage::ServerConfig>(jv);
	//mirage::onConfigRead();
}
#undef main
int main(int, char**)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);


	// readConfig();
	fmtlog::setLogLevel(fmtlog::DBG);
	fmtlog::startPollingThread();		
	mirage::network::server::networkController().start();	
	mirage::ioContext().run();
	for(;;);
	return mirage::version;
}
