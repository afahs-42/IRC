# Phase 1 Documentation - Ahmad's Work

## Overview
This phase implements the **core networking infrastructure** for the IRC server. It handles:
- Creating a TCP server socket
- Accepting multiple client connections
- Non-blocking I/O using `poll()`
- Receiving and buffering messages from clients
- Managing client lifecycle (connect/disconnect)

---

## File Structure

```
includes/
├── Server.hpp    → Server class definition
└── Client.hpp    → Client class definition

srcs/
├── main.cpp      → Program entry point
├── Server.cpp    → Server implementation
└── Client.cpp    → Client implementation
```

---

## Core Concepts

### 1. Non-Blocking I/O
**What it means:**
- Normal socket operations (read/write) **block** the program until they complete
- Non-blocking means the operation returns immediately, even if data isn't ready
- This allows handling multiple clients without threads

**How we do it:**
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```
This sets the file descriptor `fd` to non-blocking mode.

---

### 2. poll() System Call
**What it does:**
- Monitors multiple file descriptors (sockets) simultaneously
- Tells us which sockets have data ready to read
- Blocks until at least one socket has activity

**Structure:**
```cpp
struct pollfd {
    int fd;         // file descriptor to monitor
    short events;   // what events to watch for (POLLIN = data available)
    short revents;  // what events actually occurred (filled by poll)
};
```

**Our usage:**
```cpp
std::vector<struct pollfd> _fds;
poll(&_fds[0], _fds.size(), -1);  // -1 = wait forever
```

We maintain a vector of `pollfd` structs:
- `_fds[0]` → server socket (listens for new connections)
- `_fds[1]` → first client
- `_fds[2]` → second client
- etc...

---

## Class Breakdown

### Client Class (`Client.hpp` / `Client.cpp`)

**Purpose:** Represents a single connected client

#### Private Attributes:
```cpp
int _fd;                    // socket file descriptor
std::string _nickname;      // IRC nickname
std::string _username;      // IRC username
std::string _buffer;        // incomplete message buffer
bool _authenticated;        // has provided password?
```

#### Key Methods:

**Constructor:**
```cpp
Client::Client(int fd) : _fd(fd), _authenticated(false)
```
- Takes the socket file descriptor
- Initializes authenticated to false

**Buffer Management:**
```cpp
void appendBuffer(const std::string& data)  // add data to buffer
void clearBuffer()                          // empty the buffer
```
**Why we need a buffer:**
- IRC messages end with `\r\n`
- We might receive partial messages: `"NICK al"` then later `"ice\r\n"`
- We store incomplete data until we get `\r\n`

**sendMessage:**
```cpp
void Client::sendMessage(const std::string& message)
{
    std::string msg = message + "\r\n";
    send(_fd, msg.c_str(), msg.length(), 0);
}
```
- Adds `\r\n` to every message (IRC protocol requirement)
- Uses `send()` system call to write data to socket

---

### Server Class (`Server.hpp` / `Server.cpp`)

**Purpose:** Manages the server socket and all clients

#### Private Attributes:
```cpp
int _serverFd;                          // server's listening socket
int _port;                              // port number (e.g., 6667)
std::string _password;                  // server password
std::vector<Client*> _clients;          // list of connected clients
std::vector<struct pollfd> _fds;        // poll() monitoring list
```

⚠️ **Important:** `_fds` and `_clients` have different sizes!
- `_fds[0]` is the server socket
- `_fds[i]` corresponds to `_clients[i-1]` for i > 0

---

### Server Constructor - Step by Step

```cpp
Server::Server(int port, const std::string& password)
```

#### Step 1: Create Socket
```cpp
_serverFd = socket(AF_INET, SOCK_STREAM, 0);
```
- `AF_INET` → IPv4
- `SOCK_STREAM` → TCP (reliable, ordered)
- Returns a file descriptor (integer) representing the socket

#### Step 2: Set Socket Options
```cpp
int opt = 1;
setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```
- `SO_REUSEADDR` → allows reusing the port immediately after program closes
- Without this, you'd get "Address already in use" error

#### Step 3: Make Socket Non-Blocking
```cpp
fcntl(_serverFd, F_SETFL, O_NONBLOCK);
```
- Now `accept()` won't block if no clients are connecting

#### Step 4: Bind to Port
```cpp
struct sockaddr_in serverAddr;
serverAddr.sin_family = AF_INET;
serverAddr.sin_addr.s_addr = INADDR_ANY;  // listen on all interfaces
serverAddr.sin_port = htons(_port);       // convert port to network byte order

