/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:26:37 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/17 17:30:47 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>

Server::Server() : _sockfd(-1)
{
}

Server::Server(const Server &server)
{
	*this = server;
}

Server::~Server()
{
	if (_sockfd > 0)
		close(_sockfd);
	for (std::map<int, int>::iterator it = fd_to_sockfd.begin(); it != fd_to_sockfd.end(); ++it)
	{
		if (it->second == _sockfd)
			close(it->first);
	}
}

Server &Server::operator=(const Server &copy)
{
	if (this == &copy)
		return (*this);
	_hostname = copy._hostname;
	_port = copy._port;
	_server_name = copy._server_name;
	_max_body_size = copy._max_body_size;
	_root = copy._root;
	_default_file = copy._default_file;
	_locations = copy._locations;
	_error_pages = copy._error_pages;
	_cgi_extensions = copy._cgi_extensions;
	_cgibin = copy._cgibin;
	_sockfd = copy._sockfd;
	return (*this);
}

bool Server::operator==(const Server &copy) const
{
	return (_sockfd == copy._sockfd
		&& _hostname == copy._hostname
		&& _port == copy._port
		&& _server_name == copy._server_name);
}

int Server::getSocket(void) const
{
	return (_sockfd);
}

void Server::initSocket(void)
{
	char cwd[PATH_MAX];

	_root = std::string(getcwd(cwd, sizeof(cwd))) + "/" + _root;

	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr_len = sizeof(_addr);

	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Error: Could not create socket");

	int opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Error: Could not set socket options");

	if (fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, 0) | O_NONBLOCK) < 0)
		throw std::runtime_error("Error: Could not set socket options");

	if (max_fd <= _sockfd)
		max_fd = _sockfd;
	
	sockfd_to_server[_sockfd] = this;
	FD_SET(_sockfd, &current_fds);

	if (bind(_sockfd, (struct sockaddr *)&_addr, _addr_len) < 0)
		throw std::runtime_error("Error: Could not bind socket");

	if (listen(_sockfd, 10) < 0)
		throw std::runtime_error("Error: Could not listen to socket");
}

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

	std::cout << BLUE << "Config file parsed successfully" << std::endl
			  << RESET;

	return (servers);
}

//Other functions

void Server::addLocation(void)
{
	_locations.push_back(Location());
}

void Server::addCgiExtension(const std::string &key, const std::string &value)
{
	if (_cgi_extensions.find(key) == _cgi_extensions.end())
		_cgi_extensions[key] = value;
}

void Server::updateErrorPage(const int error_code, const std::string &value)
{
	_error_pages[error_code] = value;
}

//Getters and Setters

void Server::setHostname(const std::string &value)
{
	_hostname = value;
}

void Server::setPort(const int value)
{
	_port = value;
}

void Server::setServerName(const std::string &value)
{
	_server_name = value;
}

void Server::setMaxBodySize(const std::size_t value)
{
	_max_body_size = value;
}

void Server::setRoot(const std::string &value)
{
	_root = value;
}

void Server::setDefaultFile(const std::string &value)
{
	_default_file = value;
}

void Server::setCgiBin(const std::string &value)
{
	_cgibin = value;
}

std::vector<Location> &Server::getLocations(void)
{
	return (_locations);
}

std::map<int, std::string> &Server::getErrorPages(void)
{
	return (_error_pages);
}

std::string &Server::getHostname(void)
{
	return (_hostname);
}

int Server::getPort(void) const
{
	return (_port);
}

std::string &Server::getServerName(void)
{
	return (_server_name);
}

std::size_t Server::getMaxBodySize(void) const
{
	return (_max_body_size);
}

std::string &Server::getRoot(void)
{
	return (_root);
}

std::string &Server::getDefaultFile(void)
{
	return (_default_file);
}

std::string &Server::getCgiBin(void)
{
	return (_cgibin);
}

std::map<std::string, std::string> &Server::getCgiExtensions(void)
{
	return (_cgi_extensions);
}

