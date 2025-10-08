#include "Thread_Top.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Threads/Get_Thread_Name.hpp>
#include <list>
#include <map>
#include <mutex>
#include <sstream>

using namespace Mlib;

static FastMutex mutex;
static std::map<ThreadIdentifier, std::list<std::string>> tasks;

FunctionGuard::FunctionGuard(std::string task_name)
    : id{ get_thread_name(), std::this_thread::get_id() }
{
    std::scoped_lock lock{ mutex };
    task_name_ = &tasks[id].emplace_back(std::move(task_name));
}

FunctionGuard::~FunctionGuard()
{
    std::scoped_lock lock{ mutex };
    auto it = tasks.find(id);
    if (it == tasks.end()) {
        verbose_abort("Could not find task");
    }
    it->second.pop_back();
    if (it->second.empty()) {
        tasks.erase(it);
    }
}

void FunctionGuard::update(std::string task_name) {
    std::scoped_lock lock{ mutex };
    *task_name_ = std::move(task_name);
}

std::string Mlib::thread_top() {
    std::stringstream sstr;
    std::scoped_lock lock{ mutex };
    for (const auto& [id, ts] : tasks) {
        sstr << "Thread name: " << id.name << ", ID: " << id.id << '\n';
        for (const auto& t : ts) {
            if (!t.empty()) {
                sstr << "    " << t << '\n';
            }
        }
    }
    return sstr.str();
}
