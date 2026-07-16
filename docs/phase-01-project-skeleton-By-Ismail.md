# 📘 Phase 1: Project Skeleton — دليل شامل

---

## 1. نظرة عامة على Phase 1

### شنو هي هاد Phase؟

Phase 1 هي **أول خطوة** فـ بناء مشروع `ft_irc`. فـ هاد المرحلة، ماغاديش نكتبو حتى سطر ديال networking أو socket programming. الهدف هو واحد: **نبنيو الهيكل ديال المشروع** (Project Architecture).

### علاش كنبداو بيها؟

قبل ما تبني أي دار، خصك الأساس (Foundation). هاد Phase هي الأساس ديال المشروع كامل. بلا Architecture نظيفة:
- الـ code غادي يولي فوضى
- غادي يصعب عليك تزيد features جداد
- الـ debugging غادي يولي كابوس
- الـ teamwork غادي يكون مستحيل

### شنو غادي نحضّرو فـ هاد Phase؟

- ✅ **Makefile** — باش ن compile-يو المشروع
- ✅ **Server class skeleton** — الهيكل ديال الـ Server بلا أي logic
- ✅ **main.cpp** — نقطة البداية ديال البرنامج مع validation ديال الـ arguments
- ✅ **Project Structure** — تنظيم الـ files والـ folders
- ✅ **Documentation** — هاد الملف لي كتقرا دابا!

### شنو ماغاديش نديرو؟

- ❌ لا socket()
- ❌ لا bind()
- ❌ لا listen()
- ❌ لا poll()
- ❌ لا Client class
- ❌ لا Channel class
- ❌ لا IRC commands
- ❌ لا Parser

---

## 2. Project Structure — هيكل المشروع

```
ft_irc/
├── Makefile                              ← قواعد الـ compilation
├── README.md                             ← وصف المشروع
├── .gitignore                            ← files لي ما خصهمش يتسجلو فـ git
├── includes/                             ← header files (.hpp)
│   └── Server.hpp                        ← declaration ديال الـ Server class
├── src/                                  ← source files (.cpp)
│   ├── main.cpp                          ← entry point ديال البرنامج
│   └── Server.cpp                        ← implementation ديال الـ Server class
└── docs/                                 ← documentation
    └── phase-01-project-skeleton.md      ← هاد الملف!
```

### علاش هاد التنظيم؟

#### 📁 `includes/` — Header Files

هنا كنحطو كل الـ `.hpp` files. الـ header file فيه **declarations** — يعني كنقولو للـ compiler: "هاد الـ class عندها هاد الـ methods وهاد الـ attributes"، بلا ما نعطيوه الـ implementation.

**علاش كنفصلو headers عن source files؟**
- باش الـ code يكون organized
- باش أي file يقدر ي include-ي الـ header بلا ما يحتاج الـ implementation
- هادشي هو الـ standard فـ C++ projects

#### 📁 `src/` — Source Files

هنا كنحطو كل الـ `.cpp` files. فيهم الـ **implementation** — يعني الـ code الفعلي لي كيخدم.

**علاش ماحطيناش main.cpp فـ root؟**
- باش كل الـ source code يكون فـ بلاصة وحدة
- باش الـ Makefile يكون أنظف
- باش الـ root directory ما يكونش مزدحم

#### 📁 `docs/` — Documentation

هنا كنحطو كل الـ documentation. كل Phase غادي يكون عندها ملف خاص بيها.

#### 📄 `Makefile` — فـ Root

الـ Makefile خصو يكون فـ root ديال المشروع باش تقدر تكتب `make` مباشرة.

#### 📄 `.gitignore`

كيقول لـ `git`: "هاد الـ files ما تسجلهمش". مثلا: الـ binary (`ircserv`)، الـ object files (`*.o`)، وملفات الـ editor.

#### 📄 `README.md`

أول حاجة كيشوفها أي واحد كيدخل للـ repository. فيها وصف المشروع وكيفاش تستعملو.

---

## 3. علاش Makefile؟

### شنو هو Makefile؟

الـ Makefile هو **ملف فيه قواعد (rules)** لي كتقول للـ `make` tool كيفاش ي compile-ي المشروع. بلا Makefile، خصك تكتب الـ compile commands يدوياً كل مرة:

