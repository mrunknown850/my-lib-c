cmake_minimum_required(VERSION 3.10)
project(mylloc C)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Add include directory
include_directories(include)

# Grab all source files in src/
file(GLOB SOURCES "src/*.c")

# Define the executable
add_executable(mylloc ${SOURCES})
