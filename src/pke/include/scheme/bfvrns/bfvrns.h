/**
 * @file bfvrns.h -- Operations for the HPS RNS variant of the BFV cryptoscheme.
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
/*
 *
 * This code implements a RNS variant of the Brakerski-Fan-Vercauteren (BFV)
 *homomorphic encryption scheme.  This scheme is also referred to as the FV
 *scheme.
 *
 * The BFV scheme is introduced in the following papers:
 *   - Zvika Brakerski (2012). Fully Homomorphic Encryption without Modulus
 *Switching from Classical GapSVP. Cryptology ePrint Archive, Report 2012/078.
 *(https://eprint.iacr.org/2012/078)
 *   - Junfeng Fan and Frederik Vercauteren (2012). Somewhat Practical Fully
 *Homomorphic Encryption.  Cryptology ePrint Archive, Report 2012/144.
 *(https://eprint.iacr.org/2012/144.pdf)
 *
 * Our implementation builds from the designs here:
 *   - Halevi S., Polyakov Y., and Shoup V. An Improved RNS Variant of the BFV
 *Homomorphic Encryption Scheme. Cryptology ePrint Archive, Report 2018/117.
 *(https://eprint.iacr.org/2018/117)
 *   - Lepoint T., Naehrig M. (2014) A Comparison of the Homomorphic Encryption
 *Schemes FV and YASHE. In: Pointcheval D., Vergnaud D. (eds) Progress in
 *Cryptology – AFRICACRYPT 2014. AFRICACRYPT 2014. Lecture Notes in Computer
 *Science, vol 8469. Springer, Cham. (https://eprint.iacr.org/2014/062.pdf)
 *	 - Jean-Claude Bajard and Julien Eynard and Anwar Hasan and Vincent
 *Zucca (2016). A Full RNS Variant of FV like Somewhat Homomorphic Encryption
 *Schemes. Cryptology ePrint Archive, Report 2016/510.
 *(https://eprint.iacr.org/2016/510)
 *	 - Ahmad Al Badawi and Yuriy Polyakov and Khin Mi Mi Aung and Bharadwaj
 *Veeravalli and Kurt Rohloff (2018). Implementation and Performance Evaluation
 *of RNS Variants of the BFV Homomorphic Encryption Scheme. Cryptology ePrint
 *Archive, Report 2018/589. {https://eprint.iacr.org/2018/589}
 */

#ifndef LBCRYPTO_CRYPTO_BFVRNS_H
#define LBCRYPTO_CRYPTO_BFVRNS_H

#include "palisade.h"

namespace lbcrypto {

/**
 * @brief This is the parameters class for the BFVrns encryption scheme.  This
 * scheme is also referred to as the FVrns scheme.
 *
 * @tparam Element a ring element type.
 */
template <class Element>
class LPCryptoParametersBFVrns : public LPCryptoParametersRLWE<Element> {
 public:
  /**
   * Default constructor.
   */
  LPCryptoParametersBFVrns();

  /**
   * Copy constructor.
   * @param rhs - source
   */
  LPCryptoParametersBFVrns(const LPCryptoParametersBFVrns& rhs);
  /**
   * Constructor that initializes values.  Note that it is possible to set
   * parameters in a way that is overall infeasible for actual use.  There are
   * fewer degrees of freedom than parameters provided.  Typically one chooses
   * the basic noise, assurance and security parameters as the typical
   * community-accepted values, then chooses the plaintext modulus and depth as
   * needed.  The element parameters should then be choosen to provide
   * correctness and security.  In some cases we would need to operate over
   * already encrypted/provided ciphertext and the depth needs to be
   * pre-computed for initial settings.
   *
   * @param &params Element parameters.  This will depend on the specific class
   * of element being used.
   * @param &plaintextModulus Plaintext modulus, typically denoted as p in most
   * publications.
   * @param distributionParameter Noise distribution parameter, typically
   * denoted as /sigma in most publications.  Community standards typically call
   * for a value of 3 to 6. Lower values provide more room for computation while
   * larger values provide more security.
   * @param assuranceMeasure Assurance level, typically denoted as w in most
   * applications.  This is oftern perceived as a fudge factor in the
   * literature, with a typical value of 9.
   * @param securityLevel Security level as Root Hermite Factor.  We use the
   * Root Hermite Factor representation of the security level to better conform
   * with US ITAR and EAR export regulations.  This is typically represented as
   * /delta in the literature.  Typically a Root Hermite Factor of 1.006 or less
   * provides reasonable security for RLWE crypto schemes.
   * @param relinWindow The size of the relinearization window.  This is
   * relevant when using this scheme for proxy re-encryption, and the value is
   * denoted as r in the literature.
   * @param mode optimization setting (RLWE vs OPTIMIZED)
   * @param depth is the depth of computation circuit supported for these
   * parameters (not used now; for future use).
   * @param maxDepth is the maximum homomorphic multiplication depth before
   * performing relinearization
   */
  LPCryptoParametersBFVrns(shared_ptr<typename Element::Params> params,
                           const PlaintextModulus& plaintextModulus,
                           float distributionParameter, float assuranceMeasure,
                           float securityLevel, usint relinWindow,
                           MODE mode = RLWE, int depth = 1, int maxDepth = 2);

