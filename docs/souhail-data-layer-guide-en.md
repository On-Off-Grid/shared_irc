# 📦 Data Processing Layer Integration Guide (BufferManager & Parser)

**Author:** Souhail  
**Target Teammates:** Ismail (Lead Architect) & Ayoub (Network Layer)  
**Branch:** `feature/buffer_management`

---

## 1. Overview

This document explains how the **Data Processing Layer** works and how to integrate it into the `Server` and `Client` architectures.

The data layer handles:
1. **Partial Data Buffering (`BufferManager`)**: Solves TCP fragmentation by accumulating raw bytes per socket file descriptor (`fd_t`) until complete CRLF (`\r\n`) lines arrive.
2. **RFC 1459 Command Parsing (`Parser`)**: Converts raw string lines into clean, structured `ParsedCommand` objects.

---

## 2. Component 1: `BufferManager`

### File Location
- Header: `includes/BufferManager.hpp`
- Implementation: `src/BufferManager.cpp`

### Core Interface

```cpp
#include "BufferManager.hpp"

BufferManager bm;

// 1. When a client connects (or first recv):
bm.add_client(fd);

// 2. When recv(fd, buf, len, 0) receives raw bytes from socket:
std::string raw_chunk(buf, bytes_read);
bm.append_data(fd, raw_chunk);

// 3. Extract all complete lines (delimited by \r\n or \n):
std::vector<std::string> lines = bm.extract_complete_lines(fd);

for (size_t i = 0; i < lines.size(); ++i)
{
    // Pass line to Parser
}

// 4. When client disconnects:
bm.remove_client(fd);
```

### Key Guarantees
- **TCP Fragmentation safe**: Incomplete chunks remain in internal buffer across event loops until `\r\n` is received.
- **Overflow Protection**: Automatically truncates lines exceeding `512` bytes to prevent memory buffer overflow attacks.
- **Standalone**: Does not depend on `Client` or `Server` classes.

---

## 3. Component 2: `Parser` & `ParsedCommand`

### File Location
- Headers: `includes/Parser.hpp`, `includes/ParsedCommand.hpp`
- Implementation: `src/Parser.cpp`

### `ParsedCommand` Structure

```cpp
struct ParsedCommand
{
    std::string              prefix;       // e.g. "nick!user@host" (if present)
    std::string              command;      // e.g. "PRIVMSG", "NICK", "JOIN" (normalized UPPERCASE)
    std::vector<std::string> params;       // list of all parameters
    std::string              trailing;      // e.g. "Hello team!" (content after ':')
    std::string              raw_line;      // original unparsed line
    bool                     has_prefix;    // true if ':' prefix was present
    bool                     has_trailing;  // true if ':' trailing param was present
    bool                     is_valid;      // true if command token was extracted successfully
};
```

### Core Interface

```cpp
#include "Parser.hpp"

Parser parser;
ParsedCommand cmd = parser.parse_line(line);

if (!cmd.is_valid)
    return; // Ignore empty / malformed lines

if (cmd.command == "NICK")
{
    std::string new_nick = cmd.params[0];
    // handle NICK logic...
}
else if (cmd.command == "PRIVMSG")
{
    std::string target = cmd.params[0];
    std::string text   = cmd.trailing; // or cmd.params[1]
    // handle PRIVMSG logic...
}
```

---

## 4. Full Integration Workflow (for Ismail's `Server::run()`)

```cpp
// Inside event loop when recv() > 0:
char buf[1024];
ssize_t bytes = recv(clientFd, buf, sizeof(buf), 0);

if (bytes > 0)
{
    _bufferManager.append_data(clientFd, std::string(buf, bytes));
    std::vector<std::string> lines = _bufferManager.extract_complete_lines(clientFd);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        ParsedCommand cmd = _parser.parse_line(lines[i]);
        if (cmd.is_valid)
        {
            _dispatcher.dispatch(clientFd, cmd);
        }
    }
}
else if (bytes == 0)
{
    // Disconnect cleanup
    _bufferManager.remove_client(clientFd);
}
```

---

## 5. How to Compile and Run Unit Tests

```bash
# Compile project:
make re

# Run data processing unit tests (ensure whole command is on a single line):
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_runner && ./test_runner
```
