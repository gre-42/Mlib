#pragma once
#include <mutex>
#include <stdexcept>
#include <thread>

namespace Mlib {

class DeleteNodeMutex {
public:
    explicit DeleteNodeMutex()
    : deleter_thread_id_{ std::this_thread::get_id() }
    {}
    void lock() {
        mutex_.lock();
        ++nlocked_;
    }
    void unlock() {
        --nlocked_;
        mutex_.unlock();
    }
    bool is_locked() const {
        return nlocked_ > 0;
    }
    void notify_deleting() const {
        if (std::this_thread::get_id() != deleter_thread_id_) {
            throw std::runtime_error("Deletion by wrong thread");
        }
        if (!is_locked()) {
            throw std::runtime_error("Delete node mutex is not locked");
        }
    }
    void set_deleter_thread() {
        if (deleter_thread_id_ != std::thread::id()) {
            throw std::runtime_error("Deleter thread already set");
        }
        deleter_thread_id_ = std::this_thread::get_id();
    }
    void clear_deleter_thread() {
        deleter_thread_id_ = std::thread::id();
    }
private:
    std::recursive_mutex mutex_;
    unsigned int nlocked_;
    std::thread::id deleter_thread_id_;
};

class SetDeleterThreadGuard {
public:
    SetDeleterThreadGuard(DeleteNodeMutex& delete_node_mutex)
    : delete_node_mutex_{delete_node_mutex}
    {
        delete_node_mutex_.set_deleter_thread();
    }
    ~SetDeleterThreadGuard() {
        delete_node_mutex_.clear_deleter_thread();
    }
private:
    DeleteNodeMutex& delete_node_mutex_;
};

}
