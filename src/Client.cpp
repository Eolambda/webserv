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