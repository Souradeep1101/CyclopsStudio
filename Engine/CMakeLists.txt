cmake_minimum_required(VERSION 3.12)
project(Engine)

# --- 1. DEPENDENCIES ---

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

# [NEW] NFD (Native File Dialog)
# We disable tests so it builds faster and doesn't clutter your solution
set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/nfd)

# ImGui Setup
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")
file(GLOB IMGUI_CORE_SOURCES CONFIGURE_DEPENDS "${IMGUI_DIR}/*.cpp")
set(IMGUI_BACKEND_SOURCES
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
set(IMGUI_SOURCES ${IMGUI_CORE_SOURCES} ${IMGUI_BACKEND_SOURCES})

# GLAD Setup
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad")
set(GLAD_SOURCES "${GLAD_DIR}/src/glad.c")

# --- 2. ENGINE SOURCES ---
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS "src/*.h")

# --- 3. DEFINE LIBRARY ---
# [CRITICAL] SHARED = Build as .DLL
add_library(Engine SHARED 
    ${ENGINE_SOURCES} 
    ${ENGINE_HEADERS} 
    ${IMGUI_SOURCES} 
    ${GLAD_SOURCES}
)

# [CRITICAL] 
# 1. CYCLOPS_BUILD_DLL: Tells Base.h to use __declspec(dllexport)
# 2. IMGUI_API: Tells ImGui to export its symbols from the DLL
target_compile_definitions(Engine PRIVATE 
    CYCLOPS_BUILD_DLL
    "IMGUI_API=__declspec(dllexport)"
)

# --- 4. INCLUDE DIRECTORIES ---
target_include_directories(Engine PUBLIC 
    src
    vendor/glfw/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLAD_DIR}/include
    vendor/stb_image
    # NFD usually handles its own includes via target_link_libraries, 
    # but we can add this just in case:
    vendor/nfd/src/include
)

# --- 5. LINKING ---
target_link_libraries(Engine PUBLIC 
    glfw 
    glm::glm
    nfd   # [NEW] Link the file dialog library
)

if(WIN32)
    target_link_libraries(Engine PUBLIC opengl32)
endif()