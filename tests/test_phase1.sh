#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Server configuration
PORT=6667
PASSWORD="testpass"
SERVER_PID=""
TEMP_DIR=""

# Function to create temporary directory
setup_temp_dir() {
    TEMP_DIR=$(mktemp -d)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to create temporary directory${NC}"
        exit 1
    fi
}

# Function to cleanup temporary directory
cleanup_temp_dir() {
    if [ ! -z "$TEMP_DIR" ] && [ -d "$TEMP_DIR" ]; then
        rm -rf "$TEMP_DIR"
    fi
}

# Function to start server
start_server() {
    echo -e "${YELLOW}Starting IRC server on port $PORT...${NC}"
    ./ircserv $PORT $PASSWORD > "$TEMP_DIR/server_output.log" 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}FAILED: Server failed to start${NC}"
        cat "$TEMP_DIR/server_output.log"
        exit 1
    fi
    echo -e "${GREEN}Server started with PID $SERVER_PID${NC}"
}

# Function to stop server
stop_server() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server (PID $SERVER_PID)...${NC}"
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
        SERVER_PID=""
    fi
}

# Function to report test result
report_test() {
    local test_name=$1
    local result=$2
    
    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}✓ PASSED${NC}: $test_name"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAILED${NC}: $test_name"
        ((TESTS_FAILED++))
    fi
}

# Test 1: Server startup with valid parameters
test_server_startup() {
    echo -e "\n${YELLOW}Test 1: Server startup with valid parameters${NC}"
    
    ./ircserv $PORT $PASSWORD > $TEMP_DIR/test_startup.log 2>&1 &
    local pid=$!
    sleep 1
    
    if kill -0 $pid 2>/dev/null; then
        kill $pid 2>/dev/null
        wait $pid 2>/dev/null
        report_test "Server startup" 0
    else
        cat $TEMP_DIR/test_startup.log
        report_test "Server startup" 1
    fi
}

# Test 2: Server rejects invalid port
test_invalid_port() {
    echo -e "\n${YELLOW}Test 2: Server rejects invalid port${NC}"
    
    ./ircserv 0 $PASSWORD > $TEMP_DIR/test_invalid_port.log 2>&1
    local result=$?
    
    if [ $result -ne 0 ]; then
        report_test "Invalid port rejection" 0
    else
        report_test "Invalid port rejection" 1
    fi
}

# Test 3: Server rejects empty password
test_empty_password() {
    echo -e "\n${YELLOW}Test 3: Server rejects empty password${NC}"
    
    ./ircserv $PORT "" > $TEMP_DIR/test_empty_pass.log 2>&1
    local result=$?
    
    if [ $result -ne 0 ]; then
        report_test "Empty password rejection" 0
    else
        report_test "Empty password rejection" 1
    fi
}

# Test 4: Client can connect
test_client_connect() {
    echo -e "\n${YELLOW}Test 4: Client can connect${NC}"
    
    start_server
    
    # Try to connect with nc
    timeout 2 nc 127.0.0.1 $PORT < /dev/null
    local result=$?
    
    stop_server
    
    # nc returns 0 on successful connection (even if it closes immediately)
    if [ $result -eq 0 ] || [ $result -eq 124 ]; then
        report_test "Client connection" 0
    else
        report_test "Client connection" 1
    fi
}

# Test 5: Server accepts multiple clients
test_multiple_clients() {
    echo -e "\n${YELLOW}Test 5: Server accepts multiple clients${NC}"
    
    start_server
    
    # Connect 3 clients simultaneously
    timeout 2 nc 127.0.0.1 $PORT < /dev/null &
    local pid1=$!
    timeout 2 nc 127.0.0.1 $PORT < /dev/null &
    local pid2=$!
    timeout 2 nc 127.0.0.1 $PORT < /dev/null &
    local pid3=$!
    
    sleep 1
    
    # Check server log for multiple connections
    local conn_count=$(grep -c "New client connected" $TEMP_DIR/server_output.log 2>/dev/null || echo "0")
    
    kill $pid1 $pid2 $pid3 2>/dev/null
    wait $pid1 $pid2 $pid3 2>/dev/null
    
    stop_server
    
    if [ "$conn_count" -ge 2 ]; then
        report_test "Multiple client connections" 0
    else
        echo "  Only $conn_count clients connected"
        report_test "Multiple client connections" 1
    fi
}

# Test 6: Server receives and processes messages
test_message_reception() {
    echo -e "\n${YELLOW}Test 6: Server receives complete messages${NC}"
    
    start_server
    
    # Send a complete message
    echo -e "NICK testuser\r\n" | nc 127.0.0.1 $PORT &
    sleep 1
    
    # Check if message was received
    if grep -q "Received.*NICK testuser" $TEMP_DIR/server_output.log; then
        report_test "Message reception" 0
    else
        cat $TEMP_DIR/server_output.log
        report_test "Message reception" 1
    fi
    
    stop_server
}

