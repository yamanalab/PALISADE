/*
 * @file cryptocontext-ser.h - serialize cryptocontext; include this in any app
 * that needs to serialize them
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

#ifndef LBCRYPTO_CRYPTO_CRYPTOCONTEXTSER_H
#define LBCRYPTO_CRYPTO_CRYPTOCONTEXTSER_H

#include "palisade.h"
#include "cryptocontext.h"
#include "utils/serial.h"

extern template class lbcrypto::CryptoContextImpl<lbcrypto::Poly>;
extern template class lbcrypto::CryptoContextImpl<lbcrypto::NativePoly>;
extern template class lbcrypto::CryptoContextImpl<lbcrypto::DCRTPoly>;

CEREAL_CLASS_VERSION(
    lbcrypto::CryptoContextImpl<lbcrypto::Poly>,
    lbcrypto::CryptoContextImpl<lbcrypto::Poly>::SerializedVersion());
CEREAL_CLASS_VERSION(
    lbcrypto::CryptoContextImpl<lbcrypto::NativePoly>,
    lbcrypto::CryptoContextImpl<lbcrypto::NativePoly>::SerializedVersion());
CEREAL_CLASS_VERSION(
    lbcrypto::CryptoContextImpl<lbcrypto::DCRTPoly>,
    lbcrypto::CryptoContextImpl<lbcrypto::DCRTPoly>::SerializedVersion());

// the routines below are only instantiated if the user includes the appropriate
// serialize-*.h file

namespace lbcrypto {
#ifdef LBCRYPTO_SERIALIZE_JSON_H
/**
 * Deserialize for a CryptoContext (that is, a shared pointer to a
 * CryptoContextImpl PALISADE doesn't want multiple copies of the same crypto
 * context floating around, and it enforces that here
 *
 * @param obj - the target for the deserialization
 * @param stream - where the serialization is coming from
 * @param sertype - JSON serialization type
 */
template <typename T>
void Serial::Deserialize(std::shared_ptr<CryptoContextImpl<T>>& obj,
                         std::istream& stream, const SerType::SERJSON&) {
  CryptoContext<T> newob;

  try {
    cereal::JSONInputArchive archive(stream);
    archive(newob);
  } catch (std::exception& e) {
    //		std::cout << e.what() << std::endl;
    return;
  }

  obj = CryptoContextFactory<T>::GetContext(newob->GetCryptoParameters(),
                                            newob->GetEncryptionAlgorithm(),
                                            newob->getSchemeId());
}

template void Serial::Deserialize(std::shared_ptr<CryptoContextImpl<Poly>>& obj,
                                  std::istream& stream,
                                  const SerType::SERJSON&);
