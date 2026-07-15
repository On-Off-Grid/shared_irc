# Team Partition Extended - Darija

# Big picture (taswira l3amma) w plan dyal 13 nhar

B7sab ta9simtkom l7aliya, had plan dyal 13 nhar wa9i3i:

- **Day 1–2**
  - Ttafe9o 3la l'architecture.
  - Ttafe9o 3la interfaces.
  - Wjdo l'environnement.
  - Wjdo Makefile.
  - (Preparation dyal Phase 2.)

- **Day 3–5**
  - Phase 2.
  - Integration dyal server foundation.
  - Server y9der ystarti.
  - Yaccepti connections.
  - Ylogi les connections.

- **Day 6–8**
  - Phase 3.
  - Integration dyal client management.
  - Clients yregisteriw b:
    - PASS
    - NICK
    - USER
  - Disconnect ykoun clean.
  - Buffers ykouno stable.
  - Parsing ykoun stable.

- **Day 9–11**
  - Channels.
  - Modes.
  - Operator commands.
  - Error replies.
  - Robustness (robustesse).

- **Day 12–13**
  - Testing bzzaf:
    - nc
    - IRC clients
  - Leak checks.
  - Isla7 edge cases (cas limites).
  - Documentation.

Kol meetup nhari:

(15–30 min)

Kol member khaso ygol:

1. Chno kml.
2. Chno tbeddel f interfaces.
3. Chno kaybloquih w momkin y2etter 3la l'akhorin.

# Shared foundations (l2osas lmochtaraka) l kolchi

9bel ma tbda coding b3om9, khaskom kolkom ttaf9o 3la chi 9rara mochtaraka bach lkhedma tb9a interconnected (interconnectée) w merges ykouno sahlin.

## Common headers w namespaces

- Ttaf9o 3la smiyat dyal files:
  - `Server.hpp`
  - `Client.hpp`
  - `Channel.hpp`
  - `SocketManager.hpp`
  - `BufferManager.hpp`
  - `Logger.hpp`
  - `Utils.hpp`

- Ttaf9o 3la basic types:
  - `typedef int fd_t;`
  - enums dyal client state
  - error codes

Bach kolchi yst3mel nafs vocabulary.

## Command w message model

- Ttaf9o 3la structure sghira smitha `ParsedCommand` li ghadi tkhrejha parser dyal Souhail, w ghadi yst3mlha command dispatcher dyal Ismail.

Hiya fih:

- command
- params
- raw_line

- Ttaf9o chkon ghadi ykon owner dyal mapping mn command string (chaine de commande) b7al:

- `"JOIN"`
- `"PRIVMSG"`

l handler functions.

## Error w logging convention

- Ttaf9o 3la log levels:

- DEBUG
- INFO
- WARN
- ERROR

w format wa7ed:

`[timestamp] [level] [component] message`

bach logging ykoun mowa7ad.

- Ttaf9o kifach ghadi t3amlo m3a:

- fatal errors
- non-fatal errors

wach b:

- return codes
- exceptions
- aw tseddo client.

# Phase 2 – Server Foundation (Boot, Configuration, Main Loop)

## Ismail – Architecture dyal Server w Lifecycle

Khedmt Ismail hiya lli ghadi t7edded kifach ga3 code l'akhor ghadi yerkeb. Khasso yfocusi 3la classes m9smin mzyan w control flow (flux de contrôle) ykoun wazi7.

### Detailed tasks

### Requirements digestion (fahm requirements) w sketch

- 9ra subject dyal `ft_irc` mzyan.
- Kteb résumé sghir dyal entities l2asasiya:
  - `Server`
  - `Client`
  - `Channel`
  - mumkin `CommandDispatcher`
  - `SocketManager`

- Rsem diagram basit kaybeyen chkon kaymlek chno.

Exemple:

- `Server` kaymlek:
  - `std::map<fd_t, Client>`

w kaytaleb mn:

- `SocketManager`

bach yupdate `pollfd list`.

---

### Public interface dyal Server

Dir class:

```cpp
class Server
```

fih methods b7al:

```cpp
bool init(int port, const std::string &password);
void run();
void shutdown();
```

Zid hooks lli ghadi y3ayto lihom modules l'akhorin:

```cpp
onNewConnection(fd_t fd)

onClientData(fd_t fd, const std::string &line)

onClientDisconnect(fd_t fd)
```

