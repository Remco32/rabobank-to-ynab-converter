cmake_minimum_required(VERSION 3.5)
project(RaboConverter3_Clion)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES
    main.c)

add_executable(RaboConverter3_Clion ${SOURCE_FILES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "F:/Dropbox/Programmeren/Clion_output")
