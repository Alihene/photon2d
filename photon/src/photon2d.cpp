#include "photon2d.hpp"

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <stb_truetype/stb_truetype.h>
#include <cstring>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

const char *vertexShaderSource =
    "#version 330 core\n"
    "in vec2 aPos;\n"
    "in vec4 aColor;\n"
    "in vec2 aTexCoord;\n"
    "out vec4 vColor;\n"
    "out vec2 vTexCoord;\n"
    "uniform mat4 uProj;\n"
    "uniform mat4 uView;\n"
    "void main() {\n"
    "   gl_Position = uProj * uView * vec4(aPos, 0.0, 1.0);\n"
    "   vColor = aColor;\n"
    "   vTexCoord = aTexCoord;\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "in vec2 vTexCoord;\n"
    "out vec4 color;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "   color = vColor * texture(uTexture, vTexCoord);\n"
    "}\n";

static void errorCallback(i32 err, const char *msg) {
    std::cerr << "GLFW Error (" << err << "): " << msg << std::endl;
    std::exit(-1);
}

static void framebufferSizeCallback(GLFWwindow *w, i32 width, i32 height) {
    photon::Window *window = reinterpret_cast<photon::Window*>(glfwGetWindowUserPointer(w));
    window->dimensions = glm::uvec2(width, height);
    glViewport(0, 0, width, height);
}

photon::Window::Window(std::string name, u32 width, u32 height, bool resizable) : dimensions(width, height) {
    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::exit(-1);
    }

    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    handle = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

    if(!handle) {
        std::cerr << "Failed to create window" << std::endl;
        std::exit(-1);
    }

    glfwSetWindowUserPointer(handle, this);
    glfwSetFramebufferSizeCallback(handle, framebufferSizeCallback);

    glfwMakeContextCurrent(handle);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL" << std::endl;
        std::exit(-1);
    }
}

void photon::Window::destroy() {
    glfwTerminate();
}

void photon::Window::endFrame() {
    glfwSwapBuffers(handle);
    glfwPollEvents();
}

bool photon::Window::shouldClose() {
    return glfwWindowShouldClose(handle);
}

f32 photon::Window::aspectRatio() const {
    return (f32) dimensions.x / (f32) dimensions.y;
}

photon::ShaderProgram::ShaderProgram(std::string vertexSource, std::string fragmentSource) {
    i32 vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    i32 fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);

    const char *vertexSourcePtr = vertexSource.c_str();
    const char *fragmentSourcePtr = fragmentSource.c_str();

    i32 result;
    i32 logLength;

    glShaderSource(vertexHandle, 1, &vertexSourcePtr, nullptr);
    glCompileShader(vertexHandle);

    glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);

    if(result == GL_FALSE) {
        std::vector<char> error(logLength + 1);
        glGetShaderInfoLog(vertexHandle, logLength, nullptr, error.data());
        std::cerr << "Failed to compile vertex shader: " << error.data() << std::endl;
        std::exit(-1);
    }

    glShaderSource(fragmentHandle, 1, &fragmentSourcePtr, nullptr);
    glCompileShader(fragmentHandle);

    glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength);

    if(result == GL_FALSE) {
        std::vector<char> error(logLength + 1);
        glGetShaderInfoLog(fragmentHandle, logLength, nullptr, error.data());
        std::cerr << "Failed to compile fragment shader: " << error.data() << std::endl;
        std::exit(-1);
    }

    handle = glCreateProgram();
    glAttachShader(handle, vertexHandle);
    glAttachShader(handle, fragmentHandle);
    glLinkProgram(handle);

    glGetProgramiv(handle, GL_LINK_STATUS, &result);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);

    if(logLength > 0) {
        std::vector<char> error(logLength + 1);
        glGetProgramInfoLog(handle, logLength, nullptr, error.data());
        std::cerr << "Failed to link program: " << error.data() << std::endl;
        std::exit(-1);
    }

    glDetachShader(handle, vertexHandle);
    glDetachShader(handle, fragmentHandle);

    glDeleteShader(vertexHandle);
    glDeleteShader(fragmentHandle);
}

void photon::ShaderProgram::bind() {
    glUseProgram(handle);
}

void photon::ShaderProgram::setInt(std::string location, i32 value) {
    glUniform1i(glGetUniformLocation(handle, location.c_str()), value);
}

