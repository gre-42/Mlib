namespace Mlib {

/*
 * std::vector replacement to circumvent <bool> template specialization
 */
template <class TData>
class Vector {
    TData* data_;
    size_t size_;
public:
    inline Vector(): data_(nullptr) {}
    inline explicit Vector(size_t size):
        data_(new TData[size]),
        size_(size) {}
    inline ~Vector() {
        if (data_ != nullptr) {
            delete [] data_;
        }
    }
    inline const TData& operator [] (size_t index) const {
        assert(data_ != nullptr);
        assert(index < size_);
        return data_[index];
    }
    inline TData& operator [] (size_t index) {
        const Vector& a = *this;
        return const_cast<TData&>(a[index]);
    }
    inline size_t size() const {
        return size_;
    }
};

}
