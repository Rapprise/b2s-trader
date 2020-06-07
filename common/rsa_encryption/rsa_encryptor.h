/*
 * Copyright (c) 2020, Rapprise.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AUTO_TRADER_RSA_ENCRYPTOR_H
#define AUTO_TRADER_RSA_ENCRYPTOR_H

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <string>
#include <vector>

#include "common/exceptions/license_exception/crypto_exception.h"
#include "common/utils.h"
#include "resources.h"

namespace auto_trader {
namespace common {
namespace crypto {

class RsaEncryptor {
 public:
  inline void init(const char* _key, int _padding = RSA_PKCS1_OAEP_PADDING) {
    rsa = nullptr;
    bio = nullptr;
    padding = _padding;

    bio = BIO_new_mem_buf(_key, -1);
    if (bio) {
      rsa = PEM_read_bio_RSA_PUBKEY(bio, &rsa, nullptr, nullptr);

      if (!rsa) {
        throw exceptions::CryptoException("PEM_read_bio_RSA_PUBKEY error");
      }
    }
  }

  inline std::string generateSignature(const std::string& message) {
    std::vector<unsigned char> messageBuffer(message.length() + 1, 0);
    std::copy(begin(message), end(message), begin(messageBuffer));

    std::vector<unsigned char> encryptedBuffer;
    auto encryptedLenght = encrypt(messageBuffer, encryptedBuffer);

    if (encryptedLenght == resources::ENCRYPTION_DECRYPTION_ERROR_CODE) {
      throw;
    }

    std::string signature = common::base_64_encode(encryptedBuffer);
    modifySignature(signature);

    return signature;
  }

  virtual ~RsaEncryptor() {
    if (rsa) {
      RSA_free(rsa);
      rsa = nullptr;
    }

    if (bio) {
      BIO_free(bio);
      rsa = nullptr;
    }
  }

 private:
  inline int encrypt(const std::vector<unsigned char>& data,
                     std::vector<unsigned char>& encrypted) {
    encrypted.resize(RSA_size(rsa));
    return RSA_public_encrypt(data.size(), data.data(), encrypted.data(), rsa, padding);
  }

  inline void modifySignature(std::string& signature) {
    for (const auto& pair : resources::RAPPRISE_INDEXES) {
      signature.insert(pair.first, pair.second);
    }
  }

  RSA* rsa;
  BIO* bio;

  int padding;
};

}  // namespace crypto
}  // namespace common
}  // namespace auto_trader

#endif  // AUTO_TRADER_RSA_ENCRYPTOR_H
