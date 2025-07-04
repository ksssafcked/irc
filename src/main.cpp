#include "Server.hpp"

#include "Common.hpp"

static bool check_port(std::string port_string, long long& port) {
    if (port_string.size() > 5) {
        return false;
    }

    for (size_t i = 0; i < port_string.size(); i++) {
        if (!utils::is_digit(port_string[i]))
            return false;
    }

    long long tmp_port = utils::str_to_value< long long >(port_string);
    if (tmp_port < 0 || tmp_port > 65535) {
        return false;
    }

    port = tmp_port;

    return true;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }
    
    long long port = 0;
    if (!check_port(argv[1], port)) {
        std::cerr << "error: invalid port: " << argv[1] << "\n";
        return 1;
    }

    Server server(static_cast<unsigned short>(port), argv[2]);

    server.run();
}
