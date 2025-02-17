/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:20:32 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/17 17:38:11 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <utils.hpp>

std::string toBinaryString(uint8_t value) {
    std::string result;
    // Loop from the most significant bit (7) to the least significant (0)
    for (int i = 7; i >= 0; --i) {
        result.push_back((value & (1 << i)) ? '1' : '0');
    }
    return result;
}

bool check_extension(const std::string &str)
{
	if (str.find_last_of('.') == std::string::npos)
		return (false);
	return (str.substr(str.find_last_of('.')) == ".conf");
}

std::string getCurrentDate()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return std::string(buffer);
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
	else
		throw std::runtime_error("Error: invalid key in location block");
}

void parseServerBlock(const std::string &key, const std::string &value, Server &current_server)
{
	if (key == "hostaddr")
		current_server.setHostname(value);
	else if (key == "port")
		current_server.setPort(std::atoi(value.c_str()));
	else if (key == "server_name")
		current_server.setServerName(value);
	else if (key == "max_body_size")
		current_server.setMaxBodySize(std::atoll(value.c_str()));
	else if (key == "root_directory")
		current_server.setRoot(value);
	else if (key == "entry_file")
		current_server.setDefaultFile(value);
	else if (key == "cgi-bin")
		current_server.setCgiBin(value);
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

bool checkPathExists(const std::string &path)
{
	std::ifstream file(path.c_str());
	return file.good();
}

bool isDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

std::string decodeURI(const std::string &uri)
{
	std::string decoded = "";
	size_t i = 0;
	
	while (i < uri.size())
	{
		if (uri[i] == '%')
		{
			decoded += static_cast<char>(std::stoi(uri.substr(i + 1, 2), 0, 16));
			i += 3;
		}
		else
		{
			decoded += uri[i];
			i++;
		}
	}
	return decoded;
}

std::string extractPathFromURI(const std::string uri)
{	
	std::string path = uri;
	size_t pos = path.find("?");
	if (pos != std::string::npos)
		path = path.substr(0, pos);
	return decodeURI(path);
}

std::string extractAttributesFromURI(const std::string uri)
{
	std::string attributes = uri;
	size_t pos = attributes.find("?");
	if (pos != std::string::npos)
		attributes = attributes.substr(pos + 1);
	return decodeURI(attributes);
}

//Use full path of directory to generate directory listing in html
std::string generateDirectorylisting(const std::string full_path)
{
	std::stringstream directory_listing;
	std::string relative_path = full_path.substr(full_path.find_last_of('/') + 1);

	directory_listing << "<html><head>";
	directory_listing << "<title>Directory listing</title>";
	directory_listing << "<style>";
	directory_listing << "body {font-family: Arial, sans-serif;}";
	directory_listing << "h1 {color: #333;}";
	directory_listing << "table {width: 100%; border-collapse: collapse;}";
	directory_listing << "</style>";
	directory_listing << "</head><body>";
	directory_listing << "<h1>Directory listing : </h1> <h3>" << full_path << "</h3>";
	directory_listing << "<table>";

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(full_path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			directory_listing << "<tr>";
			directory_listing << "<td><a href=\"" << ent->d_name << "\">" << ent->d_name << "</a></td>";
			directory_listing << "</tr>";
		}
		closedir(dir);
	}
	else
	{
		directory_listing << "<tr><td>Could not open directory</td></tr>";
	}

	directory_listing << "</table>";
	directory_listing << "</body></html>";
	return directory_listing.str();	
}

//get pages that will return an error or not
bool isAnErrorResponse(const std::string error_number)
{
	int error_code = std::atoi(error_number.c_str());
	
	if (error_code == 0 
		|| (error_code >= 200 && error_code <= 204)
		|| (error_code >= 301 && error_code <= 302)
		|| error_code == 401)
		return false;
	return true;
}

std::string &rtrim(std::string &s)
{
	s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
	return s;
}

std::string &ltrim(std::string &s)
{
	s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
	return s;
}

std::string trim_spaces(std::string &s)
{
	return ltrim(rtrim(s));
}

void close_all_sockets(std::vector<Server> &servers)
{
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		//remove all clients
		for (std::vector<Client>::iterator it2 = it->clients.begin(); it2 != it->clients.end(); ++it2)
		{
			if (it2->getFd() > 0)
			{
				if (close (it2->getFd()) < 0)
					throw std::runtime_error("Error: Could not close socket");
				std::cout << BLUE << "Client " << it2->getFd() << " : disconnected" << RESET << std::endl;
			}
		}
		
		if (it->getSocket() > 0)
		{
			if (close(it->getSocket()) < 0)
				throw std::runtime_error("Error: Could not close socket");
			std::cout << BLUE << "Server " << it->getServerName() << " : shutdown" << RESET << std::endl;
		}
	}
}