/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:20:32 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/24 11:53:13 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <utils.hpp>

std::string to_string(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string toBinaryString(uint8_t value) {
    std::string result;
    // Loop from the most significant bit (7) to the least significant (0)
    for (int i = 7; i >= 0; --i) {
        result.push_back((value & (1 << i)) ? '1' : '0');
    }
    return result;
}

bool is_digits(const std::string& str)
{
    // Check if each character in the string is a digit
    for (size_t i = 0; i < str.length(); ++i) {
        if (!isdigit(str[i])) {
            return false; // Return false if any character is not a digit
        }
    }
    return true; // Return true if all characters are digits
}

bool checkExecutable(const std::string& path)
{
    struct stat info;
    
    // Check if the file exists
    if (stat(path.c_str(), &info) != 0)
        return false; // File does not exist

    // Check if it's a regular file or a script
    if (!S_ISREG(info.st_mode))
        return false; // Not a regular file

    // Check if the file is executable
    if (access(path.c_str(), X_OK) != 0)
        return false; // No execution rights

    return true; // File exists and is executable
}

double get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

std::string get_time_in_hours(double timestamp)
{
    // Convert the timestamp (double) to a time_t (which is in seconds)
    time_t raw_time = static_cast<time_t>(timestamp);
    
    // Use localtime to break down the timestamp into a tm structure
    struct tm *time_info = localtime(&raw_time);
    
    // Format the time in the form of day, hours, minutes, and seconds
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

    return std::string(buffer);
}

bool check_extension(const std::string &str)
{
	if (str.find_last_of('.') == std::string::npos)
		return (false);
	return (str.substr(str.find_last_of('.')) == ".conf");
}

std::string getCurrentDate()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return std::string(buffer);
}

bool checkPathExists(const std::string &path)
{
	std::ifstream file(path.c_str());
	return file.good();
}

bool isDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

std::string decodeURI(const std::string &uri)
{
	std::string decoded = "";
	size_t i = 0;
	
	while (i < uri.size())
	{
		if (uri[i] == '%')
		{
            // Convert the 2-character hex substring to an integer
            long value = std::strtol(uri.substr(i + 1, 2).c_str(), NULL, 16);
            decoded += static_cast<char>(value);
            i += 3;
		}
		else if (uri[i] == '+')
		{
			decoded += ' ';
			i++;
		}
		else
		{
			decoded += uri[i];
			i++;
		}
	}
	return decoded;
}

std::string extractPathFromURI(const std::string uri)
{	
	std::string path = uri;
	size_t pos = path.find("?");
	if (pos != std::string::npos)
		path = path.substr(0, pos);
	return decodeURI(path);
}

std::string extractAttributesFromURI(const std::string uri)
{
	std::string attributes = uri;
	size_t pos = attributes.find("?");
	if (pos != std::string::npos)
		attributes = attributes.substr(pos + 1);
	else
		attributes = "";
	return decodeURI(attributes);
}

//add a / at the end of the directory path
std::string addLastBacklash(std::string str)
{
    // Check if the string is empty or already doesn't end with a slash
    if (str.empty() || (!str.empty() && str[str.size() - 1] == '/'))
        return str;  // Return as is

    // Remove the last slash by creating a substring without the last character
    str.push_back('/');
	return (str);
}

//Use full path of directory to generate directory listing in html
std::string generateDirectorylisting(const std::string full_path, const std::string root_path)
{
	std::stringstream directory_listing;
	std::string relative_path = "/";
	size_t found = full_path.rfind(root_path);

	if (found != std::string::npos)
		relative_path = full_path.substr(found + root_path.length());

	directory_listing << "<html><head>";
	directory_listing << "<title>Directory listing</title>";
	directory_listing << "<style>";
	directory_listing << "body {font-family: Arial, sans-serif;}";
	directory_listing << "h1 {color: #333;}";
	directory_listing << "table {width: 100%; border-collapse: collapse;}";
	directory_listing << "</style>";
	directory_listing << "</head><body>";
	directory_listing << "<h1>Directory listing : </h1> <h3>" << relative_path << "</h3>";
	directory_listing << "<table>";

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(full_path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			directory_listing << "<tr>";
			directory_listing << "<td><a href=\"" << ent->d_name << "\">" << ent->d_name << "</a></td>";
			directory_listing << "</tr>";
		}
		closedir(dir);
	}
	else
	{
		directory_listing << "<tr><td>Could not open directory</td></tr>";
	}

	directory_listing << "</table>";
	directory_listing << "</body></html>";
	return directory_listing.str();	
}

