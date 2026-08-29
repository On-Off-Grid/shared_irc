# 📚 Lesson 3: Project Bugs & Fixes History 🐛

This lesson catalogs the critical bugs encountered during the development and integration of the `ft_irc` server, retrieved from the git logs, PRs, and commit history. It details the root causes, how they affected server stability, and the exact solutions implemented.

---

## 📑 Table of Contents

1. [Bug 1: Makefile Rebuild Safety (Implicit Header Dependencies)](#bug-1-makefile-rebuild-safety-implicit-header-dependencies)
2. [Bug 2: Event Loop Index-Shifting & Vector Mutation (POLLHUP/POLLERR)](#bug-2-event-loop-index-shifting--vector-mutation-pollhuppollerr)
3. [Bug 3: Nickname Case-Mapping Comparison (RFC 2812 Standard)](#bug-3-nickname-case-mapping-comparison-rfc-2812-standard)
4. [Bug 4: Registration Order Validation & Welcome Burst Trigger](#bug-4-registration-order-validation--welcome-burst-trigger)
5. [Bug 5: Synchronous Write Congestion & Message Truncation](#bug-5-synchronous-write-congestion--message-truncation)

---

## Bug 1: Makefile Rebuild Safety (Implicit Header Dependencies)

### 1. The Problem
Initially, the [Makefile](../../Makefile) only tracked changes to `.cpp` source files. If a developer modified a header file (e.g. adding a new field to `Client.hpp` or updating `Server.hpp`), `make` would report `make: 'ircserv' is up to date` instead of re-compiling. Developers had to run `make re` manually, leading to compiled objects mismatching class layout definitions and causing runtime segment faults.

### 2. The Solution
We added compiler flags `-MMD -MP` to automatically generate dependency file lists (`.d` files) tracking all included headers for each translation unit, and included them via `-include` in the Makefile.

**Makefile Diff:**
```make
 CXX			= c++
-CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
+CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -MMD -MP
...
 OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
+DEPS		= $(OBJS:.o=.d)
...
-clean:
-			rm -rf obj
+clean:
+			rm -rf obj
+
+-include $(DEPS)
```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Mli kano kaysho khedamin f header (.hpp) w ibdlo fih, Makefile kan may-compilihsh hit kiy-check ghir modifications dyal (.cpp). Hadshi kan kiy-creer segment faults hit parts dyal program bqaw qdam. Zdna flags `-MMD -MP` w included files `.d` bash kolshi i-compile safely automatic mli header ibdel.

---

## Bug 2: Event Loop Index-Shifting & Vector Mutation (POLLHUP/POLLERR)

### 1. The Problem
When a client disconnected, `disconnectClient()` called `_removePollFd(fd)`. The initial removal logic swap-erased (swapped the element with the last index and popped it in $O(1)$ complexity) to avoid shifting. 
However, because this deletion occurred directly inside the active `for (size_t i = 0; i < _pollfds.size(); ++i)` iteration loop, shrinking the vector shifted the last client's `pollfd` into the current index `i`. Unconditional loop increment (`++i`) resulted in:
1. **Skipping Events**: The swapped client socket was skipped and never checked in that loop iteration.
2. **Out of bounds access** or double processing in concurrent events.

### 2. The Solution
Instead of performing an immediate swap-and-pop inside the iteration loop, we now perform a **deferred cleanup**:
1. Mark the disconnected client's FD as `-1` inside `_removePollFd()`.
2. Post-loop, run a safe garbage-collection pass that swap-erases all FDs marked as `-1`.

**Event Loop Cleanup Diff:**
```cpp
 void	Server::_removePollFd(int fd)
 {
 	for (size_t i = 0; i < _pollfds.size(); ++i)
 	{
 		if (_pollfds[i].fd == fd)
 		{
-			/* Swap with last element, then pop (O(1) removal) */
-			_pollfds[i] = _pollfds[_pollfds.size() - 1];
-			_pollfds.pop_back();
+			_pollfds[i].fd = -1;
+			_pollfds[i].revents = 0;
 			return ;
 		}
 	}
 }
```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Mli client kan kiy-disconnecta, server kan kiy-mseh l'FD dialo direct mn vector `_pollfds` f nest loop. Vector size kantsghar, w klyan akhor ki-swap l'bla7t l'mmsou7, w `++i` kan kiykhllina n-tanqzo check dial had klyan jdid. L'7el kan deferred cleanup: kan-rddo fd = -1, w mli itsala loop, kankhwiw vector safely post-loop.

---

## Bug 3: Nickname Case-Mapping Comparison (RFC 2812 Standard)

### 1. The Problem
In the Phase 5 prototype, nickname checking used direct string comparison (`it->second->getNickname() == nick`).
However, the IRC protocol states that nickname comparisons must be case-insensitive, and further defines a specific case-mapping scheme (RFC 2812 §2.2) where the characters `[`, `]`, `\`, and `^` are the uppercase equivalents of `{`, `}`, `|`, and `~` respectively. Because of this, a user could register as `Alice[bot]` and another as `alice{bot}`, leading to nickname registration clashes and parsing errors.

### 2. The Solution
We implemented a custom case-mapping comparator `_areNicknamesEqual` that correctly applies standard IRC rules:

```cpp
static bool _areNicknamesEqual(const std::string& n1, const std::string& n2)
{
	if (n1.size() != n2.size())
		return false;
	for (size_t i = 0; i < n1.size(); ++i)
	{
		char c1 = n1[i];
		char c2 = n2[i];

		if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
		if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';

		if (c1 == '[') c1 = '{';
		else if (c1 == ']') c1 = '}';
		else if (c1 == '\\') c1 = '|';
		else if (c1 == '^') c1 = '~';

		if (c2 == '[') c2 = '{';
		else if (c2 == ']') c2 = '}';
		else if (c2 == '\\') c2 = '|';
		else if (c2 == '^') c2 = '~';

		if (c1 != c2)
			return false;
	}
	return true;
}
```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** IRC nicknames khasshom ikono case-insensitive, w ktr mn hadshi, brackets bhal `[` w `{` ki-t3tabro hashom equivalents. Direct check `==` khlla collisions i-kono momkin. Dernalha helper class case-mapping correct match per RFC standards.

---

## Bug 4: Registration Order Validation & Welcome Burst Trigger

### 1. The Problem
A client's registration is complete when they have successfully sent: `PASS` (valid) + `NICK` + `USER`. 
In early iterations, the registration check `_tryCompleteRegistration()` was only wired to trigger inside `NICK` and `USER` command handlers. If a client sent a sequence out of typical order (e.g. `NICK`, `USER`, then `PASS`), the registration check was never executed, leaving the client in a half-authenticated state.

### 2. The Solution
We wired the registration evaluation check directly inside `_handlePass()` to ensure successful authentication regardless of command execution order:

```cpp
void CommandDispatcher::_handlePass(int fd, Client* client, const ParsedCommand& cmd)
{
    ...
    if (cmd.params[0] == _server.getPassword())
    {
        client->setPassReceived(true);
        _tryCompleteRegistration(fd, client);
    }
    ...
}
```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Mli l'client kan ki-sift commands out of order (NICK, USER, 3ad PASS), welcome burst (001) kante makat-tlançash hit welcome check makansh m-wire-in f handle PASS. Zednah direct tma.

---

## Bug 5: Synchronous Write Congestion & Message Truncation

### 1. The Problem
When a channel operator kicked or broadcasted a message to 20 users, the server called `send()` immediately on each client socket.
If a client had a slow connection, the OS transmit buffer was full, and `send()` returned `-1` with `EAGAIN` / `EWOULDBLOCK`. The early codebase dropped these remaining bytes, leading to corrupt or truncated message streams on the client side, causing clients to desynchronize.

### 2. The Solution
We refactored write handling into a **poll-driven write buffer system** (Phase 7):
1. `sendToClient()` appends outgoing payloads into `_outgoingBuffers[fd]` and enables the `POLLOUT` event flag.
2. In the main loop, if `POLLOUT` triggers, `_flushOutgoingData(fd)` is called.
3. If write blocks (`EAGAIN`), it gracefully exits, waiting for the next `POLLOUT` to resume the write queue.

**FLUSH logic snippet:**
```cpp
ssize_t bytesSent = send(fd, data.c_str(), data.size(), 0);
if (bytesSent == -1)
{
    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return; // wait for next writable event
    ...
}
data.erase(0, bytesSent); // keep remainder
```

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot:** Mli kan y-trssal message l'bezah dial users, raw `send()` direct kan kiy-blocka f low network w data dyal messages kanet kat-tqa6a3 w da3 (truncate). L'7el f Phase 7 hya l'outbound queues `_outgoingBuffers` driven by `POLLOUT` events f event loop.
