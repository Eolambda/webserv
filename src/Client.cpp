#include <Client.hpp>

Client::Client()
{
	_request = new Request(this);
	_response = new Response();

	_fd = -1;
	_cgi_pipes[0] = -1;
	_cgi_pipes[1] = -1;
	_cgi_pipes_POST[0] = -1;
	_cgi_pipes_POST[1] = -1;
	close_connection = false;
	_cgitimeout = 0;
}

Client::Client(int fd, struct sockaddr_in addr, Server *server)
{
	_fd = fd;
	_addr = addr;
	_cgi_pipes[0] = -1;
	_cgi_pipes[1] = -1;
	_cgi_pipes_POST[0] = -1;
	_cgi_pipes_POST[1] = -1;
	_server = server;
	close_connection = false;
	_cgitimeout = 0;
	_request = new Request(this);
	_response = new Response();
}

Client::Client(const Client &client)
{
	_fd = client._fd;
	_cgi_pipes[0] = client._cgi_pipes[0];
	_cgi_pipes[1] = client._cgi_pipes[1];
	_cgi_pipes_POST[0] = client._cgi_pipes_POST[0];
	_cgi_pipes_POST[1] = client._cgi_pipes_POST[1];
	_server = client._server;
	_addr = client._addr;
	close_connection = client.close_connection;
	_cgitimeout = client._cgitimeout;
	_request = new Request(*(client._request));
	_response = new Response(*(client._response));
}

Client::~Client()
{
	delete _request;
	delete _response;
}

Client &Client::operator=(const Client &copy)
{
	delete _request;
	delete _response;

	_request = new Request(*(copy._request));
	_response = new Response(*(copy._response));
	_fd = copy._fd;
	_cgi_pipes[0] = copy._cgi_pipes[0];
	_cgi_pipes[1] = copy._cgi_pipes[1];
	_cgi_pipes_POST[0] = copy._cgi_pipes_POST[0];
	_cgi_pipes_POST[1] = copy._cgi_pipes_POST[1];
	_server = copy._server;
	_addr = copy._addr;
	close_connection = copy.close_connection;
	_cgitimeout = copy._cgitimeout;
	return *this;
}

bool Client::operator==(const Client &copy) const
{
	return (_fd == copy._fd);
}

void Client::closeSockets()
{
	if (_fd > 0)
		close(_fd);
	if (_cgi_pipes[0] > 0)
		close(_cgi_pipes[0]);
	if (_cgi_pipes[1] > 0)
		close(_cgi_pipes[1]);
	if (_cgi_pipes_POST[0] > 0)
		close(_cgi_pipes_POST[0]);
	if (_cgi_pipes_POST[1] > 0)
		close(_cgi_pipes_POST[1]);
}

void Client::setFd(const int fd)
{
	_fd = fd;
}

void Client::setServer(Server *server)
{
	_server = server;
}

void Client::setCGITimer(double timeout)
{
	_cgitimeout = timeout;
}

struct sockaddr_in Client::getAddr()
{
	return _addr;
}

int Client::getFd()
{
	return _fd;
}

Request *Client::getRequest()
{
	return _request;
}

Response *Client::getResponse()
{
	return _response;
}

Server *Client::getServer()
{
	return _server;
}

int *Client::getCgiPipes()
{
	return _cgi_pipes;
}

int *Client::getCgiPipes_POST()
{
	return _cgi_pipes_POST;
}

double Client::getCGITimer()
{
	return _cgitimeout;
}

void Client::resetMessages()
{
	delete _request;
	delete _response;

	_request = new Request(this);
	_response = new Response();
	_cgitimeout = 0;
	if (_cgi_pipes[0] > 0)
		close(_cgi_pipes[0]);
	_cgi_pipes[0] = -1;
	if (_cgi_pipes[1] > 0)
		close(_cgi_pipes[1]);
	_cgi_pipes[1] = -1;
	if (_cgi_pipes_POST[0] > 0)
		close(_cgi_pipes_POST[0]);
	_cgi_pipes_POST[0] = -1;
	if (_cgi_pipes_POST[1] > 0)
		close(_cgi_pipes_POST[1]);
	_cgi_pipes_POST[1] = -1;
}

void Client::printRequest()
{
	Client &client = *this;

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

void Client::printResponse()
{
	Client &client = *this;
	
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