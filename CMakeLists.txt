cmake_minimum_required(VERSION 3.1...3.16)

project(wordsmith VERSION 0.1.0 LANGUAGES C)
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# SET(BUILD_SHARED_LIBS OFF)
# SET(CMAKE_EXE_LINKER_FLAGS "-static")

find_library(hs REQUIRED libhs)

add_executable(ws src/main.c)
target_include_directories(ws PUBLIC ${hs_INCLUDE_DIRS})
target_link_directories(ws PUBLIC ${hs_LIBRARY_DIRS})
target_link_libraries(ws PUBLIC ${hs_LIBRARIES})
