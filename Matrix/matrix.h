#pragma once

#include <memory>
#include <random>
#include <sstream>
#include <iomanip>
#include <complex>

template<class T, class U>
void AssertEqualSizes(const T& a, const U& b);

template<class T, class U>
std::string PairToString(const std::pair<T, U>& pair) {
  std::stringstream ss;
  ss << "(" << pair.first << "; " << pair.second << ")";
  return ss.str();
}

template<class T>
class Matrix {
 public:
  Matrix();
  Matrix(std::initializer_list<std::initializer_list<T>>);
  Matrix(std::initializer_list<T>) = delete;
  Matrix(int n, int m);
  Matrix(int n, int m, T value);
  Matrix(const Matrix<T>& other);
  Matrix(Matrix<T>&& other) noexcept;
  explicit Matrix(int n);

  void Swap(Matrix<T>& b);
  void CopyFrom(const Matrix<T>& other);
  void Assign(const Matrix<T>& b);
  void MakeRef(Matrix<T>&& b);

  std::pair<int, int> Size() const;

  Matrix<std::complex<T>> ToComplex() const;

  bool IsSquare() const;

  int Rows() const;
  int Cols() const;

  Matrix<T> Transposed();

  static Matrix<T> Ones(int n);
  static Matrix<T> Zeros(int n, int m);
  static Matrix<T> Random(int n, int m, T min, T max, int seed = time(nullptr),
                          bool force_seed = false);
  static Matrix<T> RandomInts(int n, int m, int min, int max,
                              int seed = time(nullptr));

  std::string ToWolframString() const;

  T& operator()(int i);  // For vectors only
  const T& operator()(int i) const;  // For vectors only
  T& operator()(int i, int j);
  const T& operator()(int i, int j) const;

  T& At(int i);  // For vectors only
  const T& At(int i) const;  // For vectors only
  T& At(int i, int j);
  const T& At(int i, int j) const;

  // const Matrix<T> SubMatrix(int i, int j, int n, int m) const;
  Matrix<T> SubMatrix(int i, int j, int n, int m);

  bool IsVector() const;
  bool IsRowVector() const;
  bool IsColVector() const;

  // const Matrix<T> Row(int i) const;
  Matrix<T> Row(int i);
  // const Matrix<T> Col(int j) const;
  Matrix<T> Col(int j);

  Matrix<T>& operator=(const Matrix<T>& b);
  Matrix<T>& operator=(Matrix<T>&& b) noexcept;

  template<class U>
  friend Matrix<U> operator+(const Matrix<U>& a, const Matrix<U>& b);
  template<class U>
  friend Matrix<U> operator-(const Matrix<U>& a, const Matrix<U>& b);
  template<class U>
  friend Matrix<U> operator*(const Matrix<U>& a, const Matrix<U>& b);
  T ScalarProduct(const Matrix<T>& b) const;
  template<class U, class W>
  friend Matrix<U> operator*(const Matrix<U>& a, W b);
  template<class U, class W>
  friend Matrix<U> operator/(const Matrix<U>& a, W b);
  template<class U, class W>
  friend Matrix<U> operator*(W b, const Matrix<U>& a);

  Matrix<T>& operator+=(const Matrix<T>& a);
  Matrix<T>& operator-=(const Matrix<T>& a);
  Matrix<T>& operator*=(const Matrix<T>& a);
  template<class U>
  Matrix<T>& operator*=(U b);
  template<class U>
  Matrix<T>& operator/=(U b);

  template<class U>
  friend bool operator==(const Matrix<U>& a, const Matrix<U>& b);
  template<class U>
  friend bool operator!=(const Matrix<U>& a, const Matrix<U>& b);

  static int GetPrecision();
  static T GetEps();
  static void SetEps(T eps, int precision);

 private:
  std::shared_ptr<T[]> data_;
  int data_rows_;
  int data_cols_;
  int cols_;
  int rows_;
  int offset_i_;
  int offset_j_;

  bool IsSubMatrix() const;

  static T eps;
  static int precision;
};

template<class T>
T Matrix<T>::eps = std::numeric_limits<T>::epsilon();

template<class T>
T Matrix<T>::GetEps() {
  return eps;
}

template<class T>
void Matrix<T>::SetEps(T eps_, int precision_) {
  eps = eps_;
  precision = precision_;
}

template<class T>
int Matrix<T>::precision = 0;

template<class T>
Matrix<T>::Matrix() : Matrix<T>(0, 0) {}

template<class T>
Matrix<T>::Matrix(int n, int m) : Matrix<T>(n, m, T()) {}

template<class T>
Matrix<T>::Matrix(int n, int m, T value) :
    data_(new T[n * m]),
    offset_i_(0),
    offset_j_(0),
    data_rows_(n),
    rows_(n),
    data_cols_(m),
    cols_(m) {
  for (int i = 0; i < this->Rows(); i++) {
    for (int j = 0; j < this->Cols(); j++) {
      (*this)(i, j) = value;
    }
  }
}

