#pragma once

#include <vector>
#include <string>
#include <iostream>

struct Channel {
    
    std::string channel_name;
    std::vector <int> users;

    Channel(std::string name, int fd);

    void add_user(int fd);
    void delete_user(int fd);
    std::string getName();

    private:
        Channel();
};