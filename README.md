# ft_irc - Project Plan

## Project Overview
An IRC (Internet Relay Chat) server implementation in C++98 that handles multiple clients simultaneously using non-blocking I/O operations.

## Team Structure
- **Carla**: Parsing & Command Processing
- **Ahmad**: Server Core & Network Management

---

## Project Structure
```
ft_irc/
├── Makefile
├── README.md
├── includes/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── Parser.hpp
│   ├── Command.hpp
│   └── Utils.hpp
├── srcs/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── Parser.cpp
│   ├── commands/
│   │   ├── authenticate.cpp
│   │   ├── nick.cpp
│   │   ├── user.cpp
│   │   ├── join.cpp
│   │   ├── privmsg.cpp
│   │   ├── kick.cpp
│   │   ├── invite.cpp
│   │   ├── topic.cpp
│   │   └── mode.cpp
│   └── utils/
│       └── Utils.cpp
└── config/ (optional)
```

---

## Phase 1: Foundation & Core Setup

### **Study & Research Topics:**
- IRC Protocol basics (RFC 1459, RFC 2812)
- TCP/IP sockets in C++
- `poll()` / `select()` / `epoll()` for non-blocking I/O
- C++98 standard (no C++11 features)
- File descriptors and socket programming
- IRC message format: `:<prefix> <command> <params> :trailing\r\n`

### **Carla's Tasks (Parsing):**

**Files to Create:**
- `includes/Parser.hpp`
- `srcs/Parser.cpp`
- `includes/Command.hpp`

**Responsibilities:**
- Design the IRC message parser
  - Parse incoming IRC messages (handle `\r\n` delimiters)
  - Extract prefix, command, parameters, and trailing parts
  - Handle partial messages (buffering incomplete data)
  - Validate message format
- Create Command structure/class
  - Store parsed command data
  - Command validation
- Handle message aggregation (for `ctrl+D` test case)

**Deliverables:**
```cpp
class Parser {
public:
    static Command parseMessage(const std::string& raw);
    static bool isComplete(const std::string& buffer);
};

struct Command {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::string trailing;
};
```

### **Ahmad's Tasks (Server Core):**

**Files to Create:**
- `includes/Server.hpp`
- `srcs/Server.cpp`
- `includes/Client.hpp`
- `srcs/Client.cpp`
- `srcs/main.cpp`
- `Makefile`

**Responsibilities:**
- Set up TCP socket server
  - Create listening socket
  - Bind to port and listen
  - Handle `poll()` for multiple clients
- Implement non-blocking I/O
  - Set `O_NONBLOCK` flag on file descriptors
  - Handle `EAGAIN` / `EWOULDBLOCK` errors
- Client connection management
  - Accept new connections
  - Store client data (fd, nickname, username, etc.)
  - Handle client disconnections
- Basic message sending/receiving
  - Receive data from clients
  - Send data to clients
  - Buffer management for partial reads/writes

**Deliverables:**
```cpp
class Server {
private:
    int serverFd;
    int port;
    std::string password;
    std::vector<Client> clients;
    std::vector<struct pollfd> fds;
public:
    Server(int port, const std::string& password);
    void run();
    void acceptNewClient();
    void handleClientMessage(int clientFd);
};

class Client {
private:
    int fd;
    std::string nickname;
    std::string username;
    std::string buffer; // for partial messages
    bool authenticated;
public:
    // getters, setters, send/receive methods
};
```

### **Meeting & Merge (Phase 1):**
**Goals:**
- Integrate Parser with Server
- Test basic message reception and parsing
- Verify non-blocking I/O works correctly
- Test with `nc` command: `nc -C 127.0.0.1 <port>`
- Test partial message handling (ctrl+D test)

**Integration Points:**
- Server receives raw data → passes to Parser
- Parser returns Command → Server processes it

---

## Phase 2: Authentication & Basic Commands

### **Study & Research Topics:**
- IRC authentication flow (PASS, NICK, USER)
- IRC numeric replies (001-005 for welcome messages)
- Error handling and reply codes

### **Carla's Tasks (Command Handlers):**

**Files to Create:**
- `srcs/commands/authenticate.cpp` (PASS command)
- `srcs/commands/nick.cpp` (NICK command)
- `srcs/commands/user.cpp` (USER command)

**Responsibilities:**
- Implement PASS command
  - Verify password matches server password
  - Must be first command before registration
- Implement NICK command
  - Validate nickname format
  - Check for nickname collisions
  - Handle nickname changes
- Implement USER command
  - Set username and realname
  - Complete registration after PASS and NICK
