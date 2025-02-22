#include <Response.hpp>

Response::Response()
{
	is_complete = false;
	is_being_written = false;
	_server = NULL;
	_request = NULL;
	_is_directory = false;
	_content_length = 0;
	_buffer = "";
	_cgi_buffer = "";
	_is_cgi = false;
	_is_post_upload = false;
	_sessionid = "";
}

Response::Response(const Response &response)
{
	_status_code = response._status_code;
	_status_message = response._status_message;
	_headers = response._headers;
	_body = response._body;
	is_complete = response.is_complete;
	is_being_written = response.is_being_written;
	_full_path = response._full_path;
	_uri_attributes = response._uri_attributes;
	_redirection = response._redirection;
	_content_type = response._content_type;
	_http_version = response._http_version;
	_server = response._server;
	_is_directory = response._is_directory;
	_to_upload = response._to_upload;
	_buffer = response._buffer;
	_content_length = response._content_length;
	_request = response._request;
	_server = response._server;
	_cgi_buffer = response._cgi_buffer;
	_is_cgi = response._is_cgi;
}

Response::~Response()
{
}

Response &Response::operator=(const Response &copy)
{
	_status_code = copy._status_code;
	_status_message = copy._status_message;
	_headers = copy._headers;
	_body = copy._body;
	is_complete = copy.is_complete;
	is_being_written = copy.is_being_written;
	_full_path = copy._full_path;
	_uri_attributes = copy._uri_attributes;
	_redirection = copy._redirection;
	_content_type = copy._content_type;
	_http_version = copy._http_version;
	_server = copy._server;
	_is_directory = copy._is_directory;
	_to_upload = copy._to_upload;
	_buffer = copy._buffer;
	_content_length = copy._content_length;
	_request = copy._request;
	_server = copy._server;
	_cgi_buffer = copy._cgi_buffer;
	_is_cgi = copy._is_cgi;
	return *this;
}

void Response::setRedirection(const std::string &redirection)
{
	_redirection = redirection;
}

void Response::setFullPath(const std::string &full_path)
{
	_full_path = full_path;
}

void Response::setURIAttributes(const std::string &uri_attributes)
{
	_uri_attributes = uri_attributes;
}

void Response::setServer(Server *server)
{
	_server = server;
}

void Response::setIsDirectory(bool is_directory)
{
	_is_directory = is_directory;
}

void Response::setHTTPVersion(const std::string &http_version)
{
	_http_version = http_version;
}

void Response::setStatusCode(std::string status_code)
{
	_status_code = status_code;
}

void Response::setStatusMessage(const std::string &status_message)
{
	_status_message = status_message;
}

void Response::setHeaders(const std::string &headers)
{
	_headers = headers;
}

void Response::setBuffer(const std::string &buffer)
{
	_buffer = buffer;
}

void Response::setToUpload(const std::string &to_upload)
{
	_to_upload = to_upload;
}

void Response::setContentLength(int content_length)
{
	_content_length = content_length;
}

void Response::setRequest(Request *request)
{
	_request = request;
}

void Response::setCgiBuffer(const std::string &cgi_buffer)
{
	_cgi_buffer = cgi_buffer;
}

void Response::setIsCgi(bool is_cgi)
{
	_is_cgi = is_cgi;
}

void Response::setIsPostUpload(bool is_post_upload)
{
	_is_post_upload = is_post_upload;
}

void Response::setSessionId(const std::string &sessionid)
{
	_sessionid = sessionid;
}



std::string Response::getHeaders(void) const
{
	return _headers;
}

std::string Response::getHeaders(const std::string &header) const
{
	std::string header_value;
	size_t pos;

	if ((pos = _headers.find(header)) != std::string::npos)
	{
		header_value = _headers.substr(pos + header.size() + 2);
		if ((pos = header_value.find(CRLF)) != std::string::npos)
			header_value = header_value.substr(0, pos);
	}
	return header_value;
}

std::string Response::getBuffer(void) const
{
	return _buffer;
}

std::string Response::getContentType(void) const
{
	return _content_type;
}

std::string Response::getFullPath(void) const
{
	return _full_path;
}

std::string Response::getURIAttributes(void) const
{
	return _uri_attributes;
}

std::string Response::getRedirection(void) const
{
	return _redirection;
}

std::string Response::getStatusCode(void) const
{
	return _status_code;
}

std::string Response::getStatusMessage(void) const
{
	return _status_message;
}

std::string Response::getBody(void) const
{
	return _body;
}

std::string Response::getHTTPVersion(void) const
{
	return _http_version;
}

bool Response::getIsDirectory(void) const
{
	return _is_directory;
}

Server *Response::getServer(void) const
{
	return _server;
}

std::string Response::getToUpload(void) const
{
	return _to_upload;
}

