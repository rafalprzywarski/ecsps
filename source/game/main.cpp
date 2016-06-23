#include "RenderSystem.hpp"
#include <ecsps/EntitySystem.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <ecsps/Math.hpp>
#include <ecsps/Keyword.hpp>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <algorithm>

namespace ecsps
{

std::shared_ptr<TexturePool> createTexturePool()
{
    return std::make_shared<TexturePool>([](const std::string& path)
    {
        std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();
        texture->loadFromFile(path);
        return texture;
    });
}

struct ColliderComponent
{
    vec2f size;
    vec2f anchor;
};

struct ForceComponent
{
    vec2f velocity;
    vec2f force;
};

class PhysicsSystem
{
public:
    template <typename EntitySystem>
    void step(EntitySystem& entitySystem)
    {

    }
};

}

int main()
{
    using namespace ecsps;

    EntitySystem<TransformComponent, SpriteComponent, ViewComponent> entitySystem;

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

    for (auto& c : spriteComponents)
        entitySystem.createEntity(c.first, c.second);

    entitySystem.createEntity(ViewComponent{sf::FloatRect{0, 0, 1, 1}});

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 960), "game", sf::Style::Titlebar | sf::Style::Close, settings);
    window->setVerticalSyncEnabled(true);

    RenderSystem renderSystem(window, createTexturePool());
    renderSystem.loadSprites(spriteDescs);
    PhysicsSystem physicsSystem;

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        physicsSystem.step(entitySystem);
        renderSystem.render(entitySystem);
    }
}
