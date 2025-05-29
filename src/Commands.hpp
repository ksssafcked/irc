#pragma once

#include "Client.hpp"
#include "Common.hpp"

#include <map>
#include <string>

enum CommandResultType {
    CMD_RESULT_NOT_FOUND,
    CMD_RESULT_FAILED,
    CMD_QUIT,
    CMD_CHANNEL,                    // create or join channel
    CMD_CHECK_USER,                 // server needs to check password
    CMD_NO_RESULT,                  // no reply needed
    CMD_RESULT_REPLY,               // reply only to client who the sent the command
    CMD_RESULT_REPLY_AND_BROADCAST, // reply to client who sent the command and broadcast message to other clients
    CMD_RESULT_BROADCAST,           // broadcast message to other clients
};

struct CommandResult {
    int type;
    std::string reply;   // used to reply to client
    std::string message; // used to broadcast to clients
};

class Commands {
private:
    typedef CommandResult (*CommandHandler)(Client* client, std::vector< Token >& args);
    static std::map< std::string, CommandHandler > _commands;

public:
    static void register_commands();
    static CommandResult execute(const std::string& name, Client* client, std::vector< Token >& args);

private:
    static CommandResult cap_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult nick_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult username_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult password_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult message_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult join_command_handler(Client* client, std::vector< Token >& args);
    static CommandResult quit_command_handler(Client* client, std::vector< Token >&args);
};
