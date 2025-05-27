#include "Client.hpp"
#include "Common.hpp"
#include "Commands.hpp"

#include <iostream>

bool Client::is_sending(CommandResult result){
    if (result.type == CRD_RESULT_REPLY || result.type == CRD_RESULT_BROADCAST || result.type == CRD_RESULT_REPLY_AND_BROADCAST || result.type == CMD_CHECK_USER || result.type == CMD_CHANNEL){
        return true;
    }
    return false;
}

bool Client::process_input(CommandResult& result) {
    std::cout << ANSI_CYAN << "[debug][server]: parsing client input: [" << this->input << "]\n" << ANSI_RESET;

    // handle multi-line messages (IRC v3)
    std::string message;
    if (!split_input_buffer(message)) {
        return false;
    }

    // input.clear();

        std::cout << ANSI_CYAN << "[debug][server]: processing message: [" << message << "]\n" << ANSI_RESET;

        std::vector< Token > tokens;
        if (!tokenize_message(message, tokens)) {
            std::cout << ANSI_RED << "[error][server]: failed to tokenize client message!\n" << ANSI_RESET;
            return false;
        }

        for (TokenIterator itr = tokens.begin(); itr != tokens.end(); itr++) {
            std::cout << "token(" << itr->kind << ", [" << itr->data << "]\n";
        }

        result = Commands::execute(tokens.begin()->data, this, tokens);
        if (is_sending(result))
            return true;
        else
        {
             if (result.type == CMD_RESULT_NOT_FOUND) {
                std::cout << ANSI_CYAN << "[debug][client]: command '" << tokens.begin()->data << "' not found\n" << ANSI_RESET;
            } else if (result.type == CMD_RESULT_FAILED) {
                std::cout << ANSI_CYAN << "[debug][client]: command '" << tokens.begin()->data << "' failed\n" << ANSI_RESET;
            } else if (result.type == CMD_NO_RESULT){
                std::cout << ANSI_CYAN << result.reply << ANSI_RESET;
            }
            return false;
        }
        // if (result.type == CMD_RESULT_NOT_FOUND) {
        //     std::cout << "[debug][client]: command '" << tokens.begin()->data << "' not found\n";
        // } else if (result.type == CMD_RESULT_FAILED) {
        //     std::cout << "[debug][client]: command '" << tokens.begin()->data << "' failed\n";
        // }

        // TODO: think of way to use result
}

void Client::skip_delimiter(std::string message, size_t& i) {
    while (i < message.size())
    {
        if (message[i] == '\r')
            i++;
        else if (message[i] == '\n')
            i++;
        else
            return;
    }
}

bool Client::split_input_buffer(std::string& message) {
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
                message = input.substr(i, j - i);
                skip_delimiter(input, j);
                input = input.substr(j, input.size() - j);
                // std::cout << ANSI_PINK << "input [" << input << "]\n" << ANSI_RESET;
                std::cout << ANSI_PINK << "split [" << message << "]\n" << ANSI_RESET;
                return true;
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