```bash
# بلا Makefile — خصك تكتب هادشي كل مرة! 😫
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes -c src/main.cpp -o obj/main.o
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes -c src/Server.cpp -o obj/Server.o
c++ -Wall -Wextra -Werror -std=c++98 obj/main.o obj/Server.o -o ircserv
```

مع الـ Makefile، غير كتكتب:
```bash
make
```
وخلاص! ✅

### الـ Makefile ديالنا — سطر بسطر

```makefile
NAME        = ircserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
INCLUDES    = -Iincludes

SRC_DIR     = src
OBJ_DIR     = obj

SRCS        = src/main.cpp \
              src/Server.cpp

OBJS        = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
```

#### شرح المتغيرات (Variables):

| Variable | القيمة | الشرح |
|----------|--------|-------|
| `NAME` | `ircserv` | اسم الـ executable لي غادي يتخلق |
| `CXX` | `c++` | الـ compiler لي غادي نستعملو |
| `CXXFLAGS` | `-Wall -Wextra -Werror -std=c++98` | الـ flags ديال الـ compilation |
| `INCLUDES` | `-Iincludes` | كيقول للـ compiler فين يلقى الـ header files |
| `SRC_DIR` | `src` | الـ directory ديال الـ source files |
| `OBJ_DIR` | `obj` | الـ directory ديال الـ object files |
| `SRCS` | `src/main.cpp src/Server.cpp` | لائحة الـ source files |
| `OBJS` | التحويل من `.cpp` لـ `.o` | لائحة الـ object files |

#### شرح الـ Rules:

```makefile
all: $(NAME)
```
- `all` هي الـ rule الافتراضية — كتعيط لـ `$(NAME)` باش ت compile-ي

```makefile
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
```
- كتاخد الـ object files وكت link-هم فـ executable واحد اسمو `ircserv`

```makefile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
```
- لكل `.cpp` file، كت compile-ي object file (`.o`) فـ directory `obj/`
- `$<` = الـ source file (input)
- `$@` = الـ object file (output)

```makefile
clean:
	rm -rf $(OBJ_DIR)
```
- كتمسح الـ object files

```makefile
fclean: clean
	rm -f $(NAME)
```
- كتمسح الـ object files **و** الـ executable

```makefile
re: fclean all
```
- كت recompile-ي كلشي من الصفر

```makefile
.PHONY: all clean fclean re
```
- كتقول للـ `make`: هاد الأسماء ماشي files، هاد rules

### علاش C++98؟

المشروع ديال 42 School **mandatory** أنه يكون C++98. هادشي كيعني:
- ما نقدروش نستعملو `auto`
- ما نقدروش نستعملو `nullptr` (خصنا نستعملو `NULL`)
- ما نقدروش نستعملو range-based for loops
- ما نقدروش نستعملو `std::unique_ptr` أو `std::shared_ptr`
- ما نقدروش نستعملو lambda functions

الـ flag `-std=c++98` كيضمن أن الـ compiler غادي يرفض أي syntax ديال C++11 أو أحدث.

### علاش -Wall -Wextra -Werror؟

| Flag | شنو كيدير |
|------|-----------|
| `-Wall` | كيفعل أغلب الـ warnings المهمة |
| `-Wextra` | كيزيد warnings إضافية لي `-Wall` ما كيشملهمش |
| `-Werror` | كيحول **كل warning لـ error** — يعني الـ code ما غاديش ي compile-ي إلا كانت أي warning |

**علاش هادشي مهم؟**

بلا هاد الـ flags، الـ compiler غادي يخليك تدوز bugs خطيرة بلا ما تعرف. مثلا:
- variable لي ما تستعملاتش
- function لي ما كترجعش قيمة
- مقارنة بين signed و unsigned

مع `-Werror`، الـ compiler كيجبرك تصلح كل حاجة. هادشي كيعلمك تكتب **clean code** من البداية.

### كيفاش Compilation كتخدم؟

```
                    Compilation Flow
                    ================

  src/main.cpp ──────┐
                     ├──→ compile ──→ obj/main.o ────┐
  includes/Server.hpp┘                               │
                                                     ├──→ link ──→ ircserv
  src/Server.cpp ────┐                               │
                     ├──→ compile ──→ obj/Server.o ──┘
  includes/Server.hpp┘
```

1. **Compilation**: كل `.cpp` file كيتحول لـ `.o` (object file)
2. **Linking**: كل الـ `.o` files كيتجمعو فـ executable واحد

