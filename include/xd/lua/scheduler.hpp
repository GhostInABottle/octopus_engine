#ifndef H_XD_LUA_SCHEDULER
#define H_XD_LUA_SCHEDULER

#include "../vendor/sol/sol.hpp"
#include "scheduler_task.hpp"
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>

namespace xd
{
    namespace lua
    {
        class virtual_machine;

        // the lua scheduler, supports yielding threads
        // from both C++ and lua's side
        class scheduler
        {
        public:
            scheduler(const scheduler&) = delete;
            scheduler& operator=(const scheduler&) = delete;
            scheduler(virtual_machine&  vm);
            virtual ~scheduler() = default;
            void start(const std::string& code, const std::string& context, const std::string& chunk_name = "");
            void start(const sol::protected_function& function, const std::string& context);
            void run();
            void pause() { m_paused = true; }
            void resume() { m_paused = false;  }
            bool paused() const { return m_paused; }
            void yield(std::shared_ptr<scheduler_task> task);
            int pending_tasks();

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
            void yield(bool(*callback)())
            {
                yield(std::make_shared<callback_task>(callback));
            }

            // class types
            template <typename F>
            typename std::enable_if<std::is_class<F>::value && !std::is_base_of<scheduler_task, F>::value>::type
                yield(F f)
            {
                yield(f, &F::operator());
            }

            // support for functors
            template <typename F>
            void yield(F f, bool (F::*)() const)
            {
                yield(std::make_shared<callback_task>(f));
            }

            // Forwarding arguments
            template <typename Task, typename... Args>
            void yield(Args&& ... args)
            {
                yield(std::make_shared<xd::lua::scheduler_task>(Task(std::forward<Args>(args)...)));
            }

            // a convenience function for registering a yielding, optionally stateful, C++ function to lua.
            template <typename F>
            void register_function(std::string name, F&& f)
            {
                state[name] = sol::yielding(f);
            }

        private:
            sol::state& state;
            struct scheduler_cothread {
                sol::thread thread;
                sol::coroutine coroutine;
                std::string context;
                scheduler_cothread(sol::state& state, const std::string& code, const std::string& chunk_name, const std::string& context);
                scheduler_cothread(sol::state& state, const sol::protected_function& function, const std::string& context);
            };
            struct scheduler_thread_task
            {
                std::shared_ptr<scheduler_cothread> thread;
                std::shared_ptr<scheduler_task> task;
            };
            void start(const std::shared_ptr<scheduler_cothread>& cothread);
            std::shared_ptr<scheduler_cothread> m_current_thread;
            std::stack<std::shared_ptr<scheduler_cothread>> m_thread_stack;
            std::list<scheduler_thread_task> m_tasks;
            bool m_paused;
        };
    }
}

#endif
