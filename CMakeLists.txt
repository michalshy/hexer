cmake_minimum_required(VERSION 3.16)

project(hexer
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HEXER_BUILD_TESTS "Build hexer unit tests" ON)

add_subdirectory(lib)
add_subdirectory(src)

if(HEXER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
