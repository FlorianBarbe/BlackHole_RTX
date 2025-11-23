cmake_minimum_required(VERSION 3.10)
project(BlackHole)

set(CMAKE_CXX_STANDARD 17)

# --- Include paths ---
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/libs/glew/include
    ${PROJECT_SOURCE_DIR}/libs/glfw/include
    ${PROJECT_SOURCE_DIR}/libs/glm
)

# --- Library paths ---
link_directories(
    ${PROJECT_SOURCE_DIR}/libs/glew/lib/x64
    ${PROJECT_SOURCE_DIR}/libs/glfw/lib-vc2022
)

# --- Source files ---
set(SRC_FILES
    src/main_gpu.cpp
    src/cercle.cpp
    src/rayon.cpp
    src/utilities.cpp
)

add_executable(BlackHole ${SRC_FILES})

# --- Static GLEW ---
add_definitions(-DGLEW_STATIC)

# --- Linking ---
target_link_libraries(BlackHole
    glew32s        # GLEW static release (x64)
    glfw3
    opengl32
)
