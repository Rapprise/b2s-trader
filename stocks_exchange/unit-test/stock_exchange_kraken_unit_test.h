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

#ifndef AUTO_TRADER_STOCK_EXCHANGE_KRAKEN_UNIT_TEST_H
#define AUTO_TRADER_STOCK_EXCHANGE_KRAKEN_UNIT_TEST_H

#include <istream>

#include "gmock/gmock.h"
#include "include/kraken_query.h"
#include "include/query_factory.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

std::stringstream fakeKrakenHttpResponse;

class FakeKrakenResponse {
 public:
  std::string cancelOrderResponse(CURL *curl) const {
    std::string fake_response = "{\"error\":[],\"result\":{\"count\":1}}";

    return fake_response;
  }

  std::string getBalanceResponse(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],\"result\":{\"ZUSD\":\"7.8379\",\"XXRP\":\"3.75000000\",\"ADA\":\"6."
        "00034500\"}}";

    return fake_response;
  }

  std::string buyOrderResponse(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],\"result\":{\"descr\":{\"order\":\"buy 3.00000000 ADAUSD @ "
        "market\"},\"txid\":[\"O62OKH-DTOGU-L3AANQ\"]}}";

    return fake_response;
  }

  std::string sellOrderResponse(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],\"result\":{\"descr\":{\"order\":\"sell 30.00000000 XRPUSD @ "
        "market\"},\"txid\":[\"OW33P3-UEC4Z-ZZG3UK\"]}}";

    return fake_response;
  }

  std::string getAccountOpenOrders(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],"
        "\"result\":"
        "{\"open\":"
        "{\"OCX7TI-BWJZD-7MH22K\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"open\",\"opentm\":1566804781.2909,\"starttm\":"
        "0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"sell\",\"ordertype\":\"take-profit\",\"price\":\"1."
        "049542\",\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"sell 2.60000000 ADAUSD @ take "
        "profit 1.049542\",\"close\":\"\"}"
        ",\"vol\":\"2.60000000\",\"vol_exec\":\"0.00000000\",\"cost\":\"0.000000\",\"fee\":\"0."
        "000000\",\"price\":\"0.000000\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"}"
        "}"
        "}}";

    return fake_response;
  }

  std::string getAccountClosedOrders(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],\"result\":"
        "{\"closed\":"
        "{"
        "\"OCX7TI-BWJZD-7MH22K\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"canceled\",\"reason\":\"User "
        "requested\",\"opentm\":1566804781.2909,\"closetm\":1566805797.6595,\"starttm\":0,"
        "\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"sell\",\"ordertype\":\"take-profit\",\"price\":\"1."
        "049542\",\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"sell 2.60000000 ADAUSD @ take "
        "profit 1.049542\",\"close\":\"\"},"
        "\"vol\":\"2.60000000\",\"vol_exec\":\"0.00000000\",\"cost\":\"0.000000\",\"fee\":\"0."
        "000000\",\"price\":\"0.000000\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"ODYFO4-DN52K-ZUMQ3W\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566804578."
        "3422,\"closetm\":1566804578.3495,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"buy\",\"ordertype\":\"limit\",\"price\":\"1.049574\","
        "\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"buy 2.60000000 ADAUSD @ limit "
        "1.049574\",\"close\":\"\"},"
        "\"vol\":\"2.60000000\",\"vol_exec\":\"2.60000000\",\"cost\":\"0.129137\",\"fee\":\"0."
        "000336\",\"price\":\"0.049668\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"OLXBA4-BKKYD-ZOU4DZ\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566804235."
        "2498,\"closetm\":1566804235.2557,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"sell\",\"ordertype\":\"market\",\"price\":\"0\","
        "\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"sell 2.60000000 ADAUSD @ "
        "market\",\"close\":\"\"},"
        "\"vol\":\"2.60000000\",\"vol_exec\":\"2.60000000\",\"cost\":\"0.128890\",\"fee\":\"0."
        "000335\",\"price\":\"0.049573\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"OA2ZSY-G4B7I-WUNXBX\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566803951."
        "4052,\"closetm\":1566803951.4186,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"buy\",\"ordertype\":\"market\",\"price\":\"0\",\"price2\":"
        "\"0\",\"leverage\":\"none\",\"order\":\"buy 2.77834500 ADAUSD @ market\",\"close\":\"\"},"
        "\"vol\":\"2.77834500\",\"vol_exec\":\"2.77834500\",\"cost\":\"0.137892\",\"fee\":\"0."
        "000359\",\"price\":\"0.049631\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"OZUCCZ-V4EAO-OMEO3U\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566803854."
        "685,\"closetm\":1566803854.6913,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"sell\",\"ordertype\":\"market\",\"price\":\"0\","
        "\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"sell 2.77800000 ADAUSD @ "
        "market\",\"close\":\"\"},"
        "\"vol\":\"2.77800000\",\"vol_exec\":\"2.77800000\",\"cost\":\"0.137625\",\"fee\":\"0."
        "000358\",\"price\":\"0.049541\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"O62OKH-DTOGU-L3AANQ\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566803765."
        "6885,\"closetm\":1566803765.6968,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"ADAUSD\",\"type\":\"buy\",\"ordertype\":\"market\",\"price\":\"0\",\"price2\":"
        "\"0\",\"leverage\":\"none\",\"order\":\"buy 3.00000000 ADAUSD @ market\",\"close\":\"\"},"
        "\"vol\":\"3.00000000\",\"vol_exec\":\"3.00000000\",\"cost\":\"0.149079\",\"fee\":\"0."
        "000388\",\"price\":\"0.049693\",\"stopprice\":\"0.000000\",\"limitprice\":\"0.000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"},"
        "\"OW33P3-UEC4Z-ZZG3UK\":"
        "{\"refid\":null,\"userref\":0,\"status\":\"closed\",\"reason\":null,\"opentm\":1566803162."
        "5088,\"closetm\":1566803162.5145,\"starttm\":0,\"expiretm\":0,\"descr\":"
        "{\"pair\":\"XRPUSD\",\"type\":\"sell\",\"ordertype\":\"market\",\"price\":\"0\","
        "\"price2\":\"0\",\"leverage\":\"none\",\"order\":\"sell 30.00000000 XRPUSD @ "
        "market\",\"close\":\"\"},"
        "\"vol\":\"30.00000000\",\"vol_exec\":\"30.00000000\",\"cost\":\"8.16000\",\"fee\":\"0."
        "02121\",\"price\":\"0.27200\",\"stopprice\":\"0.00000000\",\"limitprice\":\"0.00000000\","
        "\"misc\":\"\",\"oflags\":\"fciq\"}"
        "},\"count\":7}"
        "}";

    return fake_response;
  }

  std::string getCurrencyTick(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],"
        "\"result\":{"
        "\"XXRPZUSD\":"
        "{\"a\":[\"0.31699000\",\"2085\",\"2085.000\"],"
        "\"b\":[\"0.31688000\",\"4000\",\"4000.000\"],"
        "\"c\":[\"0.31680000\",\"3480.46873938\"],"
        "\"v\":[\"951374.27230214\",\"3215627.04019437\"],"
        "\"p\":[\"0.31786885\",\"0.31839967\"],"
        "\"t\":[434,1492],"
        "\"l\":[\"0.31637000\",\"0.31637000\"],"
        "\"h\":[\"0.31978000\",\"0.32075000\"],"
        "\"o\":\"0.31913000\"}}}";

    return fake_response;
  }

  std::string getOpenedOrders(CURL *curl) const {
    std::string fake_response =
        "{\"error\":[],"
        "\"result\":{"
        "\"open\":{"
        "\"OHKQFZ-ALIP3-SBVVCF\":{"
        "\"refid\":null,"
        "\"userref\":null,"
        "\"status\":\"open\","
        "\"opentm\":\"1566804781.2909\","
        "\"starttm\":\"0\","
        "\"expiretm\":\"0\","
        "\"descr\":{"
        "\"pair\":\"LTCXBT\","
        "\"type\":\"buy\","
        "\"ordertype\":\"limit\","
        "\"price\":\"0.75000\","
        "\"price2\":\"0\","
        "\"leverage\":\"none\","
        "\"order\":\"buy 2.00000000 LTCXBT @ limit 0.75000\""
        "},"
        "\"vol\":\"2.00000000\","
        "\"vol_exec\":\"0.00000000\","
        "\"cost\":\"0.00000\","
        "\"fee\":\"0.00000\","
        "\"price\":\"0.00000\","
        "\"misc\":\"\","
        "\"oflags\":\"\""
        "},"
        "\"OSJPS5-K5GK2-NDEQBQ\":{"
        "\"refid\":\"null\","
        "\"userref\":\"null\","
        "\"status\":\"open\","
        "\"opentm\":\"1388937079.8802\","
        "\"starttm\":\"0\","
        "\"expiretm\":\"0\","
        "\"descr\":{"
        "\"pair\":\"LTCXBT\","
        "\"type\":\"buy\","
        "\"ordertype\":\"limit\","
        "\"price\":\"1.00000\","
        "\"price2\":\"0\","
        "\"leverage\":\"none\","
        "\"order\":\"buy 1.00000000 LTCXBT @ limit 1.00000\""
        "},"
        "\"vol\":\"1.00000000\","
        "\"vol_exec\":\"0.00000000\","
        "\"cost\":\"0.00000\","
        "\"fee\":\"0.00000\","
        "\"price\":\"0.00000\","
        "\"misc\":\"\","
        "\"oflags\":\"\""
        "}"
        "}"
        "}"
        "}";

    return fake_response;
  }
};