---

## 4. علاش main.cpp؟

### علاش كل C++ project كيبدا من main()؟

فـ C++، الـ operating system كيبحث على function اسمها `main()` باش يبدا تنفيذ البرنامج. بلا `main()` = لا برنامج.

### شنو هي المسؤولية ديال main()؟

الـ `main()` ديالنا عنده **3 مسؤوليات بسيطة**:
1. ✅ **Validate** الـ arguments (port و password)
2. ✅ **Create** الـ Server object
3. ✅ **Delegate** الخدمة لـ `init()` و `run()`

### علاش خص main() يبقى صغير وبسيط؟

لأن `main()` ماشي المكان ديال الـ business logic:
- إلا حطينا كلشي فـ main()، الـ code غادي يولي 1000 سطر فـ function وحدة
- ما غاديش نقدرو نعاودو نستعملو الـ code (no reusability)
- الـ testing غادي يكون مستحيل
- الـ debugging غادي يكون كابوس

**القاعدة الذهبية**: main() غير كي validate-ي الـ input وكي delegate-ي الخدمة.

### الـ Code ديال main.cpp — سطر بسطر

```cpp
#include "Server.hpp"
#include <iostream>
#include <cstdlib>
```
- `"Server.hpp"` — كيجيب الـ declaration ديال الـ Server class
- `<iostream>` — باش نطبعو messages (std::cout, std::cerr)
- `<cstdlib>` — باش نستعملو `std::atoi()` لتحويل string لـ int

```cpp
static bool isAllDigits(const std::string& str)
{
    if (str.empty())
        return false;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}
```
- helper function كت check-ي واش string فيها غير أرقام
- `static` كيعني أن هاد الـ function مرئية غير فـ هاد الـ file (internal linkage)
- ما استعملناش `std::isdigit()` من `<cctype>` باش نتفادى أي مشاكل مع locale

```cpp
int main(int argc, char **argv)
{
    // Step 1: Check argument count
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
```
- `argc` = عدد الـ arguments (اسم البرنامج + port + password = 3)
- إلا ماشي 3، كنطبعو Usage message ونخرجو بـ error code 1

```cpp
    std::string portStr(argv[1]);
    std::string password(argv[2]);
```
- كنحولو الـ `char*` لـ `std::string` باش نتعاملو معاهم بسهولة

```cpp
    // Step 2: Validate port contains only digits
    if (!isAllDigits(portStr))
    {
        std::cerr << "Error: Port must contain only digits." << std::endl;
        return 1;
    }

    // Step 3: Validate port range (1024 - 65535)
    int port = std::atoi(portStr.c_str());
    if (port < 1024 || port > 65535)
    {
        std::cerr << "Error: Port must be between 1024 and 65535." << std::endl;
        return 1;
    }
```
- أولاً كن check-يو واش الـ port فيه غير digits
- من بعد كنحولوه لـ int بـ `std::atoi()`
- خص الـ port يكون بين **1024** و **65535**
  - Ports تحت 1024 محجوزين للـ system (مثل port 80 للـ HTTP)
  - Port أكبر من 65535 ما كاينش

```cpp
    // Step 4: Validate password is not empty
    if (password.empty())
    {
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return 1;
    }
```
- الـ password ما خصوش يكون خاوي

```cpp
    // Step 5: Create Server, initialize, and run
    try
    {
        Server server(port, password);
        server.init();
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```
- كنخلقو Server object بـ port و password
- كنعيطو لـ `init()` ومن بعد `run()`
- `try/catch` كيحمينا من أي exception لي يقدر يطيح فـ المستقبل
- `return 0` = البرنامج خدم مزيان

---

## 5. علاش Server Class؟

### علاش كنخلقو Server class قبل ما نبداو Socket Programming؟

**لأن Architecture كتجي قبل Implementation!**

تخيل بغيتي تبني دار:
- أولاً كترسم التصميم (Architecture)
- من بعد كتبدا تبني (Implementation)

نفس الشي فـ البرمجة. الـ Server class هو "التصميم" ديال الـ server ديالنا. ملي غادي نبداو نزيدو socket programming فـ Phase 2، غادي يكون عندنا بلاصة واضحة فين نحطو كل حاجة.

### علاش Object-Oriented Architecture؟

