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

#include "stock_exchange_kraken_unit_test.h"

#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/utils.h"
#include "gtest/gtest.h"
#include "include/query_processor.h"
#include "include/stock_exchange_library.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

const std::string kraken_private =
    "d5IeAreqJbSoZohpJPiXFh4LWY9O7t/2E43bp7ehIe3OwTIt8BKni1P9xojPshhgtYLLZ+Mpg+uHsAapUfqlbw==";
const std::string kraken_api_key = "TVCtFQP83n14APh9XW+M+PnctIog52q6XsqIp10PnTp9qRUfNvmEhMvB";

const std::string invalidOrderMessage = "Invalid response exception raised : EOrder:Invalid order";
const std::string errorWithoutTradedBalance =
    "Invalid response exception raised : EGeneral:Invalid arguments:volume";

const double currencyTickOnFakeResponseXRP_USD_ask = 0.31699;

TEST(KrakenRealRequest, KrakenRealRequest_MarketHistory_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  auto marketHistory = query->getMarketHistory(common::Currency::USD, common::Currency::BTC,
                                               common::TickInterval::ONE_DAY);
  EXPECT_TRUE(marketHistory->marketData_.size() > 0);
}

TEST(KrakenRealRequest, KrakenRealRequest_CurrencyTickRealResponse_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  auto currencyTick = query->getCurrencyTick(common::Currency::XRP, common::Currency::USD);

  EXPECT_TRUE(currencyTick.ask_ > 0);
  EXPECT_TRUE(currencyTick.bid_ > 0);
  EXPECT_TRUE(currencyTick.ask_ > currencyTick.bid_);
}

TEST(KrakenRealRequest, KrakenRealRequest_GetMarketOpenedOrders_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  auto marketOpenedOrders =
      query->getMarketOpenOrders(common::Currency::DASH, common::Currency::USD);

  EXPECT_TRUE(marketOpenedOrders.size() > 0);
}

TEST(KrakenRealRequest, KrakenRealRequest_GetAccountOpenOrders_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  auto accountOpenOrders =
      query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);
  EXPECT_TRUE(accountOpenOrders.size() == 0);
}

TEST(KrakenRealRequest, KrakenRealRequest_CancelOrder_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  std::string message;

  try {
    auto isOrderCanceled =
        query->cancelOrder(common::Currency::XRP, common::Currency::XRP, "asdasdasdsad");
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    message = ex.what();
  }

  EXPECT_EQ(invalidOrderMessage, message);
}

TEST(KrakenRealRequest, KrakenRealRequest_BuyOrder_BadInputData_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  std::string message;

  try {
    auto buyOrder = query->buyOrder(common::Currency::BTC, common::Currency::XRP, 1, 123123123);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    message = ex.what();
  }

  EXPECT_EQ(errorWithoutTradedBalance, message);
}

TEST(KrakenRealRequest, KrakenRealRequest_SellOrder_BadInputData_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  std::string message;

  try {
    auto sellOrder = query->sellOrder(common::Currency::BTC, common::Currency::XRP, 1, 123123123);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    message = ex.what();
  }

  EXPECT_EQ(errorWithoutTradedBalance, message);
}

TEST(KrakenRealRequest, KrakenRealRequest_Kraken_Currencies_On_Valid_Pairs_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  common::KrakenCurrency krakenCurrencies;
  auto base_currencies = krakenCurrencies.getBaseCurrencies();
  for (const auto& currency : base_currencies) {
    auto traded_currencies = krakenCurrencies.getTradedCurrencies(currency);
    for (const auto& traded_currency : traded_currencies) {
      EXPECT_NO_THROW(stock_exchange_utils::getKrakenCurrencyStringFromEnum(traded_currency));
      EXPECT_NO_THROW(query->getCurrencyTick(currency, traded_currency));
    }
  }
}

TEST(KrakenRealRequest, KrakenRealRequest_GetAccountOrderResponse_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  std::string orderUuid = "OJV7QW-QCXDP-BIGIYH";

  auto order = query->getAccountOrder(common::Currency::BTC, common::Currency::QTUM, orderUuid);

  EXPECT_EQ(order.uuid_, orderUuid);
}

TEST(KrakenRealRequest, KrakenRealRequest_GetAccountOrderResponse_BadUuid_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Kraken);

  query->updateApiKey(kraken_api_key);
  query->updateSecretKey(kraken_private);

  std::string orderUuid = "asdasdasd";

  auto order = query->getAccountOrder(common::Currency::ADA, common::Currency::USD, orderUuid);

  EXPECT_TRUE(order.uuid_.empty());
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_getAccountOpenOrders_TestgetAccountOpenOrders) {
  mockKrakenQuery->DelegateToGetOpenedOrdersResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto openOrders =
      mockKrakenQuery->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);

  EXPECT_EQ(openOrders.size(), 2);
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_getNonZeroBalance_TestgetNonZeroBalance) {
  mockKrakenQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(3);

  auto ADA_balance = mockKrakenQuery->getBalance(common::Currency::ADA);
  auto XRP_balance = mockKrakenQuery->getBalance(common::Currency::XRP);
  auto USD_balance = mockKrakenQuery->getBalance(common::Currency::USD);

  EXPECT_TRUE(ADA_balance > 0);
  EXPECT_TRUE(XRP_balance > 0);
  EXPECT_TRUE(USD_balance > 0);
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_getZeroBTC_Balance_Test) {
  mockKrakenQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto BTC_balance = mockKrakenQuery->getBalance(common::Currency::BTC);

  EXPECT_EQ(BTC_balance, 0);
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_buyOrder_Test) {
  mockKrakenQuery->DelegateToBuyOrderResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto buy_order = mockKrakenQuery->buyOrder(common::Currency::ADA, common::Currency::USD, 1, 1);

  EXPECT_TRUE(!buy_order.uuid_.empty());
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_sellOrder_Test) {
  mockKrakenQuery->DelegateToSellOrderResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto sell_order = mockKrakenQuery->sellOrder(common::Currency::XRP, common::Currency::USD, 1, 1);

  EXPECT_TRUE(!sell_order.uuid_.empty());
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_cancelOrder_Test) {
  mockKrakenQuery->DelegateToCancelOrderResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto cancelOrder =
      mockKrakenQuery->cancelOrder(common::Currency::XRP, common::Currency::USD, "fake");

  EXPECT_EQ(cancelOrder, true);
}

TEST_F(KrakenQueryFixture, KrakenQueryFixture_Get_Currency_Tick_Test) {
  mockKrakenQuery->DelegateToCurrencyTickResponse();

  EXPECT_CALL(*mockKrakenQuery, sendRequest(testing::_)).Times(1);

  auto tick = mockKrakenQuery->getCurrencyTick(common::Currency::XRP, common::Currency::USD);

  EXPECT_EQ(tick.ask_, currencyTickOnFakeResponseXRP_USD_ask);
}

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader