# Pull Request: Implement BufferManager, RFC 1459 Parser & Unit Tests

**Source Branch:** `feature/buffer_management`  
**Target Branch:** `main`  
**Author:** Souhail  
**Reviewers:** Ismail (@LeadArchitect), Ayoub (@NetworkLayer)

---

## ## What changed
- **Implemented `BufferManager` (`includes/BufferManager.hpp`, `src/BufferManager.cpp`)**:
  - Accumulates incoming raw bytes per socket file descriptor (`fd_t`).
  - Solves TCP fragmentation by extracting complete lines delimited by `\r\n` (CRLF) or `\n`.
  - Added line length overflow protection (truncates un-terminated buffers exceeding 512 bytes per RFC 1459).
- **Implemented `Parser` & `ParsedCommand` (`includes/Parser.hpp`, `includes/ParsedCommand.hpp`, `src/Parser.cpp`)**:
  - Parsed raw IRC lines into `ParsedCommand` structures.
  - Added optional prefix parsing (`:prefix`).
  - Added uppercase normalization for command tokens (IRC commands are case-insensitive).
  - Extracted space-separated parameters and trailing text (`:trailing parameter`).
- **Updated `Makefile`**:
  - Added `src/BufferManager.cpp` and `src/Parser.cpp` to build targets.
- **Created Unit Test Harness (`tests/test_data_layer.cpp`)**:
  - Verified chunked data processing, multi-line TCP packets, and command parsing logic.
- **Added Teammate Documentation (`docs/`)**:
  - `docs/souhail-data-layer-guide-en.md` (English integration guide).
  - `docs/souhail-data-layer-guide-darija.md` (Darija integration guide in Latin script).
  - `docs/team-collaboration-guide-en.md` (Team collaboration handbook).
- **Added `.gitignore`**:
  - Ignores binary outputs (`ircserv`, `test_runner`) and object files (`obj/`).

---

## ## Why
- Completes Souhail's primary assigned deliverables for Phase 4 (Buffers & Parsing) as outlined in `docs/team-collaboration-guide-en.md`.
- Provides Ismail (Lead Architect) with stable, decoupled data structures to connect socket `recv()` events directly to the command dispatcher.

---

## ## Files touched
- `includes/BufferManager.hpp`
- `includes/ParsedCommand.hpp`
- `includes/Parser.hpp`
- `src/BufferManager.cpp`
- `src/Parser.cpp`
- `Makefile`
- `.gitignore`
- `tests/test_data_layer.cpp`
- `docs/souhail-data-layer-guide-en.md`
- `docs/souhail-data-layer-guide-darija.md`
- `docs/team-collaboration-guide-en.md`

---

## ## How it was tested
1. **Compilation**:
   `make re` built `ircserv` cleanly with zero warnings (`-Wall -Wextra -Werror -std=c++98`).
2. **Unit Tests**:
   Ran `c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_runner && ./test_runner`
   - Verified 100% test pass rate for chunked TCP data, CRLF extraction, parameter parsing, and prefix extraction.

---

## ## Notes for reviewers
- **For Ismail**: Check `docs/souhail-data-layer-guide-en.md` for example code showing how to call `_bufferManager.append_data()` and pass extracted lines to `_parser.parse_line()` inside your `Server::run()` loop.
- **For Ayoub**: `BufferManager` handles partial data assembly cleanly; you can pass whatever bytes `recv()` returns directly to `append_data(fd, chunk)`.
