cmake_minimum_required(VERSION 3.23)

project(OBDDisplayApp)

add_executable(OBDDisplayApp)

target_sources(OBDDisplayApp
    PRIVATE
        source/main.cpp
)

