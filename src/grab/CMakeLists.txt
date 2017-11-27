
find_package(PostgreSQL REQUIRED)
find_package(SQLite3 REQUIRED)

set(APP_NAME grab-db)

set(SOURCES main_grab.cpp)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
    ${PostgreSQL_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
)

if (UNIX)
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
elseif (MSVC)
    target_compile_options(${APP_NAME} PUBLIC
        /W3
#        /WX
        )
endif()
target_link_libraries(${APP_NAME} PUBLIC
    pg_render
    ${PostgreSQL_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

# Install
install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
