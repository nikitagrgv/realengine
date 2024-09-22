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

        // TODO: super shittt
        std::ios_base::fmtflags old_flags(std::cout.flags());
        std::cout << std::hex << "[OpenGL Error] (0x" << error << ")\n";
        std::cout.flags(old_flags);

        std::cout << "in file: " << file << "\n"
                  << "in function: " << function << "\n"
                  << "on line: " << line << std::endl;
        error = glGetError();
    }
    return success;
}
