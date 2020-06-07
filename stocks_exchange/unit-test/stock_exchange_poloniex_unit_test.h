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

#ifndef STOCK_EXCHANGE_POLONIEX_UNIT_TESTS_H
#define STOCK_EXCHANGE_POLONIEX_UNIT_TESTS_H

#include "gmock/gmock.h"
#include "include/base_query.h"
#include "include/poloniex_query.h"
#include "include/query_factory.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

std::stringstream fakePoloniexHttpResponse;

class FakePoloniexResponse {
 public:
  std::string getBalanceResponse(CURL *curl) const {
    std::string fake_response =
        "{\"1CR\":\"0.00000000\","
        "\"ABY\":\"0.00000000\","
        "\"AC\":\"0.00000000\","
        "\"ACH\":\"0.00000000\","
        "\"ADN\":\"0.00000000\","
        "\"AEON\":\"0.00000000\","
        "\"AERO\":\"0.00000000\",\"AIR\":\"0.00000000\",\"AMP\":\"0.00000000\",\"APH\":\"0."
        "00000000\","
        "\"ARCH\":\"0.00000000\",\"ARDR\":\"0.00000000\",\"ATOM\":\"0.00000000\",\"AUR\":\"0."
        "00000000\","
        "\"AXIS\":\"0.00000000\",\"BALLS\":\"0.00000000\",\"BANK\":\"0.00000000\",\"BAT\":\"0."
        "00000000\","
        "\"BBL\":\"0.00000000\",\"BBR\":\"0.00000000\",\"BCC\":\"0.00000000\",\"BCH\":\"0."
        "00000000\","
        "\"BCHABC\":\"0.00000000\",\"BCHSV\":\"0.00000000\",\"BCN\":\"0.00000000\",\"BCY\":\"0."
        "00000000\","
        "\"BDC\":\"0.00000000\",\"BDG\":\"0.00000000\",\"BELA\":\"0.00000000\",\"BITCNY\":\"0."
        "00000000\","
        "\"BITS\":\"0.00000000\",\"BITUSD\":\"0.00000000\",\"BLK\":\"0.00000000\",\"BLOCK\":\"0."
        "00000000\","
        "\"BLU\":\"0.00000000\",\"BNS\":\"0.00000000\",\"BNT\":\"0.00000000\",\"BONES\":\"0."
        "00000000\","
        "\"BOST\":\"0.00000000\",\"BTC\":\"0.00000000\",\"BTCD\":\"0.00000000\",\"BTCS\":\"0."
        "00000000\","
        "\"BTM\":\"0.00000000\",\"BTS\":\"0.00000000\",\"BURN\":\"0.00000000\",\"BURST\":\"0."
        "00000000\","
        "\"C2\":\"0.00000000\",\"CACH\":\"0.00000000\",\"CAI\":\"0.00000000\",\"CC\":\"0."
        "00000000\","
        "\"CCN\":\"0.00000000\",\"CGA\":\"0.00000000\",\"CHA\":\"0.00000000\",\"CINNI\":\"0."
        "00000000\","
        "\"CLAM\":\"0.00000000\",\"CNL\":\"0.00000000\",\"CNMT\":\"0.00000000\",\"CNOTE\":\"0."
        "00000000\","
        "\"COMM\":\"0.00000000\",\"CON\":\"0.00000000\",\"CORG\":\"0.00000000\",\"CRYPT\":\"0."
        "00000000\","
        "\"CURE\":\"0.00000000\",\"CVC\":\"0.00000000\",\"CYC\":\"0.00000000\",\"DAO\":\"0."
        "00000000\","
        "\"DASH\":\"0.00000000\",\"DCR\":\"0.00000000\",\"DGB\":\"0.00000000\",\"DICE\":\"0."
        "00000000\","
        "\"DIEM\":\"0.00000000\",\"DIME\":\"0.00000000\",\"DIS\":\"0.00000000\",\"DNS\":\"0."
        "00000000\","
        "\"DOGE\":\"0.00000000\",\"DRKC\":\"0.00000000\",\"DRM\":\"0.00000000\",\"DSH\":\"0."
        "00000000\","
        "\"DVK\":\"0.00000000\",\"EAC\":\"0.00000000\",\"EBT\":\"0.00000000\",\"ECC\":\"0."
        "00000000\","
        "\"EFL\":\"0.00000000\",\"EMC2\":\"0.00000000\",\"EMO\":\"0.00000000\",\"ENC\":\"0."
        "00000000\","
        "\"EOS\":\"0.00000000\",\"ETC\":\"0.00000000\",\"ETH\":\"0.00000000\",\"eTOK\":\"0."
        "00000000\","
        "\"EXE\":\"0.00000000\",\"EXP\":\"0.00000000\",\"FAC\":\"0.00000000\",\"FCN\":\"0."
        "00000000\","
        "\"FCT\":\"0.00000000\",\"FIBRE\":\"0.00000000\",\"FLAP\":\"0.00000000\",\"FLDC\":\"0."
        "00000000\","
        "\"FLO\":\"0.00000000\",\"FLT\":\"0.00000000\",\"FOAM\":\"0.00000000\",\"FOX\":\"0."
        "00000000\","
        "\"FRAC\":\"0.00000000\",\"FRK\":\"0.00000000\",\"FRQ\":\"0.00000000\",\"FVZ\":\"0."
        "00000000\","
        "\"FZ\":\"0.00000000\",\"FZN\":\"0.00000000\",\"GAME\":\"0.00000000\",\"GAP\":\"0."
        "00000000\","
        "\"GAS\":\"0.00000000\",\"GDN\":\"0.00000000\",\"GEMZ\":\"0.00000000\",\"GEO\":\"0."
        "00000000\","
        "\"GIAR\":\"0.00000000\",\"GLB\":\"0.00000000\",\"GML\":\"0.00000000\",\"GNO\":\"0."
        "00000000\","
        "\"GNS\":\"0.00000000\",\"GNT\":\"0.00000000\",\"GOLD\":\"0.00000000\",\"GPC\":\"0."
        "00000000\","
        "\"GPUC\":\"0.00000000\",\"GRC\":\"0.00000000\",\"GRCX\":\"0.00000000\",\"GRIN\":\"0."
        "00000000\","
        "\"GRS\":\"0.00000000\",\"GUE\":\"0.00000000\",\"H2O\":\"0.00000000\",\"HIRO\":\"0."
        "00000000\","
        "\"HOT\":\"0.00000000\",\"HUC\":\"0.00000000\",\"HUGE\":\"0.00000000\",\"HVC\":\"0."
        "00000000\","
        "\"HYP\":\"0.00000000\",\"HZ\":\"0.00000000\",\"IFC\":\"0.00000000\",\"INDEX\":\"0."
        "00000000\","
        "\"IOC\":\"0.00000000\",\"ITC\":\"0.00000000\",\"IXC\":\"0.00000000\",\"JLH\":\"0."
        "00000000\","
        "\"JPC\":\"0.00000000\",\"JUG\":\"0.00000000\",\"KDC\":\"0.00000000\",\"KEY\":\"0."
        "00000000\","
        "\"KNC\":\"0.00000000\",\"LBC\":\"0.00000000\",\"LC\":\"0.00000000\",\"LCL\":\"0."
        "00000000\","
        "\"LEAF\":\"0.00000000\",\"LGC\":\"0.00000000\",\"LOL\":\"0.00000000\",\"LOOM\":\"0."
        "00000000\","
        "\"LOVE\":\"0.00000000\",\"LPT\":\"0.00000000\",\"LQD\":\"0.00000000\",\"LSK\":\"0."
        "00000000\","
        "\"LTBC\":\"0.00000000\",\"LTC\":\"0.00000000\",\"LTCX\":\"0.00000000\",\"MAID\":\"0."
        "00000000\","
        "\"MANA\":\"0.00000000\",\"MAST\":\"0.00000000\",\"MAX\":\"0.00000000\",\"MCN\":\"0."
        "00000000\","
        "\"MEC\":\"0.00000000\",\"METH\":\"0.00000000\",\"MIL\":\"0.00000000\",\"MIN\":\"0."
        "00000000\","
        "\"MINT\":\"0.00000000\",\"MMC\":\"0.00000000\",\"MMNXT\":\"0.00000000\",\"MMXIV\":\"0."
        "00000000\","
        "\"MNTA\":\"0.00000000\",\"MON\":\"0.00000000\",\"MRC\":\"0.00000000\",\"MRS\":\"0."
        "00000000\","
        "\"MTS\":\"0.00000000\",\"MUN\":\"0.00000000\",\"MYR\":\"0.00000000\",\"MZC\":\"0."
        "00000000\","
        "\"N5X\":\"0.00000000\",\"NAS\":\"0.00000000\",\"NAUT\":\"0.00000000\",\"NAV\":\"0."
        "00000000\","
        "\"NBT\":\"0.00000000\",\"NEOS\":\"0.00000000\",\"NL\":\"0.00000000\",\"NMC\":\"0."
        "00000000\","
        "\"NMR\":\"0.00000000\",\"NOBL\":\"0.00000000\",\"NOTE\":\"0.00000000\",\"NOXT\":\"0."
        "00000000\","
        "\"NRS\":\"0.00000000\",\"NSR\":\"0.00000000\",\"NTX\":\"0.00000000\",\"NXC\":\"0."
        "00000000\","
        "\"NXT\":\"0.00000000\",\"NXTI\":\"0.00000000\",\"OMG\":\"0.00000000\",\"OMNI\":\"0."
        "00000000\","
        "\"OPAL\":\"0.00000000\",\"PAND\":\"0.00000000\",\"PASC\":\"0.00000000\",\"PAWN\":\"0."
        "00000000\","
        "\"PIGGY\":\"0.00000000\",\"PINK\":\"0.00000000\",\"PLX\":\"0.00000000\",\"PMC\":\"0."
        "00000000\","
        "\"POLY\":\"0.00000000\",\"POT\":\"0.00000000\",\"PPC\":\"0.00000000\",\"PRC\":\"0."
        "00000000\","
        "\"PRT\":\"0.00000000\",\"PTS\":\"0.00000000\",\"Q2C\":\"0.00000000\",\"QBK\":\"0."
        "00000000\","
        "\"QCN\":\"0.00000000\",\"QORA\":\"0.00000000\",\"QTL\":\"0.00000000\",\"QTUM\":\"0."
        "00000000\","
        "\"RADS\":\"0.00000000\",\"RBY\":\"0.00000000\",\"RDD\":\"0.00000000\",\"REP\":\"0."
        "00000000\","
        "\"RIC\":\"0.00000000\",\"RZR\":\"0.00000000\",\"SBD\":\"0.00000000\",\"SC\":\"0."
        "00000000\","
        "\"SDC\":\"0.00000000\",\"SHIBE\":\"0.00000000\",\"SHOPX\":\"0.00000000\",\"SILK\":\"0."
        "00000000\","
        "\"SJCX\":\"0.00000000\",\"SLR\":\"0.00000000\",\"SMC\":\"0.00000000\",\"SNT\":\"0."
        "00000000\","
        "\"SOC\":\"0.00000000\",\"SPA\":\"0.00000000\",\"SQL\":\"0.00000000\",\"SRCC\":\"0."
        "00000000\","
        "\"SRG\":\"0.00000000\",\"SSD\":\"0.00000000\",\"STEEM\":\"0.00000000\",\"STORJ\":\"0."
        "00000000\","
        "\"STR\":\"0.00000000\",\"STRAT\":\"0.00000000\",\"SUM\":\"0.00000000\",\"SUN\":\"0."
        "00000000\","
        "\"SWARM\":\"0.00000000\",\"SXC\":\"0.00000000\",\"SYNC\":\"0.00000000\",\"SYS\":\"0."
        "00000000\","
        "\"TAC\":\"0.00000000\",\"TOR\":\"0.00000000\",\"TRUST\":\"0.00000000\",\"TWE\":\"0."
        "00000000\","
        "\"UIS\":\"0.00000000\",\"ULTC\":\"0.00000000\",\"UNITY\":\"0.00000000\",\"URO\":\"0."
        "00000000\","
        "\"USDC\":\"0.00000000\",\"USDE\":\"0.00000000\",\"USDT\":\"0.00000000\",\"UTC\":\"0."
        "00000000\","
        "\"UTIL\":\"0.00000000\",\"UVC\":\"0.00000000\",\"VIA\":\"0.00000000\",\"VOOT\":\"0."
        "00000000\","
        "\"VOX\":\"0.00000000\",\"VRC\":\"0.00000000\",\"VTC\":\"0.00000000\",\"WC\":\"0."
        "00000000\","
        "\"WDC\":\"0.00000000\",\"WIKI\":\"0.00000000\",\"WOLF\":\"0.00000000\",\"X13\":\"0."
        "00000000\","
        "\"XAI\":\"0.00000000\",\"XAP\":\"0.00000000\",\"XBC\":\"0.00000000\",\"XC\":\"0."
        "00000000\","
        "\"XCH\":\"0.00000000\",\"XCN\":\"0.00000000\",\"XCP\":\"0.00000000\",\"XCR\":\"0."
        "00000000\","
        "\"XDN\":\"0.00000000\",\"XDP\":\"0.00000000\",\"XEM\":\"0.00000000\",\"XHC\":\"0."
        "00000000\","
        "\"XLB\":\"0.00000000\",\"XMG\":\"0.00000000\",\"XMR\":\"0.00000000\",\"XPB\":\"0."
        "00000000\","
        "\"XPM\":\"0.00000000\",\"XRP\":\"10\",\"XSI\":\"0.00000000\",\"XST\":\"0.00000000\","
        "\"XSV\":\"0.00000000\",\"XUSD\":\"0.00000000\",\"XVC\":\"0.00000000\",\"XXC\":\"0."
        "00000000\","
        "\"YACC\":\"0.00000000\",\"YANG\":\"0.00000000\",\"YC\":\"0.00000000\",\"YIN\":\"0."
        "00000000\","
        "\"ZEC\":\"0.00000000\",\"ZRX\":\"0.00000000\"}";

    return fake_response;
  }

