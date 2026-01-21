# Phase 1 Manual Testing Guide

This guide provides step-by-step instructions for manually testing the Phase 1 implementation of the IRC server.

## Prerequisites

- Built `ircserv` binary (run `make` in project root)
- `nc` (netcat) or an IRC client like `irssi`, `HexChat`, or `WeeChat`
- Terminal access

## Test 1: Basic Server Startup

### Steps:
1. Start the server:
   ```bash
   ./ircserv 6667 mypassword
   ```

### Expected Output:
```
Server listening on port 6667
```

### Pass Criteria:
- Server starts without errors
- Server binds to the specified port
- Server displays the listening message

---

## Test 2: Invalid Parameters

### Test 2a: Invalid Port

#### Steps:
1. Try starting with port 0:
   ```bash
   ./ircserv 0 mypassword
   ```

#### Expected Output:
```
Error: Invalid port number
```

### Test 2b: Missing Parameters

#### Steps:
1. Try starting without password:
   ```bash
   ./ircserv 6667
   ```

#### Expected Output:
```
Usage: ./ircserv <port> <password>
```

### Pass Criteria:
- Server rejects invalid inputs
- Appropriate error messages are displayed
- Server exits with non-zero code

---

## Test 3: Single Client Connection

### Steps:
1. Start the server in one terminal:
   ```bash
   ./ircserv 6667 mypassword
   ```

2. In another terminal, connect with netcat:
   ```bash
   nc -C 127.0.0.1 6667
   ```

### Expected Server Output:
```
Server listening on port 6667
New client connected: <fd_number>
```

### Pass Criteria:
- Client successfully connects
- Server logs the new connection
- Connection remains open

---

## Test 4: Sending Messages

### Steps:
1. With a connected client (from Test 3), type and press Enter:
   ```
   NICK alice
   ```

### Expected Server Output:
```
Received from <fd>: NICK alice
```

### Steps (continued):
2. Send more commands:
   ```
   USER alice 0 * :Alice Smith
   JOIN #test
   ```

### Expected Server Output:
```
Received from <fd>: USER alice 0 * :Alice Smith
Received from <fd>: JOIN #test
```

### Pass Criteria:
- All messages are received by the server
- Messages are correctly displayed with their content
- Server doesn't crash or hang

---

## Test 5: Partial Message Buffering (ctrl+D Test)

This is a **critical test** for Phase 1 as mentioned in the documentation.

### Steps:
1. Connect with netcat:
   ```bash
   nc -C 127.0.0.1 6667
   ```

2. Type (without pressing Enter):
   ```
   NI
   ```

3. Press `Ctrl+D` (sends EOF, causing netcat to flush and wait)

4. Type more:
   ```
   CK al
   ```

5. Press `Ctrl+D` again

6. Complete the message:
   ```
   ice
   ```
   Then press Enter (which sends `\r\n`)

### Expected Server Output:
```
Received from <fd>: NICK alice
```

### Pass Criteria:
- Server buffers incomplete messages
- Server reconstructs the complete message when `\r\n` is received
- Server displays the full message as "NICK alice"

---

## Test 6: Multiple Simultaneous Clients

### Steps:
1. Start the server:
   ```bash
   ./ircserv 6667 mypassword
   ```

2. Open 3 different terminals and connect from each:
   
   **Terminal 2:**
   ```bash
   nc -C 127.0.0.1 6667
   ```
   
   **Terminal 3:**
   ```bash
   nc -C 127.0.0.1 6667
   ```
   
   **Terminal 4:**
   ```bash
   nc -C 127.0.0.1 6667
   ```

3. Send a message from each client:
   
   **Client 1:** `NICK alice`
   **Client 2:** `NICK bob`
   **Client 3:** `NICK charlie`

### Expected Server Output:
```
New client connected: <fd1>
New client connected: <fd2>
New client connected: <fd3>
Received from <fd1>: NICK alice
Received from <fd2>: NICK bob
Received from <fd3>: NICK charlie
```

### Pass Criteria:
- All clients can connect simultaneously
- Server handles all connections without blocking
- Each client's messages are received independently
- Server doesn't hang or crash

---

## Test 7: Client Disconnection

