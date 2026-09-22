# Phase 5: Command Dispatcher & Registration 🔐

**Owner:** Ismail  
**Integrated by:** Souhail (data layer), Ayoub (networking layer already in place from Phase 4)  
**Branch:** `feature/phase-05-command-dispatcher`

---

## 📑 Table of Contents

1. [Overview](#1-overview)
2. [What Changed in Phase 5](#2-what-changed-in-phase-5)
3. [New Files](#3-new-files)
4. [Modified Files](#4-modified-files)
5. [Architecture](#5-architecture)
6. [Registration Flow (PASS → NICK → USER → 001)](#6-registration-flow)
7. [CommandDispatcher — Command Routing](#7-commanddispatcher--command-routing)
8. [Replies — IRC Numeric Builders](#8-replies--irc-numeric-builders)
9. [Channel — Skeleton](#9-channel--skeleton)
10. [Client — Registration Fields](#10-client--registration-fields)
11. [Server — Public API Additions](#11-server--public-api-additions)
12. [Integration Checklist](#12-integration-checklist)
13. [Testing Procedures](#13-testing-procedures)
14. [What Comes Next (Phase 6+)](#14-what-comes-next)

---

## 1. Overview

Phase 5 implements **Milestone 5 (Parser integration + Dispatcher)** and **Milestone 6 (Client Registration)** from the team collaboration guide.

**Goal:** A real IRC client (or `nc`) can now:
1. Connect to the server.
2. Authenticate with `PASS`.
3. Set a nickname with `NICK`.
4. Register identity with `USER`.
5. Receive the `001 RPL_WELCOME` burst and be fully registered.

> [!IMPORTANT]
> Until a client completes the `PASS → NICK → USER` sequence, all other commands are silently ignored.

---

## 2. What Changed in Phase 5

| Area | Before (Phase 4) | After (Phase 5) |
|---|---|---|
| `_processClientBuffer` | Logged parsed commands only (`TODO Phase 5`) | Dispatches every `ParsedCommand` to `CommandDispatcher` |
| `Client` | Only `fd`, `ip`, `registered`, `buffer` | + `nickname`, `username`, `realname`, `passReceived` |
| `Server` | Private `_disconnectClient()` | Public `disconnectClient()` + `getPassword()` |
| Registration | Never completed | Completed when PASS+NICK+USER all accepted → sends `001`–`004` |

---

## 3. New Files

```text
includes/Channel.hpp             ← Ismail — channel skeleton (members, ops, broadcast)
includes/Replies.hpp             ← Ismail — IRC numeric reply constants and builders
includes/CommandDispatcher.hpp   ← Ismail — routes ParsedCommand to handlers
src/Channel.cpp                  ← Ismail
src/Replies.cpp                  ← Ismail
src/CommandDispatcher.cpp        ← Ismail
```

---

## 4. Modified Files

```text
includes/Client.hpp     — Phase 5 registration fields added
includes/Server.hpp     — CommandDispatcher member, getPassword(), disconnectClient()
src/Client.cpp          — new getters/setters for nick/user/real/passReceived
src/Server.cpp          — dispatcher init/destroy, _processClientBuffer wired up
Makefile                — Channel, Replies, CommandDispatcher added to SRCS
```

---

## 5. Architecture

```text
Event Loop (poll)
      │
      ▼ POLLIN on client fd
_receiveData(fd)
      │  recv() → client->getBuffer().append_data(rawChunk)
      ▼
_processClientBuffer(fd)
      │  client->getBuffer().extract_complete_lines()
      │  _parser.parse_line(line) → ParsedCommand
      ▼
CommandDispatcher::dispatch(fd, cmd)
      │
      ├─ "PASS" → _handlePass()  → validate server password
      ├─ "NICK" → _handleNick()  → validate + set nickname
      ├─ "USER" → _handleUser()  → set username + realname
      ├─ "QUIT" → _handleQuit()  → send QUIT, disconnectClient()
      │
      └─ (registered) → ERR_UNKNOWNCOMMAND for unknown cmds
             (Phase 7 adds JOIN, PRIVMSG, KICK, …)
```

---

## 6. Registration Flow

```text
Client          Server
  │                │
  │──PASS pass────►│  Server::_password == pass?
  │                │  YES → client.setPassReceived(true)
  │                │  NO  → 464 ERR_PASSWDMISMATCH
  │                │
  │──NICK ismail──►│  valid nickname? not in use?
  │                │  YES → client.setNickname("ismail")
  │                │         _tryCompleteRegistration() → not yet (no USER)
  │                │
  │──USER ismail 0 * :Real Name──►│
  │                │  client.setUsername / setRealname
  │                │  _tryCompleteRegistration():
  │                │    passReceived && hasNick && hasUser → TRUE
  │                │    client.setRegistered(true)
  │◄──001 RPL_WELCOME─────────────│
  │◄──002 RPL_YOURHOST────────────│
  │◄──003 RPL_CREATED─────────────│
  │◄──004 RPL_MYINFO──────────────│
  │                │  [Client is now registered]
```

### Verification command

```bash
nc -C 127.0.0.1 6667
PASS testpass
NICK ismail
USER ismail 0 * :Ismail Test
# Expected: :ircserv 001 ismail :Welcome to the IRC Network, ismail!ismail@127.0.0.1
```

---

## 7. CommandDispatcher — Command Routing

```cpp
/* Server creates it once in constructor: */
_dispatcher = new CommandDispatcher(*this);

/* _processClientBuffer calls: */
_dispatcher->dispatch(fd, cmd);
```

### Handlers in Phase 5

| Command | Handler | Key logic |
|---|---|---|
| `PASS` | `_handlePass()` | Compares `cmd.params[0]` against `_server.getPassword()` |
| `NICK` | `_handleNick()` | Validates format, checks uniqueness, calls `_tryCompleteRegistration` |
| `USER` | `_handleUser()` | Requires 4 params, sets username+realname, calls `_tryCompleteRegistration` |
| `QUIT` | `_handleQuit()` | Sends `QUIT` message, calls `_server.disconnectClient(fd)` |

### Nickname validation rules (RFC 2812 §2.3.1)

- First char: letter or `[ ] \ \` ^ { | }`
- Remaining: letter, digit, `-`, or any of the above specials
- Max 30 characters

---

## 8. Replies — IRC Numeric Builders

```cpp
/* Every function returns a string WITHOUT trailing \r\n */
/* Server::sendToClient() appends \r\n automatically      */

Replies::welcome(nick, user, host)     // 001
Replies::yourHost(nick)                // 002
Replies::created(nick, date)           // 003
Replies::myInfo(nick)                  // 004

Replies::errNeedMoreParams(nick, cmd)  // 461
Replies::errAlreadyRegistered(nick)    // 462
Replies::errPasswdMismatch(nick)       // 464
Replies::errNoNicknameGiven(nick)      // 431
Replies::errErroneusNickname(n, bad)   // 432
Replies::errNicknameInUse(n, used)     // 433
Replies::errUnknownCommand(nick, cmd)  // 421
```

> [!TIP]
> All numeric codes are defined as `#define` constants in `Replies.hpp` so they are easy to add to dispatch logic without magic strings.

---

## 9. Channel — Skeleton

`Channel` is created in Phase 5 as a skeleton. It supports:
- `addMember(client*)` / `removeMember(fd)` / `hasMember(fd)`
- `addOperator(fd)` / `removeOperator(fd)` / `isOperator(fd)`
- `setTopic(topic)` / `getTopic()`
- `broadcast(message, excludeFd)` — sends to all members except one

> [!NOTE]
> `Channel` does **not** own `Client*` pointers — they are owned by `Server::_clients`. Channels only store references.

Phase 7 will expand `Channel` with `MODE` flags (`i`, `t`, `k`, `o`, `l`), `KICK`, `INVITE`, and `TOPIC` restriction enforcement.

---

## 10. Client — Registration Fields

New private fields added to `Client`:

| Field | Type | Set by | Meaning |
|---|---|---|---|
| `_nickname` | `std::string` | `setNickname()` | Empty until NICK received |
| `_username` | `std::string` | `setUsername()` | Empty until USER received |
| `_realname` | `std::string` | `setRealname()` | Empty until USER received |
| `_passReceived` | `bool` | `setPassReceived()` | `true` after valid PASS |

Helper predicates: `hasNick()`, `hasUser()`, `hasReceivedPass()`.

---

## 11. Server — Public API Additions

```cpp
/* New public methods (Phase 5): */
void               disconnectClient(int fd);   // previously private _disconnectClient
const std::string& getPassword()        const; // for CommandDispatcher PASS check
```

`CommandDispatcher` holds a `Server&` reference and uses these two methods.

---

## 12. Integration Checklist

```text
✔ Compiles          — make re: zero errors, zero warnings
✔ C++98             — no auto, no nullptr, no range-based for
✔ No warnings       — -Wall -Wextra -Werror all pass
✔ No leaks          — _dispatcher deleted in Server destructor
✔ Non-blocking      — no new blocking calls added
✔ poll() works      — dispatcher called only from poll event handler
✔ Interfaces        — no existing interface changed without agreement
✔ OCF               — Channel/Replies/CommandDispatcher all have OCF
✔ No globals        — static _shutdown only (pre-existing, phase 4)
✔ Registration      — PASS+NICK+USER → 001–004 verified with nc
✔ Error replies     — wrong PASS → 464, bad nick → 432/433
✔ QUIT              — disconnects cleanly, no leak
```

---

## 13. Testing Procedures

### Manual test — full registration

```bash
./ircserv 6667 testpass
# In another terminal:
nc -C 127.0.0.1 6667
PASS testpass
NICK ismail
USER ismail 0 * :Ismail Test
# Expected replies:
# :ircserv 001 ismail :Welcome to the IRC Network, ismail!ismail@127.0.0.1
# :ircserv 002 ismail :Your host is ircserv, running version ft_irc-1.0
# :ircserv 003 ismail :This server was created today
# :ircserv 004 ismail ircserv ft_irc-1.0 o o
```

### Manual test — wrong password

```bash
PASS wrongpassword
# Expected: :ircserv 464 * :Password incorrect
```

### Manual test — nick in use

```bash
# Connect two nc sessions with the same nick:
# Session 1: PASS testpass / NICK dupnick / USER u 0 * :R
# Session 2: PASS testpass / NICK dupnick
# Expected on session 2: :ircserv 433 * dupnick :Nickname is already in use
```

### Manual test — partial data (TCP fragmentation)

```bash
# Using nc with Ctrl+D to send in parts:
nc -C 127.0.0.1 6667
PAS    <Ctrl+D>
S testpass<ENTER>
# Expected: Server reassembles from BufferManager and processes PASS correctly
```

---

## 14. What Comes Next

### Phase 6 (already defined in Milestone 6 — done in Phase 5)
✅ Registration with `PASS`, `NICK`, `USER` and `001 RPL_WELCOME` → **complete**

### Phase 7 — Channels & Commands

Ismail adds to `CommandDispatcher::dispatch()`:

| Command | Handler to add |
|---|---|
| `JOIN` | Create/join channel, send topic + names list |
| `PART` | Leave channel |
| `PRIVMSG` | Send message to nick or channel |
| `NOTICE` | Like PRIVMSG but no auto-reply |
| `TOPIC` | Get/set channel topic |
| `KICK` | Op removes a user from channel |
| `INVITE` | Op invites user to channel |
| `MODE` | Channel modes: `i`, `t`, `k`, `o`, `l` |

Server will need a `std::map<std::string, Channel*> _channels` container.

> [!NOTE]
> This document is Souhail's Phase 5 integration record and serves as the handoff to Ismail for Phase 7 channel commands.
