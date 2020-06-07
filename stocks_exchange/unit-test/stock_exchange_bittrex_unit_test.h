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

#ifndef STOCKS_EXCHANGE_BITTREX_UNIT_TEST_H_
#define STOCKS_EXCHANGE_BITTREX_UNIT_TEST_H_

#include <istream>

#include "gmock/gmock.h"
#include "include/base_query.h"
#include "include/bittrex_query.h"
#include "include/query_factory.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

std::stringstream fakeHttpResponse;

typedef std::pair<std::string, std::string> HTTP_HEADERS;
class FakeBittrexResponse {
 public:
  const std::string getMarketHistoryResponse(
      const BaseQuery<Query>::ConnectionAttributes& host_and_port, Poco::Net::HTTPRequest& request,
      const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{\"success\":true,"
        "\"message\":\"\",\""
        "result\":["
        "{\"O\":7500.00000000,\"H\":7585.00000000,\"L\":7450.00000000,\"C\":6152.12000000,\"V\":33."
        "39437964,\"T\":\"2018-05-31T00:00:00\",\"BV\":252267.84804724},"
        "{\"O\":7560.00000000,\"H\":7585.00000000,\"L\":7410.00000000,\"C\":6251.13000000,\"V\":26."
        "04646179,\"T\":\"2018-06-01T00:00:00\",\"BV\":195530.84763870}],"
        "\"explanation\":null}";

    return fake_response;
  }

  const std::string buyOrderResponse(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                                     Poco::Net::HTTPRequest& request,
                                     const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{\n"
        "\t\"success\" : true,\n"
        "\t\"message\" : \"\",\n"
        "\t\"result\" : {\n"
        "\t\t\t\"uuid\" : \"614c34e4-8d71-11e3-94b5-425861b86ab6\"\n"
        "\t\t}\n"
        "}";

    return fake_response;
  }

  const std::string sellOrderResponse(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                                      Poco::Net::HTTPRequest& request,
                                      const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{\n"
        "\t\"success\" : true,\n"
        "\t\"message\" : \"\",\n"
        "\t\"result\" : {\n"
        "\t\t\t\"uuid\" : \"614c34e4-8d71-11e3-94b5-425861b86ab6\"\n"
        "\t\t}\n"
        "}";

    return fake_response;
  }

  const std::string cancelOrderResponse(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                                        Poco::Net::HTTPRequest& request,
                                        const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{\n"
        "    \"success\" : true,\n"
        "    \"message\" : \"\",\n"
        "    \"result\" : null\n"
        "}";

    return fake_response;
  }

  const std::string getBalance(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                               Poco::Net::HTTPRequest& request,
                               const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{"
        "\"success\": \"true\","
        "\"message\": \"\","
        "\"result\": ["
        "{"
        "\"Currency\": \"DOGE\","
        "\"Balance\": \"4.21549076\","
        "\"Available\": \"4.21549076\","
        "\"Pending\": \"0\","
        "\"CryptoAddress\": \"DLxcEt3AatMyr2NTatzjsfHNoB9NT62HiF\","
        "\"Requested\": \"false\","
        "\"Uuid\": \"null\""
        "}"
        "]"
        "}";

    return fake_response;
  }

  const std::string getAccountOrder(const BaseQuery<Query>::ConnectionAttributes& host_and_port,
                                    Poco::Net::HTTPRequest& request,
                                    const std::vector<HTTP_HEADERS>& headers) const {
    std::string fake_response =
        "{"
        "\"success\": \"true\","
        "\"message\": \"\","
        "\"result\": "
        "{"
        "\"Uuid\": \"string (uuid)\","
        "\"OrderUuid\": \"8925d746-bc9f-4684-b1aa-e507467aaa99\","
        "\"Exchange\": \"BTC-LTC\","
        "\"Type\": \"LIMIT_BUY\","
        "\"Quantity\": \"100000\","
        "\"QuantityRemaining\": \"100000\","
        "\"Limit\": \"1e-8\","
        "\"CommissionPaid\": \"0\","
        "\"Price\": \"12340\","
        "\"PricePerUnit\": \"null\","
        "\"Opened\": \"2014-07-09T03:55:48.583\","
        "\"Closed\": \"null\","
        "\"CancelInitiated\": \"boolean\","
        "\"ImmediateOrCancel\": \"boolean\","
        "\"IsConditional\": \"boolean\""
        "}"
        "}";

    return fake_response;
  }
};

class MockBittrexQuery : public BittrexQuery {
 public:
  MOCK_CONST_METHOD3(processHttpRequest,
                     const std::string(const BaseQuery::ConnectionAttributes&,
                                       Poco::Net::HTTPRequest&, const std::vector<HTTP_HEADERS>&));

  void DelegateToMarketHistoryResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeBittrexResponse::getMarketHistoryResponse));
  }

  void DelegateToSellResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBittrexResponse::sellOrderResponse));
  }

  void DelegateToBuyResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBittrexResponse::buyOrderResponse));
  }

  void DelegateToCancelResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBittrexResponse::cancelOrderResponse));
  }

  void DelegateToGetBalanceResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBittrexResponse::getBalance));
  }

  void DelegateToGetAccountOrderResponse() {
    ON_CALL(*this, processHttpRequest(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeBittrexResponse::getAccountOrder));
  }

 private:
  FakeBittrexResponse fake_response_;
};

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // STOCKS_EXCHANGE_BITTREX_UNIT_TEST_H_
