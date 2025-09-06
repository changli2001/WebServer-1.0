// main.cpp
#include "ConfigParser.hpp"

int main(int argc, char **argv)
{
	

	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>\n";
		return 1;
	}

	try
	{
		ConfigParser parser(argv[1]); //char *
		std::vector<ServerConfig> configs = parser.parse();

		for (size_t s = 0; s < configs.size(); ++s)
		{
			const ServerConfig &config = configs[s];
			std::cout << "Parsed Server #" << s + 1 << ":\n";
			std::cout << "- IP: " << config.Ip << "\n";
			std::cout << "- Port: " << config.Port << "\n";
			std::cout << "- Server Name: " << config.server_name << "\n";
			std::cout << "- Max Body Size: " << config.MaxBodySize << "\n";

			for (std::map<int, std::string>::const_iterator it = config.error_pages.begin(); it != config.error_pages.end(); ++it)
				std::cout << "- Error Page " << it->first << ": " << it->second << "\n";

			for (size_t i = 0; i < config.locations.size(); ++i)
			{
				const LocationBlockConfig &loc = config.locations[i];
				std::cout << "  Location: " << loc.LocationName << "\n";
				if (!loc.BlockRootPath.empty())
					std::cout << "    BlockRootPath: " << loc.BlockRootPath << "\n";
				if (!loc.Indexes.empty())
				{
					std::cout << "    index:";
					for (size_t j = 0; j < loc.Indexes.size(); ++j)
						std::cout << " " << loc.Indexes[j];
					std::cout << "\n";
				}

				if (!loc.upload_path.empty())
					std::cout << "    upload_path: " << loc.upload_path << "\n";
				if (!loc.cgi_extension.empty())
					std::cout << "    cgi_extension: " << loc.cgi_extension << "\n";
				if (loc.return_code != 0)
				{
					std::cout << "    return: " << loc.return_code << " " << loc.redirect_url << "\n";
				}
				std::cout << "    autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Config parsing error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
