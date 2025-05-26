#include "Client.hpp"
#include "Common.hpp"
#include "Commands.hpp"

#include <iostream>

void Client::process_input() {
    std::cout << ANSI_CYAN << "[debug][server]: parsing client input: [" << this->input << "]\n" << ANSI_RESET;

    // handle multi-line messages (IRC v3)
    std::vector< std::string > messages;
    if (!split_input_buffer(messages)) {
        return;
    }

    input.clear();

    for (size_t i = 0; i < messages.size(); i++) {
        const std::string& message = messages[i];
        std::cout << ANSI_CYAN << "[debug][server]: processing message: [" << message << "]\n" << ANSI_RESET;

        std::vector< Token > tokens;
        if (!tokenize_message(message, tokens)) {
            std::cout << ANSI_RED << "[error][server]: failed to tokenize client message!\n" << ANSI_RESET;
            return;
        }

        for (TokenIterator itr = tokens.begin(); itr != tokens.end(); itr++) {
            std::cout << "token(" << itr->kind << ", [" << itr->data << "]\n";
        }

        CommandResult result = Commands::execute(tokens.begin()->data, this, tokens);
        if (result.type == CMD_RESULT_NOT_FOUND) {
            std::cout << "[debug][client]: command '" << tokens.begin()->data << "' not found\n";
        } else if (result.type == CMD_RESULT_FAILED) {
            std::cout << "[debug][client]: command '" << tokens.begin()->data << "' failed\n";
        }

        // TODO: think of way to use result
    }
}

bool Client::split_input_buffer(std::vector< std::string >& messages) {
    // we should not try processing commands if they contain a delimiter
    if (input.find('\r') == std::string::npos && input.find('\n') == std::string::npos) {
        return false;
    }

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] != '\r' && input[i] != '\n') {

            size_t j = i;
            bool had_printable_chars = false;

            while (j < input.size() && (input[j] != '\r' && input[j] != '\n')) {
                if (input[j] > 32 && input[j] < 127) {
                    had_printable_chars = true;
                }
                j++;
            }

            if (j > i && had_printable_chars) {
                std::string message = input.substr(i, j - i);
                // std::cout << ANSI_PINK << "split [" << message << "]\n" << ANSI_RESET;
                messages.push_back(message);
            }

            i = j;
            continue;
        }
        i++;
    }

    return true;
}

bool Client::tokenize_message(const std::string& message, std::vector< Token >& tokens) {
    size_t i = 0;
    while (i < message.size()) {
        char curr = message[i];
        if (utils::is_space(curr)) {
            i++;
            continue;
        } else if (utils::is_alpha(curr)) {
            size_t start = i;

            // keep going until we don't find a letter
            while (i < message.size() && utils::is_alpha(message[i])) {
                i++;
            }

            tokens.push_back(Token(TK_WORD, message.substr(start, i - start)));
            continue;
        } else if (curr == '\'' || curr == '"') {
            // skip the first quote
            size_t start = ++i;

            // keep going until we find a matching quote
            while (i < message.size() && message[i] != curr) {
                i++;
            }

            // don't capture the quotes in the token data
            tokens.push_back(Token(TK_STRING, message.substr(start, i - start)));
            i++;
            continue;
        } else if (utils::is_digit(curr)) {
            size_t start = i;

            // keep going until we don't find a number
            while (i < message.size() && utils::is_digit(message[i])) {
                i++;
            }

            tokens.push_back(Token(TK_NUMBER, message.substr(start, i - start)));
            continue;
        } else {
            tokens.push_back(Token(curr, message.substr(i, 1)));
            i++;
            continue;
        }
        i++;
    }

    return true;
}
