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

#include "stock_exchange_bittrex_unit_test.h"

#include <vector>

#include "common/exceptions/no_data_found_exception.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "gtest/gtest.h"
#include "include/query_processor.h"
#include "include/stock_exchange_library.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

const double DOGE_balance = 4.21549076;

const std::string secret = "cfd5a28c7b5b4f1382b9fc3de26f36cc";
const std::string bad_secret_key = "0a07ba0a7c0d69a756cc27194";
const std::string api_key = "e0e61d2b3b784c129ede93b665a62409";
const std::string bad_api_key = "d5859b92dab29c0ced97a";

std::shared_ptr<MockBittrexQuery> mockBittrexQuery;

class MockQueryFactory : public QueryFactory {
 public:
  std::shared_ptr<Query> createQuery(common::StockExchangeType type) const override {
    return mockBittrexQuery;
  }
};

class BittrexQueryFixture : public ::testing::Test {
 public:
  void SetUp() override { mockBittrexQuery = std::make_shared<MockBittrexQuery>(); }
  void TearDown() override { mockBittrexQuery.reset(); }
};

TEST_F(BittrexQueryFixture, MarketHistory_With_Predefined_Response) {
  mockBittrexQuery->DelegateToMarketHistoryResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  auto marketHistory = mockBittrexQuery->getMarketHistory(
      common::Currency::USD, common::Currency::BTC, common::TickInterval::ONE_DAY);

  EXPECT_EQ(marketHistory->toBuy_, common::Currency::BTC);
  EXPECT_EQ(marketHistory->toSell_, common::Currency::USD);

  auto marketData = marketHistory->marketData_;
  EXPECT_EQ(marketData.size(), 2);

  common::Date date1 = {0, 0, 0, 31, 5, 2018};
  EXPECT_EQ(marketData[0].date_, date1);
  EXPECT_EQ(marketData[0].closePrice_, 6152.12);

  common::Date date2 = {0, 0, 0, 1, 6, 2018};
  EXPECT_EQ(marketData[1].date_, date2);
  EXPECT_EQ(marketData[1].closePrice_, 6251.13);
}

TEST(BittrexQuery, BittrexRealRequest_Bittrex_Currencies_On_Valid_Pairs_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  common::BittrexCurrency bittrexCurrencies;
  auto base_currencies = bittrexCurrencies.getBaseCurrencies();
  for (const auto& currency : base_currencies) {
    auto traded_currencies = bittrexCurrencies.getTradedCurrencies(currency);
    for (const auto& traded_currency : traded_currencies) {
      EXPECT_NO_THROW(query->getCurrencyTick(currency, traded_currency));
    }
  }
}

TEST(BittrexQuery, MarketHistory_Contains_More_Than_One_Record) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto marketHistory = query->getMarketHistory(common::Currency::USD, common::Currency::BTC,
                                               common::TickInterval::ONE_DAY);

  EXPECT_EQ(marketHistory->toBuy_, common::Currency::BTC);
  EXPECT_EQ(marketHistory->toSell_, common::Currency::USD);

  auto marketData = marketHistory->marketData_;
  EXPECT_TRUE(marketData.size() > 0);
}

