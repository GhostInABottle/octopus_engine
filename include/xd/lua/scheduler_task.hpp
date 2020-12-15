#ifndef H_XD_LUA_SCHEDULER_TASK
#define H_XD_LUA_SCHEDULER_TASK

#include <functional>

namespace xd
{
    namespace lua
    {
        // a base class for scheduler tasks
        class scheduler_task
        {
        public:

            virtual ~scheduler_task() {}
            virtual bool is_complete() = 0;
        };

        // callback_task is a convenience scheduler task
        // that allows user to bind functions, function objects,
        // lambdas, etc. as tasks to the scheduler
        class callback_task : public scheduler_task
        {
        public:
            callback_task(std::function<bool()> callback)
                : m_callback(callback)
            {
            }

            bool is_complete() override
            {
                return m_callback();
            }

        private:
            std::function<bool()> m_callback;
        };
    }
}

#endif
