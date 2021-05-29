cmake_minimum_required(VERSION 3.17)
project(supermap)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wshadow -Wpedantic -Werror)
set(LIBRARY supermap)
set(EXE supermap-cli)
set(TEST supermap-test)

set(LIBRARY_SOURCES
        src/exception/SupermapException.cpp
        src/exception/IllegalArgumentException.cpp
        src/exception/FileException.cpp
        src/exception/IteratorException.cpp
        src/io/OutputStream.cpp
        src/io/DiskFileManager.cpp
        src/io/RamFileManager.cpp
        src/io/InputStream.cpp
        src/core/BloomFilter.cpp src/core/SortedIndexedData.hpp)

set(CLI_SOURCES
        cli/main.cpp
        cli/CommandLineInterface.cpp
        )

set(TEST_SOURCES
        test/test.cpp)

add_library(${LIBRARY} SHARED ${LIBRARY_SOURCES})
target_include_directories(${LIBRARY} PUBLIC src)
target_link_libraries(${LIBRARY} stdc++ m)

add_executable(${TEST} ${TEST_SOURCES})
target_include_directories(${TEST} PRIVATE src)
target_include_directories(${TEST} PRIVATE extern/doctest/doctest/)
target_link_libraries(${TEST} stdc++ m ${LIBRARY})

add_executable(${EXE} ${CLI_SOURCES})
target_include_directories(${EXE} PRIVATE src)
target_include_directories(${EXE} PRIVATE extern/cxxopts/include)
target_link_libraries(${EXE} stdc++ m ${LIBRARY})
