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


#include "stock_exchange_binance_unit_test.h"

#include "common/binance_currency.h"
#include "common/currency.h"
#include "common/enumerations/stock_exchange_type.h"
#include "common/exceptions/stock_exchange_exception/invalid_stock_exchange_response_exception.h"
#include "common/utils.h"
#include "gtest/gtest.h"
#include "include/query_processor.h"
#include "include/stock_exchange_library.h"
#include "resources/resources.h"

namespace auto_trader {
namespace stock_exchange {
namespace unit_test {

const std::string secret = "mZOmeXJMvCmCfBzs8r6D2v4bEDZCsBw8ORJF8Hy8uFhZ0XYpTxknIRUrWEFGKEJS";
const std::string api_key = "OV8XBqVFy0bpBF3WnWlyVcGqmKActfaK8yYspLpbEND4EvkuzD9L4cjiQyp3eq0o";

const std::string bad_secret = "mZOmeXJMvCmCfBzs8r6D2v4bEDZChZ0XYpTxknIRUrWEFGKEJS";
const std::string bad_api_key = "OV8XBqVFy0bpBF3WnWlyVcGqmKAc9L4cjiQyp3eq0o";

TEST(Binance, BinanceRealRequest_Binance_Currencies_On_Valid_Pairs_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  common::BinanceCurrency binanceCurrencies;
  auto base_currencies = binanceCurrencies.getBaseCurrencies();
  for (const auto& currency : base_currencies) {
    auto traded_currencies = binanceCurrencies.getTradedCurrencies(currency);
    for (const auto& traded_currency : traded_currencies) {
      EXPECT_NO_THROW(query->getCurrencyTick(currency, traded_currency));
    }
  }
}

TEST(Binance, cancelOrder) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  try {
    auto cancelOrder = query->cancelOrder(common::Currency::BTC, common::Currency::ETH, "101");
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::BINANCE_UNKNOWN_ORDER_SENT);
}

TEST(Binance, getMarket_OpenOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto marketOpenOrders = query->getMarketOpenOrders(common::Currency::BTC, common::Currency::ETH);

  EXPECT_TRUE(marketOpenOrders.size() > 0);
}

TEST(Binance, exchangeInfo) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  auto currencyLotsSizes = query->getCurrencyLotsHolder();

  EXPECT_TRUE(!currencyLotsSizes.empty());
}

TEST(Binance, exchangeInfo_checkCurrencyQty) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  auto currencyLotsSizes = query->getCurrencyLotsHolder();
  EXPECT_TRUE(!currencyLotsSizes.empty());

  common::BinanceCurrency bnncCurrency;
  auto lot = currencyLotsSizes.getLot(
      bnncCurrency.getBinancePair(common::Currency::XRP, common::Currency::BTC));

  EXPECT_TRUE(lot.maxQty_ > lot.minQty_);
  EXPECT_TRUE(lot.stepSize_ > 0);
}

TEST(Binance, getAccount_OpenOrders) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto accountOpenOrders =
      query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC);

  EXPECT_TRUE(accountOpenOrders.size() == 0);
}

TEST(Binance, buyOrderWithBadCurrencyPair) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->buyOrder(common::Currency::BTC, common::Currency::BTC, 1, 1),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Binance, getAccountOpenOrdersWithBadApiKey) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(bad_api_key);

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Binance, getAccountOpenOrdersWithBadSecretKey) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(bad_secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOpenOrders(common::Currency::BTC, common::Currency::LTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Binance, Binance_GetAccountOrderWithBadOrderId_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  EXPECT_THROW(query->getAccountOrder(common::Currency::LTC, common::Currency::BTC, "sfsdfdfds"),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Binance, sellOrder_With_NoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  std::string caughtMessage = "";

  auto currencyTick = query->getCurrencyTick(common::Currency::USDT, common::Currency::BTC);

  try {
    auto sellOrder =
        query->sellOrder(common::Currency::USDT, common::Currency::BTC, 1.1, currencyTick.ask_);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::BINANCE_INSUFFICIENT_BALANCE_MESSAGE);
}

TEST(Binance, buyOrder_With_NoBalance) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto currencyTick = query->getCurrencyTick(common::Currency::BTC, common::Currency::LTC);

  std::string caughtMessage = "";

  try {
    auto buyOrder =
        query->buyOrder(common::Currency::BTC, common::Currency::LTC, 1, currencyTick.bid_);
  } catch (const common::exceptions::InvalidStockExchangeResponse& ex) {
    caughtMessage = ex.what();
  }

  EXPECT_EQ(caughtMessage, resources::messages::BINANCE_INSUFFICIENT_BALANCE_MESSAGE);
}

TEST(Binance, getCurrencyTick) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  auto tick = query->getCurrencyTick(common::Currency::BTC, common::Currency::LTC);
  EXPECT_TRUE(tick.ask_ > 0);
  EXPECT_TRUE(tick.bid_ > 0);
  EXPECT_TRUE(tick.ask_ > tick.bid_);
}

TEST(Binance, getCurrencyTick_BadCurrencyPair) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  EXPECT_THROW(query->getCurrencyTick(common::Currency::BTC, common::Currency::BTC),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST(Binance, GetBalanceWithNoMoney) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto balance = query->getBalance(common::Currency::USD);

  EXPECT_EQ(balance, 0.0);
}

TEST(Binance, GetBalanceWithUnknownCurrency) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  query->updateSecretKey(secret);
  query->updateApiKey(api_key);

  auto balance = query->getBalance(common::Currency::UNKNOWN);

  EXPECT_EQ(balance, 0.0);
}

TEST(Binance, Binance_GetMarketHistoryResponse_Test) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  auto marketHistory = query->getMarketHistory(common::Currency::BTC, common::Currency::LTC,
                                               common::TickInterval::SIX_HOURS);

  EXPECT_TRUE(marketHistory->marketData_.size() > 0);
}

TEST(Binance, Binance_GetMarketHistoryResponse_Bad_Currency_PairTest) {
  auto_trader::stock_exchange::StockExchangeLibrary stockExchangeLibrary;
  auto& queryProcessor = stockExchangeLibrary.getQueryProcessor();
  auto query = queryProcessor.getQuery(common::StockExchangeType::Binance);

  EXPECT_THROW(query->getMarketHistory(common::Currency::BTC, common::Currency::BTC,
                                       common::TickInterval::SIX_HOURS),
               common::exceptions::InvalidStockExchangeResponse);
}

TEST_F(BinanceQueryFixture, getMarketHistory) {
  mockBinanceQuery->DelegateToMarketHistoryResponse();

  EXPECT_CALL(*mockBinanceQuery, processHttpRequest(testing::_, testing::_, testing::_)).Times(1);

  auto marketHistory = mockBinanceQuery->getMarketHistory(
      common::Currency::BTC, common::Currency::USDT, common::TickInterval::ONE_DAY);

  EXPECT_EQ(marketHistory->toBuy_, common::Currency::USDT);
  EXPECT_EQ(marketHistory->toSell_, common::Currency::BTC);

  auto marketData = marketHistory->marketData_;

  EXPECT_EQ(marketData.size(), 2);
}

}  // namespace unit_test
}  // namespace stock_exchange
}  // namespace auto_trader