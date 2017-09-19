set(LIB_NAME pg_render)

set(SOURCES postgres_type.cpp
    render.cpp
    postgres_upgrade.cpp)

add_library(${LIB_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${LIB_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
)

target_compile_options(${LIB_NAME} PUBLIC
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
