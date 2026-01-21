# IRC Server Phase 1 Tests

This directory contains automated tests for Phase 1 of the IRC server implementation.

## Overview

Phase 1 tests verify the core networking infrastructure including:
- Server socket creation and binding
- Non-blocking I/O with poll()
- Client connection handling
- Message reception and buffering
- Partial message handling
- Multi-client support
- Client disconnection handling

## Requirements

- `nc` (netcat) - for simulating IRC clients
- `bash` - for running test scripts
- Built `ircserv` binary (run `make` in project root)

## Running Tests

### Run all Phase 1 tests:
```bash
cd /home/runner/work/IRC/IRC
make
./tests/test_phase1.sh
```

## Test Cases

### 1. Server Startup
Verifies the server can start with valid port and password parameters.

### 2. Invalid Port Rejection
Ensures the server rejects invalid port numbers (0, negative, > 65535).

### 3. Empty Password Rejection
Ensures the server rejects empty password strings.

### 4. Client Connection
Tests that a single client can successfully connect to the server.

### 5. Multiple Client Connections
Verifies the server can accept and handle multiple simultaneous client connections using poll().

### 6. Message Reception
Tests that the server correctly receives complete IRC messages ending with `\r\n`.

### 7. Partial Message Buffering
Verifies the server buffers incomplete messages and reconstructs them when the rest arrives.
This is critical for the "ctrl+D" test case mentioned in Phase 1 documentation.

### 8. Client Disconnection Handling
Ensures the server properly detects and handles client disconnections without crashing.

### 9. Parser Valid Commands
Tests that the parser accepts and processes valid IRC commands (NICK, USER, JOIN, etc.).

### 10. Non-Blocking I/O
Verifies the server maintains non-blocking I/O and can handle multiple clients without blocking.

## Expected Output

A successful test run will show:
```
========================================
  IRC Server Phase 1 Test Suite
========================================

Test 1: Server startup with valid parameters
✓ PASSED: Server startup

Test 2: Server rejects invalid port
✓ PASSED: Invalid port rejection

...

========================================
  Test Summary
========================================
Passed: 10
Failed: 0
Total: 10

All tests passed!
```

## Manual Testing

You can also manually test the server using netcat:

### Start the server:
```bash
./ircserv 6667 mypassword
```

### Connect with a client:
```bash
nc -C 127.0.0.1 6667
```

### Send IRC commands:
```
NICK alice
USER alice 0 * :Alice Smith
JOIN #test
```

### Test partial messages (ctrl+D):
```bash
nc -C 127.0.0.1 6667
# Type: NI
# Press Ctrl+D
# Type: CK alice
# Press Ctrl+D
# Type: \r\n
# Server should receive: "NICK alice"
```

## Integration Points

These tests verify the integration between:
- **Server** class - manages connections and poll()
- **Client** class - maintains connection state and message buffer
- **Parser** class - parses IRC messages
- **Command** class - stores parsed command data

## Success Criteria

All tests must pass for Phase 1 to be considered complete according to the project plan:
- ✓ Server handles multiple clients without blocking
- ✓ Non-blocking I/O works correctly
- ✓ Message buffering handles partial data
- ✓ Parser integrates with Server
- ✓ Client connections and disconnections work properly

## Next Steps

After Phase 1 tests pass:
- **Phase 2**: Authentication & Basic Commands (PASS, NICK, USER)
- **Phase 3**: Channels & Messaging (JOIN, PRIVMSG)
- **Phase 4**: Channel Operators & Modes (KICK, INVITE, TOPIC, MODE)