Had methods homa **contract (contrat)** bin Ismail, Ayoub w Souhail.

---

### Configuration w CLI handling

Implement parsing dyal:

```bash
./ircserv <port> <password>
```

Khasso:

- yverifia port
- yverifia password ma tkounch khawya
- ila invalid, ykherrej error wazi7 w ysali programme.

Dir struct:

```cpp
struct ServerConfig
{
    int port;
    std::string password;
};
```

bach components l'akhorin y9dro yst3mloh.

---

### Main lifecycle w integration m3a poll()

Design lmain loop.

Exemple:

```cpp
while (running)
{
    socketManager.pollOnce(*this);
}
```

T9arrer fin ghadi t7andle:

- SIGINT

bach server ytsed b graceful shutdown (arrêt propre) bla ma ykesser non-blocking.

---

### Chno khas Ismail ylookup

- IRC registration flow:

```
PASS
↓

NICK
↓

USER
↓

REGISTERED
```

- Reactor Pattern
- Single-process event loop
- C++98 architecture

---

# Ayoub – Socket Layer w Non-blocking Setup

Code dyal Ayoub howa lbab bin Operating System w Server.

Khasso ykoun simple, stable w fiable.

---

## Detailed tasks

### Listening socket setup

Implement helper b7al:

```cpp
fd_t createListeningSocket(int port)
```

b:

- socket()
- getaddrinfo()
- bind()
- listen()

m3a error checking kaml.

Configure:

```cpp
SO_REUSEADDR
```

w dir:

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK)
```

bach socket ywalli non bloquant.

---

### poll() infrastructure

Dir class:

```cpp
SocketManager
```

katmlek:

```cpp
std::vector<pollfd>
```

lli fih:

- listening socket
- clients

Methods:

```cpp
addClientFd()

removeFd()

setWritable()
```

Bach 7tta wa7ed ma y3eddel direct f pollfd.

---

### Single entry point dyal event loop

Implement:

```cpp
pollOnce(Server &server)
```

Hiya khas-ha:

### 1.

T3ayet:

```cpp
poll()
```

### 2.

Ila listening socket fih:

```
POLLIN
```

dir:

```cpp
accept()
```

↓

non-blocking

↓

server.onNewConnection()

---

### 3.

Ila client fd fih:

```
POLLIN
```

3ayet:

```cpp
BufferManager
```

↓

server.onClientData()

---

### 4.

Ila:

```
POLLERR

aw

POLLHUP
```

3ayet:

```cpp
server.onClientDisconnect()

removeFd()
```

---

### Chno khas Ayoub ylookup

- socket()
- bind()
- listen()
- accept()
- poll()
- fcntl()

w errors b7al:

- EINTR
- EAGAIN

w examples dyal poll servers (Beej Guide).

---

# Souhail – Makefile, Logger, Helpers

Khass khedmt Souhail tssehel développement mn awal nhar.

---

## Detailed tasks

### Makefile

Implement:

```
all

clean

fclean

re

NAME
```

b flags:

```
-Wall

-Wextra

-Werror

-std=c++98
```

Momkin tzid:

```bash
make debug
```

b:

```
-g

-DDEBUG
```

---

### Logger

Dir Logger basit.

Functions b7al:

```cpp
log_info()

log_error()
```

Khasso ykhdem mn ay module bla circular dependencies.

---

### Utility Helpers

Implement helpers dyal strings:

- trim
- n7yed `\r\n`
- split by spaces
- support colon parameters dyal IRC

Dir wrappers sghar dyal errors.

Exemple:

```cpp
check_syscall(...)
```

lli kaylogi errors b tari9a mowa7da.

---

### Build verification

Dir checklist:

- make re
- valgrind
- zero warnings

---

### Chno khas Souhail ylookup

- Best practices dyal Makefile
- IRC line format
- 512 bytes limit
- Buffer design
- Parsing utilities

# Phase 3 – Client Connection Management

## Ismail – Client Class, Registration State, w Lifecycle

Ismail howa lli kaymlek representation (**représentation**) logique dyal kol Client, mn lwa9t lli yconnecta 7tta ydisconnecta.

---

## Detailed tasks

### Client data model

Dir:

```cpp
class Client
```

fih had fields:

```cpp
fd

nickname

username

realname

