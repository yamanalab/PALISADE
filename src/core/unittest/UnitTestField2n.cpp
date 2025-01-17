/*
 * @file
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
This code exercises the Field2n methods of the PALISADE lattice encryption
library.
*/

#include "include/gtest/gtest.h"
#include "lattice/field2n.h"

using namespace lbcrypto;

/*---------------------------------------	TESTING METHODS OF FIELD2N
 * --------------------------------------------*/

// TEST FOR GETTER FOR FORMAT
TEST(UTField2n, get_format) {
  DEBUG_FLAG(false);

  DEBUG("Step 1");
  Field2n test(2, COEFFICIENT, true);
  DEBUG("Step 2");
  EXPECT_EQ(COEFFICIENT, test.GetFormat()) << "Failed getter" << std::endl;
}

// TEST FOR INVERSE OF FIELD ELEMENT
TEST(UTField2n, inverse) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n test(2, EVALUATION, true);
  test.at(0) = std::complex<double>(2, 1);
  test.at(1) = std::complex<double>(-4, -2);
  DEBUG("Step 2");
  Field2n inverse(2, EVALUATION, true);
  inverse.at(0) = std::complex<double>(0.4, -0.2);
  inverse.at(1) = std::complex<double>(-0.2, 0.1);
  DEBUG("Step 3");
  EXPECT_EQ(inverse, test.Inverse());
}

// TEST FOR ADDITION OPERATION
TEST(UTField2n, plus) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(2, EVALUATION, true);
  a.at(0) = std::complex<double>(2, 1);
  a.at(1) = std::complex<double>(-4, 2);
  DEBUG("Step 2");
  Field2n b(2, EVALUATION, true);
  b.at(0) = std::complex<double>(3, -0.1);
  b.at(1) = std::complex<double>(-4, 3.2);
  DEBUG("Step 3");
  Field2n c(2, EVALUATION, true);
  c.at(0) = std::complex<double>(5, 0.9);
  c.at(1) = std::complex<double>(-8, 5.2);
  EXPECT_EQ(c, a.Plus(b));
}

// TEST FOR SCALAR ADDITION
TEST(UTField2n, scalar_plus) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(2, COEFFICIENT, true);
  a.at(0) = std::complex<double>(2, 0);
  a.at(1) = std::complex<double>(-4, 0);
  DEBUG("Step 2");
  double b = 3.2;
  DEBUG("Step 3");
  Field2n c(2, COEFFICIENT, true);
  c.at(0) = std::complex<double>(5.2, 0);
  c.at(1) = std::complex<double>(-4, 0);
  EXPECT_EQ(c, a.Plus(b));
}

// TEST FOR SUBSTRACTION OPERATION
TEST(UTField2n, minus) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(2, EVALUATION, true);
  a.at(0) = std::complex<double>(2, 1);
  a.at(1) = std::complex<double>(-4, 2);
  DEBUG("Step 2");
  Field2n b(2, EVALUATION, true);
  b.at(0) = std::complex<double>(3, -0.1);
  b.at(1) = std::complex<double>(-4, 3.2);
  DEBUG("Step 3");
  Field2n c(2, EVALUATION, true);
  c.at(0) = std::complex<double>(-1, 1.1);
  c.at(1) = std::complex<double>(0, -1.2);

  Field2n d = a.Minus(b);
  for (int i = 0; i < 2; i++) {
    EXPECT_LE(fabs(d.at(i).real() - c.at(i).real()),
              fabs(c.at(i).real()) * 0.00001);
    EXPECT_LE(fabs(d.at(i).imag() - c.at(i).imag()),
              fabs(c.at(i).imag()) * 0.00001);
  }
}

// TEST FOR MULTIPLICATION OPERATION
TEST(UTField2n, times) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(2, EVALUATION, true);
  a.at(0) = std::complex<double>(4, 3);
  a.at(1) = std::complex<double>(6, -3);
  DEBUG("Step 2");
  Field2n b(2, EVALUATION, true);
  b.at(0) = std::complex<double>(4, -3);
  b.at(1) = std::complex<double>(4, -2.8);
  DEBUG("Step 3");
  Field2n c(2, EVALUATION, true);
  c.at(0) = std::complex<double>(25, 0);
  c.at(1) = std::complex<double>(15.6, -28.8);
  DEBUG("Step 4");
  Field2n d = a.Times(b);
  for (int i = 0; i < 2; i++) {
    EXPECT_LE(fabs(d.at(i).real() - c.at(i).real()),
              fabs(c.at(i).real()) * 0.00001);
    EXPECT_LE(fabs(d.at(i).imag() - c.at(i).imag()),
              fabs(c.at(i).imag()) * 0.00001);
  }
}

