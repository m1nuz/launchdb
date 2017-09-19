
set(APP_NAME grab-db)

set(SOURCES main_grab.cpp)

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