# Phase 4: Event Loop & Networking Layer - Line-by-Line Code Explanation

## Overview

Welcome to the comprehensive, line-by-line breakdown for Phase 4 of the **ft_irc** project. This document, maintained by **Ayoub**, covers the heart of the IRC server: the **Event Loop**, **Networking Layer**, and **I/O Multiplexing** using `poll()`. 

In this phase, we transition from a basic socket setup (Phase 2) and rudimentary client management (Phase 3) into a fully functional, non-blocking, asynchronous I/O server. We also integrate the Parser mechanism to process incoming IRC commands and implement robust signal handling for graceful shutdown.

> **Darija:** Had l-phase hiya l-moteur dyal s-serveur dyalna. F-blast ma nbqaw netsennaw client wa7ed yseft lina (blocking), ghadin nsta3mlo `poll()` bach nra9bo bezaf dyal les clients f-de9a we7da. Ila chi wahed sift, kanakhdo dakchi dyalo, w ila la, kandozo l-la5or. Hada howa l-concept dyal Asynchronous I/O.

---

## Table of Contents
1. [Deep Dive into Key Data Types](#deep-dive-into-key-data-types)
2. [C++98 Memory Ownership and Constraints](#c98-memory-ownership-and-constraints)
3. [Server.hpp Breakdown](#serverhpp-breakdown)
4. [Server.cpp Line-by-Line](#servercpp-line-by-line)
   - [Static Members](#static-members)
   - [Initialization: init()](#initialization-init)
   - [The Event Loop: run()](#the-event-loop-run)
   - [Accepting Clients: _acceptNewClient()](#accepting-clients-_acceptnewclient)
   - [Receiving Data: _receiveData()](#receiving-data-_receivedata)
   - [Processing Buffers: _processClientBuffer()](#processing-buffers-_processclientbuffer)
   - [Disconnections: _disconnectClient()](#disconnections-_disconnectclient)
   - [PollFD Management](#pollfd-management)
   - [Non-blocking Sockets](#non-blocking-sockets)
   - [Sending Data](#sending-data)
   - [Signal Handling](#signal-handling)
5. [Graceful Shutdown Sequence](#graceful-shutdown-sequence)

---

## 1. Deep Dive into Key Data Types

Before examining the code line-by-line, it is crucial to understand the foundational data types used heavily in network programming.

### `volatile sig_atomic_t`
- **Definition:** An integer type that guarantees atomic access, meaning it can be read or written in a single CPU instruction without being interrupted. The `volatile` keyword tells the compiler *not* to optimize this variable by caching it in a register, because its value can change at any time outside the normal program flow (e.g., from a signal handler).
- **Usage:** Used for the `_shutdown` flag. When the OS sends a `SIGINT` (Ctrl+C), our signal handler updates this variable. 
- **What if it was just `int`?** The compiler might assume the loop condition `while(!_shutdown)` never changes inside the loop, optimizing it into an infinite loop. Also, an ordinary `int` read/write might be split into multiple instructions on some architectures, leading to data corruption if interrupted halfway.
- **Darija:** `sig_atomic_t` b7al chi bouton li makaytsennach, kaybdel l-7ala dyalo f-remchat 3in (atomic). `volatile` kigoul l-compilateur "mat5bich had l-valeur f-r-ras, 9raha dima men l-memoire" 7it l-utilisateur y9der ybrek 3la Ctrl+C f-ay we9t.

### `struct pollfd`
- **Definition:** A structure used by the `poll()` system call to specify a file descriptor to monitor and the events we care about.
- **Fields:**
  - `fd`: The file descriptor (socket).
  - `events`: A bitmask of events we are waiting for (e.g., `POLLIN` for readable data).
  - `revents`: A bitmask of events that actually occurred, filled by the kernel after `poll()` returns.
- **Usage:** We keep a `std::vector<struct pollfd>` to track our server socket and all connected client sockets.

### `ssize_t` vs `size_t`
- **`size_t`:** Unsigned integer type used for sizes and counts. Cannot be negative.
- **`ssize_t`:** Signed integer type used for functions that can return a size or `-1` for an error (like `recv` or `send`).
- **Why it matters:** If you store a `recv` result in a `size_t`, checking `if (bytesRead == -1)` will be unreliable or implicitly cast to a massive positive number, causing out-of-bounds memory accesses.

### `socklen_t`
- **Definition:** An unsigned integer opaque type of length at least 32 bits.
- **Usage:** Used as a pointer parameter in `accept()`. It tells the kernel the size of the `sockaddr_in` struct we provided, and the kernel updates it with the actual size of the returned address.

---

## 2. C++98 Memory Ownership and Constraints

The ft_irc project must be written in C++98. This imposes strict architectural constraints.

### The Pointer vs Reference Dilemma
- **References:** Must be initialized upon creation and cannot be null. Great for passing parameters that are guaranteed to exist.
- **Pointers:** Can be null. Essential for dynamic allocation (`new`), arrays, and cases where an object might not exist (e.g., `Client* client = getClient(fd)`).
- **Our Model:** We map file descriptors (int) to `Client*` pointers in our `_clients` map. Since clients connect and disconnect dynamically, we dynamically allocate them and use pointers.

### Ownership Model (No Smart Pointers)
In C++11 and beyond, we would use `std::unique_ptr<Client>` to automatically delete the client memory when it is removed from the map. In C++98, we must manually manage memory:
- **Rule of Thumb:** Whoever creates the object with `new` is responsible for calling `delete`.
- In our server, the `Server` class owns the `Client` instances. When `_disconnectClient()` or `_clearClients()` is called, we must explicitly `delete` the pointer before removing it from the map. If we fail, we create a memory leak.
- **Darija:** F-C++98 ma3ndnach dak "femme de ménage" (Garbage Collector awla Smart Pointers) li kijme3 mora l-code. Ila 5la9ti chi 7aja b `new`, nta li 5assek dir liha `delete` mnin tsali biha, wila la l-memoire ghad t3mer b-zbel (memory leak) w s-serveur ghad ytboucla awla yti7.

---

## 3. Server.hpp Breakdown

Let's look at the class declaration changes for Phase 4.

### The Headers
```cpp
# include <string>
# include <iostream>
# include <cstring>
# include <cerrno>
# include <cstdlib>
# include <csignal>
# include <stdexcept>
# include <map>
# include "Parser.hpp"
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>
```
**Explanation:** 
- We include standard streams and strings for basic functionality.
- `<cstring>` and `<cerrno>`: For `strerror(errno)` and `memset`.
- `<csignal>`: Essential for `SIGINT` and `SIGPIPE` handling.
- `<vector>` and `<map>`: STL containers. `vector` is used for our `pollfd` list because `poll()` expects a contiguous array of structs in memory, and `vector` guarantees contiguous storage in C++98.
- The POSIX headers (`sys/socket.h`, `poll.h`, `fcntl.h`) provide the networking APIs.

### Class Declaration: Phase 4 Specifics
```cpp
		/* --- Send data to a client (Phase 4 — Ayoub) --- */
		void	sendToClient(int fd, const std::string& message);
```
- **What it does:** Public API to send a string to a specific client.
- **Why it's public:** Other components (like commands processing) need to send replies.

```cpp
		/* --- Private Helpers (Phase 4: Event Loop — Ayoub) --- */
		void	_acceptNewClient();
		void	_receiveData(int fd);
		void	_processClientBuffer(int fd);
		void	_sendData(int fd, const std::string& data);
		void	_disconnectClient(int fd);
		void	_addPollFd(int fd);
		void	_removePollFd(int fd);
		void	_setSocketNonBlocking(int fd);
```
- **Explanation:** These are private internal routines. By keeping them private, we encapsulate the complex socket state management away from the outside world.
- **Darija:** Hado homa les fonctions l-dakhliyin (private). Makhasch ay class o5ra t3ye6 3lihom direct, l-Serveur howa li kaynaddem l-5edma binathom f-l-kawaliss (behind the scenes).

```cpp
		/* --- Signal Handling (Phase 4 — Ayoub) --- */
		static void					_signalHandler(int signum);
		void						_setupSignals();
		static volatile sig_atomic_t	_shutdown;
```
- **Explanation:** The signal handler *must* be static because the OS signal API (`sigaction`) expects a raw function pointer of type `void (*)(int)`. Member functions in C++ have a hidden `this` pointer as their first argument, meaning their signature doesn't match what the OS expects. Making it `static` removes the `this` pointer. Since the handler is static, it can only access static member variables, which is why `_shutdown` is also static.

```cpp
		/* --- Attributes --- */
		std::vector<struct pollfd>	_pollfds;	/* Phase 4: pollfd collection */
	Parser						_parser;	/* Phase 4: IRC line parser   */
```
- **`_pollfds`:** We use `std::vector` instead of a static array or `std::list` because we can pass `&_pollfds[0]` to the `poll()` function, which expects an array pointer.
- **`_parser`:** Our integrated IRC message parser.

---

## 4. Server.cpp Line-by-Line

Now for the heavy lifting: analyzing the C++ implementation line by line.

### Static Members

```cpp
volatile sig_atomic_t Server::_shutdown = 0;
```
- **What it does:** Initializes the static shutdown flag to 0 (false). 
- **Why it's written this way:** Static members must be defined outside the class definition in C++98 to allocate storage for them. If missing, the linker will throw an "undefined reference" error.
- **Darija:** L-mot-clé `static` f-C++ kay3ni bli had l-variable kayna mra we7da fles instances kamlin dyal `Server`. Khasna n3tiwha blassa f-memoire 3la berra l-class.

### Initialization: init()

```cpp
void	Server::init()
{
	std::cout << "Initializing server..." << std::endl;
```
- **Line 1-3:** Standard console output.

```cpp
	_createSocket();
	_configureSocket();
	_setNonBlocking();
	_bindSocket();
	_startListening();
```
- **Lines 4-8:** These encapsulate Phase 2 logic (creating, binding, and listening).

```cpp
	_addPollFd(_serverFd);
```
- **What it does:** Adds the main server listening socket to our `poll()` watchlist.
- **Why:** We need to know when a new client connects. The kernel signals this by making the server socket "readable" (triggering `POLLIN`).

```cpp
	_setupSignals();
	std::cout << "Server ready." << std::endl;
}
```
- **Lines 10-12:** Configures the OS to intercept Ctrl+C and SIGPIPE, making the server production-ready.

### The Event Loop: run()

This is the most critical function in the entire project.

```cpp
void	Server::run()
{
	std::cout << "Server event loop started. (press Ctrl+C to stop)"
			  << std::endl;
```
- **Lines 1-3:** Informs the user how to terminate.

```cpp
	while (!_shutdown)
	{
```
- **Line 5:** The core infinite loop. It continues as long as `_shutdown` is 0.
- **What if missing?** The server would execute once and exit immediately.
- **Darija:** Hada howa l-9elb dyal s-serveur (event loop). Kayb9a ydour dima w ytsenna. Ila `_shutdown` wella 1 (b-sebab Ctrl+C), l-boucle ghat7bess.

```cpp
		int pollCount = poll(&_pollfds[0], _pollfds.size(), -1);
```
- **What it does:** Calls the `poll` system call.
- **Parameters:**
  1. `&_pollfds[0]`: A pointer to the first element in the vector's underlying contiguous array.
  2. `_pollfds.size()`: The number of descriptors to monitor.
  3. `-1`: The timeout in milliseconds. `-1` means "block indefinitely until an event occurs".
- **Why not `0` timeout?** A `0` timeout would create a busy-wait loop, consuming 100% CPU. `-1` puts the process to sleep, saving CPU cycles until network activity happens.

```cpp
		if (pollCount == -1)
		{
			if (errno == EINTR)
			{
				if (_shutdown)
					break ;
				continue ;
			}
			throw std::runtime_error(
				std::string("poll() failed: ") + strerror(errno));
		}
```
- **What it does:** Error handling for `poll()`.
- **`EINTR` check:** If `poll()` returns `-1` and `errno` is `EINTR` (Interrupted system call), it means a signal (like SIGINT) woke up the blocking `poll()` call.
- **Line 14:** We check `if (_shutdown)` because if the signal was SIGINT, our signal handler set `_shutdown = 1`. If so, we `break` out of the infinite loop gracefully.
- **Line 16:** If it was some other harmless signal, we just `continue` and restart `poll()`.
- **Line 18:** Any other error means something catastrophically failed in the OS, so we throw an exception.
- **Darija:** L-kernel y9der yfeye9 `poll` 9bel l-wa9t ila ja chi signal (b7al mnin katwrek 3la Ctrl+C). F-had l-7ala `errno` kikon `EINTR`. Kanchoufo ila `_shutdown` tbedlat, kankherjo men l-boucle, wila la kankemlo b `continue`.

```cpp
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
```
- **What it does:** Iterates through every file descriptor in our list to see which one triggered the wake-up.

```cpp
			if (_pollfds[i].revents == 0)
				continue ;
```
- **What it does:** Optimization. If `revents` (returned events) is 0, this socket had no activity, so we skip to the next iteration.

```cpp
			if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
```
- **What it does:** Checks for error flags.
  - `POLLERR`: Asynchronous error on the socket.
  - `POLLHUP`: Hang up (the other end closed the connection unexpectedly).
  - `POLLNVAL`: Invalid file descriptor (e.g., the socket was closed elsewhere).
- **Bitwise AND (`&`):** We use a bitwise AND because `revents` is a bitmask. For example, if `revents` is `POLLIN | POLLHUP`, this condition will correctly catch the `POLLHUP`.

```cpp
				if (_pollfds[i].fd == _serverFd)
				{
					throw std::runtime_error(
						"poll() reported error on server socket");
				}
```
- **What it does:** If the fatal error happened on the main listening socket, the server is essentially dead (e.g., port conflict, hardware failure). We have no choice but to crash/throw.

```cpp
				_disconnectClient(_pollfds[i].fd);
				--i;
				continue ;
			}
```
- **What it does:** If an error happened on a client socket, we disconnect the client.
- **The `--i` trick:** `_disconnectClient` modifies `_pollfds` by removing the client's `pollfd` from the vector. This shifts all subsequent elements left by one. If we don't do `--i`, the `for` loop's `++i` will cause us to skip evaluating the element that shifted into the current index!
- **Darija:** Mnin kansa7bo chi client men `vector`, ga3 les elements li morah kayrje3o b-blaça lor. Bach manzglou hta wahed, kankhedmo b `--i` bach l-boucle t3awd t9leb nfs l-index li 3mer b-client jdid.

```cpp
			if (_pollfds[i].revents & POLLIN)
			{
```
- **What it does:** Checks if there is data to read (`POLLIN`).

```cpp
				if (_pollfds[i].fd == _serverFd)
				{
					_acceptNewClient();
				}
```
- **What it does:** If the socket with data is our main `_serverFd`, `POLLIN` means a new client is trying to connect. We call `_acceptNewClient()`.

```cpp
				else
				{
					int currentFd = _pollfds[i].fd;
					size_t oldSize = _pollfds.size();
					_receiveData(currentFd);
```
- **What it does:** If the socket is a client socket, we read their sent data. We save the current `fd` and the `vector` size before reading.

```cpp
					if (_pollfds.size() < oldSize)
						--i;
				}
			}
		}
	}
```
- **What it does:** If `_receiveData` caused a disconnection (e.g., the client sent EOF or errored out), it will remove an element from `_pollfds`. Just like with the error blocks above, we must decrement `i` to avoid skipping elements due to the vector shift.
- **Why this logic:** Modifying a vector while iterating over it is dangerous in C++. We avoid iterators here and use indexes precisely so we can manipulate the index `i` safely when elements are deleted.

*(... End of the infinite loop ...)*

```cpp
	std::cout << "\nServer shutting down gracefully." << std::endl;
```
- **What it does:** Code reaches here only when `_shutdown == 1` (after a SIGINT).

```cpp
	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		std::cout << "Closing client socket (fd " << it->first << ")."
				  << std::endl;
		close(it->first);
	}
```
- **What it does:** Iterates over all connected clients and cleanly closes their socket file descriptors.
- **C++98 Note:** We must declare the iterator type fully `std::map<int, Client*>::iterator`. No `auto` keyword is available in C++98.

```cpp
	_clearClients();
	_pollfds.clear();

	if (_serverFd != -1)
	{
		std::cout << "Closing server socket (fd " << _serverFd << ")."
				  << std::endl;
		close(_serverFd);
		_serverFd = -1;
	}

	std::cout << "Server shutdown complete." << std::endl;
}
```
- **What it does:** Deletes all dynamically allocated `Client` objects via `_clearClients()`, empties the `pollfd` vector, closes the main server socket, and prints a final message.
- **Ownership:** This guarantees 0 memory leaks upon exit.

---

### Accepting Clients: _acceptNewClient()

```cpp
void	Server::_acceptNewClient()
{
	while (true)
	{
```
- **Why a `while(true)` loop?** Because the server socket is non-blocking. When `poll()` says the server socket is readable, there might be multiple clients trying to connect at the exact same millisecond. We must loop and `accept()` until it returns an error indicating no more clients are pending (`EWOULDBLOCK`).

```cpp
		struct sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);
		std::memset(&clientAddr, 0, sizeof(clientAddr));
```
- **What it does:** Prepares the structures to hold the new client's IP address and port data. We zero it out with `memset` for safety.

```cpp
		int clientFd = accept(_serverFd,
			(struct sockaddr *)&clientAddr, &addrLen);
```
- **What it does:** System call that extracts the first connection request on the queue of pending connections, creates a new connected socket, and returns a new file descriptor referring to that socket.
- **Cast:** `(struct sockaddr *)` is needed because `accept` takes a generic socket address structure, but we are using the IPv4 specific `sockaddr_in`.

```cpp
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
```
- **What it does:** If `accept` returns -1, it failed. 
- **`EAGAIN / EWOULDBLOCK`:** In a non-blocking socket, this specific error just means "There are no more connections left to accept right now." We break the loop and return to `poll()`.
- **Darija:** L-erreur `EWOULDBLOCK` hna machi mochkil. Kat3ni gha "walo, ma b9a ta client kaytsenna f-ssaf, rje3 l-poll chouf chghol a5or".

```cpp
			if (errno == EINTR)
				continue ;
```
- **What it does:** Standard interrupt check. Restart the accept call if interrupted by a harmless signal.

```cpp
			std::cerr << "accept() failed: " << strerror(errno) << std::endl;
			break ;
		}
```
- **What it does:** Any other error is a real issue. We log it and break.

```cpp
		std::string clientIp(inet_ntoa(clientAddr.sin_addr));
```
- **What it does:** Converts the raw binary IP address inside `clientAddr` into a readable dotted-decimal string (e.g., "192.168.1.5"). `inet_ntoa` means "Network to ASCII".

```cpp
		_setSocketNonBlocking(clientFd);
```
- **What it does:** CRITICAL! Newly accepted sockets inherit the blocking behavior of standard sockets by default on many OSes (though sometimes they inherit the parent's flags, it is safer to be explicit). We must set the client socket to non-blocking so reading from it doesn't freeze the server.

```cpp
		addClient(clientFd, clientIp);
		_addPollFd(clientFd);

		std::cout << "New connection from " << clientIp
				  << " (fd " << clientFd << ")." << std::endl;
	}
}
```
- **What it does:** Invokes the Phase 3 method `addClient` (which `new`s a Client and adds it to the map) and adds the new `fd` to the `_pollfds` vector to be monitored.

---

### Receiving Data: _receiveData()

```cpp
void	Server::_receiveData(int fd)
{
	char buf[1024];
```
- **What it does:** Allocates a 1KB static buffer on the stack to hold incoming data. This is extremely fast and avoids dynamic allocation overhead.

```cpp
	ssize_t bytesRead = recv(fd, buf, sizeof(buf), 0);
```
- **What it does:** Attempts to read up to 1024 bytes from the socket.
- **Why `recv` instead of `read`?** `recv` is specific to sockets and allows flags (the `0` at the end).

```cpp
	if (bytesRead == 0)
	{
		std::cout << "Client disconnected (fd " << fd << ")." << std::endl;
		_disconnectClient(fd);
		return ;
	}
```
- **What it does:** In network programming, a return value of `0` from `recv` means exactly one thing: **Graceful Disconnection (EOF)**. The client intentionally closed the connection. We must clean up.
- **Darija:** Mnin `recv` katrje3 lik 0, rah l-client gha gta3 l-kheyt w mcha f7alo b-chkel nadi. Khasna nmse7oh men s-serveur dyalna.

```cpp
	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		if (errno == EINTR)
			return ;
		std::cerr << "recv() error on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		_disconnectClient(fd);
		return ;
	}
```
- **What it does:** Error handling. 
  - `EAGAIN`/`EWOULDBLOCK`: The socket is non-blocking but has no data to read right now. This can happen due to OS timing. We just return.
  - Real errors: Log the error and violently disconnect the client.

```cpp
	Client* client = getClient(fd);
	if (client)
	{
		std::string rawData(buf, bytesRead);
		client->getBuffer().append_data(rawData);
		_processClientBuffer(fd);
	}
}
```
- **What it does:** If data was read successfully, we fetch the `Client` object, convert the raw char array into a `std::string`, and append it to the client's personal `ClientBuffer`. Then we tell the server to process that buffer.
- **Why convert to `std::string`?** `buf` is NOT null-terminated! Using `std::string(buf, bytesRead)` is the safe C++ way to handle binary-safe raw data.

---

### Processing Buffers: _processClientBuffer()

This connects the raw networking layer to the Parser.

```cpp
void	Server::_processClientBuffer(int fd)
{
	Client* client = getClient(fd);
	if (!client)
		return ;
```
- **Safety check:** Ensures the client hasn't been deleted.

```cpp
	std::vector<std::string> lines = client->getBuffer().extract_complete_lines();
```
- **What it does:** The IRC protocol is line-based, ending in `\r\n`. A single `recv()` might read half a line, or 3.5 lines due to TCP fragmentation. The `ClientBuffer` class handles this by keeping the fragmented data until a full `\r\n` arrives. This function call returns all fully formed commands and leaves the remaining fragment in the buffer.

```cpp
	for (size_t j = 0; j < lines.size(); ++j)
	{
		std::cout << "[fd=" << fd << "] Raw: " << lines[j] << std::endl;

		ParsedCommand cmd = _parser.parse_line(lines[j]);
```
- **What it does:** Loops through each full line and feeds it to our `Parser`.

```cpp
		if (cmd.is_valid)
		{
			std::cout << "[fd=" << fd << "] " << cmd.command;
			for (size_t p = 0; p < cmd.params.size(); ++p)
				std::cout << " " << cmd.params[p];
			std::cout << std::endl;
		}
		else
		{
			std::cout << "[fd=" << fd << "] (invalid/empty line)"
					  << std::endl;
		}
	}
}
```
- **What it does:** If the parser determined the command matches IRC formatting rules, we log the command and its parameters. (In Phase 5, this is where we will route the command to handlers like `JOIN`, `PRIVMSG`, etc).
- **Darija:** Hna s-serveur kay9ra s-stoura li jaw m9adin (kaykmlo b `\r\n`). Kay3tiha l-Parser bach yfhmha. Ila kanet s7i7a, kay3ref chno hwa l-command (b7al NICK wla JOIN) w chno homa les paramètres dyalo.

---

### Disconnections: _disconnectClient()

```cpp
void	Server::_disconnectClient(int fd)
{
	close(fd);
	removeClient(fd);
	_removePollFd(fd);
}
```
- **What it does:** Centralized teardown for a client.
  1. `close(fd)`: Tells the OS to release the socket resources.
  2. `removeClient(fd)`: Deletes the `Client` object from memory and removes it from the map.
  3. `_removePollFd(fd)`: Removes it from the `poll()` watchlist.
- **Why centralized?** Because there are many ways a client can disconnect (EOF, error, manual kick). Having one function prevents bugs and memory leaks.

---

### PollFD Management

```cpp
void	Server::_addPollFd(int fd)
{
	struct pollfd pfd;
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd      = fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}
```
- **What it does:** Creates a new `pollfd` struct, asks to monitor `POLLIN` (read activity), and pushes it to our vector. `memset` is used to clear any garbage data on the stack.

```cpp
void	Server::_removePollFd(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds[i] = _pollfds[_pollfds.size() - 1];
			_pollfds.pop_back();
			return ;
		}
	}
}
```
- **What it does:** Finds the `fd` in the vector and removes it.
- **The Swap-and-Pop Optimization:** Instead of using `_pollfds.erase()`, which forces the vector to shift all subsequent elements to the left (an $O(N)$ operation), this code copies the *last* element in the vector over the element we want to delete, and then calls `pop_back()` to remove the last element. This is an $O(1)$ operation! Because the order of `pollfds` doesn't matter, this is a massive performance boost for a server with many clients.
- **Darija:** Blast ma nms7ou element mn l-wsset w nka7zo koulchi l-lissr (li kayakhod we9t f-CPUs), kanjibou akher element f-vector n7ttoh f-blast hadak li baghin nms7ou, w kangolo l-vector n9ess size dyalk. Hada hack dki dyal C++.

---

### Non-blocking Sockets

```cpp
void	Server::_setSocketNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		std::cerr << "fcntl(F_GETFL) failed on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		close(fd);
		return ;
	}
```
- **What it does:** Uses `fcntl` (File Control) to GET the current file status flags (`F_GETFL`). We must get the existing flags first because we want to *add* the non-blocking flag without overwriting existing flags.

```cpp
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl(F_SETFL) failed on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		close(fd);
		return ;
	}
}
```
- **What it does:** Uses `fcntl` to SET the flags (`F_SETFL`), passing in the old flags bitwise OR'd with `O_NONBLOCK`. This flips the non-blocking bit to 1.
- **Why?** Without this, `accept`, `recv`, and `send` will pause (block) the entire server if the network is slow, violating the project requirements.

---

### Sending Data

```cpp
void	Server::_sendData(int fd, const std::string& data)
{
	size_t totalSent = 0;
	size_t remaining = data.size();

	while (totalSent < data.size())
	{
```
- **What it does:** A robust sending loop. Network buffers can fill up. If you try to send 2000 bytes, the OS might only accept 500 bytes right now. We must loop until all bytes are sent.

```cpp
		ssize_t bytesSent = send(fd, data.c_str() + totalSent, remaining, 0);
```
- **Pointer math:** `data.c_str() + totalSent` moves the starting pointer forward by the number of bytes we have already successfully sent.

```cpp
		if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cerr << "send() would block on fd " << fd
						  << " — dropping remaining bytes." << std::endl;
				return ;
			}
```
- **Partial sends:** In a truly robust server, if `send()` returns `EWOULDBLOCK` (meaning the OS buffer is full), we should queue the remaining data in a client output buffer and wait for `POLLOUT` from `poll()`. However, for the ft_irc requirements, dropping the remaining bytes on a saturated socket is generally acceptable to simplify architecture, as IRC messages are small.
- **Darija:** Ila bghina nsifto l-client w l-buffer dyal l-carte reseau 3mer, `send` ghatgoul lina `EWOULDBLOCK`. Hna kankhtarw nsme7o f-dakchi li b9a (dropping) bash s-serveur ma ytbouclach, 7it l-msagat dyal IRC sghar w had l-7ala nadrin w9ou3ha.

```cpp
			if (errno == EINTR)
				continue ;
			std::cerr << "send() error on fd " << fd << ": "
					  << strerror(errno) << std::endl;
			return ;
		}
		totalSent += bytesSent;
		remaining -= bytesSent;
	}
}
```
- **What it does:** Updates the counters based on how much the OS actually accepted.

```cpp
void	Server::sendToClient(int fd, const std::string& message)
{
	std::string fullMsg = message + "\r\n";
	_sendData(fd, fullMsg);
}
```
- **What it does:** Appends the mandatory IRC `\r\n` carriage-return line-feed terminator to the message before passing it to the raw send function.

---

### Signal Handling

```cpp
void	Server::_signalHandler(int signum)
{
	(void)signum;
	_shutdown = 1;
}
```
- **What it does:** This is the function the OS calls when a signal arrives. It ignores the `signum` (since we only route specific signals here) and flips the `_shutdown` flag.
- **Safety Rule:** Signal handlers should do almost *nothing* except set a flag. Calling complex functions like `std::cout` or `malloc` inside a signal handler is dangerous because they are not "async-signal-safe" (they could deadlock if the program was interrupted in the middle of a `malloc`).

```cpp
void	Server::_setupSignals()
{
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
```
- **What it does:** Prepares the `sigaction` struct. `sigemptyset` ensures no other signals are blocked while our handler runs.

```cpp
	if (sigaction(SIGINT, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGINT) failed");
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGTERM) failed");
	if (sigaction(SIGPIPE, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGPIPE) failed");
}
```
- **SIGINT:** Catches Ctrl+C.
- **SIGTERM:** Catches standard termination requests (e.g., from `kill` or Docker).
- **SIGPIPE:** CRITICAL. If a client disconnects violently, and we try to `send()` to them before the OS realizes they are gone, the OS sends a `SIGPIPE` signal to our program. The default behavior of `SIGPIPE` is to instantly crash the program. By registering it here with a custom handler, we intercept it, prevent the crash, and let `send()` return -1 with an error instead.
- **Darija:** `SIGPIPE` hiya akbar 3adou dyal network programming. Ila sifti data l-socket t9te3, systeme d'exploitation kay9tlek f-blasa b-`SIGPIPE`. Hna kangolo lih "safi gha 5lini n-gérer hadchi b-idiy", bach s-serveur ma yti7ch.

---

## 5. Graceful Shutdown Sequence

Let's trace exactly what happens when you press `Ctrl+C`.

1. **User Action:** User presses `Ctrl+C` in the terminal.
2. **OS Action:** The Linux Kernel sends a `SIGINT` signal to our process.
3. **Interrupt:** The server is likely blocked inside the `poll()` system call. The kernel interrupts `poll()` and executes our `_signalHandler`.
4. **Flag Set:** `_signalHandler` sets `_shutdown = 1` and returns.
5. **Poll Wakeup:** `poll()` returns `-1` and sets `errno` to `EINTR` (Interrupted system call).
6. **Error Check:** The loop checks `if (errno == EINTR)`. It then checks `if (_shutdown)`. Since it is 1, it executes `break ;`.
7. **Loop Exit:** The main event loop terminates.
8. **Cleanup Execution:** The server proceeds to the cleanup block.
   - It loops over `_clients` and calls `close()` on every client socket.
   - It calls `_clearClients()` to `delete` all dynamically allocated `Client` memory.
   - It calls `_pollfds.clear()` to destroy the vector.
   - It calls `close(_serverFd)` to release the listening port.
9. **Exit:** The program ends cleanly. Zero memory leaks, zero open file descriptors left behind.

## Summary

This networking layer provides a scalable, non-blocking foundation. By heavily utilizing `poll()` and avoiding threading, we conform to the strict IRC RFC requirements while ensuring our C++98 codebase remains fast and memory-safe.
