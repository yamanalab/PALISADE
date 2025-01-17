/**
 * @file mubintvecfxd.h This file contains the vector manipulation
 * functionality.
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
 * This file contains the vector manipulation functionality.
 */

#ifndef LBCRYPTO_MATH_BIGINTFXD_MUBINVECFXD_H
#define LBCRYPTO_MATH_BIGINTFXD_MUBINVECFXD_H

#include <iostream>

#include "utils/serializable.h"
#include "utils/inttypes.h"

#include <initializer_list>

#include "../bigintfxd/ubintfxd.h"

/**
 * @namespace bigintfxd
 * The namespace of bigintfxd
 */
namespace bigintfxd {

/**
 * @brief The class for representing vectors of big binary integers.
 */
template <class IntegerType>
class BigVectorImpl
    : public lbcrypto::BigVectorInterface<BigVectorImpl<IntegerType>,
                                          IntegerType>,
      public lbcrypto::Serializable {
 public:
  // CONSTRUCTORS

  /**
   * Basic constructor.
   */
  BigVectorImpl();

  static inline BigVectorImpl Single(const IntegerType &val,
                                     const IntegerType &modulus) {
    BigVectorImpl vec(1, modulus);
    vec[0] = val;
    return vec;
  }

  /**
   * Basic constructor for specifying the length of the vector and the modulus.
   *
   * @param length is the length of the big binary vector, in terms of the
   * number of entries.
   * @param modulus is the modulus of the ring.
   */
  BigVectorImpl(usint length, const IntegerType &modulus = 0);

  /**
   * Basic constructor for copying a vector
   *
   * @param bigVector is the big binary vector to be copied.
   */
  BigVectorImpl(const BigVectorImpl &bigVector);

  /**
   * Basic move constructor for moving a vector
   *
   * @param &&bigVector is the big binary vector to be moved.
   */
  BigVectorImpl(BigVectorImpl &&bigVector);  // move copy constructor

  /**
   * Basic constructor for specifying the length of the vector
   * the modulus and an initializer list.
   *
   * @param length is the length of the big binary vector, in terms of the
   * number of entries.
   * @param modulus is the modulus of the ring.
   * @param rhs is an initializer list of strings
   */

  BigVectorImpl(usint length, const IntegerType &modulus,
                std::initializer_list<std::string> rhs);

  /**
   * Basic constructor for specifying the length of the vector
   * the modulus and an initializer list.
   *
   * @param length is the length of the big binary vector, in terms of the
   * number of entries.
   * @param modulus is the modulus of the ring.
   * @param rhs is an initializer list of usint
   */
  BigVectorImpl(usint length, const IntegerType &modulus,
                std::initializer_list<uint64_t> rhs);

  /**
   * Destructor.
   */
  virtual ~BigVectorImpl();

  // ASSIGNMENT OPERATORS

  /**
   * Assignment operator to assign value from rhs
   *
   * @param &rhs is the big binary vector to be assigned from.
   * @return Assigned BigVectorImpl.
   */
  const BigVectorImpl &operator=(const BigVectorImpl &rhs);

  /**
   * Move assignment operator
   *
   * @param &&rhs is the big binary vector to be moved.
   * @return moved BigVectorImpl object
   */
  const BigVectorImpl &operator=(BigVectorImpl &&rhs);

  /**
   * Initializer list for BigVectorImpl.
   *
   * @param &&rhs is the list of strings containing integers to be assigned to
   * the BBV.
   * @return BigVectorImpl object
   */
  const BigVectorImpl &operator=(std::initializer_list<std::string> rhs);

  /**
   * Initializer list for BigVectorImpl.
   *
   * @param &&rhs is the list of integers to be assigned to the BBV.
   * @return BigVectorImpl object
   */
  const BigVectorImpl &operator=(std::initializer_list<uint64_t> rhs);

  /**
   * Assignment operator to assign value val to first entry, 0 for the rest of
   * entries.
   *
   * @param val is the value to be assigned at the first entry.
   * @return Assigned BigVectorImpl.
   */
  const BigVectorImpl &operator=(uint64_t val) {
    this->m_data[0] = val;
    if (this->m_modulus != 0) {
      this->m_data[0] %= this->m_modulus;
    }
    for (size_t i = 1; i < GetLength(); ++i) {
      this->m_data[i] = 0;
    }
    return *this;
  }

  // ACCESSORS

  /**
   * Sets/gets a value at an index.
   * This method is slower than operator[] as it checks if index out of range
   *
   * @param index is the index to set a value at.
   */
  IntegerType &at(size_t i) {
    if (!this->IndexCheck(i)) {
      PALISADE_THROW(lbcrypto::math_error, "BigVector index out of range");
    }
    return this->m_data[i];
  }

  const IntegerType &at(size_t i) const {
    if (!this->IndexCheck(i)) {
      PALISADE_THROW(lbcrypto::math_error, "BigVector index out of range");
    }
    return this->m_data[i];
  }

  /**
   * operators to get a value at an index.
   * @param idx is the index to get a value at.
   * @return is the value at the index.
   */
  IntegerType &operator[](size_t idx) { return (this->m_data[idx]); }

  const IntegerType &operator[](size_t idx) const {
    return (this->m_data[idx]);
  }

  /**
   * Sets the vector modulus.
   *
   * @param value is the value to set.
   * @param value is the modulus value to set.
   */
  void SetModulus(const IntegerType &value);

  /**
   * Sets the vector modulus and changes the values to match the new modulus.
   *
   * @param value is the value to set.
   */
  void SwitchModulus(const IntegerType &value);

  /**
   * Gets the vector modulus.
   *
   * @return the vector modulus.
   */
  const IntegerType &GetModulus() const { return this->m_modulus; }

  /**
   * Gets the vector length.
   *
   * @return vector length.
   */
  size_t GetLength() const { return this->m_length; }

  // MODULAR ARITHMETIC OPERATIONS

  /**
   * Vector modulus operator.
   *
   * @param &modulus is the modulus to perform on the current vector entries.
   * @return is the result of the modulus operation on current vector.
   */
  BigVectorImpl Mod(const IntegerType &modulus) const;

  /**
   * Vector modulus operator. In-place variant.
   *
   * @param &modulus is the modulus to perform on the current vector entries.
   * @return is the result of the modulus operation on current vector.
   */
  const BigVectorImpl &ModEq(const IntegerType &modulus);

  /**
   * Scalar-to-vector modulus addition operation.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus addition operation.
   */
  BigVectorImpl ModAdd(const IntegerType &b) const;

  /**
   * Scalar-to-vector modulus addition operation. In-place variant.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus addition operation.
   */
  const BigVectorImpl &ModAddEq(const IntegerType &b);

  /**
   * Scalar modulus addition at a particular index.
   *
   * @param i is the index of the entry to add.
   * @param &b is the scalar to add.
   * @return is the result of the modulus addition operation.
   */
  BigVectorImpl ModAddAtIndex(usint i, const IntegerType &b) const;

  /**
   * Scalar modulus addition at a particular index. In-place variant.
   *
   * @param i is the index of the entry to add.
   * @param &b is the scalar to add.
   * @return is the result of the modulus addition operation.
   */
  const BigVectorImpl &ModAddAtIndexEq(usint i, const IntegerType &b);

  /**
   * Vector component wise modulus addition.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus addition operation.
   */
  BigVectorImpl ModAdd(const BigVectorImpl &b) const;

  /**
   * Vector component wise modulus addition. In-place variant.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus addition operation.
   */
  const BigVectorImpl &ModAddEq(const BigVectorImpl &b);

  /**
   * Scalar-from-vector modulus subtraction operation.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus subtraction operation.
   */
  BigVectorImpl ModSub(const IntegerType &b) const;

  /**
   * Scalar-from-vector modulus subtraction operation. In-place variant.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus subtraction operation.
   */
  const BigVectorImpl &ModSubEq(const IntegerType &b);

  /**
   * Vector component wise modulus subtraction.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus subtraction operation.
   */
  BigVectorImpl ModSub(const BigVectorImpl &b) const;

  /**
   * Vector component wise modulus subtraction. In-place variant.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus subtraction operation.
   */
  const BigVectorImpl &ModSubEq(const BigVectorImpl &b);

  /**
   * Scalar-to-vector modulus multiplication operation.
   * Generalized Barrett modulo reduction algorithm.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus multiplication operation.
   */
  BigVectorImpl ModMul(const IntegerType &b) const;

  /**
   * Scalar-to-vector modulus multiplication operation. In-place variant.
   * Generalized Barrett modulo reduction algorithm.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus multiplication operation.
   */
  const BigVectorImpl &ModMulEq(const IntegerType &b);

  /**
   * Vector component wise modulus multiplication.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus multiplication
   * operation.
   */
  BigVectorImpl ModMul(const BigVectorImpl &b) const;

  /**
   * Vector component wise modulus multiplication. In-place variant.
   *
   * @param &b is the vector to perform operation with.
   * @return is the result of the component wise modulus multiplication
   * operation.
   */
  const BigVectorImpl &ModMulEq(const BigVectorImpl &b);

  /**
   * Scalar modulus exponentiation operation.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus exponentiation operation.
   */
  BigVectorImpl ModExp(const IntegerType &b) const;

  /**
   * Scalar modulus exponentiation operation. In-place variant.
   *
   * @param &b is the scalar to perform operation with.
   * @return is the result of the modulus exponentiation operation.
   */
  const BigVectorImpl &ModExpEq(const IntegerType &b);

  /**
   * Modulus inverse operation.
   *
   * @return is the result of the component wise modulus inverse operation.
   */
  BigVectorImpl ModInverse() const;

  /**
   * Modulus inverse operation. In-place variant.
   *
   * @return is the result of the component wise modulus inverse operation.
   */
  const BigVectorImpl &ModInverseEq();

  /**
   * Modulus 2 operation, also a least significant bit.
   *
   * @return is the result of the component wise modulus 2 operation, also a
   * least significant bit.
   */
  BigVectorImpl ModByTwo() const;

  /**
   * Modulus 2 operation, also a least significant bit. In-place variant.
   *
   * @return is the result of the component wise modulus 2 operation, also a
   * least significant bit.
   */
  const BigVectorImpl &ModByTwoEq();

  /**
   * Vector multiplication without applying the modulus operation.
   *
   * @param &b is the vector to multiply.
   * @return is the result of the multiplication operation.
   */
  BigVectorImpl MultWithOutMod(const BigVectorImpl &b) const;

  /**
   * Vector multiplication without applying the modulus operation. In-place
   * variant.
   *
   * @param &b is the vector to multiply.
   * @return is the result of the multiplication operation.
   */
  const BigVectorImpl &MultWithOutModEq(const BigVectorImpl &b);

  /**
   * Multiply and Rounding operation. Returns [x*p/q] where [] is the rounding
   * operation.
   *
   * @param &p is the numerator to be multiplied.
   * @param &q is the denominator to be divided.
   * @return is the result of multiply and round operation.
   */
  BigVectorImpl MultiplyAndRound(const IntegerType &p,
                                 const IntegerType &q) const;

  /**
   * Multiply and Rounding operation. Returns [x*p/q] where [] is the rounding
   * operation. In-place variant.
   *
   * @param &p is the numerator to be multiplied.
   * @param &q is the denominator to be divided.
   * @return is the result of multiply and round operation.
   */
  const BigVectorImpl &MultiplyAndRoundEq(const IntegerType &p,
                                          const IntegerType &q);

  /**
   * Divide and Rounding operation. Returns [x/q] where [] is the rounding
   * operation.
   *
   * @param &q is the denominator to be divided.
   * @return is the result of divide and round operation.
   */
  BigVectorImpl DivideAndRound(const IntegerType &q) const;

  /**
   * Divide and Rounding operation. Returns [x/q] where [] is the rounding
   * operation. In-place variant.
   *
   * @param &q is the denominator to be divided.
   * @return is the result of divide and round operation.
   */
  const BigVectorImpl &DivideAndRoundEq(const IntegerType &q);

  // OTHER FUNCTIONS

  /**
   * Digit vector at a specific index for all entries for a given number base.
   * Warning: only power-of-2 bases are currently supported.
   * Example: for vector (83, 1, 45), index 2 and base 4 we have:
   *
   *                           index:0,1,2,3
   * |83|                           |3,0,1,1|                 |1|
   * |1 | --base 4 decomposition--> |1,0,0,0| --at index 2--> |0|
   * |45|                           |1,3,2,0|                 |2|
   *
   * The return vector is (1,0,2)
   *
   * @param index is the index to return the digit from in all entries.
   * @param base is the base to use for the operation.
   * @return is the digit at a specific index for all entries for a given number
   * base
   */
  BigVectorImpl GetDigitAtIndexForBase(usint index, usint base) const;

  // STRINGS & STREAMS

  /**
   * ostream operator to output vector values to console
   *
   * @param os is the std ostream object.
   * @param &ptr_obj is the BigVectorImpl object to be printed.
   * @return std ostream object which captures the vector values.
   */
  template <class IntegerType_c>
  friend std::ostream &operator<<(std::ostream &os,
                                  const BigVectorImpl<IntegerType_c> &ptr_obj) {
    auto len = ptr_obj.m_length;
    os << "[";
    for (usint i = 0; i < len; i++) {
      os << ptr_obj.m_data[i];
      os << ((i == (len - 1)) ? "]" : " ");
    }
    os << " modulus: " << ptr_obj.m_modulus;
    return os;
  }

  // SERIALIZATION

  template <class Archive>
  typename std::enable_if<!cereal::traits::is_text_archive<Archive>::value,
                          void>::type
  save(Archive &ar, std::uint32_t const version) const {
    ar(::cereal::make_nvp("m", m_modulus));
    ar(::cereal::make_nvp("l", m_length));
    ar(::cereal::binary_data(m_data, sizeof(IntegerType) * m_length));
  }

  template <class Archive>
  typename std::enable_if<cereal::traits::is_text_archive<Archive>::value,
                          void>::type
  save(Archive &ar, std::uint32_t const version) const {
    ar(::cereal::make_nvp("m", m_modulus));
    ar(::cereal::make_nvp("l", m_length));
    for (size_t i = 0; i < m_length; i++) {
      ar(m_data[i]);
    }
  }

  template <class Archive>
  typename std::enable_if<!cereal::traits::is_text_archive<Archive>::value,
                          void>::type
  load(Archive &ar, std::uint32_t const version) {
    if (version > SerializedVersion()) {
      PALISADE_THROW(lbcrypto::deserialize_error,
                     "serialized object version " + std::to_string(version) +
                         " is from a later version of the library");
    }
    ar(::cereal::make_nvp("m", m_modulus));
    ar(::cereal::make_nvp("l", m_length));
    m_data = new IntegerType[m_length]();
    ar(::cereal::binary_data(m_data, sizeof(IntegerType) * m_length));
  }

  template <class Archive>
  typename std::enable_if<cereal::traits::is_text_archive<Archive>::value,
                          void>::type
  load(Archive &ar, std::uint32_t const version) {
    if (version > SerializedVersion()) {
      PALISADE_THROW(lbcrypto::deserialize_error,
                     "serialized object version " + std::to_string(version) +
                         " is from a later version of the library");
    }
    ar(::cereal::make_nvp("m", m_modulus));
    ar(::cereal::make_nvp("l", m_length));
    m_data = new IntegerType[m_length]();
    for (size_t i = 0; i < m_length; i++) {
      ar(m_data[i]);
    }
  }

  std::string SerializedObjectName() const { return "FXDInteger"; }

  static uint32_t SerializedVersion() { return 1; }

 private:
  // m_data is a pointer to the vector
  IntegerType *m_data;
  // m_length stores the length of the vector
  usint m_length;
  // m_modulus stores the internal modulus of the vector.
  IntegerType m_modulus = 0;

  // function to check if the index is a valid index.
  bool IndexCheck(size_t length) const {
    if (length > this->m_length) {
      return false;
    }
    return true;
  }
};

extern template class BigVectorImpl<
    BigInteger<integral_dtype, BigIntegerBitLength>>;

}  // namespace bigintfxd

#endif  // LBCRYPTO_MATH_BIGINTFXD_MUBINVECFXD_H
