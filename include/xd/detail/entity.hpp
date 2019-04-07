#ifndef H_XD_DETAIL_ENTITY
#define H_XD_DETAIL_ENTITY

#include "identity.hpp"

namespace xd {
    template <typename T> class entity;
}

namespace xd { namespace detail {

    template <typename T>
    class component_base
    {
    public:
        virtual ~component_base() {}
    private:
        friend xd::entity<T>;
        virtual void init(T&) {}
    };

    template <typename T>
    class logic_component : public virtual component_base<T>
    {
    private:
        friend xd::entity<T>;
        virtual void update(T&) = 0;
    };

    template <typename T>
    class render_component : public virtual component_base<T>
    {
    private:
        friend xd::entity<T>;
        virtual void render(T&) = 0;
    };

    template <typename T>
    class component : public logic_component<T>, public render_component<T>
    {
    };

} }

#endif