void photon::ShaderProgram::setFloat(std::string location, f32 value) {
    glUniform1f(glGetUniformLocation(handle, location.c_str()), value);
}

void photon::ShaderProgram::setVec2(std::string location, glm::vec2 value) {
    glUniform2fv(glGetUniformLocation(handle, location.c_str()), 1, &value[0]);
}

void photon::ShaderProgram::setMat4(std::string location, glm::mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(handle, location.c_str()), 1, GL_FALSE, &value[0][0]);
}

photon::Texture::Texture(u8 *data, u32 width, u32 height, TextureType type) : type(type) {
    glGenTextures(1, &handle);
    bind();

    if(type == RGB) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if(type == RGBA) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else if(type == RED) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

photon::Texture::Texture(std::string path, TextureType type) : type(type) {
    glGenTextures(1, &handle);
    bind();

    i32 width, height, channels;
    u8 *data;

    if(type == RGB) {
        data = stbi_load(path.c_str(), &width, &height, &channels, 3);
    } else if(type == RGBA) {
        data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    } else if(type == RED) {
        data = stbi_load(path.c_str(), &width, &height, &channels, 1);
    }

    if(!data) {
        std::cerr << "Failed to load image " << path << std::endl;
        std::exit(-1);
    }

    if(type == RGB) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if(type == RGBA) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else if(type == RED) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
}

void photon::Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, handle);
}

void photon::Texture::activate(u8 index) {
    glActiveTexture(GL_TEXTURE0 + index);
}

photon::Sprite::Sprite(glm::vec2 pos, glm::vec2 size, photon::Texture *texture) : pos(pos), size(size), texture(texture) {

}

bool photon::Sprite::isAdded() {
    return batch != nullptr;
}

void photon::Sprite::update() {
    if(isAdded()) {
        batch->updateSprite(this);
    }
}

void photon::Sprite::toggleInvisibility() {
    if(isAdded()) {
        if(invisible) {
            update();
            invisible = false;
        } else {
            float *vertices = (float*) std::calloc(Renderer2D::VERTEX_SIZE * 6, sizeof(f32));
            batch->rawSetVertices(batchIndex, vertices);
            std::free(vertices);
            if(batch->shouldBuffer == false) {
                batch->shouldBuffer = true;
            }
            invisible = true;
        }
    }
}

void photon::Sprite::remove() {
    if(isAdded()) {
        batch->removeSprite(this);
        batch = nullptr;
        batchIndex = 0;
    }
}

photon::Font::Font(std::string path) {
    std::ifstream fontFile(path.c_str(), std::ios::binary);

    if(!fontFile.is_open()) {
        std::cerr << "Failed to open TTF file " << path << std::endl;
        std::exit(-1);
    }

    i32 fileSize = 0;
    fontFile.seekg(0, std::ios::end);
    fileSize = (i32) fontFile.tellg();
    fontFile.seekg(0, std::ios::beg);
    u8 *data = new u8[fileSize];
    fontFile.read((char*) data, fileSize);
    fontFile.close();

    createFromTTF(data, fileSize);

    delete[] data;
}

void photon::Font::createFromTTF(const u8 *data, const usize dataSize) {
    stbtt_InitFont(&info, data, 0);

    size.x = 4096;
    size.y = 4096;
    maxHeight = 0;
    packedCharsBufferSize = ('~' - ' ');

    const usize fontMonochromeBufferSize = size.x * size.y;
	const usize fontRgbaBufferSize = size.x * size.y * 4;

    u8 *fontMonochromeBuffer = new u8[fontMonochromeBufferSize];
    u8 *fontRgbaBuffer = new u8[fontRgbaBufferSize];

    packedCharsBuffer = new stbtt_packedchar[packedCharsBufferSize] {};

    stbtt_pack_context stbttContext;
    stbtt_PackBegin(&stbttContext, fontMonochromeBuffer, size.x, size.y, 0, 2, nullptr);
    stbtt_PackSetOversampling(&stbttContext, 2, 2);
    stbtt_PackFontRange(&stbttContext, data, 0, 65, ' ', '~' - ' ', packedCharsBuffer);
	stbtt_PackEnd(&stbttContext);

    for(i32 i = 0; i < fontMonochromeBufferSize; i++) {
        fontRgbaBuffer[(i * 4)] = fontMonochromeBuffer[i];
		fontRgbaBuffer[(i * 4) + 1] = fontMonochromeBuffer[i];
		fontRgbaBuffer[(i * 4) + 2] = fontMonochromeBuffer[i];

		if (fontMonochromeBuffer[i] > 1) {
			fontRgbaBuffer[(i * 4) + 3] = 255;
		} else {
			fontRgbaBuffer[(i * 4) + 3] = 0;
		}
    }

    texture.type = Texture::RGBA;
    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontRgbaBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    delete[] fontMonochromeBuffer;
	delete[] fontRgbaBuffer;

    for (char c = ' '; c <= '~'; c++) {
        stbtt_aligned_quad quad = getGlyphQuad(c);
        f32 height = quad.y1 - quad.y0;

        if(height > maxHeight && height < 1.e+8f) {
            maxHeight = height;
        }
    }
}

