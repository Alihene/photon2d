cmake_minimum_required(VERSION 3.16)

project("glm")

add_library("glm" INTERFACE)

target_include_directories("glm" INTERFACE include "include")