cmake_minimum_required(VERSION 3.15)
project(fastest C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ── fastest static library (C sources) ───────────────────────────────────────
file(GLOB FASTEST_SRC src/*.c)

add_library(fastest_static STATIC ${FASTEST_SRC})
target_include_directories(fastest_static PUBLIC include)
target_compile_options(fastest_static PRIVATE -Wall -Wextra -O2)

set_target_properties(fastest_static PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
    OUTPUT_NAME fastest
)