بلا OOP:
```cpp
// ❌ بلا OOP — فوضى!
int server_fd;
int port;
std::string password;
// ... 50 global variable ...
// ... 2000 سطر ديال functions لي كيستعملو هاد الـ globals ...
```

مع OOP:
```cpp
// ✅ مع OOP — منظم!
Server server(6667, "password");
server.init();
server.run();
```

**الفوائد ديال OOP:**
- كل حاجة مرتبطة بالـ Server مجموعة فـ بلاصة وحدة
- الـ data محمية (Encapsulation)
- كنقدرو نزيدو features بسهولة
- الـ code قابل للقراءة وللصيانة

### علاش Encapsulation مهمة؟

```cpp
class Server
{
    private:
        int         _port;       // ← محمي! ما يمكنش تبدلو من برا
        std::string _password;   // ← محمي! ما يمكنش تقراه من برا
        int         _serverFd;   // ← محمي! ما يمكنش تلعب بيه من برا

    public:
        void init();  // ← الـ interface العامة
        void run();   // ← الـ interface العامة
};
```

الـ `_port`، `_password`، و `_serverFd` كلهم `private`. هادشي كيعني:
- **حتى code خارج الـ Server class ما يقدرش يوصل ليهم**
- غير الـ methods ديال الـ Server (init, run, ...) يقدرو يتعاملو معاهم
- هادشي كيحمي الـ data من التغيير الغير متوقع

### علاش ما نستعملوش Global Variables؟

```cpp
// ❌ NEVER DO THIS
int g_serverFd;
int g_port;
std::string g_password;
```

**المشاكل ديال Global Variables:**
- أي function فـ أي file يقدر يبدلهم
- ما عندكش control على من كي access-يهم
- الـ debugging كيولي كابوس
- الـ code ما كيكونش reusable
- فـ multi-threaded programs، كيخلقو data races

---

## 6. Orthodox Canonical Form (OCF)

### شنو هو OCF؟

فـ C++98، كل class **خصها يكون عندها 4 member functions أساسية**:

| # | الاسم | الدور |
|---|-------|-------|
| 1 | **Default Constructor** | كيخلق object جديد |
| 2 | **Destructor** | كيمسح object ملي ما بقيناش محتاجينو |
| 3 | **Copy Constructor** | كيخلق object جديد كنسخة من object آخر |
| 4 | **Copy Assignment Operator** | كيبدل محتوى object ب محتوى object آخر |

### Constructor — الباني

```cpp
Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _serverFd(-1)
{
    std::cout << "Server object created." << std::endl;
}
```

- كيتعيط **أوتوماتيكياً** ملي كنخلقو object جديد: `Server server(6667, "pass");`
- `: _port(port), _password(password), _serverFd(-1)` ← هادي **initializer list**
  - أحسن من أنك تدير `_port = port;` داخل الـ body
  - فـ C++98 هادي الطريقة الـ recommended
- `_serverFd(-1)` — كنبدوه بـ -1 لأن ما عندناش socket بعد (−1 = invalid file descriptor)

### Destructor — الهادم

```cpp
Server::~Server()
{
    std::cout << "Server object destroyed." << std::endl;
}
```

- كيتعيط **أوتوماتيكياً** ملي الـ object كيتمسح
- فـ Phase 2، هنا غادي ن close-يو الـ server socket
- `~` قبل الاسم كيعني "هادا Destructor"

### Copy Constructor — لي فـ Private

```cpp
private:
    Server(const Server& other);
```

- حطيناه فـ `private` **باش ما يمكنش حد ينسخ الـ Server**
- لأن الـ Server فيه socket file descriptor — إلا نسخناه غادي يكونو 2 objects كيشيرو لنفس الـ socket!
- هادشي غادي يخلق مشاكل خطيرة (double close, undefined behavior)

### Assignment Operator — لي فـ Private

```cpp
private:
    Server& operator=(const Server& other);
```

- نفس المنطق — ما بغيناش حد يدير `server1 = server2;`
- حطيناه فـ `private` باش الـ compiler يعطي error إلا شي حد حاول يستعملو

### علاش C++98 كتطلب هاد الأربعة؟

فـ C++98، إلا ما كتبتيش Copy Constructor أو Assignment Operator، الـ **compiler كيخلقهم أوتوماتيكياً**! وهاد النسخ الأوتوماتيكية كتدير **shallow copy** — يعني كتنسخ القيم بلا ما تفهم شنو كتعني.

