# ft_irc: Building a fully compliant C++98 IRC Server 🌐

Marhaban bikom f'had l'repo! `ft_irc` howa wahed l'mashrou3 dyal l'curriculum f 42 fin bneina **IRC Server** mn zero b **C++98** standard. 

The main goal dyal had l'server is to handle multiple concurrent clients using **single-threaded I/O multiplexing** via `poll()`, manage channels, handle operator-specific commands, and guarantee non-blocking message transmission using a custom poll-driven write queue.

---

## 📑 Table of Contents (l'Fihris)

1. [Quick Start (Khedem l'Server)](#1-quick-start-khedem-lserver)
2. [Server Architecture (L'bniya dyal l'code)](#2-server-architecture-lbniya-dyal-lcode)
3. [Phases Summary (L'marahil dyal l'khedma)](#3-phases-summary-lmarahil-dyal-lkhedma)
4. [Bilingual Darija/English Code Specs](#4-bilingual-darijaenglish-code-specs)
   - [Event Loop & Multiplexing (`poll`)](#event-loop--multiplexing-poll)
   - [Buffer & Parsing Layer (Partial reads)](#buffer--parsing-layer-partial-reads)
   - [Channel Management & Operator Modes](#channel-management--operator-modes)
   - [Poll-Driven Outbound Delivery (`POLLOUT`)](#poll-driven-outbound-delivery-pollout)
5. [Staged Documentation Reference](#5-staged-documentation-reference)

---

## 1. Quick Start (Khedem l'Server)

First, dir build l'exécutable b `make`:
```bash
make re
```
Hada ghadi i-compile everything safely m3a dependency tracking (`-MMD -MP`) dyal header updates.

Daba dir start l'server b l'port w pass li bghiti:
```bash
./ircserv 6667 mypassword123
```

Bash t-connecti w t-testi l'khedma manually, khdem b `nc` (netcat):
```bash
nc -C 127.0.0.1 6667
```

---

## 2. Server Architecture (L'bniya dyal l'code)

L'server structure follow-ya strictly l'Orthodox Canonical Form (OCF) w separations d'responsabilités:

```text
       Event Loop (poll)
             │
             ├──► POLLIN Event (Readable)
             │      ├──► Server Socket ──► accept() ──► non-blocking client fd
             │      └──► Client Socket ──► recv() ──► BufferManager (gluing lines)
             │                              └──► Parser ──► CommandDispatcher
             │
             └──► POLLOUT Event (Writable)
                    └──► _flushOutgoingData() ──► non-blocking send() from write queue
```

* **Data Layer:** Managed by [`Client`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Client.hpp) w [`Channel`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Channel.hpp). Channels keep operator lists w invite entries.
* **Networking Layer:** Managed in [`Server.cpp`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/Server.cpp) using a single `poll()` call.
* **Control Layer:** Managed in [`CommandDispatcher.cpp`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/CommandDispatcher.cpp) for routing and executing standard IRC commands.

---

## 3. Phases Summary (L'marahil dyal l'khedma)

* **Phase 1: Project Skeleton** — Bneina l'hikal dyal C++98 classes.
* **Phase 2: TCP Server Socket** — Setup dyal sockets, binding address, listen socket, w setsockopt logic (`SO_REUSEADDR`).
* **Phase 3: Client Tracking** — tracking active connections using a mapping table mapping file descriptors (fd) to Client references.
* **Phase 4: Reactor Loop** — Single poll event loop configuration w non-blocking client socket conversion using `fcntl()`.
* **Phase 5: Parser & Authentication** — parsing commands and structuring the user registration state machine (`PASS` → `NICK` → `USER` → `001 RPL_WELCOME`).
* **Phase 6: Channel Commands** — implementation of core IRC chatrooms commands (`JOIN`, `PART`, `PRIVMSG`, `NOTICE`, `TOPIC`, `KICK`, `INVITE`, `MODE` with modes `i`, `t`, `k`, `l`, `o`).
* **Phase 7: Poll-Driven Message Delivery** — complete non-blocking outgoing queue (`_outgoingBuffers`) utilizing `POLLOUT` event checking.

---

## 4. Bilingual Darija/English Code Specs

### Event Loop & Multiplexing (`poll`)
> 🇲🇦 **Darija:** F'had l'server, ma nsta3mlo l'khyout (threads) la process dyal `fork()`. Kolshi ki-khedem safely f single-thread b l'event multiplexer `poll()`. Ay socket (listening socket w clients) kat-kun converted non-blocking b `O_NONBLOCK` flag:
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```
Mli `poll()` alerts readable client fd (`POLLIN`), we safely call `recv()`.

### Buffer & Parsing Layer (Partial reads)
> 🇲🇦 **Darija:** TCP dynamic network streams don't keep boundaries. Ila klyan sift message partitioned (bhal `"NIC"` then `"K ismail\r\n"`), we must hold it. [`BufferManager`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/BufferManager.cpp) stores partial bytes inside client buffers and extracts commands only when `\r\n` boundary is found:
```cpp
std::vector<std::string> lines = bm.extract_complete_lines(); // returns fully assembled commands
```

### Channel Management & Operator Modes
> 🇲🇦 **Darija:** Channels (rooms) are managed in a central map inside Server. L'client li ki-join room lowwel ki-t-promota direct l'Channel Operator (`@` symbol). We support:
* `KICK`: Operator ejects a client from channel.
* `INVITE`: Invite clients to room; registers client in `Channel::_invitedFds`.
* `TOPIC`: View/set topics. If `+t` mode is active, only Operators can update topic.
* `MODE`: Enforces sign status (`+` or `-`) across five flags: `i` (invite-only), `t` (topic restrict), `k` (room key), `l` (limit), `o` (give/take operator status).

### Poll-Driven Outbound Delivery (`POLLOUT`)
> 🇲🇦 **Darija:** Direct `send()` blocks execution and truncates messages under network congestion. In Phase 7, we completely isolated write calls. When we want to send data, we queue it in `_outgoingBuffers[fd]` and enable `POLLOUT` monitoring flag inside `poll()`. Once `poll()` triggers a writable event, we call `send()`. If socket blocks, we return and leave the remaining unsent bytes in the buffer until the next `POLLOUT`.
```cpp
_outgoingBuffers[fd] += message + "\r\n";
_setPollOut(fd, true); // monitor writable event
```

---

## 5. Staged Documentation Reference

For more detailed guides and validation checklists, check our staged docs:
* Detailed Phase summaries w Darija explanations inside [docs/](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/).
* Step-by-step test commands, collision checks, and stress tests inside [testing-guide.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/testing-guide.md).
* Plan for release/maintenance phases inside [phase-08-remaining-phases-plan.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/phase-08-remaining-phases-plan.md).
