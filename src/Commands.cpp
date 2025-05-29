#include "Commands.hpp"

#include <iostream>

std::map< std::string, Commands::CommandHandler > Commands::_commands;

void Commands::register_commands() {
    _commands["CAP"] = Commands::cap_command_handler;
    _commands["NICK"] = Commands::nick_command_handler;
    _commands["USER"] = Commands::username_command_handler;
    _commands["PASS"] = Commands::password_command_handler;
    _commands["JOIN"] = Commands::join_command_handler;
    _commands["PRIVMSG"] = Commands::message_command_handler;
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
    if (args[1].data == "LS") {
        result.type = CMD_RESULT_REPLY; // TODO: change this
        result.reply = "CAP * LS :\r\n";
    } else if (args[1].data == "END") {
        result.type = CMD_NO_RESULT;
    } else {
        result.type = CMD_RESULT_NOT_FOUND;
    }
    return result;
}

CommandResult Commands::nick_command_handler(Client* client, std::vector< Token >& args) {
    (void)args;

    CommandResult result;
    client->nickname = args[1].data;
    result.type = CMD_NO_RESULT;
    result.reply = "[debug][client]: client connected with nickname: [" + client->nickname + "]\n";
    return result;
}


CommandResult Commands::username_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    client->username = args[1].data;
    client->hostname = args[2].data;
    result.type = CMD_NO_RESULT;
    result.reply = "[debug][client]: client connected with username: [" + client->username + "]\n";

    return result;
}

CommandResult Commands::password_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    client->password = args[1].data;
    result.type = CMD_CHECK_USER;                
    return result;
}

CommandResult Commands::message_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    if (args.size() < 4 || !client->registered) {
        result.type = CMD_NO_RESULT;
        return result;
    }
    result.type = CMD_RESULT_BROADCAST;
    result.reply = args[1].data;
    result.message.clear();
    for (size_t i = 3; i < args.size(); i++)
    {
        result.message += args[i].data;
    }
    return result;
}

CommandResult Commands::join_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (args.size() != 2 || !utils::is_channel(args[1].data)) {
        result.type = CMD_NO_RESULT;
        return result;
    }
    result.type = CMD_CHANNEL;
    result.reply = args[1].data;
    result.message = client->username;
    std::cout << "[debug][client]: creating channel : [" << args[1].data << "]\n";

    return result;
}