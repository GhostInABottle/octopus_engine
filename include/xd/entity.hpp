#ifndef H_XD_ENTITY
#define H_XD_ENTITY

#include "detail/entity.hpp"

#include "event_bus.hpp"
#include <boost/any.hpp>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <memory>
#include <list>
#include <map>
#include <type_traits>

namespace xd
{
    template <typename T>
    struct logic_component : detail::logic_component<T>
    {
    };

    template <typename T>
    struct render_component : detail::render_component<T>
    {
    };

    template <typename T>
    struct component : detail::component<T>
    {
    };

    class entity_placeholder
    {
    protected:
        ~entity_placeholder() {}
    };

    template <typename Class = entity_placeholder>
    class entity
    {
    public:

        // component ptr typedefs
        typedef typename std::shared_ptr<xd::component<Class>> component_ptr;
        typedef typename std::shared_ptr<xd::logic_component<Class>> logic_component_ptr;
        typedef typename std::shared_ptr<xd::render_component<Class>> render_component_ptr;

        virtual ~entity()
        {
        }

        template <typename T>
        T& get()
        {
            std::size_t hash = typeid(T).hash_code();
            auto i = m_type_to_data.find(hash);
            if (i == m_type_to_data.end())
                i = m_type_to_data.insert(std::make_pair(hash, T())).first;
            return *boost::any_cast<T>(&i->second);
        }

        template <typename T>
        T& get(const std::string& key)
        {
            auto i = m_key_to_data.find(key);
            if (i == m_key_to_data.end())
                i = m_key_to_data.insert(std::make_pair(key, T())).first;
            return *boost::any_cast<T>(&i->second);
        }

        template <typename T>
        bool has()
        {
            std::size_t hash = typeid(T).hash_code();
            return m_type_to_data.find(hash) != m_type_to_data.end();
        }

        template <typename T>
        bool has(const std::string& key)
        {
            return m_key_to_data.find(key) != m_key_to_data.end();
        }

        template <typename T>
        void on(const std::string& name, std::function<bool (const T&)> callback, std::function<bool (const T&)> filter = nullptr)
        {
            get_event_bus<T>()[name].add(callback);
        }

        template <typename T>
        void on(const std::string& name, bool (*callback)(const T&), std::function<bool (const typename detail::identity<T>::type&)> filter = nullptr)
        {
            get_event_bus<T>()[name].add(callback);
        }

        template <typename T, typename C>
        void on(const std::string& name, bool (C::*callback)(const T&), C *obj, std::function<bool (const typename detail::identity<T>::type&)> filter = nullptr)
        {
            get_event_bus<T>()[name].add(std::bind(callback, obj, std::placeholders::_1));
        }

        template <typename T, typename C>
        void on(const std::string& name, bool (C::*callback)(const T&) const, C *obj, std::function<bool (const typename detail::identity<T>::type&)> filter = nullptr)
        {
            get_event_bus<T>()[name].add(std::bind(callback, obj, std::placeholders::_1));
        }

        template <typename T>
        void on(const std::string& name, T obj)
        {
            on(name, &T::operator(), &obj);
            //get_event_bus<typename detail::first_argument<T>::type>()[name].add(obj);
        }

        template <typename T, typename U>
        void on(const std::string& name, T obj, U filter)
        {
            on(name, &T::operator(), &obj, filter);
            //get_event_bus<typename detail::first_argument<T>::type>()[name].add(obj, filter);
        }

        template <typename T>
        void trigger(const std::string& name, const T& args)
        {
            get_event_bus<T>()[name](args);
        }

        void add_component(const logic_component_ptr& component, int priority = 0)
        {
            m_components[priority].logic_components.push_back(component);
            component->init(*static_cast<Class*>(this));
        }

        void add_component(const render_component_ptr& component, int priority = 0)
        {
            m_components[priority].render_components.push_back(component);
            component->init(*static_cast<Class*>(this));
        }

        void add_component(const component_ptr& component, int priority = 0)
        {
            m_components[priority].logic_components.push_back(component);
            m_components[priority].render_components.push_back(component);
            component->init(*static_cast<Class*>(this));
        }

        void del_component(const logic_component_ptr& component, int priority)
        {
            logic_component_list_t& components = m_components[priority].logic_components;
            auto i = std::find(components.begin(), components.end(), component);
            if (i != components.end()) {
                components.erase(i);
            }
        }

        void del_component(const logic_component_ptr& component)
        {
            for (auto i = m_components.begin(); i != m_components.end(); ++i) {
                del_component(component, i->first);
            }
        }

        void del_component(const render_component_ptr& component, int priority)
        {
            logic_component_list_t& components = m_components[priority].logic_components;
            auto i = std::find(components.begin(), components.end(), component);
            if (i != components.end()) {
                components.erase(i);
            }
        }

        void del_component(const render_component_ptr& component)
        {
            for (auto i = m_components.begin(); i != m_components.end(); ++i) {
                del_component(component, i->first);
            }
        }

        void del_component(const component_ptr& component, int priority)
        {
            components_set& components = m_components[priority];
            {
                auto i = std::find(components.logic_components.begin(), components.logic_components.end(), component);
                if (i != components.logic_components.end()) {
                    components.logic_components.erase(i);
                }
            }
            {
                auto i = std::find(components.render_components.begin(), components.render_components.end(), component);
                if (i != components.render_components.end()) {
                    components.render_components.erase(i);
                }
            }
        }

        void del_component(const component_ptr& component)
        {
            for (auto i = m_components.begin(); i != m_components.end(); ++i) {
                del_component(component, i->first);
            }
        }

        void clear_components()
        {
            m_components.clear();
        }

        void update()
        {
            for (auto i = m_components.begin(); i != m_components.end(); ++i) {
                for (auto j = i->second.logic_components.begin(); j != i->second.logic_components.end(); ++j) {
                    (*j)->update(*static_cast<Class*>(this));
                }
            }
        }

        void render()
        {
            for (auto i = m_components.begin(); i != m_components.end(); ++i) {
                for (auto j = i->second.render_components.begin(); j != i->second.render_components.end(); ++j) {
                    (*j)->render(*static_cast<Class*>(this));
                }
            }
        }

    protected:
        // you should always construct entity using the derived class
        // hence constructor is protected to prevent mistakes
        entity()
        {
        }

    private:
        typedef std::list<std::shared_ptr<detail::logic_component<Class>>> logic_component_list_t;
        typedef std::list<std::shared_ptr<detail::render_component<Class>>> render_component_list_t;

        // data
        std::unordered_map<std::size_t, boost::any> m_type_to_data;
        std::unordered_map<std::string, boost::any> m_key_to_data;

        // internal struct to hold component lists per priority
        struct components_set
        {
            logic_component_list_t logic_components;
            render_component_list_t render_components;
        };

        // we have a list of logic and render components per priority
        std::map<int, components_set> m_components;

        // the bound events
        std::unordered_map<std::size_t, boost::any> m_events;

        // utility function to return event_bus for given arg type
        template <typename T>
        event_bus<T>& get_event_bus()
        {
            // calculate hash of the argument type
            std::size_t hash = typeid(T).hash_code();
            // find from the events
            auto i = m_events.find(hash);
            if (i == m_events.end()) {
                // not found, insert an empty event_bus
                m_events[hash] = event_bus<T>();
            }
            // return the event bus
            return *boost::any_cast<event_bus<T>>(&m_events[hash]);
        }
    };
}

#endif
