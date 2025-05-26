#include "Server.hpp"
#include <iostream>
#include "Common.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        // error print err
        return 1;
    }

    unsigned short port = utils::str_to_value< unsigned short >(argv[1]);
    std::string password = argv[2];

    {
        Server server(port, password);

        server.run();
    }
    std::cout << "HELOOOO2222\n";
}
