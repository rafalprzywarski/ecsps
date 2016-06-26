#pragma once
#include <memory>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <ecsps/ResourcePool.hpp>
#include <ecsps/Keyword.hpp>
#include <ecsps/Math.hpp>
#include "TransformComponent.hpp"

namespace ecsps
{

using TexturePool = ResourcePool<std::string, sf::Texture>;

struct SpriteDesc
{
    std::string texture;
    vec2i anchor;
    bool mirrored{};
    SpriteDesc(std::string texture, vec2i anchor, bool mirrored = false)
        : texture(std::move(texture)), anchor(anchor), mirrored(mirrored) { }
};

struct Sprite
{
    std::shared_ptr<const sf::Texture> texture;
    vec2i anchor;
    bool mirrored{};

    Sprite(std::shared_ptr<const sf::Texture> texture, const vec2i& anchor, bool mirrored)
        : texture(std::move(texture)), anchor(anchor), mirrored(mirrored) { }
};

using Bin = unsigned short;

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
    sf::FloatRect view;
};

class RenderSystem
{
public:
    RenderSystem(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<TexturePool> texturePool)
        : window(window), texturePool(texturePool) { }

    void loadSprites(std::vector<std::pair<Keyword, SpriteDesc>> spriteDescs)
    {
        for (auto& desc : spriteDescs)
            sprites.insert({desc.first, Sprite{texturePool->get(desc.second.texture), desc.second.anchor, desc.second.mirrored}});
    }

    template <typename EntitySystem>
    void render(const EntitySystem& es)
    {
        window->clear();

        es.template query<ViewComponent>()([&](const ViewComponent& viewComponent)
        {
            sf::View view{viewComponent.view};
            view.setViewport(viewComponent.viewport);
            window->setView(view);
            for (unsigned bin = 0, binCount = 1; bin < binCount; ++bin)
            {
                es.template query<TransformComponent, SpriteComponent>()([&](const TransformComponent& transformComponent, const SpriteComponent& spriteComponent)
                {
                    binCount = std::max<Bin>(binCount, spriteComponent.bin + 1);
                    if (spriteComponent.bin != bin)
                        return;
                    auto& sprite = sprites.at(spriteComponent.name);
                    sf::Sprite ss{*sprite.texture};
                    if (sprite.mirrored)
                    {
                        ss.setScale(-1, 1);
                        ss.setOrigin(sprite.texture->getSize().x - sprite.anchor[0], sprite.anchor[1]);
                    }
                    else
                    {
                        ss.setOrigin(sprite.anchor[0], sprite.anchor[1]);
                    }
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
