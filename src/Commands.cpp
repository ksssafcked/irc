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
    _commands["QUIT"] = Commands::quit_command_handler;
    _commands["KICK"] = Commands::kick_command_handler;
    _commands["INVITE"] = Commands::invite_command_handler;
    _commands["MODE"] = Commands::mode_command_handler;
    _commands["TOPIC"] = Commands::topic_command_handler;
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
    CommandResult result;

    if (args.size() != 2 && args.size() != 3) {
        result.type = CMD_NO_RESULT;
        return result;
    } else if (args[1].data == "LS") {
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
    CommandResult result;

    if (args.size() != 2 || client->state != S_USER)
        result.type = CMD_NO_RESULT;
    else {
        result.type = CMD_CHECK_USER;
        result.reply = args[1].data;
    }
    return result;
}

CommandResult Commands::username_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (args.size() < 3 || client->state != S_USER)
        result.type = CMD_NO_RESULT;
    else {
        result.type = CMD_CHECK_USER;
        result.reply = args[1].data;
        result.message = args[2].data;
    }
    return result;
}

CommandResult Commands::password_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    if (args.size() != 2 || client->state != S_PASS) {
        result.type = CMD_NO_RESULT;
    } else {
        result.type = CMD_CHECK_PASS;
        result.reply = args[1].data;
    }
    return result;
}

CommandResult Commands::message_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    if (args.size() < 4 || client->state != S_CONNECT) {
        result.type = CMD_NO_RESULT;
        return result;
    }
    result.type = CMD_RESULT_BROADCAST;
    result.reply = args[1].data;
    result.message.clear();
    for (size_t i = 3; i < args.size(); i++) {
        result.message += args[i].data;
    }
    return result;
}

CommandResult Commands::join_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (args.size() < 2 || args.size() > 3 || client->state != S_CONNECT || !utils::is_channel(args[1].data)) {
        result.type = CMD_NO_RESULT;
        return result;
    }
    result.type = CMD_CHANNEL;
    result.reply = args[1].data;
    result.message = client->username;
    if (args.size() == 3)
        result.extra = args[2].data;
    std::cout << "[debug][client]: creating channel : [" << args[1].data << "]\n";

    return result;
}

CommandResult Commands::quit_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (args.size() != 3) {
        result.type = CMD_NO_RESULT;
        return result;
    }
    result.type = CMD_QUIT;
    result.reply = args[2].data;
    if (client->username.empty())
        std::cout << "[debug][client]: user leaved: [NO_USERNAME]\n";
    else
        std::cout << "[debug][client]: user leaved: [" << client->username << "]\n";

    return result;
}

CommandResult Commands::mode_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    (void)client;
    if (args.size() < 3 || args.size() > 4) {
        result.type = CMD_RESULT_NOT_FOUND;
    } else {
        result.type = CMD_CHANGE_MODE;
        result.reply = args[1].data;
        result.message = args[2].data;
    }
    if (args.size() == 4) {
        result.extra == args[3].data;
        std::cout << "[debug][client]: mode extra: [" << result.extra << "]\n";
    }
    return (result);
}

CommandResult Commands::kick_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    (void)client;
    if (args.size() != 4) {
        result.type = CMD_NO_RESULT;
    } else {
        result.type = CMD_KICK;
        result.reply = args[1].data;
        result.message = args[2].data;
        result.extra = args[3].data;
    }
    return (result);
}

CommandResult Commands::invite_command_handler(Client* client, std::vector< Token >& args) {
    (void)client;
    (void)args;
    CommandResult result;
    if (args.size() != 3 || client->state != S_CONNECT || !utils::is_channel(args[2].data))
        result.type = CMD_NO_RESULT;
    else {
        result.type = CMD_INVITE;
        result.reply = args[2].data;
        result.message = args[1].data;
    }
    return (result);
}

CommandResult Commands::topic_command_handler(Client* client, std::vector< Token >& args) {
    (void)client;
    (void)args;
    return (CommandResult());
}
