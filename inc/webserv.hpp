/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:16:01 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/22 22:57:40 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __WEBSERV_HPP__
#define __WEBSERV_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <iterator>
#include <map>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
// #include <linux/limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <arpa/inet.h>  // For inet_ntoa (Linux/macOS)

#include <Server.hpp>
#include <Location.hpp>
#include <Client.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <utils.hpp>
#include <cgi.hpp>

#define BOLD "\033[1m"
#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define LIGHT_BLUE "\033[94m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define LIGHT_RED "\033[91m"
#define LIGHT_GREEN "\033[92m"
#define WHITE "\033[37m"
#define GREY "\033[90m"
#define BRIGHT_RED "\033[91m"
#define BRIGHT_GREEN "\033[92m"
#define RESET "\033[0m"

#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS 1000
#define REQUEST_TIMEOUT 512
#define CGI_TIMEOUT 512
#define COOKIES_EXPIRY_TIME 2419200

#define GET_BIT 0x01
#define POST_BIT 0x02
#define DELETE_BIT 0x04

#define CRLF "\r\n"
#define DOUBLECRLF "\r\n\r\n"

#define HTTP_ERROR_BAD_REQUEST 400

class Server;
class Client;
typedef struct ParserBlock t_parser_block;

bool check_extension(const std::string &str);
void parseLine(const std::string &line, Server &current_server, t_parser_block &parser_position);
void parseServerBlock(const std::string &key, const std::string &value, Server &current_server);
void parseLocationBlock(const std::string &key, const std::string &value, Server &current_server);

extern int max_fd;
extern std::map<int, Server *> sockfd_to_server;
extern std::map<int, int> fd_to_sockfd;
extern fd_set current_fds, write_fds, read_fds;

#endif