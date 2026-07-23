# Pull Request: Refactor BufferManager to be owned by Client

**Source Branch:** `refactor/buffer-client-integration`  
**Target Branch:** `main`  
**Author:** Souhail  
**Reviewers:** Ismail (@LeadArchitect), Ayoub (@NetworkLayer)

---

## What changed
- **Refactored `BufferManager` (`includes/BufferManager.hpp`, `src/BufferManager.cpp`)**:
  - Removed all references to `fd_t` (socket file descriptor) and the internal `std::map<fd_t, std::string> _buffers`.
  - Simplifed class to manage a single `std::string` buffer representing one client's buffer.
  - Updated methods (`append_data`, `extract_complete_lines`, etc.) to operate parameterless on the single local buffer.
- **Integrated inside `Client` (`includes/Client.hpp`, `src/Client.cpp`)**:
  - Added `BufferManager _buffer` as a private member attribute inside the `Client` class.
  - Exposed `BufferManager& getBuffer()` and `const BufferManager& getBuffer() const` to retrieve the buffer for network operations.
- **Updated Unit Tests (`tests/test_data_layer.cpp`)**:
  - Adapted the `test_buffer_manager` function to match the new single-client API.
- **Updated Documentation**:
  - Revised both the English integration guide (`docs/souhail-data-layer-guide-en.md`) and the Darija guide (`docs/souhail-data-layer-guide-darija.md`) to reflect the new API and integration workflow.

---

## ## Why
- Aligns the buffer management with the team collaboration handbook (`docs/team-collaboration-guide-en.md`), which requires `BufferManager` to work as a member of each `Client` instance.
- Prevents redundant socket file descriptor mappings inside individual client objects.
- Improves code modularity and conforms strictly to the separation of concerns.

---

## ## Files touched
- `includes/BufferManager.hpp`
- `includes/Client.hpp`
- `src/BufferManager.cpp`
- `src/Client.cpp`
- `tests/test_data_layer.cpp`
- `docs/souhail-data-layer-guide-en.md`
- `docs/souhail-data-layer-guide-darija.md`
- `docs/pr-description-refactor-buffer-client-integration.md` (this file)

---

## ## How it was tested
1. **Compilation**:
   `make re` compiles the entire project with zero warnings/errors (`-Wall -Wextra -Werror -std=c++98`).
2. **Unit Tests**:
   Ran the data processing test suite from the terminal:
   `c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_runner && ./test_runner`
   - Verified 100% test pass rate for chunked TCP data and command extraction on a single buffer.

---

## ## Notes for reviewers
- **For Ismail**: Note the new getters `client->getBuffer()` in `Client.hpp`. When processing incoming events inside `Server::run()`, fetch the client and append directly to its buffer using `client->getBuffer().append_data(raw_chunk)`.
- **For Ayoub**: No changes to the socket layer are needed.
