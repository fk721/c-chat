cmake_minimum_required(VERSION 3.10)
project(c-chat)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Link header and source files
include_directories(include)
file(GLOB SOURCES src/*.c)
add_executable(c-chat ${SOURCES})

# Link ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(c-chat ${CURSES_LIBRARIES})

# Link pthreads
find_package(Threads)
target_link_libraries(c-chat ${CMAKE_THREAD_LIBS_INIT})

