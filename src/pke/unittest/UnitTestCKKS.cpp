/*
 * @file UnitTestCKKS.cpp - Unit tests for the CKKS scheme
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

#include "include/gtest/gtest.h"
#include <iostream>
#include <vector>
#include <list>
#include <ctime>

#include "palisade.h"
#include "cryptocontext.h"
#include "cryptocontexthelper.h"
#include "cryptocontextgen.h"
#include "utils/testcasegen.h"

using namespace std;
using namespace lbcrypto;

class UTCKKS : public ::testing::Test {
 public:
  const usint m = 16;
  UTCKKS() {}
  ~UTCKKS() {}

 protected:
  void SetUp() {}

  void TearDown() {
    CryptoContextFactory<Poly>::ReleaseAllContexts();
    CryptoContextFactory<DCRTPoly>::ReleaseAllContexts();
  }
};

#define GENERATE_TEST_CASES_FUNC_BV(x, y, ORD, SCALE, NUMPRIME, RELIN, BATCH) \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN,  \
                          BATCH, BV, APPROXRESCALE)                           \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN,  \
                          BATCH, BV, EXACTRESCALE)

#define GENERATE_TEST_CASES_FUNC_GHS(x, y, ORD, SCALE, NUMPRIME, RELIN, BATCH) \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN,   \
                          BATCH, GHS, APPROXRESCALE)                           \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN,   \
                          BATCH, GHS, EXACTRESCALE)

#define GENERATE_TEST_CASES_FUNC_HYBRID(x, y, ORD, SCALE, NUMPRIME, RELIN,   \
                                        BATCH)                               \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN, \
                          BATCH, HYBRID, APPROXRESCALE)                      \
  GENERATE_CKKS_TEST_CASE(x, y, DCRTPoly, CKKS, ORD, SCALE, NUMPRIME, RELIN, \
                          BATCH, HYBRID, EXACTRESCALE)

/* *
 * ORDER: Cyclotomic order. Must be a power of 2 for CKKS.
 * NUMPRIME: Number of towers comprising the ciphertext modulus.
 * SCALE: Scaling factor bit-length.
 * 		  Should fit into a machine word, i.e., less than 64.
 * RELIN: The bit decomposition count used in BV relinearization.
 * BATCH: The length of the packed vectors to be used with CKKS.
 */
static const usint ORDER = 1024;  // 16384;
static const usint SCALE = 50;
static const usint NUMPRIME = 8;
static const usint RELIN = 20;
static const usint BATCH = 8;

/**
 * This function checks whether vectors of approximate numbers a and b are
 * equal. This is useful for CKKS tests, because numbers are approximate, so
 * results will never be exactly as expected.
 *
 * @param vectorSize The length of the two vectors.
 * @param epsilon Minimum precision to consider a and b equal.
 * 			E.g., a={0.1, 0.123} and b={0.1, 0.124} are equal for
 * epsilon = 0.01, but different for epsilon = 0.001.
 * @param failmsg Debug message to display upon failure.
 */
static void checkApproximateEquality(std::vector<std::complex<double>>& a,
                                     std::vector<std::complex<double>>& b,
                                     int vectorSize, double epsilon,
                                     const string& failmsg) {
  std::vector<std::complex<double>> allTrue(vectorSize);
  std::vector<std::complex<double>> tmp(vectorSize);
  for (int i = 0; i < vectorSize; i++) {
    allTrue[i] = 1;
    tmp[i] = abs(a[i] - b[i]) <= epsilon;
  }
  EXPECT_TRUE(tmp == allTrue) << failmsg;
}

/**
 * Tests whether addition for CKKS works properly.
 */
