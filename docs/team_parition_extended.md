<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# we formed a new team, here's how we re-partitioned the work, what I want you now is to elaborate on each step for the intricate details each one of us needs to lookup for or implement, so when we have a meet-up each one is gonna brief the others and explain what he did so. So we need interconnected partitioning so we wouldn't waste time and effectively accomplish the project in 13 days.

You already have a solid high-level partition; we can turn it into a detailed, interconnected checklist per person and phase, plus a 13‑day plan so each meetup is focused and no one blocks the others.[^1][^2]

***

## Big picture and 13‑day plan

```
Your ft_irc server must: parse `./ircserv <port> <password>`, run a non‑blocking poll‑based loop, manage clients, channels and operator commands (KICK/INVITE/TOPIC/MODE), and handle partial TCP data cleanly. Given your current division, a realistic 13‑day split could be:[^2]
```

- **Day 1–2:** Architecture \& interfaces agreed, environment \& Makefile ready (Phase 2 prep).[^3][^1]
- **Day 3–5:** Phase 2 – server foundation integrated: server boots, accepts connections, and logs them.[^1][^2]
- **Day 6–8:** Phase 3 – client mgmt integrated: clients register (PASS/NICK/USER), disconnect cleanly, buffers and parsing stable.[^2][^1]
- **Day 9–11:** Channels, modes, operator commands, error replies, robustness.[^2]
- **Day 12–13:** Heavy testing (nc, IRC clients), leak checks, edge‑case fixes, documentation.[^3][^2]

Each daily meetup: 15–30 minutes where every member reports (1) what was done, (2) interfaces changed, (3) blockers that might impact others.[^1]

***

## Shared foundations for all three

Before deep coding, you all need to lock some common decisions so the work stays interconnected and merge‑friendly.[^1]

- **Common headers and namespaces**
    - Decide filenames and namespaces: e.g. `Server.hpp`, `Client.hpp`, `Channel.hpp`, `SocketManager.hpp`, `BufferManager.hpp`, `Logger.hpp`, `Utils.hpp`.[^3][^1]
    - Agree on basic types (e.g. `typedef int fd_t;`, enums for client state, error codes) so everyone uses the same vocabulary.[^1]
- **Command and message model**
    - Agree on a minimal `ParsedCommand` structure (`command`, `params`, `raw_line`) that Souhail’s parser outputs and Ismail’s command dispatcher consumes.[^2][^1]
    - Decide which module owns the mapping from command string (e.g. `"JOIN"`, `"PRIVMSG"`) to handler functions.[^2]
- **Error and logging convention**
    - Decide on log levels (DEBUG/INFO/WARN/ERROR) and a standard format: `[timestamp] [level] [component] message` so all modules log consistently.[^3]
    - Agree how fatal vs non‑fatal errors are handled (return codes vs exceptions vs closing the client).[^3][^2]

***

## Phase 2 – Server foundation (boot, config, main loop)

### Ismail – server architecture \& lifecycle

Ismail’s work defines how all other code plugs together; he should focus on clear class boundaries and the main control flow.[^1]

**Detailed tasks**

- **Requirements digestion and sketch**
    - Read the ft_irc subject and summarize the minimal entities: `Server`, `Client`, `Channel`, maybe `CommandDispatcher` and `SocketManager`.[^2]
    - Draw a simple diagram showing who owns what (e.g. `Server` owns a `std::map<fd_t, Client>` and asks `SocketManager` to update pollfd list).[^1]
- **Server class public interface**
    - Define `class Server` with methods like `bool init(int port, const std::string &password);`, `void run();`, `void shutdown();`.[^2][^1]
    - Add hooks that other modules will call: `onNewConnection(fd_t fd)`, `onClientData(fd_t fd, const std::string &line)`, `onClientDisconnect(fd_t fd)`. These form the “contract” with Ayoub and Souhail.[^1]
- **Configuration and CLI handling**

```
- Implement parsing of `./ircserv <port> <password>`: validate port range, non‑empty password, and exit with clear error if invalid.[^2]
```

    - Put config into a struct (`ServerConfig { int port; std::string password; }`) accessible to other components if needed (e.g. for logging).[^1]
- **Main lifecycle and integration with poll**
    - Design the main loop that calls Ayoub’s poll‑driven code: e.g. `while (running) { socketManager.pollOnce(*this); }`.[^1]
    - Decide where signals (e.g. SIGINT) are handled to trigger graceful shutdown without breaking non‑blocking requirements.[^3][^2]

**What Ismail should look up / read**

- IRC basic flow: PASS / NICK / USER sequence and when to consider a client “registered”.[^3][^2]
- The Reactor pattern / single‑process event loop patterns to keep design simple and testable in C++98.[^3]

