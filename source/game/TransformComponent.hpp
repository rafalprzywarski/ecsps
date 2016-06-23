#pragma once
#include <ecsps/Math.hpp>

namespace ecsps
{

struct TransformComponent
{
    vec2f position;
    TransformComponent(vec2f position) : position(position) { }
};

}