template <class Element>
static void UnitTest_Add_Packed(const CryptoContext<Element> cc,
                                const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  // The precision after which we consider two values equal.
  // This is necessary because CKKS works for approximate numbers.
  double eps = 0.000000001;

  // vectorOfInts1 = { 0,1,2,3,4,5,6,7 };
  std::vector<std::complex<double>> vectorOfInts1(vecSize);
  std::vector<std::complex<double>> negativeInts1(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts1[i] = i;
    negativeInts1[i] = -i;
  }
  Plaintext plaintext1 = cc->MakeCKKSPackedPlaintext(vectorOfInts1);
  Plaintext negatives1 = cc->MakeCKKSPackedPlaintext(negativeInts1);

  // vectorOfInts2 = { 7,6,5,4,3,2,1,0 };
  std::vector<std::complex<double>> vectorOfInts2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts2[i] = vecSize - i - 1;
  }
  Plaintext plaintext2 = cc->MakeCKKSPackedPlaintext(vectorOfInts2);

  // vectorOfIntsAdd = { 7,7,7,7,7,7,7,7 };
  std::vector<std::complex<double>> vectorOfIntsAdd(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfIntsAdd[i] = vecSize - 1;
  }
  Plaintext plaintextAdd = cc->MakeCKKSPackedPlaintext(vectorOfIntsAdd);

  // vectorOfIntsSub = { -7,-5,-3,-1,1,3,5,7 };
  std::vector<std::complex<double>> vectorOfIntsSub(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfIntsSub[i] = 2 * i - vecSize + 1;
  }
  Plaintext plaintextSub = cc->MakeCKKSPackedPlaintext(vectorOfIntsSub);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();

  // Encrypt plaintexts
  Ciphertext<Element> ciphertext1 = cc->Encrypt(kp.publicKey, plaintext1);
  Ciphertext<Element> ciphertext2 = cc->Encrypt(kp.publicKey, plaintext2);
  Ciphertext<Element> cResult;
  Plaintext results;

  /* Testing EvalAdd
   */
  cResult = cc->EvalAdd(ciphertext1, ciphertext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextAdd->GetLength());
  auto tmp_a = plaintextAdd->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalAdd fails");

  /* Testing operator+
   */
  cResult = ciphertext1 + ciphertext2;
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextAdd->GetLength());
  tmp_a = plaintextAdd->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator+ fails");

  /* Testing operator+=
   */
  Ciphertext<Element> caddInplace(ciphertext1);
  caddInplace += ciphertext2;
  cc->Decrypt(kp.secretKey, caddInplace, &results);
  results->SetLength(plaintextAdd->GetLength());
  tmp_a = plaintextAdd->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator+= fails");

  /* Testing EvalSub
   */
  cResult = cc->EvalSub(ciphertext1, ciphertext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextSub->GetLength());
  tmp_a = plaintextSub->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalSub fails");

  /* Testing operator-
   */
  cResult = ciphertext1 - ciphertext2;
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextSub->GetLength());
  tmp_a = plaintextSub->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator- fails");

  /* Testing operator-=
   */
  Ciphertext<Element> csubInplace(ciphertext1);
  csubInplace -= ciphertext2;
  cc->Decrypt(kp.secretKey, csubInplace, &results);
  results->SetLength(plaintextSub->GetLength());
  tmp_a = plaintextSub->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator-= fails");

  /* Testing EvalAdd ciphertext + plaintext
   */
  cResult = cc->EvalAdd(ciphertext1, plaintext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextAdd->GetLength());
  tmp_a = plaintextAdd->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalAdd Ct and Pt fails");

  /* Testing EvalSub ciphertext - plaintext
   */
  cResult = cc->EvalSub(ciphertext1, plaintext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextSub->GetLength());
  tmp_a = plaintextSub->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalSub Ct and Pt fails fails");

  /* Testing EvalNegate
   */
  cResult = cc->EvalNegate(ciphertext1);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(negatives1->GetLength());
  tmp_a = negatives1->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalNegate fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_Add_Packed, ORDER, SCALE, NUMPRIME,
                            RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_Add_Packed, ORDER, SCALE,
                             NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_Add_Packed, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

/**
 * Tests whether multiplication for CKKS works properly.
 */
template <class Element>
static void UnitTest_Mult_Packed(const CryptoContext<Element> cc,
                                 const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.000000001;

  // vectorOfInts1 = { 0,1,2,3,4,5,6,7 };
  std::vector<std::complex<double>> vectorOfInts1(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts1[i] = i;
  }
  Plaintext plaintext1 = cc->MakeCKKSPackedPlaintext(vectorOfInts1);

  // vectorOfInts2 = { 7,6,5,4,3,2,1,0 };
  std::vector<std::complex<double>> vectorOfInts2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts2[i] = vecSize - i - 1;
  }
  Plaintext plaintext2 = cc->MakeCKKSPackedPlaintext(vectorOfInts2);

  // vectorOfIntsMult = { 0,6,10,12,12,10,6,0 };
  std::vector<std::complex<double>> vectorOfIntsMult(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfIntsMult[i] = i * vecSize - i * i - i;
  }
  Plaintext plaintextMult = cc->MakeCKKSPackedPlaintext(vectorOfIntsMult);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);

  // Encrypt plaintexts
  Ciphertext<Element> ciphertext1 = cc->Encrypt(kp.publicKey, plaintext1);
  Ciphertext<Element> ciphertext2 = cc->Encrypt(kp.publicKey, plaintext2);
  Ciphertext<Element> cResult;
  Plaintext results;

  /* Testing EvalMult
   */
  cc->EvalMult(ciphertext1, plaintext1);
  cc->EvalMult(ciphertext2, plaintext2);
  cResult = cc->EvalMult(ciphertext1, ciphertext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextMult->GetLength());
  auto tmp_a = plaintextMult->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  //	stringstream buffer;
  //	buffer << "p1: " << plaintext1 << ", p2: " << plaintext2 << ", expect: "
  //<< tmp_a << " - we get: " << tmp_b << endl;
  // checkApproximateEquality(tmp_a,
  // tmp_b, vecSize, eps, failmsg + 			" EvalMult fails" +
  // buffer.str());
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalMult fails");

  /* Testing operator*
   */
  cResult = ciphertext1 * ciphertext2;
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextMult->GetLength());
  tmp_a = plaintextMult->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator* fails");

  /* Testing operator*=
   */
  Ciphertext<Element> cmultInplace(ciphertext1);
  cmultInplace *= ciphertext2;
  cc->Decrypt(kp.secretKey, cmultInplace, &results);
  results->SetLength(plaintextMult->GetLength());
  tmp_a = plaintextMult->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " operator*= fails");

  /* Testing EvalMult ciphertext * plaintext
   */
  cResult = cc->EvalMult(ciphertext1, plaintext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextMult->GetLength());
  tmp_a = plaintextMult->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalMult Ct and Pt fails");

  /* Testing EvalMultNoRelin ciphertext * ciphertext
   */
  cResult = cc->EvalMultNoRelin(ciphertext1, ciphertext2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextMult->GetLength());
  tmp_a = plaintextMult->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalMultNoRelin Ct and Ct fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_Mult_Packed, ORDER, SCALE,
                            NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_Mult_Packed, ORDER, SCALE,
                             NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_Mult_Packed, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

/**
 * Tests the correct operation of the following:
 * - addition/subtraction of constant to ciphertext of depth > 1
 * - addition/subtraction of plaintext to ciphertext of depth > 1
 * - encoding of plaintext at depth > 1
 * - automatic scaling up of plaintexts to a depth that matches that of a
 * ciphertext
 */
