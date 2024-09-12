#include "Base.h"

// clang-format off
#include "glad/glad.h"
// clang-format on

#include <iostream>

void clearGLErrors()
{
    while (glGetError())
        ;
}

bool checkGLErrors(const char *function, const char *file, int line)
{
    bool success = true;
    while (unsigned int error = glGetError())
    {
        success = false;
        std::cout << "[OpenGL Error] (" << error << ")\n"
                  << "in file: " << file << "\n"
                  << "in function: " << function << "\n"
                  << "on line: " << line << "\n";
        error = glGetError();
    }
    return success;
}
