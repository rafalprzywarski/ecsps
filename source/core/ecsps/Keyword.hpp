#pragma once
#include <string>
#include <functional>

namespace ecsps
{

class Keyword
{
public:
    Keyword() : Keyword(std::string{}) { }
    explicit Keyword(const std::string& name);

    const std::string& str() const { return *name; }

    std::size_t hash() const
    {
        return std::hash<decltype(name)>()(name);
    }

    friend bool operator==(const Keyword& left, const Keyword& right)
    {
        return left.name == right.name;
    }

private:
    std::shared_ptr<const std::string> name;
};

inline Keyword operator""_k(const char *text, std::size_t length)
{
    return Keyword{std::string{text, length}};
}

inline bool operator!=(const Keyword& left, const Keyword& right)
{
    return !(left == right);
}

}

namespace std
{

template<>
struct hash<ecsps::Keyword>
{
    using argument_type = ecsps::Keyword;
    using result_type = std::size_t;
    result_type operator()(argument_type const& k) const
    {
        return k.hash();
    }
};

}
