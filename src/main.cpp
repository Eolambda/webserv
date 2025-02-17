/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:16:04 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/17 17:26:49 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <webserv.hpp>

int max_fd = 0;
std::map<int, Server *> sockfd_to_server;
std::map<int, int> fd_to_sockfd;
fd_set current_fds, write_fds, read_fds;

volatile sig_atomic_t loop = 1;

//rebuild client lists, close all disconnected clients sockets and recalculates max_fd
void remove_clients(std::vector<Server> &servers)
{
	max_fd = 0;

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (it->getSocket() >= max_fd)
			max_fd = it->getSocket();
			
		for (std::vector<Client>::iterator it2 = it->clients.begin(); it2 != it->clients.end();)
		{
			if (it2->close_connection)
			{
				FD_CLR(it2->getFd(), &current_fds);
				if (close(it2->getFd()) < 0)
					throw std::runtime_error("Error: Could not close socket");
				std::cout << GREEN << "Client " << it2->getFd() << " : disconnected" << RESET << std::endl;				
				fd_to_sockfd.erase(it2->getFd());
				it2 = it->clients.erase(it2);
			}
			else
			{
				if (it2->getFd() >= max_fd)
					max_fd = it2->getFd();
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

			if (FD_ISSET(client->getFd(), &read_fds))
				(*it).readRequest(*client);
			else if (FD_ISSET(client->getFd(), &write_fds) && client->getRequest()->isComplete())
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
			fd_to_sockfd[new_fd] = it->getSocket();

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
	remove_clients(servers);
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
		
		close_all_sockets(servers);
		std::cout << BLUE << "All sockets closed, exiting..." << RESET << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << RED << "Exception caught: " << e.what() << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
