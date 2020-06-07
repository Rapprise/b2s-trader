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

#include "include/license.h"

#include <algorithm>
#include <cctype>

#include "common/exceptions/license_exception/signature_exception.h"
#include "common/loggers/file_logger.h"
#include "resources/feature_utils.h"
#include "resources/resources.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace auto_trader {
namespace features {
namespace license {

bool License::activateProduct(std::string key) {
  try {
    common::crypto::RsaDecryptor decryptor;
    decryptor.init(resources::license::PRIVATE_KEY.data());

    auto decryptedMessage = decodeKey(key);
    parseDecryptedData(decryptedMessage);
  } catch (const common::exceptions::CryptoException& ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
    return false;
  }

  std::vector<std::string> macAddresses = getMacAddresses();
  if (isCurrentDateLessThanDateFromKey() && compareMacAdress(macAddress_, macAddresses)) {
    writeToFiles(key);
    return true;
  }

  common::loggers::FileLogger::getLogger() << "Date or mac address is not valid";

  return false;
}

bool License::isLicenseExpired() {
  try {
    auto signature = readFile();
    common::crypto::RsaDecryptor decryptor;
    decryptor.init(resources::license::PRIVATE_KEY.data());

    auto decryptedMessage = decodeKey(signature);
    parseDecryptedData(decryptedMessage);
  } catch (const common::exceptions::BaseException& ex) {
    common::loggers::FileLogger::getLogger() << ex.what();
    return true;
  }

  std::vector<std::string> macAddresses = getMacAddresses();
  if (!isCurrentDateLessThanDateFromKey() || !compareMacAdress(macAddress_, macAddresses))
    return true;

  return false;
}

std::string License::getName() const { return name_; }

std::string License::getSurname() const { return surname_; }

std::string License::getExpireDate() const { return expireDate_; }

bool License::compareMacAdress(const std::string& macAddressFromSignature,
                               const std::vector<std::string>& macAddresses) const {
  for (const auto& macAddress : macAddresses) {
    if (macAddress.size() != macAddressFromSignature.size()) continue;

    bool result = std::equal(macAddress.begin(), macAddress.end(), macAddressFromSignature.begin(),
                             [](const char& left, const char& right) {
                               return (left == right || std::toupper(left) == std::toupper(right));
                             });

    if (result) {
      return true;
    }
  }

  return false;
}

bool License::isCurrentDateLessThanDateFromKey() const {
  std::string fullDate = expireDate_ + " " + expireTime_;
  QDateTime dateFromKey =
      QDateTime::fromString(fullDate.data(), resources::license::DATE_FORMAT.data());
  QDateTime currentDate = QDateTime::currentDateTime();

  if (currentDate < dateFromKey) {
    return true;
  }

  common::loggers::FileLogger::getLogger() << "Current date is older than the expiration date.";

  return false;
}

void License::writeToFiles(const std::string& signature) {
  auto fullPathWithName = getPathWithNameToLicenseFile();
  std::ofstream fout(fullPathWithName, std::ofstream::out | std::ofstream::trunc);
  if (fout) {
    fout << signature;
  }
}

void License::setFilename(const std::string& filename) { filename_ = filename; }

std::string License::readFile() {
  auto fullPathWithName = getPathWithNameToLicenseFile();

  std::string signature;
  std::string token;
  std::ifstream infile(fullPathWithName);

  if (!infile) {
    throw common::exceptions::BaseException("Cannot open file");
  }

  while (!infile.eof()) {
    getline(infile, token);
    signature += token;
  }
  infile.close();

  return signature;
}

std::vector<std::string> License::getMacAddresses() {
  auto macAddress = feature_utils::getMacAddresses();
  return macAddress;
}

std::string License::decodeKey(std::string key) {
  std::string message;
  common::crypto::RsaDecryptor decryptor;
  decryptor.init(resources::license::PRIVATE_KEY.data());
  message = decryptor.getMessageFromSignature(key);

  return message;
}

std::string License::getPathWithNameToLicenseFile() {
  std::string pathToLicenseFile;
  std::string fullPathWithName;

#if defined(__linux__) || defined(__APPLE__)
  struct passwd* pw = getpwuid(getuid());
  pathToLicenseFile = pw->pw_dir;

  std::string pathWithFolderName = pathToLicenseFile + resources::license::SLASH +
                                   resources::license::POINT +
                                   resources::license::HIDDEN_FOLDER_NAME;
  mkdir(pathWithFolderName.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  fullPathWithName = pathWithFolderName + resources::license::SLASH + filename_;
#elif (WIN32)
  std::string licenseDir;
  if (QCoreApplication::instance()) {
    licenseDir = QApplication::applicationDirPath().toStdString() + resources::license::SLASH +
                 resources::license::CONFIG_FOLDER_NAME;
  } else {
    licenseDir = resources::license::CONFIG_FOLDER_NAME;
  }

  if (!common::isDirectoryExists(fullPathWithName)) {
    _mkdir(licenseDir.c_str());
  }

  fullPathWithName = licenseDir + resources::license::SLASH + resources::license::POINT +
                     resources::license::HIDDEN_FOLDER_NAME + filename_;
#endif

  return fullPathWithName;
}

void License::parseDecryptedData(const std::string& data) {
  size_t startPosition = 0, endPosition,
         delimeterLenght = resources::license::DECRYPTED_DATA_DELIMETER.length();
  std::string token;
  std::vector<std::string> dataParts;

  while ((endPosition = data.find(resources::license::DECRYPTED_DATA_DELIMETER, startPosition)) !=
         std::string::npos) {
    token = data.substr(startPosition, endPosition - startPosition);
    startPosition = endPosition + delimeterLenght;
    dataParts.emplace_back(token);
  }

  dataParts.push_back(data.substr(startPosition));
  if (dataParts.size() < resources::license::DECRYPTED_ARGUMENTS_COUNT)
    throw common::exceptions::SignatureException(
        "There are fewer arguments than needed to activate the license.");

  name_ = dataParts.at(resources::license::NAME_INDEX);
  surname_ = dataParts.at(resources::license::SURNAME_INDEX);
  macAddress_ = dataParts.at(resources::license::MAC_ADDRESS_INDEX);
  expireDate_ = dataParts.at(resources::license::DATE_INDEX);
  expireTime_ = dataParts.at(resources::license::TIME_INDEX);
}

}  // namespace license
}  // namespace features
}  // namespace auto_trader