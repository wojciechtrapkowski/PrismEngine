#include "context/context.hpp"

#include <iostream>
#include <stdexcept>

int main() {
    try {
        Prism::Context::Context context;
        context.RunEngine();
    } catch (const std::exception &e) {
        std::cerr << "Shader compilation error: " << e.what() << std::endl;
    }

    return 0;
}
