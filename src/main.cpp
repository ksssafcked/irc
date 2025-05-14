#include "Server.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        // error print err
        return 1;
    }

    std::string port = argv[1];
    std::string password = argv[2];

    Server server(port, password);
    server.run();
}
