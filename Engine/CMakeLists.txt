project(Engine)

# --- 1. DEPENDENCIES ---
# We manage all 3rd party libs here so the App doesn't have to care about them.

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# GLM
add_subdirectory(vendor/glm)

# ImGui (Manual Source Build)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")

# CRITICAL FIX: We split this into two parts.
# 1. Core ImGui Files: Use GLOB (not RECURSE) to only get .cpp files in the root folder.
#    Recursion caused the build to pick up examples/ and backends/ for libs we don't have.
file(GLOB IMGUI_CORE_SOURCES CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")

# 2. Specific Backends: We manually select only GLFW and OpenGL3.
set(IMGUI_BACKEND_SOURCES
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# Combine them
set(IMGUI_SOURCES ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES})

# GLAD
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad")
set(GLAD_SOURCES "${GLAD_DIR}/src/glad.c")

# --- 2. ENGINE SOURCES ---
# CONFIGURE_DEPENDS fixes the "Link Error" issue when adding new files
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h")

# --- 3. DEFINE LIBRARY ---
# We use STATIC so it compiles into a .lib file
add_library(Engine STATIC 
    ${ENGINE_SOURCES} 
    ${ENGINE_HEADERS} 
    ${IMGUI_SOURCES} 
    ${GLAD_SOURCES}
)

# --- 4. INCLUDE DIRECTORIES ---
# 'PUBLIC' means: "I use these headers, AND anyone who links me needs them too."
target_include_directories(Engine PUBLIC 
    src
    vendor/glfw/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLAD_DIR}/include
    # Note: GLM handles its own includes via target_link_libraries
)

# --- 5. LINKING ---
# We link libraries that the Engine needs to function.
target_link_libraries(Engine PUBLIC 
    glfw        # Windowing
    glm::glm    # Math
)

if(WIN32)
    # Windows system libraries required by GLFW/OpenGL
    target_link_libraries(Engine PUBLIC opengl32)
endif()