stbtt_aligned_quad photon::Font::getGlyphQuad(const char c) {
    stbtt_aligned_quad quad = {0};

	f32 x = 0;
	f32 y = 0;

	stbtt_GetPackedQuad(packedCharsBuffer, size.x, size.y, c - ' ', &x, &y, &quad, 1);
    return quad;
}

glm::vec4 photon::Font::getGlyphTexCoords(const char c) {
    f32 xOffset = 0.0f;
    f32 yOffset = 0.0f;

    stbtt_aligned_quad quad = getGlyphQuad(c);

    return glm::vec4(quad.s0, quad.t0, quad.s1, quad.t1);
}

i32 photon::Font::getGlyphKern(const char c1, const char c2) {
    i32 kern = stbtt_GetCodepointKernAdvance(&info, c1, c2);
    return kern;
}

photon::Text::Text(Font *font, std::string str, glm::vec2 pos, f32 size, f32 spacing) : font(font), str(str), pos(pos), size(size), spacing(spacing) {
    createSprites();
}

void photon::Text::createSprites() {
    u32 length = str.length();

    f32 xPos = pos.x;
    f32 yPos = pos.y;

    f32 scale = stbtt_ScaleForPixelHeight(&font->info, 64.0f);

    i32 ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);

    for(i32 i = 0; i < length; i++) {
        char c = str[i];

        stbtt_aligned_quad quad = font->getGlyphQuad(c);

        glm::ivec4 metrics;
        stbtt_GetGlyphBitmapBox(&font->info, c, scale, scale, &metrics.x, &metrics.y, &metrics.z, &metrics.w);

        f32 maxSize = font->getGlyphQuad('A').y1 - font->getGlyphQuad('A').y0;

        if(c == ' ') {
            stbtt_aligned_quad lineQuad = font->getGlyphQuad('-');
            
            f32 kern = 0.0f;
            if(i < length - 1) {
                kern = font->getGlyphKern('-', str[i + 1]);
            }

            xPos += (lineQuad.x1 - lineQuad.x0) * size + spacing + kern * size * scale;
        } else if (c >= ' ' && c <= '~') {
            
            f32 kern = 0.0f;
            if(i < length - 1) {
                kern = font->getGlyphKern(c, str[i + 1]);
            }

            photon::Sprite sprite(glm::vec2(xPos, yPos - quad.y1 * size), glm::vec2((quad.x1 - quad.x0) * size, (quad.y1 - quad.y0) * size), &font->texture);
            sprite.texCoords = font->getGlyphTexCoords(c);
            sprites.push_back(sprite);
            xPos += (quad.x1 - quad.x0) * size + spacing + kern * size * scale;
        }
    }
}

void photon::Text::update() {
    for(Sprite &sprite : sprites) {
        sprite.remove();
    }

    sprites.clear();

    createSprites();
}

photon::SpriteBatch::SpriteBatch(Texture *texture, ShaderProgram *shader) : texture(texture), shader(shader) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, BATCH_SIZE * 6 * Renderer2D::VERTEX_SIZE_BYTES, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, Renderer2D::VERTEX_SIZE_BYTES, (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Renderer2D::VERTEX_SIZE_BYTES, (void*) (2 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, Renderer2D::VERTEX_SIZE_BYTES, (void*) (6 * sizeof(f32)));
    glEnableVertexAttribArray(2);

    data = new f32[BATCH_SIZE * 6 * Renderer2D::VERTEX_SIZE];
}

void photon::SpriteBatch::addSprite(Sprite *sprite) {
    if(hasSpace()) {
        sprite->batch = this;
        sprite->batchIndex = spriteCount;

        spriteCount++;

        updateSprite(sprite);
    }
}

