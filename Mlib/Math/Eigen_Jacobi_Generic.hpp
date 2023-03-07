#pragma once

/// @file     jacobi.hpp
/// @brief    Calculate the eigenvalues and eigevectors of a symmetric matrix
///           using the Jacobi eigenvalue algorithm.
/// @author   Andrew Jewett
/// @license  CC0-1.0

#include <algorithm>
#include <cmath>
//#include <cassert>

namespace jacobi_pd {

/// @class Jacobi
/// @brief Calculate the eigenvalues and eigevectors of a symmetric matrix
///        using the Jacobi eigenvalue algorithm.
/// @note  The "Vector" and "Matrix" type arguments can be any 
///        C or C++ object that support indexing, including pointers or vectors.

template<typename Scalar,
    typename Vector,
    typename Matrix,
    typename ConstMatrix,
    typename IndexVector>
    class Jacobi
{
    int n;            //!< the size of the matrix
    Matrix M;         //!< local copy of the matrix being analyzed
    // Precomputed cosine, sine, and tangent of the most recent rotation angle:
    Scalar c;         //!< = cos(?)
    Scalar s;         //!< = sin(?)
    Scalar t;         //!< = tan(?),  (note |t|<=1)
    IndexVector max_idx_row; //!< for row i, the index j of the maximum element where j>i

public:

    /// @brief  Specify the size of the matrices you want to diagonalize later.
    /// @param n  the size (ie. number of rows) of the square matrix
    void SetSize(int n);

    Jacobi(int n = 0) {
        Init();
        SetSize(n);
    }

    ~Jacobi() {
        Dealloc();
    }

    // @typedef choose the criteria for sorting eigenvalues and eigenvectors
    typedef enum eSortCriteria {
        DO_NOT_SORT,
        SORT_DECREASING_EVALS,
        SORT_INCREASING_EVALS,
        SORT_DECREASING_ABS_EVALS,
        SORT_INCREASING_ABS_EVALS
    } SortCriteria;

    /// @brief Calculate all the eigenvalues and eigevectors of a symmetric matrix
    ///        using the Jacobi eigenvalue algorithm:
    ///        https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
    /// @returns The number of Jacobi rotations attempted if successful (>0),
    ///          and 0 otherwise.  (0 indicates that convergence failed.)
    /// @note  To reduce the computation time further, set calc_evecs=false.
    int
        Diagonalize(ConstMatrix mat, //!< the matrix you wish to diagonalize (size n)
            Vector eval,   //!< store the eigenvalues here
            Matrix evec,   //!< store the eigenvectors here (in rows)
            SortCriteria sort_criteria = SORT_DECREASING_EVALS,//!<sort results?
            bool calc_evecs = true,      //!< calculate the eigenvectors?
            int max_num_sweeps = 50);  //!< limit the number of iterations

private:

    /// @brief Calculate the components of a rotation matrix which performs a
    ///        rotation in the i,j plane by an angle (?) causing M(i, j)=0.
    ///        The resulting parameters will be stored in this->c, this->s, and
    ///        this->t (which store cos(?), sin(?), and tan(?), respectively).
    void CalcRot(const Matrix& M,   //!< matrix
        int i,      //!< row index
        int j);     //!< column index

/// @brief Apply the (previously calculated) rotation matrix to matrix M
///        by multiplying it on both sides (a "similarity transform").
///        (To save time, only the elements in the upper-right-triangular
///         region of the matrix are updated.  It is assumed that i < j.)
    void ApplyRot(Matrix M,  //!< matrix
        int i,     //!< row index
        int j);    //!< column index

/// @brief Multiply matrix E on the left by the (previously calculated)
///        rotation matrix.
    void ApplyRotLeft(Matrix E,  //!< matrix
        int i,     //!< row index
        int j);    //!< column index

///@brief Find the off-diagonal index in row i whose absolute value is largest
    int MaxEntryRow(const Matrix& M, int i) const;

    /// @brief Find the indices (i_max, j_max) marking the location of the
    ///        entry in the matrix with the largest absolute value.  This
    ///        uses the max_idx_row[] array to find the answer in O(n) time.
    /// @returns This function does not return a value.  However after it is
    ///          invoked, the location of the largest matrix element will be
    ///          stored in the i_max and j_max arguments.
    void MaxEntry(const Matrix& M, int& i_max, int& j_max) const;

