<div dir="rtl" align="right">

# ft_irc: دليل شامل لبناء IRC Server بلغة C++ 98

> هذا الملف هو خارطة طريق عملية ونظرية لمشروع `ft_irc`.
> الهدف ليس فقط أن تجعل السيرفر يخدم، بل أن تفهم لماذا كل جزء موجود، وكيف تربط بين الـ sockets، الـ `poll()`، الـ parsing، وإدارة القنوات بطريقة نظيفة ومستقرة.

## 1. مقدمة عن المشروع (Introduction)

### ما هو IRC؟

`IRC` اختصار لـ **Internet Relay Chat**، وهو بروتوكول قديم وقوي للتواصل النصي في الوقت الحقيقي. الفكرة بسيطة:

- المستخدم يفتح IRC Client مثل `irssi`, `HexChat`, أو حتى `nc`.
- الـ Client يتصل بسيرفر IRC عبر TCP.
- السيرفر يستقبل أوامر نصية مثل `NICK`, `USER`, `JOIN`, `PRIVMSG`.
- السيرفر يفسر هذه الأوامر، يغير الحالة الداخلية، ثم يرسل Replies أو Broadcasts للمستخدمين.

هندسيًا، المشروع مبني على **Client-Server Architecture**:

| الطرف | الدور |
| --- | --- |
| IRC Client | يرسل أوامر نصية للسيرفر ويعرض الردود للمستخدم |
| IRC Server | يقبل الاتصالات، يدير المستخدمين والقنوات، ويمرر الرسائل |
| TCP Socket | قناة الاتصال الثنائية بين client و server |
| IRC Protocol | القواعد النصية التي تحدد شكل الأوامر والردود |

مثال مبسط:

```irc
PASS secret
NICK ismail
USER ismail 0 * :Ismail User
JOIN #cpp
PRIVMSG #cpp :salam everyone
```

### القواعد الصارمة للمشروع

حسب الـ Subject، يجب احترام هذه القيود:

| القاعدة | معناها العملي |
| --- | --- |
| C++ 98 فقط | الكود يجب أن يترجم مع `-std=c++98` وبدون C++11 وما بعده |
| ممنوع Boost أو external libraries | اعتمد على STL المسموحة ودوال النظام |
| ممنوع `fork()` | لا تنشئ processes جديدة لكل client |
| Single Process | السيرفر كله يعمل داخل process واحد |
| Non-blocking I/O | لا تسمح لأي socket أن يعلق السيرفر |
| `poll()` واحد أو equivalent | مراقبة كل الـ FDs للقراءة والكتابة والاستماع تتم عبر loop واحدة |
| لا `recv()` أو `send()` خارج `poll()` | أي I/O يجب أن يكون بعد event من `poll()` |
| السيرفر لا ينهار | crash في أي حالة يعتبر فشلًا كبيرًا |

> الفكرة الأساسية: السيرفر يجب أن يكون event-driven. أي أنه ينتظر الأحداث من `poll()`، ثم يتصرف بسرعة، ويرجع ينتظر من جديد.

---

## 2. المرحلة الأولى: شبكات الاتصال (Socket Programming & I/O Multiplexing)

هذه أول مرحلة يجب دراستها وتطبيقها. قبل `NICK` و `JOIN` و `MODE`، خاصك تبني الأساس: كيفاش السيرفر يسمع، يقبل clients، ويقرأ/يرسل بدون block.

### إنشاء الـ Listening Socket

الـ Listening Socket هو الباب الرئيسي ديال السيرفر. كل client جديد يدخل من هنا.

#### 1. `socket()`

تنشئ file descriptor يمثل socket:

```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
```

| الجزء | المعنى |
| --- | --- |
| `AF_INET` | IPv4 |
| `SOCK_STREAM` | TCP |
| `0` | البروتوكول الافتراضي المناسب لـ TCP |

إذا رجعت `-1`، وقع خطأ ويجب التعامل معه.

#### 2. `setsockopt()` مع `SO_REUSEADDR`

تسمح بإعادة استعمال نفس port بسرعة بعد إغلاق السيرفر:

```cpp
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

بدونها، قد ترى خطأ مثل:

```text
Address already in use
```

خصوصًا أثناء التجريب المتكرر.

#### 3. `bind()`

تربط الـ socket بعنوان و port:

```cpp
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(port);

bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
```

| الحقل | المعنى |
| --- | --- |
| `INADDR_ANY` | اسمع على كل network interfaces |
| `htons(port)` | تحويل port إلى Network Byte Order |

#### 4. `listen()`

تجعل الـ socket يدخل وضع الاستماع:

```cpp
listen(server_fd, SOMAXCONN);
```

بعد `listen()`، السيرفر أصبح قادرًا على استقبال طلبات اتصال جديدة، لكن الاتصال نفسه لا يصبح client فعليًا حتى نستعمل `accept()`.

### الوضع غير الحاصر (Non-blocking Mode)

بشكل افتراضي، دوال مثل `accept()`, `recv()`, و `send()` قد تعلق البرنامج. في ft_irc هذا ممنوع، لأن client واحد بطيء يمكن أن يوقف السيرفر كله.

نستعمل:

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

يجب تطبيقها على:

- `server_fd`
- كل client fd يرجع من `accept()`

> في المشروع، `fcntl()` مسموحة فقط بهذا الشكل في سياق جعل الـ FD non-blocking.

ماذا يتغير بعد `O_NONBLOCK`؟

| العملية | قبل non-blocking | بعد non-blocking |
| --- | --- | --- |
| `accept()` | قد ينتظر اتصالًا جديدًا | يرجع فورًا إذا لا يوجد اتصال |
| `recv()` | قد ينتظر data | يرجع فورًا إذا لا توجد data |
| `send()` | قد ينتظر buffer فارغ | قد يرسل جزءًا فقط أو يرجع خطأ مؤقتًا |

أخطاء مثل `EAGAIN` أو `EWOULDBLOCK` لا تعني أن الاتصال مات، بل تعني: "جرّب لاحقًا".

### إدارة الأحداث (Event Loop)

الـ Event Loop هي قلب المشروع. الفكرة:

1. ضع `server_fd` وكل clients داخل `std::vector<pollfd>`.
2. استدع `poll()`.
3. إذا حدث event على `server_fd`، استعمل `accept()`.
4. إذا حدث event على client، استعمل `recv()` أو `send()` حسب نوع event.
5. حدّث الحالة الداخلية.
6. أعد الدورة.

شكل عام:

```cpp
while (server_running)
{
    int ready = poll(&fds[0], fds.size(), -1);
    if (ready < 0)
        continue;

    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].revents & POLLIN)
        {
            if (fds[i].fd == server_fd)
                acceptNewClient();
            else
                receiveFromClient(fds[i].fd);
        }

        if (fds[i].revents & POLLOUT)
            flushClientOutput(fds[i].fd);

        if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            disconnectClient(fds[i].fd);
    }
}
```

#### `pollfd` الأساسي

```cpp
struct pollfd pfd;
pfd.fd = client_fd;
pfd.events = POLLIN;
pfd.revents = 0;
```

| الحقل | الدور |
| --- | --- |
| `fd` | الـ file descriptor |
| `events` | ماذا نريد مراقبته |
| `revents` | ماذا حدث فعليًا |

#### `accept()`

تستعمل فقط عندما يخبرك `poll()` أن `server_fd` جاهز للقراءة:

```cpp
int client_fd = accept(server_fd, NULL, NULL);
fcntl(client_fd, F_SETFL, O_NONBLOCK);
```

بعدها:

- أنشئ `Client`.
- أضف `client_fd` إلى `poll()`.
- جهز buffers فارغة.
- لا تعتبره authenticated حتى ينفذ `PASS`, `NICK`, `USER`.

#### `recv()`

تستعمل عندما يكون client fd فيه `POLLIN`:

```cpp
char buffer[512];
ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
```

| النتيجة | المعنى |
| --- | --- |
| `bytes > 0` | data وصلت، أضفها إلى input buffer |
| `bytes == 0` | client أغلق الاتصال |
| `bytes < 0` | خطأ، تحقق من `errno` |

#### `send()`

أفضل ممارسة هي أن يكون لكل client `outputBuffer`.

- عندما تريد إرسال reply، أضف النص إلى buffer.
- فعّل `POLLOUT` لذلك client.
- عندما يخبرك `poll()` أن fd جاهز للكتابة، استعمل `send()`.
- إذا أرسلت جزءًا فقط، احذف الجزء المرسل واترك الباقي.
- إذا فرغ buffer، أزل `POLLOUT` وخلي `POLLIN`.

---

## 3. المرحلة الثانية: إدارة البيانات (Data Management & Architecture)

بعد بناء الـ sockets والـ event loop، نحتاج تنظيم الحالة الداخلية. لا تخلط كل شيء في `main.cpp`. المشروع يصبح أسهل بكثير إذا قسمت المسؤوليات.

### Server Class

مسؤولة عن حياة السيرفر كاملة:

| المسؤولية | التفاصيل |
| --- | --- |
| initialization | قراءة port/password، إنشاء socket، bind/listen |
| event loop | تشغيل `poll()` ومعالجة events |
| FD management | إضافة وحذف clients من `pollfd` |
| client registry | تخزين clients حسب fd أو nickname |
| channel registry | تخزين القنوات حسب الاسم |
| command dispatch | إرسال الأمر المناسب للـ handler المناسب |
| clean shutdown | إغلاق sockets وتنظيف الذاكرة |

تصميم ممكن:

```cpp
class Server
{
private:
    int _serverFd;
    std::string _password;
    std::vector<struct pollfd> _pollFds;
    std::map<int, Client> _clients;
    std::map<std::string, Channel> _channels;

public:
    Server(int port, const std::string &password);
    void run();
    void acceptClient();
    void receiveClient(int fd);
    void disconnectClient(int fd);
    void handleCommand(Client &client, const std::string &line);
};
```

### Client Class

تمثل مستخدمًا واحدًا متصلًا بالسيرفر.

| المعلومة | لماذا نحتاجها؟ |
| --- | --- |
| FD | لإرسال واستقبال data |
| Nickname | اسم المستخدم في IRC |
| Username | جزء من التسجيل |
| Realname | يظهر في بعض replies |
| Authentication status | هل أرسل PASS صحيح؟ هل أكمل NICK/USER؟ |
| Input buffer | لتجميع data المقطوعة |
| Output buffer | لتخزين الردود قبل `send()` |
| Joined channels | لمعرفة القنوات التي ينتمي إليها |

تصميم ممكن:

```cpp
class Client
{
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    bool _passOk;
    bool _registered;
    std::string _inputBuffer;
    std::string _outputBuffer;

public:
    int getFd() const;
    bool isRegistered() const;
    void appendInput(const std::string &data);
    void queueMessage(const std::string &message);
};
```

### Channel Class

تمثل قناة IRC مثل `#general`.