TEST(BittrexQuery, CancelOrder_Unknown_UUID) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  EXPECT_THROW(query->cancelOrder(common::Currency::ETH, common::Currency::BTC, "101"),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST_F(BittrexQueryFixture, SellOrder_Without_Api_Key) {
  mockBittrexQuery->DelegateToSellResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  common::MarketOrder currentOrder =
      query->sellOrder(common::Currency::USD, common::Currency::BTC, 1.2, 4.0);
  EXPECT_EQ(currentOrder.uuid_, "614c34e4-8d71-11e3-94b5-425861b86ab6");
}

TEST_F(BittrexQueryFixture, BuyOrder_Without_Api_Key) {
  mockBittrexQuery->DelegateToBuyResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  common::MarketOrder currentOrder =
      query->buyOrder(common::Currency::USD, common::Currency::BTC, 1.2, 4.0);
  EXPECT_EQ(currentOrder.uuid_, "614c34e4-8d71-11e3-94b5-425861b86ab6");
}

TEST_F(BittrexQueryFixture, Cancel_Active_Sell_Order) {
  mockBittrexQuery->DelegateToSellResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(2);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  common::MarketOrder currentOrder =
      query->sellOrder(common::Currency::USD, common::Currency::BTC, 1.2, 4.0);

  EXPECT_EQ(currentOrder.uuid_, "614c34e4-8d71-11e3-94b5-425861b86ab6");
  mockBittrexQuery->DelegateToCancelResponse();

  EXPECT_TRUE(query->cancelOrder(common::Currency::USD, common::Currency::BTC,
                                 "614c34e4-8d71-11e3-94b5-425861b86ab6"));
}

TEST_F(BittrexQueryFixture, Cancel_Buy_Order) {
  mockBittrexQuery->DelegateToBuyResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(2);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  common::MarketOrder currentOrder =
      query->buyOrder(common::Currency::USD, common::Currency::BTC, 1.2, 4.0);

  EXPECT_EQ(currentOrder.uuid_, "614c34e4-8d71-11e3-94b5-425861b86ab6");
  mockBittrexQuery->DelegateToCancelResponse();

  EXPECT_TRUE(query->cancelOrder(common::Currency::USD, common::Currency::BTC,
                                 "614c34e4-8d71-11e3-94b5-425861b86ab6"));
}

TEST_F(BittrexQueryFixture, Get_Balance) {
  mockBittrexQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto balance = query->getBalance(common::Currency::USD);

  EXPECT_EQ(balance, 0.0);
}

TEST_F(BittrexQueryFixture, BittrexQueryFixture_Get_Balance_UNKNOWN_Currency_Test) {
  mockBittrexQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto balance = query->getBalance(common::Currency::UNKNOWN);

  EXPECT_EQ(balance, 0.0);
}

TEST_F(BittrexQueryFixture, Get_Balance_DOGE) {
  mockBittrexQuery->DelegateToGetBalanceResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  MockQueryFactory factory;
  QueryProcessor queryProcessor(factory);
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto balance = query->getBalance(common::Currency::DOGE);

  EXPECT_EQ(balance, DOGE_balance);
}

TEST_F(BittrexQueryFixture, BittrexQueryFixture_Get_Account_Order_Test) {
  mockBittrexQuery->DelegateToGetAccountOrderResponse();

  EXPECT_CALL(*mockBittrexQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  std::string orderUuid = "8925d746-bc9f-4684-b1aa-e507467aaa99";
  auto order =
      mockBittrexQuery->getAccountOrder(common::Currency::BTC, common::Currency::LTC, orderUuid);

  EXPECT_EQ(order.uuid_, orderUuid);
}

TEST(Bittrex, getMarketOpenOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto openOrders = query->getMarketOpenOrders(common::Currency::BTC, common::Currency::LTC);

  EXPECT_TRUE(openOrders.size() > 0);
}

TEST(Bittrex, accountOpenedOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto openOrders = query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);

  EXPECT_TRUE(openOrders.size() == 0);
}

TEST(Bittrex, getMarketHistory) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  auto candles = query->getMarketHistory(common::Currency::USD, common::Currency::BTC,
                                         common::TickInterval::THIRTY_MIN);

  EXPECT_TRUE(candles->marketData_.size() > 0);
}

TEST(Bittrex, sellOrderWithNoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  try {
    auto sellOrder = query->sellOrder(common::Currency::BTC, common::Currency::LTC, 1, 10000);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::BITTREX_INSUFFICIENT_FUNDS);
}

TEST(Bittrex, buyOrderWithNoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  try {
    auto buyOrder = query->buyOrder(common::Currency::BTC, common::Currency::LTC, 1, 10000);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::BITTREX_INSUFFICIENT_FUNDS);
}

TEST(Bittrex, cancelOrder) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->cancelOrder(common::Currency::ETH, common::Currency::BTC, "101"),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Bittrex, getCurrencyTick) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto currencyTick = query->getCurrencyTick(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(currencyTick.ask_ > 0);
  EXPECT_TRUE(currencyTick.bid_ > 0);
  EXPECT_TRUE(currencyTick.ask_ > currencyTick.bid_);
}

TEST(Bittrex, getCurrencyTick_BadCurrencyPair) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  EXPECT_THROW(query->getCurrencyTick(common::Currency::BTC, common::Currency::BTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Bittrex, GetAccountOrder) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOrder(common::Currency::LTC, common::Currency::BTC,
                                      "8925d746-bc9f-4684-b1aa-e507467aaa99"),
               common::exceptions::NoDataFoundException);
}

TEST(Bittrex, Bittrex_BuyOrderWithBadCurrencyPair_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->buyOrder(common::Currency::BTC, common::Currency::BTC, 1, 1),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Bittrex, Bittrex_GetAccountOpenOrdersWithBadApiKey_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(secret);
  query->updateApiKey(bad_api_key);

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Bittrex, Bittrex_GetAccountOpenOrdersWithBadSecretKey_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Bittrex);

  query->updateSecretKey(bad_secret_key);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader