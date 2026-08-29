# ft_irc: Step-by-Step Testing & Edge Cases Guide 🧪

This guide provides a comprehensive, step-by-step testing manual for the `ft_irc` server. It outlines how to verify the mandatory features, check for edge cases, and run stress tests to ensure compliance with the C++98 non-blocking event-loop requirements.

---

## 📑 Table of Contents

1. [Starting the Server & Compilation](#starting-the-server--compilation)
2. [Section 1: Client Registration Flow (PASS, NICK, USER)](#section-1-client-registration-flow-pass-nick-user)
3. [Section 2: Networking & Fragmented TCP Buffers (Ctrl+D Test)](#section-2-networking--fragmented-tcp-buffers-ctrld-test)
4. [Section 3: Channel Joins & Parts](#section-3-channel-joins--parts)
5. [Section 4: Channel Operator Commands (KICK, INVITE, TOPIC)](#section-4-channel-operator-commands-kick-invite-topic)
6. [Section 5: Channel Modes (i, t, k, l, o)](#section-5-channel-modes-i-t-k-l-o)
7. [Section 6: Messaging & Notice (PRIVMSG, NOTICE)](#section-6-messaging--notice-privmsg-notice)
8. [Section 7: Outgoing Writable Buffers (Slow Clients & POLLOUT)](#section-7-outgoing-writable-buffers-slow-clients--pollout)
9. [Section 8: Critical Edge Cases & Stability Checks](#section-8-critical-edge-cases--stability-checks)

---

## Starting the Server & Compilation

First, clean build the server to ensure dependency files (`.d` files) are safe and flags `-Wall -Wextra -Werror -std=c++98` are enforced:
```bash
make re
```

Start the server on port `6667` with password `testpass`:
```bash
./ircserv 6667 testpass
```

---

## Section 1: Client Registration Flow (PASS, NICK, USER)

A client must register before they can send any other commands. All commands sent before a successful registration must be ignored.

### 1. Successful Registration
Open a connection using `nc`:
```bash
nc -C 127.0.0.1 6667
```
Send the registration command sequence:
```irc
PASS testpass
NICK ismail
USER ismail 0 * :Ismail User
```
*Expected Output:*
The server replies with the welcome burst:
```irc
:ircserv 001 ismail :Welcome to the IRC Network, ismail!ismail@127.0.0.1
:ircserv 002 ismail :Your host is ircserv, running version ft_irc-1.0
:ircserv 003 ismail :This server was created today
:ircserv 004 ismail ircserv ft_irc-1.0 o o
```

### 2. Registration Edge Cases to Look For:
* **Missing Password**: Run `nc -C 127.0.0.1 6667`, then send:
  ```irc
  NICK ismail
  USER ismail 0 * :Ismail User
  ```
  *Expected:* No welcome burst is received because `PASS` was not sent or was incorrect.
* **Wrong Password**: Send `PASS wrongpass` followed by `NICK` and `USER`.
  *Expected:* Server returns `:ircserv 464 * :Password incorrect`.
* **Already Registered**: After registering successfully, send `PASS testpass` or `USER ismail 0 * :Ismail User` again.
  *Expected:* Server returns `:ircserv 462 ismail :You may not reregister`.
* **Erroneous Nickname**: Try setting nickname to invalid formats:
  ```irc
  NICK 1ismail    (starts with a digit)
  NICK is@mail    (contains invalid character '@')
  ```
  *Expected:* Server returns `:ircserv 432 <current_nick> <bad_nick> :Erroneous nickname`.
* **Nickname Collision**: Connect two clients.
  - Client 1 registers as `Alice`.
  - Client 2 tries to register as `Alice` or tries to change their nickname:
    ```irc
    NICK Alice
    ```
  *Expected:* Server returns `:ircserv 433 * Alice :Nickname is already in use` (or `433 <client2_nick> Alice :Nickname is already in use`).

---

## Section 2: Networking & Fragmented TCP Buffers (Ctrl+D Test)

TCP is stream-oriented, meaning packets can arrive fragmented. The server must aggregate reads until a complete `\r\n` delimiter is found.

### The Fragmented input test:
1. Open connection:
   ```bash
   nc 127.0.0.1 6667
   ```
2. Type `PAS` and press `Ctrl+D` (sends data without newline).
3. Type `S testpass` and press `Enter`.
4. Type `NIC` and press `Ctrl+D`.
5. Type `K ismail` and press `Enter`.
6. Type `USER ismail 0 * :Test` and press `Enter`.
*Expected:* The server successfully registers the client. The command fragment `PAS` and `S testpass` must be correctly glued together as `PASS testpass` by the `BufferManager`.

---

## Section 3: Channel Joins & Parts

### 1. Multiple Channel Joins
Register a client (`Alice`) and send:
```irc
JOIN #chan1,#chan2,#chan3
```
*Expected:* Alice joins all three channels. The server sends join announcements, topics, and names replies (`353`, `366`) for each channel.

### 2. Leaving Channels (`PART`)
Leave multiple channels with a custom reason:
```irc
PART #chan1,#chan2 :goodbye everyone
```
*Expected:* Alice leaves `#chan1` and `#chan2`. Any other user on those channels receives a broadcast: `:Alice!alice@127.0.0.1 PART #chan1 :goodbye everyone`.

### 3. Edge Cases:
* **Invalid Channel Name**: Send `JOIN chan1` (missing `#` or `&` prefix).
  *Expected:* Server returns `:ircserv 403 Alice chan1 :No such channel`.
* **Not on Channel PART**: Try to leave a channel you are not in:
  *Expected:* Server returns `:ircserv 442 Alice #chan1 :You're not on that channel`.

---

## Section 4: Channel Operator Commands (KICK, INVITE, TOPIC)

Connect Client A (`Alice` - joins first so she is Op `@Alice`) and Client B (`Bob` - joins second, normal member).

### 1. KICK Command
1. Alice kicks Bob:
   ```irc
   KICK #chat Bob :violating rules
   ```
   *Expected:* Bob is removed from `#chat`. Both receive the KICK broadcast message.
2. Bob (non-operator) tries to kick Alice:
   ```irc
   KICK #chat Alice :I don't like you
   ```
   *Expected:* Server returns `:ircserv 482 Bob #chat :You're not channel operator` to Bob. Alice is not kicked.

### 2. INVITE Command
1. Alice sets `#chat` to invite-only:
   ```irc
   MODE #chat +i
   ```
2. Bob tries to join:
   ```irc
   JOIN #chat
   ```
   *Expected:* Bob gets `:ircserv 473 Bob #chat :Cannot join channel (+i)`.
3. Alice invites Bob:
   ```irc
   INVITE Bob #chat
   ```
   *Expected:* Alice receives `:ircserv 341 Alice Bob #chat`. Bob receives the invitation announcement from Alice.
4. Bob joins `#chat` again:
   ```irc
   JOIN #chat
   ```
   *Expected:* Bob successfully joins the channel (invited flag consumed).

### 3. TOPIC Command
1. Bob views topic:
   ```irc
   TOPIC #chat
   ```
   *Expected:* Server returns topic or `331 RPL_NOTOPIC`.
2. Alice sets topic restriction:
   ```irc
   MODE #chat +t
   ```
3. Bob tries to set topic:
   ```irc
   TOPIC #chat :Bob's rules
   ```
   *Expected:* Server returns `482 ERR_CHANOPRIVSNEEDED` to Bob.
4. Alice sets topic:
   ```irc
   TOPIC #chat :Welcome to Wonderland
   ```
   *Expected:* Server updates topic and broadcasts a `TOPIC` message to all members.

---

## Section 5: Channel Modes (i, t, k, l, o)

Only operators can change channel modes.

### 1. Test Mode `k` (Channel Password)
* **Set Key**: `MODE #chat +k secret123`
* **Test Join (No Key)**: `JOIN #chat` -> Expected: `475 ERR_BADCHANNELKEY`.
* **Test Join (Wrong Key)**: `JOIN #chat wrongpass` -> Expected: `475 ERR_BADCHANNELKEY`.
* **Test Join (Correct Key)**: `JOIN #chat secret123` -> Expected: Success.
* **Remove Key**: `MODE #chat -k`

### 2. Test Mode `l` (User Limit)
* **Set Limit**: `MODE #chat +l 2`
* Connect a third client (`Charlie`) and attempt to join:
  ```irc
  JOIN #chat
  ```
  *Expected:* Charlie receives `:ircserv 471 Charlie #chat :Cannot join channel (+l)`.
* **Remove Limit**: `MODE #chat -l`

### 3. Test Mode `o` (Operator Promotion/Demotion)
* **Promote**: `MODE #chat +o Bob` (sent by Alice).
  *Expected:* Bob becomes operator. His name in names list starts with `@`.
* **Verify**: Bob can now change topics or modes.
* **Demote**: `MODE #chat -o Bob` (sent by Alice).
  *Expected:* Bob is demoted back to a regular member.

---

## Section 6: Messaging & Notice (PRIVMSG, NOTICE)

### 1. PRIVMSG to Channel
Alice sends: `PRIVMSG #chat :Hello team!`
*Expected:* Bob receives the message. Alice does **NOT** receive her own message (prevents echo loop).

### 2. PRIVMSG to User (Private Message)
Alice sends: `PRIVMSG Bob :Are you there?`
*Expected:* Bob receives the private message directly.

### 3. NOTICE Command
Alice sends: `NOTICE Bob :System alert`
*Expected:* Bob receives notice. NOTICE is identical to PRIVMSG but does not generate errors. Test this by sending notice to an invalid target:
```irc
NOTICE NonExistentUser :Hello
```
*Expected:* Server silently ignores the command. No error reply (`401`) is returned.

---

## Section 7: Outgoing Writable Buffers (Slow Clients & POLLOUT)

Our server implements non-blocking outgoing buffers. Outgoing messages are only sent when `POLLOUT` is triggered by the OS.

### Outgoing congestion test:
1. Open a client connection `nc` and suspend its read stream:
   ```bash
   nc 127.0.0.1 6667
   ```
   *Register the client, then suspend it using `Ctrl+Z` in the shell.*
2. Connect another client and broadcast 50 large messages to a shared channel.
3. Check the server console and memory usage.
   *Expected:* The server continues to function normally for all other clients. The messages for the suspended client are queued in `_outgoingBuffers[fd]` instead of block-writing or crashing.
4. Resume the suspended client:
   ```bash
   fg %1  # (resumes the nc process)
   ```
   *Expected:* The suspended client immediately receives the entire queue of messages without loss or corruption.

---

## Section 8: Critical Edge Cases & Stability Checks

During evaluations, peers will look for crash-scenarios. Run these checks to verify robustness:

### 1. Loop Index-Shifting Check
When a client disconnects, we erase their socket from the `_pollfds` vector. 
* **The Danger**: If client `i` is erased, the elements shift left, and index `i+1` becomes index `i`. If our event loop does `++i` unconditionally, the next client is skipped, ignoring their events.
* **The Check**: Verify that when multiple clients send commands simultaneously and one client abruptly quits, the other clients' commands are still fully processed in the same loop cycle. Our implementation handles this post-loop in [`Server::run()`](../src/Server.cpp#L153).

### 2. IRC Case-Mapping Case-Insensitivity Check
IRC nicknames are case-insensitive. More importantly, they follow the RFC 2812 case-mapping where `{}|^` are lowercase equivalents of `[]\~`.
* **The Check**: 
  - Register a client as `Alice[bot]`.
  - Try to register another client as `alice{bot}`.
  *Expected:* Nickname collision error (`433`) must be triggered. Our comparison function [`areNicknamesEqual()`](file:///home/souhail/Desktop/Desktop/cercle_dzab/shared_irc/src/CommandDispatcher.cpp#L378) handles this mapping correctly.

### 3. Cleanup of Empty Channels
* **The Check**: When the last user leaves a channel (via `PART`, `KICK`, or `QUIT`), the channel object must be deleted from the server map.
* Verify that joining the channel again creates a fresh channel and promotes the joiner to operator (`@`).

### 4. Valgrind Memory Leak Checks
Start server under Valgrind:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 testpass
```
Connect several clients, join channels, toggle modes, send messages, disconnect them, and stop the server using `Ctrl+C`.
*Expected:* Valgrind reports `definitely lost: 0 bytes`, `indirectly lost: 0 bytes`. All dynamically allocated objects are cleaned up.
