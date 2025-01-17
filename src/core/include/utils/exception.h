/**
 * @file exception.h - framework for exceptions in PALISADE
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

#ifndef SRC_CORE_LIB_UTILS_EXCEPTION_H_
#define SRC_CORE_LIB_UTILS_EXCEPTION_H_

#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>

namespace lbcrypto {

class palisade_error : public std::runtime_error {
  std::string filename;
  int linenum;
  std::string message;

 public:
  palisade_error(const std::string& file, int line, const std::string& what)
      : std::runtime_error(what), filename(file), linenum(line) {
    message = filename + ":" + std::to_string(linenum) + " " + what;
  }

  const char* what() const throw() { return message.c_str(); }

  const std::string& GetFilename() const { return filename; }
  const int GetLinenum() const { return linenum; }
};

class config_error : public palisade_error {
 public:
  config_error(const std::string& file, int line, const std::string& what)
      : palisade_error(file, line, what) {}
};

class math_error : public palisade_error {
 public:
  math_error(const std::string& file, int line, const std::string& what)
      : palisade_error(file, line, what) {}
};

class not_implemented_error : public palisade_error {
 public:
  not_implemented_error(const std::string& file, int line,
                        const std::string& what)
      : palisade_error(file, line, what) {}
};

class not_available_error : public palisade_error {
 public:
  not_available_error(const std::string& file, int line,
                      const std::string& what)
      : palisade_error(file, line, what) {}
};

class type_error : public palisade_error {
 public:
  type_error(const std::string& file, int line, const std::string& what)
      : palisade_error(file, line, what) {}
};

// use this error when serializing palisade objects
class serialize_error : public palisade_error {
 public:
  serialize_error(const std::string& file, int line, const std::string& what)
      : palisade_error(file, line, what) {}
};

// use this error when deserializing palisade objects
class deserialize_error : public palisade_error {
 public:
  deserialize_error(const std::string& file, int line, const std::string& what)
      : palisade_error(file, line, what) {}
};

#define PALISADE_THROW(exc, expr) throw exc(__FILE__, __LINE__, (expr))

}  // namespace lbcrypto

#endif /* SRC_CORE_LIB_UTILS_EXCEPTION_H_ */
