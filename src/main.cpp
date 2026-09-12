#include "Server.hpp"
#include <iostream>
#include <cstdlib>

static bool	isAllDigits(const std::string& str)
{
	if (str.empty())
		return false;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] < '0' || str[i] > '9')
			return false;
	}
	return true;
}

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	std::string portStr(argv[1]);
	std::string password(argv[2]);

	if (!isAllDigits(portStr))
	{
		std::cerr << "Error: Port must contain only digits." << std::endl;
		return 1;
	}

	int port = std::atoi(portStr.c_str());
	if (port < 1024 || port > 65535)
	{
		std::cerr << "Error: Port must be between 1024 and 65535." << std::endl;
		return 1;
	}

	if (password.empty())
	{
		std::cerr << "Error: Password cannot be empty." << std::endl;
		return 1;
	}

	try
	{
		Server server(port, password);
		server.init();
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
