#ifndef H_XD_GRAPHICS_UTILITY
#define H_XD_GRAPHICS_UTILITY

#include "../graphics.hpp"
#include <type_traits>


namespace xd
{
    template <typename V, typename S, typename... Args>
    void render(vertex_batch<V>& batch, S& shader, Args&&... args)
    {
        shader.setup(std::forward<Args>(args)...);
        batch.render();
    }
}

#endif
