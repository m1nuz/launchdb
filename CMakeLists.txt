cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(launchdb)

set(PROJECT_NAME_UPPER ${PROJECT_NAME})
string(TOUPPER ${PROJECT_NAME_UPPER} PROJECT_NAME_UPPER)

include_directories(${PROJECT_BINARY_DIR})

# Set variable for the generate sources
set(GENERATE_SRCS ${PROJECT_SOURCE_DIR}/src/generate_version.c)
set(GENERATE_OUTPUT ${PROJECT_BINARY_DIR}/generate_build_version)
set(VERSION_OUTPUT ${PROJECT_BINARY_DIR}/version.h)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION_META "-dev")

# Create command to compile the generate command
add_custom_command(OUTPUT ${GENERATE_OUTPUT}
    COMMAND gcc -o ${GENERATE_OUTPUT} ${GENERATE_SRCS}
    DEPENDS ${GENERATE_SRCS})

add_custom_target(generate_version
    COMMAND ${GENERATE_OUTPUT}
        -M ${VERSION_MAJOR}
        -m ${VERSION_MINOR}
        -p ${VERSION_PATCH}
        -P ${PROJECT_NAME_UPPER}
        -a "${VERSION_META}"
        -F "${PROJECT_BINARY_DIR}/buildnumber"
        -i
        > ${VERSION_OUTPUT}
    DEPENDS ${GENERATE_OUTPUT}
    VERBATIM)

include_directories(include)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.hpp)
file(GLOB SHARED_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.hpp)

add_executable(launch-db ${SOURCES} ${HEADERS} ${SHARED_HEADERS})
add_dependencies(launch-db generate_version)
set_target_properties(launch-db PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_include_directories(launch-db PUBLIC
    include
)
target_compile_options(launch-db PUBLIC
    -pthread
    -pedantic
    -Wall
    -Wextra
    -Wshadow
    -Wpointer-arith
    -Wcast-qual
    -Wunused-result
)
#target_link_libraries(launch-db)
