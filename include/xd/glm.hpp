#ifndef H_XD_GLM
#define H_XD_GLM

#define GLM_FORCE_CTOR_INIT

#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/type_ptr.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

namespace xd
{
    // Brought in individually to avoid conflicts (e.g. QT)
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    using glm::ivec2;
    using glm::ivec3;
    using glm::ivec4;
    using glm::mat2;
    using glm::mat3;
    using glm::mat4;
    using glm::translate;
    using glm::rotate;
    using glm::scale;
    using glm::normalize;
    using glm::length;
    using glm::ortho;
    using glm::perspective;
    using glm::radians;
    using glm::degrees;
    using glm::sin;
    using glm::cos;
    using glm::tan;
    using glm::sinh;
    using glm::cosh;
    using glm::tanh;
    using glm::asin;
    using glm::acos;
    using glm::atan;
    using glm::asinh;
    using glm::acosh;
    using glm::atanh;
    using glm::abs;
    using glm::ceil;
    using glm::clamp;
    using glm::floatBitsToInt;
    using glm::floatBitsToUint;
    using glm::floor;
    using glm::fma;
    using glm::fract;
    using glm::frexp;
    using glm::intBitsToFloat;
    using glm::isinf;
    using glm::isnan;
    using glm::ldexp;
    using glm::max;
    using glm::min;
    using glm::mix;
    using glm::mod;
    using glm::modf;
    using glm::round;
    using glm::roundEven;
    using glm::sign;
    using glm::smoothstep;
    using glm::step;
    using glm::trunc;
    using glm::uintBitsToFloat;
}

#endif
