
set(APP_NAME migrate-db)

#file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
#file(GLOB SHARED_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.hpp)

set(SOURCES main_migrate.cpp
    context_diff.cpp
    ../process_json.cpp
    ../postgres/postgres_type.cpp
    ../postgres/render.cpp
    ../postgres/postgres_upgrade.cpp)

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
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
)

# Install
install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
