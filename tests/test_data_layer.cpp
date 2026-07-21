#include "BufferManager.hpp"
#include "Parser.hpp"
#include <iostream>
#include <cassert>

static void test_buffer_manager()
{
    std::cout << "--- Testing BufferManager ---" << std::endl;
    BufferManager bm;

    fd_t client1 = 4;
    bm.add_client(client1);

    // Test 1: Chunked input (fragmented data over TCP)
    bm.append_data(client1, "NICK ");
    std::vector<std::string> lines1 = bm.extract_complete_lines(client1);
    assert(lines1.empty()); // No CRLF yet

    bm.append_data(client1, "souhail\r\nUSER ");
    std::vector<std::string> lines2 = bm.extract_complete_lines(client1);
    assert(lines2.size() == 1);
    assert(lines2[0] == "NICK souhail");

    bm.append_data(client1, "s 0 * :Souhail Name\r\n");
    std::vector<std::string> lines3 = bm.extract_complete_lines(client1);
    assert(lines3.size() == 1);
    assert(lines3[0] == "USER s 0 * :Souhail Name");

    // Test 2: Multiple lines in single TCP chunk
    bm.append_data(client1, "PING 12345\r\nPONG 12345\r\n");
    std::vector<std::string> lines4 = bm.extract_complete_lines(client1);
    assert(lines4.size() == 2);
    assert(lines4[0] == "PING 12345");
    assert(lines4[1] == "PONG 12345");

    std::cout << "✅ BufferManager tests passed!" << std::endl;
}

static void test_parser()
{
    std::cout << "--- Testing Parser ---" << std::endl;
    Parser parser;

    // Test 1: Standard NICK command
    ParsedCommand cmd1 = parser.parse_line("NICK souhail");
    assert(cmd1.is_valid);
    assert(cmd1.command == "NICK");
    assert(cmd1.params.size() == 1);
    assert(cmd1.params[0] == "souhail");
    assert(!cmd1.has_prefix);
    assert(!cmd1.has_trailing);

    // Test 2: PRIVMSG with trailing spaces
    ParsedCommand cmd2 = parser.parse_line("privmsg #general :Hello team, welcome to ft_irc!");
    assert(cmd2.is_valid);
    assert(cmd2.command == "PRIVMSG");
    assert(cmd2.params.size() == 2);
    assert(cmd2.params[0] == "#general");
    assert(cmd2.params[1] == "Hello team, welcome to ft_irc!");
    assert(cmd2.has_trailing);
    assert(cmd2.trailing == "Hello team, welcome to ft_irc!");

    // Test 3: Command with prefix
    ParsedCommand cmd3 = parser.parse_line(":server.net 001 nickname :Welcome to IRC");
    assert(cmd3.is_valid);
    assert(cmd3.has_prefix);
    assert(cmd3.prefix == "server.net");
    assert(cmd3.command == "001");
    assert(cmd3.params.size() == 2);
    assert(cmd3.params[0] == "nickname");
    assert(cmd3.params[1] == "Welcome to IRC");

    // Test 4: Empty line
    ParsedCommand cmd4 = parser.parse_line("   ");
    assert(!cmd4.is_valid);

    std::cout << "✅ Parser tests passed!" << std::endl;
}

int main()
{
    test_buffer_manager();
    test_parser();
    std::cout << "\n🎉 ALL DATA LAYER TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}
