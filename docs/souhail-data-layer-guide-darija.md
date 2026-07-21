# 📦 Dalil l-Integration dyal Data Processing Layer (BufferManager & Parser)

**L-Khatib:** Souhail  
**Mowajah l:** Ismail (Lead Architect) & Ayoub (Network Layer)  
**Branch:** `feature/buffer_management`

---

## 1. Nadhra Aamma (Overview)

Had l-wathiqa kat-shrah kifach kaykhdmo **BufferManager** wa **Parser** li bnit, wa kifach Ismail wa Ayoub yqdro y-integrer-hom m3a l-Server wa Client b kol sohoula.

L-goal dyal had l-module:
1. **BufferManager**: Kay-jma3 l-bytes li jayin mn `recv()` hit TCP kay-qssam l-messages (fragmentation). Kay-khraj ghi l-lines kmlin li fihom `\r\n`.
2. **Parser**: Kay-yakhd l-line d string wa kay-parsiha l `ParsedCommand` struct fih command, params, wa trailing text.

---

## 2. L-Joz' L-Awwal: `BufferManager`

### L-Fichierat
- Header: `includes/BufferManager.hpp`
- Implementation: `src/BufferManager.cpp`

### Kifach katkhdem fi l-code

```cpp
#include "BufferManager.hpp"

BufferManager bm;

// 1. Mli kay-connecter client jdid:
bm.add_client(fd);

// 2. Mli kay-wsal l-data f recv(fd, buf, len, 0):
std::string raw_chunk(buf, bytes_read);
bm.append_data(fd, raw_chunk);

// 3. Kat-khrj l-lines kmlin li fihom \r\n (aw \n):
std::vector<std::string> lines = bm.extract_complete_lines(fd);

for (size_t i = 0; i < lines.size(); ++i)
{
    // Kat-passi l-line l Parser...
}

// 4. Mli kay-deconnecter l-client:
bm.remove_client(fd);
```

### Moomayyizat
- **Aman mn TCP fragmentation**: Ila l-message ja mqssam (f7al `"NICK "` fi loop wa `"souhail\r\n"` fi loop khor), BufferManager kay-jma3hom wa makay-khrjhoms 7ta kay-kmlo b `\r\n`.
- **Himaaya mn Overflow**: Ila l-buffer fat 512 bytes bla `\r\n`, BufferManager kay-qssam l-line bach y-7mi l-memory mn DoS attack.
- **Mustaqill (Standalone)**: Makay-3tamadch 3la `Server` aw `Client` classes.

---

## 3. L-Joz' L-Thani: `Parser` wa `ParsedCommand`

### L-Fichierat
- Headers: `includes/Parser.hpp`, `includes/ParsedCommand.hpp`
- Implementation: `src/Parser.cpp`

### L-Binya dyal `ParsedCommand`

```cpp
struct ParsedCommand
{
    std::string              prefix;       // masalan "nick!user@host" (ila kayn)
    std::string              command;      // masalan "PRIVMSG", "NICK", "JOIN" (dima UPPERCASE)
    std::vector<std::string> params;       // list dyal l-parameters kmlin
    std::string              trailing;      // masalan "Hello team!" (l-text li mn b3d ':')
    std::string              raw_line;      // l-line l-asliya
    bool                     has_prefix;    // true ila kayn ':' prefix
    bool                     has_trailing;  // true ila kayn ':' trailing param
    bool                     is_valid;      // true ila l-command t-parsat mzyan
};
```

### Kifach katkhdem fi l-code

```cpp
#include "Parser.hpp"

Parser parser;
ParsedCommand cmd = parser.parse_line(line);

if (!cmd.is_valid)
    return; // Ignorer l-lines khawyin aw l-ghalatin

if (cmd.command == "NICK")
{
    std::string new_nick = cmd.params[0];
    // Khdem l-logic dyal NICK...
}
else if (cmd.command == "PRIVMSG")
{
    std::string target = cmd.params[0];
    std::string text   = cmd.trailing; // aw cmd.params[1]
    // Khdem l-logic dyal PRIVMSG...
}
```

---

## 4. Kifach Ismail y-integrer hadshi f `Server::run()`

```cpp
// F-l-main loop mli recv() kay-rje3 bytes > 0:
char buf[1024];
ssize_t bytes = recv(clientFd, buf, sizeof(buf), 0);

if (bytes > 0)
{
    _bufferManager.append_data(clientFd, std::string(buf, bytes));
    std::vector<std::string> lines = _bufferManager.extract_complete_lines(clientFd);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        ParsedCommand cmd = _parser.parse_line(lines[i]);
        if (cmd.is_valid)
        {
            _dispatcher.dispatch(clientFd, cmd);
        }
    }
}
else if (bytes == 0)
{
    // Clean up l-client mli kay-disconnecter
    _bufferManager.remove_client(clientFd);
}
```

---

## 5. Kifach t-tester l-code

```bash
# Compile project:
make re

# Run unit tests dyal BufferManager wa Parser (kttb l-amr f khatt wa7ed):
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes tests/test_data_layer.cpp src/BufferManager.cpp src/Parser.cpp -o test_runner && ./test_runner
```
