#pragma once

#include <string>
#include <vector>

enum TokenKind {
    // 0..255 is kept for ascii characters
    TK_WORD = 256,
    TK_STRING,
    TK_NUMBER,
};

struct CommandResult;

struct Token {
    int kind;
    std::string data;

    Token(int kind, std::string data) : kind(kind), data(data) {}
};

struct Client {
    bool registered; // if the client is fully connected or not
    bool connected;

    std::string input;
    std::string nickname;
    std::string username;
    std::string response;
    std::string password;
    std::string hostname;


    Client() : registered(false), connected(false) {}

    bool process_input(CommandResult& result);

private:
    typedef std::vector< Token >::const_iterator TokenIterator;

    bool is_sending(CommandResult result);

    Token add_full_message(const std::string& message, size_t& i);


    bool split_input_buffer(std::string& message);
    void skip_delimiter(std::string message, size_t& i);
    bool tokenize_message(const std::string& message, std::vector< Token >& tokens);
};
