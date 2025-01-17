/*
 * @file packedencoding.cpp Represents and defines plaintext encodings in
 * Palisade with bit packing capabilities.
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

#include "encoding/packedencoding.h"

namespace lbcrypto {

std::map<ModulusM, NativeInteger> PackedEncoding::m_initRoot;
std::map<ModulusM, NativeInteger> PackedEncoding::m_bigModulus;
std::map<ModulusM, NativeInteger> PackedEncoding::m_bigRoot;

std::map<usint, usint> PackedEncoding::m_automorphismGenerator;
std::map<usint, std::vector<usint>> PackedEncoding::m_toCRTPerm;
std::map<usint, std::vector<usint>> PackedEncoding::m_fromCRTPerm;

bool PackedEncoding::Encode() {
  if (this->isEncoded) return true;
  auto mod = this->encodingParams->GetPlaintextModulus();

  if ((this->typeFlag == IsNativePoly) || (this->typeFlag == IsDCRTPoly)) {
    NativeInteger q;

    NativeVector temp;
    if (this->typeFlag == IsNativePoly) {
      q = this->GetElementModulus().ConvertToInt();
      temp = NativeVector(this->GetElementRingDimension(),
                          this->GetElementModulus().ConvertToInt());
    } else {
      q = this->encodedVectorDCRT.GetParams()
              ->GetParams()[0]
              ->GetModulus()
              .ConvertToInt();
      temp = NativeVector(this->GetElementRingDimension(), q);
      if (q < mod)
        PALISADE_THROW(config_error,
                       "the plaintext modulus size is larger than the size of "
                       "CRT moduli; either decrease the plaintext modulus or "
                       "increase the CRT moduli.");
    }

    size_t i;

    for (i = 0; i < value.size(); i++) {
      NativeInteger entry;

      if ((PlaintextModulus)llabs(value[i]) >= mod)
        PALISADE_THROW(math_error, "Cannot encode integer " +
                                       std::to_string(value[i]) +
                                       " at position " + std::to_string(i) +
                                       " that is > plaintext modulus " +
                                       std::to_string(mod));

      if (value[i] < 0) {
        // It is more efficient to encode negative numbers using the ciphertext
        // modulus no noise growth occurs
        entry = NativeInteger(mod) - NativeInteger((uint64_t)llabs(value[i]));
      } else
        entry = NativeInteger(value[i]);

      temp[i] = entry;
    }

    for (; i < this->GetElementRingDimension(); i++) temp[i] = NativeInteger(0);
    this->isEncoded = true;

    if (this->typeFlag == IsNativePoly) {
      this->GetElement<NativePoly>().SetValues(
          temp, Format::EVALUATION);  // the input plaintext data is in the
                                      // evaluation format
      this->Pack(&this->GetElement<NativePoly>(),
                 this->encodingParams
                     ->GetPlaintextModulus());  // ilVector coefficients are
                                                // packed and resulting ilVector
                                                // is in COEFFICIENT form.
    } else {
      NativePoly firstElement =
          this->GetElement<DCRTPoly>().GetElementAtIndex(0);
      firstElement.SetValues(
          temp,
          Format::EVALUATION);  // the input plaintext data
                                // is in the evaluation format

      this->Pack(&firstElement,
                 this->encodingParams
                     ->GetPlaintextModulus());  // ilVector coefficients are
                                                // packed and resulting ilVector
                                                // is in COEFFICIENT form.
      this->encodedVectorDCRT.SetElementAtIndex(0, firstElement);

      const shared_ptr<ILDCRTParams<BigInteger>> params =
          this->encodedVectorDCRT.GetParams();
      const std::vector<std::shared_ptr<ILNativeParams>> &nativeParams =
          params->GetParams();

      for (size_t i = 1; i < nativeParams.size(); i++) {
        NativePoly temp(firstElement);

        temp.SwitchModulus(nativeParams[i]->GetModulus(),
                           nativeParams[i]->GetRootOfUnity(),
                           nativeParams[i]->GetBigModulus(),
                           nativeParams[i]->GetBigRootOfUnity());

        this->encodedVectorDCRT.SetElementAtIndex(i, temp);
      }
    }

  } else {
    BigVector temp(this->GetElementRingDimension(),
                   BigInteger(this->GetElementModulus()));

    BigInteger q = this->GetElementModulus();

    size_t i;
    for (i = 0; i < value.size(); i++) {
      BigInteger entry;

      if ((PlaintextModulus)llabs(value[i]) >= mod)
        PALISADE_THROW(math_error, "Cannot encode integer " +
                                       std::to_string(value[i]) +
                                       " at position " + std::to_string(i) +
                                       " that is > plaintext modulus " +
                                       std::to_string(mod));

      if (value[i] < 0) {
        // It is more efficient to encode negative numbers using the ciphertext
        // modulus no noise growth occurs
        entry = BigInteger(mod) - BigInteger((uint64_t)llabs(value[i]));
      } else
        entry = BigInteger(value[i]);

      temp[i] = entry;
    }

    for (; i < this->GetElementRingDimension(); i++) temp[i] = BigInteger(0);
    this->isEncoded = true;

    this->GetElement<Poly>().SetValues(
        temp, Format::EVALUATION);  // the input plaintext data is in the
                                    // evaluation format

    this->Pack(&this->GetElement<Poly>(),
               this->encodingParams
                   ->GetPlaintextModulus());  // ilVector coefficients are
                                              // packed and resulting ilVector
                                              // is in COEFFICIENT form.
  }

  return true;
}

template <typename T>
static void fillVec(const T &poly, const PlaintextModulus &mod,
                    vector<int64_t> &vec) {
  vec.clear();

  int64_t half = int64_t(mod) / 2;
  // const typename T::Integer &q = poly.GetModulus();
  // typename T::Integer qHalf = q>>1;

  for (size_t i = 0; i < poly.GetLength(); i++) {
    int64_t val = poly[i].ConvertToInt();
    ;
    /*if (poly[i] > qHalf)
            val = (-(q-poly[i]).ConvertToInt());
    else
            val = poly[i].ConvertToInt();*/
    if (val > half) val -= mod;
    vec.push_back(val);
  }
}

