﻿cmake_minimum_required(VERSION 3.20)

include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)

find_package(Boost 1.86.0 REQUIRED)
find_package(hiredis REQUIRED)

if (WIN32 AND MSVC)
	add_definitions(-D_WIN32_WINNT=0x0601)
endif()

file(GLOB SOURCE_FILES *.cpp)
file(GLOB HEADER_FILES *.h)

add_executable (${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost hiredis::hiredis)