template <class Element>
static void UnitTest_ScaleFactorAdjustments(const CryptoContext<Element> cc,
                                            const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.00000001;

  // vectorOfInts1 = { 0,1,2,3,4,5,6,7 };
  std::vector<std::complex<double>> vectorOfInts1(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts1[i] = i;
  }
  Plaintext plaintext1 = cc->MakeCKKSPackedPlaintext(vectorOfInts1);

  // constantInts = { 11,11,11,11,11,11,11,11 };
  std::vector<std::complex<double>> constantInts(vecSize);
  for (int i = 0; i < vecSize; i++) {
    constantInts[i] = 11;
  }
  Plaintext plaintextConst = cc->MakeCKKSPackedPlaintext(constantInts);
  Plaintext plaintextConstDeep = cc->MakeCKKSPackedPlaintext(constantInts, 3);

  // constantInts2 = { -11,-11,-11,-11,-11,-11,-11,-11 };
  std::vector<std::complex<double>> constantInts2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    constantInts2[i] = -constantInts[i];
  }
  Plaintext plaintextConst2 = cc->MakeCKKSPackedPlaintext(constantInts2);
  Plaintext plaintextConst2Deep = cc->MakeCKKSPackedPlaintext(constantInts2, 3);

  // vectorOfInts2 = { 7,6,5,4,3,2,1,0 };
  std::vector<std::complex<double>> vectorOfInts2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts2[i] = vecSize - i - 1;
  }
  Plaintext plaintext2 = cc->MakeCKKSPackedPlaintext(vectorOfInts2);

  // vectorOfInts3 = { 0,-1,-2,-3,-4,-5,-6,-7 };
  std::vector<std::complex<double>> vectorOfInts3(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts3[i] = -i;
  }
  Plaintext plaintext3 = cc->MakeCKKSPackedPlaintext(vectorOfInts3);

  // vectorOfIntsMult = { 0,6,10,12,12,10,6,0 };
  std::vector<std::complex<double>> vectorOfIntsMult(vecSize);
  // vectorOfIntsMult2 = { 0,6,20,36,48,50,36,0 };
  std::vector<std::complex<double>> vectorOfIntsMult2(vecSize);
  // vectorOfIntsAddAfterMult = { 10,16,20,22,22,20,16,10 };
  std::vector<std::complex<double>> vectorOfIntsAddAfterMult(vecSize);
  // vectorOfIntsSubAfterMult = { -10,-4,0,2,2,0,-4,-10 };
  std::vector<std::complex<double>> vectorOfIntsSubAfterMult(vecSize);
  // vectorOfIntsAddAfterMult2 = { 11,17,31,47,59,61,47,11 };
  std::vector<std::complex<double>> vectorOfIntsAddAfterMult2(vecSize);
  // vectorOfIntsSubAfterMult2 = { -11,-5,9,25,37,39,25,-11 };
  std::vector<std::complex<double>> vectorOfIntsSubAfterMult2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfIntsMult[i] = i * vecSize - i * i - i;
    vectorOfIntsAddAfterMult[i] = vectorOfIntsMult[i] + complex<double>(10, 0);
    vectorOfIntsSubAfterMult[i] = vectorOfIntsMult[i] - complex<double>(10, 0);
    vectorOfIntsMult2[i] = vectorOfIntsMult[i] * vectorOfInts1[i];
    vectorOfIntsAddAfterMult2[i] =
        vectorOfIntsMult2[i] + constantInts[i];  // complex<double>({11,0});
    vectorOfIntsSubAfterMult2[i] =
        vectorOfIntsMult2[i] - constantInts[i];  // complex<double>({11,0});
  }
  Plaintext plaintextMult = cc->MakeCKKSPackedPlaintext(vectorOfIntsMult);
  Plaintext plaintexAddAfterMult =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsAddAfterMult);
  Plaintext plaintexSubAfterMult =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsSubAfterMult);
  Plaintext plaintexttMult2 = cc->MakeCKKSPackedPlaintext(vectorOfIntsMult2);
  Plaintext plaintexAddAfterMult2 =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsAddAfterMult2);
  Plaintext plaintexSubAfterMult2 =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsSubAfterMult2);
  Plaintext plaintex2AddAfterMult2 =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsSubAfterMult2);
  Plaintext plaintex2SubAfterMult2 =
      cc->MakeCKKSPackedPlaintext(vectorOfIntsAddAfterMult2);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);

  // Encrypt plaintexts
  Ciphertext<Element> ciphertext1 = cc->Encrypt(kp.publicKey, plaintext1);
  Ciphertext<Element> ciphertext2 = cc->Encrypt(kp.publicKey, plaintext2);
  Ciphertext<Element> cResult;
  Plaintext results;

  auto cMult = cc->EvalMult(ciphertext1, ciphertext2);
  auto cAddAfterMult = cc->EvalAdd(cMult, 10);
  auto cSubAfterMult = cc->EvalSub(cMult, 10);
  auto cMult2 = cc->EvalMult(ciphertext1, cMult);
  auto cAddAfterMult2 = cc->EvalAdd(cMult2, 11);
  auto cSubAfterMult2 = cc->EvalSub(cMult2, 11);
  auto c2AddAfterMult2 = cc->EvalAdd(cMult2, -11);
  auto c2SubAfterMult2 = cc->EvalSub(cMult2, -11);
  auto cAddPtAfterMult2 = cc->EvalAdd(cMult2, plaintextConst);
  auto cSubPtAfterMult2 = cc->EvalSub(cMult2, plaintextConst);
  auto cAddPt2AfterMult2 = cc->EvalAdd(cMult2, plaintextConst2);
  auto cSubPt2AfterMult2 = cc->EvalSub(cMult2, plaintextConst2);
  auto cDeepAdd = cc->EvalAdd(cMult2, plaintextConstDeep);
  auto cDeepSub = cc->EvalSub(cMult2, plaintextConstDeep);
  auto c2DeepAdd = cc->EvalAdd(cMult2, plaintextConst2Deep);
  auto c2DeepSub = cc->EvalSub(cMult2, plaintextConst2Deep);

  cc->Decrypt(kp.secretKey, cAddAfterMult, &results);
  results->SetLength(plaintexAddAfterMult->GetLength());
  auto tmp_a = plaintexAddAfterMult->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " add after 1 multiplication fails");

  cc->Decrypt(kp.secretKey, cSubAfterMult, &results);
  results->SetLength(plaintexSubAfterMult->GetLength());
  tmp_a = plaintexSubAfterMult->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " subtract after 1 multiplication fails");

  cc->Decrypt(kp.secretKey, cAddAfterMult2, &results);
  results->SetLength(plaintexAddAfterMult2->GetLength());
  tmp_a = plaintexAddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " add after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, cSubAfterMult2, &results);
  results->SetLength(plaintexSubAfterMult2->GetLength());
  tmp_a = plaintexSubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " subtract after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, c2AddAfterMult2, &results);
  results->SetLength(plaintex2AddAfterMult2->GetLength());
  tmp_a = plaintex2AddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  //	stringstream buffer;
  //	buffer << " we expect: " << tmp_a << endl;
  //	buffer << " we get: " << tmp_b << endl;
  //	checkApproximateEquality(tmp_a, tmp_b, vecSize, eps, failmsg +
  //			" add (negative) after 2 multiplications fails: " +
  //			buffer.str());
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " add (negative) after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, c2SubAfterMult2, &results);
  results->SetLength(plaintex2SubAfterMult2->GetLength());
  tmp_a = plaintex2SubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  //	buffer.str("");
  //	buffer << " we expect: " << tmp_a << endl;
  //	buffer << " we get: " << tmp_b << endl;
  //	checkApproximateEquality(tmp_a, tmp_b, vecSize, eps, failmsg +
  //			" subtract (negative) after 2 multiplications fails: " +
  //			buffer.str());
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " subtract (negative) after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, cAddPtAfterMult2, &results);
  results->SetLength(plaintexAddAfterMult2->GetLength());
  tmp_a = plaintexAddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg +
                               " add plaintext (auto scale factor matching) "
                               "after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, cSubPtAfterMult2, &results);
  results->SetLength(plaintexSubAfterMult2->GetLength());
  tmp_a = plaintexSubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg +
                               " subtract plaintext (auto scale factor "
                               "matching) after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, cAddPt2AfterMult2, &results);
  results->SetLength(plaintex2AddAfterMult2->GetLength());
  tmp_a = plaintex2AddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg +
                               " add negative plaintext (auto scale factor "
                               "matching) after 2 multiplications fails");

  cc->Decrypt(kp.secretKey, cSubPt2AfterMult2, &results);
  results->SetLength(plaintex2SubAfterMult2->GetLength());
  tmp_a = plaintex2SubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg +
          " subtract negative plaintext (auto scale factor matching) after 2 "
          "multiplications fails");

  cc->Decrypt(kp.secretKey, cDeepAdd, &results);
  results->SetLength(plaintexAddAfterMult2->GetLength());
  tmp_a = plaintexAddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " add with deep plaintext fails");

  cc->Decrypt(kp.secretKey, cDeepSub, &results);
  results->SetLength(plaintexSubAfterMult2->GetLength());
  tmp_a = plaintexSubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " subtract with deep plaintext fails");

  cc->Decrypt(kp.secretKey, c2DeepAdd, &results);
  results->SetLength(plaintex2AddAfterMult2->GetLength());
  tmp_a = plaintex2AddAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " add with deep negative plaintext fails");

  cc->Decrypt(kp.secretKey, c2DeepSub, &results);
  results->SetLength(plaintex2SubAfterMult2->GetLength());
  tmp_a = plaintex2SubAfterMult2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " subtract with deep negative plaintext fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_ScaleFactorAdjustments, ORDER,
                            SCALE, NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_ScaleFactorAdjustments, ORDER,
                             SCALE, NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_ScaleFactorAdjustments, ORDER,
                                SCALE, NUMPRIME, RELIN, BATCH)

