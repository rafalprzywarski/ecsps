#include "Keyword.hpp"
#include "ValuePool.hpp"

namespace ecsps
{

Keyword::Keyword(const std::string& name)
{
    static auto pool = std::make_shared<ValuePool<std::string>>();
    this->name = pool->get(name);
}

}
