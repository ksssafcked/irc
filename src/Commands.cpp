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
    _commands["PART"] = Commands::leave_command_handler;
    _commands["PING"] = Commands::ping_command_handler;
}

CommandResult Commands::execute(const std::string& name, Client* client, std::vector< Token >& args) {
    std::map< std::string, CommandHandler >::iterator itr = _commands.find(name);
    if (itr != _commands.end()) {
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
        result.type = CMD_RESULT_REPLY;
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
    if (args.size() != 2) {
    }
    if (client->state < S_USER)
        result.type = CMD_NO_RESULT;
    else if (client->state == S_CONNECT) {
        result.type = CMD_CHANGE_NICK;
        result.reply = args[1].data;
    } else {
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
    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() != 3) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "PRIVMSG");
    } else if (client->state != S_CONNECT) {
        result.type = CMD_NO_RESULT;
    } else {
        result.type = CMD_RESULT_BROADCAST;
        result.reply = args[1].data;
        result.message = args[2].data;
    }
    return result;
}

CommandResult Commands::join_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() < 2) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "JOIN");
    } else if (!utils::is_channel(args[1].data)) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_nosuchchannel(client->nickname, args[1].data);
    } else {
        result.type = CMD_CHANNEL;
        result.reply = args[1].data;
        result.message = client->username;
        if (args.size() > 2)
            result.extra = args[2].data;
    }
    return result;
}

CommandResult Commands::quit_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;
    (void)client;
    if (args.size() != 3)
        result.type = CMD_NO_RESULT;
    else {
        result.type = CMD_QUIT;
        result.reply = args[2].data;
    }
    return result;
}

CommandResult Commands::mode_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() < 3 || args.size() > 4) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "MODE");
    } else {
        result.type = CMD_CHANGE_MODE;
        result.reply = args[1].data;
        result.message = args[2].data;
        if (args.size() == 4)
            result.extra = args[3].data;
    }
    return (result);
}

CommandResult Commands::kick_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    (void)client;
    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() != 4) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "KICK");
    } else {
        result.type = CMD_KICK;
        result.reply = args[1].data;
        result.message = args[2].data;
        result.extra = args[3].data;
    }
    return (result);
}

CommandResult Commands::invite_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() != 3) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "INVITE");
    } else if (!utils::is_channel(args[2].data)) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_nosuchchannel(client->nickname, args[2].data);
    } else {
        result.type = CMD_INVITE;
        result.reply = args[2].data;
        result.message = args[1].data;
    }
    return (result);
}

CommandResult Commands::topic_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() < 2) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "TOPIC");
    } else {
        result.type = CMD_TOPIC;
        result.reply = args[1].data;
        if (args.size() == 3)
            result.message = args[2].data;
    }
    return (result);
}

CommandResult Commands::leave_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() < 2) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "PART");
    } else {
        result.type = CMD_LEAVE;
        result.reply = args[1].data;
    }
    return result;
}

CommandResult Commands::ping_command_handler(Client* client, std::vector< Token >& args) {
    CommandResult result;

    if (client->state != S_CONNECT) {
        result.type = CMD_RESULT_REPLY;
        std::string nick = !client->nickname.empty() ? client->nickname : "*";
        result.reply = error_notregistered(nick);
    } else if (args.size() != 2) {
        result.type = CMD_RESULT_REPLY;
        result.reply = error_needmoreparams(client->nickname, "PING");
    } else {
        result.type = CMD_PING;
        result.reply = args[1].data;
    }
    return result;
}
