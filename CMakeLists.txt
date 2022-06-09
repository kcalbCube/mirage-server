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
add_subdirectory("${CMAKE_SOURCE_DIR}/include/boost" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")

file(GLOB_RECURSE RESOURCES resource/*)
file(COPY ${RESOURCES} DESTINATION resources)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC
	-std=c++20 
	-Wpedantic 
	-Wall
	-Wfloat-conversion 
	-Wno-unused-but-set-variable
	-march=native
)

cotire(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/magic_enum/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/fmt/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/entt/src")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/SDL/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/imgui")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/boost")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/include/cpp-typelist")

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static SDL2main boost_serialization)
