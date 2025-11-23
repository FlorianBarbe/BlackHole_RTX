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
    ${PROJECT_SOURCE_DIR}/libs/glew/lib
    ${PROJECT_SOURCE_DIR}/libs/glfw/lib-vc2022
)

# --- Source files ---
file(GLOB SRC_FILES src/*.cpp)

# --- Single executable definition ---
add_executable(BlackHole
    ${SRC_FILES}
    include/rayon.h
    include/cercle.h
 "include/utilities.h" "src/utilities.cpp" "src/rayon.cpp")

# --- Link GLFW ---
target_link_libraries(BlackHole glfw3)

# --- Link OpenGL ---
target_link_libraries(BlackHole opengl32)

# --- Required for static GLEW ---
add_definitions(-DGLEW_STATIC)