class MockKrakenQuery : public KrakenQuery {
 public:
  MOCK_METHOD1(sendRequest, std::string(CURL *));

  void DelegateToCurrencyTickResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::getCurrencyTick));
  }

  void DelegateToGetOpenedOrdersResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::getOpenedOrders));
  }

  void DelegateToGetBalanceResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::getBalanceResponse));
  }

  void DelegateToBuyOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::buyOrderResponse));
  }

  void DelegateToSellOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::sellOrderResponse));
  }

  void DelegateToCancelOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakeKrakenResponse::cancelOrderResponse));
  }

  void DelegateToGetClosedOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakeKrakenResponse::getAccountClosedOrders));
  }

 private:
  FakeKrakenResponse fake_response_;
};

std::shared_ptr<MockKrakenQuery> mockKrakenQuery;

class MockQueryFactory : public QueryFactory {
 public:
  std::shared_ptr<Query> createQuery(common::StockExchangeType type) { return mockKrakenQuery; }
};

class KrakenQueryFixture : public ::testing::Test {
 public:
  void SetUp() override { mockKrakenQuery.reset(new MockKrakenQuery()); }
  void TearDown() override { mockKrakenQuery.reset(); }
};

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // AUTO_TRADER_STOCK_EXCHANGE_KRAKEN_UNIT_TEST_H
