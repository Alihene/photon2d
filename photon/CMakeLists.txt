cmake_minimum_required(VERSION 3.16)

project("photon2d")

set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -g -fsanitize=address,undefined")

add_library("photon2d" STATIC "src/photon2d.cpp")

target_include_directories("photon2d" PUBLIC "include")

target_link_libraries("photon2d" PUBLIC glfw glad glm stb_image stb_truetype)