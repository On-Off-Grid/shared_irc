<div dir="rtl">

# 🤝 Team Collaboration Guide — ft_irc

> [!NOTE]
> هاد الوثيقة ماشي شرح ديال IRC Protocol ولا نظرية ديال الشبكات.
> هادي **وثيقة تنسيق داخلية** بين Ismail و Ayoub و Souhail.
> الهدف ديالها واحد: **شنو كل واحد خاصو من الآخرين، وفوقاش؟**
> قرا هاد الوثيقة قبل ما تبدا تكتب حتى سطر ديال Code.

---

## 📑 الفهرس (Table of Contents)

1. [مقدمة](#1-مقدمة)
2. [Team Overview](#2-team-overview)
3. [Ismail Workflow](#3-ismail-workflow)
4. [Ayoub Workflow](#4-ayoub-workflow)
5. [Souhail Workflow](#5-souhail-workflow)
6. [Development Timeline](#6-development-timeline)
7. [Dependency Graph](#7-dependency-graph)
8. [Interface Exchange](#8-interface-exchange)
9. [Integration Plan](#9-integration-plan)
10. [Git Workflow](#10-git-workflow)
11. [Daily Workflow](#11-daily-workflow)
12. [Code Ownership](#12-code-ownership)
13. [Integration Checklist](#13-integration-checklist)
14. [Communication Rules](#14-communication-rules)
15. [Common Mistakes](#15-common-mistakes)
16. [Final Summary](#16-final-summary)

---

## 1. مقدمة

### علاش قسمنا المشروع؟

ft_irc مشروع كبير فيه Networking و Parsing و IRC Logic كلها مخلوطة. يلا خدمنا كاملين فكلشي، غادي نوقعو فبزاف ديال المشاكل:
- **Merge Conflicts** كل يوم.
- واحد يبدل Interface ويهرس خدمة الآخر.
- خدمة معاودة (Duplicate Work) وقت ضايع.

الحل هو: **كل واحد ياخد جزء واضح ديال المشروع ويكون Owner عليه.** هاكا كل واحد كيركز على الخدمة ديالو بلا ما يتداخل مع الآخرين.

### علاش كل واحد عندو Ownership؟

الـ Ownership كيعني: **نتا المسؤول الأول والأخير على هاد الـ Class أو الـ Module.** يلا وقع Bug فيه، نتا اللي كتصلحو. يلا بغا شي واحد يبدلو، خاصو يهضر معاك.

هادشي مهم حيت:
- كل واحد كيعرف فين يخدم بلا ما يسول.
- ما كاين حتى واحد غادي يبدل كود ماشي ديالو بالغلط.
- الـ Code Review كيكون واضح: Owner ديال Class هو اللي كيراجع أي تغيير فيه.

### علاش خاص التواصل يكون واضح؟

حيت حنا 3 ديال الدراري كنخدمو على نفس المشروع، ولكن كل واحد كيخدم على Module مختلف. يلا بدلتي شي حاجة بلا ما تعلم الآخرين:
- ممكن تهرس الـ Build ديالهم.
- ممكن تخلق Circular Dependency.
- ممكن تضيع ساعات ديال خدمتهم.

**القاعدة الذهبية: يلا بغيتي تبدل شي Interface، هضر مع الفريق أولا.**

### علاش Interfaces أهم من Implementation؟

الـ Interface هي الـ Contract بين الـ Modules. يعني: الـ Method signatures والـ Return types والـ Parameters اللي كل واحد كيعتمد عليها.

مثال بسيط:
- Ismail كيعيط `BufferManager::extractMessage()` فالكود ديالو.
- يلا Souhail بدل اسم هاد الـ Method ولا الـ Parameters ديالها بلا ما يعلم Ismail، الكود ديال Ismail ما غاديش يـ Compile.

**الحل:** ثبتو الـ Interfaces من البداية. الـ Implementation الداخلية كل واحد حر فيها، ولكن الـ Interface مقدسة — ما تتبدلش بلا اتفاق.

---

## 2. Team Overview

| Developer | Main Responsibility | Depends On | Provides To Others | Expected Deliverables |
|---|---|---|---|---|
| **Ismail** (Lead Architect) | هندسة الـ Server, Client, Channel, Dispatcher, Replies | Ayoub (Socket + poll), Souhail (Buffer + Parser) | Server Skeleton, Client Object, Channel Object, Interfaces, Architecture decisions | `Server.hpp/.cpp`, `Client.hpp/.cpp`, `Channel.hpp/.cpp`, `main.cpp`, Dispatcher, Replies |
| **Ayoub** | الـ Socket Layer والـ Event Loop (`poll()`) | Ismail (Server Skeleton + Interfaces) | Socket مهيأ, `poll()` loop خدام, `accept()`, `send()` | `Socket.hpp/.cpp`, `PollManager.hpp/.cpp` |
| **Souhail** | الـ Buffer والـ Parser والـ Makefile والـ Utils | Ismail (Client Interface), Ayoub (بلا dependency مباشرة) | BufferManager خدام, Parser خدام, Makefile, Testing scripts | `BufferManager.hpp/.cpp`, `Parser.hpp/.cpp`, `Makefile`, `tests/` |

> [!IMPORTANT]
> كل Developer خاصو يعرف بالضبط شنو كيتسنى من الآخرين وشنو خاصو يسلم. هاد الجدول هو المرجع ديالك يلا شكيتي.

---

## 3. Ismail Workflow

### شنو خاص Ismail يدير؟

Ismail هو الـ **Lead Architect** ديال المشروع. الخدمة ديالو هي بناء القلب النابض ديال الـ Server:

1. **Server Class:** الـ Brain ديال المشروع كامل — `init()`, `run()`, `shutdown()`.
2. **Client Class:** تمثيل كل مستخدم مكونيكطي — Registration state (PASS → NICK → USER), Nickname, FD.
3. **Channel Class:** إدارة القنوات — Membership, Operators, `broadcast()`.
4. **Command Dispatcher:** ربط كل أمر IRC (بحال `JOIN`, `NICK`, `PRIVMSG`) بالـ Handler المناسب.
5. **Replies:** بناء الـ IRC Numerics (بحال `001 RPL_WELCOME`).
6. **Integration:** دمج خدمة Ayoub (Socket + poll) وخدمة Souhail (Buffer + Parser) داخل Server.
7. **Code Review:** مراجعة الـ PRs ديال Ayoub و Souhail والتأكد أنها كتتوافق مع الـ Architecture.
8. **Decision Making:** اتخاذ القرارات التقنية الصعبة (مثلا Object Ownership, Partial Messages).

### شنو خاص يسنا من Ayoub؟

| شنو كيتسنا | علاش خاصو ليه | فوقاش خاصو يوصل |
|---|---|---|
| `Socket.hpp` + `Socket.cpp` | باش الـ Server يقدر يدير `bind()` و `listen()` على الـ Port | Phase 2 (Socket Layer) |
| `PollManager.hpp` + `PollManager.cpp` | باش `Server::run()` يقدر يعيط لـ `poll()` ويتسنى الـ Events | Phase 3 (Event Loop) |
| `accept()` function | باش يقبل الـ Clients الجداد ويرجع الـ FD | Phase 3 |
| `send()` function | باش يصيفط الـ Replies للـ Clients | Phase 3 |

**ملاحظة مهمة:** Ismail كيتعامل مع خدمة Ayoub بحال **Black Box** — كيستعمل الـ Interface بلا ما يتدخل فالـ Implementation الداخلية.

### شنو خاص يسنا من Souhail؟

| شنو كيتسنا | علاش خاصو ليه | فوقاش خاصو يوصل |
|---|---|---|
| `Makefile` | باش المشروع يـ Compile من البداية | Phase 1 (Skeleton) |
| `BufferManager.hpp` + `BufferManager.cpp` | باش كل Client يخبي الداتا اللي جات مقطعة ويجبد منها الرسائل الكاملة عبر `\r\n` (CRLF) | Phase 4 (Buffers) |
| `Parser.hpp` + `Parser.cpp` | باش ياخد الرسالة الكاملة ويرجع Object فيه Command + Parameters | Phase 4 (Parsing) |

### شنو خاص يعطي ليهم؟

| شنو كيعطي | لشكون | علاش |
|---|---|---|
| `Server.hpp` (Skeleton) | Ayoub + Souhail | باش يعرفو كيفاش الـ Server مبني ويقدرو يدمجو الخدمة ديالهم |
| `Client.hpp` (Interface) | Souhail | باش الـ BufferManager يعرف كيفاش يتخزن فالـ Client |
| Architecture decisions | Ayoub + Souhail | باش ما يديروش حاجة كتخالف الـ Design |
| Code Reviews | Ayoub + Souhail | باش الكود يبقى نقي ومتوافق |

### شنو الملفات لي مسؤول عليهم؟

```
includes/Server.hpp       ← Ismail
includes/Client.hpp       ← Ismail
includes/Channel.hpp      ← Ismail
src/main.cpp              ← Ismail
src/Server.cpp            ← Ismail
src/Client.cpp            ← Ismail
src/Channel.cpp           ← Ismail
```

### شنو الـ Interfaces خاصو يثبت؟

Ismail هو اللي كيحدد الـ Interfaces اللي الآخرين غادي يعتمدو عليها:
- `Server::init(port, password)` — كيفاش الـ Server كيبدا.
- `Server::run()` — الـ Main Loop.
- `Client` constructors و methods (مثلا `getNickname()`, `setRegistered()`, `getFd()`).
- `Channel` public methods (مثلا `addMember()`, `removeMember()`, `broadcast()`).

> [!WARNING]
> هاد الـ Interfaces ملي تتحدد فالبداية، **ممنوع يتبدلو بلا اجتماع مع الفريق كامل.** حيت Ayoub و Souhail بانيين الخدمة ديالهم عليها.

### شنو ممنوع يبدل بلا اتفاق؟

- ❌ **Parameters** ديال أي public method فالـ Classes ديالو اللي الآخرين كيستعملوها.
- ❌ **Return types** ديال الـ Methods المشتركة.
- ❌ **هيكلة الـ Folders** (مثلا يزيد مجلد جديد بلا ما يعلم).
- ❌ أي تغيير فالـ `Makefile` (حيت Souhail هو Owner ديالو).
- ❌ أي تعديل فالملفات ديال Ayoub ولا Souhail.

---

## 4. Ayoub Workflow

### شنو الخدمة ديالو؟

Ayoub مسؤول على **الطبقة السفلية ديال الشبكة** — كلشي يتعلق بالـ Sockets والـ Event Loop:

1. **Socket Class:** إنشاء الـ Socket, `bind()`, `listen()`, `accept()`, `close()`.
2. **PollManager Class:** تسيير `poll()` loop — إضافة/حذف الـ FDs, انتظار الـ Events.
3. **Non-blocking I/O:** التأكد بلي كاع الـ Sockets مديور ليهم `fcntl(O_NONBLOCK)`.
4. **Connection handling:** `accept()` للـ Clients الجداد و Detection ديال Disconnect.
5. **`send()` wrapper:** صيفط الداتا للـ Client عبر الـ Socket.

### شنو خاص Ismail يعطيه؟

| شنو كيتسنا من Ismail | علاش خاصو ليه | فوقاش |
|---|---|---|
| `Server.hpp` Skeleton | باش يعرف كيفاش الـ Server غادي يستعمل الـ Socket Layer ديالو | Phase 1 |
| Server Interface (كيفاش `Server::run()` غادي يعيط لـ `poll()`) | باش يبني الـ PollManager بشكل متوافق | Phase 2 |
| الـ Architecture decisions حول Connection/Disconnection | باش يعرف فوقاش يدير `accept()` وفوقاش يسد الـ FD | Phase 3 |

### شنو خاص Souhail يعطيه؟

Ayoub ما عندوش Dependency مباشرة على Souhail. الـ Data Flow كتمشي هكا:

```
Socket (Ayoub) → recv() → Buffer (Souhail)
```

ولكن Ayoub ما كيعيطش لـ BufferManager مباشرة — الـ Server (Ismail) هو اللي كيدير الربط بيناتهم. داكشي علاش Ayoub ما خاصو يتسنى والو من Souhail بشكل مباشر.

### شنو خاص يرجع ليهم؟

| شنو كيعطي | لشكون | علاش |
|---|---|---|
| `Socket.hpp` + `Socket.cpp` | Ismail | باش يدمجهم فالـ Server |
| `PollManager.hpp` + `PollManager.cpp` | Ismail | باش `Server::run()` يقدر يعيط لـ `poll()` |
| `accept()` — FD ديال Client جديد | Ismail | باش يخلق `Client` object جديد |
| FD Disconnect detection | Ismail | باش يعرف فوقاش يدير Cleanup للـ Client |

### شنو الـ Interfaces خاص يحترم؟

- **Server Interface:** يلا Ismail قال بلي `Server::run()` كيعيط لـ `pollManager.wait_for_events()`، Ayoub خاصو يوفر هاد الـ Method بهاد الـ Signature بالضبط.
- **Client Interface:** Ayoub ما خاصوش يعدل مباشرة على Client objects — يرجع غير الـ FD وخلي Ismail يتصرف.
- **ممنوع** يعيط لـ Parser ولا لـ BufferManager مباشرة — هاد الـ Dependency كتكسر الـ Architecture.

### شنو مسؤول عليه؟

```
includes/Socket.hpp       ← Ayoub
includes/PollManager.hpp  ← Ayoub
src/Socket.cpp            ← Ayoub
src/PollManager.cpp       ← Ayoub
```

---

## 5. Souhail Workflow

### شنو الخدمة ديالو؟

Souhail مسؤول على **معالجة البيانات** — من الـ Raw bytes اللي كتوصل من `recv()` حتى تولي IRC Command مفهومة:

1. **BufferManager Class:** تخزين الداتا المقطعة (Partial data) وجبد الرسائل الكاملة عبر `\r\n` (CRLF).
2. **Parser Class:** أخد الرسالة الكاملة وتقسيمها لـ Command + Parameters.
3. **Makefile:** بناء المشروع كامل — `make`, `make clean`, `make fclean`, `make re`.
4. **Utils/Helpers:** أي وظائف مساعدة مشتركة.
5. **Testing scripts:** السكربتات ديال الاختبار فمجلد `tests/`.

### شنو خاص Ismail يعطيه؟

| شنو كيتسنا من Ismail | علاش خاصو ليه | فوقاش |
|---|---|---|
| `Client.hpp` Interface | باش يعرف كيفاش الـ BufferManager غادي يتخزن فالـ Client (كـ member) | Phase 1-2 |
| الـ Architecture decisions حول Data Flow | باش يعرف بالضبط شنو كيدخل للـ Buffer وشنو كيخرج من الـ Parser | Phase 2 |
| Server Skeleton | باش يعرف كيفاش الـ Server غادي يعيط لـ BufferManager و Parser | Phase 1 |

### شنو خاص Ayoub يعطيه؟

Souhail ما عندوش Dependency مباشرة على Ayoub. الداتا كتوصل لـ BufferManager عبر الـ Server (Ismail). Souhail ما خاصوش يعرف شنو هو الـ Socket ولا `poll()` — هو كيتعامل غير مع `std::string` ديال الداتا.

### شنو خاص يرجع؟

| شنو كيعطي | لشكون | علاش |
|---|---|---|
| `Makefile` | Ismail + Ayoub | باش المشروع يـ Compile من البداية |
| `BufferManager.hpp` + `BufferManager.cpp` | Ismail | باش يدمجو فالـ Client |
| `Parser.hpp` + `Parser.cpp` | Ismail | باش يستعملو فالـ Dispatcher |
| Testing scripts | الفريق كامل | باش نتأكدو بلي كلشي خدام |

### شنو الملفات ديالو؟

```
includes/BufferManager.hpp  ← Souhail
includes/Parser.hpp         ← Souhail
src/BufferManager.cpp       ← Souhail
src/Parser.cpp              ← Souhail
Makefile                    ← Souhail
tests/                      ← Souhail
```

### شنو الـ Interfaces خاص يحترم؟

- **Client Interface:** الـ BufferManager خاصو يخدم كـ member داخل الـ Client — Souhail خاصو يصمم الـ BufferManager باش يكون self-contained وما يعرفش شنو هو الـ Client.
- **Server Interface:** الـ Parser خاصو يرجع Object فيه `command` و `params` بشكل واضح — Ismail هو اللي غادي يستعملو فالـ Dispatcher.
- **ممنوع** الـ BufferManager يعرف شنو هو الـ Client ولا الـ Server — خاصو يتعامل غير مع `std::string`.
- **ممنوع** الـ Parser يعيط لأي حاجة فالـ Server ولا الـ Socket — هو كياخد String وكيرجع Parsed result وصافي.

---

## 6. Development Timeline

### Week 1: الساس (Foundation)

```text
Ismail ──────── Server Skeleton + main.cpp + Server.hpp/Client.hpp
                    │
                    │ (كيوفر الـ Skeleton والـ Interfaces)
                    ▼
Souhail ─────── Makefile + BufferManager Skeleton + Parser Skeleton
                    │
                    │ (Makefile واجد — المشروع يقدر يـ Compile)
                    ▼
Ayoub ──────── Socket.hpp + Socket.cpp (bind, listen, accept)
```

**علاش هاد الترتيب؟**
- Ismail خاصو يبدا الأول حيت هو اللي كيحدد الـ Architecture والـ Interfaces. بلا Server Skeleton، الآخرين ما يعرفوش كيفاش يبنيو الخدمة ديالهم.
- Souhail كيبدا معاه فنفس الوقت حيت الـ Makefile ما عندو dependency على حد، والـ Skeletons ديال Buffer و Parser كيقدر يبداهم بالـ Interface اللي عطاه Ismail.
- Ayoub كيبدا ملي الـ Server Skeleton يكون واجد باش يعرف كيفاش الـ Socket غادي يتدمج.

### Week 2: الاتصال والبيانات (Connections & Data)

```text
Ayoub ──────── PollManager + Event Loop + Non-blocking
                    │
                    │ (poll() خدام — Server يقدر يقبل Clients)
                    ▼
Ismail ──────── Client.cpp + Server::run() + Integration مع poll()
                    │
                    │ (الـ Server كيقبل ويسجل Clients)
                    ▼
Souhail ─────── BufferManager.cpp + Parser.cpp كاملين
```

**علاش هاد الترتيب؟**
- Ayoub خاصو يكمل `poll()` أولا حيت بلاه الـ Server ما يقدر يقبل Clients متعددين.
- Ismail كيدمج `poll()` فالـ Server ويبني Client logic.
- Souhail كيكمل الـ Buffer والـ Parser بشكل مستقل — حيت كيخدم على `std::string` وما خاصوش يتسنى حد.

### Week 3: الأوامر والقنوات (Commands & Channels)

```text
Ismail ──────── Channel.cpp + Dispatcher + Handlers (JOIN, PRIVMSG, KICK...)
                    │
Souhail ─────── Integration Testing + Edge cases فالـ Parser
                    │
Ayoub ──────── Disconnect handling + FD cleanup + send() polishing
```

### Week 4: التلميع والتقييم (Polish & Evaluation)

```text
الفريق كامل ── Memory leaks (Valgrind) + Edge cases + Documentation + Peer Evaluation prep
```

---

## 7. Dependency Graph

هادي هي خريطة الاعتماديات — شكون كيعتمد على شكون:

```text
                        ┌──────────────────────┐
                        │    main.cpp           │
                        │    (Ismail)           │
                        └──────────┬───────────┘
                                   │
                                   ▼
                        ┌──────────────────────┐
                        │      Server           │
                        │    (Ismail)           │
                        └──────────┬───────────┘
                                   │
                  ┌────────────────┼────────────────┐
                  │                │                 │
                  ▼                ▼                 ▼
        ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
        │ SocketManager│  │ ClientManager│  │   Channel    │
        │   (Ayoub)    │  │  (Ismail)    │  │  (Ismail)    │
        └──────┬───────┘  └──────┬───────┘  └──────────────┘
               │                 │
               ▼                 ▼
        ┌──────────────┐  ┌──────────────┐
        │  PollManager │  │BufferManager │
        │   (Ayoub)    │  │  (Souhail)   │
        └──────┬───────┘  └──────┬───────┘
               │                 │
               ▼                 ▼
          poll() loop      ┌──────────────┐
               │           │    Parser    │
               │           │  (Souhail)   │
               │           └──────┬───────┘
               │                  │
               │                  ▼
               │           ┌──────────────┐
               │           │  Dispatcher  │
               │           │  (Ismail)    │
               │           └──────┬───────┘
               │                  │
               │                  ▼
               │           Command Handlers
               │                  │
               │                  ▼
               │              Replies
               │                  │
               └──────────────────┘
                        │
                        ▼
                     send()
                    (Ayoub)
```

### قواعد الاعتماد الممنوعة

```text
 ✅ Server → SocketManager      (مسموح — Server كيستعمل Sockets)
 ✅ Server → BufferManager      (مسموح — عبر الـ Client)
 ✅ Client → BufferManager      (مسموح — كل Client عندو Buffer ديالو)

 ❌ SocketManager → Parser      (ممنوع — الـ Socket ما خاصوش يعرف الـ Parser)
 ❌ BufferManager → Client      (ممنوع — الـ Buffer ما خاصوش يعرف الـ Client)
 ❌ Parser → Server             (ممنوع — الـ Parser ما خاصوش يعيط للـ Server)
 ❌ PollManager → Channel       (ممنوع — كل طبقة كتعرف غير الطبقة اللي تحتها)
```

> [!CAUTION]
> **الاعتماد الدائري (Circular Dependency)** هو أخطر مشكل كيقدر يوقع. يلا `A.hpp` كي include `B.hpp` و `B.hpp` كي include `A.hpp`، الـ Compiler غادي يتبلوكا. استعمل `class X;` (Forward Declaration) فالـ Headers.

---

## 8. Interface Exchange

### Ismail كيوفر:

| Interface | الملف | شنو فيه | شكون كيستعملو |
|---|---|---|---|
| **Server** | `Server.hpp` | `init(port, password)`, `run()`, Destructor | Ayoub (باش يعرف كيفاش الـ Socket كيتدمج), Souhail (باش يفهم الـ Architecture) |
| **Client** | `Client.hpp` | `getFd()`, `getNickname()`, `setRegistered()`, Registration state | Souhail (باش الـ BufferManager يتخزن كـ member) |
| **Channel** | `Channel.hpp` | `addMember()`, `removeMember()`, `broadcast()` | الفريق كامل فـ Phase 3+ |

### Ayoub كيوفر:

| Interface | الملف | شنو فيه | شكون كيستعملو |
|---|---|---|---|
| **Socket** | `Socket.hpp` | `create()`, `bind()`, `listen()`, `accept()`, `close()` | Ismail (فـ `Server::init()`) |
| **PollManager** | `PollManager.hpp` | `addFd()`, `removeFd()`, `wait_for_events()` | Ismail (فـ `Server::run()`) |
| **send()** | عبر الـ Socket | صيفط الداتا للـ Client | Ismail (باش يصيفط الـ Replies) |

### Souhail كيوفر:

| Interface | الملف | شنو فيه | شكون كيستعملو |
|---|---|---|---|
| **BufferManager** | `BufferManager.hpp` | `appendData(raw_string)`, `hasCompleteMessage()`, `extractMessage()` | Ismail (كيخزنو فالـ Client ويعيط ليه ملي `recv()` كتجيب داتا) |
| **Parser** | `Parser.hpp` | `parse(complete_message)` → يرجع Object فيه Command + Params | Ismail (كيعطي الـ Result للـ Dispatcher) |
| **Makefile** | `Makefile` | `make`, `make clean`, `make fclean`, `make re` | الفريق كامل |

> [!TIP]
> **القاعدة:** كل واحد كيوفر الـ `.hpp` (Interface) أولا قبل ما يبدا الـ `.cpp` (Implementation). هاكا الآخرين يقدرو يبداو يكتبو الكود اللي كيعتمد على الـ Interface بلا ما يتسناو الـ Implementation.

---

## 9. Integration Plan

### Milestone 1: Server Skeleton + Makefile ✅

| | التفاصيل |
|---|---|
| **Goal** | المشروع كيـ Compile وعندو Structure واضح |
| **Required from** | Ismail (`Server.hpp` Skeleton, `main.cpp`), Souhail (`Makefile`) |
| **Verification** | `make` كيـ Compile بلا أخطاء, `./ircserv 6667 pass` كيقرا الـ Arguments ويسد |

---

### Milestone 2: Socket Layer

| | التفاصيل |
|---|---|
| **Goal** | الـ Server كيحل Port ويتسنى الاتصالات |
| **Required from** | Ayoub (`Socket.hpp/.cpp`) |
| **Verification** | `./ircserv 6667 pass` + `nc localhost 6667` → الاتصال كينجح |
| **Integration** | Ismail كيدمج `Socket` فـ `Server::init()` |

---

### Milestone 3: Event Loop (poll)

| | التفاصيل |
|---|---|
| **Goal** | الـ Server كيقبل بزاف ديال الـ Clients فنفس الوقت بدون Blocking |
| **Required from** | Ayoub (`PollManager.hpp/.cpp`), Ismail (`Client.hpp/.cpp`, `Server::run()`) |
| **Verification** | 10 ديال `nc` مكونيكطين فنفس الوقت، الـ Server ما كيتبلوكاش |

---

### Milestone 4: Buffers

| | التفاصيل |
|---|---|
| **Goal** | الـ Server كيخبي الداتا المقطعة وكيجبد الرسائل الكاملة |
| **Required from** | Souhail (`BufferManager.hpp/.cpp`) |
| **Verification** | صيفط رسالة مقطعة على جوج (مثلا `NICK ism` عاد `ail\r\n`) — الـ Server كيفهمها صحيح |
| **Integration** | Ismail كيخزن الـ BufferManager فالـ Client object |

---

### Milestone 5: Parser

| | التفاصيل |
|---|---|
| **Goal** | الـ Server كيفهم الأوامر ديال IRC |
| **Required from** | Souhail (`Parser.hpp/.cpp`) |
| **Verification** | `NICK ismail\r\n` كيتحول لـ `{command: "NICK", params: ["ismail"]}` |
| **Integration** | Ismail كيعطي الـ Parsed result للـ Dispatcher |

---

### Milestone 6: Registration (PASS + NICK + USER)

| | التفاصيل |
|---|---|
| **Goal** | الـ Client كيقدر يتسجل فالـ Server ويرجع ليه `001 RPL_WELCOME` |
| **Required from** | Ismail (Registration logic, Reply), Souhail (Parser), Ayoub (send) |
| **Verification** | `irssi` ولا `nc` كيتكونيكطا ويتسجلو بنجاح |

---

### Milestone 7: Channels + Commands

| | التفاصيل |
|---|---|
| **Goal** | `JOIN`, `PRIVMSG`, `PART`, `TOPIC`, `KICK`, `INVITE`, `MODE` خدامين |
| **Required from** | Ismail (Channel, Dispatcher, Handlers) |
| **Verification** | جوج Clients كيدخلو لـ Channel ويتهاضرو |

---

### Milestone 8: Final Polish

| | التفاصيل |
|---|---|
| **Goal** | صفر Memory leaks, صفر Crashes, Documentation كاملة |
| **Required from** | الفريق كامل |
| **Verification** | `valgrind --leak-check=full`, Edge cases, Peer Evaluation prep |

---

## 10. Git Workflow

### Feature Branches (الفروع ديال الخدمة)

كل خدمة جديدة خاصها تتدار فـ Branch خاص بيها. **ممنوع تخدم مباشرة فـ `main`.**

تسمية الـ Branches:
- `feature/server-core` — لـ Ismail
- `feature/socket-layer` — لـ Ayoub
- `feature/parser` — لـ Souhail
- `fix/client-disconnect` — إصلاح Bug
- `docs/update-handbook` — تحديث الوثائق

**علاش Feature Branches؟** باش يلا وقع Bug فالخدمة ديالك، ما يأثرش على `main`. الـ `main` خاصو يبقى **ديما خدام ونقي**.

### Pull Requests (طلبات المراجعة)

ملي تكمل الخدمة فالـ Branch ديالك:
1. دير `git push origin feature/your-branch`.
2. حل **Pull Request** فـ GitHub.
3. اكتب وصف واضح: شنو درتي، علاش، وشنو خاص يتراجع.

**علاش PR وماشي Merge مباشر؟** باش واحد من الفريق يراجع الكود ديالك ويتأكد بلي ما كيهرسش حتى حاجة.

### Reviews (المراجعة)

- **Owner ديال الـ Class المعني** هو اللي خاصو يراجع. مثلا يلا الـ PR كيأثر على `Server.hpp`، Ismail خاصو يراجع.
- الـ Reviewer كيشوف: واش كيـ Compile؟ واش C++98؟ واش الـ Interface ما تبدلاتش بلا اتفاق؟
- يلا كلشي مزيان، كيدير **Approve**.
- يلا كاين مشكل، كيكتب Comment وكيرجع الـ PR للمطور.

### Merge (الدمج)

ملي الـ PR يتوافق عليه:
1. دير **Merge** لـ `main`.
2. **امسح الـ Branch** القديمة (`git branch -d feature/your-branch`) باش الـ Repo يبقى نقي.

### Sync (التحديث قبل الخدمة)

**كل يوم قبل ما تبدا تخدم:**
```bash
git checkout main
git pull origin main
git checkout feature/your-branch
git merge main
```

**علاش؟** باش تاخد آخر التحديثات ديال الدراري وما يتراكموش عليك الـ Conflicts.

---

## 11. Daily Workflow

### الصباح (Morning)

```text
1. git pull origin main                    ← تاخد آخر التحديثات
2. Discord check                           ← شوف واش شي واحد كتب شي حاجة مهمة
3. اقرا الـ Issue اللي خدام عليها          ← فكر شنو غادي تدير اليوم
4. git checkout feature/your-branch        ← رجع للـ Branch ديالك
5. git merge main                          ← دمج التحديثات الجديدة
```

### الخدمة (Coding)

```text
6. اكتب الكود                              ← ركز على Task وحدة وواضحة
7. جرب (test يدوي بـ nc)                   ← تأكد بلي خدام
8. git add + git commit                    ← Commits صغار بـ رسائل واضحة
   مثلا: feat: add Client registration state
```

> [!TIP]
> دير Commits صغار ومتكررين. أحسن من Commit واحد كبير فيه 500 سطر — حيت يلا وقع Bug، نقدرو نرجعو بسهولة.

### آخر النهار (End of Day)

```text
9.  git push origin feature/your-branch    ← صيفط الخدمة ديالك
10. حل Pull Request (يلا الخدمة سالات)     ← طلب المراجعة
11. راجع PRs ديال الدراري                   ← عطي Feedback
12. يلا تمت المراجعة → Merge               ← دمج فـ main
13. Daily sync فـ Discord                   ← شنو كملتي؟ شنو بلوكاج؟
```

### الاجتماع اليومي (5 دقايق فـ Discord)

كل واحد كيجاوب على 4 ديال الأسئلة:
1. **شنو خدمت البارح؟**
2. **شنو باقي ليا اليوم؟**
3. **واش عندي شي Blocker؟** (شي حاجة كتمنعني نتقدم)
4. **واش خاصني شي واحد يساعدني؟**

**علاش هاد الاجتماع؟** باش ما يبقاش شي واحد معطل أسبوع كامل على مشكل يقدر واحد فينا يحلو ليه ف 5 دقايق.

---

## 12. Code Ownership

| Class | Owner | شكون يقدر يعدلو | Approval required |
|---|---|---|---|
| `Server` | Ismail | Ismail فقط | لا (هو Owner) |
| `Client` | Ismail | Ismail فقط | لا (هو Owner) |
| `Channel` | Ismail | Ismail فقط | لا (هو Owner) |
| `Reply` | Ismail | Ismail فقط | لا (هو Owner) |
| `CommandDispatcher` | Ismail | الفريق كامل (لإضافة Handlers) | ✅ Ismail خاصو يوافق |
| `Socket` | Ayoub | Ayoub فقط | لا (هو Owner) |
| `PollManager` | Ayoub | Ayoub فقط | لا (هو Owner) |
| `BufferManager` | Souhail | Souhail فقط | لا (هو Owner) |
| `Parser` | Souhail | Souhail فقط | لا (هو Owner) |
| `Makefile` | Souhail | Souhail + يلا خاص يتزاد ملف → الفريق كيعلم Souhail | ✅ Souhail خاصو يوافق |
| `main.cpp` | Ismail | Ismail فقط | لا (هو Owner) |
| `tests/` | Souhail | الفريق كامل يقدر يزيد Tests | لا (ولكن ما تمسحش Tests ديال شي واحد آخر) |

> [!IMPORTANT]
> **القاعدة الأساسية:** ما تعدلش ملف ماشي ديالك بلا ما تهضر مع Owner ديالو. حتى يلا كان تعديل بسيط — PR + Review أولا.

---

## 13. Integration Checklist

قبل ما تحل Pull Request ولا تدير Merge لـ `main`، **خاصك تتأكد من هاد الحاجات كلها:**

```text
✔ Compiles          — make كتـ Compile بلا أخطاء
✔ C++98             — ما كاين لا auto, لا nullptr, لا range-based for
✔ No warnings       — -Wall -Wextra -Werror كلهم مفعلين وصفر تحذير
✔ No leaks          — valgrind --leak-check=full صافي
✔ No crashes        — الـ Server ما كيطيحش حتى مع Input خاطئ
✔ Non-blocking      — كاع الـ Sockets مديور ليهم fcntl(O_NONBLOCK)
✔ poll() works      — ما كاين حتى blocking call فالـ Main Loop
✔ Interfaces        — ما بدلتي حتى Interface بلا اتفاق مع الفريق
✔ OCF               — كاع الـ Classes فيهم Constructor, Destructor,
                      Copy Constructor, Assignment Operator
✔ No globals        — صفر Global Variables
✔ Documentation     — يلا بدلتي شي Interface, حدث الوثائق
✔ Reviewed          — واحد على الأقل من الفريق دار Review
```

> [!WARNING]
> **يلا حتى شرط واحد ما تحققش، الـ PR ما خاصوش يتدمج فـ `main`.** الـ `main` خاصو يبقى ديما خدام ونقي.

---

## 14. Communication Rules

### فوقاش تسول سؤال؟

- **فـ Discord:** يلا كان السؤال بسيط وما خاصوش جواب فوري (مثلا "واش هاد الـ Method كترجع `int` ولا `bool`؟").
- **فـ GitHub Issue:** يلا كان السؤال كبير وخاصو نقاش (مثلا "واش نستعملو `std::map` ولا `std::vector` للـ Channels؟").

### فوقاش تعيط (مكالمة)؟

- يلا كان عندك **Blocker** كيمنعك تخدم.
- يلا لقيتي **Bug** فكود واحد آخر وما فهمتيش السبب.
- يلا بغيتي **تبدل Interface** وخاصك توافق مع الفريق.

### فوقاش تحل Issue فـ GitHub؟

- يلا لقيتي **Bug** خاصو يتصلح.
- يلا عندك **Feature request** (ميزة جديدة خاص نزيدوها).
- يلا بغيتي **تقترح تغيير** فالـ Architecture.

### فوقاش تبدل Interface؟

**غير يلا اتفقتو كاملين.** الخطوات:
1. حل Issue فـ GitHub وشرح علاش بغيتي تبدل.
2. ناقش مع Owner ديال الـ Classes المتأثرة.
3. اتفقو على الـ Interface الجديدة.
4. كل واحد يحدث الكود ديالو.
5. حدث الوثائق.

### فوقاش **ما خاصكش** تعدل كود واحد آخر؟

- **ديما.** ما تعدل كود ماشي ديالك. يلا شفتي Bug فكود Ayoub:
  1. اكتب Comment فالـ PR ولا عيط ليه.
  2. خليه هو يصلح وهو يدير الـ Commit.
  3. هاكا الـ `git blame` كيبقى واضح وكل واحد مسؤول على الكود ديالو.

**الاستثناء الوحيد:** يلا كان الـ Owner غايب ولقيتي Bug كيكسر الـ Build ديال `main` — صلحو ودير PR وخلي Owner يراجع ملي يرجع.

---

## 15. Common Mistakes

### ❌ الخطأ 1: تبديل الـ Interface بلا ما تعلم الفريق

**مثال:** Souhail بدل `Parser::parse()` من `parse(std::string)` لـ `parse(std::string, int)`.
**النتيجة:** الكود ديال Ismail اللي كيعيط لـ `parse()` ما بقاش يـ Compile.
**علاش خطير:** حيت ضيعتي وقت شخص آخر على مشكل نتا اللي خلقتيه.
**الحل:** عمرك ما تبدل الـ Signature ديال Method عامة (public) بلا ما تعلم كل من كيستعملها.

### ❌ الخطأ 2: الخدمة المعاودة (Duplicate Work)

**مثال:** Ismail صاوب Parser بسيط فالـ Server ديالو حيت Souhail ما كملش.
**النتيجة:** ولاو عندهم 2 ديال الـ Parsers وما حد عارف أنهي يستعمل.
**علاش خطير:** كيضيع الوقت وكيخلق Confusion.
**الحل:** يلا تسناوك على حاجة، عيط لـ Owner ديالها واسولو فوقاش غتكون واجدة. ما تعاودش الخدمة ديالو.

### ❌ الخطأ 3: بدل نفس الملف فنفس الوقت

**مثال:** Ismail و Ayoub بدلو `Server.cpp` فنفس الوقت فـ Branches مختلفين.
**النتيجة:** Merge Conflict كبير وصعب يتحل.
**علاش خطير:** كيقدر يضيع نص يوم ديال خدمة.
**الحل:** كل واحد يخدم غير فالملفات ديالو (شوف جدول الـ Code Ownership). يلا ضروري تعدل ملف مشترك، نسقو مع بعضياتكم.

### ❌ الخطأ 4: Circular Dependency

**مثال:** `Client.hpp` كي include `Channel.hpp` و `Channel.hpp` كي include `Client.hpp`.
**النتيجة:** الـ Compiler كيتبلوكا ويعطيك أخطاء غير مفهومة.
**علاش خطير:** كيقدر ياخد ساعات باش تلقى المشكل.
**الحل:** استعمل Forward Declaration (`class Channel;`) فالـ Headers. دير `#include` غير فالـ `.cpp` files.

### ❌ الخطأ 5: كسر الـ Build ديال `main`

**مثال:** واحد دار Merge لـ `main` وما جربش — الكود ما كي Compileش.
**النتيجة:** الفريق كامل معطل حيت ما يقدروش يديرو `git pull` ويخدمو.
**علاش خطير:** كتعطل 3 ديال الناس بدل واحد.
**الحل:** **ديما** جرب بـ `make` قبل ما تدير Merge. الـ Integration Checklist كاملة خاصها تكون محققة.

### ❌ الخطأ 6: الخدمة مباشرة فـ `main` Branch

**مثال:** Ayoub كيكتب الكود مباشرة فـ `main` بلا ما يصاوب Feature Branch.
**النتيجة:** يلا هرس شي حاجة، ما يقدرش يرجع لورا بسهولة.
**علاش خطير:** الـ `main` خاصو يكون ديما Stable. يلا كتكتب فيه مباشرة، أي غلطة كتأثر على الفريق كامل.
**الحل:** **ديما** صاوب Feature Branch قبل ما تبدا تكتب.

### ❌ الخطأ 7: ما تجربش قبل الـ PR

**مثال:** Souhail حل PR بلا ما يجرب الكود بـ `make` ولا `valgrind`.
**النتيجة:** الـ PR فيه Bugs والـ Reviewer خاصو يرجعو، وقت ضايع لجوج.
**علاش خطير:** كيبطئ الفريق كامل.
**الحل:** قبل ما تحل PR، مر على الـ Integration Checklist كاملة — Compile, No warnings, No leaks, Tests.

---

## 16. Final Summary

```text
┌─────────────────────────────────────────────────────┐
│                                                     │
│    إذا كل واحد احترم الـ Ownership ديالو...         │
│                                                     │
│    واحترم الـ Interfaces ديال الآخرين...             │
│                                                     │
│    واحترم الـ Workflow (Branch → PR → Review →       │
│    Merge)...                                        │
│                                                     │
│    غادي يكون الـ Integration ساهل بزاف.             │
│                                                     │
│    ╔═══════════════════════════════════════════╗     │
│    ║  Ownership + Interfaces + Workflow        ║     │
│    ║  = Integration بلا مشاكل                  ║     │
│    ╚═══════════════════════════════════════════╝     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**خلاصة لكل عضو:**

**Ismail (Lead Architect):**
- نتا اللي كتبني الساس (Server, Client, Channel).
- نتا اللي كتحدد الـ Interfaces اللي الآخرين كيخدمو عليها.
- نتا اللي كتراجع وكتدمج.
- مسؤوليتك الكبرى: **الـ Architecture تبقى نقية ومتماسكة.**

**Ayoub (Network Layer):**
- نتا اللي كتفتح الباب بين الـ Server والعالم الخارجي.
- بلا الـ Sockets وبلا `poll()` ديالك، ما كاين لا اتصال لا Server.
- خاصك توفر Interface واضحة وتحترم الـ Architecture ديال Ismail.
- مسؤوليتك الكبرى: **الـ Network Layer تكون Non-blocking وStable.**

**Souhail (Data Processing):**
- نتا اللي كتحول الـ Raw bytes لأوامر IRC مفهومة.
- الـ BufferManager والـ Parser هما الجسر بين Network و Logic.
- خاصك توفر Interface واضحة وما تعتمدش على الـ Client ولا الـ Server داخليا.
- مسؤوليتك الكبرى: **الداتا تتعالج صحيح — Partial messages, CRLF, Parsing.**

---

> [!NOTE]
> هاد الوثيقة مبنية على [ismail-guide.md](file:///home/ismail/Data/ft_irc/ft_irc/docs/ismail-guide.md) و [team-work-distribution.md](file:///home/ismail/Data/ft_irc/ft_irc/docs/team-work-distribution.md).
> أي تناقض بين هاد الوثيقة وهاد الملفات، المرجع هو الملفات الأصلية.
> حدث هاد الوثيقة يلا تبدلات أي Interface ولا أي Ownership.

بالتوفيق للفريق كامل فـ ft_irc! 🚀

</div>
