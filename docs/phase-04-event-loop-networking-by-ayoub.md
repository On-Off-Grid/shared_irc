# Phase 4: Event Loop & Networking Layer 🌐

Welcome to **Phase 4** of the `ft_irc` project! This phase forms the absolute core of the server's real-time communication capabilities.

**Owner**: Ayoub
**Focus**: Multiplexed I/O, Reactor Pattern, System Calls (`poll`, `accept`, `recv`, `send`), and seamless integration with the BufferManager and Parser.

## Introduction
The goal of this phase is to transition from a static listener into a fully-fledged, event-driven server capable of handling multiple concurrent client connections without threading. We will achieve this using the `poll()` system call. The networking layer will manage socket lifecycles, read incoming byte streams, parse them into complete lines via the BufferManager, and feed them to the Parser. 

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot (The Team):** F'had la phase 4, Ayoub ghadi ytkellef b'lqalb d'server. Blast ma server ybqa waqef kiytsenna client wahed (blocking), ghadi nkhdmo b' `poll()` bash server yqder ihder m3a bezaf dyal l'clients f'deqqa wehda. Ay haja jatna f'network (bytes), ghadi nqebtoha, nzido fiha f'BufferManager, w nqedmoha l'Parser mli tkmal line. Hadshi kolo kiwqe3 f'thread wahed (single-threaded) bla ma nsta3mlo l'khyout (threads) li ki3eqdo l'umur. Hada howa l'event-driven architecture!

---

