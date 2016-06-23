#pragma once
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <vector>

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

}
