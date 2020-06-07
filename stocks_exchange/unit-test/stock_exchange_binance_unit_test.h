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

#include <istream>

#include "gmock/gmock.h"
#include "include/base_query.h"
#include "include/binance_query.h"
#include "include/query_factory.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

typedef std::pair<std::string, std::string> HTTP_HEADERS;
class FakeBinanceResponse {
 public:
  const std::string getMarketHistoryResponse(
      const BaseQuery<Query>::ConnectionAttributes& host_and_port, Poco::Net::HTTPRequest& request,
      const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "["
        "[\"1499040000000\", "
        "\"7500.00000000\",\"7585.00000000\",\"7450.00000000\",\"6152.12000000\",\"33.39437964\"],"
        "[\"1499040000000\", "
        "\"7560.00000000\",\"7585.00000000\",\"7410.00000000\",\"6251.13000000\",\"26.04646179\"]]";

    return fake_response;
  }

  const std::string getAccountOrder(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                                    Poco::Net::HTTPRequest& request,
                                    const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{"
        "\"symbol\": \"LTCBTC\","
        "\"orderId\": \"1\","
        "\"clientOrderId\": \"myOrder1\","
        "\"price\": \"0.1\","
        "\"origQty\": \"1.0\","
        "\"executedQty\": \"0.0\","
        "\"cummulativeQuoteQty\": \"0.0\","
        "\"status\": \"NEW\","
        "\"timeInForce\": \"GTC\","
        "\"type\": \"LIMIT\","
        "\"side\": \"BUY\","
        "\"stopPrice\": \"0.0\","
        "\"icebergQty\": \"0.0\","
        "\"time\": \"1499827319559\","
        "\"updateTime\": \"1499827319559\","
        "\"isWorking\": \"true\""
        "}";

    return fake_response;
  }
};

class MockBinanceQuery : public BinanceQuery {
 public:
  MOCK_CONST_METHOD3(processHttpRequest,
                     const std::string(const BaseQuery::ConnectionAttributes&,
                                       Poco::Net::HTTPRequest&, const std::vector<HTTP_HEADERS>&));

  void DelegateToMarketHistoryResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeBinanceResponse::getMarketHistoryResponse));
  }

  void DelegateToGetAccountOrderResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBinanceResponse::getAccountOrder));
  }

 private:
  FakeBinanceResponse fake_response_;
};

std::shared_ptr<MockBinanceQuery> mockBinanceQuery;

class MockQueryFactory : public QueryFactory {
 public:
  std::shared_ptr<Query> createQuery(common::StockExchangeType type) { return mockBinanceQuery; }
};

class BinanceQueryFixture : public ::testing::Test {
 public:
  void SetUp() override { mockBinanceQuery.reset(new MockBinanceQuery()); }
  void TearDown() override { mockBinanceQuery.reset(); }
};

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // STOCK_EXCHANGE_BINANCE_UNIT_TEST_H