void Server::readRequest(Client &client)
{
	char buffer[BUFFER_SIZE + 1];
	int bytes_received;
	int client_fd = client.getFd();

	bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

	if (bytes_received < 0)
	{
		std::cerr << RED << "Error: Could not receive data from client " << client_fd << RESET << std::endl;
		client.close_connection = true;
		return;
	}
	else if (bytes_received == 0)
	{
		client.close_connection = true;
		return;
	}

	buffer[bytes_received] = '\0';

	if (!client.getRequest()->isComplete())
	{
		
		std::cout << GREEN << "Request received from client " << client_fd << RESET << std::endl;
		
		client.getRequest()->readData(buffer);
		if (client.getRequest()->isComplete())
		{
			
				//debug stuff
				std::cout << std::endl;
				std::cout << "--- Request received by client " << client.getFd() << " ---" << std::endl;
				std::cout << "Full buffer : " << client.getRequest()->getBuffer() << std::endl;
				std::cout << "Method : " << client.getRequest()->getMethod() << std::endl;
				std::cout << "URI : " << client.getRequest()->getUri() << std::endl;
				std::cout << "HTTP version : " << client.getRequest()->getHttpVersion() << std::endl;
				// std::cout << "Body : " << client.getRequest()->getBody() << std::endl;
				std::cout << "Headers : " << std::endl;
				for (std::map<std::string, std::string>::iterator it = client.getRequest()->getHeaders().begin(); it != client.getRequest()->getHeaders().end(); ++it)
					std::cout << it->first << ": " << it->second << std::endl;
				std::cout << "Request validity : " << client.getRequest()->getRequestValidity() << std::endl;
				std::cout << "--- End of request, processing it ---" << std::endl;
				std::cout << std::endl;


			processRequest(client);
		}
	}
}

//handle the whole request, once it is complete, by filling response object and treating the request on the server side
void Server::processRequest(Client &client)
{	
	//set the server config in the response, so that we can access it later
	client.getResponse()->setServer(this);

	//check if the request is valid
	if (client.getRequest()->getRequestValidity() != 0)
	{
		std::string error_number = std::to_string(client.getRequest()->getRequestValidity());
		client.getResponse()->setStatusCode(error_number);
		return;
	}

	//get full path
	std::string root_path = this->getRoot();
	std::string extracted = extractPathFromURI(client.getRequest()->getUri());
	if (root_path.back() == '/' && extracted.front() == '/')
		extracted.erase(extracted.begin());
	else if (root_path.back() != '/' && extracted.front() != '/')
		extracted = "/" + extracted;
	std::string full_path  = root_path + extracted;

	//check for location properties
	std::string location_path;
	size_t pos;

	for (std::vector<Location>::iterator it = this->getLocations().begin(); it != this->getLocations().end(); ++it)
	{
		location_path = extractPathFromURI(client.getRequest()->getUri());

		//we check if location value ends with a /, location is a directory
		pos = location_path.find_last_of('/');
			if (pos != std::string::npos)
				location_path = location_path.substr(0, pos + 1);
		if ((*it).getPath().back() == '/' && location_path.back() != '/')
			location_path += "/";
		else if ((*it).getPath().back() != '/' && location_path.back() == '/')
			location_path.pop_back();

		//a location block exists, we handle it
		if (location_path == (*it).getPath())
		{
			//this is a redirection, we handle it
			if ((*it).getRedirect().empty() == false)
			{
					client.getResponse()->setStatusCode("301");
					client.getResponse()->setRedirection((*it).getRedirect());
					return;
			}
			
			//check if the method is allowed
			if (((*it).getAllowedMethods() & client.getRequest()->getMethodBit()) == 0)
			{
				client.getResponse()->setStatusCode("405");
				return;
			}

			//check if the request is a directory
			if (isDirectory(full_path))
			{
				//if yes, check if directory listing is allowed
					//if no, check if index file exists
						//if yes, change full path to index file
						//if no, 403 forbidden
				if ((*it).getDirectoryListing() == false)
				{
					if ((*it).getIndex().empty() == false)
						full_path += (*it).getIndex();
					else
					{
						client.getResponse()->setStatusCode("403");
						return;
					}
				}
			}
			break;
		}
	}

	client.getResponse()->setFullPath(full_path);
	client.getResponse()->setIsDirectory(isDirectory(full_path));

	//check if the file exists
	if (checkPathExists(full_path) == false)
	{
		client.getResponse()->setStatusCode("404");
		return;
	}

	//extract attributes from URI
	client.getResponse()->setURIAttributes(extractAttributesFromURI(client.getRequest()->getUri()));

	//specific method handlers
	if (client.getRequest()->getMethod() == "GET")
		client.getResponse()->handleGET();
	else if (client.getRequest()->getMethod() == "POST")
		client.getResponse()->handlePOST();
	else if (client.getRequest()->getMethod() == "DELETE")
		client.getResponse()->handleDELETE();
	else
		client.getResponse()->setStatusCode("405");

	return;
}

