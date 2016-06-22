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

using Bin = unsigned short;

struct TransformComponent
{
    vec2i position;
    TransformComponent(vec2i position) : position(position) { }
};

struct SpriteComponent
{
    Keyword name;
    Bin bin;

    SpriteComponent(Keyword name, Bin bin)
        : name(std::move(name)), bin(bin) { }
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

    void addComponents(SpriteComponent sprite, TransformComponent transform)
    {
        spriteComponents.push_back(sprite);
        transformComponents.push_back(transform);
    }

    void render()
    {
        window->clear();

        for (unsigned bin = 0, rendered = 0; bin < 128 && rendered < spriteComponents.size(); ++bin)
        {
            for (std::size_t i = 0; i < spriteComponents.size(); ++i)
            {
                if (spriteComponents[i].bin != bin)
                    continue;
                auto& sprite = sprites.find(spriteComponents[i].name)->second;
                sf::Sprite ss{*sprite.texture};
                ss.setOrigin(sprite.anchor[0], sprite.anchor[1]);
                auto position = transformComponents[i].position;
                ss.setPosition(position[0], position[1]);
                window->draw(ss);
                ++rendered;
            }
        }

        window->display();
    }
private:
    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<TexturePool> texturePool;
    std::unordered_map<Keyword, Sprite> sprites;
    std::vector<SpriteComponent> spriteComponents;
    std::vector<TransformComponent> transformComponents;
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

    std::vector<std::pair<SpriteComponent, TransformComponent>> spriteComponents = {
        {{"idle1"_k, 3}, {{320, 832}}},
        {{"tree"_k, 2}, {{0, 832}}},
        {{"grass"_k, 2}, {{256, 832}}},
        {{"cactus"_k, 2}, {{1152, 832}}},
        {{"tile1"_k, 1}, {{0, 832}}},
        {{"tile1"_k, 1}, {{1152, 832}}},
        {{"tile2"_k, 1}, {{128, 832}}},
        {{"tile3"_k, 1}, {{256, 832}}},
        {{"background"_k, 0}, {{0, 0}}}
    };

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 960), "game", sf::Style::Titlebar | sf::Style::Close, settings);
    window->setVerticalSyncEnabled(true);

    RenderSystem renderSystem(window, createTexturePool());
    renderSystem.loadSprites(spriteDescs);
    for (auto& c : spriteComponents)
        renderSystem.addComponents(c.first, c.second);

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        renderSystem.render();
    }
}