template <typename Element>
static void UnitTest_AutoLevelReduce(const CryptoContext<Element> cc,
                                     const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.000001;

  // vectorOfInts1 = { 0,1,2,3,4,5,6,7 };
  std::vector<std::complex<double>> vectorOfInts1(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts1[i] = i;
  }
  Plaintext plaintext1 = cc->MakeCKKSPackedPlaintext(vectorOfInts1);

  // vectorOfInts2 = { 7,6,5,4,3,2,1,0 };
  std::vector<std::complex<double>> vectorOfInts2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts2[i] = vecSize - i - 1;
  }
  Plaintext plaintext2 = cc->MakeCKKSPackedPlaintext(vectorOfInts2);

  std::vector<std::complex<double>> pCtMult(vecSize);
  std::vector<std::complex<double>> pCtMult3(vecSize);
  std::vector<std::complex<double>> pCt3(vecSize);
  std::vector<std::complex<double>> pCt3_b(vecSize);
  std::vector<std::complex<double>> pCt4(vecSize);
  std::vector<std::complex<double>> pCt5(vecSize);
  std::vector<std::complex<double>> pCt6(vecSize);
  std::vector<std::complex<double>> pCt7(vecSize);
  std::vector<std::complex<double>> pCt_5(vecSize);
  std::vector<std::complex<double>> pCt_6(vecSize);
  std::vector<std::complex<double>> pCt_7(vecSize);
  std::vector<std::complex<double>> pCt8(vecSize);
  std::vector<std::complex<double>> pCt9(vecSize);
  std::vector<std::complex<double>> pCt10(vecSize);
  std::vector<std::complex<double>> pCt11(vecSize);
  std::vector<std::complex<double>> pCt12(vecSize);
  std::vector<std::complex<double>> pCt13(vecSize);
  std::vector<std::complex<double>> pCt14(vecSize);
  for (int i = 0; i < vecSize; i++) {
    pCtMult[i] = vectorOfInts1[i] * vectorOfInts2[i];
    pCt3[i] = pCtMult[i] + vectorOfInts1[i];
    pCt4[i] = pCtMult[i] - vectorOfInts1[i];
    pCt5[i] = pCtMult[i] * vectorOfInts1[i];
    pCt6[i] = vectorOfInts1[i] + pCtMult[i];
    pCt7[i] = vectorOfInts1[i] - pCtMult[i];
    auto tmp = (vectorOfInts1[i] * vectorOfInts1[i] +
                vectorOfInts1[i] * vectorOfInts1[i]) *
               vectorOfInts1[i];
    pCt_5[i] = tmp + vectorOfInts2[i];
    pCt_6[i] = tmp - vectorOfInts2[i];
    pCt_7[i] = tmp * vectorOfInts2[i];
    pCt8[i] = vectorOfInts1[i] * pCtMult[i];
    pCtMult3[i] = pCtMult[i] * vectorOfInts1[i] * vectorOfInts1[i];
    pCt9[i] = pCtMult3[i] + vectorOfInts1[i];
    pCt10[i] = pCtMult3[i] - vectorOfInts1[i];
    pCt11[i] = pCtMult3[i] * vectorOfInts1[i];
    pCt12[i] = vectorOfInts1[i] + pCtMult3[i];
    pCt13[i] = vectorOfInts1[i] - pCtMult3[i];
    pCt14[i] = vectorOfInts1[i] * pCtMult3[i];
  }
  Plaintext plaintextCt3 = cc->MakeCKKSPackedPlaintext(pCt3);
  Plaintext plaintextCt4 = cc->MakeCKKSPackedPlaintext(pCt4);
  Plaintext plaintextCt5 = cc->MakeCKKSPackedPlaintext(pCt5);
  Plaintext plaintextCt6 = cc->MakeCKKSPackedPlaintext(pCt6);
  Plaintext plaintextCt7 = cc->MakeCKKSPackedPlaintext(pCt7);
  Plaintext plaintextCt_5 = cc->MakeCKKSPackedPlaintext(pCt_5);
  Plaintext plaintextCt_6 = cc->MakeCKKSPackedPlaintext(pCt_6);
  Plaintext plaintextCt_7 = cc->MakeCKKSPackedPlaintext(pCt_7);
  Plaintext plaintextCt8 = cc->MakeCKKSPackedPlaintext(pCt8);
  Plaintext plaintextCt9 = cc->MakeCKKSPackedPlaintext(pCt9);
  Plaintext plaintextCt10 = cc->MakeCKKSPackedPlaintext(pCt10);
  Plaintext plaintextCt11 = cc->MakeCKKSPackedPlaintext(pCt11);
  Plaintext plaintextCt12 = cc->MakeCKKSPackedPlaintext(pCt12);
  Plaintext plaintextCt13 = cc->MakeCKKSPackedPlaintext(pCt13);
  Plaintext plaintextCt14 = cc->MakeCKKSPackedPlaintext(pCt14);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);

  // Encrypt plaintexts
  Ciphertext<Element> ct = cc->Encrypt(kp.publicKey, plaintext1);
  Ciphertext<Element> ct2 = cc->Encrypt(kp.publicKey, plaintext2);
  Ciphertext<Element> cResult;
  Plaintext results;

  auto ctMul = cc->EvalMult(ct, ct2);
  auto ctRed = cc->ModReduce(ctMul);

  auto ct3 = cc->EvalAdd(ctRed, ct);  // Addition with tower diff = 1
  cc->Decrypt(kp.secretKey, ct3, &results);
  results->SetLength(plaintextCt3->GetLength());
  auto tmp_a = plaintextCt3->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " addition with tower diff = 1 fails");

  auto ct4 = cc->EvalSub(ctRed, ct);  // Subtraction with tower diff = 1
  cc->Decrypt(kp.secretKey, ct4, &results);
  results->SetLength(plaintextCt4->GetLength());
  tmp_a = plaintextCt4->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " subtraction with tower diff = 1 fails");

  auto ct5 = cc->EvalMult(ctRed, ct);  // Multiplication with tower diff = 1
  cc->Decrypt(kp.secretKey, ct5, &results);
  results->SetLength(plaintextCt5->GetLength());
  tmp_a = plaintextCt5->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " multiplication with tower diff = 1 fails");

  auto ct6 =
      cc->EvalAdd(ct, ctRed);  // Addition with tower diff = 1 (inputs reversed)
  cc->Decrypt(kp.secretKey, ct6, &results);
  results->SetLength(plaintextCt6->GetLength());
  tmp_a = plaintextCt6->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " addition (reverse) with tower diff = 1 fails");

  auto ct7 = cc->EvalSub(
      ct, ctRed);  // Subtraction with tower diff = 1 (inputs reversed)
  cc->Decrypt(kp.secretKey, ct7, &results);
  results->SetLength(plaintextCt7->GetLength());
  tmp_a = plaintextCt7->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " subtraction (reverse) with tower diff = 1 fails");

  auto ct8 = cc->EvalMult(
      ct, ctRed);  // Multiplication with tower diff = 1 (inputs reversed)
  cc->Decrypt(kp.secretKey, ct8, &results);
  results->SetLength(plaintextCt8->GetLength());
  tmp_a = plaintextCt8->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " multiplication (reverse) with tower diff = 1 fails");

  auto ctMul2 = cc->EvalMult(ctRed, ct);
  auto ctRed2 = cc->ModReduce(ctMul2);
  auto ctMul3 = cc->EvalMult(ctRed2, ct);
  auto ctRed3 = cc->ModReduce(ctMul3);

  auto ct9 =
      cc->EvalAdd(ctRed3, ct);  // Addition with more than 1 level difference
  cc->Decrypt(kp.secretKey, ct9, &results);
  results->SetLength(plaintextCt9->GetLength());
  tmp_a = plaintextCt9->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " addition with tower diff > 1 fails");

  auto ct10 =
      cc->EvalSub(ctRed3, ct);  // Subtraction with more than 1 level difference
  cc->Decrypt(kp.secretKey, ct10, &results);
  results->SetLength(plaintextCt10->GetLength());
  tmp_a = plaintextCt10->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " subtraction with tower diff > 1 fails");

  auto ct11 = cc->EvalMult(
      ctRed3, ct);  // Multiplication with more than 1 level difference
  cc->Decrypt(kp.secretKey, ct11, &results);
  results->SetLength(plaintextCt11->GetLength());
  tmp_a = plaintextCt11->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " multiplication with tower diff > 1 fails");

  auto ct12 = cc->EvalAdd(
      ct,
      ctRed3);  // Addition with more than 1 level difference (inputs reversed)
  cc->Decrypt(kp.secretKey, ct12, &results);
  results->SetLength(plaintextCt12->GetLength());
  tmp_a = plaintextCt12->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " addition (reverse) with tower diff > 1 fails");

  auto ct13 = cc->EvalSub(ct, ctRed3);  // Subtraction with more than 1 level
                                        // difference (inputs reversed)
  cc->Decrypt(kp.secretKey, ct13, &results);
  results->SetLength(plaintextCt13->GetLength());
  tmp_a = plaintextCt13->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " subtraction (reverse) with tower diff > 1 fails");

  auto ct14 = cc->EvalMult(ct, ctRed3);  // Multiplication with more than 1
                                         // level difference (inputs reversed)
  cc->Decrypt(kp.secretKey, ct14, &results);
  results->SetLength(plaintextCt14->GetLength());
  tmp_a = plaintextCt14->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " multiplication (reverse) with tower diff > 1 fails");

  // This scenario tests for operations on
  // ciphertext and plaintext that differ on
  // both scaling factor and number of towers.
  auto ct_1 = cc->EvalMult(ct, plaintext1);
  auto ct_2 = cc->EvalAdd(ct_1, ct_1);
  auto ct_3 = cc->ModReduce(ct_2);
  auto ct_4 = cc->EvalMult(ct_3, plaintext1);
  auto ct_5 = cc->EvalAdd(
      ct_4, plaintext2);  // Addition with plaintext and tower diff = 1
  auto ct_6 = cc->EvalSub(
      ct_4, plaintext2);  // Subtraction with plaintext and tower diff = 1
  auto ct_7 = cc->EvalMult(
      ct_4, plaintext2);  // Multiplication with plaintext and tower diff = 1
  cc->Decrypt(kp.secretKey, ct_5, &results);
  results->SetLength(plaintextCt_5->GetLength());
  tmp_a = plaintextCt_5->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " addition with plaintext and tower diff = 1 fails");

  cc->Decrypt(kp.secretKey, ct_6, &results);
  results->SetLength(plaintextCt_6->GetLength());
  tmp_a = plaintextCt_6->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " subtraction with plaintext and tower diff = 1 fails");

  cc->Decrypt(kp.secretKey, ct_7, &results);
  results->SetLength(plaintextCt_7->GetLength());
  tmp_a = plaintextCt_7->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " multiplication with plaintext and tower diff = 1 fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_AutoLevelReduce, ORDER, SCALE,
                            NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_AutoLevelReduce, ORDER, SCALE,
                             NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_AutoLevelReduce, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

