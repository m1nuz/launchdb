
set(APP_NAME migrate-db)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
file(GLOB SHARED_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.hpp)

set(VERSION_OUTPUT ${PROJECT_BINARY_DIR}/migrate_version.h)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION_META "-dev")

string(TOUPPER "migratedb" APP_NAME_UPPER)

add_custom_target(generate_migrate_version
    COMMAND ${GENERATE_OUTPUT}
        -M ${VERSION_MAJOR}
        -m ${VERSION_MINOR}
        -p ${VERSION_PATCH}
        -P ${APP_NAME_UPPER}
        -a "${VERSION_META}"
        -F "${PROJECT_BINARY_DIR}/buildnumber"
        -i
        > ${VERSION_OUTPUT}
    DEPENDS ${GENERATE_OUTPUT}
    VERBATIM)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS} ${SHARED_HEADERS})
add_dependencies(${APP_NAME} generate_migrate_version)

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
)
target_compile_options(${APP_NAME} PUBLIC
    -pthread
    -pedantic
    -Wall
    -Wextra
    -Wshadow
    -Wpointer-arith
    -Wcast-qual
    -Wunused-result
    -g
)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
