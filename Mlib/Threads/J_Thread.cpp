#include "J_Thread.hpp"

using namespace Mlib;

StopToken::StopToken()
: stop_requested_{false}
{}

void StopToken::request_stop() {
    stop_requested_ = true;
}

bool StopToken::stop_requested() const {
    return stop_requested_;
}

JThread::JThread(const std::function<void()>& f)
: thread_{f}
{}

JThread::~JThread() {
    request_stop();
    if (thread_.joinable()) {
        join();
    }
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
