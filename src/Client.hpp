#pragma once

#include <string>
#include <vector>

enum TokenKind {
    TK_WORD,
    TK_STRING,
    TK_NUMBER,
    TK_SPACE,
    TK_CHAR, // anything that is not one of the above
};

struct Token {
    int kind;
    std::string data;

    Token(int kind, std::string data) : kind(kind), data(data) {}
};

struct Client {
    bool registered; // if the client is fully connected or not

    std::string message;
    std::string response;

    Client() : registered(false) {}

    bool is_message_complete();
    void process_message();

private:
    typedef std::vector< Token >::const_iterator TokenIterator;
    bool tokenize_message(std::vector< Token >& tokens);
};
