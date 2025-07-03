cmake_minimum_required(VERSION 3.10)

project(hoconv CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("include")
add_executable(hoconv hoconv.cc)

target_compile_options(hoconv PRIVATE -Wall -Wextra -O2)
target_include_directories(hoconv PRIVATE includema)