لهادشي حطيناهم فـ `private` بلا implementation — باش الـ compiler يمنع أي نسخ.

> 💡 **فـ C++11+** كنقدرو نستعملو `= delete` بلاصة حطهم فـ private. ولكن حنا فـ C++98!

---

## 7. علاش init()؟

### علاش Constructor ما خصوش يدير خدمة ثقيلة؟

**قاعدة مهمة فـ Software Engineering:**

> الـ Constructor خصو غير ي **initialize-ي** الـ attributes بقيم بسيطة. ما خصوش يدير حاجات ثقيلة بحال فتح socket أو قراءة ملف.

**علاش؟**

1. **Error Handling**: فـ Constructor، ما عندكش return value. الطريقة الوحيدة باش تسيگنالي error هي throw exception. ولكن throw فـ Constructor كيخلق مشاكل معقدة مع الـ Destructor.

2. **Separation of Concerns**: الـ Constructor خصو يخلق الـ object. `init()` خصها تحضر الـ resources.

3. **Flexibility**: يمكن تبغي تخلق الـ object وت initialize-يه فـ وقت آخر.

### شنو غادي يتزاد فـ init() فـ Phase 2؟

```
init() — Phase 2:
├── socket()    → خلق الـ server socket
├── setsockopt() → ضبط الـ socket options
├── bind()      → ربط الـ socket بـ port
└── listen()    → بدا الاستماع للـ connections
```

دابا `init()` خاوية — ولكن الهيكل جاهز!

---

## 8. علاش run()؟

### علاش Execution Loop كيكون هنا؟

الـ `run()` هي **قلب الـ server**. هنا غادي يكون الـ main loop لي كي:
- يتسنى connections جداد
- يقرا messages من الـ clients
- يعالج الـ commands
- يبعت الـ responses

### شنو غادي يتزاد فـ run() فـ Phase 2؟

```
run() — Phase 2:
└── while (true)
    ├── poll()         → تسنا events على الـ sockets
    ├── accept()       → قبل connections جداد
    ├── recv()         → قرا data من clients
    ├── parse()        → عالج الـ commands
    └── send()         → بعت responses
```

### علاش فصلنا init() على run()؟

| `init()` | `run()` |
|----------|---------|
| كتخدم **مرة وحدة** | كتخدم **فـ loop ما كيوقفش** |
| كتحضر resources | كتستعمل resources |
| إلا فشلات → ما كندخلوش لـ run() | كتبقى خدامة حتى يوقف الـ server |
| socket, bind, listen | poll, accept, recv, send |

---

## 9. Program Flow — المسار ديال البرنامج

### مسار التنفيذ (Execution Flow)

```
╔══════════════════════════════════════════════════╗
║                  PROGRAM START                    ║
╚══════════════════════════════════════════════════╝
                        │
                        ▼
              ┌─────────────────┐
              │    main()       │
              └────────┬────────┘
                       │
           ┌───────────┴───────────┐
           │  Validate Arguments   │
           │                       │
           │  argc == 3 ?          │──── NO ──→ ❌ Error + Exit
           │  port = digits?       │──── NO ──→ ❌ Error + Exit
           │  1024 ≤ port ≤ 65535? │──── NO ──→ ❌ Error + Exit
           │  password ≠ empty?    │──── NO ──→ ❌ Error + Exit
           └───────────┬───────────┘
                       │ YES (All Valid)
                       ▼
        ┌──────────────────────────────┐
        │  Server server(port, pass);  │
        │  ↳ Constructor called        │
        │  ↳ _port = port              │
        │  ↳ _password = password      │
        │  ↳ _serverFd = -1            │
        └──────────────┬───────────────┘
                       │
                       ▼
              ┌─────────────────┐
              │  server.init()  │
              │  (Phase 2:      │
              │   socket,       │
              │   bind,         │
              │   listen)       │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │  server.run()   │
              │  (Phase 2:      │
              │   poll loop)    │
              └────────┬────────┘
                       │
                       ▼
        ┌──────────────────────────────┐
        │  ~Server() Destructor        │
        │  (automatic — end of scope)  │
        └──────────────────────────────┘
                       │
                       ▼
╔══════════════════════════════════════════════════╗
║                  PROGRAM END                      ║
╚══════════════════════════════════════════════════╝
```

