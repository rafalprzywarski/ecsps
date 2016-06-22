#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <ecsps/Math.hpp>
#include <ecsps/Keyword.hpp>
#include <ecsps/ResourcePool.hpp>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <algorithm>

namespace ecsps
{

template <typename... AllComponents>
class EntitySystem
{
public:
    template <typename... EntityComponents>
    void createEntity(EntityComponents&&... components)
    {
        Entity entity;
        addComponents(entity, std::forward<EntityComponents>(components)...);
        entities.push_back(std::move(entity));
    }

    template <typename... EntityComponents>
    auto query() const
    {
        return [this](auto f)
        {
            for (auto& entity : entities)
                if (entity.template hasComponents<EntityComponents...>())
                    f(std::get<std::vector<EntityComponents>>(components).at(entity.template getComponentIndex<EntityComponents>())...);
        };
    }

private:
    template <typename T>
    using strip = typename std::remove_const<typename std::remove_reference<T>::type>::type;

    struct Entity
    {
        std::unordered_map<std::type_index, std::ptrdiff_t> components;

        template <typename Component, typename Component2, typename... Components>
        bool hasComponents() const
        {
            return hasComponents<Component>() && hasComponents<Component2, Components...>();
        }

        template <typename Component>
        bool hasComponents() const
        {
            return components.find(std::type_index(typeid(Component))) != end(components);
        }

        template <typename Component>
        auto getComponentIndex() const
        {
            return components.at(std::type_index(typeid(Component)));
        }
    };

    void addComponents(Entity& entity) { }

    template <typename EntityComponent, typename... EntityComponents>
    void addComponents(Entity& entity, EntityComponent&& c, EntityComponents&&... cs)
    {
        auto& container = std::get<std::vector<strip<EntityComponent>>>(components);
        container.push_back(std::forward<EntityComponent>(c));
        entity.components[std::type_index(typeid(EntityComponent))] = container.size() - 1;
        addComponents(entity, std::forward<EntityComponents>(cs)...);
    }

    std::tuple<std::vector<AllComponents>...> components;
    std::vector<Entity> entities;
};

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

struct ViewComponent
{
    sf::FloatRect viewport;
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

    template <typename EntitySystem>
    void render(const EntitySystem& es)
    {
        window->clear();

        es.template query<ViewComponent>()([&](const ViewComponent& viewComponent)
        {
            sf::View view{window->getDefaultView().getCenter(), window->getDefaultView().getSize()};
            view.setViewport(viewComponent.viewport);
            window->setView(view);
            for (unsigned bin = 0, binCount = 1; bin < binCount; ++bin)
            {
                es.template query<TransformComponent, SpriteComponent>()([&](const TransformComponent& transformComponent, const SpriteComponent& spriteComponent)
                {
                    binCount = std::max<Bin>(binCount, spriteComponent.bin + 1);
                    if (spriteComponent.bin != bin)
                        return;
                    auto& sprite = sprites.find(spriteComponent.name)->second;
                    sf::Sprite ss{*sprite.texture};
                    ss.setOrigin(sprite.anchor[0], sprite.anchor[1]);
                    auto position = transformComponent.position;
                    ss.setPosition(position[0], position[1]);
                    window->draw(ss);
                });
            }
        });

        window->display();
    }
private:
    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<TexturePool> texturePool;
    std::unordered_map<Keyword, Sprite> sprites;
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

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        renderSystem.render(entitySystem);
    }
}
