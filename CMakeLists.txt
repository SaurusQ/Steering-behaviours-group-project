cmake_minimum_required (VERSION 3.1)

if(WIN32)
	set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
endif()

file(GLOB SOURCES src/*.cpp src/3D/*.cpp libs/glfw-3.3/deps/*.c src/core/*.cpp)

project(main VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw-3.3/)


set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}")

add_subdirectory(src)
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})