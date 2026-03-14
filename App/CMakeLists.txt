project(App)

# --- 1. EXECUTABLE ---
add_executable(App 
    src/main.cpp
    src/EditorLayer.cpp
    src/EditorLayer.h
    src/Panels/LayerPanel.cpp
    src/Panels/LayerPanel.h
    src/Panels/TimelinePanel.cpp
    src/Panels/TimelinePanel.h
    src/Panels/ViewportPanel.cpp
    src/Panels/ViewportPanel.h
    # [CRITICAL] Compile GLAD in the App too so it can find OpenGL functions
    "${CMAKE_SOURCE_DIR}/Engine/vendor/glad/src/glad.c"
)

# --- 2. COMPILE DEFINITIONS ---
# [CRITICAL] Tells ImGui that the symbols are coming from an external DLL
target_compile_definitions(App PRIVATE 
    "IMGUI_API=__declspec(dllimport)"
)

# --- 3. INCLUDES ---
target_include_directories(App PRIVATE 
    # [FIX] This line solves your "Panels/LayerPanel.h" include error!
    # It adds 'App/src' to the search path.
    "${CMAKE_CURRENT_SOURCE_DIR}/src"

    # Engine Headers
    "${CMAKE_SOURCE_DIR}/Engine/src"
    
    # Vendor Headers (GLAD is strictly private to App, others might inherit but safer to list)
    "${CMAKE_SOURCE_DIR}/Engine/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/imgui"
    "${CMAKE_SOURCE_DIR}/vendor/spdlog/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
)

# --- 4. LINKING ---
# We only need to link 'Engine'. 
# Because Engine links GLM, GLFW, etc. as 'PUBLIC', App gets them automatically.
target_link_libraries(App PRIVATE Engine)

# --- 5. WORKSPACE CONFIGURATION (VS Only) ---
set_target_properties(App PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:App>"
)

# --- 6. ASSET MANAGEMENT ---
add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Engine/assets"
    "$<TARGET_FILE_DIR:App>/assets" 
    COMMENT "Copying Assets to Binaries Directory..."
)