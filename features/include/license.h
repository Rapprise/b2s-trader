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

#ifndef AUTO_TRADER_LICENSE_H
#define AUTO_TRADER_LICENSE_H

#include <string>

#include "common/date.h"
#include "common/rsa_encryption/rsa_decryptor.h"
#include "features/resources/resources.h"

namespace auto_trader {
namespace features {
namespace license {

class License {
 public:
  bool activateProduct(std::string key);
  bool isLicenseExpired();

  std::string getName() const;
  std::string getSurname() const;
  std::string getExpireDate() const;

  std::string getPathWithNameToLicenseFile();
  std::string readFile();
  void writeToFiles(const std::string& signature);
  void setFilename(const std::string& filename);

  virtual std::vector<std::string> getMacAddresses();

 private:
  bool compareMacAdress(const std::string& macAddressFromSignature,
                        const std::vector<std::string>& macAddresses) const;
  bool isCurrentDateLessThanDateFromKey() const;

  std::string decodeKey(std::string key);
  void parseDecryptedData(const std::string& data);

 private:
  std::string name_ = "";
  std::string surname_ = "";
  std::string macAddress_ = "";
  std::string expireDate_ = "";
  std::string expireTime_ = "";

  std::string filename_ = resources::license::LICENSE_FILE_NAME;
};

}  // namespace license
}  // namespace features
}  // namespace auto_trader

#endif  // AUTO_TRADER_LICENSE_H
