cmake_minimum_required(VERSION 3.6)
project(Apriori)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES apriori.c hashtable.c hashtable.h lookup3.c lookup3.h)
add_executable(Apriori ${SOURCE_FILES})