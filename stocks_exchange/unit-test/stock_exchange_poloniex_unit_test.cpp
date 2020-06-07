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

#include "stock_exchange_poloniex_unit_test.h"

#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/utils.h"
#include "gtest/gtest.h"
#include "include/query_processor.h"
#include "include/stock_exchange_library.h"
#include "resources/resources.h"

const std::string api_key = "WTGYTGL4-BBR83HR5-9XS4FSW3-TVB7QIDC";
const std::string secret_key =
    "32e8d59accf9beda54a4fa031c7200bf768ae394bcf88d11f44ce166dde47063713e84fb87447cdd1c4b36d2cf75d2"
    "c13d7a1b50e5eb2c6d69d6df1cc548c65c";

const std::string buy_order_number = "514845991795";
const std::string sell_order_number = "539765738972";
const double fake_balance_xrp_response = 10.0;

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

TEST(Poloniex, PoloniexRealRequest_Poloniex_Currencies_On_Valid_Pairs_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  common::PoloniexCurrency poloniexCurrencies;
  auto base_currencies = poloniexCurrencies.getBaseCurrencies();
  for (const auto& currency : base_currencies) {
    auto traded_currencies = poloniexCurrencies.getTradedCurrencies(currency);
    for (const auto& traded_currency : traded_currencies) {
      EXPECT_NO_THROW(query->getCurrencyTick(currency, traded_currency));
    }
  }
}

TEST(Poloniex, getMarketHistory_ValidData) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  auto marketHistory = query->getMarketHistory(common::Currency::BTC, common::Currency::ETH,
                                               common::TickInterval::FIFTEEN_MIN);

  EXPECT_TRUE(marketHistory->marketData_.size() > 0);
}

TEST(Poloniex, getMarketHistory_ExpectThrow) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  EXPECT_THROW(query->getMarketHistory(common::Currency::BTC, common::Currency::BTC,
                                       common::TickInterval::FIFTEEN_MIN),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, getCurrencyTick) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  auto tick = query->getCurrencyTick(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(tick.ask_ > 0);
  EXPECT_TRUE(tick.bid_ > 0);
  EXPECT_TRUE(tick.ask_ > tick.bid_);

  EXPECT_TRUE(tick.fromCurrency_ == common::Currency::BTC);
  EXPECT_TRUE(tick.toCurrency_ == common::Currency::ETH);
}

TEST(Poloniex, getCurrencyTick_ExpectThrow) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  EXPECT_THROW(query->getCurrencyTick(common::Currency::BTC, common::Currency::BTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, getMarketOpenOrders_ValidData) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  auto marketOpenOrders = query->getMarketOpenOrders(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(marketOpenOrders.size() > 0);
}

TEST(Poloniex, getMarketOpenOrders_ExpectThrow) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  EXPECT_THROW(query->getMarketOpenOrders(common::Currency::BTC, common::Currency::BTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, getBalance_nullBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  auto balance = query->getBalance(common::Currency::USDC);
  EXPECT_EQ(balance, 0);
}

TEST(Poloniex, getBalance_badRequest) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  EXPECT_THROW(query->getBalance(common::Currency::UNKNOWN),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, sellOrder_withNoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  EXPECT_THROW(query->sellOrder(common::Currency::BTC, common::Currency::ETH, 1, 1),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, buyOrder_withNoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  EXPECT_THROW(query->buyOrder(common::Currency::BTC, common::Currency::ETH, 1, 1),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, accountOpenOrders_noOpenedOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  auto openOrders = query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);
  EXPECT_EQ(openOrders.size(), 0);
}

TEST(Poloniex, cancelOrder_badUuid) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  EXPECT_THROW(query->cancelOrder(common::Currency::BTC, common::Currency::ETH, "1234536567"),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Poloniex, getAccountOrder) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Poloniex);

  query->updateApiKey(api_key);
  query->updateSecretKey(secret_key);

  auto marketOrder =
      query->getAccountOrder(common::Currency::BTC, common::Currency::ETH, "123213123");
  EXPECT_EQ(marketOrder.uuid_, "123213123");
  EXPECT_EQ(marketOrder.isCanceled_, true);
}

TEST_F(PoloniexQueryFixture, PoloniexQueryFixture_buyOrder_response_Test) {
  mockPoloniexQuery->DelegateToBuyOrderResponse();

  EXPECT_CALL(*mockPoloniexQuery, sendRequest(testing::_)).Times(1);

  auto buy_order = mockPoloniexQuery->buyOrder(common::Currency::BTC, common::Currency::ETH, 1, 1);

  EXPECT_EQ(buy_order.uuid_, buy_order_number);
}

TEST_F(PoloniexQueryFixture, PoloniexQueryFixture_sellOrder_response_Test) {
  mockPoloniexQuery->DelegateToSellOrderResponse();

  EXPECT_CALL(*mockPoloniexQuery, sendRequest(testing::_)).Times(1);

  auto sell_order = mockPoloniexQuery->buyOrder(common::Currency::BTC, common::Currency::ETH, 1, 1);

  EXPECT_EQ(sell_order.uuid_, sell_order_number);
}

TEST_F(PoloniexQueryFixture, PoloniexQueryFixture_getBalance_response_Test) {
  mockPoloniexQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockPoloniexQuery, sendRequest(testing::_)).Times(1);

  auto balance = mockPoloniexQuery->getBalance(common::Currency::XRP);

  EXPECT_EQ(balance, fake_balance_xrp_response);
}

TEST_F(PoloniexQueryFixture, PoloniexQueryFixture_cancelOrder_response_Test) {
  mockPoloniexQuery->DelegateToCancelOrderResponse();

  EXPECT_CALL(*mockPoloniexQuery, sendRequest(testing::_)).Times(1);

  auto isCanceled = mockPoloniexQuery->cancelOrder(common::Currency::BTC, common::Currency::XRP,
                                                   "fake_order_uuid");

  EXPECT_EQ(isCanceled, true);
}

TEST_F(PoloniexQueryFixture, PoloniexQueryFixture_getAccountOpenOrders_response_Test) {
  mockPoloniexQuery->DelegateToGetAccountOpenOrdersResponse();

  EXPECT_CALL(*mockPoloniexQuery, sendRequest(testing::_)).Times(1);

  auto accountOpenOrders =
      mockPoloniexQuery->getAccountOpenOrders(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(accountOpenOrders.size() > 0);
}

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader