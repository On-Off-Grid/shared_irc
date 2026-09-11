# ft_irc — Peer Evaluation Sheet

> Use this sheet during the defence to record observations and decisions.  
> Keep the tone polite, constructive, and focused on learning.

---

## 1. Introduction & Conduct

Please comply with the following rules:

- Remain **polite, courteous, respectful and constructive** throughout the evaluation process. The well-being of the community depends on it.
- Identify with the student or group whose work is evaluated the possible **dysfunctions** in their project. Take the time to discuss and debate the problems that may have been identified.
- Consider that there might be some differences in how peers understood the project’s instructions and scope. Always keep an **open mind** and grade as honestly as possible. Pedagogy is useful only if peer-evaluation is done seriously.

---

## 2. General Guidelines

- Only grade the work that was turned in the **Git repository** of the evaluated student or group.
- Double-check that:
  - The Git repository belongs to the student(s).
  - The project is the one expected.
  - `git clone` is used in an **empty folder**.
- Check carefully that no **malicious aliases** were used to fool you and make you evaluate something that is not the content of the official repository.
- If applicable, review together any **scripts used to facilitate grading** (testing or automation scripts).
- If you have **not completed** the assignment you are going to evaluate, you must read the entire subject prior to starting the evaluation process.
- Use the available flags to report:
  - Empty repository
  - Non-functioning program
  - Norm error
  - Cheating, etc.  
  In these cases, the evaluation process ends and the final grade is:
  - **0** in most cases
  - **-42** in case of cheating  
  Except for cheating, students are strongly encouraged to review together the work that was turned in, to identify mistakes that shouldn’t be repeated.
- During the defence:
  - No **segfault**
  - No other **unexpected, premature, uncontrolled or unexpected termination** of the program  
  Otherwise, the final grade is **0**. Use the appropriate flag.
- You should **never have to edit any file** except the configuration file if it exists. If you want to edit a file:
  - Explicitly explain the reasons with the evaluated student.
  - Make sure both of you are okay with this.
- You must also verify the **absence of memory leaks**:
  - Any memory allocated on the heap must be properly freed before the end of execution.
  - You may use tools such as `leaks`, `valgrind`, or `e_fence`.
  - In case of memory leaks, tick the appropriate flag.

### Attachments

- [subject.pdf](https://github.com/rphlr/42-Subjects/)
- [bircd.tar.gz](https://github.com/rphlr/42-Subjects/)

---

## 3. Mandatory Part

### 3.1 Basic Checks

Verify the following:

- There is a **Makefile**.
- The project **compiles correctly** with the required options.
- It is written in **C++**.
- The executable is called as expected.

Ask and check:

- How many `poll()` (or equivalent) calls are present in the code?  
  - There must be **only one**.
- Verify that `poll()` (or equivalent) is called **every time before** each:
  - `accept`
  - `read`/`recv`
  - `write`/`send`  
  After these calls, `errno` should **not** be used to trigger specific actions (e.g., reading again after `errno == EAGAIN`).
- Verify that each call to `fcntl()` is done as follows:
  ```cpp
  fcntl(fd, F_SETFL, O_NONBLOCK);
  ```
  Any other use of `fcntl()` is **forbidden**.

> If any of these points is wrong, the evaluation ends now and the final mark is **0**.

#### Basic Checks — Result

| Item                                                                                         | Yes | No |
|---------------------------------------------------------------------------------------------|:---:|:--:|
| Makefile present, compiles with required options, C++, correct executable name              |     |    |
| Exactly one `poll()` (or equivalent) in the code                                            |     |    |
| `poll()` called before each `accept`, `read`/`recv`, `write`/`send`; `errno` not misused    |     |    |
| All `fcntl()` calls use: `fcntl(fd, F_SETFL, O_NONBLOCK);` and nothing else                 |     |    |

---

### 3.2 Networking

Check the following requirements:

- The server:
  - Starts correctly.
  - Listens on **all network interfaces** on the port given from the command line.
- Using the `nc` tool:
  - You can connect to the server.
  - You can send commands.
  - The server answers you back.
- Ask the team: what is their **reference IRC client**?
- Using this IRC client:
  - You can connect to the server.
- The server can handle **multiple connections at the same time**:
  - The server should not block.
  - It should be able to answer all demands.
  - Do some tests with the IRC client and `nc` at the same time.
- Join a channel using the appropriate command:
  - Ensure that all messages from one client on that channel are sent to all other clients that joined the channel.

#### Networking — Result

| Requirement                                                                                | Yes | No |
|-------------------------------------------------------------------------------------------|:---:|:--:|
| Server starts and listens on all interfaces on the given port                             |     |    |
| `nc` can connect, send commands, and receive answers                                      |     |    |
| Reference IRC client can connect to the server                                            |     |    |
| Server handles multiple simultaneous connections without blocking                         |     |    |
| Channel join works; messages are broadcast to all clients in the channel                  |     |    |

---

### 3.3 Networking Specials

Network communications can be disturbed by many strange situations.

Using `nc`:

- Try to send **partial commands**:
  - Check that the server answers correctly.
  - With a partial command sent, ensure that **other connections still run fine**.
- **Unexpectedly kill a client**:
  - Check that the server is still operational for:
    - Other connections
    - Any new incoming client
- **Unexpectedly kill a `nc`** with just half of a command sent:
  - Check again that the server is not in an odd state or blocked.
- **Stop a client** (`Ctrl+Z`) connected on a channel:
  - Then **flood the channel** using another client.
  - The server should not hang.
  - When the client is live again, all stored commands should be processed normally.
  - Also, check for **memory leaks** during this operation.

#### Networking Specials — Result

| Test                                                                                       | Yes | No |
|-------------------------------------------------------------------------------------------|:---:|:--:|
| Partial commands sent via `nc`; server responds correctly; other connections unaffected   |     |    |
| Kill a client unexpectedly; server remains operational for others and new clients         |     |    |
| Kill `nc` mid-command; server not in odd state or blocked                                 |     |    |
| `Ctrl+Z` a client on a channel, flood channel, server does not hang, resumed client OK, no leaks |     |    |

---

### 3.4 Client Commands — Basic

With both `nc` and the reference IRC client, check that you can:

- Authenticate
- Set a nickname
- Set a username
- Join a channel  

This should already be fine from previous checks.

Verify that **private messages (`PRIVMSG`)** are fully functional with different parameters.

#### Client Commands (Basic) — Result

| Requirement                                                                                | Yes | No |
|-------------------------------------------------------------------------------------------|:---:|:--:|
| Authentication, nickname, username, and channel join work with `nc` and IRC client        |     |    |
| `PRIVMSG` works correctly with different parameters                                       |     |    |

---

### 3.5 Client Commands — Channel Operator

With both `nc` and the reference IRC client:

- Check that a **regular user** does **not** have privileges to do channel operator actions.
- Then test with an **operator**:
  - All the channel operation commands should be tested.
  - Remove **one point** for each feature that is not working.

Rate this part from **0 (failed)** through **5 (excellent)**.

#### Channel Operator Commands — Score

| Aspect                                                                                     | Score (0–5) | Comments |
|-------------------------------------------------------------------------------------------|:-----------:|----------|
| Regular user cannot perform operator-only actions                                         |             |          |
| Operator can perform all required channel operations                                      |             |          |
| Overall robustness and correctness of operator features                                   |             |          |

**Final score for “Client Commands – Channel Operator”:** ___ / 5

---

## 4. Final Notes & Flags

Use this section to note:

- Any **flags** raised (empty repo, non-functioning program, norm errors, cheating, memory leaks, crashes, etc.).
- Key **observations** and **recommendations** for the student(s).
- Any **agreed modifications** (if any file was edited with mutual consent).

### Flags Raised

- [ ] Empty repository
- [ ] Non-functioning program
- [ ] Norm error
- [ ] Cheating
- [ ] Memory leaks
- [ ] Crash / segfault / unexpected termination
- [ ] Other (specify): ________________________________________

### Additional Comments

<!-- Write free-form feedback here -->



---

**Evaluated by:** ___________________________  
**Date:** ___ / ___ / _____  
**Student(s) evaluated:** _________________________