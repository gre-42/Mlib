#include "J_Thread.hpp"
#include <Mlib/Os/Os.hpp>
#include <atomic>

using namespace Mlib;

static std::atomic_uint32_t g_thread_limit = 0;
static std::atomic_uint32_t g_nthreads = 0;

void Mlib::set_thread_limit(uint32_t nthreads) {
    g_thread_limit = nthreads;
}

StopToken::StopToken()
    : stop_requested_{false}
{}

void StopToken::request_stop() {
    stop_requested_ = true;
}

bool StopToken::stop_requested() const {
    return stop_requested_;
}

JThread::JThread(std::function<void()> f) {
    check_and_increase_thread_count();
    thread_ = std::thread{std::move(f)};
}

JThread::JThread(std::function<void(const StopToken& stop_token)> f) {
    check_and_increase_thread_count();
    thread_ = std::thread{[this, f=std::move(f)](){ f(stop_token_); }};
}

JThread::~JThread() {
    request_stop();
    if (thread_.joinable()) {
        join();
    }
    --g_nthreads;
}

void JThread::check_and_increase_thread_count() const {
    if ((g_thread_limit != 0) && (g_nthreads >= g_thread_limit)) {
        throw std::runtime_error("Maximum number of threads exceeded");
    } else {
        linfo() << "Starting thread #" << g_nthreads;
    }
    ++g_nthreads;
}

StopToken& JThread::get_stop_token() {
    return stop_token_;
}

const StopToken& JThread::get_stop_token() const {
    return stop_token_;
}

void JThread::request_stop() {
    stop_token_.request_stop();
}

bool JThread::joinable() const {
    return thread_.joinable();
}

void JThread::join() {
    thread_.join();
}
