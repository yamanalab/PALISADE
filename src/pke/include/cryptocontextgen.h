/**
 * @file ciphertextgen.h -- Generator for crypto contexts.
 * @author  TPOC: contact@palisade-crypto.org
 *
 * @section LICENSE
 *
 * @copyright Copyright (c) 2019, New Jersey Institute of Technology (NJIT))
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
 */

#ifndef SRC_PKE_LIB_CRYPTOCONTEXTGEN_H_
#define SRC_PKE_LIB_CRYPTOCONTEXTGEN_H_

#include "palisade.h"
#include "cryptocontext.h"
#include "utils/parmfactory.h"
#include "lattice/elemparamfactory.h"

using namespace lbcrypto;

static const usint DefaultQbits = 59;
static const usint DefaultT = 3;

template <typename Element>
inline CryptoContext<Element> GenCryptoContextNull(usint ORDER,
                                                   PlaintextModulus ptm,
                                                   usint bits = DefaultQbits,
                                                   usint towers = DefaultT) {
  shared_ptr<typename Element::Params> p =
      ElemParamFactory::GenElemParams<typename Element::Params>(ORDER, bits,
                                                                towers);

  CryptoContext<Element> cc =
      CryptoContextFactory<Element>::genCryptoContextNull(ORDER, ptm);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);

  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextStSt(usint ORDER,
                                                   PlaintextModulus ptm,
                                                   usint bits = DefaultQbits,
                                                   usint towers = DefaultT) {
  shared_ptr<typename Element::Params> p =
      ElemParamFactory::GenElemParams<typename Element::Params>(ORDER, bits,
                                                                towers);

  CryptoContext<Element> cc =
      CryptoContextFactory<Element>::genCryptoContextStehleSteinfeld(
          p, ptm, 1, 4, 41411.5);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);

  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextBGV(usint ORDER,
                                                  PlaintextModulus ptm,
                                                  usint bits = DefaultQbits,
                                                  usint towers = DefaultT,
                                                  MODE mode = RLWE) {
  shared_ptr<typename Element::Params> p =
      ElemParamFactory::GenElemParams<typename Element::Params>(ORDER, bits,
                                                                towers);

  CryptoContext<Element> cc =
      CryptoContextFactory<Element>::genCryptoContextBGV(p, ptm, 1, 4, mode);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);

  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextBFV(usint ORDER,
                                                  PlaintextModulus ptm,
                                                  usint bits = DefaultQbits,
                                                  usint towers = DefaultT,
                                                  MODE mode = RLWE);

template <>
inline CryptoContext<Poly> GenCryptoContextBFV(usint ORDER,
                                               PlaintextModulus ptm, usint bits,
                                               usint towers, MODE mode) {
  shared_ptr<typename Poly::Params> p =
      ElemParamFactory::GenElemParams<typename Poly::Params>(ORDER, bits,
                                                             towers);

  CryptoContext<Poly> cc = CryptoContextFactory<Poly>::genCryptoContextBFV(
      ptm, 1.06, 8, 4, 0, 1, 0, mode);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);
  return cc;
}

template <>
inline CryptoContext<NativePoly> GenCryptoContextBFV(usint ORDER,
                                                     PlaintextModulus ptm,
                                                     usint bits, usint towers,
                                                     MODE mode) {
  CryptoContext<NativePoly> cc =
      CryptoContextFactory<NativePoly>::genCryptoContextBFV(ptm, 1.06, 8, 4, 0,
                                                            1, 0, mode);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);
  return cc;
}

