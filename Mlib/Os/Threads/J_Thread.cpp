#include "J_Thread.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Os/Threads/Thread_Initializer.hpp>
#include <atomic>

using namespace Mlib;

static std::atomic_uint32_t g_thread_limit = 0;
static std::atomic_uint32_t g_nthreads = 0;

void Mlib::set_thread_limit(uint32_t nthreads) {
    g_thread_limit = nthreads;
}

StopToken::StopToken()
    : stop_requested_{std::make_shared<std::atomic_bool>(false)}
{}

void StopToken::request_stop() {
    *stop_requested_ = true;
}

bool StopToken::stop_requested() const {
    return *stop_requested_;
}

JThread::JThread(
    std::string name,
    ThreadAffinity affinity,
    std::function<void()> f)
    : name_{ std::move(name) }
{
    thread_number_ = check_and_increase_thread_count();
    linfo() << "Starting thread #" << thread_number_ << ": \"" << name_ << '"';
    thread_ = std::thread{[name=name_, affinity, f=std::move(f)](){
        ThreadInitializer ti{std::move(name), affinity};
        f();
    }};
}

JThread::JThread(
    std::string name,
    ThreadAffinity affinity,
    std::function<void(StopToken stop_token)> f)
    : name_{ std::move(name) }
{
    thread_number_ = check_and_increase_thread_count();
    linfo() << "Starting thread #" << thread_number_ << ": \"" << name_ << '"';
    thread_ = std::thread{[st=stop_token_, name=name_, affinity, f=std::move(f)](){
        ThreadInitializer ti{std::move(name), affinity};
        f(std::move(st));
    }};
}

JThread::~JThread() {
    request_stop();
    if (thread_.joinable()) {
        join();
    }
    g_nthreads.fetch_sub(1, std::memory_order_relaxed);
}

uint32_t JThread::check_and_increase_thread_count() const {
    uint32_t current = g_nthreads.load(std::memory_order_relaxed);
    uint32_t limit = g_thread_limit.load(std::memory_order_relaxed);

    while (true) {
        if ((limit != 0) && (current >= limit)) {
            throw std::runtime_error("Maximum number of threads exceeded");
        }
        if (g_nthreads.compare_exchange_weak(current, current + 1, 
                                             std::memory_order_relaxed, 
                                             std::memory_order_relaxed)) {
            break; 
        }
        // If exchange failed, 'current' is updated automatically with the new actual value
    }
    return current;
}

StopToken JThread::get_stop_token() {
    return stop_token_;
}

const StopToken JThread::get_stop_token() const {
    return stop_token_;
}

void JThread::request_stop() {
    stop_token_.request_stop();
}

bool JThread::joinable() const {
    return thread_.joinable();
}

void JThread::join() {
    linfo() << "Joining thread #" << thread_number_ << ": \"" << name_ << '"';
    thread_.join();
    linfo() << "Joined thread #" << thread_number_ << ": \"" << name_ << '"';
}
