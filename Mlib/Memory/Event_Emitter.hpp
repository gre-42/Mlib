#pragma once
#include <functional>
#include <map>

namespace Mlib {

template <class... Args>
class EventEmitter;

template <class... Args>
class EventReceiverDeletionToken {
    EventReceiverDeletionToken(const EventReceiverDeletionToken&) = delete;
    EventReceiverDeletionToken& operator = (const EventReceiverDeletionToken&) = delete;
public:
    using Emitter = EventEmitter<Args...>;
    friend Emitter;
    explicit EventReceiverDeletionToken(Emitter& emitter, std::function<void(const Args&...)> f);
    ~EventReceiverDeletionToken();
    void reset();
private:
    Emitter* emitter_;
};

template <class... Args>
class EventEmitter {
public:
    using DeletionToken = EventReceiverDeletionToken<Args...>;
    friend DeletionToken;
    using EventCallback = std::function<void(const Args&...)>;
    using OnInsert = std::function<void(const EventCallback&)>;
    EventEmitter(const EventEmitter&) = delete;
    EventEmitter& operator = (const EventEmitter&) = delete;
    explicit EventEmitter(OnInsert on_insert = OnInsert());
    ~EventEmitter();
    void insert(DeletionToken& deletion_token, EventCallback f);
    void clear();
    void emit(const Args&... args);
private:
    OnInsert on_insert_;
    std::map<DeletionToken*, EventCallback> functions_;
};

}

#include "Event_Emitter.impl.hpp"