int Response::getContentLength(void) const
{
	return _content_length;
}

Request *Response::getRequest(void) const
{
	return _request;
}

std::string Response::getCgiBuffer(void) const
{
	return _cgi_buffer;
}

bool Response::getIsCgi(void) const
{
	return _is_cgi;
}

bool Response::getIsPostUpload(void) const
{
	return _is_post_upload;
}

std::string Response::getSessionId(void) const
{
	return _sessionid;
}




void Response::resetResponse()
{
	_status_code.clear();
	_status_message.clear();
	_headers.clear();
	_body.clear();
	_full_path.clear();
	_uri_attributes.clear();
	_redirection.clear();
	_content_type.clear();
	_http_version.clear();
	is_complete = false;
	is_being_written = false;
	_server = NULL;
	_request = NULL;
	_content_length = 0;
	_is_directory = false;
	_to_upload.clear();
	_buffer.clear();
	_cgi_buffer.clear();
	_is_cgi = false;
}


void Response::prepareResponse()
{
	if (_status_code.empty())
		_status_code = "500";

	if (!_body.empty() && _body.size() > _server->getMaxBodySize())
		_status_code = "413";

	if (_request->getMethod() == "POST" && _is_post_upload == true && _status_code == "201")
	{
		_status_code = "303";
		if (_redirection.empty())
			_redirection = "/";
	}

	if (isAnErrorResponse(_status_code))
		defineResponseErrorPage();

	defineStatusMessage(_status_code);

	//build status line : status-line = HTTP-version SP status-code SP [ reason-phrase ]
	_buffer = "HTTP/1.1 " + _status_code + " " + _status_message + CRLF;

	if (_is_cgi == true && (_status_code == "200" || _status_code == "201"))
		_buffer += _cgi_buffer;
	else 
	{
		defineContentType();
		defineResponseHeaders();
		_buffer += _headers;
		if (_status_code != "301" && _status_code != "302" && _status_code != "303" && _body.empty() == false)
			_buffer += _body;
	}
}

void Response::defineResponseErrorPage()
{
	//if error page is defined, fetch it
	std::string error_page = _server->getErrorPage(std::stoi(_status_code));
	if (error_page.empty() == false)
	{
		//reconstitue full path
		if (_full_path.back() == '/' && error_page.front() == '/')
			error_page.erase(error_page.begin());
		else if (_full_path.back() != '/' && error_page.front() != '/')
			error_page = "/" + error_page;
		_full_path = _server->getRoot() + error_page;

		//if error opening the file, serve default error page
		std::ifstream file(_full_path.c_str(), std::ios::binary);
		if (file.is_open() == false)
		{
			_body = "<html><head><title>Error</title></head><body><h1>Error " + _status_code + "</h1></body></html>";
			return ;
		}
		//otherwise serve error page
		else
		{
			std::ostringstream oss;
			oss << file.rdbuf();
			_body = oss.str();
			file.close();
		}
	}
	//if error page undefined, serve default error page
	else 
		_body = "<html><head><title>Error</title></head><body><h1>Error " + _status_code + "</h1></body></html>";
}

