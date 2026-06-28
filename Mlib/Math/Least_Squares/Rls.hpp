#pragma once
#include <iosfwd>

namespace Mlib {

// Recursive least squares
template <class TMatrix, class TVector, class TData>
class Rls {
public:
    explicit Rls(const TMatrix& P0, const TVector& b, const TData& alpha)
        : P_(P0)
        , b_(b)
        , alpha_{alpha}
    {}
    ~Rls() = default;
    void regularize(size_t i, const TData& lambda) {
        P_(i, i) = std::min(P_(i, i), 1 / lambda);
    }
    void update(const TVector& z, const TData& y) {
        // Gain vector
        auto K = dot1d(P_, z) / (alpha_ + dot0d(z, dot1d(P_, z)));
        b_ += K * (y - dot0d(z, b_));
        P_ = (1 / alpha_) * (P_ - dot2d(K.columns_as_1D(), dot(z, P_).rows_as_1D()));
    }
    const TVector& b() const {
        return b_;
    }
    const TData& b(size_t i) const {
        return b_(i);
    }
    void print(std::ostream& ostr) const {
        ostr << "P:\n" << P_ << "\nb: " << b_;
    }
private:
    TMatrix P_; // Covariance
    TVector b_; // Parameters
    TData alpha_;
};

template <class TMatrix, class TVector, class TData>
std::ostream& operator << (std::ostream& ostr, const Rls<TMatrix, TVector, TData>& rls) {
    rls.print(ostr);
    return ostr;
}

}
