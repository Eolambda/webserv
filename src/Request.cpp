#include <Request.hpp>

Request::Request()
{
	_is_complete = false;
	_is_valid = 0;
	_parsing_state = 0;
	_client = NULL;
}

Request::Request(Client *client)
{
	_is_complete = false;
	_is_valid = 0;
	_parsing_state = 0;
	_client = client;
	if (_client != NULL && _client->getServer() != NULL)
		_max_body_size = _client->getServer()->getMaxBodySize();
}

Request::Request(const Request &request)
{
	_buffer = request._buffer;
	_last_line = request._last_line;
	_method = request._method;
	_uri = request._uri;
	_http_version = request._http_version;
	_headers = request._headers;
	_body = request._body;
	_is_complete = request._is_complete;
	_is_valid = request._is_valid;
	_parsing_state = request._parsing_state;
	_full_path = request._full_path;
	_max_body_size = request._max_body_size;
}

Request::~Request()
{
}

Request &Request::operator=(const Request &copy)
{
	_buffer = copy._buffer;
	_last_line = copy._last_line;
	_method = copy._method;
	_uri = copy._uri;
	_http_version = copy._http_version;
	_headers = copy._headers;
	_body = copy._body;
	_is_complete = copy._is_complete;
	_is_valid = copy._is_valid;
	_parsing_state = copy._parsing_state;
	_full_path = copy._full_path;
	_max_body_size = copy._max_body_size;
	return *this;
}

//getters

const std::string &Request::getBuffer(void) const
{
	return _buffer;
}

const std::string &Request::getLastLine(void) const
{
	return _last_line;
}

const std::string &Request::getMethod(void) const
{
	return _method;
}

uint8_t Request::getMethodBit(void) const
{
	if (_method == "GET")
		return GET_BIT;
	else if (_method == "POST")
		return POST_BIT;
	else if (_method == "DELETE")
		return DELETE_BIT;
	return 0;
}

const std::string &Request::getUri(void) const
{
	return _uri;
}

const std::string &Request::getHttpVersion(void) const
{
	return _http_version;
}

std::map<std::string, std::string> &Request::getHeaders(void)
{
	return _headers;
}

std::string Request::getHeader(const std::string &header)
{
	if (_headers.find(header) != _headers.end())
		return _headers[header];
	return "";
}

const std::string &Request::getBody(void) const
{
	return _body;
}

const int &Request::getRequestValidity(void) const
{
	return _is_valid;
}

const int &Request::getMaxBodySize(void) const
{
	return _max_body_size;
}

const bool &Request::isComplete(void) const
{
	return _is_complete;
}

//setters


void Request::setMethod(const std::string &method)
{
	_method = method;
}

void Request::setUri(const std::string &uri)
{
	_uri = uri;
}

void Request::setHttpVersion(const std::string &http_version)
{
	_http_version = http_version;
}

void Request::setHeaders(const std::map<std::string, std::string> &headers)
{
	_headers = headers;
}

void Request::setBody(const std::string &body)
{
	_body = body;
}

void Request::setRequestValidity(int value, bool is_complete)
{
	_is_valid = value;
	_is_complete = is_complete;
}

void Request::setMaxBodySize(int max_body_size)
{
	_max_body_size = max_body_size;
}

void Request::resetRequest(void)
{
	_buffer.clear();
	_last_line.clear();
	_method.clear();
	_uri.clear();
	_http_version.clear();
	_headers.clear();
	_body.clear();
	_is_complete = false;
	_is_valid = 0;
	_parsing_state = 0;
	_full_path.clear();
	_max_body_size = 0;
}

//parse the first line of the request
//A request-line begins with a method token, followed by a single space (SP), the request-target, and another single space (SP), and ends with the protocol version.
bool Request::parseFirstLine(std::string line)
{
	size_t pos, pos2 = 0;

	//get the method
	pos = line.find(' ');
	if (pos == std::string::npos)
		return false;
	//check if the method is valid
	if (line.substr(0, pos) != "GET" && line.substr(0, pos) != "POST" && line.substr(0, pos) != "DELETE")
		return false;
	_method = line.substr(0, pos);


	//check there is exactly one space after the method
	if (line[pos + 1] == ' ')
		return false;

	//get the uri
	pos2 = line.find(' ', pos + 1);
	if (pos2 == std::string::npos)
		return false;
	_uri = line.substr(pos + 1, pos2 - pos - 1);

	//check there is exactly one space after the uri
	if (line[pos2 + 1] == ' ')
		return false;

	//check if http version is valid and exists
	if (line.substr(pos2 + 1) != "HTTP/1.1" && line.substr(pos2 + 1) != "HTTP/1.0")
		return false;
	_http_version = line.substr(pos2 + 1);

	return true;
}

