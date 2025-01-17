/**
 * @file boolean-serial-binary.cpp - Example for FHEW with binary serialization
 * @author  TPOC: contact@palisade-crypto.org
 *
 * @copyright Copyright (c) 2019, Duality Technologies Inc.
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

#include "binfhecontext.h"

// these header files are needed for serialization
#include "binfhecontext-ser.h"
#include "utils/serialize-binary.h"

using namespace lbcrypto;
using namespace std;

// path where files will be written to
const std::string DATAFOLDER = "demoData";

int main() {
  // Generating the crypto context

  auto cc1 = BinFHEContext();

  cc1.GenerateBinFHEContext(TOY);

  std::cout << "Generating keys." << std::endl;

  // Generating the secret key
  auto sk1 = cc1.KeyGen();

  // Generate the bootstrapping keys
  cc1.BTKeyGen(sk1);

  std::cout << "Done generating all keys." << std::endl;

  // Encryption for a ciphertext that will be serialized
  auto ct1 = cc1.Encrypt(sk1, 1);

  // CODE FOR SERIALIZATION

  // Serializing key-independent crypto context

  if (!Serial::SerializeToFile(DATAFOLDER + "/cryptoContext.txt", cc1,
                               SerType::BINARY)) {
    cerr << "Error serializing the cryptocontext" << endl;
    return 1;
  } else
    cout << "The cryptocontext has been serialized." << std::endl;

  // Serializing refreshing and key switching keys (needed for bootstrapping)

  if (!Serial::SerializeToFile(DATAFOLDER + "/refreshKey.txt",
                               cc1.GetRefreshKey(), SerType::BINARY)) {
    cerr << "Error serializing the refreshing key" << endl;
    return 1;
  } else
    cout << "The refreshing key has been serialized." << std::endl;

  if (!Serial::SerializeToFile(DATAFOLDER + "/ksKey.txt", cc1.GetSwitchKey(),
                               SerType::BINARY)) {
    cerr << "Error serializing the switching key" << endl;
    return 1;
  } else
    cout << "The key switching key has been serialized." << std::endl;

  // Serializing private keys

  if (!Serial::SerializeToFile(DATAFOLDER + "/sk1.txt", sk1, SerType::BINARY)) {
    cerr << "Error serializing sk1" << endl;
    return 1;
  } else
    cout << "The secret key sk1 key been serialized." << std::endl;

  // Serializing a ciphertext

  if (!Serial::SerializeToFile(DATAFOLDER + "/ct1.txt", ct1, SerType::BINARY)) {
    cerr << "Error serializing ct1" << endl;
    return 1;
  } else
    cout << "A ciphertext has been serialized." << std::endl;

  // CODE FOR DESERIALIZATION

  // Deserializing the cryptocontext

  BinFHEContext cc;
  if (Serial::DeserializeFromFile(DATAFOLDER + "/cryptoContext.txt", cc,
                                  SerType::BINARY) == false) {
    cerr << "Could not deserialize the cryptocontext" << endl;
    return 1;
  } else
    cout << "The cryptocontext has been deserialized." << std::endl;

  // deserializing the refreshing and switching keys (for bootstrapping)

  std::shared_ptr<RingGSWBTKey> refreshKey;
  if (Serial::DeserializeFromFile(DATAFOLDER + "/refreshKey.txt", refreshKey,
                                  SerType::BINARY) == false) {
    cerr << "Could not deserialize the refresh key" << endl;
    return 1;
  } else
    cout << "The refresh key has been deserialized." << std::endl;

  std::shared_ptr<LWESwitchingKey> ksKey;
  if (Serial::DeserializeFromFile(DATAFOLDER + "/ksKey.txt", ksKey,
                                  SerType::BINARY) == false) {
    cerr << "Could not deserialize the switching key" << endl;
    return 1;
  } else
    cout << "The switching key has been deserialized." << std::endl;

  // Loading the keys in the cryptocontext
  cc.BTKeyLoad({refreshKey, ksKey});

  // Deserializing the secret key

  LWEPrivateKey sk;
  if (Serial::DeserializeFromFile(DATAFOLDER + "/sk1.txt", sk,
                                  SerType::BINARY) == false) {
    cerr << "Could not deserialize the secret key" << endl;
    return 1;
  } else
    cout << "The secret key has been deserialized." << std::endl;

  // Deserializing a previously serialized ciphertext

  LWECiphertext ct;
  if (Serial::DeserializeFromFile(DATAFOLDER + "/ct1.txt", ct,
                                  SerType::BINARY) == false) {
    cerr << "Could not deserialize the ciphertext" << endl;
    return 1;
  } else
    cout << "The ciphertext has been deserialized." << std::endl;

  // OPERATIONS WITH DESERIALIZED KEYS AND CIPHERTEXTS

  auto ct2 = cc.Encrypt(sk, 1);

  std::cout << "Running the computation" << std::endl;

  auto ctResult = cc.EvalBinGate(AND, ct, ct2);

  std::cout << "The computation has completed" << std::endl;

  LWEPlaintext result;

  cc.Decrypt(sk, ctResult, &result);

  std::cout << "result of 1 AND 1 = " << result << std::endl;

  return 0;
}
