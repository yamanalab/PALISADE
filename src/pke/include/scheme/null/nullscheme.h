/**
 * @file nullscheme.h -- Operations for the null cryptoscheme.
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

#ifndef SRC_LIB_CRYPTO_NULLSCHEME_H_
#define SRC_LIB_CRYPTO_NULLSCHEME_H_

#include "palisade.h"

namespace lbcrypto {

template <class Element>
class LPCryptoParametersNull : public LPCryptoParameters<Element> {
 public:
  LPCryptoParametersNull() : LPCryptoParameters<Element>() {}

  LPCryptoParametersNull(const shared_ptr<typename Element::Params> ep,
                         const PlaintextModulus &plaintextModulus)
      : LPCryptoParameters<Element>(ep, plaintextModulus) {}

  LPCryptoParametersNull(shared_ptr<typename Element::Params> ep,
                         EncodingParams encodingParams)
      : LPCryptoParameters<Element>(ep, encodingParams) {}

  LPCryptoParametersNull(const LPCryptoParametersNull &rhs)
      : LPCryptoParameters<Element>(rhs) {}

  virtual ~LPCryptoParametersNull() {}

  void SetPlaintextModulus(const PlaintextModulus &plaintextModulus) {
    PALISADE_THROW(config_error,
                   "plaintext modulus is fixed to be == ciphertext modulus and "
                   "cannot be changed");
  }

  /**
   * == operator to compare to this instance of LPCryptoParametersNull object.
   *
   * @param &rhs LPCryptoParameters to check equality against.
   */
  bool operator==(const LPCryptoParameters<Element> &rhs) const {
    const LPCryptoParametersNull<Element> *el =
        dynamic_cast<const LPCryptoParametersNull<Element> *>(&rhs);

    if (el == 0) return false;

    return this->GetPlaintextModulus() == el->GetPlaintextModulus() &&
           *this->GetElementParams() == *el->GetElementParams() &&
           *this->GetEncodingParams() == *el->GetEncodingParams();
  }

  template <class Archive>
  void save(Archive &ar) const {
    ar(::cereal::base_class<LPCryptoParameters<Element>>(this));
  }

  template <class Archive>
  void load(Archive &ar) {
    ar(::cereal::base_class<LPCryptoParameters<Element>>(this));
  }

  std::string SerializedObjectName() const { return "NullSchemeParameters"; }
};

template <class Element>
class LPAlgorithmNull : public LPEncryptionAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmNull() {}

  virtual ~LPAlgorithmNull() {}

  /**
   * Method for encrypting plaintext using Null
   *
   * @param &publicKey public key used for encryption.
   * @param &plaintext the plaintext input.
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @param *ciphertext ciphertext which results from encryption.
   */
  Ciphertext<Element> Encrypt(const LPPublicKey<Element> pubKey,
                              Element ptxt) const {
    Ciphertext<Element> ciphertext(new CiphertextImpl<Element>(pubKey));

    // no difference between Encryption and non-Encryption mode for the Null
    // scheme
    ciphertext->SetElement(ptxt);

    return ciphertext;
  }

  /**
   * Method for encrypting plaintext using Null
   *
   * @param &publicKey public key used for encryption.
   * @param &plaintext the plaintext input.
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @param *ciphertext ciphertext which results from encryption.
   */
  Ciphertext<Element> Encrypt(const LPPrivateKey<Element> privKey,
                              Element ptxt) const {
    Ciphertext<Element> ciphertext(new CiphertextImpl<Element>(privKey));

    // no difference between Encryption and non-Encryption mode for the Null
    // scheme
    ciphertext->SetElement(ptxt);

    return ciphertext;
  }

  /**
   * Method for decrypting plaintext using Null
   *
   * @param &privateKey private key used for decryption.
   * @param &ciphertext ciphertext id decrypted.
   * @param *plaintext the plaintext output.
   * @return the decrypted plaintext returned.
   */
  DecryptResult Decrypt(const LPPrivateKey<Element> privateKey,
                        ConstCiphertext<Element> ciphertext,
                        NativePoly *plaintext) const {
    const Element &b = ciphertext->GetElement();
    const auto ptm = ciphertext->GetCryptoContext()
                         ->GetCryptoParameters()
                         ->GetPlaintextModulus();
    *plaintext = b.DecryptionCRTInterpolate(ptm);
    return DecryptResult(plaintext->GetLength());
  }

  /**
   * Function to generate public and private keys
   *
   * @param &publicKey private key used for decryption.
   * @param &privateKey private key used for decryption.
   * @return function ran correctly.
   */
  LPKeyPair<Element> KeyGen(CryptoContext<Element> cc,
                            bool makeSparse = false) {
    LPKeyPair<Element> kp(new LPPublicKeyImpl<Element>(cc),
                          new LPPrivateKeyImpl<Element>(cc));

    Element a(cc->GetCryptoParameters()->GetElementParams(),
              Format::COEFFICIENT, true);
    kp.secretKey->SetPrivateElement(a);
    kp.publicKey->SetPublicElementAtIndex(0, a);
    kp.publicKey->SetPublicElementAtIndex(1, a);

    return kp;
  }
};

