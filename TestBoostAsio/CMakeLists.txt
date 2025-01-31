cmake_minimum_required (VERSION 3.20)

include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)

find_package(Boost 1.86.0 REQUIRED)

file(GLOB SOURCE_FILES *.cpp)
file(GLOB HEADER_FILES *.h)

add_executable (${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)
