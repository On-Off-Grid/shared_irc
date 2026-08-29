# Phase 7: Poll-Driven Channel Message Delivery 📬

Welcome to **Phase 7** of the `ft_irc` project! This phase completes our server's compliance with the non-blocking requirement by transitioning all socket write operations to a fully event-driven, poll-buffered model.

**Owner**: Ayoub (Networking/Event Loop), with integration support from Souhail (Buffers/Data Layer).  
**Branch**: `fix/registration-and-event-loop-bugs` (finalized branch integrating Phase 7)

---

## 📑 Table of Contents

1. [Overview](#1-overview)
2. [The Problem: Synchronous Sends](#2-the-problem-synchronous-sends)
3. [The Solution: Non-Blocking Outgoing Buffers](#3-the-solution-non-blocking-outgoing-buffers)
4. [What Changed in Phase 7](#4-what-changed-in-phase-7)
5. [Outgoing Message Flow (Diagram)](#5-outgoing-message-flow-diagram)
6. [Core Implementation Details](#6-core-implementation-details)
   - [Queueing Messages (`sendToClient`)](#1-queueing-messages-sendtoclient)
   - [Monitoring Writability (`_setPollOut`)](#2-monitoring-writability-_setpollout)
   - [Flushing Queues (`_flushOutgoingData`)](#3-flushing-queues-_flushoutgoingdata)
   - [Centralized Broadcasts (`broadcastToChannel`)](#4-centralized-broadcasts-broadcasttochannel)
7. [Darija Summary Notes](#7-darija-summary-notes)
8. [Integration & Cleanup Checklist](#8-integration-cleanup-checklist)
9. [Testing Procedures](#9-testing-procedures)

---

## 1. Overview

In Phase 4, we set up `poll()` to monitor client file descriptors for incoming data (`POLLIN`). However, when sending messages (like numeric replies or private messages), the server wrote directly to the client socket using synchronous `send()` calls. 

In **Phase 7**, we eliminate synchronous writes. Instead, we queue outgoing messages in per-client buffers and monitor sockets for writability (`POLLOUT`). Sockets are only written to when `poll()` confirms they are ready to accept data, preventing blocking and ensuring high throughput.

---

## 2. The Problem: Synchronous Sends

In a production or testing environment:
1. **Network Congestion**: If a client's internet connection slows down, the OS TCP transmit buffer fills up.
2. **Blocking System Calls**: A subsequent synchronous `send()` call on a non-blocking socket will return `-1` with `EAGAIN` or `EWOULDBLOCK`. 
3. **Data Loss**: If we ignore this return value or drop the bytes (as we did in the Phase 5 skeleton), the client receives corrupted/truncated messages, causing IRC clients to disconnect or malfunction.
4. **Thread Freezing**: If we attempt to loop until the write succeeds, we block the entire single-threaded server, violating the core requirement of using a multiplexed event loop.

---

## 3. The Solution: Non-Blocking Outgoing Buffers

We solve this using a dynamic queue:
* Every client has an outgoing buffer (`std::string`).
* When we want to send a message, we append it to the client's buffer and set the `POLLOUT` flag on their file descriptor in `poll()`.
* When the event loop detects `POLLOUT`, we write as much as possible using `send()`.
* If `send()` reports `EAGAIN` or `EWOULDBLOCK`, we stop writing, keep the remaining data in the buffer, and wait for the next `POLLOUT` event.
* Once the buffer is empty, we remove the `POLLOUT` flag to prevent CPU spinning.

---

## 4. What Changed in Phase 7

| Area | Before (Phase 6) | After (Phase 7) |
|---|---|---|
| `Server::sendToClient` | Synchronous `send()` inside `_sendData` | Appends to `_outgoingBuffers[fd]` and enables `POLLOUT` |
| `poll()` loop (`run()`) | Monitored `POLLIN` only | Monitored both `POLLIN` and `POLLOUT` |
| Channel Broadcasts | `Channel::broadcast()` wrote directly to socket | `Server::broadcastToChannel()` routes to non-blocking queues |
| Disconnects | Closed socket immediately | Closed socket and purged `_outgoingBuffers[fd]` |
| CPU Usage | Efficient | Stays efficient (POLLOUT disabled when buffer is empty) |

---

## 5. Outgoing Message Flow (Diagram)

```text
  Client Logic / Command Handler
               │
               ▼
   Server::sendToClient(fd, msg)
               │
               ├─► Append "msg\r\n" to _outgoingBuffers[fd]
               └─► Server::_setPollOut(fd, true) (Enables POLLOUT flag)
               │
      [Event Loop Waits in poll()]
               │
               ▼ poll() reports POLLOUT on fd
   Server::_flushOutgoingData(fd)
               │
               ▼ send(fd, buf, len, 0)
               ├─► bytesSent > 0  ──► Erase written bytes from buffer
               │                      If buffer is now empty -> _setPollOut(fd, false)
               │
               └─► EAGAIN/EWOULDBLOCK ──► Do nothing, return (wait for next POLLOUT)
```

---

## 6. Core Implementation Details

### 1. Queueing Messages (`sendToClient`)
Instead of performing immediate writes, `sendToClient` now queues data and flags the event loop:
```cpp
void Server::sendToClient(int fd, const std::string& message)
{
    if (!hasClient(fd))
        return ;
    _outgoingBuffers[fd] += message + "\r\n";
    _setPollOut(fd, true);
}
```

### 2. Monitoring Writability (`_setPollOut`)
Toggles the `POLLOUT` event bit inside the `pollfd` vector:
```cpp
void Server::_setPollOut(int fd, bool enabled)
{
    for (size_t i = 0; i < _pollfds.size(); ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            if (enabled)
                _pollfds[i].events |= POLLOUT;
            else
                _pollfds[i].events &= ~POLLOUT;
            return ;
        }
    }
}
```

### 3. Flushing Queues (`_flushOutgoingData`)
Triggers upon receiving a `POLLOUT` event in the poll loop. It safely writes queued data and handles partial sending constraints:
```cpp
void Server::_flushOutgoingData(int fd)
{
    std::map<int, std::string>::iterator it = _outgoingBuffers.find(fd);
    if (it == _outgoingBuffers.end())
    {
        _setPollOut(fd, false);
        return ;
    }

    std::string& data = it->second;
    while (!data.empty())
    {
        ssize_t bytesSent = send(fd, data.c_str(), data.size(), 0);
        if (bytesSent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return ; // Socket buffer full, wait for next POLLOUT
            if (errno == EINTR)
                continue ; // Interrupted by signal, retry immediately
            
            std::cerr << "send() error on fd " << fd << ": " << strerror(errno) << std::endl;
            disconnectClient(fd); // Fatal write error (e.g., broken pipe)
            return ;
        }
        if (bytesSent == 0)
            return ;
        data.erase(0, bytesSent); // Keep remaining unsent data
    }
    _outgoingBuffers.erase(fd);
    _setPollOut(fd, false); // Buffer fully flushed, stop monitoring POLLOUT
}
```

### 4. Centralized Broadcasts (`broadcastToChannel`)
The channel broadcast was refactored out of the `Channel` class and moved to `Server` to utilize our new queue infrastructure:
```cpp
void Server::broadcastToChannel(const Channel& channel, const std::string& message, int excludeFd)
{
    const std::map<int, Client*>& members = channel.getMembers();
    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (it->first != excludeFd)
            sendToClient(it->first, message);
    }
}
```

---

## 7. Darija Summary Notes

> [!NOTE] **Darija Summary**
> **🇲🇦 L'khot (The Team):** Phase 7 hya li kmlat l'meshrou3 w khllato irffed non-blocking b'seh. Qbel f'Phase 6 w 5, mli kikhrej chi message (bhal command reply), server kan ki-sifto tmma b `send()`. Hada error kbir, hit ila l'klyan 3ndo net tqil aw safe l'buffer dial OS, server ghadi ibloqa w y-spinni CPU dialna. 
> 
> Daba f'Phase 7, derna dynamic queues:
> 1. Ay message bghina nsiftoh l'chi client, kan-khznouh f'wahd `std::map<int, std::string> _outgoingBuffers`.
> 2. Kan-tbb3o l'FD b event dial `POLLOUT` f vector `_pollfds`.
> 3. Mli `poll()` kitla3 blli l'client wajed istaqbel data (revents fiha POLLOUT), kan-3eyto l `_flushOutgoingData()`.
> 4. Ila tsift kolshi, kan-hiyydo `POLLOUT` event. Ila bqa chi chwia d'data w net tqil, kan-khlliwh fl-buffer w kan-tsnnaw `poll()` t-golina wajed fl-mera jaya. Bla blocking, bla threading!

---

## 8. Integration & Cleanup Checklist

```text
✔ No Synchronous send() — send() is called ONLY inside _flushOutgoingData() triggered by POLLOUT.
✔ Memory Leak Protection — _outgoingBuffers purged on client disconnect and server shutdown.
✔ Loop Safety           — Event loop properly monitors both POLLIN and POLLOUT.
✔ Zero CPU Spinning     — POLLOUT disabled as soon as the buffer is flushed.
✔ Centralized Broadcast — Channel message delivery is fully poll-driven and safe.
```

---

## 9. Testing Procedures

### Stress-Testing Writable Buffers (Simulated Congestion)

To verify that the server does not freeze when a client cannot keep up with outgoing data:

1. Launch the server:
   ```bash
   ./ircserv 6667 pass
   ```
2. Open a connection using `nc` but suspend the receiver:
   ```bash
   # In another terminal, connect using nc
   nc 127.0.0.1 6667
   ```
3. Authenticate and join a channel (e.g., `#stress`).
4. Force high volume traffic to that channel from a different client.
5. Suspend input reading on the first client (e.g., using `Ctrl+Z` on the `nc` command, or limiting its TCP window).
6. Verify that:
   - The server continues to handle other clients successfully.
   - The suspended client's outgoing data accumulates in `_outgoingBuffers` without causing crashes or memory leaks.
   - Resuming the client drains the queue cleanly.
