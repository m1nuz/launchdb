
set(APP_NAME migrate-db)

#file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
#file(GLOB SHARED_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.hpp)

set(SOURCES main_migrate.cpp
    context_diff.cpp
    ../process_json.cpp)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})
add_dependencies(${APP_NAME} generate_version)

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

# Install
install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
