# 🤝 Team Collaboration Guide — ft_irc

> [!NOTE]
> This document is not an explanation of the IRC protocol or networking theory.
> It is an **internal coordination document** between Ismail, Ayoub, and Souhail.
> Its goal is simple: **what does each person need from the others, and when?**
> Read this document before writing even a single line of code.

---

## 📑 Table of Contents

1. [Introduction](#1-introduction)
2. [Team Overview](#2-team-overview)
3. [Ismail Workflow](#3-ismail-workflow)
4. [Ayoub Workflow](#4-ayoub-workflow)
5. [Souhail Workflow](#5-souhail-workflow)
6. [Development Timeline](#6-development-timeline)
7. [Dependency Graph](#7-dependency-graph)
8. [Interface Exchange](#8-interface-exchange)
9. [Integration Plan](#9-integration-plan)
10. [Git Workflow](#10-git-workflow)
11. [Daily Workflow](#11-daily-workflow)
12. [Code Ownership](#12-code-ownership)
13. [Integration Checklist](#13-integration-checklist)
14. [Communication Rules](#14-communication-rules)
15. [Common Mistakes](#15-common-mistakes)
16. [Final Summary](#16-final-summary)

---

## 1. Introduction

### Why did we split the project?

ft_irc is a big project where networking, parsing, and IRC logic are all mixed together. If all of us work on everything, we will run into a lot of problems:
- **Merge conflicts** every day.
- One person changes an interface and breaks someone else’s work.
- Duplicate work and wasted time.

The solution is: **each person takes a clear part of the project and owns it.** That way, everyone can focus on their own work without interfering with the others.

### Why does each person have ownership?

Ownership means: **you are the primary and final person responsible for that class or module.** If a bug happens there, you fix it. If someone wants to change it, they must talk to you.

This is important because:
- Everyone knows exactly where to work without asking.
- Nobody will accidentally change code that is not theirs.
- Code review becomes clear: the class owner reviews any change related to it.

### Why must communication be clear?

Because three people are working on the same project, but each one works on a different module. If you change something without informing the others:
- You might break their build.
- You might create a circular dependency.
- You might waste hours of their work.

**Golden rule: if you want to change an interface, talk to the team first.**

### Why are interfaces more important than implementation?

The interface is the contract between modules. That means the method signatures, return types, and parameters that others depend on.

Simple example:
- Ismail calls `BufferManager::extract_complete_lines()` in his code.
- If Souhail changes that method name or its parameters without informing Ismail, Ismail’s code will no longer compile.

**Solution:** lock the interfaces early. Everyone is free to change the internal implementation, but the interface is sacred — it must not change without agreement.

---

## 2. Team Overview

| Developer | Main Responsibility | Depends On | Provides To Others | Expected Deliverables |
|---|---|---|---|---|
| **Ismail** (Lead Architect) | Server, Client, Channel, Dispatcher, and Replies architecture | Ayoub (Socket + poll), Souhail (Buffer + Parser) | Server skeleton, Client object, Channel object, interfaces, architecture decisions | `Server.hpp/.cpp`, `Client.hpp/.cpp`, `Channel.hpp/.cpp`, `main.cpp`, Dispatcher, Replies |
| **Ayoub** | Socket layer and event loop (`poll()`) | Ismail (Server skeleton + interfaces) | Configured socket, working `poll()` loop, `accept()`, `send()` | `Socket.hpp/.cpp`, `PollManager.hpp/.cpp` |
| **Souhail** | Buffer, parser, Makefile, and utils | Ismail (Client interface), Ayoub (no direct dependency) | Working BufferManager, working Parser, Makefile, testing scripts | `BufferManager.hpp/.cpp`, `Parser.hpp/.cpp`, `Makefile`, `tests/` |

> [!IMPORTANT]
> Every developer must know exactly what is expected from others and what they must deliver. This table is your reference whenever you are unsure.

---

## 3. Ismail Workflow

### What should Ismail do?

Ismail is the **Lead Architect** of the project. His job is to build the core of the server:

1. **Server class:** the brain of the whole project — `init()`, `run()`, `shutdown()`.
2. **Client class:** representation of each connected user — registration state (PASS → NICK → USER), nickname, FD.
3. **Channel class:** channel management — membership, operators, `broadcast()`.
4. **Command dispatcher:** connect every IRC command (such as `JOIN`, `NICK`, `PRIVMSG`) to the correct handler.
5. **Replies:** build IRC numerics (such as `001 RPL_WELCOME`).
6. **Integration:** integrate Ayoub’s work (Socket + poll) and Souhail’s work (Buffer + Parser) inside the server.
7. **Code review:** review Ayoub’s and Souhail’s pull requests and make sure they match the architecture.
8. **Decision making:** make difficult technical decisions (for example object ownership or partial messages).

### What should he expect from Ayoub?

| Expected item | Why he needs it | When it should arrive |
|---|---|---|
| `Socket.hpp` + `Socket.cpp` | So the server can do `bind()` and `listen()` on the port | Phase 2 (Socket Layer) |
| `PollManager.hpp` + `PollManager.cpp` | So `Server::run()` can call `poll()` and wait for events | Phase 3 (Event Loop) |
| `accept()` function | To accept new clients and return the FD | Phase 3 |
| `send()` function | To send replies to clients | Phase 3 |

**Important note:** Ismail treats Ayoub’s work like a **black box** — he uses the interface without interfering with the internal implementation.

### What should he expect from Souhail?

| Expected item | Why he needs it | When it should arrive |
|---|---|---|
| `Makefile` | So the project can compile from the start | Phase 1 (Skeleton) |
| `BufferManager.hpp` + `BufferManager.cpp` | So each client can store fragmented incoming data and extract complete messages using `\r\n` (CRLF) | Phase 4 (Buffers) |
| `Parser.hpp` + `Parser.cpp` | So a complete message can be parsed into an object containing the command and parameters | Phase 4 (Parsing) |

### What should he provide to them?

| What he provides | To whom | Why |
|---|---|---|
| `Server.hpp` (Skeleton) | Ayoub + Souhail | So they understand the server structure and can integrate their work |
| `Client.hpp` (Interface) | Souhail | So BufferManager can be stored correctly in the Client |
| Architecture decisions | Ayoub + Souhail | So they do not build something that conflicts with the design |
| Code reviews | Ayoub + Souhail | So the code stays clean and consistent |

### Which files is he responsible for?

```text
includes/Server.hpp       ← Ismail
includes/Client.hpp       ← Ismail
includes/Channel.hpp      ← Ismail
src/main.cpp              ← Ismail
src/Server.cpp            ← Ismail
src/Client.cpp            ← Ismail
src/Channel.cpp           ← Ismail
```

### Which interfaces must he freeze?

Ismail is the one who defines the interfaces the others will depend on:
- `Server::init(port, password)` — how the server starts.
- `Server::run()` — the main loop.
- `Client` constructors and methods (for example `getNickname()`, `setRegistered()`, `getFd()`).
- `Channel` public methods (for example `addMember()`, `removeMember()`, `broadcast()`).

> [!WARNING]
> Once these interfaces are defined at the beginning, **they must not change without a meeting with the whole team.** Ayoub and Souhail build their work on top of them.

### What is he not allowed to change without agreement?

- ❌ The **parameters** of any public method in classes used by others.
- ❌ The **return types** of shared methods.
- ❌ The **folder structure** (for example adding a new directory without informing the team).
- ❌ Any change to the `Makefile` (because Souhail owns it).
- ❌ Any edits to Ayoub’s or Souhail’s files.

---

## 4. Ayoub Workflow

### What is his job?

Ayoub is responsible for the **lower networking layer** — everything related to sockets and the event loop:

1. **Socket class:** create the socket, `bind()`, `listen()`, `accept()`, `close()`.
2. **PollManager class:** manage the `poll()` loop — add/remove FDs, wait for events.
3. **Non-blocking I/O:** make sure all sockets are configured with `fcntl(O_NONBLOCK)`.
4. **Connection handling:** `accept()` for new clients and disconnect detection.
5. **`send()` wrapper:** send data to the client through the socket.

### What should Ismail provide to him?

| Expected from Ismail | Why he needs it | When |
|---|---|---|
| `Server.hpp` skeleton | So he understands how the server will use his socket layer | Phase 1 |
| Server interface (how `Server::run()` will call `poll()`) | So he can build PollManager in a compatible way | Phase 2 |
| Architecture decisions around connection/disconnection | So he knows when to call `accept()` and when to close the FD | Phase 3 |

### What should Souhail provide to him?

Ayoub has no direct dependency on Souhail. The data flow goes like this:

```text
Socket (Ayoub) → recv() → Buffer (Souhail)
```

But Ayoub does not call BufferManager directly — the Server (Ismail) is the one that connects them. That is why Ayoub does not need anything directly from Souhail.

### What should he return?

| What he provides | To whom | Why |
|---|---|---|
| `Socket.hpp` + `Socket.cpp` | Ismail | So they can be integrated into the server |
| `PollManager.hpp` + `PollManager.cpp` | Ismail | So `Server::run()` can call `poll()` |
| `accept()` — FD of a new client | Ismail | So he can create a new `Client` object |
| FD disconnect detection | Ismail | So he knows when to clean up the client |

### Which interfaces must he respect?

- **Server interface:** if Ismail says `Server::run()` calls `pollManager.wait_for_events()`, Ayoub must provide that exact method with that exact signature.
- **Client interface:** Ayoub must not directly modify `Client` objects — he should only return the FD and let Ismail handle the rest.
- He is **not allowed** to call Parser or BufferManager directly — that dependency breaks the architecture.

### What is he responsible for?

```text
includes/Socket.hpp       ← Ayoub
includes/PollManager.hpp  ← Ayoub
src/Socket.cpp            ← Ayoub
src/PollManager.cpp       ← Ayoub
```

---

## 5. Souhail Workflow

### What is his job?

Souhail is responsible for **data processing** — from raw bytes received from `recv()` until they become a clear IRC command:

1. **BufferManager class:** store fragmented data (partial data) and extract complete messages using `\r\n` (CRLF).
2. **Parser class:** take a full message and split it into command + parameters.
3. **Makefile:** build the full project — `make`, `make clean`, `make fclean`, `make re`.
4. **Utils/helpers:** any shared helper functions.
5. **Testing scripts:** test scripts inside `tests/`.

### What should Ismail provide to him?

| Expected from Ismail | Why he needs it | When |
|---|---|---|
| `Client.hpp` interface | So he knows how BufferManager will be stored inside the Client (as a member) | Phase 1-2 |
| Architecture decisions around data flow | So he knows exactly what enters the buffer and what comes out of the parser | Phase 2 |
| Server skeleton | So he understands how the server will call BufferManager and Parser | Phase 1 |

### What should Ayoub provide to him?

Souhail has no direct dependency on Ayoub. Data reaches BufferManager through the server (Ismail). Souhail does not need to know what a socket or `poll()` is — he only deals with incoming `std::string` data.

### What should he return?

| What he provides | To whom | Why |
|---|---|---|
| `Makefile` | Ismail + Ayoub | So the project can compile from the start |
| `BufferManager.hpp` + `BufferManager.cpp` | Ismail | So they can be integrated into the Client |
| `Parser.hpp` + `Parser.cpp` | Ismail | So they can be used in the dispatcher |
| Testing scripts | Whole team | So everyone can verify that things work |

### Which files are his?

```text
includes/BufferManager.hpp  ← Souhail
includes/Parser.hpp         ← Souhail
src/BufferManager.cpp       ← Souhail
src/Parser.cpp              ← Souhail
Makefile                    ← Souhail
tests/                      ← Souhail
```

### Which interfaces must he respect?

- **Client interface:** BufferManager should work as a member inside `Client` — Souhail should design it as self-contained and independent from the `Client` internals.
- **Server interface:** Parser should return an object containing `command` and `params` clearly — Ismail will use that result inside the dispatcher.
- BufferManager is **not allowed** to know what `Client` or `Server` is — it should only deal with `std::string`.
- Parser is **not allowed** to call anything in `Server` or `Socket` — it only takes a string and returns a parsed result.

---

## 6. Development Timeline

### Week 1: Foundation

```text
Ismail ──────── Server Skeleton + main.cpp + Server.hpp/Client.hpp
                    │
                    │ (provides the skeleton and the interfaces)
                    ▼
Souhail ─────── Makefile + BufferManager Skeleton + Parser Skeleton
                    │
                    │ (Makefile ready — project can compile)
                    ▼
Ayoub ──────── Socket.hpp + Socket.cpp (bind, listen, accept)
```

**Why this order?**
- Ismail must start first because he defines the architecture and interfaces. Without the server skeleton, the others do not know how to build their parts.
- Souhail starts in parallel because the Makefile does not depend on anyone, and he can begin the Buffer and Parser skeletons using the interface Ismail provided.
- Ayoub starts once the server skeleton is ready so he knows how the socket layer will be integrated.

### Week 2: Connections and Data

```text
Ayoub ──────── PollManager + Event Loop + Non-blocking
                    │
                    │ (`poll()` working — server can accept clients)
                    ▼
Ismail ──────── Client.cpp + Server::run() + poll() integration
                    │
                    │ (server accepts and registers clients)
                    ▼
Souhail ─────── BufferManager.cpp + Parser.cpp complete
```

**Why this order?**
- Ayoub must finish `poll()` first because without it, the server cannot handle multiple clients.
- Ismail integrates `poll()` into the server and builds client logic.
- Souhail completes Buffer and Parser independently since they work on `std::string` and do not need to wait for others.

### Week 3: Commands and Channels

```text
Ismail ──────── Channel.cpp + Dispatcher + Handlers (JOIN, PRIVMSG, KICK...)
                    │
Souhail ─────── Integration testing + parser edge cases
                    │
Ayoub ──────── Disconnect handling + FD cleanup + send() polishing
```

### Week 4: Polish and Evaluation

```text
Whole team ─── Memory leaks (Valgrind) + edge cases + documentation + peer evaluation prep
```

---

## 7. Dependency Graph

This is the dependency map — who depends on whom:

```text
                        ┌──────────────────────┐
                        │    main.cpp          │
                        │    (Ismail)          │
                        └──────────┬───────────┘
                                   │
                                   ▼
                        ┌──────────────────────┐
                        │      Server          │
                        │    (Ismail)          │
                        └──────────┬───────────┘
                                   │
                  ┌────────────────┼────────────────┐
                  │                │                │
                  ▼                ▼                ▼
        ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
        │ SocketManager│  │ ClientManager│  │   Channel    │
        │   (Ayoub)    │  │  (Ismail)    │  │  (Ismail)    │
        └──────┬───────┘  └──────┬───────┘  └──────────────┘
               │                 │
               ▼                 ▼
        ┌──────────────┐  ┌──────────────┐
        │  PollManager │  │BufferManager │
        │   (Ayoub)    │  │  (Souhail)   │
        └──────┬───────┘  └──────┬───────┘
               │                 │
               ▼                 ▼
          poll() loop      ┌──────────────┐
               │           │    Parser    │
               │           │  (Souhail)   │
               │           └──────┬───────┘
               │                  │
               │                  ▼
               │           ┌──────────────┐
               │           │  Dispatcher  │
               │           │  (Ismail)    │
               │           └──────┬───────┘
               │                  │
               │                  ▼
               │           Command Handlers
               │                  │
               │                  ▼
               │              Replies
               │                  │
               └──────────────────┘
                        │
                        ▼
                     send()
                    (Ayoub)
```

### Forbidden dependency rules

```text
 ✅ Server → SocketManager      (allowed — Server uses sockets)
 ✅ Server → BufferManager      (allowed — through Client)
 ✅ Client → BufferManager      (allowed — each Client has its own buffer)

 ❌ SocketManager → Parser      (forbidden — Socket must not know Parser)
 ❌ BufferManager → Client      (forbidden — Buffer must not know Client)
 ❌ Parser → Server             (forbidden — Parser must not call Server)
 ❌ PollManager → Channel       (forbidden — each layer should only know the layer below it)
```

> [!CAUTION]
> **Circular dependency** is the most dangerous problem that can happen. If `A.hpp` includes `B.hpp` and `B.hpp` includes `A.hpp`, the compiler will get stuck. Use `class X;` (forward declaration) in headers.

---

## 8. Interface Exchange

### Ismail provides:

| Interface | File | What it contains | Who uses it |
|---|---|---|---|
| **Server** | `Server.hpp` | `init(port, password)`, `run()`, destructor | Ayoub (to understand socket integration), Souhail (to understand the architecture) |
| **Client** | `Client.hpp` | `getFd()`, `getNickname()`, `setRegistered()`, registration state | Souhail (so BufferManager can be stored as a member) |
| **Channel** | `Channel.hpp` | `addMember()`, `removeMember()`, `broadcast()` | Whole team in Phase 3+ |

### Ayoub provides:

| Interface | File | What it contains | Who uses it |
|---|---|---|---|
| **Socket** | `Socket.hpp` | `create()`, `bind()`, `listen()`, `accept()`, `close()` | Ismail (inside `Server::init()`) |
| **PollManager** | `PollManager.hpp` | `addFd()`, `removeFd()`, `wait_for_events()` | Ismail (inside `Server::run()`) |
| **send()** | Through Socket | Sends data to the client | Ismail (to send replies) |

### Souhail provides:

| Interface | File | What it contains | Who uses it |
|---|---|---|---|
| **BufferManager** | `BufferManager.hpp` | `add_client(fd)`, `append_data(fd, chunk)`, `extract_complete_lines(fd)`, `remove_client(fd)` | Ismail (stores it in Client and calls it when `recv()` receives data) |
| **Parser** | `Parser.hpp` | `parse_line(line)` → returns a `ParsedCommand` containing Command + Params | Ismail (passes the result to the Dispatcher) |
| **Makefile** | `Makefile` | `make`, `make clean`, `make fclean`, `make re` | Whole team |

> [!TIP]
> **Rule:** each person provides the `.hpp` (interface) first before starting the `.cpp` (implementation). This lets the others begin writing code that depends on the interface without waiting for the implementation.

---

## 9. Integration Plan

### Milestone 1: Server Skeleton + Makefile ✅

| | Details |
|---|---|
| **Goal** | The project compiles and has a clear structure |
| **Required from** | Ismail (`Server.hpp` skeleton, `main.cpp`), Souhail (`Makefile`) |
| **Verification** | `make` compiles without errors, `./ircserv 6667 pass` reads arguments and exits |

---

### Milestone 2: Socket Layer

| | Details |
|---|---|
| **Goal** | The server opens a port and waits for connections |
| **Required from** | Ayoub (`Socket.hpp/.cpp`) |
| **Verification** | `./ircserv 6667 pass` + `nc localhost 6667` → connection succeeds |
| **Integration** | Ismail integrates `Socket` into `Server::init()` |

---

### Milestone 3: Event Loop (`poll`)

| | Details |
|---|---|
| **Goal** | The server accepts many clients at the same time without blocking |
| **Required from** | Ayoub (`PollManager.hpp/.cpp`), Ismail (`Client.hpp/.cpp`, `Server::run()`) |
| **Verification** | 10 `nc` clients connected at the same time, and the server does not block |

---

### Milestone 4: Buffers

| | Details |
|---|---|
| **Goal** | The server stores fragmented data and extracts complete messages |
| **Required from** | Souhail (`BufferManager.hpp/.cpp`) |
| **Verification** | Send a message in two parts (for example `NICK ism` then `ail\r\n`) — the server understands it correctly |
| **Integration** | Ismail stores BufferManager inside the Client object |

---

### Milestone 5: Parser

| | Details |
|---|---|
| **Goal** | The server understands IRC commands |
| **Required from** | Souhail (`Parser.hpp/.cpp`) |
| **Verification** | `NICK ismail\r\n` becomes `{command: "NICK", params: ["ismail"]}` |
| **Integration** | Ismail passes the parsed result to the Dispatcher |

---

### Milestone 6: Registration (PASS + NICK + USER)

| | Details |
|---|---|
| **Goal** | The client can register on the server and receive `001 RPL_WELCOME` |
| **Required from** | Ismail (registration logic, reply), Souhail (parser), Ayoub (send) |
| **Verification** | `irssi` or `nc` connects and registers successfully |

---

### Milestone 7: Channels + Commands

| | Details |
|---|---|
| **Goal** | `JOIN`, `PRIVMSG`, `PART`, `TOPIC`, `KICK`, `INVITE`, `MODE` are working |
| **Required from** | Ismail (Channel, Dispatcher, Handlers) |
| **Verification** | Two clients join a channel and talk to each other |

---

### Milestone 8: Final Polish

| | Details |
|---|---|
| **Goal** | Zero memory leaks, zero crashes, complete documentation |
| **Required from** | Whole team |
| **Verification** | `valgrind --leak-check=full`, edge cases, peer evaluation preparation |

---

## 10. Git Workflow

### Feature Branches

Every new task must be done in its own branch. **Working directly on `main` is forbidden.**

Branch naming:
- `feature/server-core` — for Ismail
- `feature/socket-layer` — for Ayoub
- `feature/parser` — for Souhail
- `fix/client-disconnect` — bug fix
- `docs/update-handbook` — documentation update

**Why feature branches?** Because if a bug happens in your work, it will not affect `main`. `main` must stay **clean and working at all times**.

### Pull Requests

When you finish your work on your branch:
1. Run `git push origin feature/your-branch`.
2. Open a **Pull Request** on GitHub.
3. Write a clear description: what you did, why, and what needs review.

**Why a PR instead of direct merge?** So someone from the team reviews your code and makes sure it does not break anything.

### Reviews

- The **owner of the relevant class** should review. For example, if the PR affects `Server.hpp`, Ismail should review it.
- The reviewer checks: does it compile? Is it C++98? Was no interface changed without agreement?
- If everything is good, they **approve** it.
- If there is a problem, they write a comment and send the PR back to the developer.

### Merge

Once the PR is approved:
1. **Merge** it into `main`.
2. **Delete the old branch** (`git branch -d feature/your-branch`) so the repo stays clean.

### Sync

**Every day before you start working:**
```bash
git checkout main
git pull origin main
git checkout feature/your-branch
git merge main
```

**Why?** So you get the latest team updates and avoid letting conflicts pile up.

---

## 11. Daily Workflow

### Morning

```text
1. git pull origin main                    ← get the latest updates
2. Check Discord                           ← see whether anyone posted something important
3. Read the issue you are working on       ← think about what you will do today
4. git checkout feature/your-branch        ← return to your branch
5. git merge main                          ← merge the latest updates
```

### Coding

```text
6. Write the code                          ← focus on one clear task
7. Test manually (for example with nc)     ← make sure it works
8. git add + git commit                    ← small commits with clear messages
   Example: feat: add Client registration state
```

> [!TIP]
> Make small and frequent commits. That is much better than one huge commit with 500 lines — if a bug appears, it is much easier to go back.

### End of Day

```text
9.  git push origin feature/your-branch    ← push your work
10. Open Pull Request (if the work is done)← request review
11. Review teammates’ PRs                  ← give feedback
12. If approved → Merge                    ← merge into main
13. Daily sync on Discord                  ← what did you finish? what is blocked?
```

### Daily meeting (5 minutes on Discord)

Each person answers 4 questions:
1. **What did you work on yesterday?**
2. **What remains for today?**
3. **Do you have any blocker?** (something stopping progress)
4. **Do you need help from someone?**

**Why this meeting?** So nobody stays blocked for a whole week on a problem that one teammate could solve in five minutes.

---

## 12. Code Ownership

| Class | Owner | Who can modify it | Approval required |
|---|---|---|---|
| `Server` | Ismail | Only Ismail | No (he is the owner) |
| `Client` | Ismail | Only Ismail | No (he is the owner) |
| `Channel` | Ismail | Only Ismail | No (he is the owner) |
| `Reply` | Ismail | Only Ismail | No (he is the owner) |
| `CommandDispatcher` | Ismail | Whole team (to add handlers) | ✅ Ismail must approve |
| `Socket` | Ayoub | Only Ayoub | No (he is the owner) |
| `PollManager` | Ayoub | Only Ayoub | No (he is the owner) |
| `BufferManager` | Souhail | Only Souhail | No (he is the owner) |
| `Parser` | Souhail | Only Souhail | No (he is the owner) |
| `Makefile` | Souhail | Souhail + if a file must be added, the team informs Souhail | ✅ Souhail must approve |
| `main.cpp` | Ismail | Only Ismail | No (he is the owner) |
| `tests/` | Souhail | Whole team can add tests | No (but do not delete someone else’s tests) |

> [!IMPORTANT]
> **Core rule:** do not modify a file you do not own without speaking to its owner. Even if the change is small — PR + review first.

---

## 13. Integration Checklist

Before opening a pull request or merging into `main`, **you must verify all of these points:**

```text
✔ Compiles          — make compiles without errors
✔ C++98             — no auto, no nullptr, no range-based for
✔ No warnings       — -Wall -Wextra -Werror all enabled and zero warnings
✔ No leaks          — valgrind --leak-check=full is clean
✔ No crashes        — the server does not crash even with invalid input
✔ Non-blocking      — all sockets use fcntl(O_NONBLOCK)
✔ poll() works      — there is no blocking call in the main loop
✔ Interfaces        — no interface was changed without team agreement
✔ OCF               — all classes have constructor, destructor,
                      copy constructor, assignment operator
✔ No globals        — zero global variables
✔ Documentation     — if you changed an interface, update the docs
✔ Reviewed          — at least one teammate reviewed it
```

> [!WARNING]
> **If even one condition is not satisfied, the PR must not be merged into `main`.** `main` must always remain clean and working.

---

## 14. Communication Rules

### When should you ask a question?

- **On Discord:** if the question is simple and does not need an immediate answer (for example, “Should this method return `int` or `bool`?”).
- **In a GitHub issue:** if the question is larger and needs discussion (for example, “Should we use `std::map` or `std::vector` for channels?”).

### When should you call?

- If you have a **blocker** stopping your work.
- If you found a **bug** in someone else’s code and do not understand the reason.
- If you want to **change an interface** and need team agreement.

### When should you open a GitHub issue?

- If you found a **bug** that needs fixing.
- If you have a **feature request**.
- If you want to **propose an architecture change**.

### When should you change an interface?

**Only if the whole team agrees.** Steps:
1. Open a GitHub issue and explain why you want the change.
2. Discuss it with the owner of the affected classes.
3. Agree on the new interface.
4. Everyone updates their code.
5. Update the documentation.

### When should you **not** modify someone else’s code?

- **Always.** Do not modify code that is not yours. If you find a bug in Ayoub’s code:
  1. Write a comment on the PR or call him.
  2. Let him fix it and make the commit.
  3. This keeps `git blame` clear and preserves ownership.

**The only exception:** if the owner is absent and you find a bug that breaks the build on `main` — fix it, open a PR, and let the owner review it when they return.

---

## 15. Common Mistakes

### ❌ Mistake 1: Changing an interface without informing the team

**Example:** Souhail changes `Parser::parse()` from `parse(std::string)` to `parse(std::string, int)`.
**Result:** Ismail’s code that calls `parse()` no longer compiles.
**Why it is dangerous:** because you wasted someone else’s time on a problem you created.
**Solution:** never change the signature of a public method without informing everyone who uses it.

### ❌ Mistake 2: Duplicate work

**Example:** Ismail writes a simple parser inside his server because Souhail has not finished yet.
**Result:** now they have two parsers and nobody knows which one to use.
**Why it is dangerous:** it wastes time and creates confusion.
**Solution:** if you are waiting on something, call its owner and ask when it will be ready. Do not redo their work.

### ❌ Mistake 3: Editing the same file at the same time

**Example:** Ismail and Ayoub both modify `Server.cpp` at the same time in different branches.
**Result:** a big merge conflict that is hard to resolve.
**Why it is dangerous:** it can waste half a day of work.
**Solution:** each person should only work in their own files (see the code ownership table). If a shared file must be edited, coordinate first.

### ❌ Mistake 4: Circular dependency

**Example:** `Client.hpp` includes `Channel.hpp`, and `Channel.hpp` includes `Client.hpp`.
**Result:** the compiler gets stuck and gives confusing errors.
**Why it is dangerous:** it can take hours to find the issue.
**Solution:** use forward declarations (`class Channel;`) in headers. Put `#include` only in `.cpp` files where possible.

### ❌ Mistake 5: Breaking the `main` build

**Example:** someone merges into `main` without testing — the code no longer compiles.
**Result:** the whole team is blocked because they cannot pull and work safely.
**Why it is dangerous:** it blocks three people instead of one.
**Solution:** **always** run `make` before merging. The full integration checklist must be satisfied.

### ❌ Mistake 6: Working directly on the `main` branch

**Example:** Ayoub writes code directly in `main` without creating a feature branch.
**Result:** if he breaks something, it is hard to go back.
**Why it is dangerous:** `main` must always stay stable. If you write there directly, any mistake affects the whole team.
**Solution:** **always** create a feature branch before you start coding.

### ❌ Mistake 7: Not testing before opening a PR

**Example:** Souhail opens a PR without testing with `make` or `valgrind`.
**Result:** the PR contains bugs and the reviewer has to send it back, wasting both people’s time.
**Why it is dangerous:** it slows down the whole team.
**Solution:** before opening a PR, go through the full integration checklist — compile, no warnings, no leaks, tests.

---

## 16. Final Summary

```text
┌─────────────────────────────────────────────────────┐
│                                                     │
│    If everyone respects their ownership...          │
│                                                     │
│    and respects other people’s interfaces...        │
│                                                     │
│    and respects the workflow (Branch → PR → Review │
│    → Merge)...                                      │
│                                                     │
│    then integration will be much easier.            │
│                                                     │
│    ╔═══════════════════════════════════════════╗     │
│    ║  Ownership + Interfaces + Workflow       ║     │
│    ║  = Integration without problems          ║     │
│    ╚═══════════════════════════════════════════╝     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Summary for each member:**

**Ismail (Lead Architect):**
- You build the foundation (Server, Client, Channel).
- You define the interfaces others will build on.
- You review and integrate.
- Your biggest responsibility: **keep the architecture clean and consistent.**

**Ayoub (Network Layer):**
- You open the door between the server and the outside world.
- Without your sockets and `poll()`, there is no connection and no server.
- You must provide a clear interface and respect Ismail’s architecture.
- Your biggest responsibility: **keep the network layer non-blocking and stable.**

**Souhail (Data Processing):**
- You turn raw bytes into understandable IRC commands.
- BufferManager and Parser are the bridge between networking and logic.
- You must provide a clear interface and remain independent from Client and Server internally.
- Your biggest responsibility: **process data correctly — partial messages, CRLF, parsing.**

---

> [!NOTE]
> This document is based on `ismail-guide.md` and `team-work-distribution.md` from the original project docs.
> If there is any contradiction between this document and those files, the original files are the reference.
> Update this document whenever any interface or ownership changes.

Good luck to the whole team on ft_irc! 🚀