// TEST FOR MULTIPLICATION OPERATION WITH SWITCH FORMAT
TEST(UTField2n, times_with_switch) {
  DiscreteFourierTransform::PreComputeTable(8);
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(1, 0);
  a.at(1) = std::complex<double>(1, 0);
  a.at(2) = std::complex<double>(1, 0);
  a.at(3) = std::complex<double>(1, 0);
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(1, 0);
  b.at(1) = std::complex<double>(0, 0);
  b.at(2) = std::complex<double>(1, 0);
  b.at(3) = std::complex<double>(0, 0);
  DEBUG("Step 3");
  Field2n c(4, COEFFICIENT, true);
  c.at(0) = std::complex<double>(0, 0);
  c.at(1) = std::complex<double>(0, 0);
  c.at(2) = std::complex<double>(2, 0);
  c.at(3) = std::complex<double>(2, 0);
  DEBUG("Step 4");
  a.SwitchFormat();
  b.SwitchFormat();
  Field2n d = a.Times(b);
  d.SwitchFormat();
  for (int i = 0; i < 4; i++) {
    EXPECT_LE(fabs(d.at(i).real() - c.at(i).real()), pow(10, -12));
  }
  DiscreteFourierTransform::Reset();
}

// TEST FOR SHIFT RIGHT OPERATION
TEST(UTField2n, shift_right) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(1, 0);
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(-1, 0);
  b.at(1) = std::complex<double>(4, 0);
  b.at(2) = std::complex<double>(3, 0);
  b.at(3) = std::complex<double>(2, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.ShiftRight());
}

// TEST FOR TRANSPOSE OPERATION
TEST(UTField2n, transpose) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(1, 0);
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(4, 0);
  b.at(1) = std::complex<double>(-1, 0);
  b.at(2) = std::complex<double>(-2, 0);
  b.at(3) = std::complex<double>(-3, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.Transpose());
}

// TEST FOR TRANSPOSE OPERATION
TEST(UTField2n, transpose_eval) {
  DiscreteFourierTransform::PreComputeTable(8);
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(1, 0);
  // Convert to Evaluation format
  a.SwitchFormat();
  a = a.Transpose();
  // back to coefficient representation
  a.SwitchFormat();
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(4, 0);
  b.at(1) = std::complex<double>(-1, 0);
  b.at(2) = std::complex<double>(-2, 0);
  b.at(3) = std::complex<double>(-3, 0);
  DEBUG("Step 3");
  for (int i = 0; i < 4; i++) {
    EXPECT_LE(fabs(b.at(i).real() - a.at(i).real()),
              fabs(b.at(i).real()) * 0.0001);
  }
  DiscreteFourierTransform::Reset();
}

// TEST FOR AUTOMORPHISM OPERATION
TEST(UTField2n, automorphism) {
  DiscreteFourierTransform::PreComputeTable(8);
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(1, 0);
  a.at(1) = std::complex<double>(2, 0);
  a.at(2) = std::complex<double>(3, 0);
  a.at(3) = std::complex<double>(4, 0);
  a.SwitchFormat();
  a = a.AutomorphismTransform(3);
  a.SwitchFormat();
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(1, 0);
  b.at(1) = std::complex<double>(4, 0);
  b.at(2) = std::complex<double>(-3, 0);
  b.at(3) = std::complex<double>(2, 0);
  DEBUG("Step 3");
  for (int i = 0; i < 4; i++) {
    EXPECT_LE(fabs(b.at(i).real() - a.at(i).real()),
              fabs(b.at(i).real()) * 0.0001);
  }
  DiscreteFourierTransform::Reset();
}

// TEST FOR EXTRACT ODD OPERATION
TEST(UTField2n, extract_odd) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(1, 0);
  DEBUG("Step 2");
  Field2n b(2, COEFFICIENT, true);
  b.at(0) = std::complex<double>(3, 0);
  b.at(1) = std::complex<double>(1, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.ExtractOdd());
}

// TEST FOR EXTRACT EVEN OPERATION
TEST(UTField2n, extract_even) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(1, 0);
  DEBUG("Step 2");
  Field2n b(2, COEFFICIENT, true);
  b.at(0) = std::complex<double>(4, 0);
  b.at(1) = std::complex<double>(2, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.ExtractEven());
}

// TEST FOR PERMUTE OPERATION
TEST(UTField2n, permute) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(1, 0);
  a.at(1) = std::complex<double>(2, 0);
  a.at(2) = std::complex<double>(3, 0);
  a.at(3) = std::complex<double>(4, 0);
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(1, 0);
  b.at(1) = std::complex<double>(3, 0);
  b.at(2) = std::complex<double>(2, 0);
  b.at(3) = std::complex<double>(4, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.Permute());
}

