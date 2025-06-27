#pragma once

#include <string>
#include <vector>

enum TokenKind {
    // 0..255 is kept for ascii characters
    TK_WORD = 256,
    TK_STRING,
    TK_NUMBER,
};

enum ConnectionState {
    S_PASS,
    S_USER,
    S_CONNECT,
};

struct CommandResult;

struct Token {
    int kind;
    std::string data;

    Token(int kind, std::string data) : kind(kind), data(data) {}
};

struct Client {
    int state;

    std::string input;
    std::string nickname;
    std::string username;
    std::string response;
    std::string password;
    std::string hostname;

    Client() : state(S_PASS) {}

    bool process_input(CommandResult& result);

private:
    typedef std::vector< Token >::const_iterator TokenIterator;

    Token add_full_message(const std::string& message, size_t& i);

    bool split_input_buffer(std::string& message);
    void skip_delimiter(std::string message, size_t& i);
    bool tokenize_message(const std::string& message, std::vector< Token >& tokens);
};
