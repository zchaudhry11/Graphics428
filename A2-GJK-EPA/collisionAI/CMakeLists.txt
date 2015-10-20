project(untitled)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(. SRC_LIST)
aux_source_directory(./src SRC_LIST)

include_directories(./include)

file(GLOB .
    "*.h"
    "*.cpp"
)



add_executable(collisionAI ${SRC_LIST})

