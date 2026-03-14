cmake_minimum_required(VERSION 3.20.1)
project(CyclopsStudio VERSION 1.0.0)

# --- GLOBAL SETTINGS ---

# 1. Enforce Modern C++ (C++20 or C++23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Forces standard C++, disables compiler-specific extensions (e.g. MSVC specific)

# 2. Output Directories
# This forces all binaries to go to /bin and /lib inside your build folder.
# It makes finding your .exe much easier.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- SUBPROJECTS ---
# Order matters: Dependencies first, then Libraries (Engine), then Executables (App)
add_subdirectory(Engine)
add_subdirectory(App)

# Testing
# enable_testing()
# add_subdirectory(Tests)