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

#include "signature_encryptor_unit_test.h"

#include <stdlib.h>

#include <cstdio>
#include <fstream>

#include "common/rsa_encryption/rsa_decryptor.h"
#include "gtest/gtest.h"
#include "resources/resources.h"

#ifdef __unix__
#include <limits.h>
#endif

namespace auto_trader {
namespace signature_encryptor {
namespace unit_test {

TEST(Linux_signature_encryptor_Test, Linux_signature_encryptor_Test_Equals_Data_Test) {
  std::string message = "First_Last_04:92:26:07:da:0b_01.06.2019_09:44:13_11";
  std::string filename = "First_Last_01.06.2019_11";
#ifdef WIN32
  char *fileExt;
  char directory[512];
  GetFullPathName("signature_encryptor_unit_test.exe", 256, directory, &fileExt);
  std::string dirPath(directory);
  std::string fullPathToExecute = dirPath.substr(0, dirPath.find("signature_encryptor", 0));

#ifdef _DEBUG
  fullPathToExecute +=
      "signature_encryptor\\Debug\\signature_encryptor.exe First Last 04:92:26:07:da:0b "
      "01.06.2019 09:44:13 11";
#else
  fullPathToExecute +=
      "signature_encryptor\\Release\\signature_encryptor.exe First Last 04:92:26:07:da:0b "
      "01.06.2019 09:44:13 11";
#endif

  std::system(fullPathToExecute.c_str());

#elif __unix__ || __APPLE__
  char directory[PATH_MAX];
  realpath("signature_encryptor_unit_test", directory);
  std::string dirPath = directory;
  std::string fullPathToExecute = dirPath.substr(0, dirPath.find("signature_encryptor", 0));
  fullPathToExecute +=
      "signature_encryptor/signature_encryptor First Last 04:92:26:07:da:0b 01.06.2019 09:44:13 "
      "11";
  system(fullPathToExecute.c_str());
#endif

  std::string path_to_file_with_key = auto_trader::signature_encryptor::resources::FOLDER_NAME +
                                      auto_trader::signature_encryptor::resources::SLASH + filename;
  std::fstream infile(path_to_file_with_key);
  if (!infile) {
    throw common::exceptions::BaseException("File not found");
  }

  std::string signature;
  std::string token;

  while (!infile.eof()) {
    getline(infile, token);
    signature += token;
  }
  infile.close();

  common::crypto::RsaDecryptor decryptor;
  decryptor.init(resources::PRIVATE_KEY.data());

  auto message_from_file = decryptor.getMessageFromSignature(signature);

  EXPECT_EQ(message, message_from_file);
  std::remove(path_to_file_with_key.data());
}

}  // namespace unit_test
}  // namespace signature_encryptor
}  // namespace auto_trader