/**
 * @brief PRE scheme based on Null.
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmPRENull : public LPPREAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmPRENull() {}

  /**
   * Function to generate 1..log(q) encryptions for each bit of the original
   * private key Variant that uses the public key for the new secret key.
   *
   * @param &newPrivateKey encryption key for the new ciphertext.
   * @param &origPrivateKey original private key used for decryption.
   * @param &ddg discrete Gaussian generator.
   * @param *evalKey the evaluation key.
   */
  LPEvalKey<Element> ReKeyGen(
      const LPPublicKey<Element> newPrivateKey,
      const LPPrivateKey<Element> origPrivateKey) const {
    // create a new ReKey of the proper type, in this context
    LPEvalKeyNTRURelin<Element> EK(
        new LPEvalKeyNTRURelinImpl<Element>(newPrivateKey->GetCryptoContext()));

    Element a(newPrivateKey->GetCryptoContext()
                  ->GetCryptoParameters()
                  ->GetElementParams(),
              Format::COEFFICIENT, true);
    vector<Element> evalKeyElements;
    evalKeyElements.push_back(std::move(a));

    EK->SetAVector(std::move(evalKeyElements));

    return EK;
  }

  /**
   * Function to define the interface for re-encypting ciphertext using the
   * array generated by ProxyGen
   *
   * @param &evalKey the evaluation key.
   * @param &ciphertext the input ciphertext.
   * @param publicKey the public key of the recipient of the re-encrypted
   * ciphertext.
   * @param *newCiphertext the new ciphertext.
   */
  Ciphertext<Element> ReEncrypt(
      const LPEvalKey<Element> evalKey, ConstCiphertext<Element> ciphertext,
      const LPPublicKey<Element> publicKey = nullptr) const {
    Ciphertext<Element> newCiphertext(new CiphertextImpl<Element>(*ciphertext));
    return newCiphertext;
  }
};

