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
    ${PROJECT_SOURCE_DIR}/libs/glew
    ${PROJECT_SOURCE_DIR}/libs/glew/lib/x64
    ${PROJECT_SOURCE_DIR}/libs/glfw/lib-vc2022
)

# --- Static GLEW ---
add_definitions(-DGLEW_STATIC)

# ================= EXECUTABLE (BlackHole_RTX) =================
add_executable(BlackHole_RTX 
    src/black_hole.cpp 
    src/utilities.cpp 
    src/cercle.cpp 
    src/rayon.cpp
)

target_link_libraries(BlackHole_RTX
    glew32s
    glfw3
    opengl32
)

# ================= EXECUTABLE (BlackHole_BTZ) =================
add_executable(BlackHole_BTZ 
    src/main_btz.cpp 
    src/utilities.cpp 
    src/cercle.cpp 
    src/rayon.cpp
)

target_link_libraries(BlackHole_BTZ
    glew32s
    glfw3
    opengl32
)
