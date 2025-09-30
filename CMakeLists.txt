cmake_minimum_required(VERSION 3.10)

project(MyCppProject
  VERSION 1.0.0
  LANGUAGES CXX
)

# Global C++ standard settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Library from src/ ----
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE LIB_SOURCES
  ${SRC_DIR}/*.cpp
)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Build the project code as a library (so examples can link to it)
add_library(CORE_LIB STATIC ${LIB_SOURCES})
# If you prefer a shared lib, switch STATIC -> SHARED.

# Public so examples inherit include paths on link
target_include_directories(CORE_LIB PUBLIC ${SRC_DIR} ${INCLUDE_DIR})
target_compile_options(CORE_LIB PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(CORE_LIB PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
set_target_properties(CORE_LIB PROPERTIES OUTPUT_NAME "core_lib")

# ---- Executables from example/ ----
set(EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/example)
file(GLOB EXAMPLE_SOURCES
  ${EXAMPLE_DIR}/*.cpp
)

# Create one executable per source file in example/
foreach(EX_SRC IN LISTS EXAMPLE_SOURCES)
  get_filename_component(EXE_NAME "${EX_SRC}" NAME_WE)

  add_executable(${EXE_NAME} "${EX_SRC}")
  target_include_directories(${EXE_NAME} PRIVATE ${EXAMPLE_DIR} ${INCLUDE_DIR})
  target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  # Link each example to the library
  target_link_libraries(${EXE_NAME} PRIVATE CORE_LIB)

  # Optional: set a friendlier output name (e.g., prefix with "example_")
  set_target_properties(${EXE_NAME} PROPERTIES OUTPUT_NAME "${EXE_NAME}")
endforeach()

# ---- Optional toggles ----
# You can gate examples behind an option if you like:
# option(BUILD_EXAMPLES "Build example executables" ON)
# if(NOT BUILD_EXAMPLES)
#   foreach(EX_SRC IN LISTS EXAMPLE_SOURCES)
#     get_filename_component(EXE_NAME "${EX_SRC}" NAME_WE)
#     set_target_properties(${EXE_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
#   endforeach()
# endif()
