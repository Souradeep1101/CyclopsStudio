#pragma once
#include <glad/glad.h>
#include <iostream>
#include "Core/PlatformDetection.h"

// ===================================================================================
//                               CYCLOPS DEBUGGING TOOLKIT
// ===================================================================================

/**
 * @brief  Initializes the OpenGL Debug Output system (OpenGL 4.3+).
 * * @details
 * - **Windows/Linux**: Registers a callback with the GPU driver. Any OpenGL error
 * (or warning) will trigger a breakpoint and print a message to the console automatically.
 * - **macOS**: This function is a no-op (empty) because Mac only supports OpenGL 4.1.
 * Debugging on Mac relies entirely on the GLCall() macro wrapper instead.
 * * @note Must be called AFTER `gladLoadGLLoader`.
 * @note In **Release Mode** (NDEBUG), this function does nothing.
 */
void EnableGLDebugging();

// --- RELEASE MODE (Production) ---
#if defined(NDEBUG) || defined(RELEASE)

    /**
     * @brief Wraps an OpenGL function call.
     * @note  IN RELEASE MODE: This macro is stripped. 'x' is called directly with 0 overhead.
     */
    #define GLCall(x) x

         // Optimized away in release
    inline void EnableGLDebugging() {}

// --- DEBUG MODE (Development) ---
#else

    // Platform-specific Breakpoints
    #if defined(_MSC_VER)
        #define DEBUG_BREAK() __debugbreak()
    #elif defined(__GNUC__) || defined(__clang__)
        #define DEBUG_BREAK() __builtin_trap()
    #else
        #define DEBUG_BREAK() std::abort()
    #endif

    /**
     * @brief   OpenGL Error Check Wrapper.
     * * @details
     * Wraps an OpenGL function call to check for errors immediately after execution.
     * usage: GLCall( glDrawElements(...) );
     * * **Platform Behavior:**
     * - **Windows/Linux**: Passes 'x' through directly. Errors are caught globally by
     * the `EnableGLDebugging()` callback in the background.
     * - **macOS**: Executes 'x', then loops through `glGetError()` to check for failures.
     * If an error is found, it prints the file/line and triggers a breakpoint.
     * * @param x The OpenGL function to call.
     */
    #if defined(CYCLOPS_PLATFORM_MAC)
         // Legacy Loop for Mac
        inline void GLClearError() { while (glGetError() != GL_NO_ERROR); }
        inline bool GLLogCall(const char* function, const char* file, int line) {
            while (GLenum error = glGetError()) {
                std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
                return false;
            }
            return true;
        }
        #define GLCall(x) GLClearError(); x; if (!GLLogCall(#x, __FILE__, __LINE__)) DEBUG_BREAK();
    #else
         // Modern Pass-through for Windows/Linux
        #define GLCall(x) x
    #endif

#endif