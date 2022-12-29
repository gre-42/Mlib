#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <atomic>
#include <mutex>
#include <sstream>
#include <thread>

namespace Mlib {

class DeleteNodeMutex {
    friend std::ostream& operator << (std::ostream& ostr, const DeleteNodeMutex& delete_node_mutex);
public:
    explicit DeleteNodeMutex()
    : nlocked_{ 0 },
      deleter_thread_id_{ std::this_thread::get_id() },
      deletion_lock_holder_{ std::thread::id() }
    {}
    void lock() {
        mutex_.lock();
        if (deletion_lock_holder_ != std::thread::id()) {
            if (deletion_lock_holder_ != std::this_thread::get_id()) {
                THROW_OR_ABORT("Deletion lock already held by another thread");
            }
        } else {
            deletion_lock_holder_ = std::this_thread::get_id();
        }
        ++nlocked_;
    }
    void unlock() {
        if (nlocked_ == 0) {
            THROW_OR_ABORT("DeleteNodeMutex already unlocked");
        }
        --nlocked_;
        if (nlocked_ == 0) {
            deletion_lock_holder_ = std::thread::id();
        }
        mutex_.unlock();
    }
    bool is_locked_by_this_thread() const {
        return (deletion_lock_holder_ == std::this_thread::get_id());
    }
    bool this_thread_is_deleter_thread() const {
        return (std::this_thread::get_id() == deleter_thread_id_);
    }
    void assert_this_thread_is_deleter_thread() const {
        if (!this_thread_is_deleter_thread()) {
            std::stringstream sstr;
            sstr << "Deletion by wrong thread (" << std::this_thread::get_id() << " vs. " << deleter_thread_id_ << ')';
            THROW_OR_ABORT(sstr.str());
        }
    }
    void notify_deleting() const {
        assert_this_thread_is_deleter_thread();
        if (!is_locked_by_this_thread()) {
            THROW_OR_ABORT("Delete node mutex is not locked by this thread");
        }
    }
    void notify_reading() const {
        if (!is_locked_by_this_thread() && !this_thread_is_deleter_thread()) {
            THROW_OR_ABORT("Reading without locking on non-deleter-thread");
        }
    }
    void set_deleter_thread() {
        if (deleter_thread_id_ != std::thread::id()) {
            THROW_OR_ABORT("Deleter thread already set");
        }
        deleter_thread_id_ = std::this_thread::get_id();
    }
    void clear_deleter_thread() {
        deleter_thread_id_ = std::thread::id();
    }
private:
    std::recursive_mutex mutex_;
    unsigned int nlocked_;
    std::atomic<std::thread::id> deleter_thread_id_;
    std::atomic<std::thread::id> deletion_lock_holder_;
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

inline std::ostream& operator << (std::ostream& ostr, const Mlib::DeleteNodeMutex& delete_node_mutex) {
    ostr << "Deleter thread ID: " << delete_node_mutex.deleter_thread_id_;
    return ostr;
}

}