template<class T>
Matrix<T>::Matrix(
    std::initializer_list<std::initializer_list<T>> list) :
    Matrix(std::distance(list.begin(), list.end()),
           std::distance(list.begin()->begin(), list.begin()->end())) {
  int i = 0;
  for (auto row: list) {
    if (std::distance(row.begin(), row.end()) != this->Cols()) {
      throw std::invalid_argument("All rows should have same size, got "
                                      + std::to_string(std::distance(row.begin(),
                                                                     row.end()))
                                      + " instead of "
                                      + std::to_string(this->Cols()));
    }
    int j = 0;
    for (auto elem: row) {
      this->At(i, j++) = elem;
    }
    i++;
  }
}

template<class T>
Matrix<T>::Matrix(int n) : Matrix<T>(n, n) {}

template<class T>
Matrix<T>::Matrix(const Matrix<T>& other) {
  this->CopyFrom(other);
}

template<class T>
Matrix<T>::Matrix(Matrix<T>&& other) noexcept : Matrix() {
  this->Swap(other);
}

template<class T>
std::pair<int, int> Matrix<T>::Size() const {
  return {rows_, cols_};
}

template<class T>
int Matrix<T>::Rows() const {
  return rows_;
}

template<class T>
int Matrix<T>::Cols() const {
  return cols_;
}

template<class T>
Matrix<T> Matrix<T>::Transposed() {
  Matrix<T> a(this->Cols(), this->Rows());
  for (int i = 0; i < this->Cols(); i++) {
    for (int j = 0; j < this->Rows(); j++) {
      a.At(i, j) = this->At(j, i);
    }
  }
  return a;
}

template<class T>
Matrix<T> Matrix<T>::Ones(int n) {
  Matrix<T> a(n);
  for (int i = 0; i < n; i++) {
    a(i, i) = 1;
  }
  return a;
}

template<class T>
Matrix<T> Matrix<T>::Zeros(int n, int m) {
  return Matrix<T>(n, m);
}

template<class T>
Matrix<T> Matrix<T>::Random(int n, int m, T min, T max, int seed,
                            bool force_seed) {
  Matrix<T> a(n, m);
  thread_local static std::mt19937 gen(seed);
  if (force_seed) {
    gen.seed(seed);
  }
  std::uniform_real_distribution<T> dist(min, max);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      a(i, j) = dist(gen);
    }
  }
  return a;
}

template<class T>
Matrix<T> Matrix<T>::RandomInts(int n, int m, int min, int max, int seed) {
  Matrix<T> a(n, m);
  thread_local static std::mt19937 gen(seed);
  std::uniform_int_distribution<int> dist(min, max);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      a(i, j) = dist(gen);
    }
  }
  return a;
}

template<class T>
std::string Matrix<T>::ToWolframString() const {
  std::stringstream res;
  auto precision = Matrix<T>::GetPrecision();
  res << "{";
  for (int i = 0; i < this->Rows(); i++) {
    res << "{";
    for (int j = 0; j < this->Cols(); j++) {
      res << std::fixed << std::setprecision(precision)
          << this->At(i, j);
      if (j + 1 != this->Cols()) {
        res << ",";
      }
    }
    res << "}";
    if (i + 1 != this->Rows()) {
      res << ",";
    }
  }
  res << "}\n";
  return res.str();
}

template<class T>
T& Matrix<T>::operator()(int i) {
  return this->At(i);
}

template<class T>
const T& Matrix<T>::operator()(int i) const {
  return this->At(i);
}

template<class T>
T& Matrix<T>::operator()(int i, int j) {
  return this->At(i, j);
}

template<class T>
const T& Matrix<T>::operator()(int i, int j) const {
  return this->At(i, j);
}

// template<class T>
// const Matrix<T> Matrix<T>::SubMatrix(int i, int j, int n, int m) const {
//   return const_cast<Matrix<T>*>(this)->SubMatrix(i, j, n, m);
// }

template<class T>
Matrix<T> Matrix<T>::SubMatrix(int i, int j, int n, int m) {
  if (n == -1) {
    n = this->Rows() - i;
  }
  if (m == -1) {
    m = this->Cols() - j;
  }
  Matrix<T> a;
  a.data_ = this->data_;
  a.rows_ = n;
  a.cols_ = m;
  a.offset_i_ = this->offset_i_ + i;
  a.offset_j_ = this->offset_j_ + j;
  a.data_cols_ = this->data_cols_;
  a.data_rows_ = this->data_rows_;
  return a;
}

// template<class T>
// const Matrix<T> Matrix<T>::Row(int i) const {
//   return const_cast<Matrix<T>*>(this)->Row(i);
// }

