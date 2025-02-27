#include "webserv.hpp"


std::vector<Server> Server::parseConfigFile(const std::string &filename)
{
	std::vector<Server> servers;
	std::ifstream infile(filename.c_str());
	std::string line;
	t_parser_block parser_position;

	std::cout << BLUE << "Parsing config file: " << filename << std::endl
			  << RESET;

	if (!infile.is_open())
		throw std::runtime_error("Error: could not open file");

	parser_position.server = false;
	parser_position.error = false;
	parser_position.location = false;

	while (std::getline(infile, line))
	{
		line = trim_spaces(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "server:")
		{
			parser_position.server = true;
			parser_position.error = false;
			parser_position.location = false;
			servers.push_back(Server());
			continue;
		}

		if (line == "error_pages:")
		{
			if (parser_position.server == false)
				throw std::runtime_error("Error: error block outside of server block");
			parser_position.location = false;
			parser_position.error = true;
			continue;
		}

		if (line == "location:")
		{
			if (parser_position.server == false)
				throw std::runtime_error("Error: location block outside of server block");
			parser_position.location = true;
			parser_position.error = false;
			servers.back().addLocation();
			
			continue;
		}

		if (parser_position.server == false)
			throw std::runtime_error("Error: block outside of server block");
		parseLine(line, servers.back(), parser_position);
	}

	infile.close();

	return (servers);
}

void parseLocationBlock(const std::string &key, const std::string &value, Server &current_server)
{
	Location &loc = current_server.getLocations().back();

	if (key == "path")
		loc.setPath(value);
	else if (key == "allowed_methods")
		loc.addAllowedMethod(value);
	else if (key == "redirects")
		loc.setRedirect(value);
	else if (key == "allow_directory_listing")
		loc.setDirectoryListing(value == "on");
	else if (key == "index")
		loc.setIndex(value);
	else if (key == "route")
		loc.setRoute(value);
	else
		throw std::runtime_error("Error: invalid key in location block");
}

void parseServerBlock(const std::string &key, const std::string &value, Server &current_server)
{
	if (key == "hostaddr")
		current_server.setHostname(value);
	else if (key == "port")
	{
		if (current_server.getPort() == std::atoi(value.c_str()))
			throw std::runtime_error("Error: cannot specify two times the same port on the same server");
		current_server.setPort(std::atoi(value.c_str()));
	}
	else if (key == "server_name")
		current_server.setServerName(value);
	else if (key == "max_body_size")
		current_server.setMaxBodySize(std::atoll(value.c_str()));
	else if (key == "root_directory")
	{
		current_server.setRoot(value);
		current_server.relative_root = value;
	}
	else if (key == "entry_file")
		current_server.setDefaultFile(value);
	else if (key == "cgi-bin")
		current_server.setCgiBin(value);
	else if (key == "uploads")
		current_server.setUploads(value);
	else if (key == "cgi")
	{
		std::string cgiext = value.substr(0, value.find_first_of(" "));
		std::string cgibin = value.substr(value.find_first_of(" ") + 1);
		current_server.addCgiExtension(cgiext, cgibin);
	}
	else
		throw std::runtime_error("Error: invalid key in server block");
}

void parseLine(const std::string &line, Server &current_server, t_parser_block &parser_position)
{
	std::string key;
	std::string value;

	key = line.substr(0, line.find_first_of(":"));
	key = trim_spaces(key);
	value = line.substr(line.find_first_of(":") + 1);
	value = trim_spaces(value);

	if (value.empty())
		throw std::runtime_error("Error: unknown key");
	if (parser_position.error == true && parser_position.location == false)
		current_server.updateErrorPage(std::atoi(key.c_str()), value);
	else if (parser_position.location == true)
		parseLocationBlock(key, value, current_server);
	else
		parseServerBlock(key, value, current_server);
}

bool checkConfigFile(std::vector<Server> &servers)
{
	std::vector<int> ports;
	std::vector<std::string> hostname; 

	//go through each server and check if the config is valid
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		if (it->getHostname().empty())
		{
			std::cerr << RED << "Error: missing hostaddr in server block or improper format" << RESET << std::endl;
			return false;
		}
		else
			hostname.push_back(it->getHostname());
		if (it->getPort() <= 1024 || it->getPort() > 65535)
		{
			std::cerr << RED << "Error: port must be a number comprised between 1024 and 65535" << RESET << std::endl;
			return false;
		}
		else
			ports.push_back(it->getPort());
		if (it->getServerName().empty())
		{
			std::cout << YELLOW << "Warning: missing server_name in server block" << RESET << std::endl;
			std::cout << YELLOW << "Switching to default server_name" << RESET << std::endl;
			it->setServerName("default");
		}
		if (it->getRoot().empty())
		{
			std::cout << YELLOW << "Warning: missing root_directory in server block" << RESET << std::endl;
			std::cout << YELLOW << "Switching to default root_directory" << RESET << std::endl;
			it->setRoot("/");
		}
		if (it->getDefaultFile().empty())
		{
			std::cout << YELLOW << "Warning: missing entry_file in server block" << RESET << std::endl;
			std::cout << YELLOW << "Switching to default entry_file" << RESET << std::endl;
			it->setDefaultFile("index.html");
		}
		if (it->getCgiBin().empty())
		{
			std::cout << YELLOW << "Warning: missing cgi-bin in server block" << RESET << std::endl;
			std::cout << YELLOW << "Switching to default cgi-bin" << RESET << std::endl;
			it->setCgiBin("/cgi-bin/");
		}
		if (it->getUploads().empty())
		{
			std::cout << YELLOW << "Warning: missing uploads in server block" << RESET << std::endl;
			std::cout << YELLOW << "Switching to default uploads" << RESET << std::endl;
			it->setUploads("/uploads/");
		}
		if (it->getMaxBodySize() <= 0 || it->getMaxBodySize() > INT_MAX)
		{
			std::cerr << RED << "Error: max_body_size must be a number comprised between 1 and " << INT_MAX << RESET << std::endl;
			return false;
		}
	}

	//if two hostnames are the same, check that the ports are different
	for (std::vector<std::string>::iterator it = hostname.begin(); it != hostname.end(); it++)
	{
		std::vector<std::string>::iterator it2 = it;
		it2++;
		for (; it2 != hostname.end(); it2++)
		{
			if (*it == *it2)
			{
				std::vector<int>::iterator it3 = ports.begin() + (it - hostname.begin());
				std::vector<int>::iterator it4 = ports.begin() + (it2 - hostname.begin());
				if (*it3 == *it4)
				{
					std::cerr << "Error: two servers cannot have the same hostname and port" << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

bool checkIPAddrFormat(std::string addr)
{
	std::vector<std::string> parts;
	std::string part;
	std::stringstream ss(addr);

	while (std::getline(ss, part, '.'))
		parts.push_back(part);

	if (parts.size() != 4)
		return false;
	for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); it++)
	{
		if (it->empty())
			return false;
		for (std::string::iterator it2 = it->begin(); it2 != it->end(); it2++)
		{
			if (!std::isdigit(*it2))
				return false;
		}
		if (std::atoi(it->c_str()) < 0 || std::atoi(it->c_str()) > 255)
			return false;
	}
	return true;
}