  /**
   * Constructor that initializes values.
   *
   * @param &params element parameters.
   * @param &encodingParams plaintext space parameters.
   * @param distributionParameter noise distribution parameter.
   * @param assuranceMeasure assurance level. = BigInteger::ZERO
   * @param securityLevel security level (root Hermite factor).
   * @param relinWindow the size of the relinearization window.
   * @param mode optimization setting (RLWE vs OPTIMIZED)
   * @param depth is the depth of computation circuit supported for these
   * parameters (not used now; for future use).
   * @param maxDepth is the maximum homomorphic multiplication depth before
   * performing relinearization
   */
  LPCryptoParametersBFVrns(shared_ptr<typename Element::Params> params,
                           EncodingParams encodingParams,
                           float distributionParameter, float assuranceMeasure,
                           float securityLevel, usint relinWindow,
                           MODE mode = RLWE, int depth = 1, int maxDepth = 2);

  /**
   * Constructor that initializes values.
   *
   * @param &params element parameters.
   * @param &encodingParams plaintext space parameters.
   * @param distributionParameter noise distribution parameter.
   * @param assuranceMeasure assurance level. = BigInteger::ZERO
   * @param securityLevel standard security level
   * @param relinWindow the size of the relinearization window.
   * @param mode optimization setting (RLWE vs OPTIMIZED)
   * @param depth is the depth of computation circuit supported for these
   * parameters (not used now; for future use).
   * @param maxDepth is the maximum homomorphic multiplication depth before
   * performing relinearization
   */
  LPCryptoParametersBFVrns(shared_ptr<typename Element::Params> params,
                           EncodingParams encodingParams,
                           float distributionParameter, float assuranceMeasure,
                           SecurityLevel securityLevel, usint relinWindow,
                           MODE mode = RLWE, int depth = 1, int maxDepth = 2);

  /**
   * Destructor
   */
  virtual ~LPCryptoParametersBFVrns() {}

  /**
   * Computes all tables needed for decryption, homomorphic multiplication, and
   * key switching
   * @return true on success
   */
  bool PrecomputeCRTTables();

  /**
   * Gets Auxiliary CRT basis S=s1*s2*..sn used in homomorphic multiplication
   *
   * @return the precomputed CRT basis
   */
  const shared_ptr<ILDCRTParams<BigInteger>> GetDCRTParamsS() const {
    return m_paramsS;
  }

  /**
   * Auxiliary expanded CRT basis Q*S = v1*v2*...*vn used in homomorphic
   * multiplication
   *
   * @return the precomputed CRT basis
   */
  const shared_ptr<ILDCRTParams<BigInteger>> GetDCRTParamsQS() const {
    return m_paramsQS;
  }

  /**
   * Gets the Barrett modulo reduction precomputations for Q
   *
   * @return the precomputed table
   */
  std::vector<DoubleNativeInt> const& GetDCRTParamsQModulimu() const {
    return m_qModulimu;
  }

  /**
   * Gets the Barrett modulo reduction precomputations for S
   *
   * @return the precomputed table
   */
  std::vector<DoubleNativeInt> const& GetDCRTParamsSModulimu() const {
    return m_sModulimu;
  }

  /**
   * Gets the precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi; CRT modulus <
   * 45 bits
   *
   * @return the precomputed table
   */
  const std::vector<double>& GetCRTDecryptionFloatTable() const {
    return m_CRTDecryptionFloatTable;
  }

