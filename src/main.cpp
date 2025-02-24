/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:16:04 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/24 12:30:34 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <webserv.hpp>

int debug = 1;
int max_fd = 0;
fd_set current_fds, write_fds, read_fds;
volatile sig_atomic_t loop = 1;

//rebuild current fd lists, close all disconnected clients sockets and recalculates max_fd
void monitor_connexions(std::vector<Server> &servers)
{
	FD_ZERO(&current_fds);
	max_fd = 0;

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		FD_SET(it->getSocket(), &current_fds);
		if (it->getSocket() >= max_fd)
			max_fd = it->getSocket();
			
		for (std::vector<Client>::iterator it2 = it->clients.begin(); it2 != it->clients.end();)
		{
			if (it2->close_connection)
			{
				it2->closeSockets();
				std::cout << GREEN << "Client " << it2->getFd() << " : disconnected" << RESET << std::endl;				
				it2 = it->clients.erase(it2);
			}
			else
			{
				FD_SET(it2->getFd(), &current_fds);
				if (it2->getFd() >= max_fd)
					max_fd = it2->getFd();
				if (it2->getCgiPipes()[0] != -1)
				{
					FD_SET(it2->getCgiPipes()[0], &current_fds);
					if (it2->getCgiPipes()[0] >= max_fd)
						max_fd = it2->getCgiPipes()[0];
				}
				if (it2->getCgiPipes_POST()[1] != -1)
				{
					FD_SET(it2->getCgiPipes_POST()[1], &current_fds);
					if (it2->getCgiPipes_POST()[1] >= max_fd)
						max_fd = it2->getCgiPipes_POST()[1];
				}
				++it2;
			}
		}
	}
}

//loop through al servers to check if they have a client
//for each client, read or write message depending of the state of the request
void handle_clients(std::vector<Server> &servers)
{
	Client *client;
	
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		for (std::vector<Client>::iterator it2 = it->clients.begin(); it2 != it->clients.end(); ++it2)
		{
			client = &(*it2);
			
			//check if the request is too old
			if (client->getRequest() != NULL && client->getRequest()->getCreationTime() < (get_time() - REQUEST_TIMEOUT))
			{				
				client->close_connection = true;
				std::cout << GREEN << "Client " << client->getFd() << " : request timeout" << RESET << std::endl;
			}
			//check if there is stuff to be read from clients
			else if (FD_ISSET(client->getFd(), &read_fds))
				(*it).readRequest(*client);
			//check if there is a cgi to write to the cgi_process
			else if (client->getRequest()->isComplete() && client->getResponse()->getIsCgi() == true
			&& client->getCgiPipes_POST()[1] != -1 && client->getRequest()->getMethod() == "POST"
			&& FD_ISSET(client->getCgiPipes_POST()[1], &write_fds))
				(*it).sendCGI(*client);
			//check if there is cgi data to read from the cgi_process
			else if (client->getRequest()->isComplete() && client->getResponse()->getIsCgi() == true
			&& client->getCgiPipes()[0] != -1 && client->getCgiPipes_POST()[1] == -1
			&& FD_ISSET(client->getCgiPipes()[0], &read_fds))
				(*it).receiveCGI(*client);
			//check if there is a response to send to the client
			else if (FD_ISSET(client->getFd(), &write_fds) && client->getRequest()->isComplete()
			&& client->getCgiPipes()[0] == -1)
				(*it).sendResponse(*client);
		}
	}
}

void check_new_clients(std::vector<Server> &servers)
{	
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		//check max number of connexions here
		if (FD_ISSET(it->getSocket(), &read_fds))
		{
			int new_fd;
			struct sockaddr_in new_addr;
			id_t new_addrlen = sizeof(new_addr);

			new_fd = accept(it->getSocket(), (struct sockaddr *)&new_addr, &new_addrlen);
			if (new_fd < 0)
				return ;
			fcntl(new_fd, F_SETFL, O_NONBLOCK);
			FD_SET(new_fd, &current_fds);
			if (new_fd > max_fd)
				max_fd = new_fd;

			it->clients.push_back(Client(new_fd, new_addr, &(*it)));

			std::cout << GREEN << "New connection from " << inet_ntoa(new_addr.sin_addr) << ":" << ntohs(new_addr.sin_port) << ": Client " << new_fd << RESET << std::endl;
		}
	}
}

void siginthandle(int sig)
{
	(void)sig;
	std::cout << YELLOW << "SIGINT received, exiting..." << RESET << std::endl;
	loop = 0;
}

void loop_handle(std::vector<Server> &servers)
{
	int ret;

	clean_cookies(servers);
	
	read_fds = write_fds = current_fds;
	if ((ret = select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0) && loop == 1)
	{
		throw std::runtime_error("Error: Could not select");
		return ;
	}
	else if (loop == 0)
		return ;

	check_new_clients(servers);	
	handle_clients(servers);
	monitor_connexions(servers);
}

int main(int argc, char **argv)
{
	std::string filename;

	if (argc == 2)
		filename = argv[1];
	else if (argc == 1)
	{
		filename = "configfiles/default.conf";
		std::cout << YELLOW << "No config file specified, using default config file 'configfiles/default.conf'" << RESET << std::endl;
	}
	else
	{
		std::cerr << RED << "Error: too many arguments" << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	if (!check_extension(filename))
	{
		std::cerr << RED << "Error: invalid file extension" << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	try
	{
		std::vector<Server> servers = Server::parseConfigFile(filename);

		FD_ZERO(&current_fds);
		for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			std::cout << it->getServerName() << '\n'
					  << it->getHostname() << '\n'
					  << it->getPort() << '\n'
					  << std::endl;

			it->initSocket();
		}
		
		signal(SIGINT, siginthandle);	
		while (loop)
			loop_handle(servers);
		
		shutdown_server(servers);
		std::cout << BLUE << "All sockets closed, exiting..." << RESET << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << RED << "Exception caught: " << e.what() << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
