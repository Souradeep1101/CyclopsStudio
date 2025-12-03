project(App)

# 1. Define Source Files
add_executable(App src/main.cpp)

# 2. Link the Engine
target_link_libraries(App PRIVATE Engine)