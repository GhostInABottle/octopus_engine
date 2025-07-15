#include "scheduler.hpp"
#include "scheduler_task.hpp"
#include "virtual_machine.hpp"
#include "exceptions.hpp"

xd::lua::scheduler::scheduler(virtual_machine& vm)
    : state(vm.lua_state())
    , m_current_thread(0)
    , m_paused(false)
{
}

void xd::lua::scheduler::start(const std::string& code, const std::string& context, const std::string& chunk_name)
{
    start(std::make_shared<scheduler_cothread>(state, code, chunk_name, context));
}

void xd::lua::scheduler::start(const sol::protected_function& function, const std::string& context) {
    start(std::make_shared<scheduler_cothread>(state, function, context));
}

void xd::lua::scheduler::run()
{
    if (m_paused) return;

    // iterate through all pending tasks
    for (auto i = m_tasks.begin(); i != m_tasks.end();) {
        // Stop processing threads if paused
        if (m_paused) break;

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
            m_current_thread->thread.state().globals()["SCRIPT_CONTEXT"] = m_current_thread->context;
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

void xd::lua::scheduler::start(const std::shared_ptr<xd::lua::scheduler::scheduler_cothread>& cothread) {
    // set the current thread
    m_thread_stack.push(cothread);
    m_current_thread = m_thread_stack.top();
    m_current_thread->thread.state().globals()["SCRIPT_CONTEXT"] = m_current_thread->context;
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

xd::lua::scheduler::scheduler_cothread::scheduler_cothread(sol::state& state, const std::string& code,
        const std::string& chunk_name, const std::string& context) {
    thread = sol::thread::create(state);
    auto load_result = thread.state().load(code, chunk_name);
    if (!load_result.valid()) {
        sol::error err = load_result;
        throw panic_error(err.what());
    }
    coroutine = load_result;
    this->context = context;
}

xd::lua::scheduler::scheduler_cothread::scheduler_cothread(sol::state& state, const sol::protected_function& function, const std::string& context) {
    thread = sol::thread::create(state);
    coroutine = { thread.state(), function };
    this->context = context;
}
