#include "cgi.hpp"

//All the helpers cgi functions


bool is_cgi(Request *request, Client *client)
{
	std::string uri = request->getUri();
	std::string path = extractPathFromURI(uri);
	std::string extension;
	if (path.find_last_of('.') != std::string::npos)
		extension = path.substr(path.find_last_of('.'));
	else
		return false;
	std::map<std::string, std::string> cgi_extensions = client->getServer()->getCgiExtensions();

	if (client->getResponse()->getIsDirectory() == true)
		return false;

	if (request->getMethod() != "POST" && request->getMethod() != "GET")
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
	std::string extension = file_path.substr(file_path.find_last_of('.'));
	std::string root = server->getRoot();
	std::string program_path = server->getCgiBin();

	//if root path ends with a / and program path starts with a /, we remove the / from root path
	//if root path does not end with a / and program path does not start with one neither, we add a / to root path
	if (root.back() == '/' && program_path.front() == '/')
		root.pop_back();
	else if (root.back() != '/' && program_path.front() != '/')
		root += "/";
	program_path = root + program_path;
	if (program_path.back() != '/')
		program_path += "/";
	program_path += server->getCgiExtensions()[extension];

	std::vector<std::string> command(2);
	command[0] = program_path;
	command[1] = file_path;
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
	if (client->getRequest()->getMethod() == "POST" && pipe(client->getCgiPipes_POST()) < 0)
	{
		close(client->getCgiPipes()[0]);
		close(client->getCgiPipes()[1]);
		client->getResponse()->setStatusCode("500");
		client->getCgiPipes()[0] = -1;
		client->getCgiPipes()[1] = -1;
		client->getCgiPipes_POST()[0] = -1;
		client->getCgiPipes_POST()[1] = -1;
		return;
	}
	if ((pid = fork()) < 0)
	{
		client->getResponse()->setStatusCode("500");
		client->getCgiPipes()[0] = -1;
		client->getCgiPipes()[1] = -1;
		if (client->getRequest()->getMethod() == "POST")
		{
			close(client->getCgiPipes_POST()[0]);
			close(client->getCgiPipes_POST()[1]);
			client->getCgiPipes_POST()[0] = -1;
			client->getCgiPipes_POST()[1] = -1;
		}
		return;
	}
	if (pid == 0)
	{
		close(client->getCgiPipes()[0]);
		close(client->getCgiPipes_POST()[1]);

		if (dup2(client->getCgiPipes()[1], STDOUT_FILENO) < 0
			|| (client->getRequest()->getMethod() == "POST" && dup2(client->getCgiPipes_POST()[0], STDIN_FILENO) < 0))
		{
			close(client->getCgiPipes()[1]);
			if (client->getRequest()->getMethod() == "POST")
				close(client->getCgiPipes_POST()[0]);
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
			if (client->getRequest()->getMethod() == "POST")
				close(client->getCgiPipes_POST()[0]);
			exit(EXIT_FAILURE);
		}

		//worst case
		delete[] args.data();
		delete[] envp.data();
		close (client->getCgiPipes()[1]);
		if (client->getRequest()->getMethod() == "POST")
			close(client->getCgiPipes_POST()[0]);
		exit(EXIT_FAILURE);
	}
	else
	{
		client->setCGITimer(get_time());
		close(client->getCgiPipes()[1]);
		if (client->getRequest()->getMethod() == "POST")
			close(client->getCgiPipes_POST()[0]);
	}
}

std::vector<std::string> generate_cgi_env(Client *client, std::string command, std::string file)
{
	std::vector<std::string> env;
	std::string value;

	env.push_back("AUTH_TYPE=");
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
	env.push_back("QUERY_STRING=" + client->getResponse()->getURIAttributes());
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

//when the request is a CGI and a POST, we send the body to the child process
void Server::sendCGI(Client &client)
{
	int ret;
	double time = get_time();

	if (time - client.getCGITimer() > CGI_TIMEOUT)
	{
		client.getResponse()->setStatusCode("500");
		close(client.getCgiPipes_POST()[1]);
		close(client.getCgiPipes()[0]);
		client.getCgiPipes_POST()[0] = -1;
		client.getCgiPipes_POST()[1] = -1;
		client.getCgiPipes()[0] = -1;
		client.getCgiPipes()[1] = -1;
		client.getResponse()->setCgiBuffer("");
		return;
	}
	
	size_t body_size = client.getRequest()->getCGIsendBuffer().size();
	if (body_size > 0)
	{
		if (body_size <= BUFFER_SIZE)
			ret = write(client.getCgiPipes_POST()[1], client.getRequest()->getCGIsendBuffer().c_str(), body_size);
		else
			ret = write(client.getCgiPipes_POST()[1], client.getRequest()->getCGIsendBuffer().c_str(), BUFFER_SIZE);
	}
	else
		ret = 0;

	if (ret < 0)
	{
		client.getResponse()->setStatusCode("500");
		close(client.getCgiPipes_POST()[1]);
		close(client.getCgiPipes()[0]);
		client.getCgiPipes_POST()[0] = -1;
		client.getCgiPipes_POST()[1] = -1;
		client.getCgiPipes()[0] = -1;
		client.getCgiPipes()[1] = -1;
		client.getResponse()->setCgiBuffer("");
		return;
	}
	else if (ret == 0 || body_size == static_cast<size_t>(ret))
	{
		close(client.getCgiPipes_POST()[1]);
		client.getCgiPipes_POST()[1] = -1;
		client.getCgiPipes_POST()[0] = -1;
	}
	else
		client.getRequest()->setCGIsendBuffer(client.getRequest()->getCGIsendBuffer().substr(ret));
}


//when the request is a CGI, we read the CGI output in order to build the response
void Server::receiveCGI(Client &client)
{
	int ret;
	char buffer[BUFFER_SIZE + 1];
	double time = get_time();

	if (time - client.getCGITimer() > CGI_TIMEOUT)
	{
		client.getResponse()->setStatusCode("500");
		close(client.getCgiPipes()[0]);
		client.getCgiPipes()[0] = -1;
		client.getCgiPipes()[1] = -1;
		client.getResponse()->setCgiBuffer("");
		return;
	}

	ret = read(client.getCgiPipes()[0], buffer, BUFFER_SIZE);
	if (ret < 0)
	{
		client.getResponse()->setStatusCode("500");
		close (client.getCgiPipes()[0]);
		client.getCgiPipes()[0] = -1;
		client.getCgiPipes()[1] = -1;
		
	}
	else if (ret == 0)
	{
		close(client.getCgiPipes()[0]);
		client.getCgiPipes()[0] = -1;
		client.getCgiPipes()[1] = -1;
		//fetch status code from cgi buffer
		//if there is the word "Status: " in the buffer, we fetch the status code
		std::string cgi_buffer = client.getResponse()->getCgiBuffer();
		size_t pos = cgi_buffer.find("Status: ");
		if (pos != std::string::npos)
		{
			std::string status_code = cgi_buffer.substr(pos + 8, 3);
			if (atoi(status_code.c_str()) <= 0)
				status_code = "200";
			client.getResponse()->setStatusCode(status_code);
			//empty the buffer if error
		}
		else
			client.getResponse()->setStatusCode("200");}
	else
	{
		buffer[ret] = '\0';
		client.getResponse()->setCgiBuffer(client.getResponse()->getCgiBuffer() + buffer);
	}
}