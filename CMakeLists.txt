cmake_minimum_required(VERSION 3.16)

project("photon2d-demo")

add_subdirectory("thirdparty/glfw")
add_subdirectory("thirdparty/glad")
add_subdirectory("thirdparty/glm")
add_subdirectory("thirdparty/stb_image")
add_subdirectory("thirdparty/stb_truetype")

add_subdirectory("photon")

add_executable("photon2d-demo" "src/main.cpp")

target_link_libraries("photon2d-demo" PRIVATE glfw glad glm stb_image stb_truetype photon2d)