  /**
   * Gets the precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi; CRT modulus is
   * betweeen 45 and 57 bits
   *
   * @return the precomputed table
   */
  const std::vector<long double>& GetCRTDecryptionExtFloatTable() const {
    return m_CRTDecryptionExtFloatTable;
  }

#ifndef NO_QUADMATH
  /**
   * Gets the precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi; CRT modulus
   * has 58..60 bits
   *
   * @return the precomputed table
   */
  const std::vector<QuadFloat>& GetCRTDecryptionQuadFloatTable() const {
    return m_CRTDecryptionQuadFloatTable;
  }
#endif
  /**
   * Gets the precomputed table of floor[(p*[(Q/qi)^{-1}]_qi)/qi]_p
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTDecryptionIntTable() const {
    return m_CRTDecryptionIntTable;
  }

  /**
   * Gets the NTL precomputation for the precomputed table of
   * floor[(p*[(Q/qi)^{-1}]_qi)/qi]_p
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTDecryptionIntPreconTable() const {
    return m_CRTDecryptionIntPreconTable;
  }

  /**
   * Gets the precomputed table of floor(Q/p) mod qi
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTDeltaTable() const {
    return m_CRTDeltaTable;
  }

  /**
   * Gets the precomputed table of (Q/qi)^{-1} mod qi
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTInverseTable() const {
    return m_CRTInverseTable;
  }

  /**
   * Gets the NTL precomputation for the precomputed table of (Q/qi)^{-1} mod qi
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTInversePreconTable() const {
    return m_CRTInversePreconTable;
  }

  /**
   * Gets the precomputed table of (Q/qi) mod si
   *
   * @return the precomputed table
   */
  const std::vector<std::vector<NativeInteger>>& GetCRTqDivqiModsiTable()
      const {
    return m_CRTqDivqiModsiTable;
  }

  /**
   * Gets the precomputed table of Q mod si
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTqModsiTable() const {
    return m_CRTqModsiTable;
  }

  /**
   * Gets the precomputed table of [p*S*(Q*S/vi)^{-1}]_vi / vi
   *
   * @return the precomputed table
   */
  const std::vector<long double>& GetCRTMultFloatTable() const {
    return m_CRTMultFloatTable;
  }

  /**
   * Gets the precomputed table of floor[p*S*[(Q*S/vi)^{-1}]_vi/vi] mod si
   *
   * @return the precomputed table
   */
  const std::vector<std::vector<NativeInteger>>& GetCRTMultIntTable() const {
    return m_CRTMultIntTable;
  }

  /**
   * Gets the precomputed table of (S/si)^{-1} mod si
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTSInverseTable() const {
    return m_CRTSInverseTable;
  }

  /**
   * Gets the NTL precomputation for the precomputed table of (S/si)^{-1} mod si
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTSInversePreconTable() const {
    return m_CRTSInversePreconTable;
  }

  /**
   * Gets the precomputed table of (S/si) mod qi table
   *
   * @return the precomputed table
   */
  const std::vector<std::vector<NativeInteger>>& GetCRTsDivsiModqiTable()
      const {
    return m_CRTsDivsiModqiTable;
  }

  /**
   * Gets the precomputed table of S mod qi table
   *
   * @return the precomputed table
   */
  const std::vector<NativeInteger>& GetCRTsModqiTable() const {
    return m_CRTsModqiTable;
  }

  /**
   * == operator to compare to this instance of LPCryptoParametersBFVrns object.
   *
   * @param &rhs LPCryptoParameters to check equality against.
   */
  bool operator==(const LPCryptoParameters<Element>& rhs) const {
    const LPCryptoParametersBFVrns<Element>* el =
        dynamic_cast<const LPCryptoParametersBFVrns<Element>*>(&rhs);

    if (el == 0) return false;

    return LPCryptoParametersRLWE<Element>::operator==(rhs);
  }

  void PrintParameters(std::ostream& os) const {
    LPCryptoParametersRLWE<Element>::PrintParameters(os);
  }

  // NOTE that we do not serialize any of the members declared in this class.
  // they are all cached computations, and get recomputed in any implementation
  // that does a deserialization
  template <class Archive>
  void save(Archive& ar, std::uint32_t const version) const {
    ar(::cereal::base_class<LPCryptoParametersRLWE<Element>>(this));
  }

  template <class Archive>
  void load(Archive& ar, std::uint32_t const version) {
    if (version > SerializedVersion()) {
      PALISADE_THROW(deserialize_error,
                     "serialized object version " + std::to_string(version) +
                         " is from a later version of the library");
    }
    ar(::cereal::base_class<LPCryptoParametersRLWE<Element>>(this));

    PrecomputeCRTTables();
  }

  std::string SerializedObjectName() const { return "BFVrnsSchemeParameters"; }
  static uint32_t SerializedVersion() { return 1; }