### مسار الـ Compilation

```
      Source Files              Object Files           Executable
    ┌──────────────┐          ┌──────────────┐
    │ src/main.cpp │── c++ ──→│ obj/main.o   │──┐
    └──────────────┘          └──────────────┘  │     ┌──────────┐
                                                ├─ c++→│ ircserv  │
    ┌──────────────┐          ┌──────────────┐  │     └──────────┘
    │src/Server.cpp│── c++ ──→│obj/Server.o  │──┘
    └──────────────┘          └──────────────┘
           ▲
           │ #include
    ┌──────────────────┐
    │includes/Server.hpp│
    └──────────────────┘
```

---

## 10. شرح كل ملف — File-by-File Breakdown

### 📄 Makefile

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | ملف فيه rules ديال الـ compilation |
| **علاش تخلق؟** | باش ن compile-يو المشروع بـ `make` بلاصة كتابة commands طويلة |
| **علاش خصنا ليه؟** | 42 subject كتطلبو + هو standard فـ C++ projects |
| **كيفاش كيترابط مع Phases الجايين؟** | ملي غادي نزيدو files جداد (Client.cpp, Channel.cpp)، غادي نزيدوهم فـ SRCS |

### 📄 src/main.cpp

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | Entry point ديال البرنامج |
| **علاش تخلق؟** | كل C++ program خصو main() |
| **علاش خصنا ليه؟** | باش ن validate-يو الـ input ونخلقو الـ Server |
| **كيفاش كيترابط مع Phases الجايين؟** | main() ما غاديش يتبدل بزاف — الخدمة الجديدة غادي تكون فـ Server class |

### 📄 includes/Server.hpp

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | Declaration ديال الـ Server class |
| **علاش تخلق؟** | باش نفصلو الـ interface على الـ implementation |
| **علاش خصنا ليه؟** | باش أي file يقدر ي include-ي الـ Server class |
| **كيفاش كيترابط مع Phases الجايين؟** | غادي نزيدو methods و attributes جداد (std::vector<Client>, std::map<std::string, Channel>) |

### 📄 src/Server.cpp

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | Implementation ديال الـ Server class |
| **علاش تخلق؟** | باش نكتبو الـ code الفعلي ديال كل method |
| **علاش خصنا ليه؟** | فـ Phase 1 عندنا غير logs. فـ Phase 2 غادي يكون فيه الـ networking code |
| **كيفاش كيترابط مع Phases الجايين؟** | init() غادي يولي فيه socket/bind/listen. run() غادي يولي فيه poll loop |

### 📄 .gitignore

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | قائمة بالـ files لي git خصو يتجاهلهم |
| **علاش تخلق؟** | باش ما ندخلوش binary files و object files فـ الـ repository |
| **علاش خصنا ليه؟** | الـ compiled files كيتبدلو كل مرة — ما خصهمش يكونو فـ git |
| **كيفاش كيترابط مع Phases الجايين؟** | ما غادي يتبدلش بزاف |

### 📄 README.md

| السؤال | الجواب |
|--------|--------|
| **شنو تخلق؟** | وصف المشروع |
| **علاش تخلق؟** | باش أي واحد يفتح الـ repository يعرف شنو هو هاد المشروع |
| **علاش خصنا ليه؟** | Professional practice + كتساعد الـ team |
| **كيفاش كيترابط مع Phases الجايين؟** | غادي يتحدث كل Phase |

---

## 11. Future Phases — شنو جاي من بعد؟

### Phase 1 (الآن ✅)
- ✅ Project skeleton
- ✅ Makefile
- ✅ Server class skeleton
- ✅ Argument validation
- ✅ Documentation

### Phase 2 (قريباً)
- 🔲 `socket()` — خلق الـ server socket
- 🔲 `bind()` — ربط الـ socket بالـ port
- 🔲 `listen()` — بدء الاستماع
- 🔲 `poll()` — الانتظار على events
- 🔲 `accept()` — قبول connections جداد

### Phase 3 (من بعد)
- 🔲 Client class — لتمثيل كل client متصل
- 🔲 Connection management
- 🔲 Message receiving و sending

