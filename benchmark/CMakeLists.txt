#todo: fix this and include this as part of the main CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(language_parser_bench CXX)

#Configure build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#Allow the caller to provide the parser sources OR a prebuilt target to link.
#Option 1 : Provide sources directly
#cmake - DPARSER_SOURCES = "/path/parser.cpp;/path/key.cpp;..." ...
#Option 2 : Provide an existing target name to link(e.g., your library)
#cmake - DLANGUAGE_PARSER_TARGET = your_parser_lib...

set(PARSER_SOURCES "" CACHE STRING "Semicolon-separated list of parser source files to compile into the bench")
set(LANGUAGE_PARSER_TARGET "" CACHE STRING "Existing CMake target that implements LanguageParser to link against")

#Google Benchmark via FetchContent
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.4
)
FetchContent_MakeAvailable(benchmark)

#Benchmarks
add_executable(language_parser_bench
  bench/bench_language_parser.cpp
)

#C++ standard &warnings
set_target_properties(language_parser_bench PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
if (MSVC)
  target_compile_options(language_parser_bench PRIVATE /W4)
else()
  target_compile_options(language_parser_bench PRIVATE -Wall -Wextra -Wpedantic)
endif()

#Include dirs : add your include root here if needed
#Example:
#target_include_directories(                                                   \
    # language_parser_bench PRIVATE ${CMAKE_SOURCE_DIR} / include)

#Link Google Benchmark &pthread(Linux)
target_link_libraries(language_parser_bench PRIVATE benchmark::benchmark)
if (UNIX AND NOT APPLE)
  target_link_libraries(language_parser_bench PRIVATE pthread)
endif()

#Hook up your parser implementation
if (LANGUAGE_PARSER_TARGET)
  message(STATUS "Linking to existing target: ${LANGUAGE_PARSER_TARGET}")
  target_link_libraries(language_parser_bench PRIVATE ${LANGUAGE_PARSER_TARGET})
elseif (PARSER_SOURCES)
  message(STATUS "Compiling parser sources: ${PARSER_SOURCES}")
  target_sources(language_parser_bench PRIVATE ${PARSER_SOURCES})
else()
  message(FATAL_ERROR "Provide LANGUAGE_PARSER_TARGET or PARSER_SOURCES so the benchmarks can link your implementation.")
endif()