bool PackedEncoding::Decode() {
  auto ptm = this->encodingParams->GetPlaintextModulus();

  if ((this->typeFlag == IsNativePoly) || (this->typeFlag == IsDCRTPoly)) {
    if (this->typeFlag == IsNativePoly) {
      this->Unpack(&this->GetElement<NativePoly>(), ptm);
      fillVec(this->encodedNativeVector, ptm, this->value);
    } else {
      NativePoly firstElement =
          this->GetElement<DCRTPoly>().GetElementAtIndex(0);
      this->Unpack(&firstElement, ptm);
      fillVec(firstElement, ptm, this->value);
    }
  } else {
    this->Unpack(&this->GetElement<Poly>(), ptm);
    fillVec(this->encodedVector, ptm, this->value);
  }

  return true;
}

void PackedEncoding::Destroy() {
  m_initRoot.clear();
  m_bigModulus.clear();
  m_bigRoot.clear();

  m_automorphismGenerator.clear();
  m_toCRTPerm.clear();
  m_fromCRTPerm.clear();
}

void PackedEncoding::SetParams(usint m, EncodingParams params) {
  NativeInteger modulusNI(params->GetPlaintextModulus());  // native int modulus
  std::string exception_message;
  bool hadEx = false;

  // initialize the CRT coefficients if not initialized
  try {
    if (!(m & (m - 1))) {  // Check if m is a power of 2
#pragma omp critical
      { SetParams_2n(m, params); }
    } else {
#pragma omp critical
      {
        const ModulusM modulusM = {modulusNI, m};
        // Arbitrary: Bluestein based CRT Arb. So we need the 2mth root of unity
        if (params->GetPlaintextRootOfUnity() == 0) {
          NativeInteger initRoot = RootOfUnity<NativeInteger>(2 * m, modulusNI);
          m_initRoot[modulusM] = initRoot;
          params->SetPlaintextRootOfUnity(m_initRoot[modulusM].ConvertToInt());
        } else
          m_initRoot[modulusM] = params->GetPlaintextRootOfUnity();

        // Find a compatible big-modulus and root of unity for CRTArb
        if (params->GetPlaintextBigModulus() == 0) {
          usint nttDim = pow(2, ceil(log2(2 * m - 1)));
          if ((modulusNI.ConvertToInt() - 1) % nttDim == 0) {
            m_bigModulus[modulusM] = modulusNI;
          } else {
            usint bigModulusSize =
                ceil(log2(2 * m - 1)) + 2 * modulusNI.GetMSB() + 1;
            m_bigModulus[modulusM] =
                FirstPrime<NativeInteger>(bigModulusSize, nttDim);
          }
          m_bigRoot[modulusM] =
              RootOfUnity<NativeInteger>(nttDim, m_bigModulus[modulusM]);
          params->SetPlaintextBigModulus(m_bigModulus[modulusM]);
          params->SetPlaintextBigRootOfUnity(m_bigRoot[modulusM]);
        } else {
          m_bigModulus[modulusM] = params->GetPlaintextBigModulus();
          m_bigRoot[modulusM] = params->GetPlaintextBigRootOfUnity();
        }

        // Find a generator for the automorphism group
        if (params->GetPlaintextGenerator() == 0) {
          NativeInteger M(m);  // Hackish typecast
          NativeInteger automorphismGenerator =
              FindGeneratorCyclic<NativeInteger>(M);
          m_automorphismGenerator[m] = automorphismGenerator.ConvertToInt();
          params->SetPlaintextGenerator(m_automorphismGenerator[m]);
        } else
          m_automorphismGenerator[m] = params->GetPlaintextGenerator();

        // Create the permutations that interchange the automorphism and crt
        // ordering
        usint phim = GetTotient(m);
        auto tList = GetTotientList(m);
        auto tIdx = std::vector<usint>(m, -1);
        for (usint i = 0; i < phim; i++) {
          tIdx[tList[i]] = i;
        }

        m_toCRTPerm[m] = std::vector<usint>(phim);
        m_fromCRTPerm[m] = std::vector<usint>(phim);

        usint curr_index = 1;
        for (usint i = 0; i < phim; i++) {
          m_toCRTPerm[m][tIdx[curr_index]] = i;
          m_fromCRTPerm[m][i] = tIdx[curr_index];

          curr_index = curr_index * m_automorphismGenerator[m] % m;
        }
      }
    }
  } catch (std::exception &e) {
    exception_message = e.what();
    hadEx = true;
  }

  if (hadEx) PALISADE_THROW(palisade_error, exception_message);
}

