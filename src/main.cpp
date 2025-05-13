#include <iostream>
#include <vector>

template< typename T >
struct Burek {
    T value;
};

int main() {
    Burek< std::vector< int > > var;
    std::cout << "woohoo burek\n";
    return 0;
}