template bool CryptoContextImpl<Poly>::SerializeEvalMultKey<SerType::SERJSON>(
    std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<Poly>::SerializeEvalMultKey<SerType::SERJSON>(
    std::ostream& ser, const SerType::SERJSON&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalMultKey<SerType::SERJSON>(
    std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<Poly>::SerializeEvalSumKey<SerType::SERJSON>(
    std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<Poly>::SerializeEvalSumKey<SerType::SERJSON>(
    std::ostream& ser, const SerType::SERJSON&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalSumKey<SerType::SERJSON>(
    std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<Poly>::SerializeEvalAutomorphismKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool
CryptoContextImpl<Poly>::SerializeEvalAutomorphismKey<SerType::SERJSON>(
    std::ostream& ser, const SerType::SERJSON&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalAutomorphismKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);

template void Serial::Deserialize(
    std::shared_ptr<CryptoContextImpl<NativePoly>>& obj, std::istream& stream,
    const SerType::SERJSON&);
template bool CryptoContextImpl<NativePoly>::SerializeEvalMultKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalMultKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalMultKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<NativePoly>::SerializeEvalSumKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalSumKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalSumKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<NativePoly>::SerializeEvalAutomorphismKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalAutomorphismKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalAutomorphismKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);

template void Serial::Deserialize(
    std::shared_ptr<CryptoContextImpl<DCRTPoly>>& obj, std::istream& stream,
    const SerType::SERJSON&);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalMultKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalMultKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalMultKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalSumKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalSumKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalSumKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalAutomorphismKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalAutomorphismKey<
    SerType::SERJSON>(std::ostream& ser, const SerType::SERJSON&,
                      const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalAutomorphismKey<
    SerType::SERJSON>(std::istream& ser, const SerType::SERJSON&);

#endif

#ifdef LBCRYPTO_SERIALIZE_BINARY_H
/**
 * Deserialize for a CryptoContext (that is, a shared pointer to a
 * CryptoContextImpl PALISADE doesn't want multiple copies of the same crypto
 * context floating around, and it enforces that here
 *
 * @param obj - the target for the deserialization
 * @param stream - where the serialization is coming from
 * @param sertype - BINARY serialization type
 */
template <typename T>
void Serial::Deserialize(std::shared_ptr<CryptoContextImpl<T>>& obj,
                         std::istream& stream, const SerType::SERBINARY&) {
  CryptoContext<T> newob;

  try {
    cereal::PortableBinaryInputArchive archive(stream);
    archive(newob);
  } catch (std::exception& e) {
    //		std::cout << e.what() << std::endl;
    return;
  }

  obj = CryptoContextFactory<T>::GetContext(newob->GetCryptoParameters(),
                                            newob->GetEncryptionAlgorithm(),
                                            newob->getSchemeId());
}

template void Serial::Deserialize(std::shared_ptr<CryptoContextImpl<Poly>>& obj,
                                  std::istream& stream,
                                  const SerType::SERBINARY&);
template bool CryptoContextImpl<Poly>::SerializeEvalMultKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<Poly>::SerializeEvalMultKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalMultKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool CryptoContextImpl<Poly>::SerializeEvalSumKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<Poly>::SerializeEvalSumKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalSumKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool
CryptoContextImpl<Poly>::SerializeEvalAutomorphismKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool
CryptoContextImpl<Poly>::SerializeEvalAutomorphismKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, const CryptoContext<Poly> cc);
template bool CryptoContextImpl<Poly>::DeserializeEvalAutomorphismKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);

template void Serial::Deserialize(
    std::shared_ptr<CryptoContextImpl<NativePoly>>& obj, std::istream& stream,
    const SerType::SERBINARY&);
template bool
CryptoContextImpl<NativePoly>::SerializeEvalMultKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalMultKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalMultKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool
CryptoContextImpl<NativePoly>::SerializeEvalSumKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalSumKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalSumKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool
CryptoContextImpl<NativePoly>::SerializeEvalAutomorphismKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<NativePoly>::SerializeEvalAutomorphismKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<NativePoly> cc);
template bool CryptoContextImpl<NativePoly>::DeserializeEvalAutomorphismKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);

template void Serial::Deserialize(
    std::shared_ptr<CryptoContextImpl<DCRTPoly>>& obj, std::istream& stream,
    const SerType::SERBINARY&);
template bool
CryptoContextImpl<DCRTPoly>::SerializeEvalMultKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalMultKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalMultKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool
CryptoContextImpl<DCRTPoly>::SerializeEvalSumKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalSumKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalSumKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);
template bool
CryptoContextImpl<DCRTPoly>::SerializeEvalAutomorphismKey<SerType::SERBINARY>(
    std::ostream& ser, const SerType::SERBINARY&, string id);
template bool CryptoContextImpl<DCRTPoly>::SerializeEvalAutomorphismKey<
    SerType::SERBINARY>(std::ostream& ser, const SerType::SERBINARY&,
                        const CryptoContext<DCRTPoly> cc);
template bool CryptoContextImpl<DCRTPoly>::DeserializeEvalAutomorphismKey<
    SerType::SERBINARY>(std::istream& ser, const SerType::SERBINARY&);

#endif
}  // namespace lbcrypto

#endif