template <typename P>
void PackedEncoding::Pack(P *ring, const PlaintextModulus &modulus) const {
  DEBUG_FLAG(false);

  usint m = ring->GetCyclotomicOrder();  // cyclotomic order
  NativeInteger modulusNI(modulus);      // native int modulus

  const ModulusM modulusM = {modulusNI, m};

  // Do the precomputation if not initialized
  if (this->m_initRoot[modulusM].GetMSB() == 0) {
    SetParams(m, EncodingParams(new EncodingParamsImpl(modulus)));
  }

  usint phim = ring->GetRingDimension();

  DEBUG("Pack for order " << m << " phim " << phim << " modulus " << modulusNI);

  // copy values from ring to the vector
  NativeVector slotValues(phim, modulusNI);
  for (usint i = 0; i < phim; i++) {
    slotValues[i] = (*ring)[i].ConvertToInt();
  }

  DEBUG(*ring);
  DEBUG(slotValues);

  // Transform Eval to Coeff
  if (!(m & (m - 1))) {  // Check if m is a power of 2

    if (m_toCRTPerm[m].size() > 0) {
      // Permute to CRT Order
      NativeVector permutedSlots(phim, modulusNI);

      for (usint i = 0; i < phim; i++) {
        permutedSlots[i] = slotValues[m_toCRTPerm[m][i]];
      }
      ChineseRemainderTransformFTT<
          NativeVector>::InverseTransformFromBitReverse(permutedSlots,
                                                        m_initRoot[modulusM], m,
                                                        &slotValues);
    } else {
      ChineseRemainderTransformFTT<
          NativeVector>::InverseTransformFromBitReverse(slotValues,
                                                        m_initRoot[modulusM], m,
                                                        &slotValues);
    }

  } else {  // Arbitrary cyclotomic

    // Permute to CRT Order
    NativeVector permutedSlots(phim, modulusNI);
    for (usint i = 0; i < phim; i++) {
      permutedSlots[i] = slotValues[m_toCRTPerm[m][i]];
    }

    DEBUG("permutedSlots " << permutedSlots);
    DEBUG("m_initRoot[modulusM] " << m_initRoot[modulusM]);
    DEBUG("m_bigModulus[modulusM] " << m_bigModulus[modulusM]);
    DEBUG("m_bigRoot[modulusM] " << m_bigRoot[modulusM]);

    slotValues = ChineseRemainderTransformArb<NativeVector>::InverseTransform(
        permutedSlots, m_initRoot[modulusM], m_bigModulus[modulusM],
        m_bigRoot[modulusM], m);
  }

  DEBUG("slotvalues now " << slotValues);
  // copy values into the slotValuesRing
  typename P::Vector slotValuesRing(phim, ring->GetModulus());
  for (usint i = 0; i < phim; i++) {
    slotValuesRing[i] = typename P::Integer(slotValues[i].ConvertToInt());
  }

  ring->SetValues(slotValuesRing, Format::COEFFICIENT);
  DEBUG(*ring);
}

