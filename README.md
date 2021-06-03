# Supermap

[![CMake](https://github.com/Glebanister/supermap/actions/workflows/cmake.yml/badge.svg)](https://github.com/Glebanister/supermap/actions/workflows/cmake.yml)

Key-value keys implementation.

## Installation

First make sure `git` and `cmake` are installed,
`C++17` standard is supported on your machine.
Then execute following commands.

```bash
$ git clone --recursive https://github.com/Glebanister/supermap
$ mkdir build
$ cd build
$ cmake ...
```

## Usage

```bash
$ ./build/supermap 
supermap> help
'get' - Get value of key from keys
'remove' - Remove key from keys
'add' - Add key to keys
'contains' - Check if key in keys
'stop' - Stop interface
'help' - Get help
supermap> 
```