- Send appropriate numeric replies (001, 433, 461, 462, etc.)

### **Ahmad's Tasks (Client State & Command Routing):**

**Files to Create:**
- `includes/Utils.hpp`
- `srcs/utils/Utils.cpp`
- Update `Server.cpp` and `Client.cpp`

**Responsibilities:**
- Implement command routing system
  - Map commands to handler functions
  - Pass parsed Command to appropriate handler
- Client state management
  - Track authentication state
  - Track registration completion
  - Enforce command order (PASS → NICK → USER)
- Implement reply sending utilities
  - Format IRC replies with proper prefix
  - Send numeric replies
  - Handle server-to-client message formatting

### **Meeting & Merge (Phase 2):**
**Goals:**
- Test full authentication flow
- Connect with actual IRC client (irssi, HexChat, WeeChat)
- Verify client can authenticate and register
- Test error cases (wrong password, duplicate nicknames)

---

## Phase 3: Channels & Messaging

### **Study & Research Topics:**
- Channel mechanics in IRC
- Private messaging (PRIVMSG)
- Channel membership and modes
- Message broadcasting

### **Carla's Tasks (Message Commands):**

**Files to Create:**
- `srcs/commands/join.cpp`
- `srcs/commands/privmsg.cpp`
- `includes/Channel.hpp`
- `srcs/Channel.cpp`

**Responsibilities:**
- Implement JOIN command
  - Create channels if they don't exist
  - Add clients to channels
  - Send join confirmation and user list
  - Handle channel keys (passwords) if MODE +k is set
- Implement PRIVMSG command
  - Handle private messages to users
  - Handle channel messages
  - Validate targets exist
- Design Channel class structure
  - Store channel members
  - Store channel properties (topic, modes, etc.)

### **Ahmad's Tasks (Channel Management):**

**Files to Create:**
- Update `Server.cpp` to manage channels
- Update `Client.cpp` to track joined channels

**Responsibilities:**
- Channel storage and lookup
  - Store all active channels
  - Find channels by name
  - Clean up empty channels
- Message broadcasting
  - Send message to all channel members
  - Exclude sender from broadcast
  - Handle user-to-user private messages
- Track channel membership
  - Associate clients with channels
  - Associate channels with clients (bidirectional)

**Deliverables:**
```cpp
class Channel {
private:
    std::string name;
    std::string topic;
    std::string key; // password
    std::vector<Client*> members;
    std::vector<Client*> operators;
    bool inviteOnly;
    bool topicRestricted;
    int userLimit;
public:
    void broadcast(const std::string& message, Client* exclude);
    bool isMember(Client* client);
    bool isOperator(Client* client);
};
```

### **Meeting & Merge (Phase 3):**
**Goals:**
- Test joining channels
- Test sending channel messages (all members receive)
- Test private messages between users
- Connect multiple IRC clients and verify communication

---

## Phase 4: Channel Operators & Modes

### **Study & Research Topics:**
- IRC channel modes (i, t, k, o, l)
- Operator privileges
- MODE command syntax
- KICK, INVITE, TOPIC commands

### **Carla's Tasks (Operator Commands):**

**Files to Create:**
- `srcs/commands/kick.cpp`
- `srcs/commands/invite.cpp`
- `srcs/commands/topic.cpp`

**Responsibilities:**
- Implement KICK command
  - Verify operator privileges
  - Remove user from channel
  - Broadcast kick message
- Implement INVITE command
  - Verify operator privileges (if channel is +i)
  - Send invitation
  - Track invitations for invite-only channels
- Implement TOPIC command
  - View current topic
  - Set topic (if operator or +t not set)
  - Broadcast topic changes

### **Ahmad's Tasks (MODE Command):**

**Files to Create:**
- `srcs/commands/mode.cpp`

**Responsibilities:**
- Implement MODE command
  - Parse mode strings (+i, -t, +k, etc.)
  - Handle each mode type:
    - `i`: invite-only
    - `t`: topic restriction
    - `k`: channel key (password)
    - `o`: operator privilege
    - `l`: user limit
  - Validate operator permissions
  - Apply mode changes
  - Broadcast mode changes

