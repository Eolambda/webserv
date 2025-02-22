#include <Response.hpp>

void Response::handleGET(Client *client)
{
	if (_is_directory == true)
	{
		_body = generateDirectorylisting(_full_path);
		_status_code = "200";
	}
	else if (_is_cgi == true)
		handle_cgi(client);
	else
	{
		std::ifstream file(_full_path.c_str(), std::ios::binary);
		if (file.is_open() == false)
			_status_code = "500";
		else
			_status_code = "200";

		//we only present the page if the status code is not an error
		if (!isAnErrorResponse(_status_code))
		{
			if (_is_directory == false)
			{
				std::ostringstream oss;
				oss << file.rdbuf();
				_body = oss.str();
				file.close();
			}
		}
	}
}

void Response::handleDELETE()
{
	if (remove(_full_path.c_str()) != 0)
		_status_code = "500";
	else
		_status_code = "204";
}

void Response::handlePOST(Client *client)
{
	std::string postBody;
	std::map<std::string, std::string> headers = _request->getHeaders();
	size_t content_length = static_cast<size_t>(_content_length);

	//check if header content-length is present
	if (content_length == 0)
	{
		_status_code = "411";
		return ;
	}
	//and if it is conform to the body size
	else if (content_length != _request->getBody().size())
	{
		_status_code = "400";
		return ;
	}
	else
		postBody = _request->getBody();


	std::string content_type = headers["Content-Type"];
	if (content_type.empty())
	{
		_status_code = "400";
		return ;
	}

	if (_is_cgi == true)
	{
		client->getRequest()->setCGIsendBuffer(postBody);
		handle_cgi(client);
		return ;
	}

	//check Content-Type
	//if multipart/form-data, parse the body using the boundary webform
	//if application/x-www-form-urlencoded, parse the body using the & and =
	//for anything else, we don't handle it
	if (content_type.find("multipart/form-data") != std::string::npos)
		HandlePOST_multiform(postBody, content_type);
	else if (content_type.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		std::map<std::string, std::string> params = parsePOSTBodyEncoded(postBody);
		if (params.empty())
		{
			_status_code = "400";
			return ;
		}
		else 
		//data is parsed, we can use it
			_status_code = "200";
	}
	//other cases, we don't handle other content types
	else
	{
		_status_code = "400";
		return ;
	}
}

//data is cut with boundary defined in the content-type header
//each part of the data is separated by the boundary, and represents a file
void Response::HandlePOST_multiform(std::string body, std::string content_type)
{
	std::map<std::string, std::string> files; //map to store the files address / content

	//check if boundary is present
	std::string boundary = content_type.substr(content_type.find("boundary=") + 9);
	if (boundary.empty())
	{
		_status_code = "400";
		return ;
	}

	//parse the body using the boundary
	std::vector<std::string> parts = parseMultipartFormData(body, boundary);

	//if no parts are found or more than one, we return an error
	//server only accepts one file at a time
	if (parts.empty() || parts.size() > 1)
	{
		_status_code = "400";
		return ;
	}

	// Loop through each part and extract the filename, content-type, and content
	for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it)
	{
		std::string filename, fileContentType, fileContent;
		std::istringstream stream(*it);
		std::string line;
		bool isFileContent = false;

		while (std::getline(stream, line))
		{
			// Remove carriage return at the end of the line (if present)
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);

			if (line.find("Content-Disposition:") != std::string::npos && isFileContent == false)
			{
				// Extract filename if present
				size_t filenamePos = line.find("filename=\"");
				if (filenamePos != std::string::npos)
				{
					filenamePos += 10; // Move past 'filename="'
					size_t endPos = line.find("\"", filenamePos);
					if (endPos != std::string::npos)
						filename = line.substr(filenamePos, endPos - filenamePos);
				}
			}
			else if (line.find("Content-Type:") != std::string::npos && isFileContent == false)
			{
				// Extract Content-Type
				fileContentType = line.substr(line.find(":") + 2);
			}
			else if (line.empty())
			{
				// Empty line indicates start of file content
				isFileContent = true;
			}
			else if (isFileContent)
			{
				if (line == "--")
					break;
				// Append actual file content
				fileContent += line + "\n";
			}
		}

		// Remove the trailing newline from fileContent
		if (!fileContent.empty() && fileContent[fileContent.size() - 1] == '\n')
			fileContent.erase(fileContent.size() - 1);

		if (!filename.empty())
		{
			std::string absolute_path = _server->getRoot();
			if (absolute_path.back() != '/' && _server->getUploads().front() != '/')
				absolute_path += "/";
			else if (absolute_path.back() == '/' && _server->getUploads().front() == '/')
				absolute_path.pop_back();
			absolute_path += _server->getUploads();
			if (checkPathExists(absolute_path + filename))
			{
				_status_code = "409";
				return ;
			}
			files[absolute_path + filename] = fileContent;
		}
	}


	//check if uri is matching the uploads parameter of the server
	std::string temp_uri_attributes = extractPathFromURI(_request->getUri());
	if (temp_uri_attributes.back() != '/' && _server->getUploads().back() == '/')
		temp_uri_attributes += "/";
	else if (temp_uri_attributes.back() == '/' && _server->getUploads().back() != '/')
		temp_uri_attributes.pop_back();

	if (temp_uri_attributes == _server->getUploads())
	{
		//loop through the map and upload the files
		for (std::map<std::string, std::string>::iterator it = files.begin(); it != files.end(); ++it)
		{
			if (uploadFile(it->first, it->second) == false)
			{
				_status_code = "500";
				return ;
			}
		}
		_is_post_upload = true;
	}

	_status_code = "201";
	return ;
}