void photon::SpriteBatch::updateSprite(Sprite *sprite) {
    f32 vertices[] = {
        sprite->pos.x, sprite->pos.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.x, sprite->texCoords.w,
        sprite->pos.x + sprite->size.x, sprite->pos.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.z, sprite->texCoords.w,
        sprite->pos.x + sprite->size.x, sprite->pos.y + sprite->size.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.z, sprite->texCoords.y,
        sprite->pos.x + sprite->size.x, sprite->pos.y + sprite->size.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.z, sprite->texCoords.y,
        sprite->pos.x, sprite->pos.y + sprite->size.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.x, sprite->texCoords.y,
        sprite->pos.x, sprite->pos.y, sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a, sprite->texCoords.x, sprite->texCoords.w,
    };

    rawSetVertices(sprite->batchIndex, vertices);
    shouldBuffer = true;
}

void photon::SpriteBatch::removeSprite(Sprite *sprite) {
    std::memcpy(&data[sprite->batchIndex * 6 * Renderer2D::VERTEX_SIZE], &data[(spriteCount - 1) * 6 * Renderer2D::VERTEX_SIZE], Renderer2D::VERTEX_SIZE * 6 * sizeof(f32));
    std::memset(&data[(spriteCount - 1) * 6 * Renderer2D::VERTEX_SIZE], 0, 6 * Renderer2D::VERTEX_SIZE_BYTES);
    sprite->batch = nullptr;
    sprite->batchIndex = 0;

    spriteCount--;
    shouldBuffer = true;
}

bool photon::SpriteBatch::hasSpace() {
    return spriteCount < BATCH_SIZE;
}

void photon::SpriteBatch::rawSetVertices(i32 index, f32 *vertices) {
    std::memcpy(&data[(index * 6 * Renderer2D::VERTEX_SIZE)], vertices, 6 * Renderer2D::VERTEX_SIZE_BYTES);
}

void photon::SpriteBatch::render(const Camera &camera) {
    if(shouldBuffer) {
        bufferData();
        shouldBuffer = false;
    }

    shader->bind();
    shader->setInt("uTexture", 0);
    shader->setMat4("uProj", camera.proj);
    shader->setMat4("uView", camera.view);

    glBindVertexArray(vao);

    texture->bind();

    glDrawArrays(GL_TRIANGLES, 0, spriteCount * 6);
}

void photon::SpriteBatch::destroy() {
    delete[] data;
}

void photon::SpriteBatch::bufferData() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, BATCH_SIZE * 6 * Renderer2D::VERTEX_SIZE_BYTES, data);
}

photon::Renderer2D::Renderer2D(const Window *window) : window(window) {
    shader = ShaderProgram(std::string(vertexShaderSource), std::string(fragmentShaderSource));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void photon::Renderer2D::setClearColor(f32 r, f32 g, f32 b, f32 a) {
    glClearColor(r, g, b, a);
}

void photon::Renderer2D::addSprite(Sprite *sprite) {
    bool added = false;

    for(SpriteBatch *batch : batches) {
        if(batch->texture == sprite->texture && batch->hasSpace()) {
            batch->addSprite(sprite);
            added = true;
            break;
        }
    }

    if(!added) {
        SpriteBatch *batch = new SpriteBatch(sprite->texture, &shader);
        batch->addSprite(sprite);
        batches.push_back(batch);
    }
}

void photon::Renderer2D::addText(Text *text) {
    for(Sprite &sprite : text->sprites) {
        addSprite(&sprite);
    }
}

void photon::Renderer2D::updateText(Text *text) {
    text->update();
    addText(text);
}

void photon::Renderer2D::render() {
    glClear(GL_COLOR_BUFFER_BIT);

    f32 aspectRatio = window->aspectRatio();

    if(aspectRatio >= 1.0f) {
        camera.proj = glm::ortho(0.0f, 100.0f * aspectRatio, 0.0f, 100.0f, -1.0f, 1.0f);
    } else {
        camera.proj = glm::ortho(0.0f, 100.0f, 0.0f, 100.0f / aspectRatio, -1.0f, 1.0f);
    }

    camera.view =
        glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f)
		    + glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

    for(SpriteBatch *batch : batches) {
        batch->render(camera);
    }
}

void photon::Renderer2D::destroy() {
    for(SpriteBatch *batch : batches) {
        batch->destroy();
        delete batch;
    }
}