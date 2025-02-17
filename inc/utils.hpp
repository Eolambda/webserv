#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <webserv.hpp>

typedef struct ParserBlock
{
	bool server;
	bool error;
	bool location;
} t_parser_block;

std::string toBinaryString(uint8_t value);
bool check_extension(const std::string &str);
std::string getCurrentDate();
void parseLocationBlock(const std::string &key, const std::string &value, Server &current_server);
void parseServerBlock(const std::string &key, const std::string &value, Server &current_server);
void parseLine(const std::string &line, Server &current_server, t_parser_block &parser_position);
bool checkPathExists(const std::string &path);
bool isDirectory(const std::string &path);
std::string decodeURI(const std::string &uri);
std::string extractPathFromURI(const std::string uri);
std::string extractAttributesFromURI(const std::string uri);
std::string generateDirectorylisting(const std::string full_path);
bool isAnErrorResponse(const std::string error_number);
std::string &rtrim(std::string &s);
std::string &ltrim(std::string &s);
std::string trim_spaces(std::string &s);
void close_all_sockets(std::vector<Server> &servers);

#endif