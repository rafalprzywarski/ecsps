#pragma once
#include <memory>
#include <unordered_map>
#include <mutex>

namespace ecsps
{

template <typename Value>
class ValuePool : public std::enable_shared_from_this<ValuePool<Value>>
{
public:
    std::shared_ptr<const Value> get(const Value& value)
    {
        std::lock_guard<std::mutex> lock{mutex};

        auto found = pool.find(value);
        if (found != end(pool))
            return found->second.lock();

        auto ptr = create(value);
        pool.insert({value, ptr});
        return ptr;
    }

private:
    std::mutex mutex;
    std::unordered_map<Value, std::weak_ptr<const Value>> pool;

    std::shared_ptr<const Value> create(const Value& value)
    {
        std::weak_ptr<ValuePool<Value>> weakThis = this->shared_from_this();
        return std::shared_ptr<Value>(
            new Value(value),
            [weakThis](const Value *value)
            {
                if (auto _this = weakThis.lock())
                    _this->forget(*value);
                delete value;
            });
    }

    void forget(const Value& value)
    {
        std::lock_guard<std::mutex> lock{mutex};
        pool.erase(value);
    }
};

}
