project(App)

# --- 1. EXECUTABLE ---
# Define the executable using the source files
# [CHANGE] We added EditorLayer sources here because they now live in the App
add_executable(App 
    src/main.cpp
    src/EditorLayer.cpp
    src/EditorLayer.h
)

# --- 2. LINKING ---
# We only need to link 'Engine'. 
# Because Engine links GLM, GLFW, etc. as 'PUBLIC', App gets them automatically.
target_link_libraries(App PRIVATE Engine)

# --- 3. WORKSPACE CONFIGURATION (VS Only) ---
# This sets the "Working Directory" in Visual Studio debugger to the project root.
# This allows "Engine/assets/..." paths to work while debugging.
set_target_properties(App PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# --- 4. ASSET MANAGEMENT (Standalone Builds) ---
# This command runs AUTOMATICALLY after the build finishes.
# It copies the "Engine/assets" folder into the "bin/Engine/assets" folder.
# This ensures that if you run the .exe directly from the folder (outside Visual Studio),
# it can still find the shaders.
add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Engine/assets"
    "$<TARGET_FILE_DIR:App>/Engine/assets"
    COMMENT "Copying Engine Assets to Output Directory..."
)