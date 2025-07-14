cmake_minimum_required(VERSION 3.10)
project(Pattern_Recognition)

set(CMAKE_CXX_STANDARD 17 )
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP REQUIRED)

set(SOURCES 
    src/main.cpp
    src/DataLoading.cpp
    src/SearchEngine.cpp
    src/Benchmark.cpp
)

add_executable(Pattern_Recognition ${SOURCES})

target_include_directories(Pattern_Recognition PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Pattern_Recognition PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(Pattern_Recognition PRIVATE ${OpenMP_CXX_FLAGS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_command(TARGET Pattern_Recognition POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/utils ${CMAKE_BINARY_DIR}/src/utils
)