template <>
inline CryptoContext<DCRTPoly> GenCryptoContextBFV(usint ORDER,
                                                   PlaintextModulus ptm,
                                                   usint bits, usint towers,
                                                   MODE mode) {
  PALISADE_THROW(not_implemented_error, "DCRT is not supported for BFV");
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextBFVrns(PlaintextModulus ptm,
                                                     MODE mode = RLWE,
                                                     uint32_t batchSize = 0);

template <>
inline CryptoContext<Poly> GenCryptoContextBFVrns(PlaintextModulus ptm,
                                                  MODE mode,
                                                  uint32_t batchSize) {
  PALISADE_THROW(not_implemented_error, "Poly is not supported for BFVrns");
}

template <>
inline CryptoContext<NativePoly> GenCryptoContextBFVrns(PlaintextModulus ptm,
                                                        MODE mode,
                                                        uint32_t batchSize) {
  PALISADE_THROW(not_implemented_error,
                 "NativePoly is not supported for BFVrns");
}

template <>
inline CryptoContext<DCRTPoly> GenCryptoContextBFVrns(PlaintextModulus ptm,
                                                      MODE mode,
                                                      uint32_t batchSize) {
  EncodingParams encodingParams(new EncodingParamsImpl(ptm));
  encodingParams->SetBatchSize(batchSize);
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBFVrns(
          encodingParams, HEStd_128_classic, 3.2, 0, 2, 0, mode, 2, 20, 60);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);
  cc->Enable(MULTIPARTY);
  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextBFVrnsB(PlaintextModulus ptm,
                                                      MODE mode = RLWE,
                                                      uint32_t batchSize = 0);

template <>
inline CryptoContext<Poly> GenCryptoContextBFVrnsB(PlaintextModulus ptm,
                                                   MODE mode,
                                                   uint32_t batchSize) {
  PALISADE_THROW(not_implemented_error, "Poly is not supported for BFVrnsB");
}

template <>
inline CryptoContext<NativePoly> GenCryptoContextBFVrnsB(PlaintextModulus ptm,
                                                         MODE mode,
                                                         uint32_t batchSize) {
  PALISADE_THROW(not_implemented_error,
                 "NativePoly is not supported for BFVrnsB");
}

template <>
inline CryptoContext<DCRTPoly> GenCryptoContextBFVrnsB(PlaintextModulus ptm,
                                                       MODE mode,
                                                       uint32_t batchSize) {
  EncodingParams encodingParams(new EncodingParamsImpl(ptm));
  encodingParams->SetBatchSize(batchSize);
  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextBFVrnsB(
          encodingParams, HEStd_128_classic, 3.2, 0, 2, 0, mode, 2, 20, 60);
  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);
  cc->Enable(MULTIPARTY);
  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenCryptoContextCKKS(
    usint cyclOrder, usint numPrimes, usint scaleExp, usint relinWindow,
    usint batchSize, MODE mode, KeySwitchTechnique ksTech,
    RescalingTechnique rsTech);

template <>
inline CryptoContext<Poly> GenCryptoContextCKKS(usint cyclOrder,
                                                usint numPrimes, usint scaleExp,
                                                usint relinWindow,
                                                usint batchSize, MODE mode,
                                                KeySwitchTechnique ksTech,
                                                RescalingTechnique rsTech) {
  PALISADE_THROW(not_implemented_error, "Poly is not supported for CKKS");
}

/* *
 * Generate a CryptoContext for the CKKS scheme.
 *
 * @param m Cyclotomic order. Must be a power of 2.
 * @param init_size Number of co-primes comprising the ciphertext modulus.
 * 			  It is equal to the desired depth of the computation.
 * @param dcrtBits Size of each co-prime in bits. Should fit into a
 * 			 machine word, i.e., less than 64.
 * @param p Scaling parameter 2^p. p should usually be equal to dcrtBits.
 * @param relinWin The bit decomposition count used in relinearization.
 * 			 Use 0 to go with max possible. Use small values (3-4?)
 * 			 if you need rotations before any multiplications.
 * @param stdDev The standard deviation of the Normal distribution noise
 * 		   used in CKKS.
 * @param batchSize The length of the packed vectors to be used with CKKS.
 */
template <>
inline CryptoContext<NativePoly> GenCryptoContextCKKS(
    usint cyclOrder, usint numPrimes, usint scaleExp, usint relinWindow,
    usint batchSize, MODE mode, KeySwitchTechnique ksTech,
    RescalingTechnique rsTech) {
  usint m = cyclOrder;
  usint init_size = numPrimes;
  usint dcrtBits = scaleExp;
  uint64_t p = scaleExp;
  usint relinWin = relinWindow;
  float stdDev = 3.19;
  usint batch = batchSize;

  NativeInteger q = FirstPrime<NativeInteger>(dcrtBits, m);
  NativeInteger rootOfUnity = RootOfUnity<NativeInteger>(m, q);

  shared_ptr<ILNativeParams> params(new ILNativeParams(m, q, rootOfUnity));

  EncodingParams encodingParams(new EncodingParamsImpl((int64_t)1 << p));
  encodingParams->SetBatchSize(batch);

  CryptoContext<NativePoly> cc =
      CryptoContextFactory<NativePoly>::genCryptoContextCKKS(
          params, encodingParams, relinWin, stdDev, mode, init_size);

  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);

  return cc;
}

