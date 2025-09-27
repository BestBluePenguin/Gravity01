cmake_minimum_required(VERSION 3.10.0)
project(Gravity3D VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

find_package(GLEW REQUIRED)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
    )
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)

add_executable(Gravity3D main.cpp)


target_include_directories(Gravity3D PRIVATE ${GLEW_INCLUDE_DIRS})

target_link_libraries(Gravity3D PRIVATE 
    GLEW::glew
    OpenGL::GL
    glfw
    glm::glm
    )



include(CTest)
enable_testing()

