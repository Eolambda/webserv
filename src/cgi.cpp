#include "cgi.hpp"

//All the helpers cgi functions


bool is_cgi(const Request &request)
{
	std::string uri = request.getUri();
	std::string path = extractPathFromURI(uri);
	std::string extension = path.substr(path.find_last_of('.') + 1);
	std::map<std::string, std::string> cgi_extensions = request.getClient()->getServer()->getCgiExtensions();
	
	if (isDirectory(request.getClient()->getResponse()->getFullPath()))
		return false;
	if (request.getMethod() != "POST" && request.getMethod() != "GET")
		return false;
	if (cgi_extensions.find(extension) != cgi_extensions.end())
		return true;
	return false;	
}

void handle_cgi(Client *client)
{
	Server *server = client->getServer();
	Request *request = client->getRequest();
	Response *response = client->getResponse();

	if (!server || !request || !response)
		return;

	std::string file_path = response->getFullPath();
	std::string extension = file_path.substr(file_path.find_last_of('.') + 1);
	std::string program_path = server->getCgiBin();

	if (program_path.back() != '/')
		program_path += "/";
	program_path += server->getCgiExtensions()[extension];

	std::vector<std::string> command(2);
	command[0] = program_path;
	command[1] = file_path;
	//only make it compatible with GET for now
	if (request->getMethod() == "GET")
		execute_cgi(command, generate_cgi_env(client, program_path, file_path), client);
}

void execute_cgi(std::vector<std::string> cmd, std::vector<std::string> env, Client *client)
{
	pid_t pid; 
	
	if (pipe(client->getCgiPipes()) < 0)
	{
		client->getResponse()->setStatusCode("500");
		client->getCgiPipes()[0] = -1;
		client->getCgiPipes()[1] = -1;
		return;
	}
	if ((pid = fork()) < 0)
	{
		client->getResponse()->setStatusCode("500");
		client->getCgiPipes()[0] = -1;
		client->getCgiPipes()[1] = -1;
		return;
	}
	if (pid == 0)
	{
		//close unused pipe and duplicate pipe to stdout
		close(client->getCgiPipes()[0]);
		if (dup2(client->getCgiPipes()[1], STDOUT_FILENO) < 0)
		{
			close(client->getCgiPipes()[1]);
			exit(EXIT_FAILURE);
		}

		//generate args and envp
		std::vector<char *> args;
		for (std::vector<std::string>::iterator it = cmd.begin(); it != cmd.end(); ++it)
			args.push_back(const_cast<char *>(it->c_str()));
		args.push_back(NULL);
		std::vector<char *> envp;
		for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it)
			envp.push_back(const_cast<char *>(it->c_str()));
		envp.push_back(NULL);

		//execve
		if (execve(args[0], args.data(), envp.data()) < 0)
		{
			delete[] args.data();
			delete[] envp.data();
			close (client->getCgiPipes()[1]);
			exit(EXIT_FAILURE);
		}

		//worst case
		delete[] args.data();
		delete[] envp.data();
		close (client->getCgiPipes()[1]);
		exit(EXIT_FAILURE);
	}
	else
		close(client->getCgiPipes()[1]);
}

std::vector<std::string> generate_cgi_env(Client *client, std::string command, std::string file)
{
	std::vector<std::string> env;
	std::string value;

	env.push_back("AUTH_TYPE");
	if (client->getRequest()->getMethod() == "POST")
		env.push_back("CONTENT_LENGTH=" + client->getRequest()->getHeaders()["Content-Length"]);
	else
		env.push_back("CONTENT_LENGTH=");

	if (client->getRequest()->getMethod() == "POST")
		env.push_back("CONTENT_TYPE=" + client->getRequest()->getHeaders()["Content-Type"]);
	else
		env.push_back("CONTENT_TYPE=");
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("PATH_INFO=");
	env.push_back("PATH_TRANSLATED=");
	env.push_back("QUERY_STRING="); //part of the URI that follows the ?
	value = "REMOTE_ADDR=";
	value += inet_ntoa(client->getAddr().sin_addr);
	env.push_back(value);
	env.push_back("REMOTE_PORT=" + std::to_string(ntohs(client->getAddr().sin_port)));
	//REMOTE_HOST
	//REMOTE_USER
	//REMOTE_IDENT
	env.push_back("REQUEST_METHOD=" + client->getRequest()->getMethod());
	env.push_back("REQUEST_URI=" + client->getRequest()->getUri());
	env.push_back("SCRIPT_NAME=" + command);
	env.push_back("SERVER_NAME=" + client->getServer()->getHostname());
	env.push_back("SERVER_PORT=" + std::to_string(client->getServer()->getPort()));
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("SERVER_SOFTWARE=Webserv v0.0005");
	env.push_back("REDIRECT_STATUS=200");
	env.push_back("SCRIPT_FILENAME=" + file);
	env.push_back("HTTP_ACCEPT=" + client->getRequest()->getHeaders()["Accept"]);
    env.push_back("HTTP_ACCEPT_LANGUAGE=" + client->getRequest()->getHeaders()["Accept-Language"]);
    env.push_back("HTTP_USER_AGENT=" + client->getRequest()->getHeaders()["User-Agent"]);
    env.push_back("HTTP_COOKIE=" + client->getRequest()->getHeaders()["Cookie"]);

	return env;
}