void Response::defineContentType()
{
	//default content type
	_content_type = "text/html";

	//if path ends with an extension and not a directory we set the content type accordingly
	if (isDirectory(_full_path) == false && _full_path.find_last_of('.') != std::string::npos)
	{
		std::string extension = _full_path.substr(_full_path.find_last_of('.') + 1);
		if (extension == "html" || extension == "htm")
			_content_type = "text/html";
		else if (extension == "css")
			_content_type = "text/css";
		else if (extension == "js")
			_content_type = "text/javascript";
		else if (extension == "txt")
			_content_type = "text/plain";
		else if (extension == "xml")
			_content_type = "text/xml";
		else if (extension == "csv")
			_content_type = "text/csv";

		else if (extension == "jpg" || extension == "jpeg")
			_content_type = "image/jpeg";
		else if (extension == "png")
			_content_type = "image/png";
		else if (extension == "gif")
			_content_type = "image/gif";
		else if (extension == "bmp")
			_content_type = "image/bmp";
		else if (extension == "ico")
			_content_type = "image/x-icon";
		else if (extension == "svg")
			_content_type = "image/svg+xml";
			
		else if (extension == "mp3")
			_content_type = "audio/mpeg";
		else if (extension == "wav")
			_content_type = "audio/wav";
		else if (extension == "oga")
			_content_type = "audio/ogg";
		else if (extension == "spx")
			_content_type = "audio/ogg";
		else if (extension == "opus")
			_content_type = "audio/ogg";
		else if (extension == "mp4")
			_content_type = "video/mp4";
		else if (extension == "webm")
			_content_type = "video/webm";
		else if (extension == "ogg")
			_content_type = "video/ogg";
		else if (extension == "ogv")
			_content_type = "video/ogg";
		else if (extension == "webm")
			_content_type = "video/webm";

		else if (extension == "json")
			_content_type = "application/json";
		else if (extension == "pdf")
			_content_type = "application/pdf";
		else if (extension == "zip")
			_content_type = "application/zip";
		else if (extension == "tar")
			_content_type = "application/x-tar";
		else if (extension == "gz")
			_content_type = "application/gzip";
		else if (extension == "bz2")
			_content_type = "application/x-bzip2";
		else if (extension == "7z")
			_content_type = "application/x-7z-compressed";
		else if (extension == "rar")
			_content_type = "application/x-rar-compressed";
		else if (extension == "doc")
			_content_type = "application/msword";
		else if (extension == "docx")
			_content_type = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		else if (extension == "xls")
			_content_type = "application/vnd.ms-excel";
		else if (extension == "xlsx")
			_content_type = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		else if (extension == "ppt")
			_content_type = "application/vnd.ms-powerpoint";
		else if (extension == "pptx")
			_content_type = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
		else if (extension == "odt")
			_content_type = "application/vnd.oasis.opendocument.text";
		else if (extension == "ods")
			_content_type = "application/vnd.oasis.opendocument.spreadsheet";
		else if (extension == "odp")
			_content_type = "application/vnd.oasis.opendocument.presentation";
		else if (extension == "odg")
			_content_type = "application/vnd.oasis.opendocument.graphics";
		else if (extension == "odc")
			_content_type = "application/vnd.oasis.opendocument.chart";
		else if (extension == "odb")
			_content_type = "application/vnd.oasis.opendocument.database";
		else if (extension == "odf")
			_content_type = "application/vnd.oasis.opendocument.formula";
		else if (extension == "odm")
			_content_type = "application/vnd.oasis.opendocument.text-master";
		else if (extension == "ott")
			_content_type = "application/vnd.oasis.opendocument.text-template";
		else if (extension == "ots")
			_content_type = "application/vnd.oasis.opendocument.spreadsheet-template";
		else if (extension == "otp")
			_content_type = "application/vnd.oasis.opendocument.presentation-template";
		else if (extension == "otg")
			_content_type = "application/vnd.oasis.opendocument.graphics-template";
		else if (extension == "otc")
			_content_type = "application/vnd.oasis.opendocument.chart-template";
	}
}

void Response::defineStatusMessage(const std::string status_number)
{
	int status_code = 0;

	try {
		status_code = std::stoi(status_number);
	}
	catch (std::exception &e)
	{
		_status_message = "Unknown Status";
		return;
	}

	if (status_code == 200)
		_status_message = "OK";
	else if (status_code == 201)
		_status_message = "Created";
	else if (status_code == 202)
		_status_message = "Accepted";
	else if (status_code == 204)
		_status_message = "No Content";
	else if (status_code == 301)
		_status_message = "Moved Permanently";
	else if (status_code == 302)
		_status_message = "Found";
	else if (status_code == 303)
		_status_message = "See Other";
	else if (status_code == 304)
		_status_message = "Not Modified";
	else if (status_code == 400)
		_status_message = "Bad Request";
	else if (status_code == 401)
		_status_message = "Unauthorized";
	else if (status_code == 403)
		_status_message = "Forbidden";
	else if (status_code == 404)
		_status_message = "Not Found";
	else if (status_code == 405)
		_status_message = "Method Not Allowed";
	else if (status_code == 409)
		_status_message = "Conflict";
	else if (status_code == 413)
		_status_message = "Payload Too Large";
	else if (status_code == 500)
		_status_message = "Internal Server Error";
	else if (status_code == 501)
		_status_message = "Not Implemented";
	else if (status_code == 502)
		_status_message = "Bad Gateway";
	else if (status_code == 503)
		_status_message = "Service Unavailable";
	else if (status_code == 504)
		_status_message = "Gateway Timeout";
	else if (status_code == 505)
		_status_message = "HTTP Version Not Supported";
	else
		_status_message = "Unknown Status";
}

void Response::defineResponseHeaders()
{
	_headers += "Server: " + _server->getServerName() + CRLF;
	_headers += "Date: " + getCurrentDate() + CRLF;

	if (_status_code == "301" || _status_code == "302" || _status_code == "303")
	{
		_headers += "Location: " + _redirection + CRLF;
		_headers += std::string("Content-Length: 0") + CRLF;
	}
	else
	{
		_headers += "Content-Type: " + _content_type + CRLF;
		_headers += "Content-Length: " + std::to_string(_body.size()) + CRLF;
		//_headers += std::string("Connection: close") + CRLF;
	}
	if (_sessionid.empty() == false)
		_headers += "Set-Cookie: session=" + _sessionid + "; Path=/; HttpOnly" + CRLF;
	_headers += CRLF;
}

