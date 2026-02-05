cmake_minimum_required(VERSION 4.1)
project(GBemu C)

set(CMAKE_C_STANDARD 11)

add_executable(GBemu
        src/main.c
        src/cart.c
        src/cart.h
        src/cpu.c
        src/cpu.h
        src/opcodes.h
        src/opcodes.c)
