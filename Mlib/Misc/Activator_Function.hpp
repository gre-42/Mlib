#pragma once
#include <functional>
#include <memory>

namespace Mlib {

class ActivationState;

class Activator {
public:
    Activator(std::weak_ptr<ActivationState> state);
    ~Activator();
    void operator () ();
private:
    std::weak_ptr<ActivationState> state_;
};

class ActivationState: public std::enable_shared_from_this<ActivationState> {
    ActivationState(const ActivationState&) = delete;
    ActivationState& operator = (const ActivationState&) = delete;
    friend Activator;
public:
    explicit ActivationState(std::function<void()> func);
    ~ActivationState();
    void notify_deactivated();
    Activator generate_activator();
private:
    void operator () ();
    std::function<void()> func_;
    bool is_activated_;
};

}
