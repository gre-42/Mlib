#pragma once
#include <functional>
#include <map>
#include <memory>

namespace Mlib {

class EventEmitter;

class EventReceiverDeletionToken {
    EventReceiverDeletionToken(const EventReceiverDeletionToken&) = delete;
    EventReceiverDeletionToken& operator = (const EventReceiverDeletionToken&) = delete;
public:
    explicit EventReceiverDeletionToken(EventEmitter* emitter);
    ~EventReceiverDeletionToken();
private:
    EventEmitter* emitter_;
};

class EventEmitter {
    friend EventReceiverDeletionToken;
public:
    std::unique_ptr<EventReceiverDeletionToken> insert(std::function<void()> f);
    void emit();
private:
    std::map<void*, std::function<void()>> functions_;
};

}
