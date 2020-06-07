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

#ifndef STOCK_EXCHANGE_BINANCE_UNIT_TEST_H
#define STOCK_EXCHANGE_BINANCE_UNIT_TEST_H

#include <gmock/gmock.h>

#include <istream>
#include <string>

#include "features/include/telegram_announcer.h"
#include "gtest/gtest.h"

namespace auto_trader {
namespace features {
namespace telegram_announcer {
namespace unit_test {

std::stringstream fakeHttpResponse;

class FakeTelegramAnnouncerResponse {
 public:
  std::istream &initAnnouncer(Poco::URI const &uri, const std::string &) const {
    std::string fake_response =
        "{\"ok\":true,"
        "\"result\":"
        "["
        "{"
        "\"update_id\":112233445,"
        "\"message\":"
        "{"
        "\"message_id\":1,"
        "\"from\":{\"id\":222222222,\"is_bot\":true,\"first_name\":\"AutoTrader\",\"username\":"
        "\"test_bot\"},"
        "\"chat\":{\"id\":111111111,\"first_name\":\"Temp\",\"last_name\":\"Temp\",\"type\":"
        "\"private\"},"
        "\"date\":1548360779,"
        "\"text\":\"hello\""
        "}"
        "}"
        "]"
        "}";

    fakeHttpResponse.clear();
    fakeHttpResponse << fake_response;
    return fakeHttpResponse;
  }

  std::istream &getLastMessages(Poco::URI const &uri, const std::string &) const {
    std::string fake_response =
        "{\"ok\":true,"
        "\"result\":"
        "["
        "{"
        "\"update_id\":112233445,"
        "\"message\":"
        "{"
        "\"message_id\":1,"
        "\"from\":{\"id\":222222222,\"is_bot\":true,\"first_name\":\"AutoTrader\",\"username\":"
        "\"test_bot\"},"
        "\"chat\":{\"id\":111111111,\"first_name\":\"Temp\",\"last_name\":\"Temp\",\"type\":"
        "\"private\"},"
        "\"date\":1548360777,"
        "\"text\":\"hello\""
        "}"
        "},"
        "{"
        "\"update_id\":112233446,"
        "\"message\":"
        "{"
        "\"message_id\":2,"
        "\"from\":{\"id\":222222222,\"is_bot\":true,\"first_name\":\"AutoTrader\",\"username\":"
        "\"test_bot\"},"
        "\"chat\":{\"id\":111111111,\"first_name\":\"Temp\",\"last_name\":\"Temp\",\"type\":"
        "\"private\"},"
        "\"date\":1548360778,"
        "\"text\":\"AutoTrader stop\""
        "}"
        "}"
        "]"
        "}";

    fakeHttpResponse.clear();
    fakeHttpResponse << fake_response;
    return fakeHttpResponse;
  }

  std::istream &withoutMessage(Poco::URI const &uri, const std::string &) const {
    std::string fake_response =
        "{\"ok\":true,"
        "\"result\":"
        "["
        "]"
        "}";

    fakeHttpResponse.clear();
    fakeHttpResponse << fake_response;
    return fakeHttpResponse;
  }
};

class MockTelegramAnnouncer : public TelegramAnnouncer {
 public:
  MOCK_CONST_METHOD2(processHttpRequest, std::istream &(Poco::URI const &, const std::string &));

  void DelegateToInitAnnouncerResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeTelegramAnnouncerResponse::initAnnouncer));
  }

  void DelegateToInitAnnouncerResponseWithoutFirstMessageResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeTelegramAnnouncerResponse::withoutMessage));
  }

  void DelegateToGetLastMessagesNotEmptyResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeTelegramAnnouncerResponse::getLastMessages));
  }

  void DelegateToGetLastMessagesEmptyResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeTelegramAnnouncerResponse::withoutMessage));
  }

 private:
  FakeTelegramAnnouncerResponse fake_response_;
};

}  // namespace unit_test
}  // namespace telegram_announcer
}  // namespace features
}  // namespace auto_trader

#endif  // STOCK_EXCHANGE_BINANCE_UNIT_TEST_H
