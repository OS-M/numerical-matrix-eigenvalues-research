#pragma once

#include "Matrix/matrix.h"
#include "euclidean_norm.h"
#include "minimal_square_problem.h"
#include "eigenvalues.h"

namespace __internal {

template<class T>
std::pair<std::complex<T>, std::complex<T>>
PowerMethodEigenvaluesComplexIteration(
    const Matrix<std::complex<T>>& a,
    const Matrix<std::complex<T>>& squared_a,
    Matrix<std::complex<T>>& y,
    Matrix<std::complex<T>>& u) {
  y.Assign(a * u);
  u.Assign(y / EuclideanNorm<std::complex<T>>(y));

  Matrix<T> l(u.Rows(), 2);
  auto au = a * u;
  for (int i = 0; i < u.Rows(); i++) {
    l(i, 0) = u(i).real();
    l(i, 1) = au(i).real();
  }
  auto rc = -1 * squared_a * u;
  Matrix<T> r(u.Rows(), 1);
  for (int i = 0; i < u.Rows(); i++) {
    r(i) = rc(i).real();
  }
  auto c = MinimalSquareProblem(l, r);
  auto[r1, r2] =
    SolveQuadraticEquation<T>(1, c(1), c(0));

  return {r1, r2};
}

template<class T>
T PowerIterationMethod1Iteration(
    const Matrix<T>& a,
    Matrix<T>& u,
    Matrix<T>& y) {
  y.Assign(a * u);
  u.Assign(y / EuclideanNorm<T>(y));
  auto lambda = (u.ScalarProduct(a * u));
  return lambda;
}

template<class T>
std::pair<bool, Matrix<T>> PowerIterationMethod1IterationConverges(
    const Matrix<T>& a,
    int iters,
    int step) {
  int n = a.Rows();
  auto y = Matrix<T>(n, 1);
  y(0) = 1;
  auto u = y / EuclideanNorm<T>(y);
  auto lambda = u.ScalarProduct(a * u);
  auto prev_lambda = 1e18;
  std::vector<T> diffs;
  diffs.reserve(iters);
  for (int i = 0; i < iters; i++) {
    prev_lambda = lambda;
    lambda = __internal::PowerIterationMethod1Iteration(a, u, y);
    diffs.push_back(std::abs(prev_lambda - lambda));
    if (std::abs(diffs.back()) < Matrix<T>::GetEps()) {
      return {true, y};
    }
    if (diffs.size() >= step && (diffs.back() >= diffs[diffs.size() - step] ||
        std::abs(diffs.back() - diffs[diffs.size() - step]) <= Matrix<T>::GetEps())) {
      return {false, y};
    }
  }
  return {true, y};
}

template<class T>
T PowerIterationMethod2Iteration(
    const Matrix<T>& a,
    const Matrix<T>& a2,
    Matrix<T>& u,
    Matrix<T>& y) {
  y.Assign(a2 * u);
  u.Assign(y / EuclideanNorm<T>(y));
  auto lambda = u.ScalarProduct(a2 * u);
  return lambda;
}

template<class T>
std::pair<bool, Matrix<T>> PowerIterationMethod2IterationConverges(
    const Matrix<T>& a,
    const Matrix<T>& a2,
    int iters,
    int step) {
  int n = a.Rows();
  auto y = Matrix<T>(n, 1);
  y(0) = 1;
  auto u = y / EuclideanNorm<T>(y);
  auto lambda = u.ScalarProduct(a * u);
  auto prev_lambda = 1e18;
  std::vector<T> diffs;
  diffs.reserve(iters);
  for (int i = 0; i < iters; i++) {
    prev_lambda = lambda;
    lambda = __internal::PowerIterationMethod2Iteration(a, a2, u, y);
    diffs.push_back(std::abs(prev_lambda - lambda));
    if (std::abs(diffs.back()) < Matrix<T>::GetEps()) {
      return {true, y};
    }
    if (diffs.size() >= step && (diffs.back() >= diffs[diffs.size() - step] ||
        std::abs(diffs.back() - diffs[diffs.size() - step]) <= Matrix<T>::GetEps())) {
      return {false, y};
    }
  }
  return {true, y};
}

template<class T>
std::pair<T, Matrix<T>> PowerMethodEigenvalues1(
    const Matrix<T>& a,
    Matrix<T> y,
    int* iters = nullptr,
    int max_iters = 100) {
  if (!a.IsSquare()) {
    throw std::invalid_argument(
        "Matrix of size " + PairToString(a.Size()) + " is not square.");
  }
  int n = a.Rows();
  y(0) = 1;
  auto u = y / EuclideanNorm<T>(y);
  auto lambda = u.ScalarProduct(a * u);
  int iter = 0;
  auto prev_lambda = 1e18;
  while (std::abs(prev_lambda - lambda) > Matrix<T>::GetEps()) {
    prev_lambda = lambda;
    lambda = __internal::PowerIterationMethod1Iteration(a, u, y);
    iter++;
    if (iter > max_iters) {
      break;
    }
  }
  if (iters) {
    *iters = iter + 1;
    if (iter >= max_iters || EuclideanNorm<T>(u) < Matrix<T>::GetEps()) {
      *iters = -1;
    }
  }
  return {lambda, u};
}

template<class T>
std::vector<std::pair<std::complex<T>,
                      Matrix<std::complex<T>>>> PowerMethodEigenvalues2(
    const Matrix<T>& a,
    Matrix<T>& y,
    int* iters = nullptr,
    int max_iters = 100,
    T eps = Matrix<T>::GetEps()) {
  if (!a.IsSquare()) {
    throw std::invalid_argument(
        "Matrix of size " + PairToString(a.Size()) + " is not square.");
  }
  auto a2 = a * a;
  int n = a.Rows();
  auto u = y / EuclideanNorm<T>(y);
  auto lambda = std::sqrt(std::abs(u.ScalarProduct(a2 * u)));
  auto v1 = u;
  auto v2 = u;
  int iter = 0;
  auto prev_lambda = 1e18;
  while (std::abs(lambda - prev_lambda) > eps) {
    prev_lambda = lambda;
    lambda = std::sqrt(std::abs(
        __internal::PowerIterationMethod2Iteration(a, a2, u, y)));
    iter++;
    if (iter > max_iters) {
      break;
    }
  }
  v1 = (lambda * a * u + a2 * u) / (2 * lambda * lambda);
  v2 = (-lambda * a * u + a2 * u) / (2 * lambda * lambda);

  std::vector<std::pair<std::complex<T>, Matrix<std::complex<T>>>> ans;
  if (EuclideanNorm<T>(v1) > eps) {
    ans.emplace_back(lambda, v1.ToComplex());
  }
  if (EuclideanNorm<T>(v2) > eps) {
    ans.emplace_back(-lambda, v2.ToComplex());
  }

  if (iters) {
    *iters = iter + 1;
    if (iter >= max_iters || ans.empty()) {
      *iters = -1;
    }
  }

  return ans;
}

template<class T>
std::vector<std::pair<std::complex<T>,
                      Matrix<std::complex<T>>>> PowerMethodEigenvalues3(
    const Matrix<T>& a,
    int* iters = nullptr,
    int max_iters = 100) {
  if (!a.IsSquare()) {
    throw std::invalid_argument(
        "Matrix of size " + PairToString(a.Size()) + " is not square.");
  }
  auto squared_a = a * a;
  int n = a.Rows();

  auto complex_a = a.ToComplex();
  auto complex_squared_a = squared_a.ToComplex();

  Matrix<std::complex<T>> complex_y(n, 1);
  complex_y(0) = 1;
  auto u = complex_y / EuclideanNorm<std::complex<T>>(complex_y);

  std::complex<T> prev_r1 = 1e18;
  std::complex<T> r1;
  std::complex<T> prev_r2 = 1e18;
  std::complex<T> r2;

  int iter = 0;

  while (std::abs(prev_r1 - r1) > std::abs(Matrix<T>::GetEps()) ||
      std::abs(prev_r2 - r2) > std::abs(Matrix<T>::GetEps())) {
    auto[p1, p2] =
    __internal::PowerMethodEigenvaluesComplexIteration(
        complex_a, complex_squared_a, complex_y, u);
    prev_r1 = r1;
    prev_r2 = r2;
    r1 = p1;
    r2 = p2;

    iter++;
    if (iter > max_iters) {
      break;
    }
  }

  auto u1 = complex_a * u;
  auto u2 = complex_squared_a * u;

  Matrix<std::complex<T>> v1(u.Rows(), 1);
  Matrix<std::complex<T>> v2(u.Rows(), 1);

  for (int i = 0; i < u.Rows(); i++) {
    v1(i) = u2(i) - r2 * u1(i);
    v2(i) = u1(i) - u2(i) / r1;
  }

  if (iters) {
    *iters = iter + 1;
    if (iter >= max_iters) {
      *iters = -1;
    }
  }

  // std::cerr << error1 << ' ' << error2 << '\n';

  std::vector<std::pair<std::complex<T>,
                        Matrix<std::complex<T>>>> ans;
  if (std::norm(EuclideanNorm<std::complex<T>>(v1))
          > std::norm(Matrix<std::complex<T>>::GetEps())) {
    ans.emplace_back(r1, v1);
  }
  if (std::norm(EuclideanNorm<std::complex<T>>(v2))
          > std::norm(Matrix<std::complex<T>>::GetEps())) {
    ans.emplace_back(r2, v2);
  }
  return ans;
}

}