| العنصر | الدور |
| --- | --- |
| name | اسم القناة |
| topic | عنوان القناة |
| users | أعضاء القناة |
| operators | المستخدمون الذين لديهم صلاحيات |
| invited users | قائمة المدعوين عند `+i` |
| modes | حالات `i`, `t`, `k`, `l` |
| key | password القناة عند `+k` |
| user limit | الحد الأقصى عند `+l` |

تصميم ممكن:

```cpp
class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::set<int> _users;
    std::set<int> _operators;
    std::set<int> _invited;
    bool _inviteOnly;
    bool _topicRestricted;
    bool _hasKey;
    std::string _key;
    int _userLimit;

public:
    void addUser(int fd);
    void removeUser(int fd);
    bool isOperator(int fd) const;
    void broadcast(Server &server, const std::string &message, int exceptFd);
};
```

> نصيحة عملية: في C++98، استعمل `std::map`, `std::vector`, و `std::set` بشكل واضح. لا تحتاج abstractions كثيرة في البداية. المهم أن تكون ownership والمسؤوليات مفهومة.

---

## 4. المرحلة الثالثة: بروتوكول IRC والـ Parsing (Buffer & Command Handling)

### إدارة الـ Buffers: معالجة البيانات المقطوعة

TCP ليس message-based. يعني إذا أرسل client:

```irc
PRIVMSG #cpp :hello world\r\n
```

السيرفر قد يستقبلها هكذا:

```text
PRI
VMSG #cpp :hel
lo world\r\n
```

أو قد يستقبل عدة أوامر دفعة واحدة:

```text
NICK a\r\nUSER a 0 * :A\r\n
```

لذلك لا تنفذ أي أمر مباشرة بعد `recv()`. يجب:

