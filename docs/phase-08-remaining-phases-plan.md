# Phase 8: Plan for Remaining Phases 🚀

With the mandatory requirements (Phases 1 through 7) fully implemented, compiled, and tested, we can now plan the final sprint of the `ft_irc` project. This plan covers the optional bonus requirements (Bot & File Transfer) and the final preparation for peer-evaluation/defense.

---

## 📑 Table of Contents

1. [Phase 8: IRC Bot (Bonus)](#phase-8-irc-bot-bonus)
2. [Phase 9: DCC File Transfer Support (Bonus)](#phase-9-dcc-file-transfer-support-bonus)
3. [Phase 10: Performance, Valgrind, & Defense Prep](#phase-10-performance-valgrind--defense-prep)
4. [Timeline & Work Distribution](#timeline--work-distribution)

---

## Phase 8: IRC Bot (Bonus)

To satisfy the bot requirement, we will implement an automated, interactive bot that acts as a standard IRC client. 

### 1. Architectural Choice: External Bot vs. Built-in Bot
* **Rejected (Built-in)**: Hardcoding the bot logic directly inside the server executable. This violates IRC's client-server architecture and makes the server code unnecessarily bloated.
* **Approved (External Client)**: Writing a standalone C++ program (or script) that establishes a TCP socket connection, registers as a normal client (e.g. Nickname `IRCBot`), joins channels, and responds to messages.

### 2. Features of the Bot
* **Interactive Command Triggering**: Listens to messages starting with specific prefixes (e.g. `!help`, `!time`, `!joke`, `!roll`).
* **Privmsg Parsing**: Parses incoming private messages or channel messages.
* **Auto-Welcome**: Greets users who join channels that the bot is currently in.

### 3. Implementation Details
* Use C++98 socket programming (socket, connect, send, recv).
* Establish a simple parse loop to look for incoming `PRIVMSG` target commands.
* If a trigger is found, send a `PRIVMSG` reply back to the channel or user.

---

## Phase 9: DCC File Transfer Support (Bonus)

The second bonus requirement is supporting file transfers. In the IRC protocol, file transfers are handled using **DCC (Direct Client-to-Client)**.

### 1. How DCC Works
* Sockets for file transfers are **NOT** routed through the server (this would consume server bandwidth and slow down messaging).
* Instead, the server acts as a broker/signaler. 
* Client A tells Client B: *"I want to send you file X. I am listening on IP Y and Port Z."* This signaling message is sent using a standard `PRIVMSG` with a special **CTCP (Client-To-Client Protocol)** payload.

### 2. Server Requirements
* The server must support the transmission of CTCP messages (which are delimited by ASCII `0x01` characters).
* The parser must not corrupt, trim, or drop binary headers or messages with `0x01` delimiters.
* Example DCC handshake message sent as a PRIVMSG:
  ```irc
  PRIVMSG Bob :\x01DCC SEND sample.txt 2130706433 5001 1024\x01
  ```
  *(Where `2130706433` is the decimal representation of IP `127.0.0.1`, `5001` is the port, and `1024` is the filesize).*

### 3. Testing Implementation
* We will write a small Python/C++ script or use an actual IRC client (like HexChat) to initiate a DCC transfer through our server to confirm that the CTCP handshake successfully goes through.

---

## Phase 10: Performance, Valgrind, & Defense Prep

This is the cleanup and verification phase before the final evaluation.

### 1. Memory Analysis (Valgrind)
* Run the server under Valgrind to monitor for leaks:
  ```bash
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./ircserv 6667 pass
  ```
* Perform the following actions and monitor memory usage:
  - Register clients, disconnect them.
  - Join multiple channels, kick users, leave channels (causing empty channel deletion).
  - Terminate the server using `SIGINT` (Ctrl+C). Confirm all clients, channels, and outgoing buffers are deleted properly.

### 2. Stress & Resource Exhaustion Testing
* Run a script that opens 100+ concurrent connections to ensure the server handles multiplexing cleanly without crashing or using 100% CPU.
* Verify index-shifting prevention: Ensure that disconnecting client `i` in the middle of processing the `pollfd` vector does not result in skipping client `i+1` or accessing out-of-bound indices.

### 3. Orthodox Canonical Form (OCF) Review
* Verify that all major classes (`Server`, `Client`, `Channel`, `BufferManager`, `Parser`, `CommandDispatcher`) strictly implement:
  - Default constructor
  - Copy constructor
  - Copy assignment operator
  - Destructor
* In classes where copying is not allowed (e.g. `Server` or `Channel`), make the copy constructor and copy assignment operator `private` to prevent incorrect copying.

---

## Timeline & Work Distribution

| Phase | Task | Primary Owner | Target Duration |
|---|---|---|---|
| **Phase 8** | Standalone C++ IRC Bot | Ismail | 2 Days |
| **Phase 9** | CTCP parsing check & DCC signaling verification | Souhail | 1 Day |
| **Phase 10** | Valgrind, Stress-Testing, and OCF Verification | Ayoub | 2 Days |
| **Defense** | Dry-run evaluations and mock interviews | All | 1 Day |
