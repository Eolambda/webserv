/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:24:48 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/20 16:08:04 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <webserv.hpp>

class Location;
class Client;

class Server
{
public:
	Server();
	Server(const Server &server);
	~Server();
	Server &operator=(const Server &copy);
	bool 	operator==(const Server &copy) const;
	static std::vector<Server> parseConfigFile(const std::string &filename);

	void addLocation(void);
	void addCgiExtension(const std::string &key, const std::string &value);

	void updateErrorPage(const int error_code, const std::string &value);

	void setHostname(const std::string &value);
	void setPort(const int value);
	void setServerName(const std::string &value);
	void setMaxBodySize(const std::size_t value);
	void setRoot(const std::string &value);
	void setDefaultFile(const std::string &value);
	void setCgiBin(const std::string &value);
	void setUploads(const std::string &value);

	std::vector<Location> &getLocations(void);
	std::map<int, std::string> &getErrorPages(void);
	std::string &getErrorPage(const int error_code);
	std::string &getHostname(void);
	int getPort(void) const;
	std::string &getServerName(void);
	std::size_t getMaxBodySize(void) const;
	std::string &getRoot(void);
	std::string &getDefaultFile(void);
	std::string &getCgiBin(void);
	std::string &getUploads(void);
	std::map<std::string, std::string> &getCgiExtensions(void);
	int getSocket(void) const;

	void initSocket(void);


	void readRequest(Client &client);
	void readCGI(Client &client);
	void processRequest(Client &client);
	void sendResponse(Client &client);
	
	//public attributes
	std::vector<Client> clients;

	
private:
	std::string _hostname;
	int _port;
	std::string _server_name;
	std::size_t _max_body_size;
	std::string _root;
	std::string _default_file;
	std::string	_cgibin;
	std::string _uploads;
	std::map<std::string, std::string> _cgi_extensions;
	std::vector<Location> _locations;
	std::map<int, std::string> _error_pages;
	int _sockfd;
	struct sockaddr_in _addr;
	id_t _addr_len;
};
#endif
