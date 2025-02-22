#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <webserv.hpp>

class Server;
class Request;
class Response;

class Client
{
	public:
		bool close_connection;

		Client();
		Client(int fd, struct sockaddr_in addr, Server *server);
		Client(const Client &client);
		~Client();
		Client &operator=(const Client &copy);
		bool 	operator==(const Client &copy) const;

		void setFd(const int fd);
		void setServer(Server *server);
		void setTimer(double timeout);

		int  getFd(void);
		struct sockaddr_in getAddr(void);
		Request* getRequest(void);
		Response* getResponse(void);
		Server* getServer(void);
		int* getCgiPipes(void);
		int* getCgiPipes_POST(void);
		double getTimer(void);

		void resetMessages(void);

	private:
		int	_fd;
		int _cgi_pipes[2];
		int _cgi_pipes_POST[2];
		struct sockaddr_in _addr;
		double _timeout;
		
		//Check if we need pointers here or not
		Request*	_request;
		Response*	_response;
		Server*		_server;
};

#endif