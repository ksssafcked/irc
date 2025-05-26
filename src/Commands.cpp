#include "Commands.hpp"

#include <iostream>

std::map< std::string, Commands::CommandHandler > Commands::_commands;

void Commands::register_commands() {
    _commands["CAP"] = Commands::cap_command_handler;
    // _commands["PASS"] = Commands::cap_command_handler;
}

bool Commands::execute(const std::string& name, Client* client, std::vector< Token >& args) {
    Iterator itr = _commands.find(name);
    if (itr != _commands.end()) {
        return itr->second(client, args);
    }

    return false;
}

bool Commands::cap_command_handler(Client* client, std::vector< Token >& args) {
    (void)client;
    (void)args;
    std::cout << "executing " << args[0].data << "!\n";
    return true;
}
