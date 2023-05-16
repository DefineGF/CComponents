cmake_minimum_required(VERSION 3.1)
project(db)
add_definitions("-Wall -g")

include_directories(include)

aux_source_directory(src SRC_LIST)
add_executable(db ${SRC_LIST})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

