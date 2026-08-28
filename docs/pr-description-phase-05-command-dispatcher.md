# PR: Phase 5 — Command Dispatcher & Client Registration

**Branch:** `feature/phase-05-command-dispatcher` → `main`  
**Author:** Souhail (data layer) — integrating infrastructure for Ismail (lead architect)  
**Milestone:** 5 (Parser → Dispatcher) + 6 (Registration: PASS + NICK + USER + 001)

---

## What changed

- Added `CommandDispatcher` class that receives every `ParsedCommand` from
  `_processClientBuffer` and routes it to the correct handler method.
- Added `Replies` namespace with factory functions for all IRC numeric strings
  needed in Phase 5 (`001`–`004`, `421`, `431`–`433`, `461`–`464`).
- Added `Channel` class skeleton with member management, operator tracking,
  topic storage, and a `broadcast()` method ready for Phase 7.
- Extended `Client` with registration fields: `nickname`, `username`,
  `realname`, and `passReceived`, each with a getter and a setter.
- Made `Server::disconnectClient()` public (previously `_disconnectClient`)
  so `CommandDispatcher` can call it from a `QUIT` handler.
- Added `Server::getPassword()` so `CommandDispatcher` can validate `PASS`.
- Replaced the `TODO Phase 5` stub in `_processClientBuffer` with a real
  call to `_dispatcher->dispatch(fd, cmd)` and a post-dispatch safety check.
- Updated `Makefile` to compile `Channel.cpp`, `Replies.cpp`, and
  `CommandDispatcher.cpp`.
- Added `docs/phase-05-command-dispatcher-registration.md` covering the
  architecture, registration flow, handler table, reply reference, and
  step-by-step test procedures.

---

## Why

Phase 4 left a `TODO Phase 5` comment at the end of `_processClientBuffer`.
Parsed commands were being logged but never acted on. This PR closes that gap:

- **Milestone 5** — the server now understands IRC commands (parser feeds
  the dispatcher).
- **Milestone 6** — a client can complete the full `PASS → NICK → USER`
  sequence and receive `001 RPL_WELCOME` plus the `002`–`004` burst,
  which is required for any real IRC client (`irssi`, `hexchat`) to
  consider the connection successful.

Without this, Phase 7 channel commands cannot be implemented because there
is no registration state and no way to route commands.

---

## Files touched

### New files

```
includes/Channel.hpp
includes/Replies.hpp
includes/CommandDispatcher.hpp
src/Channel.cpp
src/Replies.cpp
src/CommandDispatcher.cpp
docs/phase-05-command-dispatcher-registration.md
```

### Modified files

```
includes/Client.hpp    — registration fields added (nick, user, real, passReceived)
includes/Server.hpp    — CommandDispatcher*, getPassword(), public disconnectClient()
src/Client.cpp         — new getters and setters for all registration fields
src/Server.cpp         — dispatcher init/destroy, _processClientBuffer wired up
Makefile               — 3 new source files added to SRCS
```

---

## How it was tested

```bash
# Full build — zero warnings, zero errors
make re

# Test 1: successful registration
./ircserv 6667 testpass &
printf "PASS testpass\r\nNICK ismail\r\nUSER ismail 0 * :Ismail Test\r\n" \
  | nc -q 2 127.0.0.1 6667
# Expected output (received by nc):
#   :ircserv 001 ismail :Welcome to the IRC Network, ismail!ismail@127.0.0.1
#   :ircserv 002 ismail :Your host is ircserv, running version ft_irc-1.0
#   :ircserv 003 ismail :This server was created today
#   :ircserv 004 ismail ircserv ft_irc-1.0 o o

# Test 2: wrong password
printf "PASS wrongpass\r\nNICK ismail\r\nUSER ismail 0 * :Test\r\n" \
  | nc -q 1 127.0.0.1 6667
# Expected: :ircserv 464 * :Password incorrect
# Registration does NOT complete (passReceived stays false)

# Test 3: partial data / TCP fragmentation
# nc -C 127.0.0.1 6667
# (type) PASS te  <Ctrl+D>
# (type) stpass<ENTER>
# BufferManager reassembles across recv() calls — PASS is accepted correctly

kill %1
```

---

## Notes for reviewers

- **Start with `Server.hpp`** — it shows the two new public methods
  (`disconnectClient`, `getPassword`) that changed the public interface.
  Both were agreed upon as part of the Phase 5 architecture, but confirm
  you are comfortable with them being public before approving.

- **`CommandDispatcher.hpp`** owns the handler declarations. Any Phase 7
  handler (JOIN, PRIVMSG, etc.) will be added here as a private method.
  The class holds a `Server&` reference — no ownership, no memory to free.

- **`Client.hpp`** changes are backward-compatible. Existing callers
  (`Server`, event loop) continue to work. The new fields simply start
  empty/false and are set by `CommandDispatcher` handlers.

- **`Channel.cpp`** is a skeleton only. The `broadcast()` method writes
  directly to the fd with `send()`. Phase 7 will route through
  `Server::sendToClient()` for a proper send queue — this is noted in the
  code with a comment.

- The private `_disconnectClient` rename to `disconnectClient` (public)
  is the only **interface change** in this PR. It was necessary so
  `CommandDispatcher` can close the socket on `QUIT` without coupling
  the dispatcher to poll internals.

- **Squash merge** is recommended for this PR to keep `main` history clean.
  The three commits (infrastructure / integration / docs) are intentional
  but not individually meaningful after merge.

---

## Integration checklist (from team guide)

```text
✔ Compiles       — make re: zero errors, zero warnings (-Wall -Wextra -Werror)
✔ C++98          — no auto, no nullptr, no range-based for, no lambdas
✔ No leaks       — _dispatcher deleted in Server destructor; Channel does
                   not own Client pointers
✔ Non-blocking   — no new blocking calls; dispatcher runs inside poll loop
✔ poll() works   — dispatch() called only from _processClientBuffer (POLLIN)
✔ Interfaces     — disconnectClient() rename discussed; all other interfaces
                   are backward-compatible
✔ OCF            — Channel, Replies (namespace, no state), CommandDispatcher
                   all satisfy Orthodox Canonical Form requirements
✔ No globals     — static _shutdown already existed from Phase 4; nothing new
✔ Documentation  — docs/phase-05-command-dispatcher-registration.md added
✔ Reviewed       — requesting review from Ismail (Server/Client owner)
```