    // @brief Sort the rows in M according to the numbers in v (also sorted)
    void SortRows(Vector v, //!< vector containing the keys used for sorting
        Matrix M, //!< matrix whose rows will be sorted according to v
        int n,    //!< size of the vector and matrix
        SortCriteria s = SORT_DECREASING_EVALS //!< sort decreasing order?
    ) const;

    // memory management:
    void Alloc(int N);
    void Init();
    void Dealloc();

public:
    // memory management: copy and move constructor, swap, and assignment operator
    Jacobi(const Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>& source);
    Jacobi(Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>&& other);
    void swap(Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>& other);
    Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>& operator = (Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector> source);

}; // class Jacobi





// -------------- IMPLEMENTATION --------------



template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
int Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Diagonalize(ConstMatrix mat,    // the matrix you wish to diagonalize (size n)
        Vector eval,        // store the eigenvalues here
        Matrix evec,        // store the eigenvectors here (in rows)
        SortCriteria sort_criteria, // sort results?
        bool calc_evec,     // calculate the eigenvectors?
        int max_num_sweeps) // limit the number of iterations ("sweeps")
{
    // -- Initialization --
    for (int i = 0; i < n; i++)
        for (int j = i; j < n; j++)          //copy mat[][] into M[][]
            M((size_t)i, (size_t)j) = mat((size_t)i, (size_t)j);               //(M[][] is a local copy we can modify)

    if (calc_evec)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                evec((size_t)i, (size_t)j) = (i == j) ? 1.0 : 0.0; //Set evec equal to the identity matrix

    for (int i = 0; i < n - 1; i++)          //Initialize the "max_idx_row[]" array 
        max_idx_row((size_t)i) = MaxEntryRow(M, i);  //(which is needed by MaxEntry())

        // -- Iteration --
    int n_iters;
    int max_num_iters = max_num_sweeps * n * (n - 1) / 2; //"sweep" = n*(n-1)/2 iters
    for (n_iters = 1; n_iters <= max_num_iters; n_iters++) {
        int i, j;
        MaxEntry(M, i, j); // Find the maximum entry in the matrix. Store in i,j

        // If M(i, j) is small compared to M(i, i) and M(j, j), set it to 0.
        if ((M((size_t)i, (size_t)i) + M((size_t)i, (size_t)j) == M((size_t)i, (size_t)i)) && (M((size_t)j, (size_t)j) + M((size_t)i, (size_t)j) == M((size_t)j, (size_t)j))) {
            M((size_t)i, (size_t)j) = 0.0;
            max_idx_row((size_t)i) = MaxEntryRow(M, i); //must also update max_idx_row(i)
        }

        if (M((size_t)i, (size_t)j) == 0.0)
            break;

        // Otherwise, apply a rotation to make M(i, j) = 0
        CalcRot(M, i, j);  // Calculate the parameters of the rotation matrix.
        ApplyRot(M, i, j); // Apply this rotation to the M matrix.
        if (calc_evec)     // Optional: If the caller wants the eigenvectors, then
            ApplyRotLeft(evec, i, j); // apply the rotation to the eigenvector matrix

    } //for (int n_iters=0; n_iters < max_num_iters; n_iters++)

    // -- Post-processing --
    for (int i = 0; i < n; i++)
        eval((size_t)i) = M((size_t)i, (size_t)i);

    // Optional: Sort results by eigenvalue.
    SortRows(eval, evec, n, sort_criteria);

    if ((n_iters > max_num_iters) && (n > 1))   // If we exceeded max_num_iters,
        return 0;                               // indicate an error occured.

        //assert(n_iters > 0);
    return n_iters;
}


/// @brief Calculate the components of a rotation matrix which performs a
///        rotation in the i,j plane by an angle (?) that (when multiplied on
///        both sides) will zero the ij'th element of M, so that afterwards
///        M(i, j) = 0.  The results will be stored in c, s, and t
///        (which store cos(?), sin(?), and tan(?), respectively).

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    CalcRot(const Matrix& M,    // matrix
        int i,       // row index
        int j)       // column index
{
    t = 1.0; // = tan(?)
    Scalar M_jj_ii = (M((size_t)j, (size_t)j) - M((size_t)i, (size_t)i));
    if (M_jj_ii != 0.0) {
        // kappa = (M(j, j) - M(i, i)) / (2*M(i, j))
        Scalar kappa = M_jj_ii;
        t = 0.0;
        Scalar M_ij = M((size_t)i, (size_t)j);
        if (M_ij != 0.0) {
            kappa /= (2.0 * M_ij);
            // t satisfies: t^2 + 2*t*kappa - 1 = 0
            // (choose the root which has the smaller absolute value)
            t = 1.0 / (std::sqrt(1 + kappa * kappa) + std::abs(kappa));
            if (kappa < 0.0)
                t = -t;
        }
    }
    //assert(std::abs(t) <= 1.0);
    c = 1.0 / std::sqrt(1 + t * t);
    s = c * t;
}

/// brief  Perform a similarity transformation by multiplying matrix M on both
///         sides by a rotation matrix (and its transpose) to eliminate M(i, j).
/// details This rotation matrix performs a rotation in the i,j plane by
///         angle ?.  This function assumes that c=cos(?). s=som(?), t=tan(?)
///         have been calculated in advance (using the CalcRot() function).
///         It also assumes that i<j.  The max_idx_row[] array is also updated.
///         To save time, since the matrix is symmetric, the elements
///         below the diagonal (ie. M(u, v) where u>v) are not computed.
/// verbatim
///   M' = R^T * M * R
/// where R the rotation in the i,j plane and ^T denotes the transpose.
///                 i         j
///       _                             _
///      |  1                            | 
///      |    .                          |
///      |      .                        |
///      |        1                      |
///      |          c   ...   s          |
///      |          .  .      .          |
/// R  = |          .    1    .          |
///      |          .      .  .          |
///      |          -s  ...   c          |
///      |                      1        |
///      |                        .      |
///      |                          .    |
///      |_                           1 _|
/// endverbatim
///
/// Let M' denote the matrix M after multiplication by R^T and R.
/// The components of M' are:
///
/// verbatim
///   M'_uv =  ?_w  ?_z   R_wu * M_wz * R_zv
/// endverbatim
///
/// Note that a the rotation at location i,j will modify all of the matrix
/// elements containing at least one index which is either i or j
/// such as: M(w, i), M(i, w), M(w, j), M(j, w).
/// Check and see whether these modified matrix elements exceed the 
/// corresponding values in max_idx_row[] array for that row.
/// If so, then update max_idx_row for that row.
/// This is somewhat complicated by the fact that we must only consider
/// matrix elements in the upper-right triangle strictly above the diagonal.
/// (ie. matrix elements whose second index is > the first index).
/// The modified elements we must consider are marked with an "X" below:
///
/// @verbatim
///                 i         j
///       _                             _
///      |  .       X         X          | 
///      |    .     X         X          |
///      |      .   X         X          |
///      |        . X         X          |
///      |          X X X X X 0 X X X X  |  i
///      |            .       X          |
///      |              .     X          |
/// M  = |                .   X          |
///      |                  . X          |
///      |                    X X X X X  |  j
///      |                      .        |
///      |                        .      |
///      |                          .    |
///      |_                           . _|
/// @endverbatim

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    ApplyRot(Matrix M,  // matrix
        int i,     // row index
        int j)     // column index
{
    // Recall that:
    // c = cos(?)
    // s = sin(?)
    // t = tan(?) (which should be <= 1.0)

    // Compute the diagonal elements of M which have changed:
    M((size_t)i, (size_t)i) -= t * M((size_t)i, (size_t)j);
    M((size_t)j, (size_t)j) += t * M((size_t)i, (size_t)j);
    // Note: This is algebraically equivalent to:
    // M(i, i) = c*c*M(i, i) + s*s*M(j, j) - 2*s*c*M(i, j)
    // M(j, j) = s*s*M(i, i) + c*c*M(j, j) + 2*s*c*M(i, j)

    //Update the off-diagonal elements of M which will change (above the diagonal)

    //assert(i < j);

    M((size_t)i, (size_t)j) = 0.0;

    //compute M(w, i) and M(i, w) for all w!=i,considering above-diagonal elements
    for (int w = 0; w < i; w++) {        // 0 <= w <  i  <  j < n
        M((size_t)i, (size_t)w) = M((size_t)w, (size_t)i); //backup the previous value. store below diagonal (i>w)
        M((size_t)w, (size_t)i) = c * M((size_t)w, (size_t)i) - s * M((size_t)w, (size_t)j); //M(w, i), M(w, j) from previous iteration
        if (i == max_idx_row((size_t)w)) max_idx_row((size_t)w) = MaxEntryRow(M, w);
        else if (std::abs(M((size_t)w, (size_t)i)) > std::abs(M((size_t)w, (size_t)max_idx_row((size_t)w)))) max_idx_row((size_t)w) = i;
        //assert(max_idx_row(w) == MaxEntryRow(M, w));
    }
    for (int w = i + 1; w < j; w++) {      // 0 <= i <  w  <  j < n
        M((size_t)w, (size_t)i) = M((size_t)i, (size_t)w); //backup the previous value. store below diagonal (w>i)
        M((size_t)i, (size_t)w) = c * M((size_t)i, (size_t)w) - s * M((size_t)w, (size_t)j); //M(i, w), M(w, j) from previous iteration
    }
    for (int w = j + 1; w < n; w++) {      // 0 <= i < j+1 <= w < n
        M((size_t)w, (size_t)i) = M((size_t)i, (size_t)w); //backup the previous value. store below diagonal (w>i)
        M((size_t)i, (size_t)w) = c * M((size_t)i, (size_t)w) - s * M((size_t)j, (size_t)w); //M(i, w), M(j, w) from previous iteration
    }

    // now that we're done modifying row i, we can update max_idx_row(i)
    max_idx_row((size_t)i) = MaxEntryRow(M, i);

    //compute M(w, j) and M(j, w) for all w!=j,considering above-diagonal elements
    for (int w = 0; w < i; w++) {        // 0 <=  w  <  i <  j < n
        M((size_t)w, (size_t)j) = s * M((size_t)i, (size_t)w) + c * M((size_t)w, (size_t)j); //M(i, w), M(w, j) from previous iteration
        if (j == max_idx_row((size_t)w)) max_idx_row((size_t)w) = MaxEntryRow(M, w);
        else if (std::abs(M((size_t)w, (size_t)j)) > std::abs(M((size_t)w, (size_t)max_idx_row((size_t)w)))) max_idx_row((size_t)w) = j;
        //assert(max_idx_row(w) == MaxEntryRow(M, w));
    }
    for (int w = i + 1; w < j; w++) {      // 0 <= i+1 <= w <  j < n
        M((size_t)w, (size_t)j) = s * M((size_t)w, (size_t)i) + c * M((size_t)w, (size_t)j); //M(w, i), M(w, j) from previous iteration
        if (j == max_idx_row((size_t)w)) max_idx_row((size_t)w) = MaxEntryRow(M, w);
        else if (std::abs(M((size_t)w, (size_t)j)) > std::abs(M((size_t)w, (size_t)max_idx_row((size_t)w)))) max_idx_row((size_t)w) = j;
        //assert(max_idx_row(w) == MaxEntryRow(M, w));
    }
    for (int w = j + 1; w < n; w++) {      // 0 <=  i  <  j <  w < n
        M((size_t)j, (size_t)w) = s * M((size_t)w, (size_t)i) + c * M((size_t)j, (size_t)w); //M(w, i), M(j, w) from previous iteration
    }
    // now that we're done modifying row j, we can update max_idx_row(j)
    max_idx_row((size_t)j) = MaxEntryRow(M, j);

} //Jacobi::ApplyRot()




///@brief Multiply matrix M on the LEFT side by a transposed rotation matrix R^T
///       This matrix performs a rotation in the i,j plane by angle ?  (where
///       the arguments "s" and "c" refer to cos(?) and sin(?), respectively).
/// @verbatim
///   E'_uv = ?_w  R_wu * E_wv
/// @endverbatim

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    ApplyRotLeft(Matrix E,  // matrix
        int i,     // row index
        int j)     // column index
{
    // Recall that c = cos(?) and s = sin(?)
    for (int v = 0; v < n; v++) {
        Scalar Eiv = E((size_t)i, (size_t)v); //backup E(i, v)
        E((size_t)i, (size_t)v) = c * E((size_t)i, (size_t)v) - s * E((size_t)j, (size_t)v);
        E((size_t)j, (size_t)v) = s * Eiv + c * E((size_t)j, (size_t)v);
    }
}



template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
int Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    MaxEntryRow(const Matrix& M, int i) const {
    int j_max = i + 1;
    for (int j = i + 2; j < n; j++)
        if (std::abs(M((size_t)i, (size_t)j)) > std::abs(M((size_t)i, (size_t)j_max)))
            j_max = j;
    return j_max;
}



template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    MaxEntry(const Matrix& M, int& i_max, int& j_max) const {
    // find the maximum entry in the matrix M in O(n) time
    i_max = 0;
    j_max = max_idx_row((size_t)i_max);
    Scalar max_entry = std::abs(M((size_t)i_max, (size_t)j_max));
    int nm1 = n - 1;
    for (int i = 1; i < nm1; i++) {
        int j = max_idx_row((size_t)i);
        if (std::abs(M((size_t)i, (size_t)j)) > max_entry) {
            max_entry = std::abs(M((size_t)i, (size_t)j));
            i_max = i;
            j_max = j;
        }
    }
    //#ifndef NDEBUG
    //// make sure that the maximum element really is stored at i_max, j_max
    ////   (comment out the next loop before using. it slows down the code)
    //for (int i = 0; i < nm1; i++)
    //  for (int j = i+1; j < n; j++)
    //    assert(std::abs(M(i, j)) <= max_entry);
    //#endif
}



//Sort the rows of a matrix "evec" by the numbers contained in "eval"
template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    SortRows(Vector eval, Matrix evec, int n, SortCriteria sort_criteria) const
{
    for (int i = 0; i < n - 1; i++) {
        int i_max = i;
        for (int j = i + 1; j < n; j++) {
            // find the "maximum" element in the array starting at position i+1
            switch (sort_criteria) {
            case SORT_DECREASING_EVALS:
                if (eval((size_t)j) > eval((size_t)i_max))
                    i_max = j;
                break;
            case SORT_INCREASING_EVALS:
                if (eval((size_t)j) < eval((size_t)i_max))
                    i_max = j;
                break;
            case SORT_DECREASING_ABS_EVALS:
                if (std::abs(eval((size_t)j)) > std::abs(eval((size_t)i_max)))
                    i_max = j;
                break;
            case SORT_INCREASING_ABS_EVALS:
                if (std::abs(eval((size_t)j)) < std::abs(eval((size_t)i_max)))
                    i_max = j;
                break;
            default:
                break;
            }
        }
        std::swap(eval((size_t)i), eval((size_t)i_max)); // sort "eval"
        for (int k = 0; k < n; k++)
            std::swap(evec((size_t)i, (size_t)k), evec((size_t)i_max, (size_t)k)); // sort "evec"
    }
}



template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Init() {
    n = 0;
    M.destroy();
    max_idx_row.destroy();
}

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    SetSize(int n) {
    Dealloc();
    Alloc(n);
}

// memory management:

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Alloc(int N) {
    n = N;
    if (n > 0) {
        max_idx_row.resize((size_t)n);
        M.resize((size_t)n, (size_t)n);
    }
}

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Dealloc() {
    max_idx_row.destroy();
    M.destroy();
    Init();
}

// memory management: copy and move constructor, swap, and assignment operator:

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Jacobi(const Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>& source)
{
    Init();
    SetSize(source.n);
    //assert(n == source.n);
    // The following lines aren't really necessary, because the contents
    // of source.M and source.max_idx_row are not needed (since they are
    // overwritten every time Jacobi::Diagonalize() is invoked).
    std::copy(source.max_idx_row,
        source.max_idx_row + n,
        max_idx_row);
    M = source.M.copy();
}

template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
void Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    swap(Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>& other) {
    std::swap(n, other.n);
    std::swap(max_idx_row, other.max_idx_row);
    std::swap(M, other.M);
}

// Move constructor (C++11)
template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    Jacobi(Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>&& other) {
    Init();
    this->swap(other);
}

// Using the "copy-swap" idiom for the assignment operator
template<typename Scalar, typename Vector, typename Matrix, typename ConstMatrix, typename IndexVector>
Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>&
    Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector>::
    operator = (Jacobi<Scalar, Vector, Matrix, ConstMatrix, IndexVector> source) {
    this->swap(source);
    return *this;
}

} // namespace jacobi
