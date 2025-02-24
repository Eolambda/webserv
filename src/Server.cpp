/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:26:37 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/24 18:11:25 by vincentfres      ###   ########.fr       */
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
	//clean session store
	for (std::map<std::string, struct SessionData>::iterator it = _session_store.begin(); it != _session_store.end();)
		it = _session_store.erase(it);
		
	//clean clients
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end();)
	{
		it->closeSockets();
		std::cout << BLUE << "Client " << it->getFd() << " : disconnected" << RESET << std::endl;
		it = clients.erase(it);
	}
	
	//close and clean sockets
	if (_sockfd > 0)
		close(_sockfd);
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
	_uploads = copy._uploads;
	_addr = copy._addr;
	_addr_len = copy._addr_len;
	_session_store = copy._session_store;
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
	#ifdef SO_REUSEPORT  // Some systems do not support SO_REUSEPORT
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Error: Could not set SO_REUSEPORT");
	#endif

	if (fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, 0) | O_NONBLOCK) < 0)
		throw std::runtime_error("Error: Could not set socket options");

	if (max_fd <= _sockfd)
		max_fd = _sockfd;
	
	FD_SET(_sockfd, &current_fds);

	if (bind(_sockfd, (struct sockaddr *)&_addr, _addr_len) < 0)
		throw std::runtime_error("Error: Could not bind socket");

	if (listen(_sockfd, 256) < 0)
		throw std::runtime_error("Error: Could not listen to socket");
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

void Server::setUploads(const std::string &value)
{
	_uploads = value;
}

void Server::addNewSession(const std::string &key, const std::string &value)
{
	struct SessionData new_session;
	
	new_session.data = value;
	new_session.last_access = get_time();
	_session_store[key] = new_session;
}

//Getters


std::vector<Location> &Server::getLocations(void)
{
	return (_locations);
}

std::map<int, std::string> &Server::getErrorPages(void)
{
	return (_error_pages);
}

std::string &Server::getErrorPage(const int error_code)
{
	return (_error_pages[error_code]);
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

std::string &Server::getUploads(void)
{
	return (_uploads);
}

std::map<std::string, std::string> &Server::getCgiExtensions(void)
{
	return (_cgi_extensions);
}

std::map<std::string, struct SessionData> &Server::getSessionStore(void)
{
	return (_session_store);
}


void Server::readRequest(Client &client)
{
	char buffer[BUFFER_SIZE + 1];
	int bytes_received;
	int client_fd = client.getFd();

	bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);

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

	if (debug)
		std::cout << CYAN << "Request data received from client " << client_fd << RESET << std::endl;

	if (!client.getRequest()->isComplete())
	{	
		std::string data(buffer, bytes_received);	
		client.getRequest()->readData(data);

		if (client.getRequest()->isComplete())
		{
			
			if (debug)
			{
				std::cout << CYAN << std::endl << RESET;
				std::cout << CYAN << "--- Request received by client " << client.getFd() << " ---" << std::endl << RESET;;
				std::cout << CYAN << "Full buffer : " << client.getRequest()->getBuffer() << std::endl << RESET;;
				std::cout << CYAN << "Method : " << client.getRequest()->getMethod() << std::endl << RESET;;
				std::cout << CYAN << "URI : " << client.getRequest()->getUri() << std::endl << RESET;;
				std::cout << CYAN << "HTTP version : " << client.getRequest()->getHttpVersion() << std::endl << RESET;;
				std::cout << CYAN << "Body : " << client.getRequest()->getBody() << std::endl << RESET;;
				std::cout << CYAN << "Headers : " << std::endl << RESET;;
				for (std::map<std::string, std::string>::iterator it = client.getRequest()->getHeaders().begin(); it != client.getRequest()->getHeaders().end(); ++it)
					std::cout << CYAN << it->first << ": " << it->second << std::endl << RESET;;
				std::cout << CYAN << "Request validity : " << client.getRequest()->getRequestValidity() << std::endl << RESET;;
				std::cout << CYAN << "--- End of request, processing it ---" << std::endl << RESET;;
				std::cout << CYAN << std::endl << RESET;
			}

			processRequest(client);
		}
	}
}

