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

#ifndef STOCK_EXCHANGE_HUOBI_UNIT_TESTS_H
#define STOCK_EXCHANGE_HUOBI_UNIT_TESTS_H

#include "gmock/gmock.h"
#include "include/base_query.h"
#include "include/huobi_query.h"
#include "include/query_factory.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

class FakeHuobiResponse {
 public:
  std::string getAccountOrderResponse(CURL *curl) const {
    std::string fakeResponse =
        "{"
        "\"status\" : \"ok\","
        "\"data\" : {"
        "\"id\": 59378,"
        "\"symbol\" : \"ethusdt\","
        "\"account-id\" : 100009,"
        "\"amount\" : \"43\","
        "\"price\" : \"100.23\","
        "\"created-at\" : 1494901162595,"
        "\"type\" : \"buy-limit\","
        "\"field-amount\" : \"10.1000000000\","
        "\"field-cash-amount\" : \"1011.0100000000\","
        "\"field-fees\" : \"0.0202000000\","
        "\"finished-at\" : 1494901400468,"
        "\"user-id\" : 1000,"
        "\"source\" : \"api\","
        "\"state\" : \"filled\","
        "\"canceled-at\" : 0"
        "}"
        "}";

    return fakeResponse;
  }

  std::string getMarketHistoryResponse(CURL *curl) const {
    std::string fakeResponse =
        "{ "
        "\"status\":\"ok\","
        "\"ch\" : \"market.btcusdt.kline.1day\","
        "\"ts\" : 1576784462342,"
        "\"data\" : ["
        "{"
        "\"amount\":3107.884503344746045473,"
        "\"open\" : 7144.00,"
        "\"close\" : 7160.00,"
        "\"high\" : 7175.00,"
        "\"id\" : 1576771200,"
        "\"count\" : 29347,"
        "\"low\" : 7111.00,"
        "\"vol\" : 22209664.502456653123197959100000000000000000"
        "},"
        "{"
        "\"id\":1576684800,"
        "\"open\" : 6823.00,"
        "\"close\" : 7144.00,"
        "\"high\" : 7437.00,"
        "\"low\" : 6771.00,"
        "\"vol\" : 472502139.433867268685217764,"
        "\"amount\" : 66307.324460334469507699,"
        "\"count\" : 577186"
        "}"
        "]"
        "}";

    return fakeResponse;
  }
};

class MockHuobiQuery : public HuobiQuery {
 public:
  MOCK_CONST_METHOD1(sendRequest, std::string(CURL *));

  void DelegateToMarketHistoryResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeHuobiResponse::getMarketHistoryResponse));
  }

  void DelegateToGetAccountOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeHuobiResponse::getAccountOrderResponse));
  }

 private:
  FakeHuobiResponse fake_response_;
};

std::shared_ptr<MockHuobiQuery> mockHuobiQuery;

class MockQueryFactory : public QueryFactory {
 public:
  std::shared_ptr<Query> createQuery(common::StockExchangeType type) { return mockHuobiQuery; }
};

class HuobiQueryFixture : public ::testing::Test {
 public:
  void SetUp() override { mockHuobiQuery.reset(new MockHuobiQuery()); }
  void TearDown() override { mockHuobiQuery.reset(); }
};

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // STOCK_EXCHANGE_HUOBI_UNIT_TESTS_H
