cmake_minimum_required(VERSION 3.20.1)
project(CyclopsStudio VERSION 1.0.0)

# Set C++ Standard to 23 (Modern C++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# output directories (Where the .exe goes)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add Subdirectories
# Ensure you have created the folders "Engine" and "App" with their own CMakeLists.txt
add_subdirectory(Engine)
add_subdirectory(App)