### Steps:
1. Connect a client:
   ```bash
   nc -C 127.0.0.1 6667
   ```

2. Send a message:
   ```
   NICK test
   ```

3. Close the connection:
   - Press `Ctrl+C` or type `Ctrl+D` twice

### Expected Server Output:
```
New client connected: <fd>
Received from <fd>: NICK test
Client <fd> disconnected
```

### Pass Criteria:
- Server detects client disconnection
- Server logs the disconnection
- Server doesn't crash
- Server removes client from internal structures

---

## Test 8: Non-Blocking Behavior

### Steps:
1. Connect one client and keep it open (don't send anything):
   ```bash
   nc -C 127.0.0.1 6667
   ```

2. **Without closing the first client**, connect a second client:
   ```bash
   nc -C 127.0.0.1 6667
   ```

3. From the second client, send a message:
   ```
   NICK test
   ```

### Expected Server Output:
```
New client connected: <fd1>
New client connected: <fd2>
Received from <fd2>: NICK test
```

### Pass Criteria:
- Second client can connect even though first is idle
- Server responds to second client without waiting for first
- This proves non-blocking I/O is working

---

## Test 9: Message Format Validation

### Steps:
1. Connect a client and send messages with different formats:
   
   **Valid IRC message:**
   ```
   NICK alice
   ```
   
   **With trailing:**
   ```
   USER alice 0 * :Alice Smith
   ```
   
   **With prefix:**
   ```
   :server.com NOTICE * :Test
   ```

### Expected Server Output:
```
Received from <fd>: NICK alice
Received from <fd>: USER alice 0 * :Alice Smith
Received from <fd>: :server.com NOTICE * :Test
```

### Pass Criteria:
- All valid IRC message formats are received
- Parser handles prefixes and trailing parameters
- Server doesn't crash on any valid format

---

## Test 10: Stress Test

### Steps:
1. Connect a client and rapidly send multiple messages:
   ```bash
   (
     echo "NICK alice"
     echo "USER alice 0 * :Alice"
     echo "JOIN #test"
     echo "JOIN #test2"
     echo "JOIN #test3"
   ) | nc -C 127.0.0.1 6667
   ```

### Expected Server Output:
```
New client connected: <fd>
Received from <fd>: NICK alice
Received from <fd>: USER alice 0 * :Alice
Received from <fd>: JOIN #test
Received from <fd>: JOIN #test2
Received from <fd>: JOIN #test3
Client <fd> disconnected
```

### Pass Criteria:
- All messages are received in order
- No messages are lost
- Server doesn't crash
- Connection closes cleanly

---

## Memory Leak Testing (Optional but Recommended)

### Steps:
1. Install valgrind if not already installed:
   ```bash
   sudo apt-get install valgrind
   ```

2. Run server with valgrind:
   ```bash
   valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 mypass
   ```

3. Connect clients, send messages, disconnect

4. Stop server with `Ctrl+C`

### Expected Output:
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: X allocs, X frees, Y bytes allocated

All heap blocks were freed -- no leaks are possible
```

### Pass Criteria:
- No memory leaks reported
- All allocated memory is freed
- "All heap blocks were freed" message appears

---

## Summary

After completing all manual tests, you should have verified:

- ✅ Server starts and binds to port correctly
- ✅ Invalid parameters are rejected
- ✅ Single clients can connect
- ✅ Messages are received and displayed
- ✅ Partial messages are buffered correctly (ctrl+D test)
- ✅ Multiple simultaneous clients work
- ✅ Client disconnections are handled
- ✅ Non-blocking I/O is functioning
- ✅ IRC message format parsing works
- ✅ Server is stable under load
- ✅ No memory leaks exist

If all tests pass, Phase 1 is successfully implemented!

## Troubleshooting

### Server won't start
- Check if port is already in use: `lsof -i :6667`
- Try a different port: `./ircserv 6668 mypass`

### Client can't connect
- Check firewall settings
- Verify server is running: `ps aux | grep ircserv`
- Check server output for errors

### Messages not received
- Ensure you're using `\r\n` line endings (use `nc -C`)
- Check server logs for parser errors
- Verify message format is valid IRC syntax
