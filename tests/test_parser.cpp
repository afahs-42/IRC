#include "../includes/Parser.hpp"
#include "../includes/Command.hpp"
#include <iostream>
#include <cassert>

// Color codes for output
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

int tests_passed = 0;
int tests_failed = 0;

void test_result(const std::string& test_name, bool passed)
{
	if (passed)
	{
		std::cout << GREEN << "✓ PASSED" << RESET << ": " << test_name << std::endl;
		tests_passed++;
	}
	else
	{
		std::cout << RED << "✗ FAILED" << RESET << ": " << test_name << std::endl;
		tests_failed++;
	}
}

void test_simple_command()
{
	std::cout << YELLOW << "\nTest: Simple command parsing" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("NICK alice\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "NICK" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "alice");
	
	test_result("Simple NICK command", passed);
}

void test_command_with_trailing()
{
	std::cout << YELLOW << "\nTest: Command with trailing parameter" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("USER alice 0 * :Alice Smith\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "USER" &&
				   cmd.getParams().size() == 3 &&
				   cmd.getParams()[0] == "alice" &&
				   cmd.getParams()[1] == "0" &&
				   cmd.getParams()[2] == "*" &&
				   cmd.getTrailing() == "Alice Smith");
	
	test_result("USER command with trailing", passed);
}

void test_command_with_prefix()
{
	std::cout << YELLOW << "\nTest: Command with prefix" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage(":server.com NOTICE * :Test message\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getPrefix() == "server.com" &&
				   cmd.getCommand() == "NOTICE" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "*" &&
				   cmd.getTrailing() == "Test message");
	
	test_result("Command with prefix", passed);
}

void test_command_without_crlf()
{
	std::cout << YELLOW << "\nTest: Command without CRLF" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("NICK alice");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "NICK" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "alice");
	
	test_result("Command without CRLF", passed);
}

void test_empty_message()
{
	std::cout << YELLOW << "\nTest: Empty message" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("");
	
	bool passed = !cmd.isValid();
	
	test_result("Empty message invalid", passed);
}

void test_invalid_command()
{
	std::cout << YELLOW << "\nTest: Invalid command" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("INVALIDCMD test\r\n");
	
	bool passed = !cmd.isValid();
	
	test_result("Invalid command rejected", passed);
}

void test_join_command()
{
	std::cout << YELLOW << "\nTest: JOIN command" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("JOIN #testchannel\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "JOIN" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "#testchannel");
	
	test_result("JOIN command", passed);
}

void test_privmsg_command()
{
	std::cout << YELLOW << "\nTest: PRIVMSG command" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("PRIVMSG #test :Hello, world!\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "PRIVMSG" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "#test" &&
				   cmd.getTrailing() == "Hello, world!");
	
	test_result("PRIVMSG command", passed);
}

void test_is_complete()
{
	std::cout << YELLOW << "\nTest: isComplete() function" << RESET << std::endl;
	
	bool test1 = Parser::isComplete("NICK alice\r\n");
	bool test2 = !Parser::isComplete("NICK alice");
	bool test3 = Parser::isComplete("NICK alice\r\nUSER test\r\n");
	
	bool passed = (test1 && test2 && test3);
	
	test_result("isComplete function", passed);
}

void test_extract_messages()
{
	std::cout << YELLOW << "\nTest: extractMessages() function" << RESET << std::endl;
	
	std::string buffer = "NICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #test";
	std::vector<std::string> messages = Parser::extractMessages(buffer);
	
	bool passed = (messages.size() == 2 &&
				   messages[0] == "NICK alice\r\n" &&
				   messages[1] == "USER alice 0 * :Alice\r\n" &&
				   buffer == "JOIN #test");
	
	test_result("extractMessages function", passed);
}

void test_multiple_params()
{
	std::cout << YELLOW << "\nTest: Command with multiple parameters" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("MODE #test +nt\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "MODE" &&
				   cmd.getParams().size() == 2 &&
				   cmd.getParams()[0] == "#test" &&
				   cmd.getParams()[1] == "+nt");
	
	test_result("Multiple parameters", passed);
}

void test_pass_command()
{
	std::cout << YELLOW << "\nTest: PASS command" << RESET << std::endl;
	
	Command cmd = Parser::parseMessage("PASS secretpassword\r\n");
	
	bool passed = (cmd.isValid() &&
				   cmd.getCommand() == "PASS" &&
				   cmd.getParams().size() == 1 &&
				   cmd.getParams()[0] == "secretpassword");
	
	test_result("PASS command", passed);
}

void test_ping_pong()
{
	std::cout << YELLOW << "\nTest: PING/PONG commands" << RESET << std::endl;
	
	Command cmd1 = Parser::parseMessage("PING :server.com\r\n");
	Command cmd2 = Parser::parseMessage("PONG :server.com\r\n");
	
	bool passed = (cmd1.isValid() &&
				   cmd1.getCommand() == "PING" &&
				   cmd1.getTrailing() == "server.com" &&
				   cmd2.isValid() &&
				   cmd2.getCommand() == "PONG" &&
				   cmd2.getTrailing() == "server.com");
	
	test_result("PING/PONG commands", passed);
}

int main()
{
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << YELLOW << "  Parser Unit Tests" << RESET << std::endl;
	std::cout << YELLOW << "========================================" << RESET << std::endl;

	// Run all tests
	test_simple_command();
	test_command_with_trailing();
	test_command_with_prefix();
	test_command_without_crlf();
	test_empty_message();
	test_invalid_command();
	test_join_command();
	test_privmsg_command();
	test_is_complete();
	test_extract_messages();
	test_multiple_params();
	test_pass_command();
	test_ping_pong();

	// Summary
	std::cout << YELLOW << "\n========================================" << RESET << std::endl;
	std::cout << YELLOW << "  Test Summary" << RESET << std::endl;
	std::cout << YELLOW << "========================================" << RESET << std::endl;
	std::cout << GREEN << "Passed: " << tests_passed << RESET << std::endl;
	std::cout << RED << "Failed: " << tests_failed << RESET << std::endl;
	std::cout << "Total: " << (tests_passed + tests_failed) << std::endl;

	if (tests_failed == 0)
	{
		std::cout << GREEN << "\nAll parser tests passed!" << RESET << std::endl;
		return 0;
	}
	else
	{
		std::cout << RED << "\nSome parser tests failed!" << RESET << std::endl;
		return 1;
	}
}
