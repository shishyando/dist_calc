cmake_minimum_required(VERSION 3.10)
project(1)

set(CMAKE_CXX_STANDARD 20)

add_executable(master m.cpp)
add_executable(worker w.cpp)

add_library(task_manager STATIC task_manager.cpp)
add_library(worker_manager STATIC worker_manager.cpp)
target_link_libraries(master task_manager)
target_link_libraries(master worker_manager)

target_compile_options(master PRIVATE -Wall -Wextra)
target_compile_options(worker PRIVATE -Wall -Wextra)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
