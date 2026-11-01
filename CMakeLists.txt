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

find_package(ROOT 6 REQUIRED COMPONENTS Core RIO Hist Tree TreePlayer)

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
    src/Event.cpp
    src/Categories.cpp
    src/Analyzer.cpp)
target_include_directories(ppanalysis PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ppanalysis PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::TreePlayer)
target_compile_options(ppanalysis PUBLIC -Wall -Wextra)

add_executable(pp_generate src/generate.cpp)
target_link_libraries(pp_generate PRIVATE ppanalysis Pythia8::Pythia8)

add_executable(pp_analyze src/analyze.cpp)
target_link_libraries(pp_analyze PRIVATE ppanalysis)

set(PP_NEVENTS 100000 CACHE STRING "Number of events for run_generate")
set(PP_SEED 12345 CACHE STRING "Random seed for run_generate")

add_custom_target(run_generate
    COMMAND ${CMAKE_COMMAND} -E make_directory output
    COMMAND pp_generate --config config/pythia_13p6TeV.cmnd --nevents ${PP_NEVENTS}
                        --seed ${PP_SEED} --output output/events.root
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS pp_generate
    USES_TERMINAL)

add_custom_target(run_analyze
    COMMAND pp_analyze --input output/events.root --output output/analysis.root
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS pp_analyze
    USES_TERMINAL)

find_program(ROOT_EXECUTABLE root HINTS ${ROOT_BINDIR})
add_custom_target(plots
    COMMAND ${ROOT_EXECUTABLE} -l -b -q macros/draw_plots.C
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL)

add_custom_target(run_13TeV
    COMMAND ${CMAKE_COMMAND} -E make_directory output
    COMMAND pp_generate --config config/pythia_13TeV.cmnd --nevents ${PP_NEVENTS}
                        --seed ${PP_SEED} --output output/events_13TeV.root
    COMMAND pp_analyze --input output/events_13TeV.root --output output/analysis_13TeV.root
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS pp_generate pp_analyze
    USES_TERMINAL)

add_custom_target(comparison
    COMMAND ${ROOT_EXECUTABLE} -l -b -q macros/draw_comparison.C
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL)

add_custom_target(pipeline
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_generate
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_analyze
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target plots
    USES_TERMINAL)
