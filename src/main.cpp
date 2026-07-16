/* ************************************************************************** */
/*                                                                            */
/*   main.cpp                                                                 */
/*                                                                            */
/*   Phase 1: Project Skeleton                                                */
/*   Entry point for the ircserv program                                      */
/*                                                                            */
/*   Usage: ./ircserv <port> <password>                                       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <cstdlib>

/**
 * @brief Validates that a string contains only digit characters.
 * @param str The string to validate.
 * @return true if every character is a digit, false otherwise.
 */
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
	/* --- Step 1: Check argument count --- */
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	std::string portStr(argv[1]);
	std::string password(argv[2]);

	/* --- Step 2: Validate port contains only digits --- */
	if (!isAllDigits(portStr))
	{
		std::cerr << "Error: Port must contain only digits." << std::endl;
		return 1;
	}

	/* --- Step 3: Validate port range (1024 - 65535) --- */
	int port = std::atoi(portStr.c_str());
	if (port < 1024 || port > 65535)
	{
		std::cerr << "Error: Port must be between 1024 and 65535." << std::endl;
		return 1;
	}

	/* --- Step 4: Validate password is not empty --- */
	if (password.empty())
	{
		std::cerr << "Error: Password cannot be empty." << std::endl;
		return 1;
	}

	/* --- Step 5: Create Server, initialize, and run --- */
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