template <typename P>
void PackedEncoding::Unpack(P *ring, const PlaintextModulus &modulus) const {
  DEBUG_FLAG(false);

  usint m = ring->GetCyclotomicOrder();  // cyclotomic order
  NativeInteger modulusNI(modulus);      // native int modulus

  const ModulusM modulusM = {modulusNI, m};

  // Do the precomputation if not initialized
  if (this->m_initRoot[modulusM].GetMSB() == 0) {
    SetParams(m, EncodingParams(new EncodingParamsImpl(modulus)));
  }

  usint phim = ring->GetRingDimension();  // ring dimension

  DEBUG("Unpack for order " << m << " phim " << phim << " modulus "
                            << modulusNI);

  // copy aggregate plaintext values
  NativeVector packedVector(phim, modulusNI);
  for (usint i = 0; i < phim; i++) {
    packedVector[i] = NativeInteger((*ring)[i].ConvertToInt());
  }

  DEBUG(packedVector);

  // Transform Coeff to Eval
  NativeVector permutedSlots(phim, modulusNI);
  if (!(m & (m - 1))) {  // Check if m is a power of 2
    ChineseRemainderTransformFTT<NativeVector>::ForwardTransformToBitReverse(
        packedVector, m_initRoot[modulusM], m, &permutedSlots);
  } else {  // Arbitrary cyclotomic
    permutedSlots =
        ChineseRemainderTransformArb<NativeVector>::ForwardTransform(
            packedVector, m_initRoot[modulusM], m_bigModulus[modulusM],
            m_bigRoot[modulusM], m);
  }

  if (m_fromCRTPerm[m].size() > 0) {
    // Permute to automorphism Order
    for (usint i = 0; i < phim; i++) {
      packedVector[i] = permutedSlots[m_fromCRTPerm[m][i]];
    }
  } else {
    packedVector = permutedSlots;
  }

  DEBUG(packedVector);

  // copy values into the slotValuesRing
  typename P::Vector packedVectorRing(phim, ring->GetModulus());
  for (usint i = 0; i < phim; i++) {
    packedVectorRing[i] = typename P::Integer(packedVector[i].ConvertToInt());
  }

  ring->SetValues(packedVectorRing, Format::COEFFICIENT);
}

void PackedEncoding::SetParams_2n(usint m, const NativeInteger &modulusNI) {
  const ModulusM modulusM = {modulusNI, m};

  // Power of two: m/2-point FTT. So we need the mth root of unity
  m_initRoot[modulusM] = RootOfUnity<NativeInteger>(m, modulusNI);

  // Create the permutations that interchange the automorphism and crt ordering
  // First we create the cyclic group generated by 5 and then adjoin the
  // co-factor by multiplying by (-1)

  usint phim = (m >> 1);
  usint phim_by_2 = (m >> 2);

  m_toCRTPerm[m] = std::vector<usint>(phim);
  m_fromCRTPerm[m] = std::vector<usint>(phim);

  usint curr_index = 1;
  usint logn = std::round(log2(m / 2));
  for (usint i = 0; i < phim_by_2; i++) {
    m_toCRTPerm[m][ReverseBits((curr_index - 1) / 2, logn)] = i;
    m_fromCRTPerm[m][i] = ReverseBits((curr_index - 1) / 2, logn);

    usint cofactor_index = curr_index * (m - 1) % m;

    m_toCRTPerm[m][ReverseBits((cofactor_index - 1) / 2, logn)] = i + phim_by_2;
    m_fromCRTPerm[m][i + phim_by_2] =
        ReverseBits((cofactor_index - 1) / 2, logn);

    curr_index = curr_index * 5 % m;
  }
}

void PackedEncoding::SetParams_2n(usint m, EncodingParams params) {
  NativeInteger modulusNI(params->GetPlaintextModulus());  // native int modulus
  const ModulusM modulusM = {modulusNI, m};

  // Power of two: m/2-point FTT. So we need the mth root of unity
  if (params->GetPlaintextRootOfUnity() == 0) {
    m_initRoot[modulusM] = RootOfUnity<NativeInteger>(m, modulusNI);
    params->SetPlaintextRootOfUnity(m_initRoot[modulusM]);
  } else {
    m_initRoot[modulusM] = params->GetPlaintextRootOfUnity();
  }

  // Create the permutations that interchange the automorphism and crt ordering
  // First we create the cyclic group generated by 5 and then adjoin the
  // co-factor by multiplying by (-1)
  usint phim = (m >> 1);
  usint phim_by_2 = (m >> 2);

  m_toCRTPerm[m] = std::vector<usint>(phim);
  m_fromCRTPerm[m] = std::vector<usint>(phim);

  usint curr_index = 1;
  usint logn = std::round(log2(m >> 1));
  for (usint i = 0; i < phim_by_2; i++) {
    m_toCRTPerm[m][ReverseBits((curr_index - 1) / 2, logn)] = i;
    m_fromCRTPerm[m][i] = ReverseBits((curr_index - 1) / 2, logn);

    usint cofactor_index = curr_index * (m - 1) % m;

    m_toCRTPerm[m][ReverseBits((cofactor_index - 1) / 2, logn)] = i + phim_by_2;
    m_fromCRTPerm[m][i + phim_by_2] =
        ReverseBits((cofactor_index - 1) / 2, logn);

    curr_index = curr_index * 5 % m;
  }
}
}  // namespace lbcrypto