/**
 * Tests whether EvalAtIndex for CKKS works properly.
 */
template <class Element>
static void UnitTest_EvalAtIndex(const CryptoContext<Element> cc,
                                 const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.000000001;

  // vectorOfInts1 = { 1,2,3,4,5,6,7,8 };
  std::vector<std::complex<double>> vectorOfInts1(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vectorOfInts1[i] = i + 1;
  }
  Plaintext plaintext1 = cc->MakeCKKSPackedPlaintext(vectorOfInts1);

  // vOnes = { 1,1,1,1,1,1,1,1 };
  std::vector<std::complex<double>> vOnes(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vOnes[i] = 1;
  }
  Plaintext pOnes = cc->MakeCKKSPackedPlaintext(vOnes);

  // vIntsRightShift2 = { 0,0,1,2,3,4,5,6 };
  std::vector<std::complex<double>> vIntsRightShift2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vIntsRightShift2[i] = (i >= 2) ? vectorOfInts1[i - 2] : 0;
  }
  Plaintext plaintextRight2 = cc->MakeCKKSPackedPlaintext(vIntsRightShift2);

  // vIntsLeftShift2 = { 3,4,5,6,7,8,0,0 };
  std::vector<std::complex<double>> vIntsLeftShift2(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vIntsLeftShift2[i] = (i < vecSize - 2) ? vectorOfInts1[i + 2] : 0;
  }
  Plaintext plaintextLeft2 = cc->MakeCKKSPackedPlaintext(vIntsLeftShift2);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);
  // Generate rotation keys for offsets +2 (left shift) and -2 (right shift)
  cc->EvalAtIndexKeyGen(kp.secretKey, {2, -2});

  // Encrypt plaintexts
  Ciphertext<Element> ciphertext1 = cc->Encrypt(kp.publicKey, plaintext1);
  Ciphertext<Element> cOnes = cc->Encrypt(kp.publicKey, pOnes);
  Ciphertext<Element> cResult;
  Plaintext results;

  /* First, do one multiplication and apply the rotation to the result.
   * This helps hide the rotation noise and get the correct result without
   * using a smaller relinWindow in BV (when creating the crypto context cc).
   */
  ciphertext1 *= cOnes;

  /* Testing EvalAtIndex +2
   */
  cResult = cc->EvalAtIndex(ciphertext1, 2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextLeft2->GetLength());
  auto tmp_a = plaintextLeft2->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalAtIndex(+2) fails");

  /* Testing EvalAtIndex -2
   */
  cResult = cc->EvalAtIndex(ciphertext1, -2);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(plaintextRight2->GetLength());
  tmp_a = plaintextRight2->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalAtIndex(-2) fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_EvalAtIndex, ORDER, SCALE,
                            NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_EvalAtIndex, ORDER, SCALE,
                             NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_EvalAtIndex, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