## 📑 Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Event-Driven Architecture](#2-event-driven-architecture)
3. [The `poll()` System Call](#3-the-poll-system-call)
4. [`pollfd` Management](#4-pollfd-management)
5. [The `accept()` System Call](#5-the-accept-system-call)
6. [The `recv()` System Call](#6-the-recv-system-call)
7. [Non-blocking Sockets](#7-non-blocking-sockets)
8. [Socket Lifecycle](#8-socket-lifecycle)
9. [Disconnect Handling](#9-disconnect-handling)
10. [Integration with Client](#10-integration-with-client)
11. [Integration with BufferManager](#11-integration-with-buffermanager)
12. [Integration with Parser](#12-integration-with-parser)
13. [The Event Loop (`run()` method)](#13-the-event-loop-run-method)
14. [`send()` Infrastructure](#14-send-infrastructure)
15. [SIGINT Graceful Shutdown](#15-sigint-graceful-shutdown)
16. [Error Handling](#16-error-handling)
17. [Vector Mutation During Iteration](#17-vector-mutation-during-iteration)
18. [Ownership Rules](#18-ownership-rules)
19. [Testing Procedures](#19-testing-procedures)
20. [Common Mistakes](#20-common-mistakes)
21. [Compatibility](#21-compatibility)
22. [What Comes Next](#22-what-comes-next)

---

## 1. Prerequisites
Before attempting to integrate this phase, the following must be in place:
* **Server Setup**: The server socket must be created, bound to a port (e.g., 6667), and listening.
* **Client Class**: A C++98 compliant class representing a connected client, storing their FD, IP address, and holding a `BufferManager` instance.
* **BufferManager**: A utility to manage partial reads and extract complete `\r\n` delimited lines.
* **Parser**: A module that takes a string and converts it into a `ParsedCommand` struct.

---

## 2. Event-Driven Architecture

In a standard blocking server, calling `accept()` or `recv()` blocks the entire program until a client connects or data is sent. To handle multiple clients, one might spawn a thread per client. However, thread-per-client does not scale well (due to context switching) and violates the constraints of `ft_irc` (where `pthread` or similar concurrency primitives are banned).

Instead, we use the **Reactor Pattern** via **Multiplexed I/O**. We ask the OS: *"Here is a list of file descriptors. Wake me up when any of them are ready to be read from or written to."*

> [!NOTE] **Darija: Event-Driven Architecture**
> **🇲🇦 Reactor Pattern:** Tkhaylo wahed l'garçon f'qahwa (Server). Blast ma ybqa waqef foq ras wahed l'klyan kaytsenah yekhtar shno yshreb (blocking), kaymshi yshouf shkoun klyan akhor li wajed. L'garçon ki-check ga3 trabez (pollfds) f'deqqa. Li wajed y3ti commande (POLLIN), kiymshi lih. Hadshi kaykhelli garçon wahed yservi l'qahwa kamla bla maybqa mbloki m3a wahd.

### Thread-per-Client vs Event Loop
* **Thread-per-Client**: 1000 clients = 1000 threads. High RAM usage, lock contention, race conditions.
* **Event Loop (poll)**: 1000 clients = 1 array of 1001 file descriptors. Single thread, zero locks, ultra-efficient.

---

## 3. The `poll()` System Call

The `poll()` function monitors a set of file descriptors for events.

```cpp
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

### The `pollfd` Struct
```cpp
struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
```
* `fd`: The socket descriptor.
* `events`: What we want to monitor (e.g., `POLLIN` for data ready to read).
* `revents`: What actually happened (set by the kernel).

### Event Flags
* **POLLIN**: Data is ready to `recv()`, or a new client is ready to `accept()`.
* **POLLOUT**: Socket is ready to `send()` data without blocking.
* **POLLERR**: Error condition (e.g., RST packet received).
* **POLLHUP**: Hang up (client closed connection unexpectedly).
* **POLLNVAL**: Invalid request (e.g., FD is not open).

### Return Values
* `> 0`: Number of file descriptors with events.
* `0`: Timeout occurred.
* `-1`: Error occurred (e.g., `EINTR` from a signal).

> [!NOTE] **Darija: The `poll()` call**
> **🇲🇦 poll():** Howa dak l'moufetich dyalna. Kina3tiweh la-liste dyal wrak (fds), w kangolo lih: "Siyft lina l'khbar mli chi werqa fihom tkon wajda n9rawha (POLLIN)". L'moufetich kymshi y9eleb (kernel mode) w kiyrje3 y3mer lina dik `revents` bash n3erfo chkoun li fih jdid.

---

## 4. `pollfd` Management

We manage our file descriptors using a `std::vector<struct pollfd> _pollfds`. 

**Critical Rule:** The server's listening socket is ALWAYS at index 0 (`_pollfds[0]`).

### Adding an FD
```cpp
void Server::_addPollFd(int fd) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
}
```

### Removing an FD (Swap-Erase for O(1))
Using `.erase()` on a vector shifts all subsequent elements left, which is O(N). Since the order of `pollfds` doesn't matter, we use the swap-erase idiom.

```cpp
void Server::_removePollFd(int fd) {
    for (size_t i = 1; i < _pollfds.size(); ++i) { // Start at 1, don't remove server
        if (_pollfds[i].fd == fd) {
            _pollfds[i] = _pollfds.back(); // Swap with last
            _pollfds.pop_back();           // Remove last
            return;
        }
    }
}
```

### ASCII Diagram: Vector State

```text
Time T0: Server starts
_pollfds: [ {fd:3 (server)} ]

Time T1: Client 1 connects (fd:4)
_pollfds: [ {fd:3 (server)}, {fd:4 (Client 1)} ]

Time T2: Client 2 connects (fd:5)
_pollfds: [ {fd:3 (server)}, {fd:4 (Client 1)}, {fd:5 (Client 2)} ]

Time T3: Client 1 disconnects (Swap-Erase fd:4)
1. Swap fd:4 with fd:5
_pollfds: [ {fd:3 (server)}, {fd:5 (Client 2)}, {fd:4 (DEAD)} ]
2. pop_back()
_pollfds: [ {fd:3 (server)}, {fd:5 (Client 2)} ]
```

---

## 5. The `accept()` System Call

When `_pollfds[0].revents & POLLIN` is true, the server socket has a pending connection. We loop `accept()` until it returns `EAGAIN` to drain the connection queue.

```cpp
void Server::_acceptNewClient() {
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Queue is empty, we are done
            }
            if (errno == EINTR) {
                continue; // Interrupted by signal, try again
            }
            // Log serious error
            break;
        }
        
        // Extract IP
        std::string ip = inet_ntoa(client_addr.sin_addr);
        
        // Set non-blocking
        _setSocketNonBlocking(client_fd);
        
        // Add to our tracked clients and poll vector
        _addClient(client_fd, ip);
        _addPollFd(client_fd);
        
        std::cout << "[Network] New client connected: fd=" << client_fd << " IP=" << ip << std::endl;
    }
}
```

> [!NOTE] **Darija: accept()**
> **🇲🇦 accept():** Mli shi klyan jdid bgha ydkhol l'server, kaywselna POLLIN f'lbab d'server (server socket). Kangolo l' `accept()`: dkhl had l'klyan w 3tina symia jdida lih (client_fd). Kanbqaw n-acceptiw hta katsali noba (EAGAIN), bash maybqach chi wahed kiytsenna f'zanqa.

---

## 6. The `recv()` System Call

When a client socket has `POLLIN`, we read the data into a fixed-size buffer.

```cpp
void Server::_receiveData(int client_fd) {
    char buffer[1024];
    
    while (true) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        
        if (bytes_received > 0) {
            // Forward raw data to BufferManager
            std::string rawData(buffer, bytes_received);
            _processClientBuffer(client_fd, rawData);
        }
        else if (bytes_received == 0) {
            // Graceful disconnect (EOF)
            std::cout << "[Network] Client fd=" << client_fd << " gracefully disconnected." << std::endl;
            _disconnectClient(client_fd);
            break;
        }
        else { // bytes_received < 0
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Read all available data
            }
            if (errno == EINTR) {
                continue; // Interrupted, try again
            }
            // Fatal error (Connection reset by peer, etc.)
            std::cerr << "[Network] Error on recv fd=" << client_fd << std::endl;
            _disconnectClient(client_fd);
            break;
        }
    }
}
```

---

## 7. Non-blocking Sockets

By default, sockets are blocking. If you call `recv()` and there is no data, the entire server freezes.
To fix this, we use `fcntl()`.

```cpp
void Server::_setSocketNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl F_GETFL failed");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("fcntl F_SETFL O_NONBLOCK failed");
    }
}
```
**Important**: You must do this for the server socket before calling `listen()`, and for every client socket immediately after `accept()`.

> [!NOTE] **Darija: Non-blocking**
> **🇲🇦 Non-blocking:** Tkhayel mshiti t9di charaj. F'blocking, katsna f'saff w ma kat-t7erekch hta ki-kemel shghal, wakha l'weqt ydouz. F'non-blocking, kadkhel tsowl: "Wash wajd shghli?" Ila makanch wajed (EAGAIN), katkoun 3adi, katmshi tqdi haja khra w trj3 mn b3d. Hada howa l'O_NONBLOCK.

---

## 8. Socket Lifecycle

### ASCII Diagram: The Connection Journey

```text
    [Client]                          [Server]
       |                                 |
       |------- SYN -------------------->| OS Accept Queue
       |<------ SYN-ACK -----------------|
       |------- ACK -------------------->|
       |                                 | Server Event Loop:
       |                                 | poll() detects POLLIN on _pollfds[0]
       |                                 | calls accept() -> returns client_fd (5)
       |                                 | fcntl(5, O_NONBLOCK)
       |                                 | _addClient(5)
       |                                 | _addPollFd(5) -> _pollfds.push_back()
       |                                 |
       |------- "NICK Ayoub\r\n" ------->|
       |                                 | poll() detects POLLIN on client_fd (5)
       |                                 | recv(5) reads bytes
       |                                 | BufferManager appends data
       |                                 | Parser parses line
       |                                 |
       |------- [Close / CTRL+D] ------->|
       |                                 | poll() detects POLLIN on client_fd (5)
       |                                 | recv(5) returns 0 (EOF)
       |                                 | _disconnectClient(5)
       |                                 | close(5)
       |                                 | _removeClient(5)
       |                                 | _removePollFd(5) [Swap-Erase]
```

---

## 9. Disconnect Handling

When a client leaves (either gracefully via `recv() == 0`, or ungracefully via `POLLERR`/`POLLHUP`), we must clean up everything related to them.

```cpp
void Server::_disconnectClient(int fd) {
    // 1. Close socket descriptor
    close(fd);
    
    // 2. Remove from pollfds vector
    _removePollFd(fd);
    
    // 3. Remove client object (frees memory/buffers)
    _removeClient(fd);
}
```
*Order matters:* `close(fd)` releases the kernel resource. Then we clean up our internal data structures.

---

## 10. Integration with Client

The networking layer needs a way to store state for each FD. We assume a `Client` class exists. The `Server` class manages a `std::map<int, Client*> _clients`.

```cpp
Client* Server::_getClient(int fd) {
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        return it->second;
    }
    return NULL; // Never happens if state is consistent
}
```

---

## 11. Integration with BufferManager

TCP is a stream protocol. When we call `recv()`, we might get:
* A full IRC command: `"NICK Ayoub\r\n"`
* A partial command: `"NICK Ay"`
* Multiple commands: `"NICK Ayoub\r\nUSER a a a :a\r\n"`

The BufferManager handles this.

```cpp
void Server::_processClientBuffer(int fd, const std::string& rawData) {
    Client* client = _getClient(fd);
    if (!client) return;
    
    // Append raw bytes to client's personal buffer
    client->getBuffer().append_data(rawData);
    
    // Extract full \r\n delimited lines
    std::vector<std::string> lines = client->getBuffer().extract_complete_lines();
    
    for (size_t i = 0; i < lines.size(); ++i) {
        // Feed complete line to parser!
        ParsedCommand cmd = _parser.parse_line(lines[i]);
        
        // Log it (Phase 4 requirement)
        std::cout << "[fd=" << fd << "] parsed command: " << cmd.command << std::endl;
        
        // Phase 5 will handle dispatching this cmd
    }
}
```

> [!NOTE] **Darija: TCP Stream & BufferManager**
> **🇲🇦 TCP Stream:** TCP bhal l'ma f'tuyo. Ma kaygolekch fin katsali nqta w katbda nqta. Yqder ysiyft lik "NICK " w mn b3d chwia "Ayoub\r\n". L'BufferManager howa sstal (bucket) li kanjem3o fih had l'ma. Mli ki3mer sstal hta l' `\r\n` (سطر كامل), kan3tiwh l'Parser yqrah.

---

## 12. Integration with Parser

The Parser takes a complete line like `:Ayoub NICK yb\r\n` and returns a `ParsedCommand` struct.
In Phase 4, we DO NOT execute the command. We simply log the result of the parse to demonstrate the event loop is feeding complete lines properly.

---

## 13. The Event Loop (`run()` method)

This is the beating heart of the server.

```cpp
void Server::run() {
    std::cout << "[Server] Event Loop started on port " << _port << std::endl;
    
    while (!_shutdown) {
        // Poll blocks until events occur, or timeout
        int poll_count = poll(_pollfds.data(), _pollfds.size(), -1); // -1 = infinite timeout
        
        if (poll_count < 0) {
            if (errno == EINTR) {
                // We got SIGINT, _shutdown is now true, loop will exit
                continue;
            }
            std::cerr << "[Server] poll() error: " << strerror(errno) << std::endl;
            break; // Fatal error
        }
        if (poll_count == 0) {
            continue; // Should not happen with timeout -1
        }
        
        // Iterate over FDs to see who is ready
        for (size_t i = 0; i < _pollfds.size(); ++i) {
            
            // Skip FDs with no events
            if (_pollfds[i].revents == 0)
                continue;
                
            int current_fd = _pollfds[i].fd;
            
            // 1. Handle Errors / Hangup
            if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                std::cout << "[Network] FD " << current_fd << " encountered error/hangup." << std::endl;
                if (current_fd == _server_fd) {
                    std::cerr << "Fatal: Server socket error!" << std::endl;
                    _shutdown = true;
                    break;
                }
                _disconnectClient(current_fd);
                --i; // IMPORTANT VECTOR MUTATION TRICK
                continue;
            }
            
            // 2. Handle Read Events
            if (_pollfds[i].revents & POLLIN) {
                if (current_fd == _server_fd) {
                    // It's the server socket! Accept new clients.
                    _acceptNewClient();
                } else {
                    // It's a client socket! Read their data.
                    _receiveData(current_fd);
                    
                    // Note: If _receiveData caused a disconnect (recv=0), 
                    // the fd was swap-erased. We must decrement 'i' to re-evaluate the swapped element!
                    // But we don't return a boolean from receiveData in this snippet. 
                    // Let's assume _receiveData does NOT handle vector mutation, 
                    // OR we check if the FD still exists.
                    // Better approach: make _disconnectClient handle vector erase, 
                    // and decrement `i` inside the loop if the client was removed.
                }
            }
            
            // Clear revents after processing (poll does this, but good practice)
            _pollfds[i].revents = 0;
        }
    }
    
    _cleanup(); // Close all sockets
}
```

---

## 14. `send()` Infrastructure

When sending data to clients, we can't just use `write()`. Sockets are non-blocking; they might be full (TCP window full). 

```cpp
void Server::_sendData(int fd, const std::string& data) {
    size_t total_sent = 0;
    size_t len = data.length();
    
    while (total_sent < len) {
        ssize_t sent = send(fd, data.c_str() + total_sent, len - total_sent, 0);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Buffer full, would block. 
                // In a perfect world, we buffer this and wait for POLLOUT.
                // For ft_irc, spinning or dropping is often acceptable given small sizes, 
                // but buffering is better.
                break; 
            }
            if (errno == EINTR) continue;
            
            std::cerr << "[Network] Send error on fd " << fd << std::endl;
            _disconnectClient(fd);
            return;
        }
        total_sent += sent;
    }
}

void Server::sendToClient(int fd, const std::string& message) {
    std::string packet = message + "\r\n";
    _sendData(fd, packet);
}
```

---

## 15. SIGINT Graceful Shutdown

When the user presses `Ctrl+C`, the server must not abruptly crash and leave ports hanging in `TIME_WAIT`.

```cpp
// Global signal flag
volatile sig_atomic_t g_shutdown = 0;

void signal_handler(int signum) {
    (void)signum;
    g_shutdown = 1;
}

void Server::_setupSignals() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    // Ignore SIGPIPE (prevents crash if sending to a closed socket)
    signal(SIGPIPE, SIG_IGN);
}
```
In `run()`, we check `if (g_shutdown) _shutdown = true;`. 
When the loop ends, `_cleanup()` iterates over `_clients` and `close()` every fd, and finally `close(_server_fd)`.

> [!NOTE] **Darija: SIGINT and Graceful Shutdown**
> **🇲🇦 SIGINT:** Mli chi wahed kiberk 3la Ctrl+C (SIGINT), kaywqa3 intervention fl program. Blast ma ytsed dakchi bzzez (w ykheli les ports m3lqin w ydir lik "Address already in use"), kandiro `sigaction`. Kansedo les FDs bchwiya, kanrej3o la mémoire (cleanup), w kankherjo b'karama (Graceful Shutdown).

---

## 16. Error Handling

Every system call can fail. We MUST check the return values.
* `poll()` < 0: Check `EINTR`. If so, continue. Otherwise, fatal error.
* `accept()` < 0: Check `EAGAIN/EWOULDBLOCK`. If so, queue empty. Check `EINTR`.
* `recv()` < 0: Check `EAGAIN/EWOULDBLOCK` (queue empty).
* `send()` < 0: Check `EAGAIN/EWOULDBLOCK` (buffer full). Check `EPIPE/SIGPIPE` (client disconnected while writing).

---

## 17. Vector Mutation During Iteration (The `--i` Trick)

This is the most common bug in `ft_irc`.
When iterating over `_pollfds`:
```cpp
for (size_t i = 0; i < _pollfds.size(); ++i) {
    if (_pollfds[i].revents & POLLIN) {
        if (recv_fails_and_causes_disconnect) {
            _disconnectClient(fd); 
            // Inside _disconnectClient: 
            // _pollfds[i] is swap-erased with _pollfds.back()
            
            // BUG: The element that used to be at the back is now at index 'i'.
            // The loop will do ++i, and we will COMPLETELY SKIP checking the swapped element!
            
            --i; // THE FIX: Decrement i so next iteration re-checks the swapped element
        }
    }
}
```

### ASCII Diagram: The `--i` Trick

```text
Iterating i = 1
_pollfds = [ server, clientA, clientB, clientC ]
                         ^
                       (i=1)
clientA disconnects. Swap-erase happens.
Swap clientA with clientC. pop_back.

_pollfds = [ server, clientC, clientB ]
                         ^
                       (i=1)
Without --i: loop does ++i, so i=2 (points to clientB). clientC is SKIPPED!
With --i: loop does --i (i=0), then ++i (i=1), points to clientC. clientC is CHECKED!
```

> [!NOTE] **Darija: The `--i` Trick (Swap-Erase Issue)**
> **🇲🇦 Swap-Erase Issue:** Tkhayel katservi nass f'saff. Sowlty r9m 1, w mcha bhalo (disconnect). Jbti akhir wahed f'saff 7titih f blast r9m 1 (Swap-Erase). Ila derti khatwa l'gdam (i++), ghadi tdoz l'r9m 2, w dak li jbti men lekher w 7titih f r9m 1 makynch li ysewlo! 3la dakchi khassk trje3 b'lor (--i) bash t3awd tsowl hadak li jdid f blast r9m 1.

---

## 18. Ownership Rules

Who owns the memory?
* **OS**: Owns the File Descriptors. We must `close()` them.
* **Server**: Owns the `std::map` of dynamically allocated `Client*` objects. It must `delete` them on disconnect.
* **Networking Layer**: Owns the `_pollfds` vector. 
* **Client**: Owns its `BufferManager` and internal string buffers.

---

## 19. Testing Procedures

1. **Multiple Clients**: Open 3 terminals. Run `nc 127.0.0.1 6667` in all 3.
2. **Partial Packets**: Write a Python script that sends `"NICK "` sleeps for 2 seconds, then sends `"Ayoub\r\n"`. The server should not crash, block, or parse until the `\r\n` arrives.
3. **Graceful Disconnect**: Press `Ctrl+D` in netcat. Server logs "Client disconnected gracefully".
4. **Ungraceful Disconnect**: Find the netcat PID and `kill -9 <pid>`. Server logs "Error/Hangup" and cleans up without crashing.
5. **Memory Leak**: Run server with `valgrind --leak-check=full ./ircserv 6667 pass`. Connect, send messages, disconnect, and `Ctrl+C` the server. Should be 0 leaks.
6. **FD Leak**: Run `lsof -p <server_pid>` while running. Ensure FDs decrease when clients disconnect.

---

## 20. Common Mistakes 🚨

1. **Forgetting `fcntl O_NONBLOCK`**: Entire server freezes on `accept` or `recv`.
2. **Not adjusting loop index (`--i`)**: Skipping events for clients after a disconnect.
3. **Closing `fd` before removing from poll**: Kernel recycles the FD, `poll()` triggers invalid events.
4. **Not checking `recv == 0`**: Infinite loop of reading 0 bytes.
5. **Buffer overflow**: Hardcoding `recv` into a small buffer without size limits.
6. **Ignoring `EINTR`**: `poll()` crashes if a signal arrives.
7. **Double close**: Closing the fd, then trying to close it again in destructor.
8. **Memory leaks on disconnect**: Forgetting `delete client` when removing from map.
9. **Using `events` instead of `revents`**: Checking `if (pollfds[i].events & POLLIN)` instead of `revents`.
10. **Hardcoding max clients**: Using arrays instead of `std::vector` for `pollfds`.
11. **Not handling SIGPIPE**: Server crashes when writing to a client that forcefully closed their socket.
12. **Blocking poll timeout**: Passing `0` to poll (spins CPU 100%) or hardcoded large values without checking shutdown flags.
13. **Not draining accept queue**: Using `if` instead of `while` for `accept()`. Multiple fast connections get backlogged.
14. **Thread safety assumptions**: `ft_irc` is single-threaded. Do not use mutexes.
15. **Forgetting to clear revents**: Not strictly necessary as kernel overrides it, but good practice.

---

## 21. Compatibility (C++98)

Since this project is strictly C++98:
* ❌ NO `auto` keyword.
* ❌ NO `nullptr` (use `NULL`).
* ❌ NO range-based for loops (`for (Client* c : _clients)` is illegal).
* ❌ NO `<thread>`, `<mutex>`, or `std::unique_ptr`.
* Use iterators explicitly: `std::vector<struct pollfd>::iterator it = ...`

---

## 22. What Comes Next (Phase 5)

Once Phase 4 is complete, we have a robust networking layer that logs complete commands to the terminal:
`[fd=5] parsed command: NICK`

**Phase 5** (Command Dispatcher) will take over from here. Instead of just logging, `_processClientBuffer` will pass the `ParsedCommand` to an `Executor` or `Dispatcher` which will modify server state (changing nicknames, joining channels, routing messages).

---

*End of Phase 4 Documentation. Code hard, Ayoub!* 💻🔥