  std::string buyOrderResponse(CURL *curl) const {
    std::string fake_response =
        "{\"orderNumber\":\"514845991795\","
        "\"resultingTrades\":["
        "{\"amount\":\"3.0\","
        "\"date\":\"2018-10-25 23:03:21\","
        "\"rate\":\"0.0002\","
        "\"total\":\"0.0006\","
        "\"tradeID\":\"251834\","
        "\"type\":\"buy\"}],"
        "\"fee\":\"0.01000000\","
        "\"clientOrderId\":\"12345\","
        "\"currencyPair\":\"BTC_ETH\"}";

    return fake_response;
  }

  std::string sellOrderResponse(CURL *curl) const {
    std::string fake_response =
        "{\"orderNumber\":\"539765738972\","
        "\"resultingTrades\":["
        "{\"amount\":\"3.0\","
        "\"date\":\"2018-10-25 23:03:21\","
        "\"rate\":\"0.0002\","
        "\"total\":\"0.0006\","
        "\"tradeID\":\"251834\","
        "\"type\":\"sell\"}],"
        "\"fee\":\"0.01000000\","
        "\"clientOrderId\":\"12345\","
        "\"currencyPair\":\"BTC_ETH\"}";

    return fake_response;
  }

  std::string cancelOrderResponse(CURL *curl) const {
    std::string fake_response =
        "{\"success\": \"1\","
        "\"amount\": \"50.00000000\","
        "\"clientOrderId\":\"12345\","
        "\"message\":\"Order #514845991795 canceled.\"}";

    return fake_response;
  }

