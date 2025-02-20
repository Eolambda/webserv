#ifndef __CGI_HPP__
#define __CGI_HPP__

#include <webserv.hpp>

bool is_cgi(const Request &request);
void handle_cgi(Client *client);
void execute_cgi(std::vector<std::string> cmd, std::vector<std::string> env, Client *client);
std::vector<std::string> generate_cgi_env(Client *client, std::string command, std::string file);

#endif