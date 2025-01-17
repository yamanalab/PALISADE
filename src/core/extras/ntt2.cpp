/*
 * @file ntt2.cpp - Another example of NTT operations
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

// This is a main() file built to test and time NTT operations
// D. Cousins

#define PROFILE  // need to define in order to turn on timing

#include <iostream>
#include <fstream>
#include <vector>
#include "palisadecore.h"
#include "time.h"
#include <chrono>
#include <exception>

using namespace std;
using namespace lbcrypto;

// define the main sections of the test
void test_NTT(const usint level, const usint nloop);  // test code

// main()   need this for Kurts' makefile to ignore this.
int main(int argc, char *argv[]) {
  if (argc < 2)  // argc should be 2 for correct execution
    // We print argv[0] assuming it is the program name
    std::cout << "usage: " << argv[0] << " 1|2|3(default 1) nloop (default 10)"
              << endl;
  usint level = 1;
  usint nloop = 10;
  if (argc > 1) level = atoi(argv[1]);
  if (argc > 2) nloop = atoi(argv[2]);

  if (level > 3) level = 3;
  if (level < 1) level = 1;

  if (nloop < 1) nloop = 1;
  cout << "running " << argv[0] << " level = " << level << " nloop = " << nloop
       << endl;

  test_NTT(level, nloop);
  return 0;
}

// function to compare two BigVectors and print differing indicies
void vec_diff(BigVector &a, BigVector &b) {
  for (usint i = 0; i < a.GetLength(); ++i) {
    if (a.at(i) != b.at(i)) {
      cout << "i: " << i << endl;
      cout << "first vector " << endl;
      cout << a.at(i);
      cout << endl;
      cout << "second vector " << endl;
      cout << b.at(i);
      cout << endl;
    }
  }
}

// function to compare two Poly and print differing values
bool clonetest(Poly &a, Poly &b, string name) {
  if (a != b) {
    cout << name << " FAILED " << endl;
    return true;
  } else {
    return false;
  }
}

// main NTT test suite.
void test_NTT(const usint level, const usint nloop) {
  // Code to test NTT at three different numbers of limbs.

  TimeVar t1, t2, t3, t_setup, t_total;  // timers for TIC() TOC()

  // captures the time
  double time1ar, time1af;
  double time2ar, time2af;
  double time3ar, time3af;

  double time1br, time1bf;
  double time2br, time2bf;
  double time3br, time3bf;

  cout << "testing NTT backend " << MATHBACKEND;
  if (BigIntegerBitLength > 0) cout << " BITLENGTH " << BigIntegerBitLength;
  cout << endl;

  TIC(t_total);
  TIC(t_setup);

  BigInteger q1("270337");  // test case 1 smaller than 32 bits

  usint m = 2048;
  cout << "m=" << m << endl;

  BigInteger rootOfUnity1(RootOfUnity<BigInteger>(m, q1));
  cout << "q1 = " << q1 << endl;
  cout << "rootOfUnity1 = " << rootOfUnity1 << endl;

  // build parameters for two vectors.
  ILParams params1(m, q1, rootOfUnity1);
  shared_ptr<ILParams> x1p(new ILParams(params1));

  Poly::DugType dug1;
  dug1.SetModulus(q1);  // random # generator to use

  // two vectors
  Poly x1a(dug1, x1p, Format::COEFFICIENT);
  Poly x1b(dug1, x1p, Format::COEFFICIENT);

  for (size_t ix = 0; ix < m / 2; ix++) {
    if (x1a.GetValues().at(ix) >= q1) {
      cout << "bad value x1a " << endl;
    }
    if (x1b.GetValues().at(ix) >= q1) {
      cout << "bad value x1a " << endl;
    }
  }

  // make copies to compare against
  Poly x1aClone(x1a);
  Poly x1bClone(x1b);
  cout << "setup 1 time " << TOC_US(t_setup) << " usec" << endl;
  TIC(t_setup);
  // repeat for q2;
  BigInteger q2("4503599627446273");  // test case 2 32 > x> 64 bits

  BigInteger rootOfUnity2(RootOfUnity<BigInteger>(m, q2));
  cout << "q2 = " << q2 << endl;
  cout << "rootOfUnity2 = " << rootOfUnity2 << endl;

  ILParams params2(m, q2, rootOfUnity2);
  shared_ptr<ILParams> x2p(new ILParams(params2));

  Poly::DugType dug2;
  dug2.SetModulus(q1);  // random # generator to use

  Poly x2a(dug2, x2p, Format::COEFFICIENT);
  Poly x2b(dug2, x2p, Format::COEFFICIENT);

  Poly x2aClone(x2a);
  Poly x2bClone(x2b);

  cout << "setup 2 time " << TOC_US(t_setup) << " usec" << endl;
  TIC(t_setup);

  // repeat for q3
  // note computation of root of unity for big numbers takes forever
  // hardwire this case
  BigInteger q3(
      "130935624315845674800527587873103966088665681841722591579331654723845351"
      "856186982195330803693036166286035467365102402840368690261835415722133141"
      "10873601");

  BigInteger rootOfUnity3(
      "120238484638556494666603774400695561444642670309493651659937259422204414"
      "126327993119899739382548230714053366233156689615011395926730002978876828"
      "95033094");

  cout << "q3 : " << q3.ToString() << endl;
  cout << "rootOfUnity3 : " << rootOfUnity3.ToString() << endl;

  ILParams params3(m, q3, rootOfUnity3);
  shared_ptr<ILParams> x3p(new ILParams(params3));

  Poly::DugType dug3;
  dug3.SetModulus(q1);  // random # generator to use

  // two vectors
  Poly x3a(dug3, x3p, Format::COEFFICIENT);
  Poly x3b(dug3, x3p, Format::COEFFICIENT);

  // make copies to compare against
  Poly x3aClone(x3a);
  Poly x3bClone(x3b);
  cout << "setup 3 time " << TOC_US(t_setup) << " usec" << endl;

  // Precomputations for FTT
  TIC(t_setup);
  ChineseRemainderTransformFTT<BigVector>::PreCompute(rootOfUnity1, m, q1);
  ChineseRemainderTransformFTT<BigVector>::PreCompute(rootOfUnity2, m, q2);
  cout << "CRT 2 setup time " << TOC_US(t_setup) << " usec" << endl;
  TIC(t_setup);
  ChineseRemainderTransformFTT<BigVector>::PreCompute(rootOfUnity3, m, q3);
  cout << "CRT 3 setup time " << TOC_US(t_setup) << " usec" << endl;

  time1af = 0.0;
  time1bf = 0.0;

  time2af = 0.0;
  time2bf = 0.0;

  time3af = 0.0;
  time3bf = 0.0;

  time1ar = 0.0;
  time1br = 0.0;

  time2ar = 0.0;
  time2br = 0.0;

  time3ar = 0.0;
  time3br = 0.0;

  bool failed = false;
  usint ix;
  cout << "Starting timing" << endl;

  for (ix = 0; ix < nloop; ix++) {
    if (ix % 100 == 0) cout << ix << endl;  // print out status every 100 loops

    // forward transforms
    if (level > 0) {
      TIC(t1);
      x1a.SwitchFormat();
      time1af += TOC_US(t1);

      TIC(t1);
      x1b.SwitchFormat();
      time1bf += TOC_US(t1);
    }
    if (level > 1) {
      TIC(t1);
      x2a.SwitchFormat();
      time2af += TOC_US(t1);

      TIC(t1);
      x2b.SwitchFormat();
      time2bf += TOC_US(t1);
    }
    if (level > 2) {
      TIC(t1);
      x3a.SwitchFormat();
      time3af += TOC_US(t1);

      TIC(t1);
      x3b.SwitchFormat();
      time3bf += TOC_US(t1);
    }
    // reverse transforms
    if (level > 0) {
      TIC(t1);
      x1a.SwitchFormat();
      time1ar += TOC_US(t1);

      TIC(t1);
      x1b.SwitchFormat();
      time1br += TOC_US(t1);
    }
    if (level > 1) {
      TIC(t1);
      x2a.SwitchFormat();
      time2ar += TOC_US(t1);

      TIC(t1);
      x2b.SwitchFormat();
      time2br += TOC_US(t1);
    }
    if (level > 2) {
      TIC(t1);
      x3a.SwitchFormat();
      time3ar += TOC_US(t1);

      TIC(t1);
      x3b.SwitchFormat();
      time3br += TOC_US(t1);
    }
    if (level > 0) {
      failed |= clonetest(x1a, x1aClone, "x1a");
      failed |= clonetest(x1b, x1bClone, "x1b");
    }
    if (level > 1) {
      failed |= clonetest(x2a, x2aClone, "x2a");
      failed |= clonetest(x2b, x2bClone, "x2b");
    }
    if (level > 2) {
      failed |= clonetest(x3a, x3aClone, "x3a");
      failed |= clonetest(x3b, x3bClone, "x3b");
    }
  }

  if (failed) {
    cout << "failure in loop number " << ix << endl;
  } else {
    time1af /= (double)nloop;
    time1bf /= (double)nloop;
    time2af /= (double)nloop;
    time2bf /= (double)nloop;
    time3af /= (double)nloop;
    time3bf /= (double)nloop;

    time1ar /= (double)nloop;
    time1br /= (double)nloop;
    time2ar /= (double)nloop;
    time2br /= (double)nloop;
    time3ar /= (double)nloop;
    time3br /= (double)nloop;

    cout << nloop << " loops" << endl;
    if (level > 0) {
      cout << "t1af: "
           << "\t" << time1af << " us" << endl;
      cout << "t1bf: "
           << "\t" << time1bf << " us" << endl;
      cout << "t1ar: "
           << "\t" << time1ar << " us" << endl;
      cout << "t1br: "
           << "\t" << time1br << " us" << endl;
    }
    if (level > 1) {
      cout << "t2af: "
           << "\t" << time2af << " us" << endl;
      cout << "t2bf: "
           << "\t" << time2bf << " us" << endl;
      cout << "t2ar: "
           << "\t" << time2ar << " us" << endl;
      cout << "t2br: "
           << "\t" << time2br << " us" << endl;
    }
    if (level > 2) {
      cout << "t3af: "
           << "\t" << time3af << " us" << endl;
      cout << "t3bf: "
           << "\t" << time3bf << " us" << endl;
      cout << "t3ar: "
           << "\t" << time3ar << " us" << endl;
      cout << "t3br: "
           << "\t" << time3br << " us" << endl;
    }
  }

  return;
}
