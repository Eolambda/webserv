#pragma once

#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include <webserv.hpp>

class Response
{
	public:
		bool is_being_written;
		bool is_complete;

		Response();
		Response(const Response &response);
		~Response();
		Response &operator=(const Response &copy);

		void setRedirection(const std::string &redirection);
		void setFullPath(const std::string &full_path);
		void setURIAttributes(const std::string &uri_attributes);
		void setServer(Server *server);
		void setIsDirectory(bool is_directory);
		void setHTTPVersion(const std::string &http_version);
		void setStatusCode(std::string status_code);
		void setStatusMessage(const std::string &status_message);
		void setHeaders(const std::string &headers);
		void setBuffer(const std::string &buffer);

		std::string getHeaders() const;
		std::string getHeaders(const std::string &header) const;
		std::string getBuffer() const;
		std::string getContentType() const;
		std::string getFullPath() const;
		std::string getURIAttributes() const;
		std::string getRedirection() const;
		std::string getStatusCode() const;
		std::string getStatusMessage() const;
		std::string getBody() const;
		std::string getHTTPVersion() const;
		bool getIsDirectory() const;
		Server *getServer() const;

		void handleGET();
		void handlePOST();
		void handleDELETE();

		void prepareResponse();
		void defineContentType();
		void defineStatusMessage(const std::string status_number);
		void defineResponseHeaders();
		void getFileContent();

		void resetResponse();
	
	private:
		Server 		*_server;
		std::string _buffer;
		std::string	_status_code;
		std::string _status_message;
		std::string _headers;
		std::string _body;
		std::string _full_path;
		std::string _uri_attributes;
		std::string _redirection;
		std::string _content_type;
		std::string _http_version;
		bool _is_directory;
};

#endif