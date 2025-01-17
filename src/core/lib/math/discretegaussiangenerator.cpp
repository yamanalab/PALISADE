/*
 * @file discretegaussiangenerator.cpp This code provides generation of gaussian
 * distibutions of discrete values. Discrete uniform generator relies on the
 * built-in C++ generator for 32-bit unsigned integers defined in <random>.
 * @author  TPOC: contact@palisade-crypto.org
 *
 * @copyright Copyright (c) 2019, New Jersey Institute of Technology (NJIT)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. THIS SOFTWARE IS
 * PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "math/discretegaussiangenerator.h"
#include "math/nbtheory.h"
#include "math/backend.h"

namespace lbcrypto {

#define KARNEY_THRESHOLD ((float)300.0)

template <typename VecType>
DiscreteGaussianGeneratorImpl<VecType>::DiscreteGaussianGeneratorImpl(
    double std)
    : DistributionGenerator<VecType>() {
  SetStd(std);
}

template <typename VecType>
void DiscreteGaussianGeneratorImpl<VecType>::SetStd(double std) {
  m_std = std;
  if (m_std < KARNEY_THRESHOLD)
    peikert = true;
  else
    peikert = false;
  if (peikert) {
    Initialize();
  }
}

template <typename VecType>
double DiscreteGaussianGeneratorImpl<VecType>::GetStd() const {
  return m_std;
}

template <typename VecType>
void DiscreteGaussianGeneratorImpl<VecType>::Initialize() {
  m_vals.clear();

  // weightDiscreteGaussian
  double acc = 1e-15;
  double variance = m_std * m_std;

  int fin = (int)ceil(m_std * sqrt(-2 * log(acc)));
  // this value of fin (M) corresponds to the limit for double precision
  // usually the bound of m_std * M is used, where M = 20 .. 40 - see DG14 for
  // details M = 20 corresponds to 1e-87
  // double mr = 20; // see DG14 for details
  // int fin = (int)ceil(m_std * mr);

  double cusum = 1.0;

  for (int x = 1; x <= fin; x++) {
    cusum = cusum + 2 * exp(-x * x / (variance * 2));
  }

  m_a = 1 / cusum;

  // fin = (int)ceil(sqrt(-2 * variance * log(acc))); //not needed - same as
  // above
  double temp;

  for (int i = 1; i <= fin; i++) {
    temp = m_a * exp(-((double)(i * i) / (2 * variance)));
    m_vals.push_back(temp);
  }

  // take cumulative summation
  for (usint i = 1; i < m_vals.size(); i++) {
    m_vals[i] += m_vals[i - 1];
  }
}

template <typename VecType>
int32_t DiscreteGaussianGeneratorImpl<VecType>::GenerateInt() const {
  std::uniform_real_distribution<double> distribution(0.0, 1.0);

  usint val = 0;
  double seed;
  int32_t ans;

  seed = distribution(PseudoRandomNumberGenerator::GetPRNG()) -
         0.5;  // we need to use the binary uniform generator rather than
               // regular continuous distribution; see DG14 for details
  if (std::abs(seed) <= m_a / 2) {
    val = 0;
  } else if (seed > 0) {
    val = FindInVector(m_vals, (std::abs(seed) - m_a / 2));
  } else {
    val = -(int)FindInVector(m_vals, (std::abs(seed) - m_a / 2));
  }
  ans = val;

  return ans;
}

template <typename VecType>
std::shared_ptr<int32_t>
DiscreteGaussianGeneratorImpl<VecType>::GenerateIntVector(usint size) const {
  std::shared_ptr<int32_t> ans(new int32_t[size], std::default_delete<int[]>());
  usint val = 0;
  double seed;
  if (peikert) {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    for (usint i = 0; i < size; i++) {
      seed = distribution(PseudoRandomNumberGenerator::GetPRNG()) -
             0.5;  // we need to use the binary uniform generator rather than
                   // regular continuous distribution; see DG14 for details
      if (std::abs(seed) <= m_a / 2) {
        val = 0;
      } else {
        if (seed > 0) {
          val = FindInVector(m_vals, (std::abs(seed) - m_a / 2));
        } else {
          val = -(int)FindInVector(m_vals, (std::abs(seed) - m_a / 2));
        }
      }
      (ans.get())[i] = val;
    }
  } else {
    for (usint i = 0; i < size; i++) {
      (ans.get())[i] = GenerateIntegerKarney(0, m_std);
    }
  }
  return ans;
}

template <typename VecType>
usint DiscreteGaussianGeneratorImpl<VecType>::FindInVector(
    const std::vector<double> &S, double search) const {
  // STL binary search implementation
  auto lower = std::lower_bound(S.begin(), S.end(), search);
  if (lower != S.end()) {
    return lower - S.begin() + 1;
  }

  else
    PALISADE_THROW(not_available_error,
                   "DGG Inversion Sampling. FindInVector value not found: " +
                       std::to_string(search));
}

template <typename VecType>
typename VecType::Integer
DiscreteGaussianGeneratorImpl<VecType>::GenerateInteger(
    const typename VecType::Integer &modulus) const {
  int32_t val = 0;
  double seed;
  typename VecType::Integer ans;
  std::uniform_real_distribution<double> distribution(0.0, 1.0);

  seed = distribution(PseudoRandomNumberGenerator::GetPRNG()) - 0.5;

  if (std::abs(seed) <= m_a / 2) {
    val = 0;
  } else if (seed > 0) {
    val = FindInVector(m_vals, (std::abs(seed) - m_a / 2));
  } else {
    val = -(int)FindInVector(m_vals, (std::abs(seed) - m_a / 2));
  }

  if (val < 0) {
    val *= -1;
    ans = modulus - typename VecType::Integer(val);
  } else
    ans = typename VecType::Integer(val);

  return ans;
}

template <typename VecType>
VecType DiscreteGaussianGeneratorImpl<VecType>::GenerateVector(
    const usint size, const typename VecType::Integer &modulus) const {
  std::shared_ptr<int32_t> result = GenerateIntVector(size);

  VecType ans(size);
  ans.SetModulus(modulus);

  for (usint i = 0; i < size; i++) {
    int32_t v = (result.get())[i];
    if (v < 0) {
      v *= -1;
      ans[i] = modulus - typename VecType::Integer(v);
    } else {
      ans[i] = typename VecType::Integer(v);
    }
  }

  return ans;
}

template <typename VecType>
typename VecType::Integer
DiscreteGaussianGeneratorImpl<VecType>::GenerateInteger(
    double mean, double stddev, size_t n,
    const typename VecType::Integer &modulus) const {
  double t = log2(n) * stddev;

  typename VecType::Integer result;

  std::uniform_int_distribution<int32_t> uniform_int(floor(mean - t),
                                                     ceil(mean + t));
  std::uniform_real_distribution<double> uniform_real(0.0, 1.0);

  bool flagSuccess = false;
  int32_t x;

  while (!flagSuccess) {
    //  pick random int
    x = uniform_int(PseudoRandomNumberGenerator::GetPRNG());
    //  roll the uniform dice
    double dice = uniform_real(PseudoRandomNumberGenerator::GetPRNG());
    //  check if dice land below pdf
    if (dice <= UnnormalizedGaussianPDF(mean, stddev, x)) {
      flagSuccess = true;
    }
  }

  if (x < 0) {
    x *= -1;
    result = modulus - typename VecType::Integer(x);
  } else
    result = typename VecType::Integer(x);

  return result;
}

template <typename VecType>
int32_t DiscreteGaussianGeneratorImpl<VecType>::GenerateInteger(
    double mean, double stddev, size_t n) const {
  DEBUG_FLAG(false);
  int32_t x;

  double t = log2(n) *
             stddev;  // this representation of log_2 is used for Visual Studio
  DEBUG("DiscreteGaussianGeneratorImpl =========");
  DEBUG("mean " << mean);
  DEBUG("stddev " << stddev);
  DEBUG("n " << n);
  DEBUG("t " << t);

  if (std::isinf(mean)) {
    PALISADE_THROW(not_available_error,
                   "DiscreteGaussianGeneratorImpl called with mean == +-inf");
  }
  if (std::isinf(stddev)) {
    PALISADE_THROW(not_available_error,
                   "DiscreteGaussianGeneratorImpl called with stddev == +-inf");
  }
  typename VecType::Integer result;

  std::uniform_int_distribution<int32_t> uniform_int(floor(mean - t),
                                                     ceil(mean + t));
  DEBUG("uniform( " << floor(mean - t) << ", " << ceil(mean + t) << ")");
  std::uniform_real_distribution<double> uniform_real(0.0, 1.0);

  double sigmaFactor = -1 / (2. * stddev * stddev);
  DEBUG("sigmaFactor " << sigmaFactor);

  bool flagSuccess = false;

  usint count = 0;
  const usint limit = 10000;
  // PALISADE_THROW(palisade_error, "dbg throw");

  while (!flagSuccess) {
    //  pick random int
    x = uniform_int(PseudoRandomNumberGenerator::GetPRNG());

    //  roll the uniform dice
    double dice = uniform_real(PseudoRandomNumberGenerator::GetPRNG());
    //  check if dice land below pdf
    if (dice <= UnnormalizedGaussianPDFOptimized(mean, sigmaFactor, x)) {
      flagSuccess = true;
    }
    // DEBUG("x "<<x<<" dice "<<dice);
    count++;
    if (count > limit) {
      DEBUG("x " << x << " dice " << dice);
      PALISADE_THROW(
          not_available_error,
          "GenerateInteger could not find success after repeated attempts");
    }
  }

  return x;
}
template <typename VecType>
int64_t DiscreteGaussianGeneratorImpl<VecType>::GenerateIntegerKarney(
    double mean, double stddev) {
  int64_t result;
  std::uniform_int_distribution<int32_t> uniform_sign(0, 1);
  std::uniform_int_distribution<int64_t> uniform_j(0, ceil(stddev) - 1);

  PRNG &g = PseudoRandomNumberGenerator::GetPRNG();

  bool flagSuccess = false;
  int32_t k;

  while (!flagSuccess) {
    // STEP D1
    k = AlgorithmG(g);

    // STEP D2
    if (!AlgorithmP(g, k * (k - 1))) continue;

    // STEP D3
    int32_t s = uniform_sign(g);
    if (s == 0) s = -1;

    // STEP D4
    double di0 = stddev * k + s * mean;
    int64_t i0 = std::ceil(di0);
    double x0 = (i0 - di0) / stddev;
    int64_t j = uniform_j(g);

    double x = x0 + j / stddev;

    // STEPS D5 and D6
    if (!(x < 1) || (x == 0 && s < 0 && k == 0)) continue;

    // STEP D7
    int32_t h = k + 1;
    while (h-- && AlgorithmB(g, k, x)) {
    };
    if (!(h < 0)) continue;

    // STEP D8
    result = s * (i0 + j);
    flagSuccess = true;
  }

  return result;
}

template <typename VecType>
bool DiscreteGaussianGeneratorImpl<VecType>::AlgorithmP(PRNG &g, int n) {
  while (n-- && AlgorithmH(g)) {
  };
  return n < 0;
}

template <typename VecType>
int32_t DiscreteGaussianGeneratorImpl<VecType>::AlgorithmG(PRNG &g) {
  int n = 0;
  while (AlgorithmH(g)) ++n;
  return n;
}

// Use single floating-point precision in most cases; if a situation w/ not
// enough precision is encountered, call the double-precision algorithm
template <typename VecType>
bool DiscreteGaussianGeneratorImpl<VecType>::AlgorithmH(PRNG &g) {
  std::uniform_real_distribution<float> dist(0, 1);
  float h_a, h_b;
  h_a = dist(g);

  // less than the half
  if (h_a > 0.5)
    return true;
  else if (h_a < 0.5) {
    for (;;) {
      h_b = dist(g);
      if (h_b > h_a)
        return false;
      else if (h_b < h_a)
        h_a = dist(g);
      else  // numbers are equal - need higher precision
        return AlgorithmHDouble(g);
      if (h_a > h_b)
        return true;
      else if (h_a == h_b)  // numbers are equal - need higher precision
        return AlgorithmHDouble(g);
    }
  } else  // numbers are equal - need higher precision
    return AlgorithmHDouble(g);
}

template <typename VecType>
bool DiscreteGaussianGeneratorImpl<VecType>::AlgorithmHDouble(PRNG &g) {
  std::uniform_real_distribution<double> dist(0, 1);
  double h_a, h_b;
  h_a = dist(g);
  // less than the half
  if (!(h_a < 0.5)) return true;
  for (;;) {
    h_b = dist(g);
    if (!(h_b < h_a))
      return false;
    else
      h_a = dist(g);
    if (!(h_a < h_b)) return true;
  }
}

template <typename VecType>
bool DiscreteGaussianGeneratorImpl<VecType>::AlgorithmB(PRNG &g, int32_t k,
                                                        double x) {
  std::uniform_real_distribution<float> dist(0.0, 1.0);

  float y = x;
  int32_t n = 0, m = 2 * k + 2;
  float z, r;
  float rTemp;

  for (;; ++n) {
    z = dist(g);
    if (z > y)
      break;
    else if (z < y) {
      r = dist(g);
      rTemp = (2 * k + x) / m;
      if (r > rTemp)
        break;
      else if (r < rTemp)
        y = z;
      else  // r == Temp - need double precision
        return AlgorithmBDouble(g, k, x);
    } else  // z == x - need double precision
      return AlgorithmBDouble(g, k, x);
  }

  return (n % 2) == 0;
}

template <typename VecType>
bool DiscreteGaussianGeneratorImpl<VecType>::AlgorithmBDouble(PRNG &g,
                                                              int32_t k,
                                                              double x) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);

  double y = x;
  int32_t n = 0, m = 2 * k + 2;
  double z, r;

  for (;; ++n) {
    z = dist(g);
    if (!(z < y)) break;
    r = dist(g);
    if (!(r < (2 * k + x) / m)) break;
    y = z;
  }

  return (n % 2) == 0;
}
}  // namespace lbcrypto
