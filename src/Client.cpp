#include "Client.hpp"
#include "Common.hpp"

#include <iostream>
#include <unistd.h>

Client::~Client() {
}

bool Client::is_message_complete() {
    if (this->message.find("\r\n") != std::string::npos) {
        this->message = this->message.substr(0, this->message.size() - 2);
        return true;
    }

    if (this->message.find("\r") != std::string::npos || this->message.find("\n") != std::string::npos) {
        this->message = this->message.substr(0, this->message.size() - 1);
        return true;
    }

    return false;
}

void Client::process_message() {
    std::cout << ANSI_CYAN << "[debug][server]: parsing client message: [" << this->message << "]\n" << ANSI_RESET;

    //
    {
        std::vector< Token > tokens;
        if (!tokenize_message(tokens)) {
            std::cout << ANSI_RED << "[error][server]: failed to tokenize client message!\n" << ANSI_RESET;
            return;
        }

        for (TokenIterator itr = tokens.begin(); itr != tokens.end(); itr++) {
            std::cout << "token(" << itr->kind << ", [" << itr->data << "]\n";
        }

        if(!tokens.empty() && tokens[0].kind == TK_WORD)
        {
            std::string cmd = tokens[0].data;

            if (cmd == "CAP")
            {
                if (tokens[2].data == "LS")
                    response =  "CAP * LS :\r\n";
                if (tokens[2].data == "END")
                {
                    // response = ":lsaiti JOIN #ledion\r\n";
                    response = ":ircserver 001 lsaiti :Welcome to the IRC Network lsaiti!lsaiti@lsaiti\r\n";
                }
            }
            else if (cmd == "MODE")
            {
                if (tokens[4].data == "+")
                    response = ":ircserver 221 lsaiti +i\r\n";
            }
            else if (cmd == "PING")
            {
                response = "PONG ircserver\r\n";
            }
            else if (cmd == "JOIN")
            {
                response = ":lsaiti!lsaiti@lsaiti JOIN :#";
                response += tokens[3].data;
                response += "\r\n";
            }
            else
                response.clear();
        }
    }
    // std::cout << ANSI_CYAN << "[debug][server]: senddd client message: [" << msg << "]\n" << ANSI_RESET;

    this->message.clear();
}

bool Client::tokenize_message(std::vector< Token >& tokens) {
    const std::string& str = this->message;

    size_t i = 0;
    while (i < str.size()) {
        char curr = str[i];
        if (curr == ' ') {
            tokens.push_back(Token(TK_SPACE, " "));
            i++;
            continue;
        } else if (utils::is_alpha(curr)) {
            size_t start = i;

            // keep going until we don't find a letter
            while (i < str.size() && utils::is_alpha(str[i])) {
                i++;
            }

            tokens.push_back(Token(TK_WORD, str.substr(start, i - start)));
            continue;
        } else if (curr == '\'' || curr == '"') {
            // skip the first quote
            size_t start = ++i;

            // keep going until we find a matching quote
            while (i < str.size() && str[i] != curr) {
                i++;
            }

            // don't capture the quotes in the token data
            tokens.push_back(Token(TK_STRING, str.substr(start, i - start)));
            i++;
            continue;
        } else if (utils::is_digit(curr)) {
            size_t start = i;

            // keep going until we don't find a number
            while (i < str.size() && utils::is_digit(str[i])) {
                i++;
            }

            tokens.push_back(Token(TK_NUMBER, str.substr(start, i - start)));
            continue;
        } else {
            tokens.push_back(Token(TK_CHAR, str.substr(i, 1)));
            i++;
            continue;
        }
        i++;
    }

    return true;
}