1. تضيف bytes إلى `client.inputBuffer`.
2. تبحث عن نهاية السطر `\r\n`.
3. كلما وجدت line كاملة، تقطعها من buffer.
4. ترسل line للـ parser.
5. تترك الباقي غير المكتمل في buffer.

مثال:

```cpp
client.appendInput(std::string(buffer, bytes));

std::string &input = client.getInputBuffer();
size_t pos;
while ((pos = input.find("\r\n")) != std::string::npos)
{
    std::string line = input.substr(0, pos);
    input.erase(0, pos + 2);
    handleCommand(client, line);
}
```

> اختبار `nc` في السوجي مبني بالضبط على هذه النقطة. إذا لم تجمع data المقطوعة، السيرفر سيفشل في evaluation.

### الـ Parser

الأمر في IRC غالبًا يتبع هذا الشكل:

```irc
COMMAND param1 param2 :trailing parameter with spaces
```

أمثلة:

```irc
PASS secret
NICK ismail
USER ismail 0 * :Ismail Real Name
PRIVMSG #cpp :salam labas?
```

الـ Parser يجب أن يستخرج:

| الجزء | مثال |
| --- | --- |
| command | `PRIVMSG` |
| params | `#cpp` |
| trailing | `salam labas?` |

نقاط مهمة:

- الأوامر case-insensitive عمليًا، لكن من الأسهل تحويل command إلى uppercase.
- أي شيء بعد `:` كأول حرف في parameter يعتبر trailing وقد يحتوي spaces.
- line فارغة يجب تجاهلها.
- إذا command غير معروف، أرسل `421 ERR_UNKNOWNCOMMAND`.

تصميم parsing بسيط:

```cpp
struct Command
{
    std::string name;
    std::vector<std::string> params;
};
```

### أوامر التوثيق والتسجيل الأساسية

التسجيل في IRC ليس أمرًا واحدًا. غالبًا يحتاج:

1. `PASS`
2. `NICK`
3. `USER`

بعد اكتمال الشروط، ترسل welcome numeric replies.

#### `PASS`

الصيغة:

```irc
PASS <password>
```

المنطق:

- إذا لا يوجد parameter: `461 ERR_NEEDMOREPARAMS`.
- إذا password خاطئ: error مناسب ثم يمكن إغلاق الاتصال.
- إذا صحيح: علم client بـ `_passOk = true`.
- إذا client مسجل بالفعل: يمكن إرسال `462 ERR_ALREADYREGISTERED`.

#### `NICK`

الصيغة:

```irc
NICK <nickname>
```

المنطق:

- إذا nickname ناقص: `431 ERR_NONICKNAMEGIVEN`.
- إذا nickname مستعمل: `433 ERR_NICKNAMEINUSE`.
- إذا صالح: خزنه في `Client`.
- إذا اكتمل `PASS + NICK + USER`: سجل client وأرسل welcome.

#### `USER`

الصيغة:

```irc
USER <username> 0 * :<realname>
```

المنطق:

- إذا parameters ناقصة: `461 ERR_NEEDMOREPARAMS`.
- إذا client مسجل بالفعل: `462 ERR_ALREADYREGISTERED`.
- خزّن username و realname.
- تحقق هل أصبح client جاهزًا للتسجيل.

### Numeric Replies

كل reply في IRC له prefix من السيرفر:

```irc
:<server_name> <numeric> <nick_or_*> <message>\r\n
```

مثال:

```irc
:ircserv 001 ismail :Welcome to the IRC server ismail\r\n
```

#### Replies الترحيبية

| Numeric | الاسم | متى يرسل؟ | مثال |
| --- | --- | --- | --- |
| `001` | `RPL_WELCOME` | بعد اكتمال التسجيل | `:ircserv 001 nick :Welcome to the IRC server nick` |
| `002` | `RPL_YOURHOST` | بعد `001` | `:ircserv 002 nick :Your host is ircserv, running version 1.0` |
| `003` | `RPL_CREATED` | بعد `002` | `:ircserv 003 nick :This server was created today` |
| `004` | `RPL_MYINFO` | بعد `003` | `:ircserv 004 nick ircserv 1.0 itkol itkol` |

#### Errors شائعة