//handle the whole request, once it is complete, by filling response object and treating the request on the server side
void Server::processRequest(Client &client)
{	
	//set the server config in the response, so that we can access it later
	client.getResponse()->setServer(this);
	client.getResponse()->setRequest(client.getRequest());

		
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
					client.getResponse()->setRedirection((*it).getRedirect());
					//only actively redirects on GET and DELETE, POST is handled differently
					if (client.getRequest()->getMethod() == "GET" || client.getRequest()->getMethod() == "DELETE")
					{
						client.getResponse()->setStatusCode("301");
						return;
					}
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

	//set full path
	client.getResponse()->setFullPath(full_path);
	client.getResponse()->setIsDirectory(isDirectory(full_path));

	//check if the file exists, in case of GET or DELETE
	if (checkPathExists(full_path) == false && client.getRequest()->getMethod() != "POST")
	{
		client.getResponse()->setStatusCode("404");
		return;
	}
	
	//in case of POST, save the body size
	if (client.getRequest()->getMethod() == "POST")
		client.getResponse()->setContentLength(client.getRequest()->getBody().size());

	//extract attributes from URI
	client.getResponse()->setURIAttributes(extractAttributesFromURI(client.getRequest()->getUri()));
			

	//check if the request is a CGI
	if (is_cgi(client.getRequest(), &client))
		client.getResponse()->setIsCgi(true);
	else
		client.getResponse()->setIsCgi(false);

	//check if in the cookie data there is a session id
	if (client.getRequest()->getHeader("Cookie").empty() == false)
	{
		std::string cookie_data = client.getRequest()->getHeader("Cookie");
		if (cookie_data.find("session_id=") != std::string::npos || cookie_data.find("SESSIONID=") != std::string::npos || cookie_data.find("session=") != std::string::npos)
		{
			std::string session_id;
			if (cookie_data.find("session_id=") != std::string::npos)
				session_id = cookie_data.substr(cookie_data.find("session_id=") + 10);
			else if (cookie_data.find("session=") != std::string::npos)
				session_id = cookie_data.substr(cookie_data.find("session=") + 8);
			else
				session_id = cookie_data.substr(cookie_data.find("SESSIONID=") + 9);
				
			if (session_id.find(';') != std::string::npos)
				session_id = session_id.substr(0, session_id.find(';'));

			//check if the session id is valid and stores value in storage
			if (_session_store.find(session_id) != _session_store.end())
			{
				_session_store[session_id].last_access = get_time();
				_session_store[session_id].data = cookie_data;
			}
		}
		else
		{
			//if no session id, we create a new one
			std::string new_session_id = generateSessionId();
			client.getResponse()->setSessionId(new_session_id);
			addNewSession(new_session_id, cookie_data);
		}
	}

		
	//specific method handlers
	if (client.getRequest()->getMethod() == "GET")
		client.getResponse()->handleGET(&client);
	else if (client.getRequest()->getMethod() == "POST")
		client.getResponse()->handlePOST(&client);
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
		if (debug)
		{
			std::cout << PURPLE << std::endl;
			std::cout << PURPLE << "--- Response generated for client " << client.getFd() << " ---" << std::endl << RESET;
			std::cout << PURPLE << "Status code : " << client.getResponse()->getStatusCode() << std::endl << RESET;
			std::cout << PURPLE << "Status message : " << client.getResponse()->getStatusMessage() << std::endl << RESET;
			std::cout << PURPLE << "Headers : " << std::endl << RESET;
			std::cout << PURPLE << client.getResponse()->getHeaders() << std::endl << RESET;
			std::cout << PURPLE << "Body : " << std::endl << RESET;
			std::cout << PURPLE << client.getResponse()->getBody() << std::endl << RESET;
			std::cout << PURPLE << "Full path : " << client.getResponse()->getFullPath() << std::endl << RESET;
			std::cout << PURPLE << "URI attributes : " << client.getResponse()->getURIAttributes() << std::endl << RESET;
			std::cout << PURPLE << "Redirection : " << client.getResponse()->getRedirection() << std::endl << RESET;
			std::cout << PURPLE << "Content type : " << client.getResponse()->getContentType() << std::endl << RESET;
			std::cout << PURPLE << "HTTP version : " << client.getResponse()->getHTTPVersion() << std::endl << RESET;
			std::cout << PURPLE << "Is directory : " << client.getResponse()->getIsDirectory() << std::endl << RESET;
			std::cout << PURPLE << "Is CGI : " << client.getResponse()->getIsCgi() << std::endl << RESET;
			std::cout << PURPLE << "CGI Buffer : " << client.getResponse()->getCgiBuffer() << std::endl << RESET;
			std::cout << PURPLE << "--- End of response ---" << std::endl << RESET;
			std::cout << PURPLE << std::endl << RESET;
		}
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