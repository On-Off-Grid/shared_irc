# ft_irc Test Suite Documentation

This directory contains automated test suites for the `ft_irc` project. The tests are designed to verify RFC 2812 compliance, C++ data structures, multi-client interaction logic, and network edge-case behavior specified in the 42 evaluation sheet.

---

## 📂 Directory Overview

| File | Type | Target Scope | Prerequisites |
| :--- | :--- | :--- | :--- |
| **`test_data_layer.cpp`** | C++ Unit/Integration Test | Low-level C++ classes (`BufferManager`, `Parser`) | Compiler (`c++`) |
| **`test_multi_client.py`** | Python Socket Test | Protocol logic, channel operations, and permissions | Running `./ircserv` instance |
| **`test_edge_cases.py`** | Python Stress Test | TCP fragmentation, abrupt disconnects, non-blocking flood handling | Running `./ircserv` instance |

---

## 📄 Test Scripts & Command Syntax

### 1. C++ Data Layer Unit Test (`test_data_layer.cpp`)

#### **Purpose**
Verifies memory buffer handling and command parsing isolated from network sockets:
- **`BufferManager`**: Tests CRLF (`\r\n`) line extraction across incomplete TCP byte chunks and multiple commands in a single buffer read.
- **`Parser`**: Validates extraction of command names, prefixes (`:server.net`), parameters, trailing arguments (`:Hello world`), and handling of invalid/empty lines.

#### **Compilation & Syntax**
```bash
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_data_layer
```

#### **Execution**
```bash
./test_data_layer
```

---

### 2. Multi-Client Protocol Test (`test_multi_client.py`)

#### **Purpose**
Automates multi-client interactions over real TCP sockets to verify standard IRC workflow:
- **Test 1: Registration Flow**: Connection handshake (`PASS` -> `NICK` -> `USER`), checking for numeric reply `001 RPL_WELCOME`.
- **Test 2: Channel Operations & Broadcast**: Multiple clients joining `#42school`, `JOIN` notification distribution, and channel message broadcasting (`PRIVMSG #channel`).
- **Test 3: Private Messaging**: Direct client-to-client messaging (`PRIVMSG nickname`).
- **Test 4: Channel Modes & Operator Privileges**:
  - Privilege enforcement: verifies regular users receive `482 ERR_CHANOPRIVSNEEDED` when attempting `MODE`.
  - Invite-only channel mode (`MODE +i`): blocks uninvited users (`473 ERR_INVITEONLYCHAN`).
  - Invitations (`INVITE`): permits invited users to join invite-only channels.
  - Operator eviction (`KICK`): verifies operator ability to evict channel members.

#### **Command-Line Syntax**
```bash
python3 tests/test_multi_client.py [--host HOST] [--port PORT] [--password PASSWORD]
```

#### **CLI Flags**
| Flag | Short | Default | Description |
| :--- | :--- | :--- | :--- |
| `--host` | | `127.0.0.1` | IP address of the target IRC server |
| `--port` | | `6667` | Port number the server is listening on |
| `--password` | | `password` | Password configured on the server |

#### **Example Usage**
```bash
# Default parameters (127.0.0.1:6667, password 'password'):
python3 tests/test_multi_client.py

# Custom port and password:
python3 tests/test_multi_client.py --port 8080 --password mysecretpass
```

---

### 3. Networking Edge Cases & Stress Test (`test_edge_cases.py`)

#### **Purpose**
Validates server stability and non-blocking I/O behavior under non-ideal network conditions:
- **Edge Case 1: Partial Commands / TCP Fragmentation**: Sends command fragments byte-by-byte with delays (simulating slow networks) and ensures other clients remain unaffected.
- **Edge Cases 2 & 3: Abrupt Disconnects & Half-Command Kills**: Unexpectedly closes socket connections in the middle of un-terminated strings (simulating `kill -9` or crash) and verifies the server does not freeze or leak state.
- **Edge Case 4: Suspended Client (Ctrl+Z) & Channel Flood**: Simulates a client that stops calling `recv()`. Floods the channel with 150 messages and tests that the main `poll()` loop continues serving active clients without blocking.

#### **Command-Line Syntax**
```bash
python3 tests/test_edge_cases.py [--host HOST] [--port PORT] [--password PASSWORD]
```

#### **CLI Flags**
| Flag | Short | Default | Description |
| :--- | :--- | :--- | :--- |
| `--host` | | `127.0.0.1` | Target server IP address |
| `--port` | | `6667` | Target server port |
| `--password` | | `password` | Target server password |

#### **Example Usage**
```bash
python3 tests/test_edge_cases.py --port 6667 --password password
```

---

## 🚀 Quick Start Guide

To run all tests against a freshly built server:

```bash
# 1. Compile the server
make

# 2. Run C++ data layer unit test
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_data_layer && ./test_data_layer && rm test_data_layer

# 3. Launch server in terminal 1
./ircserv 6667 mypassword

# 4. Run python test suites in terminal 2
python3 tests/test_multi_client.py --port 6667 --password mypassword
python3 tests/test_edge_cases.py --port 6667 --password mypassword
```

---

## 🔍 Troubleshooting

- **`ConnectionRefusedError: [Errno 111] Connection refused`**
  - Make sure `./ircserv` is actively running before executing the Python scripts.
  - Verify that the port passed in `--port` matches the port passed to `./ircserv`.

- **`Registration failed / No 001 RPL_WELCOME`**
  - Verify that the password specified in `--password` matches the password used when starting `./ircserv`.
