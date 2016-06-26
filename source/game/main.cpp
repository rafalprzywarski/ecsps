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

struct CharacterState
{
    Keyword state = "idle"_k;
    Keyword direction = "right"_k;
};

class InputSystem
{
public:
    template <typename EntitySystem>
    void apply(EntitySystem& entitySystem)
    {
        entitySystem.template modify<MovementInputComponent, CharacterState, VelocityComponent>()([&](const auto& input, auto& state, auto& velocity)
        {
            state.state = shouldJump ? "jumping"_k : (movingRight != movingLeft ? "running"_k : "idle"_k);
            if (movingRight != movingLeft)
                state.direction = movingRight ? "right"_k : "left"_k;
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

template <typename T>
class im
{
public:
    im() : value{} {}
    im(T&& value) : value(std::move(value)) { }
    const T& operator*() const { return value; }
    const T *operator->() const { return &**this; }
private:
    const T value;
};

struct Animation
{
    std::vector<Keyword> frames;
    bool loop = true;
    float framesPerSecond = 15;
};

struct AnimationComponent
{
    Keyword animation;
    float time = 0;
};

class AnimationSystem
{
public:

    AnimationSystem(std::vector<std::pair<Keyword, Animation>> animations) : animations{begin(animations), end(animations)} { }

    template <typename EntitySystem>
    void step(EntitySystem& entitySystem, float delta)
    {
        entitySystem.template modify<SpriteComponent, AnimationComponent>()([&](auto& sprite, auto& animationComponent)
        {
            auto& animation = animations.at(animationComponent.animation);
            if (!animation.loop)
                animationComponent.time = std::min(animationComponent.time + delta, (animation.frames.size() - 1) / animation.framesPerSecond);
            else
                animationComponent.time = std::fmod(animationComponent.time + delta, animation.frames.size() / animation.framesPerSecond);
            sprite.name = animation.frames.at(animationComponent.time * animation.framesPerSecond);
        });
    }

private:
    std::unordered_map<Keyword, Animation> animations;
};

std::vector<Keyword> frameNames(const std::string& prefix, unsigned n)
{
    std::vector<Keyword> names;
    names.reserve(n);
    for (unsigned i = 1; i <= n; ++i)
        names.push_back(Keyword{prefix + std::to_string(i)});
    return names;
}

struct CharacterAnimation
{
    Keyword idle_left, idle_right;
    Keyword run_left, run_right;
    Keyword jump_left, jump_right;
};

class CharacterAnimationSystem
{
public:
    template <typename EntitySystem>
    void apply(EntitySystem& entitySystem)
    {
        entitySystem.template modify<CharacterAnimation, CharacterState, VelocityComponent, AnimationComponent>()([&](const auto& character, const auto& state, const auto& velocity, auto& animation)
        {
            if (state.state == "jumping"_k)
            {
                if (animation.animation == character.jump_left || animation.animation == character.jump_right)
                    return;
                animation.animation = state.direction == "left"_k ? character.jump_left : character.jump_right;
                animation.time = 0;
            }
            else if (state.state == "running"_k)
            {
                if (animation.animation == character.run_left || animation.animation == character.run_right)
                    return;
                animation.animation = state.direction == "left"_k ? character.run_left : character.run_right;
                animation.time = 0;
            }
            else
            {
                if (animation.animation == character.idle_left || animation.animation == character.idle_right)
                    return;
                animation.animation = state.direction == "left"_k ? character.idle_left : character.idle_right;
                animation.time = 0;
            }
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
        MovementInputComponent,
        CharacterAnimation,
        CharacterState> entitySystem;

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
        {"run_r_1"_k, {"assets/character/run_1.png", { 64, 128 }}},
        {"run_r_2"_k, {"assets/character/run_2.png", { 64, 128 }}},
        {"run_r_3"_k, {"assets/character/run_3.png", { 64, 128 }}},
        {"run_r_4"_k, {"assets/character/run_4.png", { 64, 128 }}},
        {"run_r_5"_k, {"assets/character/run_5.png", { 64, 128 }}},
        {"run_r_6"_k, {"assets/character/run_6.png", { 64, 128 }}},
        {"run_r_7"_k, {"assets/character/run_7.png", { 64, 128 }}},
        {"run_r_8"_k, {"assets/character/run_8.png", { 64, 128 }}},

        {"run_l_1"_k, {"assets/character/run_1.png", { 64, 128 }, true}},
        {"run_l_2"_k, {"assets/character/run_2.png", { 64, 128 }, true}},
        {"run_l_3"_k, {"assets/character/run_3.png", { 64, 128 }, true}},
        {"run_l_4"_k, {"assets/character/run_4.png", { 64, 128 }, true}},
        {"run_l_5"_k, {"assets/character/run_5.png", { 64, 128 }, true}},
        {"run_l_6"_k, {"assets/character/run_6.png", { 64, 128 }, true}},
        {"run_l_7"_k, {"assets/character/run_7.png", { 64, 128 }, true}},
        {"run_l_8"_k, {"assets/character/run_8.png", { 64, 128 }, true}},

        {"idle_r_1"_k, {"assets/character/idle_1.png", { 64, 128 }}},
        {"idle_r_2"_k, {"assets/character/idle_2.png", { 64, 128 }}},
        {"idle_r_3"_k, {"assets/character/idle_3.png", { 64, 128 }}},
        {"idle_r_4"_k, {"assets/character/idle_4.png", { 64, 128 }}},
        {"idle_r_5"_k, {"assets/character/idle_5.png", { 64, 128 }}},
        {"idle_r_6"_k, {"assets/character/idle_6.png", { 64, 128 }}},
        {"idle_r_7"_k, {"assets/character/idle_7.png", { 64, 128 }}},
        {"idle_r_8"_k, {"assets/character/idle_8.png", { 64, 128 }}},
        {"idle_r_9"_k, {"assets/character/idle_9.png", { 64, 128 }}},
        {"idle_r_10"_k, {"assets/character/idle_10.png", { 64, 128 }}},

        {"idle_l_1"_k, {"assets/character/idle_1.png", { 64, 128 }, true}},
        {"idle_l_2"_k, {"assets/character/idle_2.png", { 64, 128 }, true}},
        {"idle_l_3"_k, {"assets/character/idle_3.png", { 64, 128 }, true}},
        {"idle_l_4"_k, {"assets/character/idle_4.png", { 64, 128 }, true}},
        {"idle_l_5"_k, {"assets/character/idle_5.png", { 64, 128 }, true}},
        {"idle_l_6"_k, {"assets/character/idle_6.png", { 64, 128 }, true}},
        {"idle_l_7"_k, {"assets/character/idle_7.png", { 64, 128 }, true}},
        {"idle_l_8"_k, {"assets/character/idle_8.png", { 64, 128 }, true}},
        {"idle_l_9"_k, {"assets/character/idle_9.png", { 64, 128 }, true}},
        {"idle_l_10"_k, {"assets/character/idle_10.png", { 64, 128 }, true}},

        {"jump_r_1"_k, {"assets/character/jump_1.png", { 64, 128 }}},
        {"jump_r_2"_k, {"assets/character/jump_2.png", { 64, 128 }}},
        {"jump_r_3"_k, {"assets/character/jump_3.png", { 64, 128 }}},
        {"jump_r_4"_k, {"assets/character/jump_4.png", { 64, 128 }}},
        {"jump_r_5"_k, {"assets/character/jump_5.png", { 64, 128 }}},
        {"jump_r_6"_k, {"assets/character/jump_6.png", { 64, 128 }}},
        {"jump_r_7"_k, {"assets/character/jump_7.png", { 64, 128 }}},
        {"jump_r_8"_k, {"assets/character/jump_8.png", { 64, 128 }}},
        {"jump_r_9"_k, {"assets/character/jump_9.png", { 64, 128 }}},
        {"jump_r_10"_k, {"assets/character/jump_10.png", { 64, 128 }}},

        {"jump_l_1"_k, {"assets/character/jump_1.png", { 64, 128 }, true}},
        {"jump_l_2"_k, {"assets/character/jump_2.png", { 64, 128 }, true}},
        {"jump_l_3"_k, {"assets/character/jump_3.png", { 64, 128 }, true}},
        {"jump_l_4"_k, {"assets/character/jump_4.png", { 64, 128 }, true}},
        {"jump_l_5"_k, {"assets/character/jump_5.png", { 64, 128 }, true}},
        {"jump_l_6"_k, {"assets/character/jump_6.png", { 64, 128 }, true}},
        {"jump_l_7"_k, {"assets/character/jump_7.png", { 64, 128 }, true}},
        {"jump_l_8"_k, {"assets/character/jump_8.png", { 64, 128 }, true}},
        {"jump_l_9"_k, {"assets/character/jump_9.png", { 64, 128 }, true}},
        {"jump_l_10"_k, {"assets/character/jump_10.png", { 64, 128 }, true}},
    };

    std::vector<std::pair<Keyword, Animation>> animations = {
        {"run_r"_k, Animation{frameNames("run_r_", 8), true, 15}},
        {"run_l"_k, Animation{frameNames("run_l_", 8), true, 15}},
        {"idle_r"_k, Animation{frameNames("idle_r_", 10), true, 15}},
        {"idle_l"_k, Animation{frameNames("idle_l_", 10), true, 15}},
        {"jump_r"_k, Animation{frameNames("jump_r_", 10), false, 15}},
        {"jump_l"_k, Animation{frameNames("jump_l_", 10), false, 15}}
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
        SpriteComponent{"idle_r_1"_k, 3},
        AnimationComponent{"idle_r"_k, 0},
        CharacterAnimation{"idle_l"_k, "idle_r"_k, "run_l"_k, "run_r"_k, "jump_l"_k, "jump_r"_k},
        CharacterState{},
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
    CharacterAnimationSystem characterAnimationSystem;
    AnimationSystem animationSystem{animations};

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
        characterAnimationSystem.apply(entitySystem);
        animationSystem.step(entitySystem, delta);
    }
}
