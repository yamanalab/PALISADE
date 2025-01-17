/**
 * @file be4-math-impl.cpp This file contains template instantiations for all
 * math classes & functions using math be4
 *
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

#include "math/backend.h"
#include "math/matrix.h"

#include "math/binaryuniformgenerator.cpp"
#include "math/ternaryuniformgenerator.cpp"
#include "math/discreteuniformgenerator.cpp"
#include "math/discretegaussiangenerator.cpp"
#include "math/nbtheory.cpp"
#include "math/transfrm.cpp"
#include "math/matrix.cpp"

namespace lbcrypto {

template class DiscreteGaussianGeneratorImpl<M4Vector>;
template class BinaryUniformGeneratorImpl<M4Vector>;
template class TernaryUniformGeneratorImpl<M4Vector>;
template class DiscreteUniformGeneratorImpl<M4Vector>;
template class ChineseRemainderTransformFTT<M4Vector>;
template class ChineseRemainderTransformArb<M4Vector>;

template M4Integer RootOfUnity<M4Integer>(usint m, const M4Integer &modulo);
template std::vector<M4Integer> RootsOfUnity(
    usint m, const std::vector<M4Integer> moduli);
template M4Integer GreatestCommonDivisor(const M4Integer &a,
                                         const M4Integer &b);
template bool MillerRabinPrimalityTest(const M4Integer &p, const usint niter);
template const M4Integer PollardRhoFactorization(const M4Integer &n);
template void PrimeFactorize(M4Integer n, std::set<M4Integer> &primeFactors);
template M4Integer FirstPrime(uint64_t nBits, uint64_t m);
template M4Integer NextPrime(const M4Integer &q, usint cyclotomicOrder);
template M4Integer PreviousPrime(const M4Integer &q, usint cyclotomicOrder);
template std::vector<M4Integer> GetTotientList(const M4Integer &n);
template M4Vector PolyMod(const M4Vector &dividend, const M4Vector &divisor,
                          const M4Integer &modulus);
template M4Vector PolynomialMultiplication(const M4Vector &a,
                                           const M4Vector &b);
template M4Vector GetCyclotomicPolynomial(usint m, const M4Integer &modulus);
template M4Integer SyntheticRemainder(const M4Vector &dividend,
                                      const M4Integer &a,
                                      const M4Integer &modulus);
template M4Vector SyntheticPolyRemainder(const M4Vector &dividend,
                                         const M4Vector &aList,
                                         const M4Integer &modulus);
template M4Vector PolynomialPower<M4Vector>(const M4Vector &input, usint power);
template M4Vector SyntheticPolynomialDivision(const M4Vector &dividend,
                                              const M4Integer &a,
                                              const M4Integer &modulus);
template M4Integer FindGeneratorCyclic(const M4Integer &modulo);
template bool IsGenerator(const M4Integer &g, const M4Integer &modulo);
template std::shared_ptr<std::vector<int64_t>> GetDigits(const M4Integer &u,
                                                         uint64_t base,
                                                         uint32_t k);

template class Matrix<M4Integer>;
ONES_FOR_TYPE(M4Integer)
IDENTITY_FOR_TYPE(M4Integer)
GADGET_FOR_TYPE(M4Integer)

template class Matrix<M4Vector>;
ONES_FOR_TYPE(M4Vector)
IDENTITY_FOR_TYPE(M4Vector)
GADGET_FOR_TYPE(M4Vector)

}  // namespace lbcrypto

CEREAL_CLASS_VERSION(M4Integer, M4Integer::SerializedVersion());
CEREAL_CLASS_VERSION(M4Vector, M4Vector::SerializedVersion());
