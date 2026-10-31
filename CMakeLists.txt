cmake_minimum_required(VERSION 3.20)
project(pp_13p6tev_analysis LANGUAGES CXX)

# Code is C++17; Homebrew ROOT 6.40 headers require C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(ROOT 6 REQUIRED COMPONENTS Core RIO Tree)

set(PYTHIA8_DIR "" CACHE PATH "Pythia 8 installation prefix (optional)")
if(NOT PYTHIA8_DIR)
    find_program(PYTHIA8_CONFIG pythia8-config)
    if(PYTHIA8_CONFIG)
        execute_process(COMMAND ${PYTHIA8_CONFIG} --prefix
                        OUTPUT_VARIABLE PYTHIA8_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endif()
find_path(PYTHIA8_INCLUDE_DIR Pythia8/Pythia.h HINTS ${PYTHIA8_DIR}/include)
find_library(PYTHIA8_LIBRARY pythia8 HINTS ${PYTHIA8_DIR}/lib)
if(NOT PYTHIA8_INCLUDE_DIR OR NOT PYTHIA8_LIBRARY)
    message(FATAL_ERROR "Pythia 8 not found. Install it or pass -DPYTHIA8_DIR=<prefix>.")
endif()
add_library(Pythia8::Pythia8 UNKNOWN IMPORTED)
set_target_properties(Pythia8::Pythia8 PROPERTIES
    IMPORTED_LOCATION ${PYTHIA8_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${PYTHIA8_INCLUDE_DIR})
message(STATUS "Pythia 8: ${PYTHIA8_LIBRARY}")

add_library(ppanalysis STATIC
    src/Event.cpp)
target_include_directories(ppanalysis PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ppanalysis PUBLIC ROOT::Core ROOT::RIO ROOT::Tree)
target_compile_options(ppanalysis PUBLIC -Wall -Wextra)

add_executable(pp_generate src/generate.cpp)
target_link_libraries(pp_generate PRIVATE ppanalysis Pythia8::Pythia8)
