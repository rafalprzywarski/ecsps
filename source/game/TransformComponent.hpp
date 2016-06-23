#pragma once
#include <ecsps/Math.hpp>

namespace ecsps
{

struct TransformComponent
{
    vec2i position;
    TransformComponent(vec2i position) : position(position) { }
};

}
