#pragma once

#include "Client.hpp"

#include <map>
#include <string>

class Commands {
private:
    typedef bool (*CommandHandler)(Client* client, std::vector< Token >& args);
    static std::map< std::string, CommandHandler > _commands;

public:
    typedef std::map< std::string, CommandHandler >::iterator Iterator;

    static void register_commands();
    static bool execute(const std::string& name, Client* client, std::vector< Token >& args);

private:
    static bool cap_command_handler(Client* client, std::vector< Token >& args);
};
