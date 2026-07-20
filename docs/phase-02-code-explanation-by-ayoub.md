<div dir="rtl">

# 📘 Phase 2: شرح الكود — Code Explanation

**المرحلة الثانية: Sockets & Connections**
**المسؤول: Ayoub**
**المشروع: ft_irc — 42**

---

## 📑 الفهرس (Table of Contents)

1. [Overview — شنو دارنا](#1-overview)
2. [Files Modified — الملفات اللي تبدلو](#2-files-modified)
3. [Server Class — الـ Class ديال الـ Server](#3-server-class)
4. [Constructor — شرح الـ Constructor](#4-constructor)
5. [Destructor — شرح الـ Destructor](#5-destructor)
6. [init() — شرح خطوة بخطوة](#6-init)
7. [System Calls — شرح كل System Call](#7-system-calls)
8. [Memory View — الرؤية ديال الذاكرة](#8-memory-view)
9. [Lifecycle — دورة الحياة الكاملة](#9-lifecycle)
10. [Error Handling — التعامل مع الأخطاء](#10-error-handling)
11. [Testing — كيفاش تختبر](#11-testing)
12. [Common Mistakes — الأخطاء الشائعة](#12-common-mistakes)
13. [Self Check — أسئلة التحقق الذاتي](#13-self-check)
14. [Summary — الخلاصة](#14-summary)

---

## 1. Overview — شنو دارنا <a name="1-overview"></a>

### شنو تيمبلومونطينا فـ Phase 2؟

فـ Phase 2، دارنا حاجة وحدة أساسية: **خلينا الـ Server يسمع على الشبكة**.

يعني بنينا الجزء ديال الـ Networking اللي كيخلي الـ Server يفتح **باب** (Socket) ويتسنى الناس يدقو عليه.

### شنو بالضبط اللي خدمنا؟

1. خلقنا **TCP Socket** بـ `socket()`
2. ضبطنا الـ Socket بـ `setsockopt()` (SO_REUSEADDR)
3. خليناه **non-blocking** بـ `fcntl()`
4. ربطنا الـ Socket بالـ **port** اللي عطانا الـ user بـ `bind()`
5. بدينا **الإنصات** بـ `listen()`

### علاش دارنا هادشي؟

حيت بلا Socket، الـ Server ما يقدرش يتواصل مع حتى واحد. الـ Socket هو الأساس — بحال الباب ديال الدار. إيلا ما فتحتيش الباب، حتى واحد ما غادي يدخل.

### فين وقفنا؟

وقفنا **مورا `listen()`**. الـ Server كيتسنى ولكن ما كيقبلش حتى client. الـ `accept()` والـ `poll()` والـ `recv()` — هادو ديال Phase الجاية.

---

## 2. Files Modified — الملفات اللي تبدلو <a name="2-files-modified"></a>

### `includes/Server.hpp`

هاد الملف فيه **الإعلان** (declaration) ديال الـ Server class.

**شنو زدنا:**

| الحاجة | علاش |
|---|---|
| `#include <sys/socket.h>` | باش نستعملو `socket()`, `setsockopt()`, `bind()`, `listen()` |
| `#include <netinet/in.h>` | باش نستعملو `sockaddr_in`, `INADDR_ANY`, `htons()` |
| `#include <fcntl.h>` | باش نستعملو `fcntl()` وO_NONBLOCK |
| `#include <unistd.h>` | باش نستعملو `close()` |
| `#include <cstring>` | باش نستعملو `std::memset()`, `strerror()` |
| `#include <cerrno>` | باش نستعملو `errno` |
| `#include <stdexcept>` | باش نستعملو `std::runtime_error` |
| `_createSocket()` | Private helper — كيخلق الـ Socket |
| `_configureSocket()` | Private helper — كيضبط SO_REUSEADDR |
| `_setNonBlocking()` | Private helper — كيخلي الـ Socket non-blocking |
| `_bindSocket()` | Private helper — كيربط الـ Socket بالـ port |
| `_startListening()` | Private helper — كيبدا الإنصات |

**شنو ما بدلناش:**

الـ member variables بقاو بحال ما كانو: `_port`, `_password`, `_serverFd`. الـ Orthodox Canonical Form (copy constructor و assignment operator) بقاو private — ما بغيناش حد ينسخ الـ Server.

### `src/Server.cpp`

هاد الملف فيه **التنفيذ** (implementation) ديال الـ Server class.

**شنو بدلنا:**

| الجزء | الشرح |
|---|---|
| **Destructor** | دابا كيسد الـ Socket إيلا كان مفتوح (`_serverFd != -1`) |
| **init()** | دابا كيعيط لـ 5 private helpers بالترتيب |
| **run()** | كيتسنى بـ `pause()` — غير باش الـ process ما يموتش |
| **_createSocket()** | كيخلق TCP Socket |
| **_configureSocket()** | كيضبط SO_REUSEADDR |
| **_setNonBlocking()** | كيخلي الـ Socket non-blocking |
| **_bindSocket()** | كيربط بالـ port |
| **_startListening()** | كيبدا listen |

### `src/main.cpp`

**ما بدلناش فيه والو.** الكود ديال Phase 1 كان مزيان — الـ `main()` كيدير validation على الـ port والـ password، كيخلق الـ Server، كيعيط لـ `init()` ومن بعد `run()`.

### `Makefile`

**ما بدلناش فيه والو.** الـ Makefile كان مكتوب مزيان فـ Phase 1 — كيكومبايلي بـ `-Wall -Wextra -Werror -std=c++98` وكيلينكي كلشي.

---

## 3. Server Class — الـ Class ديال الـ Server <a name="3-server-class"></a>

### Member Variables — المتغيرات

| المتغير | النوع | الشرح |
|---|---|---|
| `_port` | `int` | رقم الـ port اللي غادي يسمع عليه الـ Server (مثلاً 6667) |
| `_password` | `std::string` | الـ password اللي خاص الـ clients يعطيو باش يتصلو |
| `_serverFd` | `int` | الـ File Descriptor ديال الـ Listening Socket. كيبدا بـ `-1` (ما كاينش Socket) |

### Methods — الوظائف

| الوظيفة | Public/Private | الشرح |
|---|---|---|
| `Server(int, string&)` | Public | Constructor — كيحفظ الـ port والـ password |
| `~Server()` | Public | Destructor — كيسد الـ Socket |
| `init()` | Public | كيبدا الـ networking (socket → listen) |
| `run()` | Public | كيتسنى بلا ما يدير والو (Phase 2) |
| `_createSocket()` | Private | Step 1: `socket()` |
| `_configureSocket()` | Private | Step 2: `setsockopt()` |
| `_setNonBlocking()` | Private | Step 3: `fcntl()` |
| `_bindSocket()` | Private | Step 4+5: `sockaddr_in` + `bind()` |
| `_startListening()` | Private | Step 6: `listen()` |

### Ownership — الملكية

الـ Server class هي **المالك الوحيد** ديال الـ Listening Socket:

- **Constructor** كيحط `_serverFd = -1` (ما كاينش Socket بعد)
- **init()** كيخلق الـ Socket وكيحط القيمة فـ `_serverFd`
- **Destructor** كيسد الـ Socket بـ `close(_serverFd)`
- **ما كاين حتى واحد آخر** يقدر يوصل للـ `_serverFd` — حيت هو `private`
- Copy constructor و assignment operator **ممنوعين** (private بلا implementation)

هادا هو **RAII** (Resource Acquisition Is Initialization) — الـ resource (Socket) كيتخلق فالـ init وكيموت فالـ Destructor.

---

## 4. Constructor — شرح الـ Constructor <a name="4-constructor"></a>

```cpp
Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _serverFd(-1)
{
    std::cout << "Server object created. Port: " << _port
              << ", Password: [PROTECTED]" << std::endl;
}
```

### علاش الـ Constructor ما كيديرش بزاف؟

الـ Constructor كيدير **غير التخزين** ديال القيم:

1. كيخزن الـ `port` فـ `_port`
2. كيخزن الـ `password` فـ `_password`
3. كيحط `_serverFd` لـ `-1`

**ما كيخلقش Socket فالـ Constructor.** علاش؟

- **Separation of Concerns**: الـ Constructor غرضو **يخلق الـ object**. الـ `init()` غرضو **يبدا الشبكة**. كل واحد عندو مسؤولية وحدة.
- **Error Handling أسهل**: إيلا فشل `socket()` فالـ Constructor، صعيب تعالجو. ولكن إيلا فشل فـ `init()`, سهل ترمي exception.
- **Two-Phase Initialization**: هاد الـ pattern (Constructor + init) هو pattern مشهور فـ C++. الـ Constructor كيخلق Object valid (بـ `_serverFd = -1`), والـ `init()` كيديرلو initialization كامل.

### علاش `_serverFd = -1`؟

`-1` هو القيمة اللي كتعني **"ما كاينش File Descriptor"**. هادا هو الـ convention فـ Unix/Linux:
- `socket()` كترجع `-1` ملي تفشل
- `open()` كترجع `-1` ملي تفشل
- يعني `-1` = "ما كاين والو"

ملي كنحطو `_serverFd = -1` فالـ Constructor، كنقولو: "الـ Socket ما تخلقش بعد."

### الـ Initializer List

```cpp
: _port(port), _password(password), _serverFd(-1)
```

فـ C++، كنستعملو الـ **Initializer List** (اللي كتجي بعد `:`) بدل ما نحطو القيم فالـ body ديال الـ Constructor. هادا:
- **أكثر كفاءة** — كيبني الـ members مباشرة بدل ما يخلقهم فارغين ومن بعد يعاود يعطيهم قيم
- **ضروري** لبعض الأنواع (بحال `const` والـ references)

---

## 5. Destructor — شرح الـ Destructor <a name="5-destructor"></a>

```cpp
Server::~Server()
{
    if (_serverFd != -1)
    {
        close(_serverFd);
        std::cout << "Server socket closed (fd " << _serverFd << ")." << std::endl;
    }
    std::cout << "Server object destroyed." << std::endl;
}
```

### علاش `close()` ضروري؟

كل Socket هو **File Descriptor** — يعني مورد ديال الـ Kernel. ملي كتخلقو بـ `socket()`, الـ Kernel كيخصص:
- **Socket Object** فالـ Kernel Space
- **Send Buffer** و **Receive Buffer**
- **Entry** فالـ File Descriptor Table

إيلا ما سديتيش الـ Socket بـ `close()`:
- الموارد هادو كيبقاو مخصصين
- الـ port كيبقى محجوز
- هادا هو **Resource Leak** — مشكل خطير

### علاش كنتشكو واش `_serverFd != -1`؟

حيت ممكن يكون الـ Destructor تعيط **قبل ما الـ Socket يتخلق**:

| السيناريو | `_serverFd` | واش كنسدو؟ |
|---|---|---|
| الـ Server تخلق ولكن `init()` ما تعيطاتش | `-1` | لا — ما كاين ما نسدو |
| `init()` فشلات فـ `socket()` | `-1` | لا |
| `init()` نجحات ومن بعد انتهى البرنامج | `4` (ولا أي رقم) | نعم — كنسدو |

بلا هاد الـ check، ممكن نعيطو لـ `close(-1)` وهادا غادي يعطي error.

### File Descriptor Lifetime — مدة حياة الـ FD

```
Constructor          init()          Program runs          Destructor
    │                  │                  │                    │
    ▼                  ▼                  ▼                    ▼
 _serverFd = -1    _serverFd = 4      Socket مفتوح        close(4)
 (ما كاين والو)   (Socket تخلق)     (كيتسنى)           (Socket تسد)
                                                         _serverFd مزال = 4
                                                         (ولكن ما مهمش حيت
                                                          الـ Object غادي يموت)
```

القاعدة: **اللي خلق الـ Socket هو اللي يسدو.** الـ Server هو اللي عيط لـ `socket()` فالـ `init()`, يعني الـ Server هو اللي خاصو يعيط لـ `close()` فالـ Destructor.

---

## 6. init() — شرح خطوة بخطوة <a name="6-init"></a>

```cpp
void Server::init()
{
    std::cout << "Initializing server..." << std::endl;
    _createSocket();       // Step 1: socket()
    _configureSocket();    // Step 2: setsockopt()
    _setNonBlocking();     // Step 3: fcntl()
    _bindSocket();         // Step 4+5: sockaddr_in + bind()
    _startListening();     // Step 6: listen()
    std::cout << "Server ready." << std::endl;
}
```

### الترتيب ديال الخطوات

الترتيب هاد مش عشوائي — هو **الترتيب الوحيد اللي كيخدم**:

```
socket()        ← أولاً: خلق الـ Socket
    │
    ▼
setsockopt()    ← ثانياً: ضبط الـ Socket (قبل bind)
    │
    ▼
fcntl()         ← ثالثاً: خليه non-blocking
    │
    ▼
sockaddr_in     ← رابعاً: حضر العنوان
    │
    ▼
bind()          ← خامساً: ربط بالـ port
    │
    ▼
listen()        ← سادساً: بدا الإنصات
    │
    ▼
   ✓ جاهز!
```

**علاش هاد الترتيب؟**
- ما تقدرش تدير `setsockopt()` على Socket ما تخلقش بعد
- ما تقدرش تدير `bind()` بلا ما تضبط الـ Socket
- ما تقدرش تدير `listen()` على Socket ما تربطش بعد بـ port
- خاصك تدير `O_NONBLOCK` **قبل** ما تبدا تستعمل الـ Socket

### علاش قسمنا الكود لـ Private Helpers؟

بدل ما نحطو كلشي فـ `init()` واحدة كبيرة، قسمنا لـ 5 functions صغيرة:

1. **قابلية القراءة** — كل function كتدير حاجة وحدة واضحة
2. **Error Handling أنظف** — كل function كتعالج الأخطاء ديالها
3. **Single Responsibility** — كل function عندها مسؤولية وحدة
4. **سهل الـ debugging** — إيلا فشلات شي حاجة، كتعرف فين بالضبط

### شنو كيوقع إيلا فشلات شي خطوة؟

إيلا فشلات أي خطوة:
1. كنسدو الـ Socket (إيلا كان مفتوح)
2. كنحطو `_serverFd = -1`
3. كنرميو `std::runtime_error` برسالة واضحة
4. الـ `main()` كيلقط الـ exception وكيطبع الـ error وكيخرج

ما كاين حتى resource leak — حيت كنسدو الـ Socket قبل ما نرميو الـ exception.

---

## 7. System Calls — شرح كل System Call <a name="7-system-calls"></a>

### 7.1 socket()

```cpp
_serverFd = socket(AF_INET, SOCK_STREAM, 0);
```

**Purpose — الغرض:**
`socket()` كيخلق **نقطة نهاية ديال الاتصال** (Communication Endpoint). هادا هو أول شي كنديرو — كنشريو "التيليفون".

**Arguments — الـ Parameters:**

| Parameter | القيمة | الشرح |
|---|---|---|
| `domain` | `AF_INET` | كنستعملو IPv4 (عناوين بحال 127.0.0.1) |
| `type` | `SOCK_STREAM` | كنبغيو TCP — اتصال مستمر وموثوق |
| `protocol` | `0` | خلي الـ OS يختار (TCP مع SOCK_STREAM) |

**Return Value — القيمة اللي كترجع:**
- **نجاح**: رقم الـ File Descriptor (عادةً 3 أو أكثر)
- **فشل**: `-1` وكيحط الـ error فـ `errno`

**Possible Errors — الأخطاء الممكنة:**

| Error | السبب |
|---|---|
| `EMFILE` | البرنامج وصل للحد الأقصى ديال الـ File Descriptors المفتوحين |
| `ENFILE` | النظام كامل وصل للحد الأقصى |
| `EACCES` | ما عندكش الصلاحيات |
| `ENOMEM` | ما بقاتش ذاكرة كافية |
| `EPROTONOSUPPORT` | البروتوكول اللي طلبتي ما مدعومش |

**كيفاش كنستعملوه:**
```cpp
_serverFd = socket(AF_INET, SOCK_STREAM, 0);
if (_serverFd == -1)
    throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
```

كنتشكو واش رجع `-1`. إيلا نعم، كنرميو exception بالرسالة ديال الـ error.

---

### 7.2 setsockopt()

```cpp
int optval = 1;
setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
```

**Purpose — الغرض:**
`setsockopt()` كيضبط خيار (option) على الـ Socket. حنا كنستعملو `SO_REUSEADDR` باش نقدرو نعاودو نربطو الـ port فوراً بعد ما الـ Server يتسد.

**بلا SO_REUSEADDR شنو كيوقع؟**

ملي الـ Server كيتسد، الـ port كيبقى فحالة **TIME_WAIT** لمدة 1-2 دقائق. إيلا حاولتي تعاود تشغل الـ Server فهاد الوقت، `bind()` غادي يعطيك:
```
bind() failed: Address already in use
```

مع `SO_REUSEADDR`, كتقدر تعاود تربط فوراً. هادا مهم بزاف فالـ development ملي كتسد وكتشعل الـ Server بزاف.

**Arguments — الـ Parameters:**

| Parameter | القيمة | الشرح |
|---|---|---|
| `sockfd` | `_serverFd` | الـ Socket اللي بغينا نضبطو |
| `level` | `SOL_SOCKET` | المستوى — Socket level (مش TCP level) |
| `optname` | `SO_REUSEADDR` | الخيار — إعادة استعمال العنوان |
| `optval` | `&optval` (= 1) | القيمة — `1` يعني فعّل |
| `optlen` | `sizeof(optval)` | الحجم ديال القيمة |

**Return Value:**
- **نجاح**: `0`
- **فشل**: `-1`

**Possible Errors:**

| Error | السبب |
|---|---|
| `EBADF` | الـ fd ما صالحش |
| `ENOTSOCK` | الـ fd ماشي Socket |
| `ENOPROTOOPT` | الخيار ما مدعومش |

**كيفاش كنستعملوه:**
```cpp
int optval = 1;
if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
{
    close(_serverFd);
    _serverFd = -1;
    throw std::runtime_error(std::string("setsockopt() failed: ") + strerror(errno));
}
```

كنتشكو، وإيلا فشل كنسدو الـ Socket ومن بعد كنرميو exception.

---

### 7.3 fcntl()

```cpp
int flags = fcntl(_serverFd, F_GETFL, 0);
fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK);
```

**Purpose — الغرض:**
`fcntl()` (File Control) كيتحكم فالـ File Descriptor. حنا كنستعملوه باش نحطو الـ Socket فـ **non-blocking mode**.

**شنو هو Non-Blocking Mode؟**

| Blocking (العادي) | Non-Blocking |
|---|---|
| `accept()` كيتسنى حتى يجي client | `accept()` كيرجع فوراً (بـ EAGAIN إيلا ما كاين حد) |
| `recv()` كيتسنى حتى توصل data | `recv()` كيرجع فوراً |
| البرنامج كيوقف | البرنامج كيكمل يخدم |

هادا ضروري لأن فالـ Phase الجاية، الـ Server خاصو يتعامل مع **بزاف ديال الـ clients فنفس الوقت**. ما يقدرش يتسنى لواحد ويتجاهل الباقي.

**Arguments:**

| الاستعمال | الشرح |
|---|---|
| `fcntl(fd, F_GETFL, 0)` | كيجيب الـ flags الحالية ديال الـ fd |
| `fcntl(fd, F_SETFL, flags \| O_NONBLOCK)` | كيضيف O_NONBLOCK على الـ flags الحالية |

**علاش كنديرو F_GETFL قبل F_SETFL؟**

حيت ما بغيناش نمسحو الـ flags الأخرى. إيلا درنا `fcntl(fd, F_SETFL, O_NONBLOCK)` مباشرة، غادي نمسحو أي flags كانو من قبل. الطريقة الصحيحة هي:
1. جيب الـ flags الحالية
2. زيد O_NONBLOCK عليهم بـ `|` (OR)
3. حط الـ flags الجديدة

**Return Value:**
- **F_GETFL نجاح**: الـ flags (رقم موجب)
- **F_SETFL نجاح**: `0`
- **فشل**: `-1`

**Possible Errors:**

| Error | السبب |
|---|---|
| `EBADF` | الـ fd ما صالحش |
| `EINVAL` | الـ command ما صالحش |

---

### 7.4 bind()

```cpp
struct sockaddr_in addr;
std::memset(&addr, 0, sizeof(addr));
addr.sin_family      = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port        = htons(_port);

bind(_serverFd, (struct sockaddr *)&addr, sizeof(addr));
```

**Purpose — الغرض:**
`bind()` كيربط الـ Socket بعنوان محلي (IP + Port). بلا `bind()`, الـ Socket ما عندو حتى عنوان — بحال تيليفون بلا رقم.

**sockaddr_in — الهيكل ديال العنوان:**

```
struct sockaddr_in {
    sa_family_t    sin_family;    // عائلة العناوين (AF_INET)
    in_port_t      sin_port;      // رقم الـ port (بـ network byte order)
    struct in_addr sin_addr;      // عنوان الـ IP
};
```

| الحقل | القيمة | الشرح |
|---|---|---|
| `sin_family` | `AF_INET` | IPv4 |
| `sin_addr.s_addr` | `INADDR_ANY` | قبل الاتصالات على **أي** interface (0.0.0.0) |
| `sin_port` | `htons(_port)` | الـ port بـ network byte order |

**علاش `memset(&addr, 0, ...)`؟**

الـ `sockaddr_in` ممكن يكون فيه padding bytes — يعني bytes زايدة ما كنستعملوهمش. `memset` كيحط كلشي لـ 0 باش ما يكونش فيه بيانات عشوائية ديال الـ stack.

**علاش `htons()`؟**

الكومبيوتر ديالك كيخزن الأرقام بـ **Little-Endian** (الـ byte الصغير أولاً). الشبكة كتستعمل **Big-Endian** (الـ byte الكبير أولاً). `htons()` (Host TO Network Short) كيحول من الترتيب ديال الكومبيوتر للترتيب ديال الشبكة.

```
Port 6667 = 0x1A0B

Little-Endian (الكومبيوتر): 0B 1A
Big-Endian (الشبكة):       1A 0B
                             ↑
                          htons() كيدير هاد التحويل
```

**علاش `INADDR_ANY`؟**

`INADDR_ANY` (= `0.0.0.0`) كيعني: قبل الاتصالات على **كل** الـ network interfaces:
- `127.0.0.1` (localhost)
- `192.168.1.x` (الشبكة المحلية)
- أي IP آخر عندو الجهاز

**علاش `(struct sockaddr *)`؟**

`bind()` كتقبل `struct sockaddr *` — وهادا هو النوع العام. ولكن حنا عندنا `struct sockaddr_in` (خاص بـ IPv4). الـ cast كيحول من النوع الخاص للنوع العام.

**Return Value:**
- **نجاح**: `0`
- **فشل**: `-1`

**Possible Errors:**

| Error | السبب |
|---|---|
| `EADDRINUSE` | الـ port مستعمل من برنامج آخر |
| `EACCES` | بغيت port < 1024 بلا root |
| `EBADF` | الـ fd ما صالحش |
| `EINVAL` | الـ Socket مربوط من قبل |

---

### 7.5 listen()

```cpp
listen(_serverFd, 10);
```

**Purpose — الغرض:**
`listen()` كيحول الـ Socket من حالة **CLOSED** لحالة **LISTENING**. يعني الـ Socket دابا جاهز يستقبل الاتصالات.

هادا هو آخر شي كنديرو فـ Phase 2.

**Arguments:**

| Parameter | القيمة | الشرح |
|---|---|---|
| `sockfd` | `_serverFd` | الـ Socket اللي بغينا يسمع |
| `backlog` | `10` | الحد الأقصى ديال الاتصالات اللي كيتسناو فالطابور |

**شنو هو الـ Backlog؟**

الـ backlog هو **عدد الاتصالات المعلقة** (pending connections) اللي الـ Kernel كيخليهم يتسناو قبل ما الـ Server يقبلهم بـ `accept()`.

```
Client 1 ──→ ┌────────────────┐
Client 2 ──→ │ Pending Queue  │ ──→ accept() ──→ كيتعالج
Client 3 ──→ │ (backlog = 10) │
   ...   ──→ │ max 10 يتسناو │
Client 11 ─→ │ ✗ ما كاينش    │ ──→ كيتردّ (Connection Refused)
             │   بلاصة        │
             └────────────────┘
```

`10` هو رقم معقول — كافي لـ IRC Server صغير.

**Return Value:**
- **نجاح**: `0`
- **فشل**: `-1`

**Possible Errors:**

| Error | السبب |
|---|---|
| `EBADF` | الـ fd ما صالحش |
| `ENOTSOCK` | الـ fd ماشي Socket |
| `EOPNOTSUPP` | نوع الـ Socket ما كيدعمش listen (مثلاً SOCK_DGRAM) |
| `EADDRINUSE` | الـ port مستعمل |

---

## 8. Memory View — الرؤية ديال الذاكرة <a name="8-memory-view"></a>

### العلاقة بين البرنامج والـ Kernel

```
┌────────────────────────────────────────────────────────────┐
│                      User Space                             │
│                                                            │
│   ┌──────────────────────────────────────────────────┐     │
│   │              البرنامج (ft_irc)                     │     │
│   │                                                    │     │
│   │   ┌──────────────────────────────┐                │     │
│   │   │       Server Object           │                │     │
│   │   │                                │                │     │
│   │   │   _port     = 6667             │                │     │
│   │   │   _password = "password"       │                │     │
│   │   │   _serverFd = 4        ───────────┐            │     │
│   │   │                                │   │            │     │
│   │   └──────────────────────────────┘   │            │     │
│   │                                        │            │     │
│   └──────────────────────────────────────│────────────┘     │
│                                            │                  │
├────────────────────────────────────────────│──────────────────┤
│                      Kernel Space          │                  │
│                                            ▼                  │
│   ┌──────────────────────────────────────────────┐           │
│   │          File Descriptor Table                │           │
│   │                                                │           │
│   │   FD 0  ──→  stdin                            │           │
│   │   FD 1  ──→  stdout                           │           │
│   │   FD 2  ──→  stderr                           │           │
│   │   FD 3  ──→  (ممكن يكون مستعمل)              │           │
│   │   FD 4  ──→  Socket Object ◄──────────────────│───┐      │
│   │                                                │   │      │
│   └──────────────────────────────────────────────┘   │      │
│                                                        │      │
│   ┌──────────────────────────────────────────────┐   │      │
│   │           Socket Object (Kernel)              │◄──┘      │
│   │                                                │          │
│   │   domain:     AF_INET                          │          │
│   │   type:       SOCK_STREAM                      │          │
│   │   protocol:   TCP                              │          │
│   │   state:      LISTENING                        │          │
│   │   local_addr: 0.0.0.0:6667                     │          │
│   │   backlog:    10                               │          │
│   │   send_buf:   [empty]                          │          │
│   │   recv_buf:   [empty]                          │          │
│   │                                                │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   ┌──────────────────────────────────────────────┐          │
│   │           TCP/IP Stack                        │          │
│   └──────────────────────────┬───────────────────┘          │
│                               │                              │
│   ┌──────────────────────────▼───────────────────┐          │
│   │           Network Interface Card (NIC)        │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### شنو كيمثل كل جزء؟

| الجزء | فين كاين | الشرح |
|---|---|---|
| **Server Object** | User Space (RAM) | الـ Object ديال C++ — فيه `_port`, `_password`, `_serverFd` |
| **`_serverFd` (= 4)** | User Space | غير رقم — كيشير للـ Socket Object فالـ Kernel |
| **FD Table** | Kernel Space | الجدول اللي كيربط الأرقام بالموارد |
| **Socket Object** | Kernel Space | الكائن الحقيقي ديال الـ Socket — فيه كلشي |
| **TCP/IP Stack** | Kernel Space | كيعالج الـ packets |
| **NIC** | Hardware | كيبعث ويستقبل على الشبكة |

### الحاجة المهمة اللي خاصك تفهم:

الـ `_serverFd` هو **غير رقم**. ماشي pointer, ماشي object — غير `int`. ولكن وراه، كاين عالم كامل فالـ Kernel. كل ملي كتعيط لـ `bind(_serverFd, ...)` ولا `listen(_serverFd, ...)`, الـ Kernel كيشوف هاد الرقم وكيلقى الـ Socket Object المقابل ليه وكيخدم عليه.

---

## 9. Lifecycle — دورة الحياة الكاملة <a name="9-lifecycle"></a>

```
┌─────────────────────────────────────────────────────────┐
│                  دورة حياة الـ Server                      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   1. البرنامج كيبدا                                     │
│      │   ./ircserv 6667 password                        │
│      │                                                  │
│      ▼                                                  │
│   2. main() كيتحقق من الـ arguments                    │
│      │   port = 6667, password = "password"             │
│      │                                                  │
│      ▼                                                  │
│   3. Server object كيتخلق                              │
│      │   Server(6667, "password")                       │
│      │   _serverFd = -1                                 │
│      │                                                  │
│      ▼                                                  │
│   4. init() كتتعيط                                     │
│      │                                                  │
│      ├──→ socket()        _serverFd = 4                │
│      │                                                  │
│      ├──→ setsockopt()    SO_REUSEADDR = ON            │
│      │                                                  │
│      ├──→ fcntl()         O_NONBLOCK = ON              │
│      │                                                  │
│      ├──→ bind()          0.0.0.0:6667                 │
│      │                                                  │
│      └──→ listen()        backlog = 10                 │
│      │                                                  │
│      ▼                                                  │
│   5. Server جاهز! 🎉                                   │
│      │   "Server listening on port 6667"               │
│      │                                                  │
│      ▼                                                  │
│   6. run() كتتعيط                                      │
│      │   كيتسنى بـ pause()                              │
│      │   (Phase 2: ما كاين لا accept لا poll)          │
│      │                                                  │
│      ▼                                                  │
│   7. Ctrl+C ولا Signal                                 │
│      │   البرنامج كيخرج                                 │
│      │                                                  │
│      ▼                                                  │
│   8. ~Server() (Destructor)                            │
│      │   close(_serverFd)                              │
│      │   الـ Socket تسد                                 │
│      │   الـ port تحرر                                  │
│      │                                                  │
│      ▼                                                  │
│   9. البرنامج انتهى ✓                                  │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### الحالات ديال الـ Socket

```
CREATED ──→ CONFIGURED ──→ NON-BLOCKING ──→ BOUND ──→ LISTENING
   │              │              │              │           │
socket()    setsockopt()    fcntl()        bind()     listen()
```

فكل حالة، الـ Socket كيقدر يدير حوايج محدودة:
- **CREATED**: ما كيقدر يدير والو بعد — غير موجود
- **CONFIGURED**: مضبوط ولكن ما مربوطش بـ port
- **BOUND**: مربوط بـ port ولكن ما كيسمعش
- **LISTENING**: جاهز يقبل الاتصالات (ولكن فـ Phase 2، ما كنقبلوش)

---

## 10. Error Handling — التعامل مع الأخطاء <a name="10-error-handling"></a>

### الفلسفة ديال Error Handling ديالنا

كل system call كنتشكو عليه. إيلا فشل، كنديرو 3 حوايج:

1. **كنسدو الـ Socket** (إيلا كان مفتوح) — باش ما يكونش resource leak
2. **كنحطو `_serverFd = -1`** — باش الـ Destructor ما يحاولش يسد Socket مسدود
3. **كنرميو exception** — باش الـ `main()` يعرف بلي فشل وكيخرج نظيف

### كل خطأ ممكن

| الخطوة | الـ System Call | الخطأ الشائع | السبب | شنو كنديرو |
|---|---|---|---|---|
| 1 | `socket()` | `EMFILE` | بزاف ديال الـ files مفتوحين | throw exception |
| 2 | `setsockopt()` | `EBADF` | الـ fd ما صالحش (ما يقعش عادةً) | close + throw |
| 3 | `fcntl()` | `EBADF` | الـ fd ما صالحش | close + throw |
| 4+5 | `bind()` | `EADDRINUSE` | الـ port مستعمل | close + throw |
| 6 | `listen()` | `EADDRINUSE` | الـ port مستعمل | close + throw |

### مثال على الـ Error Flow

```
init()
  │
  ├──→ socket()     ✓ نجح (fd = 4)
  │
  ├──→ setsockopt() ✓ نجح
  │
  ├──→ fcntl()      ✓ نجح
  │
  ├──→ bind()       ✗ فشل! (EADDRINUSE — الـ port مستعمل)
  │     │
  │     ├──→ close(4)          كنسدو الـ Socket
  │     ├──→ _serverFd = -1    كنحيدو الـ reference
  │     └──→ throw runtime_error("bind() failed: Address already in use")
  │
  ▼
main()
  │
  ├──→ catch (std::exception& e)
  │     │
  │     └──→ std::cerr << "Fatal error: " << e.what()
  │
  └──→ return 1

~Server()
  │
  └──→ _serverFd == -1 → ما كنسدوش والو (مسدود من قبل)
```

### علاش ما كنستعملوش `exit()`؟

`exit()` كيخرج من البرنامج **فوراً** بلا ما يعيط للـ Destructors. هادا كيعني:
- الـ Socket ما غاديش يتسد بالـ Destructor
- أي cleanup آخر ما غاديش يتدار

بدل `exit()`, كنرميو **exception** اللي الـ `main()` كيلقطها. هادا كيخلي الـ Stack Unwinding يدير كلشي بالترتيب — كل Destructor كيتعيط.

---

## 11. Testing — كيفاش تختبر <a name="11-testing"></a>

### الخطوة 1: Compilation

```bash
make re
```

خاص يكومبايلي بلا أي error ولا warning (حيت عندنا `-Wall -Wextra -Werror`).

### الخطوة 2: تشغيل الـ Server

```bash
./ircserv 6667 password
```

خاص يطبع:

```
Server object created. Port: 6667, Password: [PROTECTED]
Initializing server...
Creating socket...
Socket created (fd 4).
Setting SO_REUSEADDR...
Socket configured.
Enabling non-blocking mode...
Socket set to non-blocking.
Binding to port 6667...
Bound successfully.
Listening...
Server listening on port 6667
Server ready.
Server is waiting... (press Ctrl+C to stop)
```

### الخطوة 3: التحقق بـ `ss`

فـ terminal آخر:

```bash
ss -tlnp | grep 6667
```

خاص يبان شي حاجة بحال:

```
LISTEN  0  10  0.0.0.0:6667  0.0.0.0:*  users:(("ircserv",pid=XXXX,fd=4))
```

هادا كيأكد بلي:
- الـ Socket فحالة **LISTEN**
- الـ backlog هو **10**
- كيسمع على **0.0.0.0:6667** (كل الـ interfaces)
- الـ fd هو **4**

### الخطوة 4: التحقق بـ `netstat`

```bash
netstat -tlnp | grep 6667
```

نفس المعلومات ولكن بـ format مختلف:

```
tcp  0  0  0.0.0.0:6667  0.0.0.0:*  LISTEN  XXXX/ircserv
```

### الخطوة 5: التحقق بـ `lsof`

```bash
lsof -i :6667
```

كيبين الـ process اللي كيستعمل الـ port 6667:

```
COMMAND    PID  USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
ircserv  XXXXX ismail   4u  IPv4 XXXXXX      0t0  TCP *:6667 (LISTEN)
```

### الخطوة 6: اختبار الأخطاء

```bash
# Port غالط
./ircserv abc password
# → Error: Port must contain only digits.

# Port خارج المجال
./ircserv 80 password
# → Error: Port must be between 1024 and 65535.

# بلا arguments
./ircserv
# → Usage: ./ircserv <port> <password>

# Port مستعمل (شغل الـ Server فـ terminal وحاول تشغل واحد آخر)
./ircserv 6667 password  # فـ terminal 1
./ircserv 6667 password  # فـ terminal 2
# → Fatal error: bind() failed: Address already in use
```

### الخطوة 7: اختبار بـ `nc` (netcat)

```bash
nc localhost 6667
```

هادا غادي يتصل بالـ Server. فـ Phase 2، الـ Server **ما غاديش يرد** (حيت ما كاين لا `accept()` لا `recv()`), ولكن الاتصال غادي **ينجح** على مستوى TCP حيت الـ Kernel كيقبل الاتصال فالـ backlog queue.

---

## 12. Common Mistakes — الأخطاء الشائعة <a name="12-common-mistakes"></a>

### 1. نسيان `close()` فالـ Destructor

```cpp
// ✗ غالط — resource leak
Server::~Server() {
    std::cout << "Destroyed" << std::endl;
}

// ✓ صحيح
Server::~Server() {
    if (_serverFd != -1)
        close(_serverFd);
}
```

بلا `close()`, الـ port كيبقى محجوز حتى يموت الـ process.

### 2. نسيان `close()` قبل ما ترمي exception

```cpp
// ✗ غالط — resource leak إيلا فشل bind
void Server::init() {
    _serverFd = socket(...);
    bind(_serverFd, ...);  // إيلا فشل، الـ Socket كيبقى مفتوح!
    throw std::runtime_error("bind failed");
}

// ✓ صحيح
if (bind(_serverFd, ...) == -1) {
    close(_serverFd);
    _serverFd = -1;
    throw std::runtime_error("bind failed");
}
```

### 3. نسيان `htons()`

```cpp
// ✗ غالط — الـ port غادي يكون غالط
addr.sin_port = _port;

// ✓ صحيح
addr.sin_port = htons(_port);
```

بلا `htons()`, الـ port كيتخزن بـ byte order غالط. مثلاً port 6667 غادي يولي port مختلف تماماً.

### 4. نسيان `memset()` على `sockaddr_in`

```cpp
// ✗ غالط — ممكن يكون فيه garbage data
struct sockaddr_in addr;
addr.sin_family = AF_INET;

// ✓ صحيح
struct sockaddr_in addr;
std::memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
```

### 5. استعمال `O_NONBLOCK` مباشرة بلا F_GETFL

```cpp
// ✗ غالط — كيمسح الـ flags الأخرى
fcntl(fd, F_SETFL, O_NONBLOCK);

// ✓ صحيح — كيحافظ على الـ flags الأخرى
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

### 6. نسيان SO_REUSEADDR

بلا SO_REUSEADDR، ملي كتسد الـ Server وكتعاود تشغلو بسرعة:
```
bind() failed: Address already in use
```

وخاصك تتسنى 1-2 دقائق. مع SO_REUSEADDR، كتقدر تعاود فوراً.

### 7. تجاهل القيمة اللي كترجع من System Calls

```cpp
// ✗ غالط — ما كتعرفش واش نجح ولا لا
socket(AF_INET, SOCK_STREAM, 0);
bind(_serverFd, ...);
listen(_serverFd, 10);

// ✓ صحيح — كنتشكو على كل حاجة
_serverFd = socket(AF_INET, SOCK_STREAM, 0);
if (_serverFd == -1) throw ...;
```

### 8. استعمال `exit()` بدل exceptions

```cpp
// ✗ غالط — الـ Destructors ما كيتعيطوش
if (bind(...) == -1) {
    exit(1);
}

// ✓ صحيح — الـ Stack Unwinding كيدير cleanup
if (bind(...) == -1) {
    close(_serverFd);
    throw std::runtime_error("bind failed");
}
```

### 9. Double close

```cpp
// ✗ غالط — كنسدو مرتين
close(_serverFd);
// ... code ...
close(_serverFd);  // Undefined Behavior!

// ✓ صحيح — كنحطو -1 بعد close
close(_serverFd);
_serverFd = -1;
```

### 10. نسيان تحديث `_serverFd` بعد close فالـ error handling

```cpp
// ✗ غالط — الـ Destructor غادي يعاود يسد
if (bind(...) == -1) {
    close(_serverFd);
    throw ...;  // _serverFd مزال فيه الرقم القديم!
}

// ✓ صحيح
if (bind(...) == -1) {
    close(_serverFd);
    _serverFd = -1;  // ← ضروري!
    throw ...;
}
```

---

## 13. Self Check — أسئلة التحقق الذاتي <a name="13-self-check"></a>

حاول تجاوب على هاد الأسئلة بلا ما تشوف الأجوبة:

### السؤال 1
**شنو هو الـ Socket وعلاش كنحتاجوه فـ IRC Server؟**

> الـ Socket هو نقطة نهاية ديال الاتصال الشبكي. بلا Socket، البرنامج ما يقدرش يتواصل مع الشبكة ولا يستقبل clients.

### السؤال 2
**شنو كيرجع `socket()` ملي كينجح؟ وملي كيفشل؟**

> نجاح: File Descriptor (رقم >= 0, عادةً 3+). فشل: `-1`.

### السؤال 3
**علاش كنستعملو `SO_REUSEADDR`؟ شنو كيوقع بلاه؟**

> باش نقدرو نعاودو نربطو الـ port فوراً بعد ما الـ Server يتسد. بلاه، كنتسناو 1-2 دقائق حيت الـ port كيبقى فـ TIME_WAIT.

### السؤال 4
**شنو الفرق بين Blocking و Non-Blocking mode؟**

> Blocking: الـ function كتتسنى حتى يوصل شي حاجة. Non-Blocking: كترجع فوراً (بـ EAGAIN إيلا ما كاينش data).

### السؤال 5
**علاش كنديرو `F_GETFL` قبل `F_SETFL`؟**

> باش نحافظو على الـ flags القديمة ونزيدو O_NONBLOCK عليهم. إيلا درنا F_SETFL مباشرة، غادي نمسحو الـ flags الأخرى.

### السؤال 6
**شنو كيدير `htons()` وعلاش هو ضروري؟**

> كيحول الرقم من Host Byte Order (Little-Endian) لـ Network Byte Order (Big-Endian). ضروري حيت الشبكة كتستعمل Big-Endian.

### السؤال 7
**شنو هو `INADDR_ANY` وعلاش كنستعملوه بدل IP محدد؟**

> `INADDR_ANY` (0.0.0.0) كيعني: قبل الاتصالات على أي network interface. هادشي باش الـ Server يكون accessible من localhost ومن الشبكة.

### السؤال 8
**شنو هو الـ backlog فـ `listen()` وشنو كيوقع ملي كيمتلا؟**

> الـ backlog هو الحد الأقصى ديال الاتصالات اللي كيتسناو فالطابور. ملي كيمتلا، الاتصالات الجديدة كتتردّ (Connection Refused).

### السؤال 9
**علاش كنسدو الـ Socket قبل ما نرميو exception فـ error handling؟**

> باش ما يكونش resource leak. إيلا ما سديناش، الـ Socket كيبقى مفتوح والـ port محجوز.

### السؤال 10
**علاش كنحطو `_serverFd = -1` بعد `close()`؟**

> باش الـ Destructor ما يحاولش يسد Socket مسدود (double close = Undefined Behavior).

### السؤال 11
**شنو الفرق بين `sockaddr` و `sockaddr_in`؟ وعلاش كنديرو cast؟**

> `sockaddr` هو النوع العام. `sockaddr_in` هو النوع الخاص بـ IPv4. الـ cast ضروري حيت `bind()` كتقبل `sockaddr*`.

### السؤال 12
**علاش كنستعملو `memset()` على `sockaddr_in`؟**

> باش نمسحو أي garbage data فالـ padding bytes. بلا memset، ممكن يكون فيه قيم عشوائية.

### السؤال 13
**واش الـ Constructor خاصو يخلق الـ Socket؟ علاش ولا علاش لا؟**

> لا. خاص الـ Socket يتخلق فـ `init()` (Two-Phase Initialization). هادا كيسهل Error Handling وكيفصل المسؤوليات.

### السؤال 14
**كيفاش كتتحقق بلي الـ Server كيسمع مزيان على الـ port؟**

> بـ `ss -tlnp | grep PORT` ولا `netstat -tlnp | grep PORT` ولا `lsof -i :PORT`.

### السؤال 15
**شنو Phase 2 كتغطي وشنو ما كتغطيش؟**

> كتغطي: socket(), setsockopt(), fcntl(), bind(), listen(). ما كتغطيش: accept(), poll(), recv(), send(), Client, Channel, Parser.

---

## 14. Summary — الخلاصة <a name="14-summary"></a>

### شنو كملنا فـ Phase 2

فـ Phase 2، بنينا **الأساس الشبكي** ديال الـ Server:

| الخطوة | الـ System Call | شنو دارت |
|---|---|---|
| 1 | `socket()` | خلقنا TCP Socket |
| 2 | `setsockopt()` | فعلنا SO_REUSEADDR |
| 3 | `fcntl()` | خلينا الـ Socket non-blocking |
| 4 | `sockaddr_in` + `bind()` | ربطنا بالـ port |
| 5 | `listen()` | بدينا الإنصات |

الـ Server دابا **جاهز يقبل الاتصالات**... ولكن ما كيقبلهمش بعد. غير كيتسنى.

### ⛔ فين وقفنا بالضبط

**Phase 2 كتنتهي مورا `listen()`.**

الـ Server كيسمع على الـ port، ولكن:
- ❌ ما كيقبلش clients (`accept()`)
- ❌ ما كيستعملش `poll()`
- ❌ ما كيستقبلش data (`recv()`)
- ❌ ما كيبعثش data (`send()`)
- ❌ ما كاين Client class
- ❌ ما كاين Channel class
- ❌ ما كاين Parser

### ✅ Phase الجاية شنو فيها

Phase الجاية (Phase 3) غادي تزيد:

| الحاجة | الشرح |
|---|---|
| `accept()` | قبول الاتصالات الجديدة |
| `poll()` | مراقبة بزاف ديال الـ File Descriptors فنفس الوقت |
| Client class | تمثيل كل client متصل |
| Event Loop | الحلقة الرئيسية اللي كتعالج الأحداث |

**ما يجبش تيمبلومونطي حتى حاجة من هادو فـ Phase 2.**

### 🎯 الخلاصة النهائية

Phase 2 = **فتحنا الباب ديال الدار**.

الباب مفتوح. الـ Server كيتسنى. ولكن مزال ما دخل حتى واحد.

فـ Phase الجاية، غادي نبداو **نقبلو الناس ونهضرو معاهم**.

</div>