***

### Ayoub – socket layer and non‑blocking setup

Ayoub’s code is “the door” between the OS and the server logic; it must be rock‑solid and simple to use from `Server`.[^1]

**Detailed tasks**

- **Listening socket setup**
    - Implement helper like `fd_t createListeningSocket(int port)` using `socket`, `getaddrinfo`, `bind`, `listen`, and proper error checking.[^3][^2]
    - Configure `SO_REUSEADDR`, and set the socket to non‑blocking mode with `fcntl(fd, F_SETFL, O_NONBLOCK)` as allowed.[^2][^3]
- **poll() infrastructure**
    - Define a `SocketManager` (or similar) that owns `std::vector<struct pollfd>` including the listening socket and all client fds.[^2][^1]
    - Provide methods like `addClientFd(fd_t fd)`, `removeFd(fd_t fd)`, `setWritable(fd_t fd, bool)` so other modules don’t manipulate pollfd directly.[^1]
- **Single entry point for event loop**
    - Implement `void pollOnce(Server &server)` that:
        - Calls `poll()` with timeout.
        - For POLLIN on listening socket: `accept()` new client, make it non‑blocking, and call `server.onNewConnection(new_fd)`.
        - For POLLIN on client fd: call a buffer function (from Souhail) to read data and eventually `server.onClientData(fd, line)`.
        - For POLLHUP/POLLERR: call `server.onClientDisconnect(fd)` and `removeFd(fd)`.[^2][^1]

**What Ayoub should look up / read**

- Man pages for `socket`, `bind`, `listen`, `accept`, `poll`, `fcntl` and common error codes (`EINTR`, `EAGAIN`).[^3]
- Examples of non‑blocking TCP servers using `poll` (Beej’s guide style) to avoid subtle bugs like FD leaks and blocking calls.[^3][^2]

***

### Souhail – Makefile, logger, helpers (Phase 2)

Souhail’s work should make development smooth and debugging easy from day one.[^1]

**Detailed tasks**

- **Makefile and build hygiene**
    - Implement the required targets (`NAME`, `all`, `clean`, `fclean`, `re`) with the mandated flags `-Wall -Wextra -Werror -std=c++98`.[^2]
    - Add a `debug` target (optional) that compiles with `-g` and maybe `-DDEBUG` for extra logging.[^3]
- **Logger design and implementation**
    - Provide a simple global or singleton logger with functions like `log_info(component, message)`, `log_error(component, message)`.[^3]
    - Ensure logger can be used everywhere without circular dependencies, e.g. by declaring it in `Logger.hpp` and implementing in `Logger.cpp`.[^1]
- **Utility helpers**
    - Implement string helpers: trim whitespace, strip trailing `\r\n`, split by space while keeping colon‑parameters (for IRC messages).[^2][^1]
    - Provide small wrappers for error handling (e.g. `check_syscall(int ret, const char *what)` that logs and handles failures consistently).[^3]
- **Build verification**
    - Integrate a basic script or manual checklist: compile with `make re`, run `valgrind` on a simple scenario, ensure zero warnings.[^2][^3]

**What Souhail should look up / read**

- Makefile best practices for medium projects (pattern rules, object file organization).[^3]
- IRC line format and the 512‑byte limit to design safe buffer and parsing utilities.[^2][^3]

***

## Phase 3 – Client connection management

### Ismail – Client class, registration state, lifecycle

Ismail owns the semantic representation of each client and their journey from connection to disconnection.[^1]

**Detailed tasks**

- **Client data model**
    - Define `class Client` with fields: `fd`, `nickname`, `username`, `realname`, `bool registered`, `bool isOperator`, `std::set<std::string> channels`, and timestamps if needed.[^1][^2]
    - Add an enum `RegistrationState { WAITING_PASS, WAITING_NICK_USER, REGISTERED };` to express the IRC auth flow clearly.[^2]
- **Lifecycle management**
    - Implement methods in `Server` like `Client &getClientByFd(fd_t fd)` and `void removeClient(fd_t fd)` that are used by Ayoub and Souhail.[^1]
    - Provide hooks when a client becomes registered (send welcome messages, MOTD, etc.) and when they disconnect (cleanup channels).[^2]
- **Command dispatching**
    - Implement `void Server::handleCommand(Client &client, const ParsedCommand &cmd)` that routes to handlers (`handleJoin`, `handlePrivmsg`, etc.).[^1][^2]
    - Define minimal stubs for channel operations so later phases (channels, modes) can be implemented incrementally.

**What Ismail should look up / read**

