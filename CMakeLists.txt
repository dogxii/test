cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(Robot3DRoaming LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(Robot3DRoaming ${PROJECT_SOURCES})

if(WIN32)
    set(DUST2_OBJ_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/de_dust2-cs-map/source/de_dust2/de_dust2.obj"
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dust2Resource.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Dust2Resource.rc"
        @ONLY
    )
    target_sources(Robot3DRoaming PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/Dust2Resource.rc"
    )
    target_compile_definitions(Robot3DRoaming PRIVATE
        ROBOT3D_EMBEDDED_DUST2=1
    )
endif()

target_include_directories(Robot3DRoaming PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(Robot3DRoaming PRIVATE
    glfw
    glm::glm
    OpenGL::GL
)

if(APPLE)
    target_compile_options(Robot3DRoaming PRIVATE -Wno-deprecated-declarations)
endif()

if(MSVC)
    set_property(TARGET Robot3DRoaming PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()
