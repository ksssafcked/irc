#include "Commands.hpp"

#include <iostream>

std::map< std::string, Commands::CommandHandler > Commands::_commands;

void Commands::register_commands() {
    _commands["CAP"] = Commands::cap_command_handler;
    // _commands["PASS"] = Commands::cap_command_handler;
}

CommandResult Commands::execute(const std::string& name, Client* client, std::vector< Token >& args) {
    std::map< std::string, CommandHandler >::iterator itr = _commands.find(name);
    if (itr != _commands.end()) {
        std::cout << "executing " << name << "!\n";
        return itr->second(client, args);
    }

    CommandResult result;
    result.type = CMD_RESULT_NOT_FOUND;

    return result;
}

CommandResult Commands::cap_command_handler(Client* client, std::vector< Token >& args) {
    (void)client;
    (void)args;

    CommandResult result;
    result.type = CMD_RESULT_FAILED; // TODO: change this

    return result;
}