 private:
  // Auxiliary CRT basis S=s1*s2*..sn used in homomorphic multiplication
  shared_ptr<ILDCRTParams<BigInteger>> m_paramsS;

  // Auxiliary expanded CRT basis Q*S = v1*v2*...*vn used in homomorphic
  // multiplication
  shared_ptr<ILDCRTParams<BigInteger>> m_paramsQS;

  // Barrett modulo reduction precomputation
  std::vector<DoubleNativeInt> m_qModulimu;

  // Barrett modulo reduction precomputation
  std::vector<DoubleNativeInt> m_sModulimu;

  // when log2 qi <= 44 bits
  // Stores a precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi
  std::vector<double> m_CRTDecryptionFloatTable;

  // when 44 < log2 qi <= 57  bits
  // Stores a precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi
  std::vector<long double> m_CRTDecryptionExtFloatTable;

#ifndef NO_QUADMATH
  // when log2 qi = 58..60 bits
  // Stores a precomputed table of ((p*[(Q/qi)^{-1}]_qi)%qi)/qi
  std::vector<QuadFloat> m_CRTDecryptionQuadFloatTable;
#endif

  // Stores a precomputed table of floor[(p*[(Q/qi)^{-1}]_qi)/qi]_p
  std::vector<NativeInteger> m_CRTDecryptionIntTable;

  // Stores a precomputed table of floor(Q/p) mod qi
  std::vector<NativeInteger> m_CRTDeltaTable;

  // Stores a precomputed table of (Q/qi)^{-1} mod qi
  std::vector<NativeInteger> m_CRTInverseTable;

  // Stores an NTL precomputation for the precomputed table of (Q/qi)^{-1} mod
  // qi
  std::vector<NativeInteger> m_CRTInversePreconTable;

  // Stores a precomputed table of (Q/qi) mod si
  std::vector<std::vector<NativeInteger>> m_CRTqDivqiModsiTable;

  // Stores a precomputed table of Q mod si
  std::vector<NativeInteger> m_CRTqModsiTable;

  // Stores a precomputed table of floor[p*S*[(Q*S/vi)^{-1}]_vi/vi] mod si
  std::vector<std::vector<NativeInteger>> m_CRTMultIntTable;

  // Stores a precomputed table of [p*S*(Q*S/vi)^{-1}]_vi / vi
  std::vector<long double> m_CRTMultFloatTable;

  // Stores a precomputed table of (S/si)^{-1} mod si
  std::vector<NativeInteger> m_CRTSInverseTable;

  // Stores an NTL precomputation for the precomputed table of (S/si)^{-1} mod
  // si
  std::vector<NativeInteger> m_CRTSInversePreconTable;

  // Stores a precomputed table of (S/si) mod qi table
  std::vector<std::vector<NativeInteger>> m_CRTsDivsiModqiTable;

  // Stores a precomputed table of S mod qi table
  std::vector<NativeInteger> m_CRTsModqiTable;

  // Stores an NTL precomputation for the precomputed table of
  // floor[(p*[(Q/qi)^{-1}]_qi)/qi]_p
  std::vector<NativeInteger> m_CRTDecryptionIntPreconTable;
};

/**
 * @brief Parameter generation for BFVrns.  This scheme is also referred to as
 * the FV scheme.
 *
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmParamsGenBFVrns : public LPAlgorithmParamsGenBFV<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmParamsGenBFVrns() {}

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
                 int32_t keySwitchCount = 0, size_t dcrBits = 60,
                 uint32_t n = 0) const;
};

/**
 * @brief Encryption algorithm implementation for BFVrns for the basic public
 * key encrypt, decrypt and key generation methods for the BFVrns encryption
 * scheme.
 *
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmBFVrns : public LPAlgorithmBFV<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmBFVrns() {}

  /**
   * Method for encrypting plaintext using BFVrns.
   *
   * @param publicKey public key used for encryption.
   * @param &plaintext the plaintext input.
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @return ciphertext which results from encryption.
   */
  Ciphertext<Element> Encrypt(const LPPublicKey<Element> publicKey,
                              Element plaintext) const;

  /**
   * Method for encrypting plaintext with private key using BFVrns.
   *
   * @param privateKey private key used for encryption.
   * @param plaintext the plaintext input.
   * @param doEncryption encrypts if true, embeds (encodes) the plaintext into
   * cryptocontext if false
   * @return ciphertext which results from encryption.
   */
  Ciphertext<Element> Encrypt(const LPPrivateKey<Element> privateKey,
                              Element plaintext) const;