/**
 * Tests whether EvalMerge for CKKS works properly.
 */
template <class Element>
static void UnitTest_EvalMerge(const CryptoContext<Element> cc,
                               const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.000000001;

  // v* = { i,0,0,0,0,0,0,0 };
  std::vector<std::complex<double>> vOne(vecSize);
  std::vector<std::complex<double>> vTwo(vecSize);
  std::vector<std::complex<double>> vThree(vecSize);
  std::vector<std::complex<double>> vFour(vecSize);
  std::vector<std::complex<double>> vFive(vecSize);
  std::vector<std::complex<double>> vSix(vecSize);
  std::vector<std::complex<double>> vSeven(vecSize);
  std::vector<std::complex<double>> vEight(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vOne[i] = (i == 0) ? 1 : 0;
    vTwo[i] = (i == 0) ? 2 : 0;
    vThree[i] = (i == 0) ? 3 : 0;
    vFour[i] = (i == 0) ? 4 : 0;
    vFive[i] = (i == 0) ? 5 : 0;
    vSix[i] = (i == 0) ? 6 : 0;
    vSeven[i] = (i == 0) ? 7 : 0;
    vEight[i] = (i == 0) ? 8 : 0;
  }
  Plaintext pOne = cc->MakeCKKSPackedPlaintext(vOne);
  Plaintext pTwo = cc->MakeCKKSPackedPlaintext(vTwo);
  Plaintext pThree = cc->MakeCKKSPackedPlaintext(vThree);
  Plaintext pFour = cc->MakeCKKSPackedPlaintext(vFour);
  Plaintext pFive = cc->MakeCKKSPackedPlaintext(vFive);
  Plaintext pSix = cc->MakeCKKSPackedPlaintext(vSix);
  Plaintext pSeven = cc->MakeCKKSPackedPlaintext(vSeven);
  Plaintext pEight = cc->MakeCKKSPackedPlaintext(vEight);

  // vMerged = { 1,2,3,4,5,6,7,8 };
  std::vector<std::complex<double>> vMerged(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vMerged[i] = i + 1;
  }
  Plaintext pMerged = cc->MakeCKKSPackedPlaintext(vMerged);

  std::vector<std::complex<double>> vOnes(vecSize);
  for (int i = 0; i < vecSize; i++) {
    vOnes[i] = 1;
  }
  Plaintext pOnes = cc->MakeCKKSPackedPlaintext(vOnes);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);
  // Generate rotation keys for all right rotations 1 to 8.
  vector<int32_t> indexList = {-1, -2, -3, -4, -5, -6, -7, -8};
  cc->EvalAtIndexKeyGen(kp.secretKey, indexList);

  // Encrypt plaintexts
  Ciphertext<Element> cOnes = cc->Encrypt(kp.publicKey, pOnes);
  std::vector<Ciphertext<Element>> ciphertexts;

  // Here, we perform the same trick (mult with one) as in
  // UnitTest_EvalAtiIndex.
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pOne) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pTwo) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pThree) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pFour) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pFive) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pSix) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pSeven) * cOnes);
  ciphertexts.push_back(cc->Encrypt(kp.publicKey, pEight) * cOnes);
  Plaintext results;

  /* Testing EvalMerge
   */
  auto cResult = cc->EvalMerge(ciphertexts);
  cc->Decrypt(kp.secretKey, cResult, &results);
  results->SetLength(pMerged->GetLength());
  auto tmp_a = pMerged->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalMerge fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_EvalMerge, ORDER, SCALE, NUMPRIME,
                            RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_EvalMerge, ORDER, SCALE, NUMPRIME,
                             RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_EvalMerge, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

