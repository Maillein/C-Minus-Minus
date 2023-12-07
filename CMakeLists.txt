cmake_minimum_required(VERSION 3.27)
project(cmm C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")

add_executable(cmm src/main.c
                   src/codegen.c
                   src/type.c
                   src/parser.c 
                   src/tokenizer.c
                   src/visualizer.c
              )
target_include_directories(cmm PRIVATE ${PROJECT_SOURCE_DIR}/include)