  /**
   * Method for decrypting using BFVrns. See the class description for citations
   * on where the algorithms were taken from.
   *
   * @param privateKey private key used for decryption.
   * @param ciphertext ciphertext to be decrypted.
   * @param *plaintext the plaintext output.
   * @return the decrypted plaintext returned.
   */
  DecryptResult Decrypt(const LPPrivateKey<Element> privateKey,
                        ConstCiphertext<Element> ciphertext,
                        NativePoly* plaintext) const;
};

/**
 * @brief SHE algorithms implementation for BFVrns.
 *
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmSHEBFVrns : public LPAlgorithmSHEBFV<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmSHEBFVrns() {}

  /**
   * Function for homomorphic addition of ciphertext and plaintext.
   *
   * @param ct1 input ciphertext.
   * @param pt  input ciphertext.
   * @return new ciphertext.
   */
  Ciphertext<Element> EvalAdd(ConstCiphertext<Element> ct,
                              ConstPlaintext pt) const;

  /**
   * Function for homomorphic subtraction of ciphertext ans plaintext.
   *
   * @param ct input ciphertext.
   * @param pt input ciphertext.
   * @return new ciphertext.
   */
  Ciphertext<Element> EvalSub(ConstCiphertext<Element> ct1,
                              ConstPlaintext pt) const;

  /**
   * Function for homomorphic evaluation of ciphertexts.
   * The multiplication is supported for a fixed level without keyswitching
   * requirement (default level=2). If the total depth of the ciphertexts
   * exceeds the supported level, it throws an error.
   *
   * @param ciphertext1 first input ciphertext.
   * @param ciphertext2 second input ciphertext.
   * @return resulting EvalMult ciphertext.
   */
  Ciphertext<Element> EvalMult(ConstCiphertext<Element> ct1,
                               ConstCiphertext<Element> ct2) const;

  /**
   * Method for generating a KeySwitchHint using RLWE relinearization
   *
   * @param originalPrivateKey Original private key used for encryption.
   * @param newPrivateKey New private key to generate the keyswitch hint.
   * @return resulting keySwitchHint.
   */
  LPEvalKey<Element> KeySwitchGen(
      const LPPrivateKey<Element> originalPrivateKey,
      const LPPrivateKey<Element> newPrivateKey) const;

  /**
   * Method for key switching based on a KeySwitchHint using RLWE
   * relinearization
   *
   * @param keySwitchHint Hint required to perform the ciphertext switching.
   * @param &cipherText Original ciphertext to perform switching on.
   * @return new ciphertext
   */
  Ciphertext<Element> KeySwitch(const LPEvalKey<Element> keySwitchHint,
                                ConstCiphertext<Element> cipherText) const;

  /**
   * Function for evaluating multiplication on ciphertext followed by
   * relinearization operation. Currently it assumes that the input arguments
   * have total depth smaller than the supported depth. Otherwise, it throws an
   * error.
   *
   * @param ct1 first input ciphertext.
   * @param ct2 second input ciphertext.
   * @param ek is the evaluation key to make the newCiphertext
   *  decryptable by the same secret key as that of ciphertext1 and ciphertext2.
   * @return new ciphertext
   */
  Ciphertext<Element> EvalMultAndRelinearize(
      ConstCiphertext<Element> ct1, ConstCiphertext<Element> ct,
      const vector<LPEvalKey<Element>>& ek) const;
};

/**
 * @brief PRE algorithms implementation for BFVrns.
 *
 * @tparam Element a ring element.
 */
