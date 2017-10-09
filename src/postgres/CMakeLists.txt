set(LIB_NAME pg_render)

set(SOURCES postgres_type.cpp
    render.cpp
    postgres_upgrade.cpp)

add_library(${LIB_NAME} STATIC ${SOURCES} ${HEADERS})

set_target_properties(${LIB_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${LIB_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
)

if (UNIX)
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
elseif (MSVC)
    target_compile_options(${LIB_NAME} PUBLIC
        /W3
        /WX
        )
endif()