| Numeric | الاسم | متى يرسل؟ | صيغة تقريبية |
| --- | --- | --- | --- |
| `421` | `ERR_UNKNOWNCOMMAND` | أمر غير معروف | `:ircserv 421 nick COMMAND :Unknown command` |
| `461` | `ERR_NEEDMOREPARAMS` | parameters ناقصة | `:ircserv 461 nick COMMAND :Not enough parameters` |
| `462` | `ERR_ALREADYREGISTERED` | محاولة تسجيل ثانية | `:ircserv 462 nick :You may not reregister` |
| `431` | `ERR_NONICKNAMEGIVEN` | `NICK` بدون اسم | `:ircserv 431 * :No nickname given` |
| `433` | `ERR_NICKNAMEINUSE` | nickname مستعمل | `:ircserv 433 * nick :Nickname is already in use` |
| `401` | `ERR_NOSUCHNICK` | target user غير موجود | `:ircserv 401 nick target :No such nick/channel` |
| `403` | `ERR_NOSUCHCHANNEL` | channel غير موجودة | `:ircserv 403 nick #chan :No such channel` |
| `442` | `ERR_NOTONCHANNEL` | user ليس في القناة | `:ircserv 442 nick #chan :You're not on that channel` |
| `482` | `ERR_CHANOPRIVSNEEDED` | ليس operator | `:ircserv 482 nick #chan :You're not channel operator` |

> في بداية التسجيل، إذا لم يكن عند client nickname بعد، استعمل `*` مكان nick في الردود.

---

## 5. المرحلة الرابعة: إدارة القنوات والصلاحيات (Channel Operations & Modes)

### الأوامر العامة

#### `JOIN`

الصيغة:

```irc
JOIN <#channel> [key]
```

المنطق:

1. تحقق من أن client registered.
2. تحقق من وجود parameter.
3. إذا القناة غير موجودة، أنشئها.
4. إذا القناة جديدة، اجعل أول user هو operator.
5. تحقق من modes:
   - `+i`: هل user invited؟
   - `+k`: هل key صحيح؟
   - `+l`: هل القناة ممتلئة؟
6. أضف user للقناة.
7. أرسل broadcast لكل أعضاء القناة:

```irc
:nick!user@host JOIN #channel\r\n
```

8. أرسل topic إن وجد.
9. أرسل names list إن كنت تطبقها للتوافق مع client حقيقي.

Broadcast يعني: كل الموجودين في القناة يشوفون الحدث، بمن فيهم المستخدم الجديد في أغلب clients.

#### `PRIVMSG`

الصيغة:

```irc
PRIVMSG <target> :<message>
```

الـ target يمكن أن يكون:

| Target | المعنى |
| --- | --- |
| `nickname` | رسالة خاصة لمستخدم واحد |
| `#channel` | رسالة لكل أعضاء القناة |

المنطق:

- إذا target ناقص: error.
- إذا message ناقصة: error.
- إذا target قناة:
  - تحقق أن القناة موجودة.
  - تحقق أن sender داخل القناة.
  - أرسل لكل أعضاء القناة ما عدا sender:

```irc
:nick!user@host PRIVMSG #channel :message\r\n
```

- إذا target user:
  - ابحث عن nickname.
  - أرسل له فقط.

### صلاحيات الـ Operator

#### `KICK`

الصيغة:

```irc
KICK <#channel> <nick> [:reason]
```

المنطق:

| الشرط | إذا فشل |
| --- | --- |
| القناة موجودة | `403 ERR_NOSUCHCHANNEL` |
| المنفذ داخل القناة | `442 ERR_NOTONCHANNEL` |
| المنفذ operator | `482 ERR_CHANOPRIVSNEEDED` |
| الهدف داخل القناة | error مناسب مثل `441` |

بعد النجاح:

```irc
:operator!user@host KICK #channel target :reason\r\n
```

ثم احذف target من users/operators/invited حسب الحاجة.

#### `INVITE`

الصيغة:

```irc
INVITE <nick> <#channel>
```

المنطق:

- تحقق أن target user موجود.
- تحقق أن channel موجودة.
- إذا channel invite-only أو policy عندك تتطلب ذلك، تحقق أن المنفذ operator.
- أضف target إلى invited list.
- أرسل للـ target:

```irc
:operator!user@host INVITE target #channel\r\n
```

#### `TOPIC`

الصيغة للعرض:

```irc
TOPIC <#channel>
```

الصيغة للتغيير:

