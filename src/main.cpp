#include "Server.hpp"

#include "Common.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        // error print err
        return 1;
    }
    std::string port_string(argv[1]);
    if (port_string.size() > 5)
        return 1;
    for (size_t i = 0; i < port_string.size(); i++) {
        if (!utils::is_digit(port_string[i]))
            return 1;
    }
    unsigned short port = utils::str_to_value< unsigned short >(argv[1]);
    std::string password = argv[2];

    Server server(port, password);

    server.run();
}
