#pragma once
#include <core/json.h>
#include <core/utility.h>

namespace mirage
{	
	
	struct ServerConfig
	{
		int port = 5000;
		std::string serverName = "FILLME";
	};

	
	inline ServerConfig tag_invoke(
		boost::json::value_to_tag<ServerConfig>, 
		boost::json::value const& jv)
	{
		auto&& obj = jv.as_object();
		return ServerConfig 
		{
			boost::json::value_to<int>(obj.at("server_port")),
			boost::json::value_to<std::string>(obj.at("server_name"))
		};
	}

	MIRAGE_COFU(std::string, serverConfigFile, "config.json");
	MIRAGE_COFU(ServerConfig, serverConfig);

	//inline boost::signals2::signal<void(void)> onConfigRead;
}