bind(_serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
```
- Associates the socket with a specific port
- `INADDR_ANY` means accept connections from any IP address
- `htons()` converts port from host to network byte order (big-endian)

#### Step 5: Start Listening
```cpp
listen(_serverFd, 10);
```
- Makes socket ready to accept connections
- `10` is the backlog (max pending connections queue)

#### Step 6: Add Server Socket to poll()
```cpp
struct pollfd serverPollFd;
serverPollFd.fd = _serverFd;
serverPollFd.events = POLLIN;  // watch for incoming connections
_fds.push_back(serverPollFd);
```
- Now `poll()` will monitor the server socket

---

### Server Destructor

```cpp
Server::~Server()
{
    for (size_t i = 0; i < _clients.size(); i++)
        delete _clients[i];
    _clients.clear();
    
    if (_serverFd >= 0)
        close(_serverFd);
}
```

**What happens here:**
1. Delete all client objects (frees memory)
2. Clear the vector
3. Close the server socket

⚠️ **Prevents memory leaks:** Every `new Client()` must have a matching `delete`

---

### acceptNewClient() - Accepting Connections

```cpp
void Server::acceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int clientFd = accept(_serverFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0)
        return;  // no connection ready (non-blocking)

    fcntl(clientFd, F_SETFL, O_NONBLOCK);  // make client socket non-blocking

    Client* newClient = new Client(clientFd);
    _clients.push_back(newClient);

    struct pollfd clientPollFd;
    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN;  // watch for data from this client
    _fds.push_back(clientPollFd);
}
```

**Step-by-step:**
1. `accept()` creates a new socket for the client connection
2. Make client socket non-blocking
3. Create a `Client` object (allocated with `new`)
4. Add client to `_clients` vector
5. Add client's socket to `_fds` for `poll()` monitoring

---

### handleClientMessage() - Receiving Data

```cpp
void Server::handleClientMessage(int index)
{
    char buffer[512];
    Client* client = _clients[index - 1];  // adjust index (-1 because server socket)
    
    int bytesRead = recv(client->getFd(), buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0)
    {
        removeClient(index);  // client disconnected or error
        return;
    }

    buffer[bytesRead] = '\0';
    client->appendBuffer(std::string(buffer));

    // Process complete messages (ending with \r\n)
    std::string buf = client->getBuffer();
    size_t pos;
    while ((pos = buf.find("\r\n")) != std::string::npos)
    {
        std::string message = buf.substr(0, pos);
        buf.erase(0, pos + 2);
        
        std::cout << "Received: " << message << std::endl;
        // TODO: Pass to Parser (Carla's work)
    }
    
    client->clearBuffer();
    client->appendBuffer(buf);  // save incomplete part
}
```

**How message buffering works:**

Example scenario:
1. Client sends: `"NICK ali"` (no `\r\n` yet)
   - We store `"NICK ali"` in buffer
   - No complete message to process

2. Client sends: `"ce\r\n"`
   - Buffer now: `"NICK alice\r\n"`
   - We find `\r\n`, extract `"NICK alice"`
   - Clear buffer (or save remaining data if more messages)

3. Client sends: `"USER alice 0 * :Real Name\r\nJOIN #chan"`
   - First message complete: `"USER alice 0 * :Real Name"`
   - Second message incomplete: `"JOIN #chan"` stays in buffer

---

### removeClient() - Cleanup

```cpp
void Server::removeClient(int index)
{
    Client* client = _clients[index - 1];
    close(client->getFd());  // close socket
    
    delete client;  // free memory
    _clients.erase(_clients.begin() + (index - 1));
    _fds.erase(_fds.begin() + index);
}
```

**Important index handling:**
- `_fds[0]` = server socket
- `_fds[index]` = client socket
- `_clients[index - 1]` = client object

Must remove from both vectors!

---

### run() - Main Event Loop

```cpp
void Server::run()
{
    while (true)
    {
        int pollCount = poll(&_fds[0], _fds.size(), -1);
        if (pollCount < 0)
            break;

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)  // data ready to read
            {
                if (_fds[i].fd == _serverFd)
                    acceptNewClient();
                else
                    handleClientMessage(i);
            }
        }
    }
}
```

**Flow:**
1. `poll()` waits until activity on any socket
2. Loop through all sockets
3. Check `revents` (returned events)
4. If data ready (`POLLIN`):
   - Server socket → new connection → `acceptNewClient()`
   - Client socket → data from client → `handleClientMessage()`

**Visual representation:**
```
poll() waits...
    ↓
