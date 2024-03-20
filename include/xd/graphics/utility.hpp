#ifndef H_XD_GRAPHICS_UTILITY
#define H_XD_GRAPHICS_UTILITY

#include "../graphics/vertex_batch.hpp"
#include <utility>


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
