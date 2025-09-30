cmake_minimum_required(VERSION 3.10)

project(MyCppProject
  VERSION 1.0.0
  LANGUAGES CXX
)

# ---- Global config ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Helpful defaults
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build toggles
option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_BENCHMARKS "Build benchmarks (if Google Benchmark is found)" ON)
option(BUILD_TESTING "Build tests (if GoogleTest is found)" ON)

# Common dirs
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# ---- Core library from src/ ----
# Use explicit file list instead of GLOB_RECURSE for better build reliability
add_library(CORE_LIB STATIC
  ${SRC_DIR}/parser.cpp
)

# Public headers live in include/, src/ stays private
target_include_directories(CORE_LIB
  PUBLIC  ${INCLUDE_DIR}
  PRIVATE ${SRC_DIR}
)

# Cross-platform warnings
if(MSVC)
  target_compile_options(CORE_LIB PRIVATE /W4 /permissive-)
else()
  target_compile_options(CORE_LIB PRIVATE -Wall -Wextra -Wpedantic)
  # Add optimization flags for better performance in Release builds
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(CORE_LIB PRIVATE -O3 -march=native -flto)
    set_target_properties(CORE_LIB PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

target_compile_definitions(CORE_LIB PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
set_target_properties(CORE_LIB PROPERTIES OUTPUT_NAME "core_lib")

# ---- Examples from example/ ----
if(BUILD_EXAMPLES)
  set(EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/example)
  file(GLOB EXAMPLE_SOURCES "${EXAMPLE_DIR}/*.cpp")

  foreach(EX_SRC IN LISTS EXAMPLE_SOURCES)
    get_filename_component(EXE_NAME "${EX_SRC}" NAME_WE)

    add_executable(${EXE_NAME} "${EX_SRC}")
    target_include_directories(${EXE_NAME} PRIVATE ${EXAMPLE_DIR} ${INCLUDE_DIR})

    if(MSVC)
      target_compile_options(${EXE_NAME} PRIVATE /W4 /permissive-)
    else()
      target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    target_link_libraries(${EXE_NAME} PRIVATE CORE_LIB)
    set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "${EXE_NAME}")
  endforeach()
endif()

# ---- Benchmarks from benchmark/ ----
if(BUILD_BENCHMARKS)
  set(BENCH_DIR ${CMAKE_SOURCE_DIR}/benchmark)
  file(GLOB BENCH_SOURCES "${BENCH_DIR}/*.cpp")

  find_package(benchmark QUIET)
  find_package(Threads QUIET)

  if(benchmark_FOUND)
    foreach(BENCH_SRC IN LISTS BENCH_SOURCES)
      get_filename_component(BENCH_NAME "${BENCH_SRC}" NAME_WE)

      add_executable(${BENCH_NAME} "${BENCH_SRC}")
      target_include_directories(${BENCH_NAME} PRIVATE ${BENCH_DIR} ${INCLUDE_DIR})

      if(MSVC)
        target_compile_options(${BENCH_NAME} PRIVATE /W4 /permissive-)
      else()
        target_compile_options(${BENCH_NAME} PRIVATE -Wall -Wextra -Wpedantic)
      endif()

      target_link_libraries(${BENCH_NAME}
        PRIVATE CORE_LIB benchmark::benchmark
      )

      # pthreads on Unix, if available
      if(Threads_FOUND)
        target_link_libraries(${BENCH_NAME} PRIVATE Threads::Threads)
      endif()

      set_target_properties(${BENCH_NAME} PROPERTIES OUTPUT_NAME "${BENCH_NAME}")
    endforeach()
  else()
    message(WARNING "Google Benchmark not found. Benchmarks will not be built.")
  endif()
endif()

# ---- Tests from test/ ----
if(BUILD_TESTING)
  enable_testing()

  set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
  file(GLOB TEST_SOURCES "${TEST_DIR}/*.cpp")

  find_package(GTest QUIET)
  find_package(Threads QUIET)

  if(GTest_FOUND)
    foreach(TEST_SRC IN LISTS TEST_SOURCES)
      get_filename_component(TEST_NAME "${TEST_SRC}" NAME_WE)

      add_executable(${TEST_NAME} "${TEST_SRC}")
      target_include_directories(${TEST_NAME} PRIVATE ${TEST_DIR} ${INCLUDE_DIR})

      if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /W4 /permissive-)
      else()
        target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
      endif()

      target_link_libraries(${TEST_NAME}
        PRIVATE CORE_LIB GTest::gtest_main
      )

      if(Threads_FOUND)
        target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads)
      endif()

      set_target_properties(${TEST_NAME} PROPERTIES OUTPUT_NAME "${TEST_NAME}")
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
  else()
    message(WARNING "GoogleTest not found. Tests will not be built.")
  endif()
endif()