  std::string getAccountOpenOrdersResponse(CURL *curl) const {
    std::string fake_response =
        "{ "
        "\"BTC_ARDR\": [],"
        "\"BTC_BAT\": [],"
        "\"BTC_BCH\": [],"
        "\"BTC_ETH\": "
        "[ "
        "{ "
        "\"orderNumber\": \"514515459658\","
        "\"type\": \"buy\","
        "\"rate\": \"0.00001000\","
        "\"startingAmount\": \"100.00000000\","
        "\"amount\": \"100.00000000\","
        "\"total\": \"0.00100000\","
        "\"date\": \"2018-10-23 17:41:15\","
        "\"margin\": \"0\""
        "},"
        "{"
        "\"orderNumber\": \"514515389728\","
        "\"type\": \"buy\","
        "\"rate\": \"0.00003000\","
        "\"startingAmount\": \"100.00000000\","
        "\"amount\": \"100.00000000\","
        "\"total\": \"0.00300000\","
        "\"date\": \"2018-10-23 17:40:55\","
        "\"margin\": \"0\" "
        "}"
        "],"
        "\"BTC_FCT\": [],"
        "\"BTC_SC\":"
        "["
        "{"
        "\"orderNumber\": \"26422960740\","
        "\"type\": \"buy\","
        "\"rate\": \"0.00000001\","
        "\"startingAmount\": \"100000.00000000\","
        "\"amount\": \"100000.00000000\","
        "\"total\": \"0.00100000\","
        "\"date\": \"2018-10-23 17:41:49\","
        "\"margin\": \"0\""
        "},"
        "{"
        "\"orderNumber\": \"26422963737\","
        "\"type\": \"buy\","
        "\"rate\": \"0.00000002\","
        "\"startingAmount\": \"100000.00000000\","
        "\"amount\": \"100000.00000000\","
        "\"total\": \"0.00200000\","
        "\"date\": \"2018-10-23 17:42:00\","
        "\"margin\": \"0\""
        "}"
        "],"
        "\"BTC_SNT\": []"
        "}";

    return fake_response;
  }
};

