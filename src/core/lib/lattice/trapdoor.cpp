/**
 * @file trapdoor.cpp Provides the utility for sampling trapdoor lattices as
 * described in https://eprint.iacr.org/2017/844.pdf
 * https://eprint.iacr.org/2018/946, and "Implementing Token-Based Obfuscation
 * under (Ring) LWE" (not publicly available yet)
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

#ifndef _SRC_LIB_CRYPTO_SIGNATURE_TRAPDOOR_CPP
#define _SRC_LIB_CRYPTO_SIGNATURE_TRAPDOOR_CPP

#include "lattice/trapdoor.h"

namespace lbcrypto {

// On-line stage of pre-image sampling (includes only G-sampling)

template <class Element>
Matrix<Element> RLWETrapdoorUtility<Element>::GaussSampOnline(
    size_t n, size_t k, const Matrix<Element>& A,
    const RLWETrapdoorPair<Element>& T, const Element& u,
    typename Element::DggType& dgg, const shared_ptr<Matrix<Element>> pHat,
    int64_t base) {
  const shared_ptr<typename Element::Params> params = u.GetParams();
  auto zero_alloc = Element::Allocator(params, EVALUATION);

  double c = (base + 1) * SIGMA;

  const typename Element::Integer& modulus = A(0, 0).GetModulus();

  // It is assumed that A has dimension 1 x (k + 2) and pHat has the dimension
  // of (k + 2) x 1 perturbedSyndrome is in the evaluation representation
  Element perturbedSyndrome = u - (A.Mult(*pHat))(0, 0);

  Matrix<int64_t> zHatBBI([]() { return 0; }, k, n);

  // converting perturbed syndrome to coefficient representation
  perturbedSyndrome.SwitchFormat();

  LatticeGaussSampUtility<Element>::GaussSampGqArbBase(
      perturbedSyndrome, c, k, modulus, base, dgg, &zHatBBI);

  // Convert zHat from a matrix of integers to a vector of Element ring elements
  // zHat is in the coefficient representation
  Matrix<Element> zHat = SplitInt64AltIntoElements<Element>(zHatBBI, n, params);
  // Now converting it to the evaluation representation before multiplication
  zHat.SwitchFormat();

  Matrix<Element> zHatPrime(zero_alloc, k + 2, 1);

  zHatPrime(0, 0) = (*pHat)(0, 0) + T.m_e.Mult(zHat)(0, 0);
  zHatPrime(1, 0) = (*pHat)(1, 0) + T.m_r.Mult(zHat)(0, 0);

  for (size_t row = 2; row < k + 2; ++row)
    zHatPrime(row, 0) = (*pHat)(row, 0) + zHat(row - 2, 0);

  return zHatPrime;
}

// Offline stage of pre-image sampling (perturbation sampling)

template <class Element>
shared_ptr<Matrix<Element>> RLWETrapdoorUtility<Element>::GaussSampOffline(
    size_t n, size_t k, const RLWETrapdoorPair<Element>& T,
    typename Element::DggType& dgg, typename Element::DggType& dggLargeSigma,
    int64_t base) {
  const shared_ptr<typename Element::Params> params = T.m_e(0, 0).GetParams();
  auto zero_alloc = Element::Allocator(params, EVALUATION);

  double c = (base + 1) * SIGMA;

  // spectral bound s
  double s = SPECTRAL_BOUND(n, k, base);

  // perturbation vector in evaluation representation
  shared_ptr<Matrix<Element>> result(new Matrix<Element>(zero_alloc, k + 2, 1));
  ZSampleSigmaP(n, s, c, T, dgg, dggLargeSigma, result);

  return result;
}

template <>
inline void RLWETrapdoorUtility<DCRTPoly>::ZSampleSigmaP(
    size_t n, double s, double sigma, const RLWETrapdoorPair<DCRTPoly>& Tprime,
    const DCRTPoly::DggType& dgg, const DCRTPoly::DggType& dggLargeSigma,
    shared_ptr<Matrix<DCRTPoly>> perturbationVector) {
  DEBUG_FLAG(false);
  TimeVar t1, t2, t3, t1_tot, t2_tot, t3_tot;

  TIC(t1);
  TIC(t1_tot);
  Matrix<DCRTPoly> Tprime0 = Tprime.m_e;
  Matrix<DCRTPoly> Tprime1 = Tprime.m_r;
  // k is the bit length
  size_t k = Tprime0.GetCols();

  const shared_ptr<DCRTPoly::Params> params = Tprime0(0, 0).GetParams();

  DEBUG("z1a: " << TOC(t1));  // 0
  TIC(t1);
  // all three Polynomials are initialized with "0" coefficients
  NativePoly va((*params)[0], EVALUATION, 1);
  NativePoly vb((*params)[0], EVALUATION, 1);
  NativePoly vd((*params)[0], EVALUATION, 1);

  for (size_t i = 0; i < k; i++) {
    va += (NativePoly)Tprime0(0, i).GetElementAtIndex(0) *
          Tprime0(0, i).Transpose().GetElementAtIndex(0);
    vb += (NativePoly)Tprime1(0, i).GetElementAtIndex(0) *
          Tprime0(0, i).Transpose().GetElementAtIndex(0);
    vd += (NativePoly)Tprime1(0, i).GetElementAtIndex(0) *
          Tprime1(0, i).Transpose().GetElementAtIndex(0);
  }
  DEBUG("z1b: " << TOC(t1));  // 9
  TIC(t1);

  // Switch the ring elements (Polynomials) to coefficient representation
  va.SwitchFormat();
  vb.SwitchFormat();
  vd.SwitchFormat();

  DEBUG("z1c: " << TOC(t1));  // 5
  TIC(t1);

  // Create field elements from ring elements
  Field2n a(va), b(vb), d(vd);

  double scalarFactor = -s * s * sigma * sigma / (s * s - sigma * sigma);

  a = a.ScalarMult(scalarFactor);
  b = b.ScalarMult(scalarFactor);
  d = d.ScalarMult(scalarFactor);

  a = a + s * s;
  d = d + s * s;
  DEBUG("z1d: " << TOC(t1));  // 0
  TIC(t1);

  // converts the field elements to DFT representation
  a.SwitchFormat();
  b.SwitchFormat();
  d.SwitchFormat();
  DEBUG("z1e: " << TOC(t1));  // 0
  TIC(t1);

  Matrix<int64_t> p2ZVector([]() { return 0; }, n * k, 1);

  double sigmaLarge = sqrt(s * s - sigma * sigma);

  // for distribution parameters up to KARNEY_THRESHOLD (experimentally found
  // threshold) use the Peikert's inversion method otherwise, use Karney's
  // method
  if (sigmaLarge > KARNEY_THRESHOLD) {
    // Karney rejection sampling method
    for (size_t i = 0; i < n * k; i++) {
      p2ZVector(i, 0) = dgg.GenerateIntegerKarney(0, sigmaLarge);
    }
  } else {
    // Peikert's inversion sampling method
    std::shared_ptr<int32_t> dggVector = dggLargeSigma.GenerateIntVector(n * k);

    for (size_t i = 0; i < n * k; i++) {
      p2ZVector(i, 0) = (dggVector.get())[i];
    }
  }
  DEBUG("z1f1: " << TOC(t1));
  TIC(t1);

  // create k ring elements in coefficient representation
  Matrix<DCRTPoly> p2 = SplitInt64IntoElements<DCRTPoly>(p2ZVector, n, params);
  DEBUG("z1f2: " << TOC(t1));
  TIC(t1);

  // now converting to evaluation representation before multiplication
  p2.SwitchFormat();

  DEBUG("z1g: " << TOC(t1));  // 17

  TIC(t1);

  auto zero_alloc = NativePoly::Allocator((*params)[0], EVALUATION);
  Matrix<NativePoly> Tp2(zero_alloc, 2, 1);
  for (unsigned int i = 0; i < k; i++) {
    Tp2(0, 0) += Tprime0(0, i).GetElementAtIndex(0) *
                 (NativePoly)p2(i, 0).GetElementAtIndex(0);
    Tp2(1, 0) += Tprime1(0, i).GetElementAtIndex(0) *
                 (NativePoly)p2(i, 0).GetElementAtIndex(0);
  }

  DEBUG("z1h2: " << TOC(t1));
  TIC(t1);
  // change to coefficient representation before converting to field elements
  Tp2.SwitchFormat();
  DEBUG("z1h3: " << TOC(t1));
  TIC(t1);

  Matrix<Field2n> c([]() { return Field2n(); }, 2, 1);

  c(0, 0) =
      Field2n(Tp2(0, 0)).ScalarMult(-sigma * sigma / (s * s - sigma * sigma));
  c(1, 0) =
      Field2n(Tp2(1, 0)).ScalarMult(-sigma * sigma / (s * s - sigma * sigma));

  shared_ptr<Matrix<int64_t>> p1ZVector(
      new Matrix<int64_t>([]() { return 0; }, n * 2, 1));
  DEBUG("z1i: " << TOC(t1));
  TIC(t1);

  LatticeGaussSampUtility<DCRTPoly>::ZSampleSigma2x2(a, b, d, c, dgg,
                                                     p1ZVector);
  DEBUG("z1j1: " << TOC(t1));  // 14
  TIC(t1);

  // create 2 ring elements in coefficient representation
  Matrix<DCRTPoly> p1 = SplitInt64IntoElements<DCRTPoly>(*p1ZVector, n, params);
  DEBUG("z1j2: " << TOC(t1));
  TIC(t1);

  // Converts p1 to Evaluation representation
  p1.SwitchFormat();
  DEBUG("z1j3: " << TOC(t1));
  TIC(t1);

  *perturbationVector = p1.VStack(p2);
  DEBUG("z1j4: " << TOC(t1));
  TIC(t1);
  DEBUG("z1tot: " << TOC(t1_tot));
}

}  // namespace lbcrypto
#endif