# Test 7: Server handles partial messages (buffering)
test_partial_messages() {
    echo -e "\n${YELLOW}Test 7: Server handles partial messages (buffering)${NC}"
    
    start_server
    
    # Send message in parts using printf with delays
    (
        printf "NICK "
        sleep 0.5
        printf "test"
        sleep 0.5
        printf "user\r\n"
    ) | nc 127.0.0.1 $PORT &
    
    sleep 2
    
    # Check if complete message was reconstructed
    if grep -q "Received.*NICK testuser" $TEMP_DIR/server_output.log; then
        report_test "Partial message buffering" 0
    else
        cat $TEMP_DIR/server_output.log
        report_test "Partial message buffering" 1
    fi
    
    stop_server
}

# Test 8: Server handles client disconnection
test_client_disconnect() {
    echo -e "\n${YELLOW}Test 8: Server handles client disconnection${NC}"
    
    start_server
    
    # Connect and immediately disconnect
    timeout 2 bash -c "echo -e 'NICK test\r\n' | nc 127.0.0.1 $PORT" 2>/dev/null
    sleep 2
    
    # Check if disconnection was logged
    if grep -q "disconnected" $TEMP_DIR/server_output.log; then
        report_test "Client disconnection handling" 0
    else
        cat $TEMP_DIR/server_output.log
        report_test "Client disconnection handling" 1
    fi
    
    stop_server
}

# Test 9: Parser handles valid IRC commands
test_parser_valid_commands() {
    echo -e "\n${YELLOW}Test 9: Parser accepts valid IRC commands${NC}"
    
    start_server
    
    # Send various valid commands
    (
        echo -e "NICK alice\r\n"
        sleep 0.2
        echo -e "USER alice 0 * :Alice Smith\r\n"
        sleep 0.2
        echo -e "JOIN #test\r\n"
    ) | nc 127.0.0.1 $PORT &
    
    sleep 2
    
    # Count received messages
    local msg_count=$(grep -c "Received" $TEMP_DIR/server_output.log 2>/dev/null || echo "0")
    
    if [ "$msg_count" -ge 3 ]; then
        report_test "Parser valid commands" 0
    else
        echo "  Only $msg_count messages received"
        cat $TEMP_DIR/server_output.log
        report_test "Parser valid commands" 1
    fi
    
    stop_server
}

# Test 10: Server maintains non-blocking I/O
test_nonblocking_io() {
    echo -e "\n${YELLOW}Test 10: Server maintains non-blocking I/O${NC}"
    
    start_server
    
    # Connect one client and keep it open
    nc 127.0.0.1 $PORT > /dev/null 2>&1 &
    local client1=$!
    sleep 0.5
    
    # Try to connect another client while first is active
    echo -e "NICK test\r\n" | nc 127.0.0.1 $PORT &
    local client2=$!
    sleep 1
    
    # Check if both clients connected (server didn't block)
    local conn_count=$(grep -c "New client connected" $TEMP_DIR/server_output.log 2>/dev/null || echo "0")
    
    kill $client1 $client2 2>/dev/null
    wait $client1 $client2 2>/dev/null
    
    stop_server
    
    if [ "$conn_count" -ge 2 ]; then
        report_test "Non-blocking I/O" 0
    else
        echo "  Only $conn_count clients connected (blocking detected)"
        report_test "Non-blocking I/O" 1
    fi
}

# Main test execution
main() {
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}  IRC Server Phase 1 Test Suite${NC}"
    echo -e "${YELLOW}========================================${NC}"
    
    # Check if server binary exists
    if [ ! -f "./ircserv" ]; then
        echo -e "${RED}ERROR: ircserv binary not found. Please run 'make' first.${NC}"
        exit 1
    fi
    
    # Check if binary is executable
    if [ ! -x "./ircserv" ]; then
        echo -e "${RED}ERROR: ircserv is not executable. Please run 'chmod +x ircserv'.${NC}"
        exit 1
    fi
    
    # Set up temporary directory
    setup_temp_dir
    
    # Run all tests
    test_server_startup
    test_invalid_port
    test_empty_password
    test_client_connect
    test_multiple_clients
    test_message_reception
    test_partial_messages
    test_client_disconnect
    test_parser_valid_commands
    test_nonblocking_io
    
    # Summary
    echo -e "\n${YELLOW}========================================${NC}"
    echo -e "${YELLOW}  Test Summary${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    echo -e "Total: $((TESTS_PASSED + TESTS_FAILED))"
    
    # Cleanup
    cleanup_temp_dir
    
    # Exit with appropriate code
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}All tests passed!${NC}"
        exit 0
    else
        echo -e "\n${RED}Some tests failed!${NC}"
        exit 1
    fi
}

# Cleanup on exit
trap 'stop_server; cleanup_temp_dir' EXIT

# Run main
main