class MockPoloniexQuery : public PoloniexQuery {
 public:
  MOCK_METHOD1(sendRequest, std::string(CURL *));

  void DelegateToGetBalanceResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakePoloniexResponse::getBalanceResponse));
  }

  void DelegateToBuyOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakePoloniexResponse::buyOrderResponse));
  }

  void DelegateToSellOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(testing::Invoke(&fake_response_, &FakePoloniexResponse::sellOrderResponse));
  }

  void DelegateToCancelOrderResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakePoloniexResponse::cancelOrderResponse));
  }

  void DelegateToGetAccountOpenOrdersResponse() {
    ON_CALL(*this, sendRequest(::testing::_))
        .WillByDefault(
            testing::Invoke(&fake_response_, &FakePoloniexResponse::getAccountOpenOrdersResponse));
  }

 private:
  FakePoloniexResponse fake_response_;
};

std::shared_ptr<MockPoloniexQuery> mockPoloniexQuery;

class MockQueryFactory : public QueryFactory {
 public:
  std::shared_ptr<Query> createQuery(common::StockExchangeType type) { return mockPoloniexQuery; }
};

class PoloniexQueryFixture : public ::testing::Test {
 public:
  void SetUp() override { mockPoloniexQuery.reset(new MockPoloniexQuery()); }
  void TearDown() override { mockPoloniexQuery.reset(); }
};

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader

#endif  // STOCK_EXCHANGE_POLONIEX_UNIT_TESTS_H
