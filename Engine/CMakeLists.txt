project(Engine)

# --- 1. VENDOR: GLFW ---
# This builds GLFW from source alongside your engine.
# GLFW_BUILD_DOCS=OFF prevents it from wasting time building documentation.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# --- 2. VENDOR: ImGui ---
# ImGui is just a few .cpp files, so we add them manually to our list.
# We also need the "backends" for GLFW and OpenGL3.
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")
file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# --- 3. VENDOR: GLAD ---
set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad")
set(GLAD_SOURCES "${GLAD_DIR}/src/glad.c")

# --- 4. ENGINE SOURCES ---
file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE ENGINE_HEADERS "src/*.h")

# Combine all sources
set(ALL_SOURCES ${ENGINE_SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES})
set(ALL_HEADERS ${ENGINE_HEADERS})

# --- 5. BUILD LIBRARY ---
add_library(Engine STATIC ${ALL_SOURCES} ${ALL_HEADERS})

# --- 6. INCLUDE DIRECTORIES ---
target_include_directories(Engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLAD_DIR}/include
)

# --- 7. LINKING ---
# We link the GLFW library that we just built (it's named 'glfw' by default).
target_link_libraries(Engine PUBLIC glfw)

if(WIN32)
    target_link_libraries(Engine PUBLIC opengl32)
endif()