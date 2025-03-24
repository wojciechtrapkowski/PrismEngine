#include "context/public/context.hpp"

#include <stdexcept>
#include <iostream>

int main()
{
    try {
        Prism::Context::Context context;
        context.RunEngine();
    } catch (const std::exception& e) {
        std::cerr << "Shader compilation error: " << e.what() << std::endl;
    }
    
    return 0;
}

