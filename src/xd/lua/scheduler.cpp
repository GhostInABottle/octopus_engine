#include "../../../include/xd/lua/scheduler.hpp"
#include "../../../include/xd/lua/scheduler_task.hpp"
#include "../../../include/xd/lua/virtual_machine.hpp"
#include "../../../include/xd/lua/exceptions.hpp"

xd::lua::scheduler::scheduler(virtual_machine& vm)
    : state(vm.lua_state())
    , m_current_thread(0)
{
}

void xd::lua::scheduler::start(const std::string& code_or_filename, bool is_file)
{
    // set the current thread
    m_thread_stack.push(std::make_shared<scheduler_cothread>(state, code_or_filename, is_file));
    m_current_thread = m_thread_stack.top();
    // start the thread
    auto result = m_current_thread->coroutine();
    if (!result.valid()) {
        sol::error err = result;
        throw panic_error(err.what());
    }
    // reset current thread
    m_thread_stack.pop();
    if (m_thread_stack.empty())
        m_current_thread = nullptr;
    else
        m_current_thread = m_thread_stack.top();
}

void xd::lua::scheduler::run()
{
    // iterate through all pending tasks
    for (auto i = m_tasks.begin(); i != m_tasks.end();) {
        // if the thread is dead, remove it
        if (i->thread->coroutine.status() != sol::call_status::yielded) {
            i = m_tasks.erase(i);
            continue;
        }

        // is this task complete?
        if (i->task->is_complete()) {
            // get the thread task
            scheduler_thread_task thread_task = *i;
            // remove it from the pending list
            i = m_tasks.erase(i);
            // set the current thread
            m_thread_stack.push(thread_task.thread);
            m_current_thread = thread_task.thread;
            // resume the thread
            auto result = thread_task.thread->coroutine();
            if (!result.valid()) {
                sol::error err = result;
                throw panic_error(err.what());
            }
            // reset current thread
            m_thread_stack.pop();
            if (m_thread_stack.empty())
                m_current_thread = nullptr;
            else
                m_current_thread = m_thread_stack.top();
        }
        else {
            ++i;
        }
    }
}

void xd::lua::scheduler::yield(std::shared_ptr<scheduler_task> task)
{
    scheduler_thread_task thread_task;
    thread_task.thread = m_current_thread;
    thread_task.task = task;
    m_tasks.push_back(thread_task);
}

int xd::lua::scheduler::pending_tasks()
{
    return m_tasks.size();
}

xd::lua::scheduler::scheduler_cothread::scheduler_cothread(sol::state& state, const std::string& code, bool is_file) {
    thread = sol::thread::create(state);
    auto load_result = is_file ? thread.state().load_file(code) : thread.state().load(code);
    if (!load_result.valid()) {
        sol::error err = load_result;
        throw panic_error(err.what());
    }
    coroutine = load_result;
}
