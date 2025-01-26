#pragma once

#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <webserv.hpp>

#define REQUEST_FIRST_LINE 0
#define REQUEST_HEADERS 1
#define REQUEST_BODY 2

class Request
{
	public:
		Request();
		Request(const Request &request);
		~Request();
		Request &operator=(const Request &copy);

		void setMethod(const std::string &method);
		void setUri(const std::string &uri);
		void setHttpVersion(const std::string &http_version);
		void setHeaders(const std::map<std::string, std::string> &headers);
		void setBody(const std::string &body);

		const std::string &getBuffer(void) const;
		const std::string &getLastLine(void) const;
		const std::string &getMethod(void) const;
		const std::string &getUri(void) const;
		const std::string &getHttpVersion(void) const;
		const std::map<std::string, std::string> &getHeaders(void) const;
		const std::string &getBody(void) const;
		const int &getRequestValidity(void) const;

		void readData(std::string data);
		void setRequestValidity(int value, bool is_complete);
		bool parseFirstLine(std::string line);
		bool parseHeaders(std::string line);
		bool parseBody(std::string line);

	private:
		std::string _buffer;
		std::string _last_line;
		std::string _method;
		std::string _uri;
		std::string _http_version;
		std::map<std::string, std::string> _headers;
		std::string _body;
		bool _is_complete;
		int _is_valid;
		int _parsing_state; //0 = first line, 1 = headers, 2 = body
		std::string _full_path;
		bool _is_directory;
};

#endif