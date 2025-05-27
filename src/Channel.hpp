#pragma once

#include <vector>
#include <string>
#include <iostream>

class Channel {
    private:
        Channel();

        std::string _channel_name;
        public:
        std::vector <int> _users;
        Channel(std::string channel_name);

        void add_user(int fd);
        std::string getName();
};