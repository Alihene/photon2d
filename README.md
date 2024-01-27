# Photon 2D

A flexible 2D rendering library based on OpenGL which supports sprite, text and UI rendering.

### Compiling

Supports GCC and Clang.

`cmake -S . -B build`

`cmake --build build`

### How to use

A demo application is located in `src/main.cpp` that encompasses the usage of the library.

### Features

- Draw textured sprites
- Texture atlases with custom UV coordinates
- Rotations
- Draw text using TTF fonts
- Draw UI elements
- Moveable camera
- Custom shader support
- Can be added as a CMake subdirectory