void Server::sendResponse(Client &client)
{
	int	send_ret = 0;

	//check if the response is being written, then prepare response and set it up for writing
	if (!client.getResponse()->is_being_written)
	{		
		client.getResponse()->prepareResponse();
		client.getResponse()->is_being_written = true;


			//print response for debug
			std::cout << std::endl;
			std::cout << "--- Response generated for client " << client.getFd() << " ---" << std::endl;
			std::cout << "Status code : " << client.getResponse()->getStatusCode() << std::endl;
			std::cout << "Status message : " << client.getResponse()->getStatusMessage() << std::endl;
			std::cout << "Headers : " << std::endl;
			std::cout << client.getResponse()->getHeaders() << std::endl;
			// std::cout << "Body : " << std::endl;
			// std::cout << client.getResponse()->getBody() << std::endl;
			std::cout << "Full path : " << client.getResponse()->getFullPath() << std::endl;
			std::cout << "URI attributes : " << client.getResponse()->getURIAttributes() << std::endl;
			std::cout << "Redirection : " << client.getResponse()->getRedirection() << std::endl;
			std::cout << "Content type : " << client.getResponse()->getContentType() << std::endl;
			std::cout << "HTTP version : " << client.getResponse()->getHTTPVersion() << std::endl;
			std::cout << "Is directory : " << client.getResponse()->getIsDirectory() << std::endl;
			std::cout << "--- End of response ---" << std::endl;
			std::cout << std::endl;
	}

	//we write the response
	if (client.getResponse()->is_being_written == true)
	{
		if (client.getResponse()->getBuffer().length() <= BUFFER_SIZE)
			send_ret = write(client.getFd(), client.getResponse()->getBuffer().c_str(), client.getResponse()->getBuffer().length());
		else
			send_ret = write(client.getFd(), client.getResponse()->getBuffer().c_str(), BUFFER_SIZE);
	}

	//we check return of write
	if (send_ret < 0)
	{
		std::cout << RED << "Error sending data to Client " << client.getFd() << RESET << std::endl;
		client.close_connection = true;
		return;
	}
	else if (send_ret == 0 || client.getResponse()->getBuffer().length() == static_cast<size_t>(send_ret))
	{
		client.getResponse()->is_being_written = false;
		client.getResponse()->setBuffer("");
	}
	else
		client.getResponse()->setBuffer(client.getResponse()->getBuffer().substr(send_ret));

	//if response is complete, we reset the response and request
	if (client.getResponse()->is_being_written == false)
	{
		if (client.getResponse()->getStatusCode() == "400" || client.getResponse()->getStatusCode() == "500" || client.getRequest()->getHeader("Connection") == "close")
			client.close_connection = true;
		else
		{
			//for now, disconnect client and remove it from server after each successful request
			client.close_connection = false;
			client.resetMessages();
		}

			//debug
			std::cout << GREEN << "Response succesfully sent to Client " << client.getFd() << RESET << std::endl;
	}
}