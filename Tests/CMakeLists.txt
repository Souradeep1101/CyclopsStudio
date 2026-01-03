project(CyclopsTests)

# 1. Initialize GoogleTest
# FIX: When including a folder that is "Out-of-Tree" (outside the 'Tests' folder),
# CMake requires a second argument to specify where to store the compiled files.
# We tell it to put them in a folder named "googletest-build" inside your build dir.
add_subdirectory(
    "${CMAKE_SOURCE_DIR}/Engine/vendor/googletest" 
    "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
)

# 2. Define Test Executable
add_executable(CyclopsTests 
    src/main.cpp
    src/Test_BrushMath.cpp
)

# 3. Link Dependencies
# We link GTest and the Engine.
target_link_libraries(CyclopsTests 
    PRIVATE 
    GTest::gtest_main 
    Engine 
)

# 4. Include Directories
target_include_directories(CyclopsTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/Engine/src
    ${CMAKE_SOURCE_DIR}/Engine/vendor/glm
)

# 5. Working Directory Fix
set_target_properties(CyclopsTests PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# 6. Register with CTest
include(GoogleTest)
gtest_discover_tests(CyclopsTests)