// TEST FOR INVERSE PERMUTE OPERATION
TEST(UTField2n, inverse_permute) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, COEFFICIENT, true);
  a.at(0) = std::complex<double>(1, 0);
  a.at(1) = std::complex<double>(3, 0);
  a.at(2) = std::complex<double>(2, 0);
  a.at(3) = std::complex<double>(4, 0);
  DEBUG("Step 2");
  Field2n b(4, COEFFICIENT, true);
  b.at(0) = std::complex<double>(1, 0);
  b.at(1) = std::complex<double>(2, 0);
  b.at(2) = std::complex<double>(3, 0);
  b.at(3) = std::complex<double>(4, 0);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.InversePermute());
}

// TEST FOR SCALAR MULT OPERATION
TEST(UTField2n, scalar_mult) {
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(4, EVALUATION, true);
  a.at(0) = std::complex<double>(1, -1);
  a.at(1) = std::complex<double>(3, -2);
  a.at(2) = std::complex<double>(2, -3);
  a.at(3) = std::complex<double>(4, -4);
  DEBUG("Step 2");
  Field2n b(4, EVALUATION, true);
  b.at(0) = std::complex<double>(3, -3);
  b.at(1) = std::complex<double>(9, -6);
  b.at(2) = std::complex<double>(6, -9);
  b.at(3) = std::complex<double>(12, -12);
  DEBUG("Step 3");
  EXPECT_EQ(b, a.ScalarMult(3));
}

// TEST FOR COEFFICIENT TO EVALUATION FORMAT CHANGE
TEST(UTField2n, coefficient_evaluation) {
  DiscreteFourierTransform::PreComputeTable(16);
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n a(8, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(5, 0);
  a.at(2) = std::complex<double>(5, 0);
  a.at(3) = std::complex<double>(4.2, 0);
  a.at(4) = std::complex<double>(5, 0);
  a.at(5) = std::complex<double>(7.1, 0);
  a.at(6) = std::complex<double>(6, 0);
  a.at(7) = std::complex<double>(3, 0);
  DEBUG("Step 2");
  Field2n b(8, EVALUATION, true);
  b.at(0) = std::complex<double>(4.03087, 26.2795);
  b.at(1) = std::complex<double>(8.15172, 5.84489);
  b.at(2) = std::complex<double>(1.26249, 0.288539);
  b.at(3) = std::complex<double>(2.55492, 0.723132);
  b.at(4) = std::complex<double>(2.55492, -0.723132);
  b.at(5) = std::complex<double>(1.26249, -0.288539);
  b.at(6) = std::complex<double>(8.15172, -5.84489);
  b.at(7) = std::complex<double>(4.03087, -26.2795);
  DEBUG("Step 3");
  a.SwitchFormat();
  for (int i = 0; i < 8; i++) {
    EXPECT_LE(fabs(a.at(i).real() - b.at(i).real()),
              fabs(b.at(i).real()) * 0.0001);
    EXPECT_LE(fabs(a.at(i).imag() - b.at(i).imag()),
              fabs(b.at(i).imag()) * 0.0001);
  }
  DiscreteFourierTransform::Reset();
}

// TEST FOR EVALUATION TO COEFFICIENT FORMAT CHANGE
TEST(UTField2n, evaluation_coefficient) {
  DiscreteFourierTransform::PreComputeTable(16);
  DEBUG_FLAG(false);
  DEBUG("Step 1");
  Field2n b(8, EVALUATION, true);
  b.at(0) = std::complex<double>(4.03087, 26.2795);
  b.at(1) = std::complex<double>(8.15172, 5.84489);
  b.at(2) = std::complex<double>(1.26249, 0.288539);
  b.at(3) = std::complex<double>(2.55492, 0.723132);
  b.at(4) = std::complex<double>(2.55492, -0.723132);
  b.at(5) = std::complex<double>(1.26249, -0.288539);
  b.at(6) = std::complex<double>(8.15172, -5.84489);
  b.at(7) = std::complex<double>(4.03087, -26.2795);
  DEBUG("Step 2");
  Field2n a(8, COEFFICIENT, true);
  a.at(0) = std::complex<double>(4, 0);
  a.at(1) = std::complex<double>(5, 0);
  a.at(2) = std::complex<double>(5, 0);
  a.at(3) = std::complex<double>(4.2, 0);
  a.at(4) = std::complex<double>(5, 0);
  a.at(5) = std::complex<double>(7.1, 0);
  a.at(6) = std::complex<double>(6, 0);
  a.at(7) = std::complex<double>(3, 0);

  DEBUG("Step 3");
  b.SwitchFormat();
  for (int i = 0; i < 8; i++) {
    EXPECT_LE(fabs(a.at(i).real() - b.at(i).real()),
              fabs(a.at(i).real()) * 0.0001);
  }
  DiscreteFourierTransform::Reset();
}