/**
 * @brief Concrete class for the FHE Multiparty algorithms on the Null scheme.
 * A version of this multiparty scheme built on the BGV scheme is seen here:
 *   - Asharov G., Jain A., López-Alt A., Tromer E., Vaikuntanathan V., Wichs D.
 * (2012) Multiparty Computation with Low Communication, Computation and
 * Interaction via Threshold FHE. In: Pointcheval D., Johansson T. (eds)
 * Advances in Cryptology – EUROCRYPT 2012. EUROCRYPT 2012. Lecture Notes in
 * Computer Science, vol 7237. Springer, Berlin, Heidelberg
 *
 * During offline key generation, this multiparty scheme relies on the clients
 * coordinating their public key generation.  To do this, a single client
 * generates a public-secret key pair. This public key is shared with other keys
 * which use an element in the public key to generate their own public keys. The
 * clients generate a shared key pair using a scheme-specific approach, then
 * generate re-encryption keys.  Re-encryption keys are uploaded to the server.
 * Clients encrypt data with their public keys and send the encrypted data
 * server. The data is re-encrypted.  Computations are then run on the data. The
 * result is sent to each of the clients. One client runs a "Leader" multiparty
 * decryption operation with its own secret key.  All other clients run a
 * regular "Main" multiparty decryption with their own secret key. The resulting
 * partially decrypted ciphertext are then fully decrypted with the decryption
 * fusion algorithms.
 *
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmMultipartyNull : public LPMultipartyAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmMultipartyNull() {}

  /**
   * Function to generate public and private keys for multiparty homomrophic
   * encryption in coordination with a leading client that generated a first
   * public key.
   *
   * @param cc cryptocontext for the keys to be generated.
   * @param pk1 private key used for decryption to be fused.
   * @param makeSparse set to true if ring reduce by a factor of 2 is to be
   * used.
   * @param pre set to true if proxy re-encryption is used in multi-party
   * protocol
   * @return key pair including the private and public key
   */
  LPKeyPair<Element> MultipartyKeyGen(CryptoContext<Element> cc,
                                      const LPPublicKey<Element> pk1,
                                      bool makeSparse = false,
                                      bool pre = false) {
    LPKeyPair<Element> kp(new LPPublicKeyImpl<Element>(cc),
                          new LPPrivateKeyImpl<Element>(cc));

    Element a(cc->GetCryptoParameters()->GetElementParams(),
              Format::COEFFICIENT, true);
    kp.secretKey->SetPrivateElement(a);
    kp.publicKey->SetPublicElementAtIndex(0, a);
    kp.publicKey->SetPublicElementAtIndex(1, a);

    return kp;
  }

  /**
   * Function to generate public and private keys for multiparty homomrophic
   * encryption server key pair in coordination with secret keys of clients.
   *
   * @param cc cryptocontext for the keys to be generated.
   * @param secretkeys private keys used for decryption to be fused.
   * @param makeSparse set to true if ring reduce by a factor of 2 is to be
   * used.
   * @return key pair including the private and public key
   */
  LPKeyPair<Element> MultipartyKeyGen(
      CryptoContext<Element> cc,
      const vector<LPPrivateKey<Element>> &secretKeys,
      bool makeSparse = false) {
    LPKeyPair<Element> kp(new LPPublicKeyImpl<Element>(cc),
                          new LPPrivateKeyImpl<Element>(cc));

    Element a(cc->GetCryptoParameters()->GetElementParams(),
              Format::COEFFICIENT, true);
    kp.secretKey->SetPrivateElement(a);
    kp.publicKey->SetPublicElementAtIndex(0, a);
    kp.publicKey->SetPublicElementAtIndex(1, a);

    return kp;
  }

  /**
   * Method for main decryption operation run by most decryption clients for
   * multiparty homomorphic encryption
   *
   * @param privateKey private key used for decryption.
   * @param ciphertext ciphertext id decrypted.
   */
  Ciphertext<Element> MultipartyDecryptMain(
      const LPPrivateKey<Element> privateKey,
      ConstCiphertext<Element> ciphertext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();
    Element plaintext(ciphertext->GetElement());
    newCiphertext->SetElement(plaintext);

    return newCiphertext;
  }

  /**
   * Method for decryption operation run by the lead decryption client for
   * multiparty homomorphic encryption
   *
   * @param privateKey private key used for decryption.
   * @param ciphertext ciphertext id decrypted.
   */
  Ciphertext<Element> MultipartyDecryptLead(
      const LPPrivateKey<Element> privateKey,
      ConstCiphertext<Element> ciphertext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();
    Element plaintext(ciphertext->GetElement());
    newCiphertext->SetElement(plaintext);

    return newCiphertext;
  }

  /**
   * Method for fusing the partially decrypted ciphertext.
   *
   * @param &ciphertextVec ciphertext id decrypted.
   * @param *plaintext the plaintext output.
   * @return the decoding result.
   */
  DecryptResult MultipartyDecryptFusion(
      const vector<Ciphertext<Element>> &ciphertextVec,
      NativePoly *plaintext) const {
    Element b = ciphertextVec[0]->GetElement();
    const auto ptm = ciphertextVec[0]
                         ->GetCryptoContext()
                         ->GetCryptoParameters()
                         ->GetPlaintextModulus();
    *plaintext = b.DecryptionCRTInterpolate(ptm);
    return DecryptResult(plaintext->GetLength());
  }
};

/**
 * @brief Concrete feature class for Leveled SHE operations
 * @tparam Element a ring element.
 */
template <class Element>
class LPLeveledSHEAlgorithmNull : public LPLeveledSHEAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPLeveledSHEAlgorithmNull() {}

  /**
   * Method for ModReducing CipherText and the Private Key used for encryption.
   *
   * @param *cipherText Ciphertext to perform and apply modreduce on.
   */
  Ciphertext<Element> ModReduce(ConstCiphertext<Element> cipherText) const {
    Ciphertext<Element> newcipherText(new CiphertextImpl<Element>(*cipherText));

    return newcipherText;
  }

  /**
   * Method for ComposedEvalMult
   *
   * @param &cipherText1 ciphertext1, first input ciphertext to perform
   * multiplication on.
   * @param &cipherText2 cipherText2, second input ciphertext to perform
   * multiplication on.
   * @param &quadKeySwitchHint is for resultant quadratic secret key after
   * multiplication to the secret key of the particular level.
   * @param &cipherTextResult is the resulting ciphertext that can be decrypted
   * with the secret key of the particular level.
   */
  Ciphertext<Element> ComposedEvalMult(
      ConstCiphertext<Element> cipherText1,
      ConstCiphertext<Element> cipherText2,
      const LPEvalKey<Element> quadKeySwitchHint) const {
    Ciphertext<Element> prod =
        cipherText1->GetCryptoContext()->GetEncryptionAlgorithm()->EvalMult(
            cipherText1, cipherText2, quadKeySwitchHint);

    return this->ModReduce(prod);
  }

  /**
   * Method for Level Reduction from sk -> sk1. This method peforms a keyswitch
   * on the ciphertext and then performs a modulus reduction.
   *
   * @param &cipherText1 is the original ciphertext to be key switched and mod
   * reduced.
   * @param &linearKeySwitchHint is the linear key switch hint to perform the
   * key switch operation.
   * @param &cipherTextResult is the resulting ciphertext.
   */
  Ciphertext<Element> LevelReduce(ConstCiphertext<Element> cipherText1,
                                  const LPEvalKey<Element> linearKeySwitchHint,
                                  size_t levels) const {
    PALISADE_THROW(not_implemented_error,
                   "LevelReduce not implemented for Null");
  }
};