bool registered

bool isOperator

std::set<std::string> channels
```

Ila 7tajiti, zid timestamps.

Dir enum bach registration flow ykoun wazi7:

```cpp
enum RegistrationState
{
    WAITING_PASS,
    WAITING_NICK_USER,
    REGISTERED
};
```

Had enum ghadi ybeyen fin wa9ef kol client f process dyal authentication.

---

### Lifecycle management

Implement methods f `Server` b7al:

```cpp
Client &getClientByFd(fd_t fd);
```

w

```cpp
void removeClient(fd_t fd);
```

Ayoub w Souhail ghadi yst3mlo had methods.

Dir hooks l:

### Mlli client ywelli registered

Khasso:

- ytsift lih welcome messages
- MOTD (Message Of The Day)
- ay initialization l'akhor.

### Mlli client ydisconnecta

Khasso:

- ytn7a mn channels
- ttsafa resources
- cleanup ykoun kaml.

---

### Command dispatching

Implement:

```cpp
void Server::handleCommand(
    Client &client,
    const ParsedCommand &cmd
);
```

Hiya lli ghadi troutei (**rediriger**) command l handler s7i7.

Exemple:

```cpp
handleJoin()

handlePrivmsg()

handleKick()

...
```

F had phase ma khasch implementation kamla.

Dir ghir stubs (**implémentations temporaires**) bach phases jayyin ykmlouhom bchwiya.

---

### Chno khas Ismail ylookup

- IRC numeric replies

Exemple:

```
001
002
003
...
```

- Error replies b7al:

```
ERR_NICKNAMEINUSE
```

- Registration state machine
- Protocol state transitions

Bach maykounouch clients half-registered.

---

# Ayoub – poll() Event Loop Details w FD Housekeeping

Daba Ayoub khaso yrebet `poll()` m3a `Server` w `BufferManager`.

---

## Detailed tasks

### Per-event behaviour

Kol mara `poll()` y3ti:

```
POLLIN
```

3la client:

3ayet:

```cpp
bufferManager.onReadable(fd, server);
```

BufferManager howa lli khaso y9ra data.

Ma khasch `poll()` ydir parsing.

---

Ila l event howa:

```
POLLERR

aw

POLLHUP
```

Dir:

- logging
- remove fd mn poll list
- server.onClientDisconnect()

---

### FD w Resource management

Khassk t7afed 3la mapping.

Ya imma:

```cpp
fd -> Client*
```

aw

`Server` howa owner.

Ma khasch ykoun double ownership.

7it ghadi ydir:

- leaks
- dangling pointers
- confusion.

---

Kol mara katdir:

```cpp
close(fd)
```

Khassha tkon m3aha:

```cpp
removeFd(fd)
```

Bach `poll()` ma yb9ach kay9ra fd msedoud.

---

### Chno khas Ayoub ylookup

- Mapping bin:

```
pollfd
```

w

```
Client
```

- recv() edge cases

Exemple:

```cpp
recv(...)
```

katrje3:

```
0
```

=> client tsed.

aw

```
-1
```

m3a

```
EAGAIN
```

=> ma kaynch data daba.

Khasso y3ref kifach y3amel m3a kol cas.

---

# Souhail – Buffer Management, recv(), CRLF, Parser Input

Souhail howa lli ghadi y7awwel bytes dyal TCP l IRC messages kamlin.

TCP ma kay3tikch darori command kamla f recv wa7da.

Momkin:

- nis command
- juj commands m3a b3diyathom
- aw aktar.

Hna kayji dour dyal BufferManager.

---

## Detailed tasks

### Buffer l kol client

Khassk t7afed:

```cpp
std::map<fd_t, std::string> buffers;
```

Kol fd 3endo buffer dyalo.

Mlli client yconnecta:

-> create buffer.

Mlli ydisconnecta:

-> erase buffer.

---

### 512 bytes limit

IRC kay7edded:

```
512 bytes
```

(m3a CRLF).

Ila client t3adda had limit:

Khassk:

- truncate (couper)
- aw disconnect
- w log error.

---

### Data ingestion

Implement:

```cpp
BufferManager::onReadable(
    fd_t fd,
    Server &server
);
```

Dakhelha:

Dir loop:

```cpp
recv(...)
```

7tta ma yb9ach data.

Kol data katji:

appendiha l buffer.

---

Ila:

```cpp
recv() == 0
```

=> client sedd connection.

Ila:

```cpp
recv() == -1
```

w

```
EAGAIN
```

=> wa9ef.

Ma kaynch data jdida.

---

Ma khasch ay function okhra t3ayet `recv()`.

Ghir:

```
BufferManager::onReadable()
```

bach architecture tb9a n9iya.

---

### Line extraction

Qelleb f buffer 3la:

```
\r\n
```

Kol line kamla:

khrejha.

B3d menha:

3ayet:

```cpp
server.onClientData(
    fd,
    line_without_crlf
);
```

---

Ila b9at ghir nis message:

Matms7hach.

Khalliha f buffer.

Mlli yji event jdid:

tkemmel mn fin wa9fat.

---

### Chno khas Souhail ylookup

- TCP fragmentation
- TCP packet coalescing
- IRC grammar
- Parsing dyal command + parameters
- Buffering strategies

# Interconnected Interfaces (Interfaces interconnectées) w "Contracts" (Contrats)

Bach lkhedma tb9a mrebta bin kolchi w ma tdi3ouch lwa9t, khaskom tdocumentiw (**documenter**) chi interfaces mohimmin w kol wa7ed y7teremhom.

---

# Server ↔ SocketManager (Ismail & Ayoub)

`SocketManager` khaso yexposi (**exposer**) had methods:

```cpp
pollOnce(Server&)

