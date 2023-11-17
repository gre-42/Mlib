#pragma once
#include <functional>
#include <map>
#include <memory>

namespace Mlib {

class EventEmitter;

class EventReceiverDeletionToken {
    friend EventEmitter;
    EventReceiverDeletionToken(const EventReceiverDeletionToken&) = delete;
    EventReceiverDeletionToken& operator = (const EventReceiverDeletionToken&) = delete;
public:
    explicit EventReceiverDeletionToken(EventEmitter& emitter);
    ~EventReceiverDeletionToken();
private:
    EventEmitter* emitter_;
};

class EventEmitter {
    friend EventReceiverDeletionToken;
    EventEmitter(const EventEmitter&) = delete;
    EventEmitter& operator = (const EventEmitter&) = delete;
public:
    explicit EventEmitter(std::function<bool()> fire_initial_event);
    ~EventEmitter();
    std::unique_ptr<EventReceiverDeletionToken> insert(std::function<void()> f);
    void emit();
private:
    std::function<bool()> fire_initial_event_;
    std::map<EventReceiverDeletionToken*, std::function<void()>> functions_;
};

}
