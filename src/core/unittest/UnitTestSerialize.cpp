/*
 * @file UnitTestSerialize.cpp
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
  This code exercises serialization in CORE for the PALISADE lattice encryption
  library.
 */

#include "include/gtest/gtest.h"
#include <iostream>

#include "math/backend.h"
#include "lattice/backend.h"
#include "math/nbtheory.h"
#include "math/distrgen.h"
#include "math/matrix.h"
#include "utils/inttypes.h"
#include "utils/utilities.h"
#include "utils/parmfactory.h"
#include "lattice/elemparamfactory.h"

#include "utils/serialize-binary.h"
#include "utils/serialize-json.h"

using namespace std;
using namespace lbcrypto;

template <typename T>
void bigint(const string& msg) {
  T small(7);
  T medium(uint64_t(1) << 27 | uint64_t(1) << 22);
  T larger(uint64_t(1) << 40 | uint64_t(1) << 22);

  auto sfunc = [&](T& val, string siz) {
    stringstream s;
    T deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(val, deser) << msg << " " << siz
                          << " integer json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);

    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(val, deser) << msg << " " << siz
                          << " integer binary ser/deser fails";
  };

  sfunc(small, "small");
  sfunc(medium, "medium");
  sfunc(larger, "larger");
}

TEST(UTSer, bigint) { RUN_ALL_BACKENDS_INT(bigint, "bigint") }

template <typename T>
void hugeint(const string& msg) {
  T yooge("371828316732191777888912");

  auto sfunc = [&](T& val, string siz) {
    stringstream s;
    T deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(val, deser) << msg << " " << siz
                          << " integer json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(val, deser) << msg << " " << siz
                          << " integer binary ser/deser fails";
  };

  sfunc(yooge, "Huge");
}

TEST(UTSer, hugeint) { RUN_BIG_BACKENDS_INT(hugeint, "hugeint") }

template <typename V>
void vector_of_bigint(const string& msg) {
  DEBUG_FLAG(false);
  const int vecsize = 100;

  DEBUG("step 0");
  typename V::Integer mod((uint64_t)1 << 40);

  DEBUG("step 1");
  V testvec(vecsize, mod);
  DEBUG("step 2");
  DiscreteUniformGeneratorImpl<V> dug;
  DEBUG("step 3");
  dug.SetModulus(mod);
  typename V::Integer ranval;

  for (int i = 0; i < vecsize; i++) {
    ranval = dug.GenerateInteger();
    testvec.at(i) = ranval;
  }

  auto sfunc = [&](V& val) {
    stringstream s;
    V deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(val, deser) << msg << " vector json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(val, deser) << msg << " vector binary ser/deser fails";
  };

  sfunc(testvec);
}

template <typename Element>
void ilparams_test(const string& msg) {
  auto p = ElemParamFactory::GenElemParams<typename Element::Params>(1024);

  auto sfunc = [&msg](decltype(p) val) {
    stringstream s;
    decltype(p) deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(*val, *deser) << msg << " json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(*val, *deser) << msg << " binary ser/deser fails";
  };

  sfunc(p);
}

TEST(UTSer, ilparams_test) { RUN_ALL_POLYS(ilparams_test, "ilparams_test") }

template <typename Element>
void ildcrtparams_test(const string& msg) {
  auto p = GenerateDCRTParams<typename Element::Integer>(1024, 5, 30);

  auto sfunc = [&msg](decltype(p) val) {
    stringstream s;
    decltype(p) deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(*val, *deser) << msg << " json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(*val, *deser) << msg << " binary ser/deser fails";
  };

  sfunc(p);
}

TEST(UTSer, ildcrtparams_test) {
  RUN_BIG_DCRTPOLYS(ildcrtparams_test, "ildcrtparams_test")
}

template <typename Element>
void ilvector_test(const string& msg) {
  auto p = ElemParamFactory::GenElemParams<typename Element::Params>(1024);
  typename Element::DugType dug;
  Element vec(dug, p);

  auto sfunc = [&](Element& val) {
    stringstream s;
    Element deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(val, deser) << msg << " vector json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(val, deser) << msg << " vector binary ser/deser fails";
  };

  sfunc(vec);
}

TEST(UTSer, ilvector_test) { RUN_ALL_POLYS(ilvector_test, "ilvector_test") }

template <typename Element>
void ildcrtpoly_test(const string& msg) {
  auto p = GenerateDCRTParams<typename Element::Integer>(1024, 5, 30);
  typename Element::DugType dug;
  Element vec(dug, p);

  auto sfunc = [&](Element& val) {
    stringstream s;
    Element deser;

    Serial::Serialize(val, s, SerType::JSON);
    Serial::Deserialize(deser, s, SerType::JSON);
    EXPECT_EQ(val, deser) << msg << " vector json ser/deser fails";

    s.str("");
    Serial::Serialize(val, s, SerType::BINARY);
    Serial::Deserialize(deser, s, SerType::BINARY);
    EXPECT_EQ(val, deser) << msg << " vector binary ser/deser fails";
  };

  sfunc(vec);
}

TEST(UTSer, ildcrtpoly_test) {
  RUN_BIG_DCRTPOLYS(ildcrtpoly_test, "ildcrtpoly_test")
}

////////////////////////////////////////////////////////////
template <typename V>
void serialize_matrix_bigint(const string& msg) {
  DEBUG_FLAG(false);
  // dimensions of matrix.
  const int nrows = 4;
  const int ncols = 8;

  DEBUG("step 0");
  const typename V::Integer mod((uint64_t)1 << 40);

  DEBUG("step 1");
  Matrix<typename V::Integer> testmat(V::Integer::Allocator, nrows, ncols);

  DEBUG("step 2");
  DiscreteUniformGeneratorImpl<V> dug;

  DEBUG("step 3");
  dug.SetModulus(mod);
  typename V::Integer ranval;

  // load up the matix with random values
  for (size_t i = 0; i < nrows; i++) {
    for (size_t j = 0; j < ncols; j++) {
      ranval = dug.GenerateInteger();
      testmat(i, j) = ranval;
    }
  }

  DEBUG("step 4");
  // serialize the Matrix

  stringstream ss;
  Serial::Serialize(testmat, ss, SerType::BINARY);

#if !defined(NDEBUG)
  if (dbg_flag) {
    std::cout << Serial::SerializeToString(testmat) << std::endl;
  }
#endif
  DEBUG("step 5");

  Matrix<typename V::Integer> newmat(V::Integer::Allocator, 0,
                                     0);  // empty matrix

  Serial::Deserialize(newmat, ss, SerType::BINARY);

  DEBUG("step 9");
  EXPECT_EQ(testmat, newmat) << msg << " Mismatch after ser/deser";
  DEBUGEXP(testmat);
  DEBUGEXP(newmat);
}

TEST(UTSer, serialize_matrix_bigint) {
  RUN_ALL_BACKENDS(serialize_matrix_bigint, "serialize_matrix_bigint")
}
