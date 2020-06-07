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

#include "future_license_unit_test.h"

#include <gtest/gtest.h>

#include <cstdio>

#include "common/rsa_encryption/rsa_decryptor.h"
#include "common/rsa_encryption/rsa_encryptor.h"
#include "common/utils.h"
#include "include/license.h"
#include "resources/feature_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace features {
namespace license {
namespace unit_test {

TEST(RsaCrypto, encrypt_decrypt) {
  const std::string message = "First_Last_04:92:26:07:da:0b_01.01.2019.18.30.15";

  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  common::crypto::RsaDecryptor privKey;
  privKey.init(resources::license::PRIVATE_KEY.c_str());
  auto messageFromSignature = privKey.getMessageFromSignature(signature);

  EXPECT_EQ(message, messageFromSignature);
}

TEST(License, mac_address_exists) {
  License license;
  auto macAddresses = license.getMacAddresses();
  EXPECT_TRUE(!macAddresses.empty());
}

TEST(License, License_Not_enought_arguments_Test) {
  const std::string message = "First_Last_04:92:26:07:da:0b";

  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  License license;
  EXPECT_THROW(license.activateProduct(signature), common::exceptions::BaseException);
}

TEST_F(LicenseFixture, ActivateFalse_NotValidDate) {
  mockLincenseObject->DelegateToFakeMacAddressAndLicenseFilename();

  EXPECT_CALL(*mockLincenseObject, getMacAddresses()).Times(1);

  const std::string message = "First_Last_04:92:26:07:da:0b_01.06.2019_09:44:13";

  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  EXPECT_FALSE(mockLincenseObject->activateProduct(signature));
}

TEST_F(LicenseFixture, read_write) {
  mockLincenseObject->DelegateToFakeMacAddressAndLicenseFilename();

#ifdef WIN32
  _mkdir("config");
#endif

  const std::string message = "First_Last_04:92:26:07:da:0b_01.06.2019_09:44:13";
  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  mockLincenseObject->setFilename(resources::license::LICENSE_FILE_NAME_FOR_TESTS);
  mockLincenseObject->writeToFiles(signature);
  auto signatureFromFile = mockLincenseObject->readFile();

  EXPECT_EQ(signature, signatureFromFile);

  auto pathToFile = mockLincenseObject->getPathWithNameToLicenseFile();
  std::remove(pathToFile.data());
}

TEST_F(LicenseFixture, isLicenseExpired_True) {
  mockLincenseObject->DelegateToFakeMacAddressAndLicenseFilename();

  EXPECT_CALL(*mockLincenseObject, getMacAddresses()).Times(1);

  const std::string message = "First_Last_04:92:26:07:da:0b_01.06.2019_09:44:13";
  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  mockLincenseObject->setFilename(resources::license::LICENSE_FILE_NAME_FOR_TESTS);
  mockLincenseObject->writeToFiles(signature);

  EXPECT_TRUE(mockLincenseObject->isLicenseExpired());

  auto pathToFile = mockLincenseObject->getPathWithNameToLicenseFile();
  std::remove(pathToFile.data());
}

TEST_F(LicenseFixture, isLicenseExpired_False) {
  mockLincenseObject->DelegateToFakeMacAddressAndLicenseFilename();

  EXPECT_CALL(*mockLincenseObject, getMacAddresses()).Times(1);

  QDateTime currentDate = QDateTime::currentDateTime();
  auto dateWithAdditionalMounths = currentDate.addMonths(1);
  std::string date_str = dateWithAdditionalMounths.toString(resources::license::DATE_FORMAT.data())
                             .toUtf8()
                             .constData();
  std::replace(date_str.begin(), date_str.end(), ' ', '_');

  const std::string message = "First_Last_04:92:26:07:da:0b_" + date_str;
  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  mockLincenseObject->setFilename(resources::license::LICENSE_FILE_NAME_FOR_TESTS);
  mockLincenseObject->writeToFiles(signature);

  EXPECT_FALSE(mockLincenseObject->isLicenseExpired());

  auto pathToFile = mockLincenseObject->getPathWithNameToLicenseFile();
  std::remove(pathToFile.data());
}

TEST_F(LicenseFixture, isLicenseExpired_True_withBadMacAddresses) {
  mockLincenseObject->DelegateToFakeMacAddressAndLicenseFilename();

  EXPECT_CALL(*mockLincenseObject, getMacAddresses()).Times(1);

  QDateTime currentDate = QDateTime::currentDateTime();
  auto dateWithAdditionalMounths = currentDate.addMonths(1);
  std::string date_str = dateWithAdditionalMounths.toString(resources::license::DATE_FORMAT.data())
                             .toUtf8()
                             .constData();
  std::replace(date_str.begin(), date_str.end(), ' ', '_');

  const std::string message = "First_Last_04:92:21:07:da:0b_" + date_str;
  common::crypto::RsaEncryptor encryptor;
  encryptor.init(resources::license::PUBLIC_KEY.c_str());
  auto signature = encryptor.generateSignature(message);

  mockLincenseObject->setFilename(resources::license::LICENSE_FILE_NAME_FOR_TESTS);
  mockLincenseObject->writeToFiles(signature);

  EXPECT_TRUE(mockLincenseObject->isLicenseExpired());

  auto pathToFile = mockLincenseObject->getPathWithNameToLicenseFile();
  std::remove(pathToFile.data());
}

}  // namespace unit_test
}  // namespace license
}  // namespace features
}  // namespace auto_trader
