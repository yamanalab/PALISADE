/*
 * @file transfrm.cpp This file contains the linear transform interface
 * functionality.
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

#include <mutex>

#include "math/transfrm.h"
#include "utils/defines.h"

#ifdef WITH_INTEL_HEXL
#include "hexl/hexl.hpp"
#endif

namespace lbcrypto {

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformFTT<VecType>::m_cycloOrderInverseTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, NativeVector> ChineseRemainderTransformFTT<
    VecType>::m_cycloOrderInversePreconTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformFTT<VecType>::m_rootOfUnityReverseTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, VecType> ChineseRemainderTransformFTT<
    VecType>::m_rootOfUnityInverseReverseTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, NativeVector> ChineseRemainderTransformFTT<
    VecType>::m_rootOfUnityPreconReverseTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, NativeVector> ChineseRemainderTransformFTT<
    VecType>::m_rootOfUnityInversePreconReverseTableByModulus;

#ifdef WITH_INTEL_HEXL
template <typename VecType>
// N, modulus
std::unordered_map<std::pair<uint64_t, uint64_t>, intel::hexl::NTT, HashPair>
    ChineseRemainderTransformFTT<VecType>::m_IntelNtt;
template <typename VecType>
std::mutex ChineseRemainderTransformFTT<VecType>::m_mtxIntelNTT;
#endif

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformArb<VecType>::m_cyclotomicPolyMap;

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformArb<VecType>::m_cyclotomicPolyReverseNTTMap;

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformArb<VecType>::m_cyclotomicPolyNTTMap;

template <typename VecType>
std::map<ModulusRoot<typename VecType::Integer>, VecType>
    BluesteinFFT<VecType>::m_rootOfUnityTableByModulusRoot;

template <typename VecType>
std::map<ModulusRoot<typename VecType::Integer>, VecType>
    BluesteinFFT<VecType>::m_rootOfUnityInverseTableByModulusRoot;

template <typename VecType>
std::map<ModulusRoot<typename VecType::Integer>, VecType>
    BluesteinFFT<VecType>::m_powersTableByModulusRoot;

template <typename VecType>
std::map<ModulusRootPair<typename VecType::Integer>, VecType>
    BluesteinFFT<VecType>::m_RBTableByModulusRootPair;

template <typename VecType>
std::map<typename VecType::Integer, ModulusRoot<typename VecType::Integer>>
    BluesteinFFT<VecType>::m_defaultNTTModulusRoot;

template <typename VecType>
std::map<typename VecType::Integer, VecType>
    ChineseRemainderTransformArb<VecType>::m_rootOfUnityDivisionTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, VecType> ChineseRemainderTransformArb<
    VecType>::m_rootOfUnityDivisionInverseTableByModulus;

template <typename VecType>
std::map<typename VecType::Integer, typename VecType::Integer>
    ChineseRemainderTransformArb<VecType>::m_DivisionNTTModulus;

template <typename VecType>
std::map<typename VecType::Integer, typename VecType::Integer>
    ChineseRemainderTransformArb<VecType>::m_DivisionNTTRootOfUnity;

template <typename VecType>
std::map<usint, usint> ChineseRemainderTransformArb<VecType>::m_nttDivisionDim;

template <typename VecType>
void NumberTheoreticTransform<VecType>::ForwardTransformIterative(
    const VecType &element, const VecType &rootOfUnityTable, VecType *result) {
  usint n = element.GetLength();
  if (result->GetLength() != n) {
    PALISADE_THROW(
        math_error,
        "size of input element and size of output element not of same size");
  }

  auto modulus = element.GetModulus();
  IntType mu = modulus.ComputeMu();
  result->SetModulus(modulus);

  usint msb = GetMSB64(n - 1);
  for (size_t i = 0; i < n; i++) {
    (*result)[i] = element[ReverseBits(i, msb)];
  }

  IntType omega, omegaFactor, oddVal, evenVal;
  usint logm, i, j, indexEven, indexOdd;

  usint logn = GetMSB64(n - 1);
  for (logm = 1; logm <= logn; logm++) {
    // calculate the i indexes into the root table one time per loop
    vector<usint> indexes(1 << (logm - 1));
    for (i = 0; i < (usint)(1 << (logm - 1)); i++) {
      indexes[i] = (i << (logn - logm));
    }

    for (j = 0; j < n; j = j + (1 << logm)) {
      for (i = 0; i < (usint)(1 << (logm - 1)); i++) {
        omega = rootOfUnityTable[indexes[i]];
        indexEven = j + i;
        indexOdd = indexEven + (1 << (logm - 1));
        oddVal = (*result)[indexOdd];

        omegaFactor = omega.ModMul(oddVal, modulus, mu);
        evenVal = (*result)[indexEven];
        oddVal = evenVal;
        oddVal += omegaFactor;
        if (oddVal >= modulus) {
          oddVal -= modulus;
        }

        if (evenVal < omegaFactor) {
          evenVal += modulus;
        }
        evenVal -= omegaFactor;

        (*result)[indexEven] = oddVal;
        (*result)[indexOdd] = evenVal;
      }
    }
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::InverseTransformIterative(
    const VecType &element, const VecType &rootOfUnityInverseTable,
    VecType *result) {
  usint n = element.GetLength();

  IntType modulus = element.GetModulus();
  IntType mu = modulus.ComputeMu();

  NumberTheoreticTransform<VecType>::ForwardTransformIterative(
      element, rootOfUnityInverseTable, result);
  IntType cycloOrderInv(IntType(n).ModInverse(modulus));
  for (usint i = 0; i < n; i++) {
    (*result)[i].ModMulEq(cycloOrderInv, modulus, mu);
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::ForwardTransformToBitReverseInPlace(
    const VecType &rootOfUnityTable, VecType *element) {
  usint n = element->GetLength();
  IntType modulus = element->GetModulus();
  IntType mu = modulus.ComputeMu();

  usint i, m, j1, j2, indexOmega, indexLo, indexHi;
  IntType omega, omegaFactor, loVal, hiVal, zero(0);

  usint t = (n >> 1);
  usint logt1 = GetMSB64(t);
  for (m = 1; m < n; m <<= 1) {
    for (i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityTable[indexOmega];
      for (indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;
        loVal = (*element)[indexLo];
        omegaFactor = (*element)[indexHi];
        omegaFactor.ModMulFastEq(omega, modulus, mu);

        hiVal = loVal + omegaFactor;
        if (hiVal >= modulus) {
          hiVal -= modulus;
        }

        if (loVal < omegaFactor) {
          loVal += modulus;
        }
        loVal -= omegaFactor;

        (*element)[indexLo] = hiVal;
        (*element)[indexHi] = loVal;
      }
    }
    t >>= 1;
    logt1--;
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::ForwardTransformToBitReverse(
    const VecType &element, const VecType &rootOfUnityTable, VecType *result) {
  usint n = element.GetLength();
  if (result->GetLength() != n) {
    PALISADE_THROW(
        math_error,
        "size of input element and size of output element not of same size");
  }

  IntType modulus = element.GetModulus();
  IntType mu = modulus.ComputeMu();
  result->SetModulus(modulus);

  usint i, m, j1, j2, indexOmega, indexLo, indexHi;
  IntType omega, omegaFactor, loVal, hiVal, zero(0);

  for (i = 0; i < n; ++i) {
    (*result)[i] = element[i];
  }

  usint t = (n >> 1);
  usint logt1 = GetMSB64(t);
  for (m = 1; m < n; m <<= 1) {
    for (i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityTable[indexOmega];
      for (indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;
        loVal = (*result)[indexLo];
        omegaFactor = (*result)[indexHi];
        if (omegaFactor != zero) {
          omegaFactor.ModMulFastEq(omega, modulus, mu);

          hiVal = loVal + omegaFactor;
          if (hiVal >= modulus) {
            hiVal -= modulus;
          }

          if (loVal < omegaFactor) {
            loVal += modulus;
          }
          loVal -= omegaFactor;

          (*result)[indexLo] = hiVal;
          (*result)[indexHi] = loVal;
        } else {
          (*result)[indexHi] = loVal;
        }
      }
    }
    t >>= 1;
    logt1--;
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::ForwardTransformToBitReverseInPlace(
    const VecType &rootOfUnityTable, const NativeVector &preconRootOfUnityTable,
    VecType *element) {
  usint n = element->GetLength();
  IntType modulus = element->GetModulus();

  uint32_t indexOmega, indexHi;
  NativeInteger preconOmega;
  IntType omega, omegaFactor, loVal, hiVal, zero(0);

  usint t = (n >> 1);
  usint logt1 = GetMSB64(t);
  for (uint32_t m = 1; m < n; m <<= 1, t >>= 1, --logt1) {
    uint32_t j1, j2;
    for (uint32_t i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityTable[indexOmega];
      preconOmega = preconRootOfUnityTable[indexOmega];
      for (uint32_t indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;
        loVal = (*element)[indexLo];
        omegaFactor = (*element)[indexHi];
        omegaFactor.ModMulFastConstEq(omega, modulus, preconOmega);

        hiVal = loVal + omegaFactor;
        if (hiVal >= modulus) {
          hiVal -= modulus;
        }

        if (loVal < omegaFactor) {
          loVal += modulus;
        }
        loVal -= omegaFactor;

        (*element)[indexLo] = hiVal;
        (*element)[indexHi] = loVal;
      }
    }
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::ForwardTransformToBitReverse(
    const VecType &element, const VecType &rootOfUnityTable,
    const NativeVector &preconRootOfUnityTable, VecType *result) {
  usint n = element.GetLength();

  if (result->GetLength() != n) {
    PALISADE_THROW(
        math_error,
        "size of input element and size of output element not of same size");
  }

  IntType modulus = element.GetModulus();

  result->SetModulus(modulus);

  for (uint32_t i = 0; i < n; ++i) {
    (*result)[i] = element[i];
  }

  uint32_t indexOmega, indexHi;
  NativeInteger preconOmega;
  IntType omega, omegaFactor, loVal, hiVal, zero(0);

  usint t = (n >> 1);
  usint logt1 = GetMSB64(t);
  for (uint32_t m = 1; m < n; m <<= 1, t >>= 1, --logt1) {
    uint32_t j1, j2;
    for (uint32_t i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityTable[indexOmega];
      preconOmega = preconRootOfUnityTable[indexOmega];
      for (uint32_t indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;
        loVal = (*result)[indexLo];
        omegaFactor = (*result)[indexHi];
        if (omegaFactor != zero) {
          omegaFactor.ModMulFastConstEq(omega, modulus, preconOmega);

          hiVal = loVal + omegaFactor;
          if (hiVal >= modulus) {
            hiVal -= modulus;
          }

          if (loVal < omegaFactor) {
            loVal += modulus;
          }
          loVal -= omegaFactor;

          (*result)[indexLo] = hiVal;
          (*result)[indexHi] = loVal;
        } else {
          (*result)[indexHi] = loVal;
        }
      }
    }
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::InverseTransformFromBitReverseInPlace(
    const VecType &rootOfUnityInverseTable, const IntType &cycloOrderInv,
    VecType *element) {
  usint n = element->GetLength();
  IntType modulus = element->GetModulus();
  IntType mu = modulus.ComputeMu();

  IntType loVal, hiVal, omega, omegaFactor;
  usint i, m, j1, j2, indexOmega, indexLo, indexHi;

  usint t = 1;
  usint logt1 = 1;
  for (m = (n >> 1); m >= 1; m >>= 1) {
    for (i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityInverseTable[indexOmega];

      for (indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;

        hiVal = (*element)[indexHi];
        loVal = (*element)[indexLo];

        omegaFactor = loVal;
        if (omegaFactor < hiVal) {
          omegaFactor += modulus;
        }

        omegaFactor -= hiVal;

        loVal += hiVal;
        if (loVal >= modulus) {
          loVal -= modulus;
        }

        omegaFactor.ModMulFastEq(omega, modulus, mu);

        (*element)[indexLo] = loVal;
        (*element)[indexHi] = omegaFactor;
      }
    }
    t <<= 1;
    logt1++;
  }

  for (i = 0; i < n; i++) {
    (*element)[i].ModMulFastEq(cycloOrderInv, modulus, mu);
  }
  return;
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::InverseTransformFromBitReverse(
    const VecType &element, const VecType &rootOfUnityInverseTable,
    const IntType &cycloOrderInv, VecType *result) {
#if 0
	usint n = element.GetLength();

	if(result->GetLength() != n) {
		PALISADE_THROW(math_error, "size of input element and size of output element not of same size");
	}

	IntType modulus = element.GetModulus();
	IntType mu = modulus.ComputeMu();
	result->SetModulus(modulus);

	IntType loVal, hiVal, omega, omegaFactor;
	usint i, m, j1, j2, indexOmega, indexLo, indexHi;

	for (i = 0; i < n; i++) {
		(*result)[i]= element[i];
	}

	usint t = 1;
	usint logt1 = 1;
	for (m = (n >> 1); m >= 1; m >>= 1) {
		for (i = 0; i < m; ++i) {
			j1 = i << logt1;
			j2 = j1 + t;
			indexOmega = m + i;
			omega = rootOfUnityInverseTable[indexOmega];

			for (indexLo = j1; indexLo < j2; ++indexLo) {
				indexHi = indexLo + t;

				hiVal = (*result)[indexHi];
				loVal = (*result)[indexLo];

				omegaFactor = loVal;
				if (omegaFactor < hiVal) {
					omegaFactor += modulus;
				}

				omegaFactor -= hiVal;

				loVal += hiVal;
				if (loVal >= modulus) {
					loVal -= modulus;
				}

				omegaFactor.ModMulFastEq(omega, modulus, mu);

				(*result)[indexLo] = loVal;
				(*result)[indexHi] = omegaFactor;
			}
		}
		t <<= 1;
		logt1++;
	}

	for (i = 0; i < n; i++) {
		(*result)[i].ModMulFastEq(cycloOrderInv, modulus, mu);
	}
	return;
#else
  usint n = element.GetLength();

  if (result->GetLength() != n) {
    PALISADE_THROW(
        math_error,
        "size of input element and size of output element not of same size");
  }

  result->SetModulus(element.GetModulus());

  for (usint i = 0; i < n; i++) {
    (*result)[i] = element[i];
  }
  InverseTransformFromBitReverseInPlace(rootOfUnityInverseTable, cycloOrderInv,
                                        result);
#endif
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::InverseTransformFromBitReverseInPlace(
    const VecType &rootOfUnityInverseTable,
    const NativeVector &preconRootOfUnityInverseTable,
    const IntType &cycloOrderInv, const NativeInteger &preconCycloOrderInv,
    VecType *element) {
  usint n = element->GetLength();

  IntType modulus = element->GetModulus();

  IntType loVal, hiVal, omega, omegaFactor;
  NativeInteger preconOmega;
  usint i, m, j1, j2, indexOmega, indexLo, indexHi;

  usint t = 1;
  usint logt1 = 1;
  for (m = (n >> 1); m >= 1; m >>= 1) {
    for (i = 0; i < m; ++i) {
      j1 = i << logt1;
      j2 = j1 + t;
      indexOmega = m + i;
      omega = rootOfUnityInverseTable[indexOmega];
      preconOmega = preconRootOfUnityInverseTable[indexOmega];

      for (indexLo = j1; indexLo < j2; ++indexLo) {
        indexHi = indexLo + t;

        hiVal = (*element)[indexHi];
        loVal = (*element)[indexLo];

        omegaFactor = loVal;
        if (omegaFactor < hiVal) {
          omegaFactor += modulus;
        }

        omegaFactor -= hiVal;

        loVal += hiVal;
        if (loVal >= modulus) {
          loVal -= modulus;
        }

        omegaFactor.ModMulFastConstEq(omega, modulus, preconOmega);

        (*element)[indexLo] = loVal;
        (*element)[indexHi] = omegaFactor;
      }
    }
    t <<= 1;
    logt1++;
  }

  for (i = 0; i < n; i++) {
    (*element)[i].ModMulFastConstEq(cycloOrderInv, modulus,
                                    preconCycloOrderInv);
  }
}

template <typename VecType>
void NumberTheoreticTransform<VecType>::InverseTransformFromBitReverse(
    const VecType &element, const VecType &rootOfUnityInverseTable,
    const NativeVector &preconRootOfUnityInverseTable,
    const IntType &cycloOrderInv, const NativeInteger &preconCycloOrderInv,
    VecType *result) {
#if 0
	usint n = element.GetLength();
	if(result->GetLength() != n) {
		PALISADE_THROW(math_error, "size of input element and size of output element not of same size");
	}

	IntType modulus = element.GetModulus();
	result->SetModulus(modulus);

	IntType loVal, hiVal, omega, omegaFactor;
	NativeInteger preconOmega;
	usint i, m, j1, j2, indexOmega, indexLo, indexHi;

	for (i = 0; i < n; i++) {
		(*result)[i]= element[i];
	}

	usint t = 1;
	usint logt1 = 1;
	for (m = (n >> 1); m >= 1; m >>= 1) {
		for (i = 0; i < m; ++i) {
			j1 = i << logt1;
			j2 = j1 + t;
			indexOmega = m + i;
			omega = rootOfUnityInverseTable[indexOmega];
			preconOmega = preconRootOfUnityInverseTable[indexOmega];

			for (indexLo = j1; indexLo < j2; ++indexLo) {
				indexHi = indexLo + t;

				hiVal = (*result)[indexHi];
				loVal = (*result)[indexLo];

				omegaFactor = loVal;
				if (omegaFactor < hiVal) {
					omegaFactor += modulus;
				}

				omegaFactor -= hiVal;

				loVal += hiVal;
				if (loVal >= modulus) {
					loVal -= modulus;
				}

				omegaFactor.ModMulFastConstEq(omega, modulus, preconOmega);

				(*result)[indexLo] = loVal;
				(*result)[indexHi] = omegaFactor;
			}
		}
		t <<= 1;
		logt1++;
	}

	for (i = 0; i < n; i++) {
		(*result)[i].ModMulFastConstEq(cycloOrderInv, modulus, preconCycloOrderInv);
	}
	return;
#else
  usint n = element.GetLength();
  if (result->GetLength() != n) {
    PALISADE_THROW(
        math_error,
        "size of input element and size of output element not of same size");
  }

  result->SetModulus(element.GetModulus());

  for (usint i = 0; i < n; i++) {
    (*result)[i] = element[i];
  }
  InverseTransformFromBitReverseInPlace(
      rootOfUnityInverseTable, preconRootOfUnityInverseTable, cycloOrderInv,
      preconCycloOrderInv, result);

  return;
#endif
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::ForwardTransformToBitReverseInPlace(
    const IntType &rootOfUnity, const usint CycloOrder, VecType *element) {
  if (rootOfUnity == IntType(1) || rootOfUnity == IntType(0)) {
    return;
  }

  if (!IsPowerOfTwo(CycloOrder)) {
    PALISADE_THROW(math_error, "CyclotomicOrder is not a power of two");
  }

  usint CycloOrderHf = (CycloOrder >> 1);
  if (element->GetLength() != CycloOrderHf) {
    PALISADE_THROW(math_error,
                   "element size must be equal to CyclotomicOrder / 2");
  }

  IntType modulus = element->GetModulus();

  bool reCompute = false;
  PALISADE_UNUSED(reCompute);  // Used only when WITH_INTEL_HEXL=ON
  auto mapSearch = m_rootOfUnityReverseTableByModulus.find(modulus);
  if (mapSearch == m_rootOfUnityReverseTableByModulus.end() ||
      mapSearch->second.GetLength() != CycloOrderHf) {
    PreCompute(rootOfUnity, CycloOrder, modulus);
    reCompute = true;
  }

  if (typeid(IntType) == typeid(NativeInteger)) {
#ifdef WITH_INTEL_HEXL
    if (std::is_same<VecType, NativeVector>::value) {
      std::pair<uint64_t, uint64_t> key{element->GetLength(),
                                        modulus.ConvertToInt()};
      intel::hexl::NTT *p_ntt;
      std::unique_lock<std::mutex> lock(m_mtxIntelNTT);
      auto ntt_it = m_IntelNtt.find(key);
      if (reCompute || ntt_it == m_IntelNtt.end()) {
        intel::hexl::NTT ntt(element->GetLength(), modulus.ConvertToInt(),
                             rootOfUnity.ConvertToInt());
        m_IntelNtt[key] = std::move(ntt);
        ntt_it = m_IntelNtt.find(key);
      }
      p_ntt = &ntt_it->second;
      lock.unlock();

      auto *data = reinterpret_cast<uint64_t *>(&element->at(0));
      p_ntt->ComputeForward(data, data, 1, 1);
      element->SetModulus(modulus);
    } else {
      NumberTheoreticTransform<VecType>::ForwardTransformToBitReverseInPlace(
          m_rootOfUnityReverseTableByModulus[modulus],
          m_rootOfUnityPreconReverseTableByModulus[modulus], element);
    }
#else
    NumberTheoreticTransform<VecType>::ForwardTransformToBitReverseInPlace(
        m_rootOfUnityReverseTableByModulus[modulus],
        m_rootOfUnityPreconReverseTableByModulus[modulus], element);
#endif
  } else {
    NumberTheoreticTransform<VecType>::ForwardTransformToBitReverseInPlace(
        m_rootOfUnityReverseTableByModulus[modulus], element);
  }
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::ForwardTransformToBitReverse(
    const VecType &element, const IntType &rootOfUnity, const usint CycloOrder,
    VecType *result) {
  if (rootOfUnity == IntType(1) || rootOfUnity == IntType(0)) {
    *result = element;
    return;
  }

  if (!IsPowerOfTwo(CycloOrder)) {
    PALISADE_THROW(math_error, "CyclotomicOrder is not a power of two");
  }

  usint CycloOrderHf = (CycloOrder >> 1);
  if (result->GetLength() != CycloOrderHf) {
    PALISADE_THROW(math_error,
                   "result size must be equal to CyclotomicOrder / 2");
  }

  IntType modulus = element.GetModulus();

  bool reCompute = false;
  PALISADE_UNUSED(reCompute);  // Used only when WITH_INTEL_HEXL=ON
  auto mapSearch = m_rootOfUnityReverseTableByModulus.find(modulus);
  if (mapSearch == m_rootOfUnityReverseTableByModulus.end() ||
      mapSearch->second.GetLength() < CycloOrderHf) {
    PreCompute(rootOfUnity, CycloOrder, modulus);
    reCompute = true;
  }

  if (typeid(IntType) == typeid(NativeInteger)) {
    NumberTheoreticTransform<VecType>::ForwardTransformToBitReverse(
        element, m_rootOfUnityReverseTableByModulus[modulus],
        m_rootOfUnityPreconReverseTableByModulus[modulus], result);
  } else {
    NumberTheoreticTransform<VecType>::ForwardTransformToBitReverse(
        element, m_rootOfUnityReverseTableByModulus[modulus], result);
  }

  return;
}

template <typename VecType>
void ChineseRemainderTransformFTT<
    VecType>::InverseTransformFromBitReverseInPlace(const IntType &rootOfUnity,
                                                    const usint CycloOrder,
                                                    VecType *element) {
  if (rootOfUnity == IntType(1) || rootOfUnity == IntType(0)) {
    return;
  }

  if (!IsPowerOfTwo(CycloOrder)) {
    PALISADE_THROW(math_error, "CyclotomicOrder is not a power of two");
  }

  usint CycloOrderHf = (CycloOrder >> 1);
  if (element->GetLength() != CycloOrderHf) {
    PALISADE_THROW(math_error,
                   "element size must be equal to CyclotomicOrder / 2");
  }

  IntType modulus = element->GetModulus();

  bool reCompute = false;
  PALISADE_UNUSED(reCompute);  // Used only when WITH_INTEL_HEXL=ON
  auto mapSearch = m_rootOfUnityReverseTableByModulus.find(modulus);
  if (mapSearch == m_rootOfUnityReverseTableByModulus.end() ||
      mapSearch->second.GetLength() != CycloOrderHf) {
    PreCompute(rootOfUnity, CycloOrder, modulus);
    reCompute = true;
  }

  usint msb = GetMSB64(CycloOrderHf - 1);
  if (typeid(IntType) == typeid(NativeInteger)) {
#ifdef WITH_INTEL_HEXL
    if (std::is_same<VecType, NativeVector>::value) {
      std::pair<uint64_t, uint64_t> key{element->GetLength(),
                                        modulus.ConvertToInt()};
      intel::hexl::NTT *p_ntt;
      std::unique_lock<std::mutex> lock(m_mtxIntelNTT);
      auto ntt_it = m_IntelNtt.find(key);
      if (reCompute || ntt_it == m_IntelNtt.end()) {
        intel::hexl::NTT ntt(element->GetLength(), modulus.ConvertToInt(),
                             rootOfUnity.ConvertToInt());
        m_IntelNtt[key] = std::move(ntt);
        ntt_it = m_IntelNtt.find(key);
      }
      p_ntt = &ntt_it->second;
      lock.unlock();
      auto *data = reinterpret_cast<uint64_t *>(&element->at(0));
      p_ntt->ComputeInverse(data, data, 1, 1);
      element->SetModulus(modulus);
    } else {
      NumberTheoreticTransform<VecType>::InverseTransformFromBitReverseInPlace(
          m_rootOfUnityInverseReverseTableByModulus[modulus],
          m_rootOfUnityInversePreconReverseTableByModulus[modulus],
          m_cycloOrderInverseTableByModulus[modulus][msb],
          m_cycloOrderInversePreconTableByModulus[modulus][msb], element);
    }
#else
    NumberTheoreticTransform<VecType>::InverseTransformFromBitReverseInPlace(
        m_rootOfUnityInverseReverseTableByModulus[modulus],
        m_rootOfUnityInversePreconReverseTableByModulus[modulus],
        m_cycloOrderInverseTableByModulus[modulus][msb],
        m_cycloOrderInversePreconTableByModulus[modulus][msb], element);
#endif
  } else {
    NumberTheoreticTransform<VecType>::InverseTransformFromBitReverseInPlace(
        m_rootOfUnityInverseReverseTableByModulus[modulus],
        m_cycloOrderInverseTableByModulus[modulus][msb], element);
  }
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::InverseTransformFromBitReverse(
    const VecType &element, const IntType &rootOfUnity, const usint CycloOrder,
    VecType *result) {
  if (rootOfUnity == IntType(1) || rootOfUnity == IntType(0)) {
    *result = element;
    return;
  }

  if (!IsPowerOfTwo(CycloOrder)) {
    PALISADE_THROW(math_error, "CyclotomicOrder is not a power of two");
  }

  usint CycloOrderHf = (CycloOrder >> 1);
  if (result->GetLength() != CycloOrderHf) {
    PALISADE_THROW(math_error,
                   "result size must be equal to CyclotomicOrder / 2");
  }

  IntType modulus = element.GetModulus();

  bool reCompute = false;
  PALISADE_UNUSED(reCompute);  // Used only when WITH_INTEL_HEXL=ON
  auto mapSearch = m_rootOfUnityReverseTableByModulus.find(modulus);
  if (mapSearch == m_rootOfUnityReverseTableByModulus.end() ||
      mapSearch->second.GetLength() < CycloOrderHf) {
    PreCompute(rootOfUnity, CycloOrder, modulus);
    reCompute = true;
  }

  usint msb = GetMSB64(CycloOrderHf - 1);
  if (typeid(IntType) == typeid(NativeInteger)) {
    NumberTheoreticTransform<VecType>::InverseTransformFromBitReverse(
        element, m_rootOfUnityInverseReverseTableByModulus[modulus],
        m_rootOfUnityInversePreconReverseTableByModulus[modulus],
        m_cycloOrderInverseTableByModulus[modulus][msb],
        m_cycloOrderInversePreconTableByModulus[modulus][msb], result);
  } else {
    NumberTheoreticTransform<VecType>::InverseTransformFromBitReverse(
        element, m_rootOfUnityInverseReverseTableByModulus[modulus],
        m_cycloOrderInverseTableByModulus[modulus][msb], result);
  }

  return;
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::PreCompute(
    const IntType &rootOfUnity, const usint CycloOrder,
    const IntType &modulus) {
  usint CycloOrderHf = (CycloOrder >> 1);

  auto mapSearch = m_rootOfUnityReverseTableByModulus.find(modulus);
  if (mapSearch == m_rootOfUnityReverseTableByModulus.end() ||
      mapSearch->second.GetLength() < CycloOrderHf) {
#pragma omp critical
    {
      IntType x(1), xinv(1);
      usint msb = GetMSB64(CycloOrderHf - 1);
      IntType mu = modulus.ComputeMu();
      VecType Table(CycloOrderHf, modulus);
      VecType TableI(CycloOrderHf, modulus);
      IntType rootOfUnityInverse = rootOfUnity.ModInverse(modulus);
      usint iinv;
      for (usint i = 0; i < CycloOrderHf; i++) {
        iinv = ReverseBits(i, msb);
        Table[iinv] = x;
        TableI[iinv] = xinv;
        x.ModMulEq(rootOfUnity, modulus, mu);
        xinv.ModMulEq(rootOfUnityInverse, modulus, mu);
      }
      m_rootOfUnityReverseTableByModulus[modulus] = Table;
      m_rootOfUnityInverseReverseTableByModulus[modulus] = TableI;

      VecType TableCOI(msb + 1, modulus);
      for (usint i = 0; i < msb + 1; i++) {
        IntType coInv(IntType(1 << i).ModInverse(modulus));
        TableCOI[i] = coInv;
      }
      m_cycloOrderInverseTableByModulus[modulus] = TableCOI;

      if (typeid(IntType) == typeid(NativeInteger)) {
        NativeInteger nativeModulus = modulus.ConvertToInt();
        NativeVector preconTable(CycloOrderHf, nativeModulus);
        NativeVector preconTableI(CycloOrderHf, nativeModulus);

        for (usint i = 0; i < CycloOrderHf; i++) {
          preconTable[i] =
              NativeInteger(m_rootOfUnityReverseTableByModulus[modulus]
                                .
                                operator[](i)
                                .ConvertToInt())
                  .PrepModMulConst(nativeModulus);
          preconTableI[i] =
              NativeInteger(m_rootOfUnityInverseReverseTableByModulus[modulus]
                                .
                                operator[](i)
                                .ConvertToInt())
                  .PrepModMulConst(nativeModulus);
        }

        NativeVector preconTableCOI(msb + 1, nativeModulus);
        for (usint i = 0; i < msb + 1; i++) {
          preconTableCOI[i] =
              NativeInteger(m_cycloOrderInverseTableByModulus[modulus]
                                .
                                operator[](i)
                                .ConvertToInt())
                  .PrepModMulConst(nativeModulus);
        }

        m_rootOfUnityPreconReverseTableByModulus[modulus] = preconTable;
        m_rootOfUnityInversePreconReverseTableByModulus[modulus] = preconTableI;
        m_cycloOrderInversePreconTableByModulus[modulus] = preconTableCOI;
      }
    }
  }
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::PreCompute(
    std::vector<IntType> &rootOfUnity, const usint CycloOrder,
    std::vector<IntType> &moduliiChain) {
  usint numOfRootU = rootOfUnity.size();
  usint numModulii = moduliiChain.size();

  if (numOfRootU != numModulii) {
    PALISADE_THROW(
        math_error,
        "size of root of unity and size of moduli chain not of same size");
  }

  for (usint i = 0; i < numOfRootU; ++i) {
    IntType currentRoot(rootOfUnity[i]);
    IntType currentMod(moduliiChain[i]);
    PreCompute(currentRoot, CycloOrder, currentMod);
  }
}

template <typename VecType>
void ChineseRemainderTransformFTT<VecType>::Reset() {
  m_cycloOrderInverseTableByModulus.clear();
  m_cycloOrderInversePreconTableByModulus.clear();
  m_rootOfUnityReverseTableByModulus.clear();
  m_rootOfUnityInverseReverseTableByModulus.clear();
  m_rootOfUnityPreconReverseTableByModulus.clear();
  m_rootOfUnityInversePreconReverseTableByModulus.clear();
}

template <typename VecType>
void BluesteinFFT<VecType>::PreComputeDefaultNTTModulusRoot(
    usint cycloOrder, const IntType &modulus) {
  usint nttDim = pow(2, ceil(log2(2 * cycloOrder - 1)));
  const auto nttModulus =
      FirstPrime<IntType>(log2(nttDim) + 2 * modulus.GetMSB(), nttDim);
  const auto nttRoot = RootOfUnity(nttDim, nttModulus);
  const ModulusRoot<IntType> nttModulusRoot = {nttModulus, nttRoot};
  m_defaultNTTModulusRoot[modulus] = nttModulusRoot;

  PreComputeRootTableForNTT(cycloOrder, nttModulusRoot);
}

template <typename VecType>
void BluesteinFFT<VecType>::PreComputeRootTableForNTT(
    usint cyclotoOrder, const ModulusRoot<IntType> &nttModulusRoot) {
  usint nttDim = pow(2, ceil(log2(2 * cyclotoOrder - 1)));
  const auto &nttModulus = nttModulusRoot.first;
  const auto &nttRoot = nttModulusRoot.second;

  IntType root(nttRoot);

  auto rootInv = root.ModInverse(nttModulus);

  usint nttDimHf = (nttDim >> 1);
  VecType rootTable(nttDimHf, nttModulus);
  VecType rootTableInverse(nttDimHf, nttModulus);

  IntType x(1);
  for (usint i = 0; i < nttDimHf; i++) {
    rootTable[i] = x;
    x = x.ModMul(root, nttModulus);
  }

  x = 1;
  for (usint i = 0; i < nttDimHf; i++) {
    rootTableInverse[i] = x;
    x = x.ModMul(rootInv, nttModulus);
  }

  m_rootOfUnityTableByModulusRoot[nttModulusRoot] = rootTable;
  m_rootOfUnityInverseTableByModulusRoot[nttModulusRoot] = rootTableInverse;
}

template <typename VecType>
void BluesteinFFT<VecType>::PreComputePowers(
    usint cycloOrder, const ModulusRoot<IntType> &modulusRoot) {
  const auto &modulus = modulusRoot.first;
  const auto &root = modulusRoot.second;

  VecType powers(cycloOrder, modulus);
  powers[0] = 1;
  for (usint i = 1; i < cycloOrder; i++) {
    auto iSqr = (i * i) % (2 * cycloOrder);
    auto val = root.ModExp(IntType(iSqr), modulus);
    powers[i] = val;
  }
  m_powersTableByModulusRoot[modulusRoot] = powers;
}

template <typename VecType>
void BluesteinFFT<VecType>::PreComputeRBTable(
    usint cycloOrder, const ModulusRootPair<IntType> &modulusRootPair) {
  const auto &modulusRoot = modulusRootPair.first;
  const auto &modulus = modulusRoot.first;
  const auto &root = modulusRoot.second;
  const auto rootInv = root.ModInverse(modulus);

  const auto &nttModulusRoot = modulusRootPair.second;
  const auto &nttModulus = nttModulusRoot.first;
  // const auto &nttRoot = nttModulusRoot.second;
  const auto &rootTable =
      m_rootOfUnityTableByModulusRoot[nttModulusRoot];  // assumes rootTable is
                                                        // precomputed
  usint nttDim = pow(2, ceil(log2(2 * cycloOrder - 1)));

  VecType b(2 * cycloOrder - 1, modulus);
  b[cycloOrder - 1] = 1;
  for (usint i = 1; i < cycloOrder; i++) {
    auto iSqr = (i * i) % (2 * cycloOrder);
    auto val = rootInv.ModExp(IntType(iSqr), modulus);
    b[cycloOrder - 1 + i] = val;
    b[cycloOrder - 1 - i] = val;
  }

  auto Rb = PadZeros(b, nttDim);
  Rb.SetModulus(nttModulus);

  VecType RB(nttDim);
  NumberTheoreticTransform<VecType>::ForwardTransformIterative(Rb, rootTable,
                                                               &RB);
  m_RBTableByModulusRootPair[modulusRootPair] = RB;
}

template <typename VecType>
VecType BluesteinFFT<VecType>::ForwardTransform(const VecType &element,
                                                const IntType &root,
                                                const usint cycloOrder) {
  const auto &modulus = element.GetModulus();
  const auto &nttModulusRoot = m_defaultNTTModulusRoot[modulus];

  return ForwardTransform(element, root, cycloOrder, nttModulusRoot);
}

template <typename VecType>
VecType BluesteinFFT<VecType>::ForwardTransform(
    const VecType &element, const IntType &root, const usint cycloOrder,
    const ModulusRoot<IntType> &nttModulusRoot) {
  if (element.GetLength() != cycloOrder) {
    PALISADE_THROW(
        math_error,
        "expected size of element vector should be equal to cyclotomic order");
  }

  const auto &modulus = element.GetModulus();
  const ModulusRoot<IntType> modulusRoot = {modulus, root};
  const VecType &powers = m_powersTableByModulusRoot[modulusRoot];

  const auto &nttModulus = nttModulusRoot.first;
  const auto &rootTable =
      m_rootOfUnityTableByModulusRoot[nttModulusRoot];  // assumes rootTable is
                                                        // precomputed
  const auto &rootTableInverse = m_rootOfUnityInverseTableByModulusRoot
      [nttModulusRoot];  // assumes rootTableInverse is precomputed
  VecType x = element.ModMul(powers);

  usint nttDim = pow(2, ceil(log2(2 * cycloOrder - 1)));
  auto Ra = PadZeros(x, nttDim);
  Ra.SetModulus(nttModulus);
  VecType RA(nttDim);
  NumberTheoreticTransform<VecType>::ForwardTransformIterative(Ra, rootTable,
                                                               &RA);

  const ModulusRootPair<IntType> modulusRootPair = {modulusRoot,
                                                    nttModulusRoot};
  const auto &RB = m_RBTableByModulusRootPair[modulusRootPair];

  auto RC = RA.ModMul(RB);
  VecType Rc(nttDim);
  NumberTheoreticTransform<VecType>::InverseTransformIterative(
      RC, rootTableInverse, &Rc);
  auto resizeRc = Resize(Rc, cycloOrder - 1, 2 * (cycloOrder - 1));
  resizeRc.SetModulus(modulus);
  resizeRc.ModEq(modulus);
  auto result = resizeRc.ModMul(powers);

  return result;
}

template <typename VecType>
VecType BluesteinFFT<VecType>::PadZeros(const VecType &a,
                                        const usint finalSize) {
  usint s = a.GetLength();
  VecType result(finalSize, a.GetModulus());

  for (usint i = 0; i < s; i++) {
    result[i] = a[i];
  }

  for (usint i = a.GetLength(); i < finalSize; i++) {
    result[i] = IntType(0);
  }

  return result;
}

template <typename VecType>
VecType BluesteinFFT<VecType>::Resize(const VecType &a, usint lo, usint hi) {
  VecType result(hi - lo + 1, a.GetModulus());

  for (usint i = lo, j = 0; i <= hi; i++, j++) {
    result[j] = a[i];
  }

  return result;
}

template <typename VecType>
void BluesteinFFT<VecType>::Reset() {
  m_rootOfUnityTableByModulusRoot.clear();
  m_rootOfUnityInverseTableByModulusRoot.clear();
  m_powersTableByModulusRoot.clear();
  m_RBTableByModulusRootPair.clear();
  m_defaultNTTModulusRoot.clear();
}

template <typename VecType>
void ChineseRemainderTransformArb<VecType>::SetCylotomicPolynomial(
    const VecType &poly, const IntType &mod) {
  m_cyclotomicPolyMap[mod] = poly;
}

template <typename VecType>
void ChineseRemainderTransformArb<VecType>::PreCompute(const usint cyclotoOrder,
                                                       const IntType &modulus) {
  BluesteinFFT<VecType>::PreComputeDefaultNTTModulusRoot(cyclotoOrder, modulus);
}

template <typename VecType>
void ChineseRemainderTransformArb<VecType>::SetPreComputedNTTModulus(
    usint cyclotoOrder, const IntType &modulus, const IntType &nttModulus,
    const IntType &nttRoot) {
  const ModulusRoot<IntType> nttModulusRoot = {nttModulus, nttRoot};
  BluesteinFFT<VecType>::PreComputeRootTableForNTT(cyclotoOrder,
                                                   nttModulusRoot);
}

template <typename VecType>
void ChineseRemainderTransformArb<VecType>::SetPreComputedNTTDivisionModulus(
    usint cyclotoOrder, const IntType &modulus, const IntType &nttMod,
    const IntType &nttRootBig) {
  DEBUG_FLAG(false);

  usint n = GetTotient(cyclotoOrder);
  DEBUG("GetTotient(" << cyclotoOrder << ")= " << n);

  usint power = cyclotoOrder - n;
  m_nttDivisionDim[cyclotoOrder] = 2 * std::pow(2, ceil(log2(power)));

  usint nttDimBig = std::pow(2, ceil(log2(2 * cyclotoOrder - 1)));

  // Computes the root of unity for the division NTT based on the root of unity
  // for regular NTT
  IntType nttRoot = nttRootBig.ModExp(
      IntType(nttDimBig / m_nttDivisionDim[cyclotoOrder]), nttMod);

  m_DivisionNTTModulus[modulus] = nttMod;
  m_DivisionNTTRootOfUnity[modulus] = nttRoot;
  // part0 setting of rootTable and inverse rootTable
  usint nttDim = m_nttDivisionDim[cyclotoOrder];
  IntType root(nttRoot);
  auto rootInv = root.ModInverse(nttMod);

  usint nttDimHf = (nttDim >> 1);
  VecType rootTable(nttDimHf, nttMod);
  VecType rootTableInverse(nttDimHf, nttMod);

  IntType x(1);
  for (usint i = 0; i < nttDimHf; i++) {
    rootTable[i] = x;
    x = x.ModMul(root, nttMod);
  }

  x = 1;
  for (usint i = 0; i < nttDimHf; i++) {
    rootTableInverse[i] = x;
    x = x.ModMul(rootInv, nttMod);
  }

  m_rootOfUnityDivisionTableByModulus[nttMod] = rootTable;
  m_rootOfUnityDivisionInverseTableByModulus[nttMod] = rootTableInverse;

  // end of part0
  // part1
  const auto &RevCPM =
      InversePolyMod(m_cyclotomicPolyMap[modulus], modulus, power);
  auto RevCPMPadded = BluesteinFFT<VecType>::PadZeros(RevCPM, nttDim);
  RevCPMPadded.SetModulus(nttMod);
  // end of part1

  VecType RA(nttDim);
  NumberTheoreticTransform<VecType>::ForwardTransformIterative(RevCPMPadded,
                                                               rootTable, &RA);
  m_cyclotomicPolyReverseNTTMap[modulus] = RA;

  const auto &cycloPoly = m_cyclotomicPolyMap[modulus];

  VecType QForwardTransform(nttDim, nttMod);
  for (usint i = 0; i < cycloPoly.GetLength(); i++) {
    QForwardTransform[i] = cycloPoly[i];
  }

  VecType QFwdResult(nttDim);
  NumberTheoreticTransform<VecType>::ForwardTransformIterative(
      QForwardTransform, rootTable, &QFwdResult);

  m_cyclotomicPolyNTTMap[modulus] = QFwdResult;
}

template <typename VecType>
VecType ChineseRemainderTransformArb<VecType>::InversePolyMod(
    const VecType &cycloPoly, const IntType &modulus, usint power) {
  VecType result(power, modulus);
  usint r = ceil(log2(power));
  VecType h(1, modulus);  // h is a unit polynomial
  h[0] = 1;

  // Precompute the Barrett mu parameter
  IntType mu = modulus.ComputeMu();

  for (usint i = 0; i < r; i++) {
    usint qDegree = std::pow(2, i + 1);
    VecType q(qDegree + 1, modulus);  // q = x^(2^i+1)
    q[qDegree] = 1;
    auto hSquare = PolynomialMultiplication(h, h);

    auto a = h * IntType(2);
    auto b = PolynomialMultiplication(hSquare, cycloPoly);
    // b = 2h - gh^2
    for (usint j = 0; j < b.GetLength(); j++) {
      if (j < a.GetLength()) {
        b[j] = a[j].ModSub(b[j], modulus, mu);
      } else {
        b[j] = modulus.ModSub(b[j], modulus, mu);
      }
    }
    h = PolyMod(b, q, modulus);
  }
  // take modulo x^power
  for (usint i = 0; i < power; i++) {
    result[i] = h[i];
  }

  return result;
}

template <typename VecType>
VecType ChineseRemainderTransformArb<VecType>::ForwardTransform(
    const VecType &element, const IntType &root, const IntType &nttModulus,
    const IntType &nttRoot, const usint cycloOrder) {
  usint phim = GetTotient(cycloOrder);
  if (element.GetLength() != phim) {
    PALISADE_THROW(math_error, "element size should be equal to phim");
  }

  const auto &modulus = element.GetModulus();
  const ModulusRoot<IntType> modulusRoot = {modulus, root};

  const ModulusRoot<IntType> nttModulusRoot = {nttModulus, nttRoot};
  const ModulusRootPair<IntType> modulusRootPair = {modulusRoot,
                                                    nttModulusRoot};

#pragma omp critical
  {
    if (BluesteinFFT<VecType>::m_rootOfUnityTableByModulusRoot[nttModulusRoot]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputeRootTableForNTT(cycloOrder,
                                                       nttModulusRoot);
    }

    if (BluesteinFFT<VecType>::m_powersTableByModulusRoot[modulusRoot]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputePowers(cycloOrder, modulusRoot);
    }

    if (BluesteinFFT<VecType>::m_RBTableByModulusRootPair[modulusRootPair]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputeRBTable(cycloOrder, modulusRootPair);
    }
  }

  VecType inputToBluestein = Pad(element, cycloOrder, true);
  auto outputBluestein = BluesteinFFT<VecType>::ForwardTransform(
      inputToBluestein, root, cycloOrder, nttModulusRoot);
  VecType output = Drop(outputBluestein, cycloOrder, true, nttModulus, nttRoot);

  return output;
}

template <typename VecType>
VecType ChineseRemainderTransformArb<VecType>::InverseTransform(
    const VecType &element, const IntType &root, const IntType &nttModulus,
    const IntType &nttRoot, const usint cycloOrder) {
  usint phim = GetTotient(cycloOrder);
  if (element.GetLength() != phim) {
    PALISADE_THROW(math_error, "element size should be equal to phim");
  }

  const auto &modulus = element.GetModulus();
  auto rootInverse(root.ModInverse(modulus));
  const ModulusRoot<IntType> modulusRootInverse = {modulus, rootInverse};

  const ModulusRoot<IntType> nttModulusRoot = {nttModulus, nttRoot};
  const ModulusRootPair<IntType> modulusRootPair = {modulusRootInverse,
                                                    nttModulusRoot};

#pragma omp critical
  {
    if (BluesteinFFT<VecType>::m_rootOfUnityTableByModulusRoot[nttModulusRoot]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputeRootTableForNTT(cycloOrder,
                                                       nttModulusRoot);
    }

    if (BluesteinFFT<VecType>::m_powersTableByModulusRoot[modulusRootInverse]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputePowers(cycloOrder, modulusRootInverse);
    }

    if (BluesteinFFT<VecType>::m_RBTableByModulusRootPair[modulusRootPair]
            .GetLength() == 0) {
      BluesteinFFT<VecType>::PreComputeRBTable(cycloOrder, modulusRootPair);
    }
  }
  VecType inputToBluestein = Pad(element, cycloOrder, false);
  auto outputBluestein = BluesteinFFT<VecType>::ForwardTransform(
      inputToBluestein, rootInverse, cycloOrder, nttModulusRoot);
  auto cyclotomicInverse((IntType(cycloOrder)).ModInverse(modulus));
  outputBluestein = outputBluestein * cyclotomicInverse;
  VecType output =
      Drop(outputBluestein, cycloOrder, false, nttModulus, nttRoot);
  return output;
}

template <typename VecType>
VecType ChineseRemainderTransformArb<VecType>::Pad(const VecType &element,
                                                   const usint cycloOrder,
                                                   bool forward) {
  usint n = GetTotient(cycloOrder);

  const auto &modulus = element.GetModulus();
  VecType inputToBluestein(cycloOrder, modulus);

  if (forward) {  // Forward transform padding
    for (usint i = 0; i < n; i++) {
      inputToBluestein[i] = element[i];
    }
  } else {  // Inverse transform padding
    auto tList = GetTotientList(cycloOrder);
    usint i = 0;
    for (auto &coprime : tList) {
      inputToBluestein[coprime] = element[i++];
    }
  }

  return inputToBluestein;
}

template <typename VecType>
VecType ChineseRemainderTransformArb<VecType>::Drop(const VecType &element,
                                                    const usint cycloOrder,
                                                    bool forward,
                                                    const IntType &bigMod,
                                                    const IntType &bigRoot) {
  usint n = GetTotient(cycloOrder);

  const auto &modulus = element.GetModulus();
  VecType output(n, modulus);

  if (forward) {  // Forward transform drop
    auto tList = GetTotientList(cycloOrder);
    for (usint i = 0; i < n; i++) {
      output[i] = element[tList[i]];
    }
  } else {  // Inverse transform drop
    if ((n + 1) == cycloOrder) {
      IntType mu = modulus.ComputeMu();  // Precompute the Barrett mu parameter
      // cycloOrder is prime: Reduce mod Phi_{n+1}(x)
      // Reduction involves subtracting the coeff of x^n from all terms
      auto coeff_n = element[n];
      for (usint i = 0; i < n; i++) {
        output[i] = element[i].ModSub(coeff_n, modulus, mu);
      }
    } else if ((n + 1) * 2 == cycloOrder) {
      IntType mu = modulus.ComputeMu();  // Precompute the Barrett mu parameter
      // cycloOrder is 2*prime: 2 Step reduction
      // First reduce mod x^(n+1)+1 (=(x+1)*Phi_{2*(n+1)}(x))
      // Subtract co-efficient of x^(i+n+1) from x^(i)
      for (usint i = 0; i < n; i++) {
        auto coeff_i = element[i];
        auto coeff_ip = element[i + n + 1];
        output[i] = coeff_i.ModSub(coeff_ip, modulus, mu);
      }
      auto coeff_n = element[n].ModSub(element[2 * n + 1], modulus, mu);
      // Now reduce mod Phi_{2*(n+1)}(x)
      // Similar to the prime case but with alternating signs
      for (usint i = 0; i < n; i++) {
        if (i % 2 == 0) {
          output[i].ModSubEq(coeff_n, modulus, mu);
        } else {
          output[i].ModAddEq(coeff_n, modulus, mu);
        }
      }
    } else {
      // precompute root of unity tables for division NTT
      if ((m_rootOfUnityDivisionTableByModulus[bigMod].GetLength() == 0) ||
          (m_DivisionNTTModulus[modulus] != bigMod)) {
        SetPreComputedNTTDivisionModulus(cycloOrder, modulus, bigMod, bigRoot);
      }

      // cycloOrder is arbitrary
      // auto output = PolyMod(element, this->m_cyclotomicPolyMap[modulus],
      // modulus);

      const auto &nttMod = m_DivisionNTTModulus[modulus];
      const auto &rootTable = m_rootOfUnityDivisionTableByModulus[nttMod];
      VecType aPadded2(m_nttDivisionDim[cycloOrder], nttMod);
      // perform mod operation
      usint power = cycloOrder - n;
      for (usint i = n; i < element.GetLength(); i++) {
        aPadded2[power - (i - n) - 1] = element[i];
      }
      VecType A(m_nttDivisionDim[cycloOrder]);
      NumberTheoreticTransform<VecType>::ForwardTransformIterative(
          aPadded2, rootTable, &A);
      auto AB = A * m_cyclotomicPolyReverseNTTMap[modulus];
      const auto &rootTableInverse =
          m_rootOfUnityDivisionInverseTableByModulus[nttMod];
      VecType a(m_nttDivisionDim[cycloOrder]);
      NumberTheoreticTransform<VecType>::InverseTransformIterative(
          AB, rootTableInverse, &a);

      VecType quotient(m_nttDivisionDim[cycloOrder], modulus);
      for (usint i = 0; i < power; i++) {
        quotient[i] = a[i];
      }
      quotient.ModEq(modulus);
      quotient.SetModulus(nttMod);

      VecType newQuotient(m_nttDivisionDim[cycloOrder]);
      NumberTheoreticTransform<VecType>::ForwardTransformIterative(
          quotient, rootTable, &newQuotient);
      newQuotient *= m_cyclotomicPolyNTTMap[modulus];

      VecType newQuotient2(m_nttDivisionDim[cycloOrder]);
      NumberTheoreticTransform<VecType>::InverseTransformIterative(
          newQuotient, rootTableInverse, &newQuotient2);
      newQuotient2.SetModulus(modulus);
      newQuotient2.ModEq(modulus);

      IntType mu = modulus.ComputeMu();  // Precompute the Barrett mu parameter

      for (usint i = 0; i < n; i++) {
        output[i] =
            element[i].ModSub(newQuotient2[cycloOrder - 1 - i], modulus, mu);
      }
    }
  }
  return output;
}

template <typename VecType>
void ChineseRemainderTransformArb<VecType>::Reset() {
  m_cyclotomicPolyMap.clear();
  m_cyclotomicPolyReverseNTTMap.clear();
  m_cyclotomicPolyNTTMap.clear();
  m_rootOfUnityDivisionTableByModulus.clear();
  m_rootOfUnityDivisionInverseTableByModulus.clear();
  m_DivisionNTTModulus.clear();
  m_DivisionNTTRootOfUnity.clear();
  m_nttDivisionDim.clear();
  BluesteinFFT<VecType>::Reset();
}

}  // namespace lbcrypto