//get pages that will return an error or not
bool isAnErrorResponse(const std::string error_number)
{
	int error_code = std::atoi(error_number.c_str());
	
	if (error_code == 0 
		|| (error_code >= 200 && error_code <= 204)
		|| (error_code >= 301 && error_code <= 303))
		//|| error_code == 401)
		return false;
	return true;
}

std::string &rtrim(std::string &s)
{
	s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
	return s;
}

std::string &ltrim(std::string &s)
{
	s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
	return s;
}

std::string trim_spaces(std::string &s)
{
	return ltrim(rtrim(s));
}

void shutdown_server(std::vector<Server> &servers)
{
	//erase all servers
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();)
		it = servers.erase(it);
}

std::vector<std::string> parseMultipartFormData(const std::string& body, const std::string& bound) 
{
	std::vector<std::string> 	parts;
	size_t 						start = 0;
	size_t 						end = 0;
	std::string 				boundary = "--" + bound;
	size_t 						boundaryLength = boundary.length();

				
	while ((start = body.find(boundary, start)) != std::string::npos) 
	{
		//if we find boundary + "--" we stop
		if (body.compare(start, boundaryLength + 2, boundary + "--") == 0)
			break;
		
		start += boundaryLength;
		end = body.find(boundary, start);
		if (end == std::string::npos) {
			end = body.length();
		}
		std::string toadd = body.substr(start, end - start);

		//trim the \r\n at the beginning and end of the part
		if (toadd.find("\r\n") == 0)
			toadd.erase(0, 2);
		if (toadd.rfind("\r\n") == toadd.length() - 2)
			toadd.erase(toadd.length() - 2, 2);
		
		parts.push_back(toadd);
	}
	
	return parts;
}

// URL-decodes a string. For example, "Hello%20World%21" becomes "Hello World!".
// Also converts '+' into space.
std::string postBodyDecode(const std::string &src)
{
    std::string ret;
    char 		ch;
    size_t 		i;
	
    for (i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < src.length() && std::isxdigit(src[i+1]) && std::isxdigit(src[i+2])) {
                std::istringstream iss(src.substr(i + 1, 2));
                unsigned int hex;
                iss >> std::hex >> hex;
                ch = static_cast<char>(hex);
                ret += ch;
                i += 2;
            } else {
                ret += '%'; // Invalid sequence, keep the '%'
            }
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

// Splits a URL-encoded query string (application/x-www-form-urlencoded)
// into key-value pairs and returns them as a std::map.
std::map<std::string, std::string> parsePOSTBodyEncoded(const std::string &body)
{
    std::map<std::string, std::string> 	params;
    std::string::size_type 				start = 0;
	
    while (start < body.length())
	{
        // Find the position of the next '&'
        std::string::size_type end = body.find('&', start);
        if (end == std::string::npos)
            end = body.length();

        std::string pair = body.substr(start, end - start);
        std::string::size_type eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string key = pair.substr(0, eqPos);
            std::string value = pair.substr(eqPos + 1);
            // Decode both key and value
            key = postBodyDecode(key);
            value = postBodyDecode(value);
            params[key] = value;
        } else {
            // No '=' found, treat the whole pair as key with empty value
            std::string key = postBodyDecode(pair);
            params[key] = "";
        }
        start = end + 1;
    }
	
    return params;
}

bool uploadFile(const std::string &filename, const std::string &content)
{
	std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
	if (!file.is_open())
		return false;
	file << content;
	file.close();
	return true;
}

std::string generateSessionId(void)
{
	std::string session_id = "";
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	
	for (int i = 0; i < 32; i++)
		session_id += charset[rand() % charset.length()];
	return session_id;
}

void clean_cookies(std::vector<Server> &servers)
{
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		std::map<std::string, struct SessionData> &session_store = it->getSessionStore();
		for (std::map<std::string, struct SessionData>::iterator it2 = session_store.begin(); it2 != session_store.end();)
		{
			if (it2->second.last_access + COOKIES_EXPIRY_TIME < time(NULL))
				session_store.erase(it2++);
			else
				++it2;
		}
	}
}