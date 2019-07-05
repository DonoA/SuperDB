cmake_minimum_required(VERSION 3.10)
project(SuperDB C)

set(CMAKE_C_STANDARD 11)

add_executable(SuperDB main.c database.h database.c query_lang.c query_lang.h util.c util.h)