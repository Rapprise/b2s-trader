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

#ifndef AUTO_TRADER_FUTURE_LICENSE_UNIT_TEST_H
#define AUTO_TRADER_FUTURE_LICENSE_UNIT_TEST_H

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <vector>

#include "features/include/license.h"

namespace auto_trader {
namespace features {
namespace license {
namespace unit_test {

class FakeLicenseResponse {
 public:
  std::vector<std::string> getMacAddresses() { return {"04:92:26:07:da:0b"}; }
};

class MockLicenseObject : public License {
 public:
  MOCK_METHOD0(getMacAddresses, std::vector<std::string>());

  void DelegateToFakeMacAddressAndLicenseFilename() {
    ON_CALL(*this, getMacAddresses())
        .WillByDefault(testing::Invoke(&fake_response_, &FakeLicenseResponse::getMacAddresses));
  }

 private:
  FakeLicenseResponse fake_response_;
};

std::shared_ptr<MockLicenseObject> mockLincenseObject;

class LicenseFixture : public ::testing::Test {
 public:
  void SetUp() override { mockLincenseObject.reset(new MockLicenseObject()); }
  void TearDown() override { mockLincenseObject.reset(); }
};

}  // namespace unit_test
}  // namespace license
}  // namespace features
}  // namespace auto_trader

#endif  // AUTO_TRADER_FUTURE_LICENSE_UNIT_TEST_H
