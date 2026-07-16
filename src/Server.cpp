/* ************************************************************************** */
/*                                                                            */
/*   Server.cpp                                                               */
/*                                                                            */
/*   Phase 1: Project Skeleton                                                */
/*   Implementation of the Server class                                       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _serverFd(-1)
{
	std::cout << "Server object created. Port: " << _port
			  << ", Password: [PROTECTED]" << std::endl;
}

Server::~Server()
{
	std::cout << "Server object destroyed." << std::endl;
}

/* ************************************************************************** */
/*                              CORE METHODS                                  */
/* ************************************************************************** */

void	Server::init()
{
	std::cout << "Initializing server..." << std::endl;
	// Phase 2: socket(), bind(), listen() will be implemented here.
}

void	Server::run()
{
	std::cout << "Running server..." << std::endl;
	// Phase 2: poll() loop and event handling will be implemented here.
}
