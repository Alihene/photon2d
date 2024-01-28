#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_truetype/stb_truetype.h>

#include <cstdint>
#include <iostream>
#include <vector>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;

namespace photon {

typedef void (*KeyCallback)(i32, i32);

struct SpriteBatch;

struct Camera {
    glm::mat4 view;
    glm::mat4 proj;
};

struct Window {
    GLFWwindow *handle;

    glm::uvec2 dimensions;
    KeyCallback keyCallback = nullptr;

    Window(std::string name, u32 width, u32 height, bool resizable);

    void destroy();

    void endFrame();
    bool shouldClose();

    f32 aspectRatio() const;

    bool isKeyDown(i32 key);
};

struct ShaderProgram {
    u32 handle;

    ShaderProgram() = default;
    ShaderProgram(std::string vertexCode, std::string fragmentCode);

    void bind();

    void setInt(std::string location, i32 value);
    void setFloat(std::string location, f32 value);
    void setVec2(std::string location, glm::vec2 value);
    void setMat4(std::string location, glm::mat4 value);
};

struct Texture {
    enum TextureType {
        RGB,
        RGBA,
        RED
    };

    u32 handle;
    TextureType type;

    Texture() = default;
    Texture(u8 *data, u32 width, u32 height, TextureType type);
    Texture(std::string path, TextureType type);

    void bind();

    static void activate(u8 index);
};

struct Sprite {
    SpriteBatch *batch = nullptr;
    i32 batchIndex = 0;

    bool invisible = false;

    glm::vec2 pos;
    glm::vec4 color = glm::vec4(1.0f);
    glm::vec2 size;
    Texture *texture;
    glm::vec4 texCoords = {0.0f, 0.0f, 1.0f, 1.0f};

    Sprite() = default;
    Sprite(glm::vec2 pos, glm::vec2 size, Texture *texture);

    bool isAdded();

    void update();
    void toggleInvisibility();

    void remove();
};

struct Font {
    stbtt_fontinfo info;
    Texture texture;
    glm::ivec2 size;
    stbtt_packedchar *packedCharsBuffer = 0;
    i32 packedCharsBufferSize = 0;
    f32 maxHeight = 0.0f;

    Font() = default;
    Font(std::string path);

    void createFromTTF(const u8 *data, const usize dataSize);

    stbtt_aligned_quad getGlyphQuad(const char c);
    glm::vec4 getGlyphTexCoords(const char c);
};

struct Text {
    Font *font;

    std::string str;
    std::vector<Sprite> sprites;

    glm::vec2 pos;
    f32 size;
    glm::vec4 color;
    f32 spacing;
    bool centered;

    Text() = default;
    Text(Font *font, std::string str, glm::vec2 pos, f32 size, glm::vec4 color, f32 spacing, bool centered);

    void createSprites();
    void update();
};

struct SpriteBatch {
    static constexpr u32 BATCH_SIZE = 10000;

    f32 *data = nullptr;

    u32 vao;
    u32 vbo;

    Texture *texture;
    ShaderProgram *shader;

    u32 spriteCount = 0;

    bool shouldBuffer = false;

    SpriteBatch() = default;
    SpriteBatch(Texture *texture, ShaderProgram *shader);

    void addSprite(Sprite *sprite);
    void updateSprite(Sprite *sprite);
    void removeSprite(Sprite *sprite);

    bool hasSpace();

    void rawSetVertices(i32 index, f32 *vertices);

    void render(const Camera &camera);

    void destroy();

private:
    void bufferData();
};

struct Renderer2D {
    static constexpr usize VERTEX_SIZE = 8;
    static constexpr usize VERTEX_SIZE_BYTES = VERTEX_SIZE * sizeof(f32);

    const Window *window;

    ShaderProgram shader;

    Camera camera;

    Renderer2D() = default;
    Renderer2D(const Window *window);

    Renderer2D(const Renderer2D &other) = delete;
    Renderer2D(Renderer2D &&other) = delete;
    Renderer2D &operator=(const Renderer2D &other) = delete;
    Renderer2D &operator=(Renderer2D &&other) = delete;

    void setClearColor(f32 r, f32 g, f32 b, f32 a);

    void addSprite(Sprite *sprite);
    
    void addText(Text *text);
    void updateText(Text *text);

    void render();

    void destroy();

private:
    std::vector<SpriteBatch*> batches;
};

}