/* *
 * Generate a CryptoContext for the CKKS scheme.
 *
 * @param m Cyclotomic order. Must be a power of 2.
 * @param init_size Number of co-primes comprising the ciphertext modulus.
 * 			  It is equal to the desired depth of the computation.
 * @param dcrtBits Size of each co-prime in bits. Should fit into a
 * 			 machine word, i.e., less than 64.
 * @param p Scaling parameter 2^p. p should usually be equal to dcrtBits.
 * @param relinWin The bit decomposition count used in relinearization.
 * 			 Use 0 to go with max possible. Use small values (3-4?)
 * 			 if you need rotations before any multiplications.
 * @param stdDev The standard deviation of the Normal distribution noise
 * 		   used in CKKS.
 * @param batchSize The length of the packed vectors to be used with CKKS.
 */
template <>
inline CryptoContext<DCRTPoly> GenCryptoContextCKKS(
    usint cyclOrder, usint numPrimes, usint scaleExp, usint relinWindow,
    usint batchSize, MODE mode, KeySwitchTechnique ksTech,
    RescalingTechnique rsTech) {
  usint n = cyclOrder / 2;
  usint dcrtBits = scaleExp;
  usint relinWin = relinWindow;
  usint batch = batchSize;

  CryptoContext<DCRTPoly> cc =
      CryptoContextFactory<DCRTPoly>::genCryptoContextCKKS(
          numPrimes - 1, dcrtBits, batch, HEStd_NotSet, n, /*ringDimension*/
          rsTech, ksTech, 3,                               /*numLargeDigits*/
          2,                                               /*maxDepth*/
          60,                                              /*firstMod*/
          relinWin, mode);

  cc->Enable(ENCRYPTION);
  cc->Enable(SHE);
  cc->Enable(LEVELEDSHE);
  cc->Enable(MULTIPARTY);
  return cc;
}

template <typename Element>
inline CryptoContext<Element> GenTestCryptoContext(
    const string& name, usint ORDER, PlaintextModulus ptm,
    usint bits = DefaultQbits, usint towers = DefaultT, usint relinWin = 0,
    usint batchSize = 8, KeySwitchTechnique ksTech = BV,
    RescalingTechnique rsTech = APPROXRESCALE) {
  shared_ptr<typename Element::Params> p =
      ElemParamFactory::GenElemParams<typename Element::Params>(ORDER, bits,
                                                                towers);

  CryptoContext<Element> cc;

  if (name == "Null") {
    cc = CryptoContextFactory<Element>::genCryptoContextNull(ORDER, ptm);
  } else if (name == "StSt")
    cc = CryptoContextFactory<Element>::genCryptoContextStehleSteinfeld(
        p, ptm, 1, 4, 41411.5);
  else if (name == "BGV_rlwe")
    cc = CryptoContextFactory<Element>::genCryptoContextBGV(p, ptm, 1, 4, RLWE);
  else if (name == "BGV_opt")
    cc = CryptoContextFactory<Element>::genCryptoContextBGV(p, ptm, 1, 4,
                                                            OPTIMIZED);
  else if (name == "BFV_rlwe")
    cc = GenCryptoContextBFV<Element>(ORDER, ptm, bits, towers, RLWE);
  else if (name == "BFV_opt")
    cc = GenCryptoContextBFV<Element>(ORDER, ptm, bits, towers, OPTIMIZED);
  else if (name == "BFVrns_rlwe")
    cc = GenCryptoContextBFVrns<Element>(ptm, RLWE, batchSize);
  else if (name == "BFVrns_opt")
    cc = GenCryptoContextBFVrns<Element>(ptm, OPTIMIZED, batchSize);
  else if (name == "BFVrnsB_rlwe")
    cc = GenCryptoContextBFVrnsB<Element>(ptm, RLWE, batchSize);
  else if (name == "BFVrnsB_opt")
    cc = GenCryptoContextBFVrnsB<Element>(ptm, OPTIMIZED, batchSize);
  else if (name == "CKKS_sparse")
    cc = GenCryptoContextCKKS<Element>(ORDER, towers, ptm, relinWin, batchSize,
                                       SPARSE, ksTech, rsTech);
  else if (name == "CKKS") {
    cc = GenCryptoContextCKKS<Element>(ORDER, towers, ptm, relinWin, batchSize,
                                       OPTIMIZED, ksTech, rsTech);
  } else {
    cout << "nothing for " << name << endl;
    PALISADE_THROW(not_available_error, "No generator for " + name);
  }

  cc->Enable(ENCRYPTION);
  cc->Enable(PRE);
  cc->Enable(SHE);

  return cc;
}

#endif /* SRC_PKE_LIB_CRYPTOCONTEXTGEN_H_ */