template<class T>
std::vector<std::pair<std::complex<T>,
                      Matrix<std::complex<T>>>> PowerMethodEigenvalues(
    const Matrix<T>& a,
    int* iters = nullptr,
    int max_iters = 100,
    int check_iters = 10,
    int check_step = 5,
    int force_method = -1) {
  if (iters) {
    *iters = 0;
  }
  if (force_method != -1) {
    Matrix<T> y(a.Rows(), 1);
    y(0) = 1;
    switch (force_method) {
      case 0: {
        auto[e, v] =
            __internal::PowerMethodEigenvalues1(a, y, iters, max_iters);
        return {std::make_pair(e, v.ToComplex())};
      }
      case 1: {
        return __internal::PowerMethodEigenvalues2(a, y, iters, max_iters);
      }
      case 2: {
        return __internal::PowerMethodEigenvalues3(a, iters, max_iters);
      }
      default: {
        break;
      }
    }
  }
  {
    Matrix<T> y(a.Rows(), 1);
    y(0) = 1;
    int iters_ = 0;
    __internal::PowerMethodEigenvalues2(a, y, &iters_, max_iters, 0.1);
    if (iters_ >= 0) {
      int iters__ = 0;
      auto res = __internal::PowerMethodEigenvalues2(a, y, &iters__, max_iters);
      if (iters__ >= 0 && !res.empty()) {
        if (iters) {
          *iters = iters_ + iters__;
        }
        return res;
      }
    }
  }
  // std::cerr << "Method 3\n";
  return __internal::PowerMethodEigenvalues3(a, iters, max_iters);
}