```irc
TOPIC <#channel> :new topic
```

المنطق:

- إذا لا يوجد topic، أرسل `331 RPL_NOTOPIC`.
- إذا يوجد topic، أرسل `332 RPL_TOPIC`.
- عند التغيير:
  - تحقق أن user داخل القناة.
  - إذا mode `+t` مفعّل، فقط operator يغير topic.
  - بعد التغيير، broadcast:

```irc
:nick!user@host TOPIC #channel :new topic\r\n
```

### تطبيق الـ MODEs

الصيغة العامة:

```irc
MODE <#channel> <modes> [params...]
```

أمثلة:

```irc
MODE #cpp +i
MODE #cpp -i
MODE #cpp +k secret
MODE #cpp -k
MODE #cpp +o ismail
MODE #cpp -o ismail
MODE #cpp +l 10
MODE #cpp -l
```

#### منطق `+` و `-`

اقرأ mode string حرفًا بحرف:

```text
+it-k
```

يعني:

- `+i`
- `+t`
- `-k`

احتفظ بمتغير `sign`:

| الحرف | التصرف |
| --- | --- |
| `+` | sign يصبح add |
| `-` | sign يصبح remove |
| `i`, `t`, `k`, `o`, `l` | طبق mode حسب sign |

#### جدول الـ Modes المطلوبة

| Mode | عند `+` | عند `-` | يحتاج parameter؟ |
| --- | --- | --- | --- |
| `i` | اجعل القناة invite-only | ألغ invite-only | لا |
| `t` | topic فقط للـ operators | topic لأي عضو | لا |
| `k` | ضع key للقناة | احذف key | نعم مع `+k` |
| `o` | أعط operator privilege | اسحب operator privilege | نعم: nickname |
| `l` | ضع user limit | احذف user limit | نعم مع `+l` |

#### تفاصيل كل Mode

##### `i` Invite-only

- `+i`: لا يدخل القناة إلا من تمت دعوته بـ `INVITE`.
- `-i`: أي user يمكنه الدخول إذا مر من باقي الشروط.

##### `t` Topic restrictions

- `+t`: فقط channel operators يمكنهم تغيير topic.
- `-t`: أي عضو في القناة يمكنه تغيير topic.

##### `k` Channel key

- `+k secret`: القناة تحتاج password.
- `JOIN #channel secret` يجب أن ينجح.
- `JOIN #channel wrong` يجب أن يفشل.
- `-k`: احذف password.

##### `o` Operator privilege

- `+o nick`: اجعل nick operator.
- `-o nick`: اسحب منه operator.
- يجب أن يكون target داخل القناة.

##### `l` User limit

- `+l 5`: الحد الأقصى 5 users.
- `-l`: ألغ الحد.
- تحقق من الرقم: يجب أن يكون موجبًا ومعقولًا.

> أهم قاعدة في `MODE`: لا تنس parameters. بعض modes تستهلك parameter وبعضها لا. parser ديالك يجب أن يمشي بالتوازي بين حروف mode و vector ديال params.

---

## 6. المرحلة الخامسة: اختبار النظام والاستقرار (Testing & Crash Proofing)

### الاختبار اليدوي باستعمال `nc -C`

شغل السيرفر:

```bash
./ircserv 6667 secret
```

اتصل من terminal آخر:

```bash
nc -C 127.0.0.1 6667
```

ثم جرّب:

```irc
PASS secret
NICK user1
USER user1 0 * :User One
JOIN #test
PRIVMSG #test :hello from nc
```

`-C` مهم لأنه يجعل `nc` يرسل نهاية السطر بصيغة IRC:

```text
\r\n
```

### اختبار الأوامر المقطوعة بـ `Ctrl+D`

السوجي يطلب التأكد من partial data. باستعمال `nc -C`:

```text
com
man
d
```

استعمل `Ctrl+D` بعد كل جزء لإرساله بدون إغلاق كامل في بعض نسخ `nc`.

الفكرة أن السيرفر يجب ألا ينفذ الأمر حتى يجد:

```text
\r\n
```

اختبار عملي:

```text
PA
SS secret
```

إذا buffer management صحيح، السيرفر سيفهمها كـ:

```irc
PASS secret
```

### Crash Test وقطع الاتصال المفاجئ

يجب تجربة هذه الحالات:

