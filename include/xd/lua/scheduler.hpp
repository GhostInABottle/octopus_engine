#ifndef H_XD_LUA_SCHEDULER
#define H_XD_LUA_SCHEDULER

#include "detail/scheduler.hpp"
#include "types.hpp"
#include "virtual_machine.hpp"
#include "scheduler_task.hpp"
#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>
#ifndef LUABIND_CPLUSPLUS_LUA
extern "C"
{
#endif
#include <lua.h>
#ifndef LUABIND_CPLUSPLUS_LUA
}
#endif
#include <luabind/tag_function.hpp>
#include <list>
#include <type_traits>

namespace xd
{
    namespace lua
    {
        // the lua scheduler, supports yielding threads
        // from both C++ and lua's side
        class scheduler : public boost::noncopyable
        {
        public:
            scheduler(virtual_machine& vm);
            virtual ~scheduler();
            lua_State *current_thread();
            void start(luabind::object func);
            void run();
            void yield(std::shared_ptr<scheduler_task> task);
            int pending_tasks();

            // a convenience function for starting a xd::lua::function<T>
            // instead of having to cast it to luabind::object yourself
            template <typename T>
            void start(function<T> func)
            {
                start(func.object());
            }

            // yields a thread; copies the passed scheduler_task
            template <typename T>
            typename std::enable_if<std::is_base_of<scheduler_task, T>::value>::type
            yield(const T& task)
            {
                yield(std::make_shared<T>(task));
            }

            // yields a shared_ptr to a task
            template <typename T>
            typename std::enable_if<std::is_base_of<scheduler_task, T>::value>::type
            yield(std::shared_ptr<T> task)
            {
                yield(static_cast<std::shared_ptr<scheduler_task>>(task));
            }

            // we also want to support plain function types as tasks
            void yield(bool (*callback)())
            {
                yield(std::make_shared<detail::callback_task>(callback));
            }

            // for function tasks that have a result
            void yield(bool (*callback)(scheduler_task_result&))
            {
                yield(std::make_shared<detail::callback_task_result>(callback));
            }

            // for class types, assume we're dealing with a function object
            // since we have to support tasks with and without result,
            // we also have to check for the functor arity, which we do by
            // proxying the call to one of the two following overloads
            template <typename F>
            typename std::enable_if<std::is_class<F>::value && !std::is_base_of<scheduler_task, F>::value>::type
            yield(F f)
            {
                yield(f, &F::operator());
            }

            // an overload for functor callbacks with no result
            // this doesn't have to be called directly; just call yield(functor)
            // and it'll proxy the call to this function
            template <typename F>
            void yield(F f, bool (F::*callback)() const)
            {
                yield(std::make_shared<detail::callback_task>(f));
            }

            // an overload for functor callbacks with a result
            // this doesn't have to be called directly; just call yield(functor)
            // and it'll proxy the call to this function
            template <typename F>
            void yield(F f, bool (F::*callback)(scheduler_task_result&) const)
            {
                yield(std::make_shared<detail::callback_task_result>(f));
            }

            template <typename Task, typename... Args>
            void yield(Args&&... args)
            {
                yield(std::shared_ptr<xd::lua::scheduler_task>(new Task(std::forward<Args>(args)...)));
            }

            // a convenience function for registering a yielding, optionally stateful,
            // C++ function to lua. By specifying module you can export the function
            // in a given lua table
            template <typename S, typename F>
            void register_function(std::string name, F f, std::string module = "")
            {
                luabind::module(m_vm.lua_state(), module.size() ? module.c_str() : 0)
                [
                    luabind::def(name.c_str(), luabind::tag_function<S>(f), luabind::yield)
                ];
            }

        private:
            virtual_machine& m_vm;
            lua_State *m_current_thread;
            detail::scheduler_task_list *m_tasks;
            detail::thread_stack *m_thread_stack;
        };
    }
}

#endif
