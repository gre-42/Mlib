#pragma once

namespace Mlib {

template <class TObject, class TLock>
class LockedGuardedObject {
    LockedGuardedObject(const LockedGuardedObject&) = delete;
    LockedGuardedObject& operator = (const LockedGuardedObject&) = delete;
public:
    template <class TMutex>
    LockedGuardedObject(TObject& object, TMutex& mutex)
        : object_{object}
        , lock_{mutex}
    {}
    TObject* operator -> () const {
        return &object_;
    }
private:
    TObject& object_;
    TLock lock_;
};

template <class TObject, class TLock>
class GeneralGuardedObject {
    GeneralGuardedObject(const GeneralGuardedObject&) = delete;
    GeneralGuardedObject& operator = (const GeneralGuardedObject&) = delete;
public:
    template <class... TArgs>
    GeneralGuardedObject(TArgs&&... args)
        : object_{ std::forward<TArgs>(args)... }
    {}
    LockedGuardedObject<TObject, TLock> lock() {
        return LockedGuardedObject<TObject, TLock>{object_, object_.mutex_};
    }
private:
    TObject object_;
};

template <class TObject, class TMutex, class TLock>
class SimpleGuardedObject {
    SimpleGuardedObject(const SimpleGuardedObject&) = delete;
    SimpleGuardedObject& operator = (const SimpleGuardedObject&) = delete;
public:
    template <class... TArgs>
    SimpleGuardedObject(TArgs&&... args)
        : object_{ std::forward<TArgs>(args)... }
    {}
    LockedGuardedObject<TObject, TLock> lock() {
        return LockedGuardedObject<TObject, TLock>{object_, mutex_};
    }
private:
    TObject object_;
    TMutex mutex_;
};

}
