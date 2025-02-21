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
		void setToUpload(const std::string &to_upload);
		void setContentLength(int content_length);
		void setRequest(Request *request);
		void setCgiBuffer(const std::string &cgi_buffer);
		void setIsCgi(bool is_cgi);

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
		std::string getToUpload() const;
		int getContentLength() const;
		Request *getRequest() const;
		std::string getCgiBuffer() const;
		bool getIsCgi() const;

		void handleGET(Client *client);
		void handlePOST();
		void HandlePOST_multiform(std::string body, std::string content_type);
		void handleDELETE();

		void prepareResponse();
		void defineContentType();
		void defineStatusMessage(const std::string status_number);
		void defineResponseHeaders();
		void defineResponseErrorPage();
		void getFileContent();

		void resetResponse();
	
	private:
		Server 		*_server;
		Request		*_request;
		std::string _buffer;
		std::string	_status_code;
		std::string _status_message;
		std::string _headers;
		std::string _body;
		std::string _full_path;
		std::string _uri_attributes;
		std::string _redirection;
		std::string _content_type;
		int			_content_length;
		std::string _http_version;
		std::string _to_upload;
		std::string _cgi_buffer;
		bool _is_directory;
		bool _is_cgi;
};

#endif