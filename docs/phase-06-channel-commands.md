# Phase 6: Channel Commands 💬

Welcome to **Phase 6** of the `ft_irc` project! This phase marks the transition of our server from a simple connection listener to a fully functional IRC server supporting channel creation, management, messaging, and access control.

**Owner**: Ismail  
**Integrated by**: Souhail (Data layer integration & safety fixes)  
**Branch**: `feature/phase-06-channel-commands`  

---

## 📑 Table of Contents

1. [Overview](#1-overview)
2. [What Changed in Phase 6](#2-what-changed-in-phase-6)
3. [New and Modified Files](#3-new-and-modified-files)
4. [Architecture & Channel Life Cycle](#4-architecture--channel-life-cycle)
5. [IRC Channel Commands Reference](#5-irc-channel-commands-reference)
   - [JOIN](#join)
   - [PART](#part)
   - [PRIVMSG](#privmsg)
   - [NOTICE](#notice)
   - [TOPIC](#topic)
   - [KICK](#kick)
   - [INVITE](#invite)
   - [MODE](#mode)
6. [Replies - New Numeric Codes](#6-replies---new-numeric-codes)
7. [Integration Checklist](#7-integration-checklist)
8. [Testing Procedures](#8-testing-procedures)

---

## 1. Overview

In Phase 5, we set up the `CommandDispatcher` and client registration flow (`PASS` → `NICK` → `USER`). In **Phase 6**, we implement all core channel operations. 

Clients can now:
1. Create and join channels using `JOIN`.
2. Leave channels using `PART`.
3. Communicate in rooms or privately using `PRIVMSG` and `NOTICE`.
4. Manage channel topics via `TOPIC`.
5. Kick misbehaving users with `KICK` (requires operator status).
6. Invite other users using `INVITE`.
7. Configure channel restrictions and promote operators with `MODE` (supporting `i`, `t`, `k`, `l`, `o`).

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot (The Team):** F'had Phase 6, khdemna 3la ga3 l'awamir dyal channels (l'ghoraf). Daba l'clients li m-enregistryin iqdro idkhlo l'shi channel b `JOIN`, ikhrojo b `PART`, isifto msg l'channel kamla aw l'chi hed private b `PRIVMSG`, i-kickiw bnadem b `KICK`, w i-controliw les modes dyal room b `MODE` (+i invite-only, +t restricted topic, +k key/passwd, +l limit dyal bnadem, +o operator status). Hadshi kamlo blockeina l'clients li mam-enregistryinch bash mayqdrosh idiroh.

---

## 2. What Changed in Phase 6

| Area / Command | Before (Phase 5 Skeleton) | After (Phase 6) |
|---|---|---|
| `JOIN` | Ignored | Creates/joins channel, validates keys (`+k`), limits (`+l`), invite-only (`+i`), sends topic + name list |
| `PART` | Ignored | Removes member, broadcasts PART, deletes channel if empty |
| `PRIVMSG` | Ignored | Routes message to targeted channel members or private nick |
| `NOTICE` | Ignored | Sends message without generating any error replies |
| `TOPIC` | Skeleton only | Views topic or sets topic (restricts to ops if `+t` is active) |
| `KICK` | Ignored | Operator removes target client from channel |
| `INVITE` | Ignored | Operator invites user to channel; adds target client to invited list |
| `MODE` | Ignored | Sets/unsets channel modes `i`, `t`, `k`, `l`, `o` with parameters |

---

## 3. New and Modified Files

No new files were created in this phase, but existing skeletons were heavily extended:

```text
includes/Channel.hpp             — Added flags for inviteOnly, topicRestricted, keys, limits, and invited lists.
src/Channel.cpp                  — Implemented member, operator, invite lists and helper logic.
includes/CommandDispatcher.hpp   — Added declarations for _handleJoin, _handlePart, _handlePrivmsg, etc.
src/CommandDispatcher.cpp        — Implemented routing and command execution for the 8 channel commands.
includes/Replies.hpp             — Declared new numeric reply builders (324, 331, 332, 341, 353, 366, 401, 403, 404, 441, 442, 443, 471, 473, 475, 482).
src/Replies.cpp                  — Implemented string builders for the above numeric replies.
includes/Server.hpp              — Added channel storage (_channels map) and helper accessors (getChannel, createChannel, deleteChannel, getChannels).
src/Server.cpp                   — Implemented channel factory methods and clean-up in destructor/disconnect.
```

---

## 4. Architecture & Channel Life Cycle

Channels are owned by the `Server` using a `std::map<std::string, Channel*> _channels` container. 
* **Creation**: A channel is created automatically when the first client joins it via `JOIN`. The first user joining a new channel is automatically promoted to **Channel Operator** (`@` prefix).
* **Destruction**: When the last client leaves a channel (via `PART`, `KICK`, or `QUIT`), the channel object is deleted from memory to prevent memory leaks.

```text
[Client 1: JOIN #general]
       │
       ▼
Server searches map for "#general" -> Not Found!
       │
       ├─► Server::createChannel("#general")
       ├─► Channel::addMember(Client 1)
       ├─► Channel::addOperator(Client 1) (Client 1 becomes Op)
       └─► Broadcast JOIN message & send RPL_WELCOME/RPL_NAMREPLY
```

---

## 5. IRC Channel Commands Reference

All commands require the client to be registered (`isRegistered() == true`). If not registered, commands are silently ignored.

### JOIN
* **Syntax**: `JOIN <channels> [<keys>]`
* **Behavior**: 
  - Splits `<channels>` and `<keys>` by commas `,` to handle multiple joins in a single command.
  - Channels must start with `#` or `&` (returns `ERR_NOSUCHCHANNEL` if invalid).
  - Checks if channel is full (`+l` mode -> `ERR_CHANNELISFULL`).
  - Checks if channel is invite-only (`+i` mode -> user must be in invited list, else `ERR_INVITEONLYCHAN`).
  - Checks if channel password matches (`+k` mode -> `ERR_BADCHANNELKEY`).
  - Upon successful join, broadcasts a `JOIN` message to the channel, sends the channel's `TOPIC`, and sends the list of names (`RPL_NAMREPLY` + `RPL_ENDOFNAMES`).

### PART
* **Syntax**: `PART <channels> [:<reason>]`
* **Behavior**:
  - Splits `<channels>` by commas `,`.
  - Verifies that the channel exists (`ERR_NOSUCHCHANNEL`) and the client is currently a member (`ERR_NOTONCHANNEL`).
  - Broadcasts the `PART` message to the channel (with optional reason) and removes the user from the channel.
  - If the channel becomes empty, the channel is destroyed.

### PRIVMSG
* **Syntax**: `PRIVMSG <target> :<message>`
* **Behavior**:
  - Directs a message to either a channel or a nickname.
  - If target is a channel: verifies the channel exists (`ERR_NOSUCHCHANNEL`) and the client is a member (`ERR_CANNOTSENDTOCHAN`). Broadcasts to all channel members *except* the sender.
  - If target is a nickname: verifies target client is connected (`ERR_NOSUCHNICK`). Delivers message directly to the target.

### NOTICE
* **Syntax**: `NOTICE <target> :<message>`
* **Behavior**:
  - Operates identically to `PRIVMSG`, but does **not** send any error replies (such as `ERR_NOSUCHNICK` or `ERR_NOSUCHCHANNEL`) to avoid loops and spam.

### TOPIC
* **Syntax**: `TOPIC <channel> [:<new_topic>]`
* **Behavior**:
  - If no `<new_topic>` is provided: retrieves the current topic. Returns `RPL_TOPIC` or `RPL_NOTOPIC`.
  - If `<new_topic>` is provided: updates the channel topic. If topic restriction mode is enabled (`+t` mode), verifies that the client is a Channel Operator (`ERR_CHANOPRIVSNEEDED`). Broadcasts the new topic to all members.

### KICK
* **Syntax**: `KICK <channel> <user> [:<reason>]`
* **Behavior**:
  - Verifies channel exists, sender is a member, and sender is a Channel Operator (`ERR_CHANOPRIVSNEEDED`).
  - Verifies target user is in the channel (`ERR_USERNOTINCHANNEL`).
  - Broadcasts a `KICK` message to the channel, removes the target user, and destroys the channel if it becomes empty.

### INVITE
* **Syntax**: `INVITE <user> <channel>`
* **Behavior**:
  - Verifies channel exists and the sender is in the channel (`ERR_NOTONCHANNEL`).
  - If the channel is invite-only (`+i` mode), verifies the sender is a Channel Operator (`ERR_CHANOPRIVSNEEDED`).
  - Verifies target user exists (`ERR_NOSUCHNICK`) and is not already in the channel (`ERR_USERONCHANNEL`).
  - Adds target user to the channel's invite list.
  - Sends a confirmation to the sender (`RPL_INVITING`) and forwards the `INVITE` message to the target client.

### MODE
* **Syntax**: `MODE <channel> [<modes> [<parameters>]]`
* **Behavior**:
  - If only channel is provided: returns active modes (`RPL_CHANNELMODEIS`), including parameters for keys (`+k`) and limits (`+l`).
  - If modes are provided: checks if the client is a Channel Operator (`ERR_CHANOPRIVSNEEDED`).
  - Parses the mode string character-by-character, toggling flags based on `+` or `-` state:
    - `i`: Invite-only channel toggled.
    - `t`: Topic restrictions toggled.
    - `k`: Sets password (requires parameter) or unsets password.
    - `l`: Sets client limit (requires positive integer parameter) or unsets limit.
    - `o`: Promotes client to operator (requires nick parameter) or demotes client.
  - Applies valid changes and broadcasts the applied mode change to the channel.

---

## 6. Replies - New Numeric Codes

The following IRC numeric replies were added to `Replies.cpp`:

```cpp
Replies::channelModeIs(...)      // 324 RPL_CHANNELMODEIS
Replies::noTopic(...)            // 331 RPL_NOTOPIC
Replies::topic(...)              // 332 RPL_TOPIC
Replies::inviting(...)           // 341 RPL_INVITING
Replies::namReply(...)           // 353 RPL_NAMREPLY
Replies::endOfNames(...)         // 366 RPL_ENDOFNAMES

Replies::errNoSuchNick(...)      // 401 ERR_NOSUCHNICK
Replies::errNoSuchChannel(...)   // 403 ERR_NOSUCHCHANNEL
Replies::errCannotSendToChan(...)// 404 ERR_CANNOTSENDTOCHAN
Replies::errUserNotInChannel(...)// 441 ERR_USERNOTINCHANNEL
Replies::errNotOnChannel(...)    // 442 ERR_NOTONCHANNEL
Replies::errUserOnChannel(...)   // 443 ERR_USERONCHANNEL
Replies::errChannelIsFull(...)   // 471 ERR_CHANNELISFULL
Replies::errInviteOnlyChan(...)  // 473 ERR_INVITEONLYCHAN
Replies::errBadChannelKey(...)   // 475 ERR_BADCHANNELKEY
Replies::errChanOpPrivsNeeded(...)// 482 ERR_CHANOPRIVSNEEDED
```

---

## 7. Integration Checklist

```text
✔ Compiles          — make re: zero errors, zero warnings (-Wall -Wextra -Werror)
✔ C++98 compliant   — no std::to_string, no auto, no nullptr
✔ Multi-channel     — JOIN and PART parse comma-separated lists correctly
✔ Empty Cleanup     — Channel instances deleted from memory when empty
✔ Privileges        — Operators check (KICK, INVITE, TOPIC, MODE) works
✔ Silent notice     — NOTICE does not generate error replies on failures
```

---

## 8. Testing Procedures

### Manual Testing with netcat (`nc`)

Run the server on a terminal:
```bash
./ircserv 6667 pass123
```

In another terminal, connect Client A:
```bash
nc -C 127.0.0.1 6667
PASS pass123
NICK Alice
USER alice 0 * :Alice In Wonderland
```

In a third terminal, connect Client B:
```bash
nc -C 127.0.0.1 6667
PASS pass123
NICK Bob
USER bob 0 * :Bob The Builder
```

#### Test Scenario 1: Join/Part & Channel creation
1. On Alice's terminal, type:
   ```irc
   JOIN #chat
   ```
   *Expected*: Alice receives join broadcast, topic empty (`331`), names reply (`353` showing `@Alice`) and end of names (`366`).
2. On Bob's terminal, type:
   ```irc
   JOIN #chat
   ```
   *Expected*: Bob receives join broadcast. Alice also receives broadcast that Bob has joined. Bob receives names reply showing `@Alice Bob`.
3. On Bob's terminal, type:
   ```irc
   PART #chat :leaving now
   ```
   *Expected*: Both Bob and Alice receive the PART broadcast.

#### Test Scenario 2: Channel Modes & Protections
1. Alice joins `#secret`. She is operator (`@Alice`).
2. Alice sets invite-only and key:
   ```irc
   MODE #secret +ik key123
   ```
3. Bob attempts to join without key:
   ```irc
   JOIN #secret
   ```
   *Expected*: Bob receives `:ircserv 473 Bob #secret :Cannot join channel (+i)` or `:ircserv 475 Bob #secret :Cannot join channel (+k)`.
4. Bob attempts to join with wrong key:
   ```irc
   JOIN #secret wrongkey
   ```
   *Expected*: Bob receives key mismatch error (`475`).
5. Alice invites Bob:
   ```irc
   INVITE Bob #secret
   ```
   *Expected*: Alice receives `341` inviting confirmation. Bob receives the invite notification.
6. Bob joins with the key:
   ```irc
   JOIN #secret key123
   ```
   *Expected*: Bob successfully joins `#secret`.

#### Test Scenario 3: KICK and TOPIC Restricting
1. Alice sets topic restriction:
   ```irc
   MODE #secret +t
   ```
2. Bob attempts to change topic:
   ```irc
   TOPIC #secret :Bob's topic
   ```
   *Expected*: Bob receives `:ircserv 482 Bob #secret :You're not channel operator`.
3. Alice changes topic:
   ```irc
   TOPIC #secret :Alice's topic
   ```
   *Expected*: Both receive topic broadcast.
4. Alice kicks Bob:
   ```irc
   KICK #secret Bob :no spammers
   ```
   *Expected*: Bob is removed from channel; both receive KICK broadcast.
