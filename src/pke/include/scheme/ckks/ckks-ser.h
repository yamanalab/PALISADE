/*
 * @file bgv-ser.h -- serialize ckks; include this in any app that needs to
 * serialize this scheme
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

#ifndef LBCRYPTO_CRYPTO_CKKSSER_H
#define LBCRYPTO_CRYPTO_CKKSSER_H

#include "palisade.h"
#include "utils/serial.h"

extern template class lbcrypto::LPCryptoParametersCKKS<lbcrypto::DCRTPoly>;
extern template class lbcrypto::LPPublicKeyEncryptionSchemeCKKS<
    lbcrypto::DCRTPoly>;
extern template class lbcrypto::LPAlgorithmCKKS<lbcrypto::DCRTPoly>;
extern template class lbcrypto::LPAlgorithmMultipartyCKKS<lbcrypto::DCRTPoly>;
extern template class lbcrypto::LPAlgorithmSHECKKS<lbcrypto::DCRTPoly>;

CEREAL_REGISTER_TYPE(lbcrypto::LPCryptoParametersCKKS<lbcrypto::DCRTPoly>);
CEREAL_REGISTER_TYPE(
    lbcrypto::LPPublicKeyEncryptionSchemeCKKS<lbcrypto::DCRTPoly>);

#endif