template <class Element>
class LPAlgorithmPREBFVrns : public LPAlgorithmPREBFV<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmPREBFVrns() {}

  /**
   * The generation of re-encryption keys is based on the BG-PRE scheme
   * described in Polyakov, et. al., "Fast proxy re-encryption for
   * publish/subscribe systems".
   *
   * The above scheme was found to have a weakness in Cohen, "What about Bob?
   * The inadequacy of CPA Security for proxy re-encryption". Section 5.1 shows
   * an attack where given an original ciphertext c=(c0,c1) and a re-encrypted
   * ciphertext c'=(c'0, c'1), the subscriber (Bob) can compute the secret key
   * of the publisher (Alice).
   *
   * We fix this vulnerability by making re-encryption keys be encryptions of
   * the s*(2^{i*r}) terms, instead of simple addition as previously defined.
   * This makes retrieving the secret key using the above attack as hard as
   * breaking the RLWE assumption.
   *
   * Our modification makes the scheme CPA-secure, but does not achieve
   * HRA-security as it was defined in the Cohen paper above. Please look at the
   * ReEncrypt method for an explanation of the two security definitions and how
   * to achieve each in Palisade.
   *
   * @param newKey public key for the new private key.
   * @param origPrivateKey original private key used for decryption.
   * @return evalKey the evaluation key for switching the ciphertext to be
   * decryptable by new private key.
   */
  LPEvalKey<Element> ReKeyGen(const LPPublicKey<Element> newKey,
                              const LPPrivateKey<Element> origPrivateKey) const;

  /**
   * This method implements re-encryption using the evaluation key generated by
   * ReKeyGen.
   *
   * The PRE scheme used can achieve two different levels of security, based on
   * the value supplied in the publicKey argument:
   *
   * If publicKey is nullptr, the PRE scheme is CPA-secure. If the publicKey of
   * the recipient of the re-encrypted ciphertext is supplied, then the scheme
   * is HRA- secure. Please refer to Cohen, "What about Bob? The inadequacy of
   * CPA Security for proxy re-encryption", for more information on HRA
   * security.
   *
   * The tradeoff of going for HRA is twofold: (1) performance is a little worst
   * because we add one additional encryption and homomorphic addition to the
   * result, and (2) more noise is added to the result because of the additional
   * operations - in particular, the extra encryption draws noise from a
   * distribution whose standard deviation is scaled by K, the number of digits
   * in the PRE decomposition.
   *
   * @param evalKey the evaluation key.
   * @param ciphertext the input ciphertext.
   * @param publicKey the public key of the recipient of the re-encrypted
   * ciphertext.
   * @return resulting ciphertext after the re-encryption operation.
   */
  Ciphertext<Element> ReEncrypt(
      const LPEvalKey<Element> EK, ConstCiphertext<Element> ciphertext,
      const LPPublicKey<Element> publicKey = nullptr) const;
};

/**
 * @brief Concrete class for the FHE Multiparty algorithms on BFVrns.    This
 * scheme is also referred to as the FV scheme.  A version of this multiparty
 * scheme built on the BGV scheme is seen here:
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
class LPAlgorithmMultipartyBFVrns : public LPAlgorithmMultipartyBFV<Element> {
 public:
  /**
   * Default constructor
   */
  LPAlgorithmMultipartyBFVrns() {}

  /**
   * Method for fusing the partially decrypted ciphertext.
   *
   * @param &ciphertextVec ciphertext id decrypted.
   * @param *plaintext the plaintext output.
   * @return the decoding result.
   */
  DecryptResult MultipartyDecryptFusion(
      const vector<Ciphertext<Element>>& ciphertextVec,
      NativePoly* plaintext) const;

  template <class Archive>
  void save(Archive& ar) const {
    ar(cereal::base_class<LPAlgorithmMultipartyBFV<Element>>(this));
  }

  template <class Archive>
  void load(Archive& ar) {
    ar(cereal::base_class<LPAlgorithmMultipartyBFV<Element>>(this));
  }

  LPEvalKey<Element> MultiKeySwitchGen(
      const LPPrivateKey<Element> originalPrivateKey,
      const LPPrivateKey<Element> newPrivateKey,
      const LPEvalKey<Element> ek) const;

  std::string SerializedObjectName() const { return "BFVrnsMultiparty"; }
};

/**
 * @brief Main public key encryption scheme for BFVrns implementation,
 * @tparam Element a ring element.
 */
template <class Element>
class LPPublicKeyEncryptionSchemeBFVrns
    : public LPPublicKeyEncryptionScheme<Element> {
 public:
  LPPublicKeyEncryptionSchemeBFVrns();

  bool operator==(const LPPublicKeyEncryptionScheme<Element>& sch) const {
    if (dynamic_cast<const LPPublicKeyEncryptionSchemeBFVrns<Element>*>(&sch) ==
        0)
      return false;
    return true;
  }

  void Enable(PKESchemeFeature feature);

  template <class Archive>
  void save(Archive& ar, std::uint32_t const version) const {
    ar(::cereal::base_class<LPPublicKeyEncryptionScheme<Element>>(this));
  }

  template <class Archive>
  void load(Archive& ar, std::uint32_t const version) {
    ar(::cereal::base_class<LPPublicKeyEncryptionScheme<Element>>(this));
  }

  std::string SerializedObjectName() const { return "BFVrnsScheme"; }
};

}  // namespace lbcrypto

#endif
