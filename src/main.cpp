#include <photon2d.hpp>

int main() {
    photon::Window window("Hello, World!", 854, 480, true);
    photon::Renderer2D renderer(&window);
    renderer.setClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    photon::Texture nullTexture("../resources/null.png", photon::Texture::RGBA);
    photon::Texture cowTexture("../resources/cow.png", photon::Texture::RGBA);

    photon::Sprite sprite(glm::vec2(0.0f), glm::vec2(25.0f), &nullTexture);
    photon::Sprite sprite2(glm::vec2(25.0f, 0.0f), glm::vec2(25.0f), &cowTexture);

    renderer.addSprite(&sprite);
    renderer.addSprite(&sprite2);

    photon::Font font("../resources/arial.ttf");
    photon::Text text(&font, "Hello, Photon!", glm::vec2(0.0f, 50.0f), 0.15f, 0.5f);
    renderer.addText(&text);

    f64 lastTime = glfwGetTime();
    i32 nbFrames = 0;

    while(!window.shouldClose()) {
        f64 currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        renderer.render();
        window.endFrame();
    }

    renderer.destroy();
    window.destroy();

    return 0;
}