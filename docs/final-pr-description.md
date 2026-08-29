# PR: Final ft_irc Server Implementation 🌐

This Pull Request connects all development dots, integrating the final networking, buffer assembly, protocol parsing, and channel operators subsystem into a fully compliant C++98 `ft_irc` server. 

---

## 📑 Core Subsystems Implemented

### 1. Multiplexed Event Loop (Reactor Pattern)
* **Single Thread / Process:** Zero threads (`pthread`) or child processes (`fork()`) are used, respecting evaluation rules.
* **Non-Blocking I/O:** Sockets are put into non-blocking mode via `fcntl(fd, F_SETFL, O_NONBLOCK)`. No read/write operation is called unless prompted by `poll()`.
* **Multiplexing:** Single event loop monitoring both `POLLIN` (incoming connections/data) and `POLLOUT` (outgoing writable buffers) for all file descriptors.

### 2. Stream Reassembly & Buffer Layer
* **Fragmented TCP Input:** [`BufferManager`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/BufferManager.cpp) holds partially read bytes and aggregates them in client buffers, only flushing complete lines delimited by `\r\n` to the parser.
* **Non-Blocking Send Queue:** To avoid server hangs on slow clients, [`Server::sendToClient()`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/Server.cpp#L752) queues outgoing bytes in [`_outgoingBuffers`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/includes/Server.hpp#L137) and sets the `POLLOUT` monitoring bit. Data is flushed asynchronously without server blocks.

### 3. Protocol Parsing & Dispatching
* **IRC Commands Parsing:** Parses commands (case-insensitive command strings, parameter vectors, and `:` trailing parameters).
* **Command Dispatching:** Routes parsed entities to standard command handlers inside [`CommandDispatcher.cpp`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/CommandDispatcher.cpp).
* **Registration Flow:** Implements sequential `PASS` → `NICK` → `USER` validation order before dispatching welcome reply bursts (`001`-`004`).

### 4. Channel Management & Access Control
* **Channel Life Cycle:** Dynamically allocates and deletes channels in `Server::_channels` (cleaned up when empty). Promotes room creators to operators (`@`).
* **Operator Controls:** Enforces permissions checks on `KICK`, `INVITE`, `TOPIC`, and `MODE`.
* **Channel Modes:** Full configuration and validation support for:
  - `i` (Invite-only channel)
  - `t` (Topic changes restricted to operators)
  - `k` (Channel key/password verification)
  - `l` (Maximum members limit check)
  - `o` (Operator privileges toggles)

---

## 🔒 Verification & Compliance Checklist

```text
✔ Compiles cleanly  — make re: compiles with -Wall -Wextra -Werror -std=c++98.
✔ No leaks          — Checked with Valgrind (definitely lost: 0 bytes). 
✔ Signal Handling   — SIGINT caught safely to perform clean shutdowns of all sockets and maps.
✔ Edge-Cases Ok     — Prevents skipped events during disconnect (index shifting fixes).
                      Enforces case-insensitive nickname comparisons with special character mapping.
```

---

## 📑 Staged Documentation List
All files are staged in Git under the `docs/` and `lessons/` directories:
1. **[phase-06-channel-commands.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/phase-06-channel-commands.md):** Guides on channel management logic and modes.
2. **[phase-07-poll-driven-message-delivery.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/phase-07-poll-driven-message-delivery.md):** Architectural guide to non-blocking write queues.
3. **[testing-guide.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/testing-guide.md):** Step-by-step testing manual using `nc` and stress cases.
4. **[phase-08-remaining-phases-plan.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/docs/phase-08-remaining-phases-plan.md):** Plan for final phases and OCF validations.
5. **[lesson-03-phases-theory-and-practice.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/lessons/lesson-03-phases-theory-and-practice.md):** Lessons on networking theories and practical implementations.
6. **[lesson-03-bugs-and-fixes.md](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/notes/03-bugs-and-fixes/lesson-03-bugs-and-fixes.md):** Notes documenting solved bugs.
