cmake_minimum_required(VERSION 3.19)
project(supermap)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wshadow -Wpedantic)
set(EXE ${CMAKE_PROJECT_NAME})

set(SOURCES
        src/main.cpp
        src/exception/SupermapException.cpp
        src/DiskIndex.cpp
        src/DiskDataStorage.cpp
        src/BloomFilter.cpp
        src/MemTable.cpp src/BST.cpp
        src/Supermap.cpp
        src/cli/CommandLineInterface.cpp
        src/cli/KvsCommandHandler.cpp
        src/KeyOffset.h)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE src)
target_include_directories(${EXE} PRIVATE extern/cxxopts/include)
target_link_libraries(${EXE} stdc++ m)
