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

struct StaticColliderComponent
{
    vec2f size;
    vec2f anchor;
};

struct ColliderComponent
{
    vec2f size;
    vec2f anchor;
};

struct VelocityComponent
{
    vec2f velocity;
    vec2f previousPosition;
};

struct GravityComponent
{
    float gravity;
};

class PhysicsSystem
{
public:
    template <typename EntitySystem>
    void step(EntitySystem& entitySystem, float delta)
    {
        entitySystem.template modify<TransformComponent, VelocityComponent>()([&](auto& transformComponent, auto& velocityComponent)
        {
            velocityComponent.previousPosition = transformComponent.position;
            transformComponent.position += velocityComponent.velocity * delta;
        });
        entitySystem.template modify<TransformComponent, VelocityComponent, GravityComponent>()([&](auto& transformComponent, auto& velocityComponent, const auto& gravityComponent)
        {
            transformComponent.position += vec2f{0, gravityComponent.gravity * delta * delta / 2};
            velocityComponent.velocity += vec2f{0, gravityComponent.gravity * delta};
        });
        entitySystem.template modify<TransformComponent, VelocityComponent, GravityComponent, ColliderComponent>()([&](auto& transformComponent, auto& velocityComponent, const auto& gravityComponent, const auto& collider)
        {
            entitySystem.template query<TransformComponent, StaticColliderComponent>()([&](const auto& staticTransform, const auto& staticCollider)
            {
                vec2f dynPos = transformComponent.position - collider.anchor;
                vec2f dynSize = collider.size;
                vec2f staPos = staticTransform.position - staticCollider.anchor;
                vec2f staSize = staticCollider.size;
                vec2f prevPos = velocityComponent.previousPosition - collider.anchor;

                if (collides(dynPos, dynSize, staPos, staSize))
                {
                    if (!collides({dynPos[0], prevPos[1]}, dynSize, staPos, staSize))
                    {
                        dynPos[1] = prevPos[1];
                        velocityComponent.velocity[1] = 0;
                    }
                    else if (!collides({prevPos[0], dynPos[1]}, dynSize, staPos, staSize))
                    {
                        dynPos[0] = prevPos[0];
                        velocityComponent.velocity[0] = 0;
                    }
                    else
                    {
                        dynPos = prevPos;
                        velocityComponent.velocity = {0, 0};
                    }
                    transformComponent.position = dynPos + collider.anchor;
                }
            });
        });
    }

private:
    static bool collides(vec2f pos1, vec2f size1, vec2f pos2, vec2f size2)
    {
        return
            pos1[0] + size1[0] > pos2[0] && pos1[0] < pos2[0] + size2[0] &&
            pos1[1] + size1[1] > pos2[1] && pos1[1] < pos2[1] + size2[1];
    }
};

struct MovementInputComponent
{
    float movementSpeed{};
};

class InputSystem
{
public:
    template <typename EntitySystem>
    void apply(EntitySystem& entitySystem)
    {
        entitySystem.template modify<MovementInputComponent, VelocityComponent>()([&](const auto& input, auto& velocity)
        {
            velocity.velocity[0] = 0;
            if (movingRight)
                velocity.velocity[0] += input.movementSpeed;
            if (movingLeft)
                velocity.velocity[0] -= input.movementSpeed;
            if (shouldJump)
                velocity.velocity[1] = -input.movementSpeed;
        });
    }

    void moveLeft(bool yes) { movingLeft = yes; }
    void moveRight(bool yes) { movingRight = yes; }
    void jump(bool yes) { shouldJump = yes; }

private:
    bool movingRight = false;
    bool movingLeft = false;
    bool shouldJump = false;
};

struct AnimationComponent
{
    std::vector<Keyword> frames;
    float framesPerSecond = 10;
    float time = 0;
};

class AnimationSystem
{
public:
    template <typename EntitySystem>
    void step(EntitySystem& entitySystem, float delta)
    {
        entitySystem.template modify<SpriteComponent, AnimationComponent>()([&](auto& sprite, auto& animation)
        {
            animation.time = std::fmod(animation.time + delta, animation.frames.size() / animation.framesPerSecond);
            sprite.name = animation.frames.at(animation.time * animation.framesPerSecond);
        });
    }
};

}