### **Meeting & Merge (Phase 4):**
**Goals:**
- Test all operator commands
- Test mode changes (setting/removing each mode)
- Test permission enforcement (non-operators can't use operator commands)
- Verify invite-only channels work
- Verify channel keys work
- Verify user limits work

---

## Phase 5: Polish & Error Handling

### **Study & Research Topics:**
- IRC error codes and numeric replies
- Edge cases and error scenarios
- Memory leak detection (valgrind)
- Signal handling (SIGINT, SIGPIPE)

### **Carla's Tasks:**
- Comprehensive error handling in all commands
- Add all missing numeric replies
- Input validation and sanitization
- Handle edge cases in parsing

### **Ahmad's Tasks:**
- Resource cleanup (close sockets, free memory)
- Signal handling (graceful shutdown)
- Handle client disconnections properly
- Fix memory leaks
- Handle edge cases (bandwidth, partial data)
- Optimize poll() usage

### **Meeting & Merge (Phase 5):**
**Goals:**
- Run valgrind tests (no leaks)
- Test with low bandwidth scenarios
- Test sudden disconnections
- Test with multiple clients (stress test)
- Verify no crashes under any circumstance
- Final testing with reference IRC client

---

## Phase 6: Bonus (Optional)

### **Study & Research Topics:**
- DCC (Direct Client-to-Client) for file transfer
- Bot implementation basics

### **Potential Tasks:**
- File transfer implementation
- Create a simple bot (responds to commands)

---

## Testing Strategy

### **Tools:**
- **Reference IRC Client**: Choose one (irssi, WeeChat, HexChat)
- **Testing Tools**: 
  - `nc -C 127.0.0.1 <port>` for manual testing
  - `valgrind` for memory leaks
  - Multiple client instances for concurrency testing

### **Test Cases:**
1. Connection and authentication
2. Nickname changes and collisions
3. Joining multiple channels
4. Sending/receiving messages
5. Operator commands (KICK, INVITE, TOPIC, MODE)
6. Edge cases (partial messages, disconnections)
7. Permission enforcement
8. Error handling (invalid commands, wrong parameters)

---

## Key Implementation Notes

### **Non-Blocking I/O Requirements:**
- Must use `poll()` (or `select()`/`epoll()`/`kqueue()`)
- Only ONE `poll()` call for all operations
- All file descriptors must be non-blocking: `fcntl(fd, F_SETFL, O_NONBLOCK)`
- Handle `EAGAIN`/`EWOULDBLOCK` properly

### **Message Format:**
```
:<prefix> <command> <param1> <param2> ... :<trailing>
```
- Messages end with `\r\n`
- May receive partial messages (need buffering)

### **C++98 Compliance:**
- No C++11 features (no `auto`, `nullptr`, range-based for loops)
- Use `std::vector`, `std::map`, `std::string`
- Compile with: `c++ -Wall -Wextra -Werror -std=c++98`

### **Makefile Requirements:**
```makefile
NAME = ircserv
all: $(NAME)
clean:
    # remove object files
fclean: clean
    # remove executable
re: fclean all
```

---

## Timeline Suggestion

- **Phase 1**: 3-4 days (foundation is crucial)
- **Phase 2**: 2-3 days (authentication)
- **Phase 3**: 3-4 days (channels and messaging)
- **Phase 4**: 3-4 days (operator commands)
- **Phase 5**: 2-3 days (polish and testing)
- **Phase 6**: 1-2 days (bonus, if time permits)

**Total**: ~2-3 weeks

---

## Resources

### **IRC Protocol:**
- RFC 1459: https://tools.ietf.org/html/rfc1459
- RFC 2812: https://tools.ietf.org/html/rfc2812
- Modern IRC docs: https://modern.ircdocs.horse/

### **Socket Programming:**
- Beej's Guide to Network Programming
- `man poll`, `man socket`, `man fcntl`

### **IRC Numeric Replies:**
- https://www.alien.net.au/irc/irc2numerics.html

---

## Collaboration Workflow

1. **Branch Strategy**: 
   - `main`: stable code only
   - `carla-dev`: Carla's working branch
   - `ahmad-dev`: Ahmad's working branch

2. **Merge Process**:
   - Complete individual tasks
   - Test individually
   - Code review together
   - Merge to `main`
   - Test integration

3. **Communication**:
   - Daily sync (15 min)
   - Phase-end meetings (detailed review)
   - Document decisions and blockers

---

## Success Criteria

- [ ] Server handles multiple clients without blocking
- [ ] Full authentication flow works
- [ ] Channels work (join, message, leave)
- [ ] All operator commands implemented (KICK, INVITE, TOPIC, MODE)
- [ ] All modes work (i, t, k, o, l)
- [ ] No memory leaks (valgrind clean)
- [ ] No crashes under any circumstance
- [ ] Reference IRC client can connect and use all features
- [ ] Passes `nc` ctrl+D test (partial message handling)
- [ ] Code compiles with `-Wall -Wextra -Werror -std=c++98`