### Phase 4 (من بعد)
- 🔲 IRC Protocol parsing
- 🔲 Commands: PASS, NICK, USER, JOIN, PRIVMSG, KICK, INVITE, TOPIC, MODE
- 🔲 Channel class

### ⚠️ ملاحظة مهمة

فـ هاد Phase 1:
- **ما عندناش حتى socket** — ماشي مشكل!
- **ما عندناش حتى Client class** — غادي نخلقوها ملي نحتاجوها
- **ما عندناش حتى Channel class** — غادي نخلقوها ملي نحتاجوها
- **ما عندناش حتى Parser** — غادي نخلقوه ملي نحتاجوه

**هادشي normal!** كل حاجة فـ وقتها.

---

## 12. أخطاء شائعة عند المبتدئين — Common Beginner Mistakes

### ❌ الخطأ 1: حط كلشي فـ main()

```cpp
// ❌ ما تديرش هادشي!
int main() {
    int server_fd = socket(...);
    bind(server_fd, ...);
    listen(server_fd, ...);
    while (true) {
        poll(...);
        accept(...);
        recv(...);
        // ... 500 سطر آخرين ...
    }
}
```

**الحل**: خلق classes، وخلي main() صغير.

### ❌ الخطأ 2: استعمال Global Variables

```cpp
// ❌ ما تديرش هادشي!
int g_serverFd;
int g_port;
std::vector<int> g_clients;
```

**الحل**: حط كلشي فـ class attributes.

### ❌ الخطأ 3: خلط Initialization مع Execution

```cpp
// ❌ ما تديرش هادشي!
Server::Server(int port, const std::string& pass) {
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    bind(_serverFd, ...);
    listen(_serverFd, ...);
    while (true) {
        poll(...);  // الـ loop فـ Constructor?! 😱
    }
}
```

**الحل**: Constructor = initialize attributes فقط. init() = setup resources. run() = execution loop.

### ❌ الخطأ 4: زيادة socket() بكري بزاف

بعض الطلبة كيبداو مباشرة يكتبو networking code بلا ما يكون عندهم Architecture واضحة.

**الحل**: Phase 1 أولاً (Architecture)، Phase 2 من بعد (Networking).

### ❌ الخطأ 5: خلق Classes لي مازال ما خصناش ليهم

```cpp
// ❌ فـ Phase 1، ما خصكش تخلق هادو!
class Client { };
class Channel { };
class Parser { };
class CommandHandler { };
class Buffer { };
```

**الحل**: خلق غير الـ classes لي خصك دابا. الباقي غادي يجي فـ وقتو.

### ❌ الخطأ 6: نسيان Orthodox Canonical Form

```cpp
// ❌ ما تنساش OCF!
class Server {
public:
    Server(int port, const std::string& pass);
    // فين الـ Destructor?
    // فين الـ Copy Constructor?
    // فين الـ Assignment Operator?
};
```

**الحل**: ديما خلق الـ 4 member functions ديال OCF.

### ❌ الخطأ 7: ما دارش Error Handling فـ main()

```cpp
// ❌ إلا طاح exception = crash!
int main(int argc, char **argv) {
    Server server(std::atoi(argv[1]), argv[2]); // ماشي آمن!
    server.init();
    server.run();
}
```

**الحل**: validate كلشي + استعمل try/catch.

### ❌ الخطأ 8: ما استعملش -Wall -Wextra -Werror

```makefile
# ❌ ما تديرش هادشي!
CXXFLAGS = -std=c++98
```

**الحل**: ديما استعمل `-Wall -Wextra -Werror` باش الـ compiler يعاونك تلقى bugs.

---

## 13. Key Takeaways — خلاصة

بعد ما كملتي Phase 1، هاد الحاجات خصك تكون فهمتيهم مزيان:

1. ✅ **Architecture قبل Implementation** — بني الهيكل قبل ما تكتب الـ logic
2. ✅ **Makefile** — كيسهل الـ compilation ومهم فـ كل مشروع C++
3. ✅ **main() خصو يبقى صغير** — غير validation و delegation
4. ✅ **Server class** — كيجمع كلشي مرتبط بالـ server فـ بلاصة وحدة
5. ✅ **Orthodox Canonical Form** — 4 member functions أساسية فـ C++98
6. ✅ **Encapsulation** — private attributes + public methods
7. ✅ **init() ≠ Constructor** — Constructor يخلق، init() يحضر
8. ✅ **run() ≠ init()** — init() مرة وحدة، run() loop مستمر
9. ✅ **ما تزيدش حاجات بكري** — كل class فـ وقتو
10. ✅ **-Wall -Wextra -Werror** — الـ compiler صاحبك، خليه يعاونك