/**
 * Tests whether EvalLinearWSum for CKKS works properly.
 */
template <class Element>
static void UnitTest_EvalLinearWSum(const CryptoContext<Element> cc,
                                    const string& failmsg) {
  int vecSize = 8;

  const shared_ptr<LPCryptoParametersCKKS<DCRTPoly>> cryptoParams =
      std::dynamic_pointer_cast<LPCryptoParametersCKKS<DCRTPoly>>(
          cc->GetCryptoParameters());

  double eps = 0.0000000001;

  vector<double> weights(3);
  for (int i = 0; i < 3; i++) {
    weights[i] = i;
  }
  vector<complex<double>> in1(vecSize);
  vector<complex<double>> in2(vecSize);
  vector<complex<double>> in3(vecSize);
  vector<complex<double>> out(vecSize);
  for (int i = 0; i < vecSize; i++) {
    in1[i] = 3;
    in2[i] = 2;
    in3[i] = 1;
    out[i] = weights[0] * in1[i] + weights[1] * in2[i] + weights[2] * in3[i];
  }
  Plaintext pIn1 = cc->MakeCKKSPackedPlaintext(in1);
  Plaintext pIn2 = cc->MakeCKKSPackedPlaintext(in2);
  Plaintext pIn3 = cc->MakeCKKSPackedPlaintext(in3);
  Plaintext pOut = cc->MakeCKKSPackedPlaintext(out);

  // Generate encryption keys
  LPKeyPair<Element> kp = cc->KeyGen();
  // Generate multiplication keys
  cc->EvalMultKeyGen(kp.secretKey);

  // Encrypt plaintexts
  Ciphertext<Element> cIn1 = cc->Encrypt(kp.publicKey, pIn1);
  Ciphertext<Element> cIn2 = cc->Encrypt(kp.publicKey, pIn2);
  Ciphertext<Element> cIn3 = cc->Encrypt(kp.publicKey, pIn3);
  vector<Ciphertext<Element>> ciphertexts(3);

  ciphertexts[0] = cIn1;
  ciphertexts[1] = cIn2;
  ciphertexts[2] = cIn3;

  Plaintext results;

  auto cResult = cc->EvalLinearWSum(ciphertexts, weights);
  cc->Decrypt(kp.secretKey, cResult, &results);

  results->SetLength(pOut->GetLength());
  auto tmp_a = pOut->GetCKKSPackedValue();
  auto tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalLinearWSum fails");

  auto cResult2 = cc->EvalLinearWSumMutable(ciphertexts, weights);
  cc->Decrypt(kp.secretKey, cResult2, &results);

  results->SetLength(pOut->GetLength());
  tmp_a = pOut->GetCKKSPackedValue();
  tmp_b = results->GetCKKSPackedValue();
  checkApproximateEquality(tmp_a, tmp_b, vecSize, eps,
                           failmsg + " EvalLinearWSumMutable fails");
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_EvalLinearWSum, ORDER, SCALE,
                            NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_GHS(UTCKKS, UnitTest_EvalLinearWSum, ORDER, SCALE,
                             NUMPRIME, RELIN, BATCH)
GENERATE_TEST_CASES_FUNC_HYBRID(UTCKKS, UnitTest_EvalLinearWSum, ORDER, SCALE,
                                NUMPRIME, RELIN, BATCH)

template <typename Element>
static void UnitTest_ReEncryption(const CryptoContext<Element> cc,
                                  const string& failmsg) {
  size_t vecSize = 128;
  double eps = 0.01;

  auto ptm = 10;

  std::vector<std::complex<double>> intvec;
  for (size_t ii = 0; ii < vecSize; ii++) {
    intvec.push_back((rand() % (ptm / 2)) * (rand() % 2 ? 1 : -1));
  }
  Plaintext plaintextInt = cc->MakeCKKSPackedPlaintext(intvec);

  LPKeyPair<Element> kp = cc->KeyGen();
  EXPECT_EQ(kp.good(), true)
      << failmsg << " key generation for scalar encrypt/decrypt failed";

  LPKeyPair<Element> newKp = cc->KeyGen();
  EXPECT_EQ(newKp.good(), true)
      << failmsg << " second key generation for scalar encrypt/decrypt failed";

  // This generates the keys which are used to perform the key switching.
  LPEvalKey<Element> evalKey;
  evalKey = cc->ReKeyGen(newKp.publicKey, kp.secretKey);

  Ciphertext<Element> ciphertext = cc->Encrypt(kp.publicKey, plaintextInt);
  Plaintext plaintextIntNew;
  Ciphertext<Element> reCiphertext = cc->ReEncrypt(evalKey, ciphertext);
  cc->Decrypt(newKp.secretKey, reCiphertext, &plaintextIntNew);
  plaintextIntNew->SetLength(plaintextInt->GetLength());
  auto tmp_a = plaintextIntNew->GetCKKSPackedValue();
  auto tmp_b = plaintextInt->GetCKKSPackedValue();
  stringstream buffer;
  buffer << tmp_b << " - we get: " << tmp_a << endl;
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " ReEncrypt integer plaintext " + buffer.str());

  stringstream buffer2;
  Ciphertext<Element> ciphertext2 = cc->Encrypt(kp.publicKey, plaintextInt);
  Plaintext plaintextIntNew2;
  Ciphertext<Element> reCiphertext2 =
      cc->ReEncrypt(evalKey, ciphertext2, kp.publicKey);
  cc->Decrypt(newKp.secretKey, reCiphertext2, &plaintextIntNew2);
  plaintextIntNew2->SetLength(plaintextInt->GetLength());
  tmp_a = plaintextIntNew2->GetCKKSPackedValue();
  tmp_b = plaintextInt->GetCKKSPackedValue();
  buffer2 << tmp_b << " - we get: " << tmp_a << endl;
  checkApproximateEquality(
      tmp_a, tmp_b, vecSize, eps,
      failmsg + " HRA-secure ReEncrypt integer plaintext " + buffer2.str());
}

GENERATE_TEST_CASES_FUNC_BV(UTCKKS, UnitTest_ReEncryption, ORDER, SCALE,
                            NUMPRIME, RELIN, BATCH)