int main()
{
    using namespace ecsps;

    EntitySystem<
        TransformComponent,
        SpriteComponent,
        AnimationComponent,
        ViewComponent,
        StaticColliderComponent,
        ColliderComponent,
        VelocityComponent,
        GravityComponent,
        MovementInputComponent> entitySystem;

    std::vector<std::pair<Keyword, SpriteDesc>> spriteDescs = {
        {"background"_k, {"assets/bg.png", { 0, 0 }}},
        {"tile1"_k, {"assets/tiles/1.png", { 0, 0 }}},
        {"tile2"_k, {"assets/tiles/2.png", { 0, 0 }}},
        {"tile3"_k, {"assets/tiles/3.png", { 0, 0 }}},
        {"tile8"_k, {"assets/tiles/8.png", { 0, 0 }}},
        {"tile6"_k, {"assets/tiles/6.png", { 0, 0 }}},
        {"tile7"_k, {"assets/tiles/7.png", { 0, 0 }}},
        {"tile14"_k, {"assets/tiles/14.png", { 0, 0 }}},
        {"tile15"_k, {"assets/tiles/15.png", { 0, 0 }}},
        {"tile16"_k, {"assets/tiles/16.png", { 0, 0 }}},
        {"tree"_k, {"assets/objects/tree.png", { 0, 260 }}},
        {"grass"_k, {"assets/objects/grass2.png", { 0, 50 }}},
        {"cactus"_k, {"assets/objects/cactus3.png", { 0, 96 }}},
        {"run1"_k, {"assets/character/run_1.png", { 64, 128 }}},
        {"run2"_k, {"assets/character/run_2.png", { 64, 128 }}},
        {"run3"_k, {"assets/character/run_3.png", { 64, 128 }}},
        {"run4"_k, {"assets/character/run_4.png", { 64, 128 }}},
        {"run5"_k, {"assets/character/run_5.png", { 64, 128 }}},
        {"run6"_k, {"assets/character/run_6.png", { 64, 128 }}},
        {"run7"_k, {"assets/character/run_7.png", { 64, 128 }}},
        {"run8"_k, {"assets/character/run_8.png", { 64, 128 }}},
    };

    std::vector<std::pair<SpriteComponent, TransformComponent>> spriteComponents = {
        {{"tree"_k, 2}, {{0, 832}}},
        {{"grass"_k, 2}, {{256, 704}}},
        {{"cactus"_k, 2}, {{1152, 832}}},
        {{"background"_k, 0}, {{0, 0}}}
    };

    std::vector<std::pair<SpriteComponent, TransformComponent>> tiles = {
        {{"tile2"_k, 1}, {{0, 832}}},
        {{"tile7"_k, 1}, {{128, 832}}},
        {{"tile8"_k, 1}, {{256, 832}}},
        {{"tile6"_k, 1}, {{384, 832}}},

        {{"tile1"_k, 1}, {{256, 704}}},
        {{"tile3"_k, 1}, {{384, 704}}},

        {{"tile14"_k, 1}, {{640, 576}}},
        {{"tile15"_k, 1}, {{768, 576}}},
        {{"tile16"_k, 1}, {{896, 576}}},

        {{"tile1"_k, 1}, {{1152, 832}}}
    };

    for (auto& c : spriteComponents)
        entitySystem.createEntity(c.first, c.second);

    for (auto& c : tiles)
        entitySystem.createEntity(c.first, c.second, StaticColliderComponent{{128, 128}, {0, 0}});

    entitySystem.createEntity(ViewComponent{sf::FloatRect{0, 0, 1, 1}});
    entitySystem.createEntity(
        SpriteComponent{"run1"_k, 3},
        AnimationComponent{{"run1"_k, "run2"_k, "run3"_k, "run4"_k, "run5"_k, "run6"_k, "run7"_k, "run8"_k}, 15, 0},
        TransformComponent{{100, 822}},
        VelocityComponent{{100, -400}},
        GravityComponent{1200},
        ColliderComponent{{70, 129}, {24, 128}},
        MovementInputComponent{400});

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    auto window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 960), "game", sf::Style::Titlebar | sf::Style::Close, settings);
    window->setVerticalSyncEnabled(true);

    RenderSystem renderSystem(window, createTexturePool());
    renderSystem.loadSprites(spriteDescs);
    PhysicsSystem physicsSystem;
    InputSystem inputSystem;
    AnimationSystem animationSystem;

    sf::Clock clock;
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
            if (event.type == sf::Event::Closed)
                window->close();

        inputSystem.moveRight(sf::Keyboard::isKeyPressed(sf::Keyboard::Right));
        inputSystem.moveLeft(sf::Keyboard::isKeyPressed(sf::Keyboard::Left));
        inputSystem.jump(sf::Keyboard::isKeyPressed(sf::Keyboard::Up));

        auto delta = clock.restart().asSeconds();
        physicsSystem.step(entitySystem, delta);
        renderSystem.render(entitySystem);
        inputSystem.apply(entitySystem);
        animationSystem.step(entitySystem, delta);
    }
}
