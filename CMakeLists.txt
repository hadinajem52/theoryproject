cmake_minimum_required(VERSION 3.10)
project(TheoryProject VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML packages
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Source files
set(SOURCES
    main.cpp
    lexer.cpp
    parser.cpp
    token.cpp
    ast.cpp
    ast_extended.cpp
    semantic_analyzer.cpp
    js_code_generator.cpp
    automata_visualizer.cpp
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Create resources directory at configure time
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources)

# Try to copy Arial font from Windows system directory
if(WIN32)
    file(COPY "C:/Windows/Fonts/arial.ttf" 
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources
         FOLLOW_SYMLINK_CHAIN)
endif()

# Enable warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Add debug information in Debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Optimize in Release mode
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Define a custom target for running the program with visualization
add_custom_target(run_visualize
    COMMAND ${PROJECT_NAME} input.py output.js --visualize
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
