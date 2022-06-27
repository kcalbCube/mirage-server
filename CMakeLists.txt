cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(MirageServer CXX)
set(PROJECT_NAME server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CMake/Utils.cmake)
include(CMake/cotire.cmake)

add_subdirectory("${CMAKE_SOURCE_DIR}/include/SDL")
add_subdirectory("${CMAKE_SOURCE_DIR}/include/boost")
add_subdirectory("${CMAKE_SOURCE_DIR}/include/fmtlog")
add_subdirectory("${CMAKE_SOURCE_DIR}/core")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

file(GLOB_RECURSE RESOURCES resource/*)
file(COPY ${RESOURCES} DESTINATION resource)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

cotire(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static SDL2main boost_serialization fmt-header-only fmtlog-static mirage-core fmt)