template <class Element>
class LPAlgorithmSHENull : public LPSHEAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmSHENull() {}

  /**
   * Function for evaluation addition on ciphertext.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Element> EvalAdd(ConstCiphertext<Element> ciphertext1,
                              ConstCiphertext<Element> ciphertext2) const {
    Ciphertext<Element> newCiphertext = ciphertext1->CloneEmpty();

    Element cResult = ciphertext1->GetElement() + ciphertext2->GetElement();

    newCiphertext->SetElement(std::move(cResult));

    return newCiphertext;
  }

  /**
   * Function for evaluation addition on ciphertext and plaintext
   *
   * @param ciphertext1 input ciphertext.
   * @param plaintext input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Element> EvalAdd(ConstCiphertext<Element> ciphertext,
                              ConstPlaintext plaintext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();

    Element cResult =
        ciphertext->GetElement() + plaintext->GetElement<Element>();

    newCiphertext->SetElement(std::move(cResult));

    return newCiphertext;
  }

  /**
   * Function for evaluation subtraction on ciphertext.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Element> EvalSub(ConstCiphertext<Element> ciphertext1,
                              ConstCiphertext<Element> ciphertext2) const {
    Ciphertext<Element> newCiphertext = ciphertext1->CloneEmpty();

    Element cResult = ciphertext1->GetElement() - ciphertext2->GetElement();

    newCiphertext->SetElement(std::move(cResult));

    return newCiphertext;
  }

  /**
   * Function for evaluation addition on ciphertext and plaintext
   *
   * @param ciphertext1 input ciphertext.
   * @param plaintext input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Element> EvalSub(ConstCiphertext<Element> ciphertext,
                              ConstPlaintext plaintext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();

    Element cResult =
        ciphertext->GetElement() - plaintext->GetElement<Element>();

    newCiphertext->SetElement(std::move(cResult));

    return newCiphertext;
  }

  /**
   * Function for evaluating multiplication on ciphertext.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Poly> EvalMult(ConstCiphertext<Poly> ciphertext1,
                            ConstCiphertext<Poly> ciphertext2) const;

  /**
   * Function for evaluating multiplication of ciphertext by plaintext
   *
   * @param ciphertext input ciphertext.
   * @param plaintext input plaintext embedded in cryptocontext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<Poly> EvalMult(ConstCiphertext<Poly> ciphertext1,
                            ConstPlaintext plaintext) const;

  Ciphertext<Poly> EvalMult(ConstCiphertext<Poly> ciphertext,
                            double constant) const {
    PALISADE_THROW(not_implemented_error,
                   "Scalar multiplication is not implemented for this scheme");
  }

  /**
   * Function for evaluating multiplication on ciphertext.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<NativePoly> EvalMult(
      ConstCiphertext<NativePoly> ciphertext1,
      ConstCiphertext<NativePoly> ciphertext2) const;

  /**
   * Function for evaluating multiplication of ciphertext by plaintext
   *
   * @param ciphertext input ciphertext.
   * @param plaintext input plaintext embedded in cryptocontext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<NativePoly> EvalMult(ConstCiphertext<NativePoly> ciphertext1,
                                  ConstPlaintext plaintext) const;

  Ciphertext<NativePoly> EvalMult(ConstCiphertext<NativePoly> ciphertext,
                                  double constant) const {
    PALISADE_THROW(not_implemented_error,
                   "Scalar multiplication is not implemented for this scheme");
  }

  /**
   * Function for evaluating multiplication on ciphertext.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly> ciphertext1,
                                ConstCiphertext<DCRTPoly> ciphertext2) const;

  /**
   * Function for evaluating multiplication of ciphertext by plaintext
   *
   * @param ciphertext input ciphertext.
   * @param plaintext input plaintext embedded in cryptocontext.
   * @return the new resulting ciphertext.
   */
  Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly> ciphertext,
                                ConstPlaintext plaintext) const;

  Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly> ciphertext,
                                double constant) const {
    PALISADE_THROW(not_implemented_error,
                   "Scalar multiplication is not implemented for this scheme");
  }
  /**
   * Function for evaluating multiplication on ciphertext followed by key
   * switching operation.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @param ek is the evaluation key to make the newCiphertext decryptable by
   * the same secret key as that of the operands
   * @return the new resulting ciphertext.
   */
  Ciphertext<Element> EvalMult(ConstCiphertext<Element> ciphertext1,
                               ConstCiphertext<Element> ciphertext2,
                               const LPEvalKey<Element> ek) const {
    return EvalMult(ciphertext1, ciphertext2);
  }

  /**
   * Unimplemented function to support  a multiplication with depth larger than
   * 2 for the NULL scheme.
   *
   * @param ciphertext1 The first input ciphertext.
   * @param ciphertext2 The second input ciphertext.
   * @param evalKey The evaluation key input.
   * @return A shared pointer to the ciphertext which is the EvalMult of the two
   * inputs.
   */
  Ciphertext<Element> EvalMultAndRelinearize(
      ConstCiphertext<Element> ciphertext1,
      ConstCiphertext<Element> ciphertext2,
      const vector<LPEvalKey<Element>> &evalKey) const {
    // since there's no relinearize needed in NULL:
    return EvalMult(ciphertext1, ciphertext2);
  }

  /**
   * Function for homomorpic negation of ciphertext.
   *
   * @param &ciphertext input ciphertext.
   * @param *newCiphertext the new resulting ciphertext.
   */

  Ciphertext<Element> EvalNegate(ConstCiphertext<Element> ciphertext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();

    const Element &c1 = ciphertext->GetElement();

    Element cResult = c1.Negate();

    newCiphertext->SetElement(std::move(cResult));

    return newCiphertext;
  }

  /**
   * Method for generating a KeySwitchHint
   *
   * @param &originalPrivateKey Original private key used for encryption.
   * @param &newPrivateKey New private key to generate the keyswitch hint.
   * @param *keySwitchHint is where the resulting keySwitchHint will be placed.
   */
  LPEvalKey<Element> KeySwitchGen(
      const LPPrivateKey<Element> originalPrivateKey,
      const LPPrivateKey<Element> newPrivateKey) const {
    return LPEvalKey<Element>(new LPEvalKeyNTRURelinImpl<Element>(
        originalPrivateKey->GetCryptoContext()));
  }

  /**
   * Function to define key switching operation
   *
   * @param &keySwitchHint the evaluation key.
   * @param &ciphertext the input ciphertext.
   * @param *newCiphertext the new ciphertext.
   */
  Ciphertext<Element> KeySwitch(const LPEvalKey<Element> keySwitchHint,
                                ConstCiphertext<Element> cipherText) const {
    Ciphertext<Element> newCiphertext = cipherText->CloneEmpty();
    return newCiphertext;
  }

  /**
   * Method for KeySwitching based on RLWE relinearization.
   * Function to generate 1..log(q) encryptions for each bit of the original
   * private key
   *
   * @param &newPublicKey encryption key for the new ciphertext.
   * @param origPrivateKey original private key used for decryption.
   */
  LPEvalKey<Element> KeySwitchRelinGen(
      const LPPublicKey<Element> newPublicKey,
      const LPPrivateKey<Element> origPrivateKey) const {
    return LPEvalKey<Element>(new LPEvalKeyNTRURelinImpl<Element>(
        origPrivateKey->GetCryptoContext()));
  }

  /**
   * Method for KeySwitching based on RLWE relinearization
   *
   * @param evalKey the evaluation key.
   * @param ciphertext the input ciphertext.
   * @return the resulting Ciphertext
   */
  Ciphertext<Element> KeySwitchRelin(
      const LPEvalKey<Element> evalKey,
      ConstCiphertext<Element> ciphertext) const {
    Ciphertext<Element> newCiphertext = ciphertext->CloneEmpty();
    return newCiphertext;
  }

  /**
   * Function to generate key switch hint on a ciphertext for depth 2.
   *
   * @param &newPrivateKey private key for the new ciphertext.
   * @param *keySwitchHint the key switch hint.
   */
  LPEvalKey<Element> EvalMultKeyGen(
      const LPPrivateKey<Element> originalPrivateKey) const {
    LPEvalKey<Element> EK(new LPEvalKeyNTRURelinImpl<Element>(
        originalPrivateKey->GetCryptoContext()));

    Element a(originalPrivateKey->GetCryptoContext()
                  ->GetCryptoParameters()
                  ->GetElementParams(),
              Format::COEFFICIENT, true);
    vector<Element> evalKeyElements;
    evalKeyElements.push_back(std::move(a));

    EK->SetAVector(std::move(evalKeyElements));

    return EK;
  }

  /**
   * Function to generate key switch hint on a ciphertext for depth more than 2.
   * Currently it is not supported.
   *
   * @param &newPrivateKey private key for the new ciphertext.
   * @param *keySwitchHint the key switch hint list.
   */
  vector<LPEvalKey<Element>> EvalMultKeysGen(
      const LPPrivateKey<Element> originalPrivateKey) const {
    std::string errMsg =
        "LPAlgorithmSHENULL::EvalMultKeysGen is not implemented for NULL SHE "
        "Scheme.";
    PALISADE_THROW(not_implemented_error, errMsg);
  }

  /**
   * Function for evaluating automorphism of ciphertext at index i
   *
   * @param ciphertext the input ciphertext.
   * @param i automorphism index
   * @param &evalKeys - reference to the map of evaluation keys generated by
   * EvalAutomorphismKeyGen.
   * @return resulting ciphertext
   */
  Ciphertext<Element> EvalAutomorphism(
      ConstCiphertext<Element> ciphertext, usint i,
      const std::map<usint, LPEvalKey<Element>> &evalKeys) const {
    Ciphertext<Element> permutedCiphertext(
        new CiphertextImpl<Element>(*ciphertext));

    Element temp = ciphertext->GetElement();

    temp = temp.AutomorphismTransform(i);

    permutedCiphertext->SetElement(temp);

    return permutedCiphertext;
  }

  /**
   * Generate automophism keys
   *
   * @param privateKey private key.
   * @param indexList list of automorphism indices to be computed
   * @return returns the evaluation keys
   */

  shared_ptr<std::map<usint, LPEvalKey<Element>>> EvalAutomorphismKeyGen(
      const LPPublicKey<Element> publicKey,
      const LPPrivateKey<Element> origPrivateKey,
      const std::vector<usint> &indexList) const {
    shared_ptr<std::map<usint, LPEvalKey<Element>>> evalKeys(
        new std::map<usint, LPEvalKey<Element>>());

    for (auto &i : indexList) {
      (*evalKeys)[i] = this->KeySwitchGen(origPrivateKey, origPrivateKey);
    }

    return evalKeys;
  }

  shared_ptr<std::map<usint, LPEvalKey<Element>>> EvalAutomorphismKeyGen(
      const LPPrivateKey<Element> privateKey,
      const std::vector<usint> &indexList) const {
    shared_ptr<std::map<usint, LPEvalKey<Element>>> evalKeys(
        new std::map<usint, LPEvalKey<Element>>());

    for (auto &i : indexList) {
      (*evalKeys)[i] = this->KeySwitchGen(privateKey, privateKey);
    }

    return evalKeys;
  }

 private:
  typename Element::PolyType ElementNullSchemeMultiply(
      const typename Element::PolyType &c1,
      const typename Element::PolyType &c2, const BigInteger &ptmod) const {
    typename Element::PolyType cResult(c1.GetParams(), Format::COEFFICIENT,
                                       true);

    if (c1.GetParams()->OrderIsPowerOfTwo()) {
      typename Element::PolyType cLarger(c1.GetParams(), Format::COEFFICIENT,
                                         true);

      typename Element::PolyType::Integer ptm(ptmod.ConvertToInt());

      int ringdim = c1.GetRingDimension();
      for (int c1e = 0; c1e < ringdim; c1e++) {
        typename Element::PolyType::Integer answer, c1val, c2val, prod;
        c1val = c1.at(c1e);
        if (c1val != typename Element::PolyType::Integer(0)) {
          for (int c2e = 0; c2e < ringdim; c2e++) {
            c2val = c2.at(c2e);
            if (c2val != typename Element::PolyType::Integer(0)) {
              prod = c1val * c2val;

              int index = (c1e + c2e);

              if (index >= ringdim) {
                index %= ringdim;
                cLarger.at(index) = (cLarger.at(index) + prod) % ptm;
              } else
                cResult.at(index) = (cResult.at(index) + prod) % ptm;
            }
          }
        }
      }

      // fold cLarger back into the answer
      for (int i = 0; i < ringdim; i++) {
        typename Element::PolyType::Integer adj;
        adj = cResult.at(i) + (ptm - cLarger.at(i)) % ptm;
        cResult.at(i) = adj % ptm;
      }

    } else
      PALISADE_THROW(
          not_implemented_error,
          "Polynomial multiplication in coefficient representation is not "
          "currently supported for non-power-of-two polynomials");

    return std::move(cResult);
  }
};

