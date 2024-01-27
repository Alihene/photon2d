# Photon 2D

A flexible 2D rendering library based on OpenGL which supports sprite, text and UI rendering.

### Compiling

Supports GCC and Clang.

`cmake -S . -B build`

`cmake --build build`

### How to use

A demo application is located in `src/main.cpp` that encompasses the usage of the library.

### Features

- [x] Draw textured sprites
- [x] Texture atlases with custom UV coordinates
- [x] Rotations
- [x] Draw text using TTF fonts
- [x] Draw UI elements
- [x] Moveable camera
- [x] Custom shader support
- [x] Can be added as a CMake subdirectory