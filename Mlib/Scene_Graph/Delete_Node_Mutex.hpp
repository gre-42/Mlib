#pragma once
#include <Mlib/Os/Os.hpp>
#include <atomic>
#include <mutex>
#include <sstream>
#include <thread>

namespace Mlib {

class DeleteNodeMutex {
    friend std::ostream& operator << (std::ostream& ostr, const DeleteNodeMutex& delete_node_mutex);
public:
    DeleteNodeMutex()
        : deleter_thread_id_{ std::this_thread::get_id() }
    {}
    bool this_thread_is_deleter_thread() const {
        return (std::this_thread::get_id() == deleter_thread_id_);
    }
    void assert_this_thread_is_deleter_thread() const {
        if (!this_thread_is_deleter_thread()) {
            std::stringstream sstr;
            sstr << "Deletion by wrong thread (" << std::this_thread::get_id() << " vs. " << deleter_thread_id_ << ')';
            verbose_abort(sstr.str());
        }
    }
    void set_deleter_thread() {
        if (deleter_thread_id_ != std::thread::id()) {
            verbose_abort("Deleter thread already set");
        }
        deleter_thread_id_ = std::this_thread::get_id();
    }
    void clear_deleter_thread() {
        deleter_thread_id_ = std::thread::id();
    }
private:
    std::atomic<std::thread::id> deleter_thread_id_;
};

class SetDeleterThreadGuard {
    SetDeleterThreadGuard(const SetDeleterThreadGuard&) = delete;
    SetDeleterThreadGuard& operator=(const SetDeleterThreadGuard&) = delete;
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

class DeferredSetDeleterThreadGuard {
    DeferredSetDeleterThreadGuard(const DeferredSetDeleterThreadGuard&) = delete;
    DeferredSetDeleterThreadGuard& operator=(const DeferredSetDeleterThreadGuard&) = delete;
public:
    DeferredSetDeleterThreadGuard(DeleteNodeMutex& delete_node_mutex)
        : delete_node_mutex_{delete_node_mutex}
        , is_set_{ false }
    {}
    ~DeferredSetDeleterThreadGuard() {
        delete_node_mutex_.clear_deleter_thread();
    }
    void clear_deleter_thread() {
        delete_node_mutex_.clear_deleter_thread();
    }
    void set_deleter_thread() {
        if (is_set_) {
            THROW_OR_ABORT("set_deleter_thread already called");
        }
        delete_node_mutex_.set_deleter_thread();
        is_set_ = true;
    }
    bool is_set() const {
        return is_set_;
    }
private:
    DeleteNodeMutex& delete_node_mutex_;
    bool is_set_;
};

inline std::ostream& operator << (std::ostream& ostr, const Mlib::DeleteNodeMutex& delete_node_mutex) {
    ostr << "Deleter thread ID: " << delete_node_mutex.deleter_thread_id_;
    return ostr;
}

}
