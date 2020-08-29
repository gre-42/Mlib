#include <Mlib/Math/Math.hpp>

// Source: https://math.nist.gov/iml++/cg.h.txt

namespace Mlib {

//*****************************************************************
// Iterative template routine -- CG
//
// CG solves the symmetric positive definite linear
// system Ax=b using the Conjugate Gradient method.
//
// CG follows the algorithm described on p. 15 in the
// SIAM Templates book.
//
// The return value indicates convergence within max_iter (input)
// iterations , or no convergence within max_iter iterations (1).
//
// Upon successful return, output arguments have the following values:
//
//        x  --  approximate solution to Ax = b
// max_iter  --  the number of iterations performed before the
//               tolerance was reached
//      tol  --  the residual after the final iteration
//
//*****************************************************************

template < class Data, class Matrix, class Vector, class Preconditioner, class Real >
int
CG(const Matrix &A, Vector &x, const Vector &b,
   const Preconditioner &M, int &max_iter, Real &tol)
{
  Real resid;
  Vector p, z, q;
  Data alpha, beta, rho, rho_1 = NAN;

  Real normb = sum(squared(b));
  Vector r = b - dot1d(A, x);

  if (normb == 0.0)
    normb = 1;

  if ((resid = sum(squared(norm(r))) / normb) <= tol) {
    tol = resid;
    max_iter = 0;
    return 0;
  }

  for (int i = 1; i <= max_iter; i++) {
    z = M(r);
    rho = dot0d(r, z);

    if (i == 1)
      p = z;
    else {
      beta = rho / rho_1;
      p = z + beta * p;
    }

    q = dot1d(A, p);
    alpha = rho / dot0d(p, q);

    x += alpha * p;
    r -= alpha * q;

    if ((resid = sum(squared(r)) / normb) <= tol) {
      tol = resid;
      max_iter = i;
      return 0;
    }

    rho_1 = rho;
  }

  tol = resid;
  return 1;
}

template <class TMatrix, class TData, class TReal>
Array<float> cg_simple(const TMatrix& A, const Array<TData>& x0, const Array<TData>& b, int max_iter, TReal tol, bool nothrow=false)
{
    Array<TData> x = x0.copy();
    int code = CG<TData>(A, x, b, [](const Array<float>& x){ return x; }, max_iter, tol);
    if (!nothrow && code != 0) {
        throw std::runtime_error("CG did not converge");
    }
    return x;
}

}
