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

#include "stock_exchange_huobi_unit_test.h"

#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/huobi_currency.h"
#include "common/utils.h"
#include "gtest/gtest.h"
#include "include/query_processor.h"
#include "include/stock_exchange_library.h"
#include "include/stock_exchange_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

constexpr char secret_key[] = "15dea147-60569174-799d1c22-3ee82";
constexpr char api_key[] = "1qdmpe4rty-83fd0ed1-d93bfc93-44e83";

TEST(Huobi, Currencies_On_Valid_Pairs_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  common::HuobiCurrency huobiCurrencies;
  auto base_currencies = huobiCurrencies.getBaseCurrencies();
  for (const auto& currency : base_currencies) {
    auto traded_currencies = huobiCurrencies.getTradedCurrencies(currency);
    for (const auto& traded_currency : traded_currencies) {
      EXPECT_NO_THROW(query->getCurrencyTick(currency, traded_currency));
    }
  }
}

TEST(Huobi, CancelOrder) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  try {
    query->cancelOrder(common::Currency::BTC, common::Currency::ETH, "101");
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::HUOBI_UNKNOWN_ORDER_STATE);
}

TEST(Huobi, GetMarketOpenOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  auto marketOpenOrders = query->getMarketOpenOrders(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(!marketOpenOrders.empty());
}

TEST(Huobi, ExchangeInfo) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  auto currencyLotsSizes = query->getCurrencyLotsHolder();

  EXPECT_TRUE(currencyLotsSizes.empty());
}

TEST(Huobi, GetAccountOpenOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  auto accountOpenOrders =
      query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);

  EXPECT_TRUE(accountOpenOrders.empty());
}

TEST(Huobi, BuyOrderWithBadCurrencyPair) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->buyOrder(common::Currency::BTC, common::Currency::ETH, 1, 1),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, SellOrder_With_NoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  auto currencyTick = query->getCurrencyTick(common::Currency::USDT, common::Currency::BTC);

  try {
    auto sellOrder =
        query->sellOrder(common::Currency::USDT, common::Currency::BTC, 1.1, currencyTick.bid_);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::HUOBI_INVALID_BALANCE);
}

TEST(Huobi, BuyOrder_With_NoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  auto currencyTick = query->getCurrencyTick(common::Currency::BTC, common::Currency::LTC);

  std::string caughtMessage = "";

  try {
    auto buyOrder =
        query->buyOrder(common::Currency::BTC, common::Currency::LTC, 1, currencyTick.ask_);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::HUOBI_INVALID_BALANCE);
}

TEST(Huobi, GetAccountOpenOrdersWithBadApiKey) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey("invalid_api_key");

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, GetAccountOpenOrdersWithBadSecretKey) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey("invalid_secret_key");
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, GetAccountOrderWithBadOrderId_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOrder(common::Currency::LTC, common::Currency::BTC, "invalid_id"),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, GetCurrencyTick) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  auto tick = query->getCurrencyTick(common::Currency::BTC, common::Currency::LTC);
  EXPECT_TRUE(tick.ask_ > 0);
  EXPECT_TRUE(tick.bid_ > 0);
  EXPECT_TRUE(tick.ask_ > tick.bid_);
}

TEST(Huobi, GetCurrencyTick_BadCurrencyPair) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  EXPECT_THROW(query->getCurrencyTick(common::Currency::BTC, common::Currency::BTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, GetBalanceWithNoMoney) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  auto balance = query->getBalance(common::Currency::ETH);

  EXPECT_EQ(balance, 0.0);
}

TEST(Huobi, GetBalanceWithUnknownCurrency) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  query->updateSecretKey(secret_key);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getBalance(common::Currency::UNKNOWN),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Huobi, GetMarketHistoryResponse_Real_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  auto marketHistory = query->getMarketHistory(common::Currency::BTC, common::Currency::LTC,
                                               common::TickInterval::FIFTEEN_MIN);

  EXPECT_TRUE(marketHistory->marketData_.size() > 0);
}

TEST(Huobi, GetMarketHistoryResponse_Bad_Currency_PairTest) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Huobi);

  EXPECT_THROW(query->getMarketHistory(common::Currency::BTC, common::Currency::BTC,
                                       common::TickInterval::ONE_HOUR),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST_F(HuobiQueryFixture, GetAccountOrderTest) {
  mockHuobiQuery->DelegateToGetAccountOrderResponse();

  EXPECT_CALL(*mockHuobiQuery, sendRequest(testing::_)).Times(1);

  std::string orderUuid = "59378";
  common::MarketOrder expectedOrder;
  expectedOrder.uuid_ = orderUuid;
  expectedOrder.price_ = 100.23;
  expectedOrder.quantity_ = 43;
  expectedOrder.fromCurrency_ = common::Currency::USDT;
  expectedOrder.toCurrency_ = common::Currency::ETH;
  expectedOrder.isCanceled_ = false;
  expectedOrder.stockExchangeType_ = common::StockExchangeType::Huobi;
  expectedOrder.orderType_ = common::OrderType::BUY;
  expectedOrder.opened_ = stock_exchange_utils::getDataFromTimestamp(1494901162595, true);

  auto order =
      mockHuobiQuery->getAccountOrder(common::Currency::BTC, common::Currency::LTC, orderUuid);
  EXPECT_EQ(order, expectedOrder);
}

TEST_F(HuobiQueryFixture, GetMarketHistoryResponse_FakeData) {
  mockHuobiQuery->DelegateToMarketHistoryResponse();

  EXPECT_CALL(*mockHuobiQuery, sendRequest(testing::_)).Times(1);

  common::MarketData expectedFirstData;
  expectedFirstData.volume_ = 22209664.502456654;
  expectedFirstData.openPrice_ = 7144.00;
  expectedFirstData.closePrice_ = 7160.00;
  expectedFirstData.highPrice_ = 7175.00;
  expectedFirstData.lowPrice_ = 7111.00;
  expectedFirstData.date_ = stock_exchange_utils::getDataFromTimestamp(1576771200, false);

  common::MarketData expectedSecondData;
  expectedSecondData.volume_ = 472502139.43386728;
  expectedSecondData.openPrice_ = 6823.00;
  expectedSecondData.closePrice_ = 7144.00;
  expectedSecondData.highPrice_ = 7437.00;
  expectedSecondData.lowPrice_ = 6771.00;
  expectedSecondData.date_ = stock_exchange_utils::getDataFromTimestamp(1576684800, false);

  common::MarketHistory expectedHistory;
  expectedHistory.toBuy_ = common::Currency::BTC;
  expectedHistory.toSell_ = common::Currency::USDT;

  expectedHistory.marketData_.push_back(expectedFirstData);
  expectedHistory.marketData_.push_back(expectedSecondData);

  auto marketHistory = mockHuobiQuery->getMarketHistory(
      common::Currency::USDT, common::Currency::BTC, common::TickInterval::ONE_DAY);

  EXPECT_EQ(marketHistory->toBuy_, common::Currency::BTC);
  EXPECT_EQ(marketHistory->toSell_, common::Currency::USDT);

  EXPECT_EQ(marketHistory->marketData_.size(), 2);

  EXPECT_TRUE(marketHistory->marketData_.at(0) == expectedHistory.marketData_.at(0));
  EXPECT_TRUE(marketHistory->marketData_.at(1) == expectedHistory.marketData_.at(1));
}

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader