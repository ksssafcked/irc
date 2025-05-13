#include "Server.hpp"

#include <cstdlib>

int main(int argc, char** argv) {
    if (argc != 3) {
        // error print err
        return 1;
    }

    unsigned short port = std::atoi(argv[1]);
    std::string password = argv[2];

    Server server(port, password);
    server.run();

    return 0;
}
