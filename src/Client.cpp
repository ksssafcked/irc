#include "Client.hpp"
#include "Common.hpp"
#include "Commands.hpp"

#include <iostream>

bool Client::process_input(CommandResult& result) {
    std::string message;
    if (!split_input_buffer(message)) {
        return false;
    }

    std::vector< Token > tokens;
    if (!tokenize_message(message, tokens)) {
        std::cout << ANSI_RED << "[error][server]: failed to tokenize client message!\n" << ANSI_RESET;
        return false;
    }

    for (TokenIterator itr = tokens.begin(); itr != tokens.end(); itr++) {
        std::cout << "token(" << itr->kind << ", [" << itr->data << "]\n";
    }

    if (!tokens.size()) {
        input.clear();
        return false;
    }

    result = Commands::execute(tokens.begin()->data, this, tokens);
    if (result.type > CMD_NO_RESULT) {
        return true;
    } else {
        if (result.type == CMD_RESULT_NOT_FOUND) {
            std::cout << ANSI_CYAN << "[debug][client]: command '" << tokens.begin()->data << "' not found\n"
                      << ANSI_RESET;
        } else if (result.type == CMD_RESULT_FAILED) {
            std::cout << ANSI_CYAN << "[debug][client]: command '" << tokens.begin()->data << "' failed\n"
                      << ANSI_RESET;
        } else if (result.type == CMD_NO_RESULT) {
            std::cout << ANSI_CYAN << result.reply << ANSI_RESET;
        }
        return false;
    }
}

bool Client::split_input_buffer(std::string& message) {
    size_t pos = input.find("\r\n");

    if (pos == std::string::npos)
        return false;

    message = input.substr(0, pos);
    input.erase(0, pos + 2);
    return true;
}

Token Client::add_full_message(const std::string& message, size_t& i) {
    if (utils::is_space(message[i]))
        i++;
    size_t j = i;
    while (j < message.size() && message[j] != '\r' && message[j] != '\n' && message[j] != ':')
        j++;
    Token t(TK_STRING, message.substr(i, j - i));
    i = j;
    return t;
}

bool Client::tokenize_message(const std::string& message, std::vector< Token >& tokens) {
    if (message.empty()) {
        return false;
    }

    tokens.clear();
    std::string current_token;
    std::string::const_iterator it = message.begin();

    if (*it == ':') {
        it++;
        while (it != message.end() && *it != ' ') {
            current_token += *it;
            it++;
        }
        tokens.push_back(Token(TK_WORD, current_token));
        current_token.clear();

        while (it != message.end() && *it == ' ') {
            it++;
        }
    }

    while (it != message.end()) {
        if (*it == ':') {
            it++;
            tokens.push_back(Token(TK_STRING, std::string(it, message.end())));
            return true;
        }

        if (*it == '#') {
            while (it != message.end() && *it != ' ') {
                current_token += *it;
                it++;
                
            }
            if (!current_token.empty()) {
                tokens.push_back(Token(TK_WORD, current_token));
                current_token.clear();
            }
            continue;
        }
        if (*it == ' ') {
            if (!current_token.empty()) {
                tokens.push_back(Token(TK_WORD, current_token));
                current_token.clear();
            }
            while (it != message.end() && *it == ' ') {
                it++;
            }
        } else {
            current_token += *it;
            it++;
        }
    }

    if (!current_token.empty()) {
        tokens.push_back(Token(TK_WORD, current_token));
    }

    return !tokens.empty();
}