| الحالة | المتوقع |
| --- | --- |
| client يغلق terminal | السيرفر يحذف client بدون crash |
| `recv()` يرجع `0` | اعتبرها disconnect |
| `POLLHUP` | نظف client والقنوات |
| client داخل عدة قنوات ثم يخرج | احذفه من كل القنوات |
| operator يخرج | قرر هل تعطي operator لأول user آخر أو تتركها حسب تصميمك |
| channel تصبح فارغة | احذفها من `_channels` |

خطوات التنظيف عند disconnect:

1. احذف fd من `pollFds`.
2. احذف client من `_clients`.
3. أزل user من كل channel.
4. أرسل `QUIT` broadcast للقنوات إن لزم.
5. أغلق fd بـ `close(fd)`.

> لا تحذف عنصرًا من `std::vector<pollfd>` وأنت تكمل loop بنفس index بدون انتباه. إما استعمل erase بحذر، أو علّم fd للحذف ونظفه بعد نهاية الدورة.

### اختبار عدة Clients

افتح terminalين:

Client 1:

```irc
PASS secret
NICK user1
USER user1 0 * :User One
JOIN #room
```

Client 2:

```irc
PASS secret
NICK user2
USER user2 0 * :User Two
JOIN #room
PRIVMSG #room :salam user1
```

المتوقع: `user1` يستقبل رسالة `user2`.

### تجربة IRC Client حقيقي

المشروع يطلب اختيار reference client. اختيارات جيدة:

| Client | لماذا مفيد؟ |
| --- | --- |
| `irssi` | terminal-based، واضح في debug |
| `HexChat` | GUI client، يكشف مشاكل التوافق بسرعة |

مثال `irssi`:

```bash
irssi -c 127.0.0.1 -p 6667 -w secret -n user1
```

أو من داخل `irssi`:

```irc
/connect 127.0.0.1 6667 secret
/nick user1
/join #test
/msg #test hello
```

### Checklist قبل الدفاع

| الاختبار | الحالة |
| --- | --- |
| `make`, `make clean`, `make fclean`, `make re` تعمل | TODO |
| يترجم بـ `-Wall -Wextra -Werror -std=c++98` | TODO |
| السيرفر يرفض arguments غير صحيحة | TODO |
| `PASS/NICK/USER` تعمل بترتيبات مختلفة مع errors صحيحة | TODO |
| client لا يعلق السيرفر | TODO |
| partial commands تعمل | TODO |
| عدة clients يتصلون في نفس الوقت | TODO |
| `JOIN` ينشئ قناة ويعطي operator لأول user | TODO |
| `PRIVMSG` user/channel تعمل | TODO |
| `KICK`, `INVITE`, `TOPIC`, `MODE` تعمل | TODO |
| disconnect لا يسبب crash | TODO |
| reference client يتصل بدون errors ظاهرة | TODO |

### ترتيب دراسة وتنفيذ مقترح

| الترتيب | المهمة | النتيجة المطلوبة |
| --- | --- | --- |
| 1 | اقرأ subject وحدد allowed functions | تعرف الحدود |
| 2 | ابن listening socket | السيرفر يسمع على port |
| 3 | فعل non-blocking | لا توجد operations حاصرة |
| 4 | ابن `poll()` loop | عدة FDs في process واحد |
| 5 | أضف `Client` و buffers | استقبال data بشكل صحيح |
| 6 | أضف line extraction بـ `\r\n` | partial data تعمل |
| 7 | أضف parser | command + params |
| 8 | طبق `PASS/NICK/USER` | تسجيل clients |
| 9 | طبق numeric replies | client حقيقي يفهم السيرفر |
| 10 | أضف `Channel` | users/operators/topic/modes |
| 11 | طبق `JOIN/PRIVMSG` | chat أساسي |
| 12 | طبق `KICK/INVITE/TOPIC/MODE` | متطلبات subject |
| 13 | اختبر crash/disconnect/partial send | استقرار |
| 14 | جرّب `irssi` أو `HexChat` | توافق حقيقي |

> خدم المشروع طبقة بطبقة. إذا حاولت تبدأ بـ `MODE` قبل ما يكون عندك buffer management و registration واضحين، غادي تزيد التعقيد على راسك. الأساس القوي هنا هو نصف المشروع.

</div>