---

## 14. Self-Check Questions — أسئلة للمراجعة الذاتية

اختبر راسك! حاول تجاوب على هاد الأسئلة بلا ما تشوف الأجوبة:

1. **علاش كنخلقو Server class قبل ما نكتبو socket()؟**
   > لأن Architecture خصها تجي قبل Implementation. خص يكون عندنا هيكل واضح قبل ما نبداو نكتبو networking code.

2. **علاش main() خصو يبقى صغير وبسيط؟**
   > لأن main() خصو غير ي validate-ي الـ input وي delegate-ي الخدمة. الـ business logic خصها تكون فـ classes.

3. **شنو هو Orthodox Canonical Form فـ C++98؟**
   > هو 4 member functions: Constructor, Destructor, Copy Constructor, Assignment Operator. كل class خصها يكون عندها هاد الأربعة.

4. **علاش -Wall -Wextra -Werror مهمين؟**
   > `-Wall` و `-Wextra` كيفعلو الـ warnings. `-Werror` كيحول كل warning لـ error. هادشي كيجبرك تكتب clean code.

5. **شنو الفرق بين init() و Constructor؟**
   > Constructor كيخلق الـ object ويعطي قيم أولية بسيطة. init() كتدير الخدمة الثقيلة بحال فتح socket.

6. **شنو الفرق بين init() و run()؟**
   > init() كتخدم مرة وحدة (setup). run() كتخدم فـ loop مستمر (execution).

7. **علاش _serverFd كيبدا بـ -1؟**
   > لأن -1 كتعني "invalid file descriptor". مازال ما خلقنا حتى socket، فالقيمة الافتراضية خصها تكون invalid.

8. **علاش حطينا Copy Constructor و Assignment Operator فـ private؟**
   > باش نمنعو النسخ ديال الـ Server object. لأن النسخ غادي يخلق مشاكل مع الـ socket file descriptor.

9. **شنو كيوقع إلا ما عطيناش port صحيح (مثلا: "abc")؟**
   > `isAllDigits()` غادي ترجع false، البرنامج غادي يطبع error message ويخرج بـ return 1.

10. **علاش ما نستعملوش Global Variables؟**
    > لأن أي code يقدر يبدلهم، ما عندكش control عليهم، الـ debugging كيصعب، والـ code ما كيكونش reusable.

11. **شنو هو Makefile rule؟**
    > Rule هي قاعدة كتقول: "باش تخلق هاد الـ target، خصك هاد الـ dependencies، وخدم هاد الـ commands." مثال: `all: $(NAME)` — باش تخلق `all`، خصك تخلق `$(NAME)` أولاً.

12. **علاش كنفصلو Header Files (.hpp) على Source Files (.cpp)؟**
    > الـ header فيه الـ declaration (شنو كاين). الـ source فيه الـ implementation (كيفاش كيخدم). هادشي كيخلي أي file ي include-ي الـ class بلا ما يحتاج الـ implementation كاملة.

13. **شنو كيدير `.PHONY` فـ Makefile؟**
    > كيقول لـ `make`: هاد الأسماء (all, clean, fclean, re) ماشي ملفات — هاد rules. بلا `.PHONY`، إلا كان ملف اسمو "clean" فـ directory، `make clean` ما غاديش يخدم.

14. **علاش الـ port خصو يكون بين 1024 و 65535؟**
    > Ports تحت 1024 محجوزين للـ system services (port 80 = HTTP, port 443 = HTTPS). والـ maximum port number هو 65535 (لأن الـ port مخزن فـ 16-bit unsigned integer).

15. **شنو غادي يتبدل فـ Server.cpp فـ Phase 2؟**
    > `init()` غادي يولي فيها socket(), setsockopt(), bind(), listen(). `run()` غادي يولي فيها poll() loop مع accept(), recv(), send().

---

> 🎓 **تهانينا!** دابا فهمتي Phase 1 كاملة. الهيكل ديال المشروع جاهز، والـ code كي compile-ي بلا أي error أو warning. فـ Phase 2 غادي نبداو الـ networking! 🚀
