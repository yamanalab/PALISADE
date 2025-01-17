/**
 * @file be4-poly-impl.cpp This file contains template instantiations for all
 * classes using math be4
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

#include "math/backend.h"
#include "lattice/backend.h"
#include "math/matrix.cpp"
#include "matrix-lattice-impl.cpp"

#include "elemparams.cpp"
#include "ilparams.cpp"
#include "poly.cpp"

namespace lbcrypto {

template class ElemParams<M4Integer>;
template class ILParamsImpl<M4Integer>;
template class PolyImpl<M4Vector>;
template class DCRTPolyImpl<M4Vector>;

template class Matrix<M4Poly>;
ONES_FOR_TYPE(M4Poly)
IDENTITY_FOR_TYPE(M4Poly)
GADGET_FOR_TYPE(M4Poly)
NORM_FOR_TYPE(M4Poly)
SPLIT64_FOR_TYPE(M4Poly)
SPLIT64ALT_FOR_TYPE(M4Poly)
SPLIT32ALT_FOR_TYPE(M4Poly)
template Matrix<M4Vector> RotateVecResult(Matrix<M4Poly> const& inMat);
template Matrix<M4Integer> Rotate(Matrix<M4Poly> const& inMat);

template class Matrix<M4DCRTPoly>;
ONES_FOR_TYPE(M4DCRTPoly)
IDENTITY_FOR_TYPE(M4DCRTPoly)
GADGET_FOR_TYPE_DCRT(M4DCRTPoly)
NORM_FOR_TYPE(M4DCRTPoly)
SPLIT64_FOR_TYPE(M4DCRTPoly)
SPLIT64ALT_FOR_TYPE(M4DCRTPoly)
SPLIT32ALT_FOR_TYPE(M4DCRTPoly)
template Matrix<M4Vector> RotateVecResult(Matrix<M4DCRTPoly> const& inMat);
template Matrix<M4Integer> Rotate(Matrix<M4DCRTPoly> const& inMat);

// biginteger version
template <>
PolyImpl<NativeVector> PolyImpl<M4Vector>::ToNativePoly() const {
  PolyImpl<NativeVector> interp(
      shared_ptr<ILParamsImpl<NativeInteger>>(new ILParamsImpl<NativeInteger>(
          this->GetCyclotomicOrder(), std::numeric_limits<uint64_t>::max(), 1)),
      this->GetFormat(), true);

  for (usint i = 0; i < this->GetLength(); i++) {
    interp[i] = (*this)[i].ConvertToInt();
  }

  return std::move(interp);
}

}  // namespace lbcrypto

CEREAL_CLASS_VERSION(lbcrypto::M4Poly, lbcrypto::M4Poly::SerializedVersion());
CEREAL_CLASS_VERSION(lbcrypto::M4DCRTPoly,
                     lbcrypto::M4DCRTPoly::SerializedVersion());
