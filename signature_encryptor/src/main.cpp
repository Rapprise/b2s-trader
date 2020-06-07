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

#include <fstream>
#include <iostream>

#include "common/rsa_encryption/rsa_encryptor.h"
#include "resources/resources.h"

// 1: Name
// 2: Surname
// 3: mac address
// 4: date
// 5: time
// 6: account id
//"First Name Last Name 04:92:26:07:da:0b 01.06.2019 09:44:13"
int main(int argc, char** argv) {
  if (argc < 7) {
    std::cerr << "Error. Invalid arguments count.\n";
    return 1;
  }

  std::vector<std::string> commandLineArguments;
  commandLineArguments.reserve(7);
  for (int i = 1; i < argc; ++i) commandLineArguments.emplace_back(argv[i]);

  std::string encryptionFormt;
  for (const auto& argument : commandLineArguments) {
    encryptionFormt += argument + "_";
  }

  encryptionFormt.pop_back();

  auto_trader::common::crypto::RsaEncryptor encryptor;
  encryptor.init(auto_trader::signature_encryptor::resources::PUBLIC_KEY.data());
  auto signature = encryptor.generateSignature(encryptionFormt);

  std::string filename = auto_trader::signature_encryptor::resources::FOLDER_NAME +
                         auto_trader::signature_encryptor::resources::SLASH +
                         commandLineArguments.at(0) + "_" + commandLineArguments.at(1) + "_" +
                         commandLineArguments.at(3) + "_" + commandLineArguments.at(5);

#ifdef WIN32
  CreateDirectory(auto_trader::signature_encryptor::resources::FOLDER_NAME.c_str(), NULL);
#elif __unix__ || __APPLE__
  std::string makeDirrectoryStr =
      "mkdir -p " + auto_trader::signature_encryptor::resources::FOLDER_NAME;
  system(makeDirrectoryStr.data());
#endif

  std::ofstream fout(filename, std::ofstream::out | std::ofstream::trunc);
  if (!fout) {
    std::cerr << "Error. File with license key could not be created.\n";
    return 1;
  }

  fout << signature;

  return 0;
}