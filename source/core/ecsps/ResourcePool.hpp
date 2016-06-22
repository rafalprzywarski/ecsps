#pragma once
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace ecsps
{

template <typename Id, typename Resource>
class ResourcePool : public std::enable_shared_from_this<ResourcePool<Id, Resource>>
{
public:
    using Factory = std::function<std::unique_ptr<const Resource>(const Id& )>;

    ResourcePool(Factory createResource) : createResource(std::move(createResource)) { }

    std::shared_ptr<const Resource> get(const Id& id)
    {
        std::lock_guard<std::mutex> lock{mutex};

        auto found = pool.find(id);
        if (found != end(pool))
            return found->second.lock();

        auto ptr = create(id);
        pool.insert({id, ptr});
        return ptr;
    }

private:
    Factory createResource;
    std::mutex mutex;
    std::unordered_map<Id, std::weak_ptr<const Resource>> pool;

    std::shared_ptr<const Resource> create(const Id& id)
    {
        std::weak_ptr<ResourcePool<Id, Resource>> weakThis = this->shared_from_this();
        auto deleter = [weakThis, id](const Resource *value)
        {
            if (auto _this = weakThis.lock())
                _this->forget(id);
            delete value;
        };
        return std::shared_ptr<const Resource>(
            createResource(id).release(),
            std::move(deleter));
    }

    void forget(const Id& value)
    {
        std::lock_guard<std::mutex> lock{mutex};
        pool.erase(value);
    }
};

}
