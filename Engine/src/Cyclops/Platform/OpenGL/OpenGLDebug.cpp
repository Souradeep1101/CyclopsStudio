#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"

#if !defined(NDEBUG) && !defined(RELEASE) // Only compile this code in Debug mode

namespace Cyclops {
    /**
     * @brief Implementation of the Modern OpenGL Debug Callback.
     * * Defines the lambda function that the GPU driver calls when it detects an error.
     * Filters out insignificant warnings (like NVidia buffer info) and crashes on real errors.
     */
    void EnableGLDebugging()
    {
        // macOS does not support GL_DEBUG_OUTPUT (Requires OpenGL 4.3+)
        #if !defined(__APPLE__) 
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            glDebugMessageCallback(
                [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
                {
                    // Ignore non-significant error codes (Nvidia performance warnings, etc.)
                    // 131169 - Framebuffer detailed info
                    // 131185 - Buffer memory usage info
                    // 131218 - Shader recompilation performance warning
                    // 131204 - Texture base level inconsistency
                    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

                    std::cout << "---------------------" << std::endl;
                    std::cout << "[OpenGL Debug Message] (" << id << "): " << message << std::endl;

                    // Severity: High = Crash immediately
                    // Type: Error = Crash immediately
                    if (type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH) {
                        std::cout << "(Critical Error) Breaking into debugger..." << std::endl;
                        DEBUG_BREAK();
                    }
                }, 
                nullptr
            );
        #endif
    }
}

#endif