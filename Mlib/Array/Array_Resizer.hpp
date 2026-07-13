#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Array_Shape.hpp>
#include <Mlib/Os/Os.hpp>
#include <functional>
#include <iostream>

namespace Mlib {

class ArrayResizer {
    std::function<void(const ArrayShape&)> resize_;
    // ArrayShape shape_;
    // bool done_;
    ArrayResizer& operator = (const ArrayResizer&) = delete;
public:
    explicit ArrayResizer(std::function<void(const ArrayShape&)> resize)
        : resize_{std::move(resize)}
        // , done_{true}
    {}
    ~ArrayResizer() {
        // if (!done_) {
        //     verbose_abort("Missing () in array.{resize|reshape}");
        // }
    }
    // ArrayResizer& operator [] (size_t size) {
    //     if (done_) {
    //         shape_.clear();
    //         done_ = false;
    //     }
    //     shape_.append(size);
    //     return *this;
    // }
    // void operator [] (const ArrayShape& shape) {
    //     if (shape.ndim() > 0) {
    //         (*this)[shape(0)];
    //         (*this)[shape.erased_first()];
    //     }
    // }
    void operator () (size_t size) {
        // (*this)[size];
        // (*this)();
        resize_(ArrayShape{size});
    }
    void operator () (size_t size0, size_t size1) {
        // (*this)[size0];
        // (*this)[size1];
        // (*this)();
        resize_(ArrayShape{size0, size1});
    }
    void operator () (size_t size0, size_t size1, size_t size2) {
        // (*this)[size0];
        // (*this)[size1];
        // (*this)[size2];
        // (*this)();
        resize_(ArrayShape{size0, size1, size2});
    }
    void operator () (const ArrayShape& shape) {
        // (*this)[shape];
        // (*this)();
        resize_(shape);
    }
    // void operator () () {
    //     resize_(shape_);
    //     done_ = true;
    // }
};

}