addClientFd()

removeFd()
```

`Server` khaso yprovidi (**fournir**) had hooks:

```cpp
onNewConnection()

onClientData()

onClientDisconnect()
```

Hadchi howa contract bin Ismail w Ayoub.

Ila tbeddlat signature (**signature**) dyal chi function:

- khas ytsra7 biha f daily meetup
- bach l'akhorin mayt9ta3ch 3lihom compilation.

---

# Server ↔ BufferManager (Ismail & Souhail)

`BufferManager` khaso yexposi:

```cpp
onReadable(fd, Server&)
```

Role dyalo howa:

- y9ra data
- yjme3ha
- ykhrrej complete IRC lines.

`Server` howa lli kayhandle:

- clients
- commands
- registration
- execution dyal commands

mlli line kamla twelli wajda.

---

Ttaf9o kifach BufferManager yreporti (**rapporter**) errors.

Exemple:

wach ghadi yst3mel:

- exceptions
- return codes

bach event loop yb9a n9i w simple.

---

# Client / Channel / Command APIs

(Hadi akthar haja kathemm Ismail walakin kolchi ghadi yst3mlha.)

Khasskom tdirou headers mn bkkri.

7tta ila implementation mazal mawjodach.

Exemple:

```cpp
Client.hpp

Channel.hpp

Server.hpp
```

Momkin ykouno fihom ghir declarations.

Hadchi kaykhlli Ayoub w Souhail y9dro ycompiliw code dyalhom bla ma ytsennaw implementation kamla.

---

Khass interfaces yb9aw stable.

Ila khas tbeddel:

- smit function
- parameters
- return type

khass tcoordoni (**coordonner**) m3a team 9bel push.

Bach matw9a3ch merge conflicts w compilation errors.

---

# Kifach tst3mlo hadchi f Meetups

F kol meetup (kol nhar aw kol juj iyam):

## Ismail khaso yreporti

- fin wasel `Server`
- fin wasel `Client`
- command handlers
- methods jdadin
- methods lli tbeddlo
- chno khas Ayoub w Souhail yst3mlo mn code dyalo.

---

## Ayoub khaso yreporti

- listening socket
- poll loop
- FD management
- chno mn events kaykhdem daba
- chno mazal ma tkammlch.

---

## Souhail khaso yreporti

- build status
- Makefile
- Logger
- Buffer behaviour
- parsing utilities
- ay helper jdida tzadat.

---

# Goal (Lhadaf)

Ila tb3to had checklists:

- kol wa7ed ghadi y3ref exactement chno khaso ydir.
- lkhedma ghadi tb9a interconnected (interconnectée).
- merges ghadi ykouno sahl.
- blockers ghadi ybano bkkri.
- meetups ghadi ywlliw mofidin.
- ma ghadich t3awdou nafs lkhedma juj mrat.
- w ghadi t9dro tkemlou project f 13 nhar b organisation mzyana.