/**
 * @brief Parameter generation for BFV.
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmParamsGenNull
    : public LPParameterGenerationAlgorithm<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmParamsGenNull() {}

  /**
   * Method for computing all derived parameters based on chosen primitive
   * parameters
   *
   * @param cryptoParams the crypto parameters object to be populated with
   * parameters.
   * @param evalAddCount number of EvalAdds assuming no EvalMult and KeySwitch
   * operations are performed.
   * @param evalMultCount number of EvalMults assuming no EvalAdd and KeySwitch
   * operations are performed.
   * @param keySwitchCount number of KeySwitch operations assuming no EvalAdd
   * and EvalMult operations are performed.
   * @param dcrtBits number of bits in each CRT modulus
   * @param n ring dimension in case the user wants to use a custom ring
   * dimension
   */
  bool ParamsGen(shared_ptr<LPCryptoParameters<Element>> cryptoParams,
                 int32_t evalAddCount = 0, int32_t evalMultCount = 0,
                 int32_t keySwitchCount = 0, size_t dcrtBits = 0,
                 uint32_t n = 0) const {
    return true;
  }
};

/**
 * @brief Main public key encryption scheme for Null implementation,
 * @tparam Element a ring element.
 */
template <class Element>
class LPPublicKeyEncryptionSchemeNull
    : public LPPublicKeyEncryptionScheme<Element> {
 public:
  LPPublicKeyEncryptionSchemeNull() : LPPublicKeyEncryptionScheme<Element>() {
    this->m_algorithmParamsGen.reset(new LPAlgorithmParamsGenNull<Element>());
  }

  bool operator==(const LPPublicKeyEncryptionScheme<Element> &sch) const {
    if (dynamic_cast<const LPPublicKeyEncryptionSchemeNull<Element> *>(&sch) ==
        0)
      return false;
    return true;
  }

  void Enable(PKESchemeFeature feature) {
    switch (feature) {
      case ENCRYPTION:
        if (this->m_algorithmEncryption == NULL)
          this->m_algorithmEncryption.reset(new LPAlgorithmNull<Element>());
        break;
      case PRE:
        if (this->m_algorithmEncryption == NULL)
          this->m_algorithmEncryption.reset(new LPAlgorithmNull<Element>());
        if (this->m_algorithmPRE == NULL)
          this->m_algorithmPRE.reset(new LPAlgorithmPRENull<Element>());
        break;
      case MULTIPARTY:
        if (this->m_algorithmEncryption == NULL)
          this->m_algorithmEncryption.reset(new LPAlgorithmNull<Element>());
        if (this->m_algorithmMultiparty == NULL)
          this->m_algorithmMultiparty.reset(
              new LPAlgorithmMultipartyNull<Element>());
        break;
      case SHE:
        if (this->m_algorithmEncryption == NULL)
          this->m_algorithmEncryption.reset(new LPAlgorithmNull<Element>());
        if (this->m_algorithmSHE == NULL)
          this->m_algorithmSHE.reset(new LPAlgorithmSHENull<Element>());
        break;
      case FHE:
        PALISADE_THROW(not_implemented_error,
                       "FHE feature not supported for Null scheme");
      case LEVELEDSHE:
        if (this->m_algorithmEncryption == NULL)
          this->m_algorithmEncryption.reset(new LPAlgorithmNull<Element>());
        if (this->m_algorithmLeveledSHE == NULL)
          this->m_algorithmLeveledSHE.reset(
              new LPLeveledSHEAlgorithmNull<Element>());
        break;
      case ADVANCEDSHE:
        PALISADE_THROW(not_implemented_error,
                       "ADVANCEDSHE feature not supported for NULL scheme");
      case ADVANCEDMP:
        PALISADE_THROW(not_implemented_error,
                       "ADVANCEDMP feature not supported for NULL scheme");
    }
  }

  template <class Archive>
  void save(Archive &ar, std::uint32_t const version) const {
    ar(::cereal::base_class<LPPublicKeyEncryptionScheme<Element>>(this));
  }

  template <class Archive>
  void load(Archive &ar, std::uint32_t const version) {
    ar(::cereal::base_class<LPPublicKeyEncryptionScheme<Element>>(this));
  }

  std::string SerializedObjectName() const { return "NullScheme"; }
};

}  // namespace lbcrypto

#endif /* SRC_LIB_CRYPTO_NULLSCHEME_H_ */
