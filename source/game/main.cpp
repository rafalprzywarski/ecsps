#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <ecsps/Math.hpp>
#include <ecsps/Keyword.hpp>
#include <ecsps/ResourcePool.hpp>
#include <unordered_map>

namespace ecsps
{

using TexturePool = ResourcePool<std::string, sf::Texture>;

std::shared_ptr<TexturePool> createTexturePool()
{
    return std::make_shared<TexturePool>([](const std::string& path)
    {
        std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();
        texture->loadFromFile(path);
        return texture;
    });
}

struct SpriteDesc
{
    std::string texture;
    vec2i anchor;
    SpriteDesc(std::string texture, vec2i anchor)
        : texture(std::move(texture)), anchor(anchor) { }
};

struct SpriteComponent
{
    Keyword name;
    vec2i position;

    SpriteComponent(Keyword name, vec2i position)
        : name(std::move(name)), position(position) { }
};

struct Sprite
{
    std::shared_ptr<const sf::Texture> texture;
    vec2i anchor;

    Sprite(std::shared_ptr<const sf::Texture> texture, const vec2i& anchor)
        : texture(std::move(texture)), anchor(anchor) { }
};

class RenderSystem
{
public:
    RenderSystem(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<TexturePool> texturePool)
        : window(window), texturePool(texturePool) { }

    void loadSprites(std::vector<std::pair<Keyword, SpriteDesc>> spriteDescs)
    {
        for (auto& desc : spriteDescs)
            sprites.insert({desc.first, Sprite{texturePool->get(desc.second.texture), desc.second.anchor}});
    }

    void setComponents(std::vector<SpriteComponent> components)
    {
        spriteComponents = components;
    }

    void render()
    {
        window->clear();

        for (auto& component : spriteComponents)
        {
            auto& sprite = sprites.find(component.name)->second;
            sf::Sprite ss{*sprite.texture};
            ss.setOrigin(sprite.anchor[0], sprite.anchor[1]);
            ss.setPosition(component.position[0], component.position[1]);
            window->draw(ss);
        }

        window->display();
    }
private:
    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<TexturePool> texturePool;
    std::unordered_map<Keyword, Sprite> sprites;
    std::vector<SpriteComponent> spriteComponents;
};

}

int main()
{
    using namespace ecsps;

    std::vector<std::pair<Keyword, SpriteDesc>> spriteDescs = {
        {"background"_k, {"assets/bg.png", { 0, 0 }}},
        {"tile1"_k, {"assets/tiles/1.png", { 0, 0 }}},
        {"tile2"_k, {"assets/tiles/2.png", { 0, 0 }}},
        {"tile3"_k, {"assets/tiles/3.png", { 0, 0 }}},
        {"tree"_k, {"assets/objects/tree.png", { 0, 260 }}},
        {"grass"_k, {"assets/objects/grass2.png", { 0, 50 }}},
        {"cactus"_k, {"assets/objects/cactus3.png", { 0, 96 }}},
        {"idle1"_k, {"assets/character/idle_1.png", { 64, 128 }}},
    };

    std::vector<SpriteComponent> spriteComponents = {
        {"background"_k, {0, 0}},
        {"tile1"_k, {0, 832}},
        {"tile1"_k, {1152, 832}},
        {"tile2"_k, {128, 832}},
        {"tile3"_k, {256, 832}},
        {"tree"_k, {0, 832}},
        {"grass"_k, {256, 832}},
        {"cactus"_k, {1152, 832}},
        {"idle1"_k, {320, 832}}
    };

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 960), "game", sf::Style::Titlebar | sf::Style::Close, settings);
    window->setVerticalSyncEnabled(true);

    RenderSystem renderSystem(window, createTexturePool());
    renderSystem.loadSprites(spriteDescs);
    renderSystem.setComponents(spriteComponents);

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        renderSystem.render();
    }
}