- IRC RFC summaries: expected numeric replies for registration (e.g. `001`, `002`…) and basic error codes (e.g. `ERR_NICKNAMEINUSE`).[^3][^2]
- Typical state machines for protocol registration to avoid race conditions or half‑registered clients.[^3]

***

### Ayoub – poll() event loop details and FD housekeeping

Ayoub now ties poll events tightly to the Server and BufferManager behaviour.[^1]

**Detailed tasks**

- **Per‑event behaviour**
    - For each POLLIN on a client fd, call `bufferManager.onReadable(fd, server)` so that data reading is centralized but triggered only from the poll handler.[^2][^1]
    - For POLLHUP/POLLERR, ensure robust cleanup: logging, removing fd from poll list, and calling `server.onClientDisconnect(fd)`.[^2]
- **FD and resource management**
    - Maintain a map `fd -> Client*` or let `Server` own it and expose queries; avoid double ownership to prevent leaks.[^1]
    - Guarantee that closing a client (`close(fd)`) always corresponds to removing it from poll vector to avoid polling dead fds.[^2]

**What Ayoub should look up / read**

- Patterns for mapping between `pollfd` array and higher‑level objects (client structs) efficiently.[^3]
- Edge cases: what happens when `poll` returns but `recv` gets 0 or `EAGAIN`, and how to treat such cases consistently.[^3][^2]

***

### Souhail – buffer management, recv(), CRLF, parser input

Souhail ensures that raw TCP bytes become clean IRC lines ready for parsing, even when fragmented or combined.[^1]

**Detailed tasks**

- **Per‑client buffers**
    - Maintain `std::map<fd_t, std::string> buffers`; create on new connection, erase on disconnect.[^2][^1]
    - Enforce the IRC max line length (512 bytes including CRLF) by truncating or disconnecting misbehaving clients with proper error logging.[^3][^2]
- **Data ingestion**
    - Implement `void BufferManager::onReadable(fd_t fd, Server &server)`:
        - Call `recv(fd, tmp, sizeof(tmp), 0)` in a loop while data available and append to buffer.
        - Handle `recv` returning 0 (client closed) or `-1` with `EAGAIN` correctly and log issues.[^3][^2]
    - Never call `recv` outside this function to respect the “only inside poll‑driven handling” constraint.[^2]
- **Line extraction \& parser preparation**
    - Scan buffer for `\r\n`; for each complete line, extract and pass to something like `server.onClientData(fd, line_without_crlf)`.[^1][^2]
    - Keep any trailing partial message in the buffer for the next poll event.[^1][^2]

**What Souhail should look up / read**

- TCP behaviour for partial sends and concatenated packets to design correct buffering logic.[^3]
- IRC command line grammar to know how to split into command and parameters for later parsing.[^3][^2]

***

## Interconnected interfaces and “contracts”

To keep work linked and avoid wasting time, you should explicitly document a few key interfaces and respect them:

- **Server ↔ SocketManager (Ismail \& Ayoub)**
    - SocketManager exposes `pollOnce(Server&)`, `addClientFd`, `removeFd`; Server provides `onNewConnection`, `onClientData`, `onClientDisconnect`.[^1]
    - Any change in these signatures must be announced in your daily meetup to avoid compilation breakage for others.[^1]
- **Server ↔ BufferManager (Ismail \& Souhail)**
    - BufferManager exposes `onReadable(fd, Server&)`; Server handles the semantic part (clients, commands) once a full line is ready.[^1]
    - Agree on how errors inside BufferManager are reported (exceptions vs return codes) to keep the loop clean.
- **Client / Channel / Command APIs (mostly Ismail, but everyone uses them)**
    - Define minimal headers early, even with stub implementations, so Ayoub and Souhail can compile against them while Ismail fills in behaviour later.[^1]
    - Keep interfaces stable; if you must change a function name or parameters, coordinate before pushing.[^1]

***

## How to use this in your meetups

For each meetup (daily or every 2 days):

- **Ismail reports:** current state of `Server`, `Client`, and command handlers; any new or changed methods that Ayoub/Souhail must use.[^1]
- **Ayoub reports:** status of listening socket, poll loop, FD housekeeping; list of events already handled and what remains.[^1]
- **Souhail reports:** build status, logger capabilities, buffer behaviour, and any parsing utilities added or modified.[^1]

If you follow these checklists, you’ll have clear, interconnected responsibilities and each briefing will naturally cover “what I did, what changed, and how you should plug into it” while staying on track to finish in roughly 13 days.[^2][^1]
<span style="display:none">[^4]</span>

<div align="center">⁂</div>

[^1]: team-work-distribution.md

[^2]: ft_irc_mandatory_summary.md

[^3]: ft_irc_resources.md

[^4]: ft_irc_subject.pdf