template<class T>
Matrix<T> Matrix<T>::Row(int i) {
  return this->SubMatrix(i, 0, 1, this->Cols());
}

// template<class T>
// const Matrix<T> Matrix<T>::Col(int j) const {
//   return const_cast<Matrix<T>*>(this)->Col(j);
// }

template<class T>
Matrix<T> Matrix<T>::Col(int j) {
  return this->SubMatrix(0, j, this->Rows(), 1);
}

template<class T>
Matrix<T>& Matrix<T>::operator+=(const Matrix<T>& a) {
  AssertEqualSizes(*this, a);
  for (int i = 0; i < a.Rows(); ++i) {
    for (int j = 0; j < a.Cols(); ++j) {
      this->At(i, j) += a.At(i, j);
    }
  }
  return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator-=(const Matrix<T>& a) {
  AssertEqualSizes(*this, a);
  for (int i = 0; i < a.Rows(); ++i) {
    for (int j = 0; j < a.Cols(); ++j) {
      this->At(i, j) -= a.At(i, j);
    }
  }
  return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator*=(const Matrix<T>& a) {
  *this->CopyFrom(*this * a);
  return *this;
}

template<class T>
template<class U>
Matrix<T>& Matrix<T>::operator*=(U b) {
  for (int i = 0; i < this->Rows(); ++i) {
    for (int j = 0; j < this->Cols(); ++j) {
      this->At(i, j) *= b;
    }
  }
  return *this;
}

template<class T>
template<class U>
Matrix<T>& Matrix<T>::operator/=(U b) {
  for (int i = 0; i < this->Rows(); ++i) {
    for (int j = 0; j < this->Cols(); ++j) {
      this->At(i, j) /= b;
    }
  }
  return *this;
}

template<class T>
bool Matrix<T>::IsSubMatrix() const {
  return !(cols_ == data_cols_
      && rows_ == data_rows_
      && offset_i_ == 0
      && offset_j_ == 0);
}

template<class T>
bool Matrix<T>::IsRowVector() const {
  return this->Rows() == 1;
}

template<class T>
bool Matrix<T>::IsColVector() const {
  return this->Cols() == 1;
}

template<class T>
T& Matrix<T>::At(int i) {
  if (this->IsRowVector()) {
    return this->At(0, i);
  } else if (this->IsColVector()) {
    return this->At(i, 0);
  } else {
    throw std::runtime_error(
        "trying to get value by single index in matrix of size "
            + PairToString(this->Size()));
  }
}

template<class T>
const T& Matrix<T>::At(int i) const {
  return const_cast<Matrix<T>*>(this)->At(i);
}

template<class T>
T& Matrix<T>::At(int i, int j) {
#ifndef DEBUG
  if (i < 0 || j < 0 || i >= rows_ || j >= cols_) {
    throw std::out_of_range(
        "Indexes (" + std::to_string(i) + "; " + std::to_string(j)
            + ") out of matrix size " + PairToString(this->Size()));
  }
#endif
  return data_[(i + offset_i_) * data_cols_ + offset_j_ + j];
}

template<class T>
const T& Matrix<T>::At(int i, int j) const {
  return const_cast<Matrix<T>*>(this)->At(i, j);
}

template<class T>
bool Matrix<T>::IsSquare() const {
  return this->Cols() == this->Rows();
}

template<class T>
bool Matrix<T>::IsVector() const {
  return this->IsColVector() || this->IsRowVector();
}

template<class T>
Matrix<T> operator+(const Matrix<T>& a, const Matrix<T>& b) {
  AssertEqualSizes(a, b);
  Matrix<T> res = a;
  res += b;
  return res;
}

template<class T>
Matrix<T> operator-(const Matrix<T>& a, const Matrix<T>& b) {
  AssertEqualSizes(a, b);
  Matrix<T> res = a;
  res -= b;
  return res;
}

template<class T>
Matrix<T> operator*(const Matrix<T>& lhs, const Matrix<T>& rhs) {
  if (lhs.Cols() != rhs.Rows()) {
    throw std::runtime_error(
        "Bad matrix sizes " + PairToString(lhs.Size()) + " "
            + PairToString(rhs.Size()));
  }
  Matrix<T> result(lhs.Rows(), rhs.Cols());
  for (int i = 0; i < lhs.Rows(); i++) {
    for (int k = 0; k < lhs.Cols(); k++) {
      for (int j = 0; j < rhs.Cols(); j++) {
        result.At(i, j) += lhs.At(i, k) * rhs.At(k, j);
      }
    }
  }
  return result;
}

template<class T, class U>
Matrix<T> operator*(const Matrix<T>& lhs, U rhs) {
  Matrix<T> result = lhs;
  result *= rhs;
  return result;
}

template<class T, class U>
Matrix<T> operator/(const Matrix<T>& lhs, U rhs) {
  Matrix<T> result = lhs;
  result /= rhs;
  return result;
}

template<class T>
T Matrix<T>::ScalarProduct(const Matrix<T>& b) const {
  if (!this->IsVector() || !b.IsVector()) {
    throw std::runtime_error("Matrices of sizes " + PairToString(this->Size()) +
        " and " + PairToString(b.Size()) + " are not both vectors");
  }
  AssertEqualSizes(*this, b);
  T ans = T();
  for (int i = 0; i < std::max(this->Cols(), this->Rows()); i++) {
    ans += this->At(i) * b.At(i);
  }
  return ans;
}

template<class T>
bool operator==(const Matrix<T>& a, const Matrix<T>& b) {
  return !(a != b);
}

template<class T>
bool operator!=(const Matrix<T>& a, const Matrix<T>& b) {
  AssertEqualSizes(a, b);
  for (int i = 0; i < a.Rows(); i++) {
    for (int j = 0; j < a.Cols(); j++) {
      if (a(i, j) != a(i, j) || b(i, j) != b(i, j) ||  // check for NaNs
          std::abs(a(i, j) - b(i, j)) > Matrix<T>::GetEps()) {
        return true;
      }
    }
  }
  return false;
}

template<class T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T>& b) {
  this->CopyFrom(b);
  return *this;
}

template<class T>
Matrix<T>& Matrix<T>::operator=(Matrix<T>&& b) noexcept {
  this->Swap(b);
  return *this;
}

template<class U, class W>
Matrix<U> operator*(W b, const Matrix<U>& a) {
  return a * b;
}

template<class T>
void Matrix<T>::CopyFrom(const Matrix<T>& other) {
  this->MakeRef(Matrix<T>(other.Rows(), other.Cols()));
  this->Assign(other);
}

template<class T>
void Matrix<T>::MakeRef(Matrix<T>&& b) {
  data_ = b.data_;
  data_rows_ = b.data_rows_;
  data_cols_ = b.data_cols_;
  cols_ = b.cols_;
  rows_ = b.rows_;
  offset_i_ = b.offset_i_;
  offset_j_ = b.offset_j_;
}

template<class T>
void Matrix<T>::Assign(const Matrix<T>& b) {
  AssertEqualSizes(*this, b);
  for (int i = 0; i < this->Rows(); i++) {
    for (int j = 0; j < this->Cols(); j++) {
      this->At(i, j) = b.At(i, j);
    }
  }
}

template<class T>
int Matrix<T>::GetPrecision() {
  return precision;
}

template<class T>
void Matrix<T>::Swap(Matrix<T>& b) {
  std::swap(data_, b.data_);
  std::swap(data_rows_, b.data_rows_);
  std::swap(data_cols_, b.data_cols_);
  std::swap(cols_, b.cols_);
  std::swap(rows_, b.rows_);
  std::swap(offset_i_, b.offset_i_);
  std::swap(offset_j_, b.offset_j_);
}

template<class T>
Matrix<std::complex<T>> Matrix<T>::ToComplex() const {
  Matrix<std::complex<T>> ans(this->Rows(), this->Cols());
  for (int i = 0; i < this->Rows(); i++) {
    for (int j = 0; j < this->Cols(); j++) {
      ans.At(i, j) = this->At(i, j);
    }
  }
  return ans;
}

template<class T, class U>
void AssertEqualSizes(const T& a, const U& b) {
  if (a.Size() != b.Size()) {
    throw std::invalid_argument("Wrong matrix sizes: " +
        PairToString(a.Size()) + " " + PairToString(b.Size()));
  }
}

template<class U>
std::ostream& operator<<(std::ostream& stream,
                         const Matrix<U>& matrix) {
  size_t maxlen = 0;
  auto precision = Matrix<U>::GetPrecision();
  for (size_t i = 0; i < matrix.Rows(); i++) {
    for (size_t j = 0; j < matrix.Cols(); j++) {
      std::stringstream ss;
      ss << std::fixed << std::setprecision(precision) << matrix.At(i, j);
      maxlen = std::max(maxlen, ss.str().length());
    }
  }
  stream << "[";
  for (size_t i = 0; i < matrix.Rows(); i++) {
    if (i != 0) {
      stream << ' ';
    }
    for (size_t j = 0; j < matrix.Cols(); j++) {
      stream << std::fixed << std::setprecision(precision) << std::setw(maxlen)
             << matrix.At(i, j);
      if (i + 1 < matrix.Rows() || j + 1 < matrix.Cols()) {
        stream << ", ";
      }
    }
    if (i + 1 < matrix.Rows()) {
      stream << '\n';
    }
  }
  stream << "]\n";
  return stream;
}

typedef Matrix<double> DMatrix;
