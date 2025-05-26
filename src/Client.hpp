#pragma once

#include <string>
#include <vector>

enum TokenKind {
    // 0..255 is kept for ascii characters
    TK_WORD = 256,
    TK_STRING,
    TK_NUMBER,
};

struct Token {
    int kind;
    std::string data;

    Token(int kind, std::string data) : kind(kind), data(data) {}
};

struct Client {
    bool registered; // if the client is fully connected or not

    std::string input;
    std::string response;

    Client() : registered(false) {}

    void process_input();

private:
    typedef std::vector< Token >::const_iterator TokenIterator;

    bool split_input_buffer(std::vector< std::string >& messages);
    bool tokenize_message(const std::string& message, std::vector< Token >& tokens);
};
