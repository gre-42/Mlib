#pragma once
#include <condition_variable>
#include <mutex>

namespace Mlib {

// Usage:
// Barrier barrier;
// BarrierParticipant participant{barrier};
// while (true) {
//   {
//     ParallelBlock section{block};
//     // Parallel code
//   }
//   // Sequential code
// }
class Barrier {
    Barrier(const Barrier&) = delete;
    Barrier& operator = (const Barrier&) = delete;
public:
    inline explicit Barrier(std::chrono::milliseconds wait_time)
    : started_{false},
      finished_{false},
      nparticipants_{0},
      nwaiting_for_start_{0},
      nwaiting_for_finish_{0},
      wait_time_{wait_time}
    {}
    inline ~Barrier() = default;
    inline void start() {
        // From: https://stackoverflow.com/questions/28916238/create-a-barrier-for-each-time-c
        std::unique_lock lock(start_mutex_);
        ++nwaiting_for_start_;
        if (nwaiting_for_start_ == nparticipants_) {
            start_cv_.notify_all();
        } else {
            start_cv_.wait(lock, [this]{ return started_ || (nwaiting_for_start_ == nparticipants_); });
        }
        if (nwaiting_for_start_ == nparticipants_) {
            started_ = true;
        }
        --nwaiting_for_start_;
        if (nwaiting_for_start_ == 0) {
            started_ = false;
        }
    }
    inline void finish() {
        // From: https://stackoverflow.com/questions/28916238/create-a-barrier-for-each-time-c
        std::unique_lock lock(finish_mutex_);
        ++nwaiting_for_finish_;
        if (nwaiting_for_finish_ == nparticipants_) {
            finish_cv_.notify_all();
        } else {
            finish_cv_.wait_for(lock, wait_time_, [this]{ return finished_ || (nwaiting_for_finish_ == nparticipants_); });
        }
        if (nwaiting_for_finish_ == nparticipants_) {
            finished_ = true;
        }
        --nwaiting_for_finish_;
        if (nwaiting_for_finish_ == 0) {
            finished_ = false;
        }
    }
    inline void register_participant() {
        {
            std::scoped_lock lock{start_mutex_, finish_mutex_};
            ++nparticipants_;
        }
        start_cv_.notify_all();
        finish_cv_.notify_all();
    }
    inline void deregister_participant() {
        {
            std::scoped_lock lock{start_mutex_, finish_mutex_};
            --nparticipants_;
        }
        start_cv_.notify_all();
        finish_cv_.notify_all();
    }
private:
    bool started_;
    bool finished_;
    unsigned int nparticipants_;
    unsigned int nwaiting_for_start_;
    unsigned int nwaiting_for_finish_;
    std::mutex start_mutex_;
    std::condition_variable start_cv_;
    std::mutex finish_mutex_;
    std::condition_variable finish_cv_;
    std::chrono::milliseconds wait_time_;
};

class BarrierParticipant {
    BarrierParticipant(const BarrierParticipant&) = delete;
    BarrierParticipant& operator = (const BarrierParticipant&) = delete;
public:
    inline BarrierParticipant(Barrier& barrier)
    : barrier_{barrier}
    {
        barrier_.register_participant();
    }
    inline ~BarrierParticipant() {
        barrier_.deregister_participant();
    }
private:
    Barrier& barrier_;
};

class ParallelBlock {
    ParallelBlock(const ParallelBlock&) = delete;
    ParallelBlock& operator = (const ParallelBlock&) = delete;
public:
    explicit inline ParallelBlock(Barrier& barrier)
    : barrier_{barrier} {
        barrier_.start();
    }
    inline ~ParallelBlock() {
        barrier_.finish();
    }
private:
    Barrier& barrier_;
};

}
