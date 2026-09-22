# 📚 Lesson 3: Phases - Theory and Practice 🛠️

This lesson details the theoretical and practical concepts introduced throughout the seven development phases of the `ft_irc` server. It explains the core network programming concepts, system calls, and design decisions behind our server architecture.

---

## 📑 Table of Contents

1. [Phase 1: Project Architecture & C++98 OOP Foundations](#phase-1-project-architecture--c98-oop-foundations)
2. [Phase 2: TCP Server Socket Setup & Configuration](#phase-2-tcp-server-socket-setup--configuration)
3. [Phase 3: Client Connection Management & Lifecycle](#phase-3-client-connection-management--lifecycle)
4. [Phase 4: Multiplexed Event Loop & Non-Blocking I/O](#phase-4-multiplexed-event-loop--non-blocking-io)
5. [Phase 5: Protocol Parsing & Command Routing](#phase-5-protocol-parsing--command-routing)
6. [Phase 6: Channel Architecture & Operator Commands](#phase-6-channel-architecture--operator-commands)
7. [Phase 7: Outgoing Writable Buffers & Writable Event Monitoring](#phase-7-outgoing-writable-buffers--writable-event-monitoring)

---

## Phase 1: Project Architecture & C++98 OOP Foundations

### 1. Theoretical Concepts
* **Orthodox Canonical Form (OCF)**: C++98 standard requires classes managing dynamic resources to strictly declare a Default Constructor, Copy Constructor, Copy Assignment Operator, and Destructor. This prevents compiler-generated defaults from performing shallow copies, which leads to double-free bugs.
* **Separation of Concerns**: Splitting the codebase into independent, encapsulated modules: Networking ([`Server`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Server.hpp)), Data Layer ([`Client`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Client.hpp), [`Channel`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Channel.hpp)), and Protocol Parsing ([`Parser`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Parser.hpp)).

### 2. Practical Implementation
* **Forbidden Copying**: For singleton-like manager classes (like `Server` or `Channel`), we make the Copy Constructor and Assignment Operator `private` to disable copying explicitly.
* **Header Guards**: Systematically using `#ifndef CLASS_HPP` block wrappers to avoid duplicate declarations during translation unit builds.

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Phase 1 kmatel l'ساس dyal software design f C++98. Kano khass classes kamlin (bhal Client w Channel) ikono tab3in l'Orthodox Canonical Form (OCF) bash n-evitaw double-frees w memory leaks. Classes li ma khasshomsh it-copied (bhal Server) dernalhom Copy Constructors private.

---

## Phase 2: TCP Server Socket Setup & Configuration

### 1. Theoretical Concepts
* **Sockets as SIDs**: A Socket is a software file descriptor (FD) mapped by the OS to represent an endpoint for sending or receiving data over the network.
* **TCP Bind and Listen**:
  - `bind()` binds the socket to a local port.
  - `listen()` shifts the socket state from active (connecting out) to passive (listening for incoming handshakes).

### 2. Practical Implementation
* **System Calls**:
  ```cpp
  _serverFd = socket(AF_INET, SOCK_STREAM, 0);
  ```
* **SO_REUSEADDR**: Setsockopt option to avoid the `Address already in use` error when restarting the server (bypasses TCP's `TIME_WAIT` socket state).
  ```cpp
  int optval = 1;
  setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  ```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Socket hya l'bab dyal network. F'had Phase 2, zedna l'code bash server ikhleq socket TCP (`SOCK_STREAM`), w y-bindiha m3a l'port dyal listining. Dernalha `SO_REUSEADDR` bash mli ntfiw server nqdro n-restartiwuh f'l'bla7a bla may-bloqina OS.

---

## Phase 3: Client Connection Management & Lifecycle

### 1. Theoretical Concepts
* **Client Mapping**: Storing connected client references mapped directly by their unique socket file descriptors (`std::map<int, Client*>`).
* **Connection Lifecycle**: Creating a data entity representing a client upon connection (`accept`), modifying its variables during execution, and safely deleting it on socket closure (`close`).

### 2. Practical Implementation
* Centralizing client storage inside `Server::_clients`.
* Using clean setters/getters inside `Client.cpp` to store registration parameters, IP addresses, and state flags.

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Kol klyan dkhl 3ndna khassna n-trakiwuh. Phase 3 khedmat 3la `Client` class w l'mapping fd ──► Client* inside `Server`. Mli client ikounicta, kan-accepteiwuh, kan-creeaw object Client jdid w y-traka safely hta ikhroj.

---

## Phase 4: Multiplexed Event Loop & Non-Blocking I/O

### 1. Theoretical Concepts
* **I/O Multiplexing (Reactor Pattern)**: Instead of dedicating a thread or process per client, a single thread monitors a list of file descriptors using a system call (`poll()`). The kernel alerts the thread only when an event (read/write/error) is ready.
* **Non-blocking Sockets**: Prevents network system calls (`accept`, `recv`, `send`) from blocking execution if no data is present.

### 2. Practical Implementation
* **Fcntl Config**:
  ```cpp
  fcntl(fd, F_SETFL, O_NONBLOCK);
  ```
* **Event Monitoring**: Keeping a dynamic `std::vector<struct pollfd>` list. Adding the server socket (looking for `POLLIN` for new connections) and client sockets (looking for read events).

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Blast ma ndiro thread l'kol client, l'event loop b `poll()` kadiro single-threaded monitoring. Kat-gul l'OS: "3lemny ila shi client sifet chi haja (`POLLIN`) aw khtafet (`POLLHUP`)". Hada l'qalb d'server.

---

## Phase 5: Protocol Parsing & Command Routing

### 1. Theoretical Concepts
* **TCP Stream Assembly**: TCP does not preserve message boundaries. Messages can arrive fragmented (e.g. `"NICK "` then `"souhail\r\n"`). The server must buffer incoming bytes and only parse them when a complete `\r\n` boundary is found.
* **IRC Registration State Machine**: Clients must complete `PASS` → `NICK` → `USER` validation before receiving numeric `RPL_WELCOME` (001) replies.

### 2. Practical Implementation
* [`BufferManager`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/BufferManager.cpp) appends data on read and extracts full strings delimited by `\r\n`.
* [`Parser`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/Parser.cpp) breaks down raw command strings into `ParsedCommand` structures.
* [`CommandDispatcher`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/CommandDispatcher.cpp) routes commands to their respective private handlers.

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** TCP ki-sift data mferta (fragmented). BufferManager ki-gluiha hta nlqaw `\r\n`. Parser ki-parseha l'ParsedCommand w CommandDispatcher ki-routerha l'handler dialha dial PASS, NICK, w USER.

---

## Phase 6: Channel Architecture & Operator Commands

### 1. Theoretical Concepts
* **IRC Channels (Rooms)**: Group nodes containing references to multiple connected clients. 
* **Operator Privileges**: An authorization level separating channel creators/moderators (`@` operator) from regular users. Operators control access modes and eject users.

### 2. Practical Implementation
* **Dynamic Room Allocation**: Allocating [`Channel`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/Channel.cpp) instances in `Server::_channels` map. 
* **Operator Flags**: Enforcing Operator constraints on `KICK`, `INVITE`, `TOPIC`, and `MODE` commands.
* **Mode String Parsing**: Toggling modes `i` (invite-only), `t` (topic restrict), `k` (password), `l` (limit), `o` (give/take operator status) with variable parameters.

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Channels huma l'ghoraf. L'user li ki-cree room lowel ki-koun automatic operator (`@`). Phase 6 khedmat 3la controls dyal operator: `KICK` bash y-jri 3la user, `INVITE` w les modes b `MODE`.

---

## Phase 7: Outgoing Writable Buffers & Writable Event Monitoring

### 1. Theoretical Concepts
* **Outbound Buffering**: Just as TCP read operations can fragment, write operations can block if the kernel's transmit buffer is full. Writing synchronously using blocking calls blocks the server.
* **Asynchronous Write Loops**: Appending outgoing messages to a queue, setting the `POLLOUT` event flag, and performing non-blocking writes once `poll()` indicates the socket is writable.

### 2. Practical Implementation
* **Outgoing Queue**: Storing data in `std::map<int, std::string> _outgoingBuffers`.
* **POLLOUT Toggling**: `_setPollOut(fd, true)` when messages are queued; `_setPollOut(fd, false)` when buffers are fully flushed to avoid spinning the CPU.
* **Non-blocking Write**: Writing inside `_flushOutgoingData(fd)` and handling `EAGAIN` / `EWOULDBLOCK` by leaving the remaining unsent bytes in the queue.

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Phase 7 kat-khelli hta l'ktaba (write/send) tkoun fully non-blocking. Blast ma nsifto message direct, kan-khznouh f `_outgoingBuffers` map w n-checkiw bit `POLLOUT` f `poll()`. Mli OS kigul lina wajed (POLLOUT), kan-sifto as much as possible safely.