//parse the header line of the request
/*
Each field line consists of a case-insensitive field name followed by a colon (":"),
optional leading whitespace, the field line value, and optional trailing whitespace.

   field-line   = field-name ":" OWS field-value OWS
*/
bool Request::parseHeaders(std::string data)
{
	size_t pos = 0;
	std::string key, value;

	//find the colon
	pos = data.find(':');
	if (pos == std::string::npos)
		return false;
	//get the key
	key = data.substr(0, pos);
	//get the value and trim the leading / trailing spaces
	value = data.substr(pos + 1);
	value = trim_spaces(value);
	if (value.empty())
		return false;
	//we can eventually manually check for allowed headers here
	//add the key value pair to the headers
	_headers[key] = value;
	return true;
}

bool Request::checkBodyContentLength(std::string data)
{
	//if content length is not present, error
	if (_headers.find("Content-Length") == _headers.end())
		return false;

	size_t content_length = std::stoi(_headers["Content-Length"]);

	if (content_length == 0 || content_length > static_cast<size_t>(_max_body_size))
		return false;
	if (_body.size() + data.size() > static_cast<size_t>(_max_body_size))
		return false;

	return true;
}

//true if body is complete, false otherwise
bool Request::parseBody(std::string data)
{
	//if content length is not present, error
	if (_headers.find("Content-Length") == _headers.end())
		return true;

	size_t content_length = std::stoi(_headers["Content-Length"]);

	_body += data;

	if (_body.size() >= content_length)
	{
		//trim the body so that it is exactly the size of content length
		_body = _body.substr(0, content_length);
		return true;
	}
	return false;
}


//Adds new read data to the existing request
void Request::readData(std::string data)
{
	//if buffer is empty and data is newline, do nothing
	if (_buffer.empty() && data == CRLF)
		return;
	//if buffer is empty and data is empty and request does not contain double CRLF, request is invalid
	if (_last_line.empty() && data.empty() && _buffer.find(DOUBLECRLF) == std::string::npos)
	{
		setRequestValidity(HTTP_ERROR_BAD_REQUEST, true);
		return;
	}

	//append remainder of the last line to the data
	data = _last_line + data;
	_last_line.clear();


	size_t pos = 0;
	//we read data line by line, separated by \r\n
	while ((pos = data.find(CRLF)) != std::string::npos)
	{
		//if the request buffer is empty, we parse the first line of the request
		if (_buffer.empty() && _parsing_state == REQUEST_FIRST_LINE)
		{
			if (pos != 0 && parseFirstLine(data.substr(0, pos)) == false)
			{
				setRequestValidity(HTTP_ERROR_BAD_REQUEST, true);
				return;
			}
			_parsing_state = REQUEST_HEADERS;
		}
		//we search for a double CRLF, either cropped or not, to check end of headers
		else if (_parsing_state == REQUEST_HEADERS &&
				(((pos == 0) && (_buffer.size() >= 2 && _buffer[_buffer.size() - 1] == '\n' && _buffer[_buffer.size() - 2] == '\r'))
				|| ((pos == 1) && (_buffer.size() >= 1 && _buffer[_buffer.size() - 1] == '\r'))))
		{
			_parsing_state = REQUEST_BODY;
			//if method is not post, we consider the request complete
			if (_method != "POST")
			{
				_buffer += data.substr(0, pos + 2);
				setRequestValidity(0, true);
				return;
			}
		}
		//otherwise we have a complete header line
		else if (_parsing_state == REQUEST_HEADERS)
		{
			if (parseHeaders(data.substr(0, pos)) == false)
			{
				setRequestValidity(HTTP_ERROR_BAD_REQUEST, true);
				return;
			}
		}
		//handle case were a binary CRLF is found in the body, we parse it
		else if (_parsing_state == REQUEST_BODY && _method == "POST")
		{
			if (checkBodyContentLength(data.substr(0, pos + 2)) == false)
			{
				setRequestValidity(413, true);
				return;
			}
			if (parseBody(data.substr(0, pos + 2)) == true)
			{
				_buffer += data.substr(0, pos + 2);
				setRequestValidity(0, true);
				return;
			}
		}
		//other cases are invalid (check if we need to handle more cases here)
		else
		{
			setRequestValidity(HTTP_ERROR_BAD_REQUEST, true);
			return;
		}


		//we add the line to the buffer
		_buffer += data.substr(0, pos + 2);
		//we trim the beginning of the line and continue the loop
		data = data.substr(pos + 2);
	}
	
	//here we have remainder of data without newline, so if we are in parsing state 2 and method is POST, we add the data to the body
	//and we check that body is complete by comparing with content length
	if (_parsing_state == REQUEST_BODY && _method == "POST")
	{
		if (checkBodyContentLength(data.substr(0, pos + 2)) == false)
		{
			setRequestValidity(413, true);
			return;
		}
		if (parseBody(data) == true)
		{
			_buffer += data;
			setRequestValidity(0, true);
			return;
		}
	}

	//if message is just a CRLF, check if we store or not

	//request is not complete
	//we store the remainder of the data in the last line
	_last_line = data;
	return ;
}