/**
 * @file stdlatticeparms.h: Header for the standard values for Lattice Parms, as
 * determined by homomorphicencryption.org
 *
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

#ifndef SRC_CORE_LIB_LATTICE_STDLATTICEPARMS_H_
#define SRC_CORE_LIB_LATTICE_STDLATTICEPARMS_H_

#include "backend.h"
#include <map>
using std::map;

namespace lbcrypto {

// this is the representation of the standard lattice parameters defined in the
// Homomorphic Encryption Standard, as defined by
// http://homomorphicencryption.org

// given a distribution type and a security level, you can get the maxQ for a
// given ring dimension, and you can get the ring dimension given a maxQ

// The code below is very specific to the layout of the DistributionType and
// SecurityLevel enums IF you change them, go look at and change byRing and
// byLogQ

enum DistributionType {
  HEStd_uniform,
  HEStd_error,
  HEStd_ternary,
};

enum SecurityLevel {
  HEStd_128_classic,
  HEStd_192_classic,
  HEStd_256_classic,
  HEStd_NotSet,
};

class StdLatticeParm {
  DistributionType distType;
  usint ringDim;
  SecurityLevel minSecLev;
  usint maxLogQ;

  // NOTE!!! the declaration below relies upon there being three possible values
  // for the first index (the distribution type), and three possible values for
  // the second index (the security level)
  // The values in the enums, above, meet this criteria
  // it's also important that the different values are numbered from 0-2
  // again, the enums above do this
  // DO NOT change the values of the enums to be anything other than consecutive
  // numbers starting from 0, or this code will break in strange ways, and you
  // will suffer MAKE SURE that the number of entries in the DistributionType
  // enum is == the first index, and MAKE SURE that the number of entries in the
  // SecurityLevel enum is == the second index
  static map<usint, StdLatticeParm*> byRing[3][3];
  static map<usint, StdLatticeParm*> byLogQ[3][3];

  static vector<StdLatticeParm> StandardLatticeParmSets;
  static bool initialized;

 public:
  StdLatticeParm(DistributionType distType, usint ringDim,
                 SecurityLevel minSecLev, usint maxLogQ)
      : distType(distType),
        ringDim(ringDim),
        minSecLev(minSecLev),
        maxLogQ(maxLogQ) {}

  static void initializeLookups() {
    for (size_t i = 0; i < StandardLatticeParmSets.size(); i++) {
      StdLatticeParm& s = StandardLatticeParmSets[i];
      byRing[s.distType][s.minSecLev][s.ringDim] = &s;
      byLogQ[s.distType][s.minSecLev][s.maxLogQ] = &s;
    }
    initialized = true;
  }

  static usint FindMaxQ(DistributionType distType, SecurityLevel minSecLev,
                        usint ringDim) {
    if (!initialized) initializeLookups();
    auto it = byRing[distType][minSecLev].find(ringDim);
    if (it == byRing[distType][minSecLev].end()) return 0;
    return it->second->getMaxLogQ();
  }

  static usint FindRingDim(DistributionType distType, SecurityLevel minSecLev,
                           usint curLogQ) {
    if (!initialized) initializeLookups();
    usint prev = 0;
    map<usint, StdLatticeParm*>::iterator it;
    for (it = byLogQ[distType][minSecLev].begin();
         it != byLogQ[distType][minSecLev].end(); it++) {
      if ((curLogQ < it->second->getMaxLogQ()) && (curLogQ > prev))
        return it->second->getRingDim();
      prev = it->second->getMaxLogQ();
    }
    return 65536;
  }

  DistributionType getDistType() const { return distType; }
  usint getRingDim() const { return ringDim; }
  SecurityLevel getMinSecLev() const { return minSecLev; }
  usint getMaxLogQ() const { return maxLogQ; }
};

// map<StdLatticeParm&,StdLatticeParm&>

} /* namespace lbcrypto */

#endif /* SRC_CORE_LIB_LATTICE_STDLATTICEPARMS_H_ */