Activity on socket!
    ↓
Is it server socket? → Yes → accept new client
    ↓
    No
    ↓
Is it client socket? → Yes → read message from client
```

---

## main.cpp - Entry Point

```cpp
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: Invalid port number" << std::endl;
        return 1;
    }

    std::string password = argv[2];
    if (password.empty())
    {
        std::cerr << "Error: Password cannot be empty" << std::endl;
        return 1;
    }

    try
    {
        Server server(port, password);
        server.run();  // runs forever
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

**Validation:**
1. Check argument count
2. Convert port string to integer
3. Validate port range (1-65535)
4. Check password not empty
5. Create server in try-catch (handles constructor errors)

---

## Testing Phase 1

### Compile:
```bash
make
```

### Run Server:
```bash
./ircserv 6667 mypassword
```

### Test with netcat:
```bash
nc -C 127.0.0.1 6667
```

Then type:
```
NICK alice
USER alice 0 * :Alice Smith
```

### Test partial messages (ctrl+D):
```bash
nc -C 127.0.0.1 6667
NI^DCK ^Dal^Dice^D\r\n
```
(Press ctrl+D after each `^D`)

Server should reconstruct: `"NICK alice"`

---

## Integration with Carla's Work

**Current (Phase 1):**
```cpp
std::string message = buf.substr(0, pos);
std::cout << "Received: " << message << std::endl;
```

**After integration (Phase 2):**
```cpp
std::string message = buf.substr(0, pos);
Command cmd = Parser::parseMessage(message);
// Process command based on cmd.command
```

---

## Memory Management

### Allocations:
```cpp
Client* newClient = new Client(clientFd);  // allocate
```

### Deallocations:
```cpp
delete client;  // free in removeClient()
```

### Destructor cleanup:
```cpp
for (size_t i = 0; i < _clients.size(); i++)
    delete _clients[i];
```

**Check for leaks:**
```bash
valgrind --leak-check=full ./ircserv 6667 pass
```

---

## Common Issues & Solutions

### Issue: "Address already in use"
**Solution:** `SO_REUSEADDR` flag in constructor

### Issue: Server blocks on single client
**Solution:** Non-blocking I/O with `O_NONBLOCK`

### Issue: Partial messages not handled
**Solution:** Buffer incomplete data in `Client::_buffer`

### Issue: Memory leaks
**Solution:** Delete all clients in destructor

### Issue: Crash on client disconnect
**Solution:** Check `bytesRead <= 0` in `handleClientMessage()`

---

## Next Steps (Phase 2)

Ahmad will need to:
1. Integrate Carla's `Parser::parseMessage()`
2. Route commands to handlers
3. Implement command execution framework
4. Add reply sending utilities

Carla will provide:
- `Parser` class with `parseMessage()` method
- `Command` struct with parsed data
- Command validation logic