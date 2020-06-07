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

#include "trading_run_ut.h"

#include <thread>

#include "common/exceptions/no_data_found_exception.h"
#include "common/market_history.h"
#include "include/trading_manager.h"
#include "model/include/settings/strategies_settings/custom_strategy_settings.h"
#include "model/include/settings/strategies_settings/strategy_settings.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

/*
 * Test plan:
 *  1. Buy order trigger.
 *  2. Check closed buy orders.
 *  3. Sell order trigger.
 *  4. Check closed sell orders.
 *  5. Customer open buy order manually.
 *  6. Customer cancel buy orders.
 *  7. Customer withdraw funds from base currency.
 *  8. Customer withdraw funds from traded currency.
 *  9. Customer open sell order manually.
 *  10. Customer cancel sell order manually.
 *  11. Orders are the same after restart.
 *  12. Client restart trading with closed buy orders.
 *  13. Client restart trading with closed sell orders.
 *  14. Close outdated buy orders.
 *  15. Close outdated sell orders.
 *  16. Check max open orders count for buying orders.
 *  17. Check open positions per coin for buying orders.
 *  18. Check min coin amount for buying orders.
 *  19. Check several strategies signal.
 *  20. Stop loss.
 *  21. Change trading configuration settings.
 *  22. Restart trading with closed buy orders(active orders profit).
 *  23. Crossing interval
 *  24. One market history cannot be analyzed several times.
 *  25. Check any indicator in trading.
 *  26. Invalid amount of money.
 *  27. Restart trading and reset previous data.
 *  28. Restart trading and save previous trading data.
 *  29. Strategy market data exists
 *  30. Strategy market data save and restore.
 *  31. Reset order profit.
 *  32. Sell order be lower if balance is not enough to cover closed buy order.
 *  33. Lots holder should correlate orders.
 */

constexpr int MAX_TICK_COUNT = 60;

static void waitCallbackEvent(std::function<bool()> callback) {
  unsigned int tickCounter = 0;
  while (true) {
    if (callback()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return;
    }

    if (tickCounter > MAX_TICK_COUNT) {
      FAIL();
    }
    ++tickCounter;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

TEST_F(TradingUIFixture, BuyOrderTrigger_1) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto accountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (accountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool ordersMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, CheckClosedBuyOrders_2) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    return (tradeOrdersHolder.getBuyOrdersCount() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.5);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
  orderProfit.forEachOrder(
      [&](const common::MarketOrder &marketOrder) { EXPECT_EQ(marketOrder, orderToClose); });
}

TEST_F(TradingUIFixture, SellOrderTrigger_3) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.10, 1.06, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyOrdersCount == 1) && (sellOrdersCount == 1));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);
  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);
  for (auto &accountOrder : accountOrders) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
    } else if (accountOrder.orderType_ == common::OrderType::SELL) {
      EXPECT_TRUE(tradeOrdersHolder.containSellOrder(accountOrder));
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching(
      [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
        EXPECT_TRUE(tradeOrdersHolder.containSellOrder(fromOrder));
        EXPECT_TRUE(toOrder == orderToClose);
        ordersMatched = true;
      });

  EXPECT_EQ(ordersMatched, true);
}

TEST_F(TradingUIFixture, CheckClosedSellOrders_4) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1.0);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.15, 1.05, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyOrdersCount == 1) && (sellOrdersCount == 1));
  });

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);
  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  tradeOrdersHolder.forEachSellingOrder(
      [&](const common::MarketOrder &order) { fakeQuery->closeOrder(order.uuid_); });

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto updatedAccountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(updatedAccountOrders.size(), 2);

  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching([&](const common::MarketOrder &fromOrder,
                                    const common::MarketOrder &toOrder) { ordersMatched = true; });

  EXPECT_EQ(ordersMatched, false);
}

TEST_F(TradingUIFixture, CustomerOpenBuyOrderManually_5) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1.3);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  fakeQuery->buyOrder(common::Currency::USD, common::Currency::LTC, 0.2, 0.1);

  waitCallbackEvent([&]() -> bool {
    auto buyOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    auto sellOrdersCount = tradeOrdersHolder.getSellOrdersCount();

    return ((buyOrdersCount == 3) && (sellOrdersCount == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 3);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 3);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool isMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { isMatched = true; });

  EXPECT_FALSE(isMatched);
}

TEST_F(TradingUIFixture, CustomerCancelBuyOrder_6) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);

  auto marketHistoryPtr3 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr3->toBuy_ = common::Currency::LTC;
  marketHistoryPtr3->toSell_ = common::Currency::USD;
  marketHistoryPtr3->marketData_ = smaCandlesWithoutBuySignal;
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr3);

  auto marketOrder = accountOrders.front();
  fakeQuery->cancelOrder(common::Currency::USD, common::Currency::LTC, marketOrder.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();

    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);
  for (auto &order : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(order));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool isMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { isMatched = true; });

  EXPECT_FALSE(isMatched);
}

TEST_F(TradingUIFixture, CustomerWithdrawFundsFromBaseCurrency_7) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesWithoutBuySignal;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  fakeQuery->setBalance(common::Currency::USD, 0.7);

  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  EXPECT_EQ(fakeQuery->getBalance(common::Currency::USD), 0.099999999999999950);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  auto &ordersMatching = getTradeOrdersHolder().getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { EXPECT_TRUE(false); });
}

TEST_F(TradingUIFixture, CustomerWidthrawFundsFromTradedCurrency_8) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();

  fakeQuery->closeOrder(orderToClose.uuid_);
  fakeQuery->setBalance(common::Currency::LTC, 0.2);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();

    return (tradingManager->isRunning() && (buyingOrdersCount == 1) && (sellingOrdersCount == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.5);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
  EXPECT_TRUE(!orderProfit.isEmpty());
}

TEST_F(TradingUIFixture, CustomerOpenSellOrderManually_9) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 0));
  });

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto manualSellOrder = fakeQuery->sellOrder(common::Currency::USD, common::Currency::LTC, 0.3, 1);
  EXPECT_FALSE(tradeOrdersHolder.containSellOrder(manualSellOrder));

  waitCallbackEvent([&]() -> bool {
    try {
      auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
      return (orderProfit.getBalance() == 0.5);
    } catch (common::exceptions::NoDataFoundException &exception) {
      return false;
    }
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.5);
  EXPECT_TRUE(!orderProfit.isEmpty());
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
}

TEST_F(TradingUIFixture, CustomerCancelSellOrderManually_10) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();

  fakeQuery->closeOrder(orderToClose.uuid_);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  common::CurrencyTick newCurrencyTick{1.10, 1.06, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1));
  });

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);

  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  common::CurrencyTick updatedCurrencyTick{1.02, 1.01, common::Currency::USD,
                                           common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, updatedCurrencyTick);

  tradeOrdersHolder.forEachSellingOrder([&](const common::MarketOrder &order) {
    fakeQuery->cancelOrder(common::Currency::USD, common::Currency::LTC, order.uuid_);
  });

  waitCallbackEvent([&]() -> bool {
    try {
      auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
      return (orderProfit.getBalance() == 0.5);
    } catch (common::exceptions::NoDataFoundException &exception) {
      return false;
    }
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.5);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
  unsigned ordersCount = 0;
  bool orderExists = false;
  orderProfit.forEachOrder([&](const common::MarketOrder &order) {
    ++ordersCount;
    if (order.uuid_ == orderToClose.uuid_) {
      orderExists = true;
    }
  });

  EXPECT_EQ(ordersCount, 1);
  EXPECT_TRUE(orderExists);
}

TEST_F(TradingUIFixture, OrdersTheSameAfterRestart_11) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.10, 1.06, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);
  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  tradeOrdersHolder.clear();

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1) && tradingManager->isRunning());
  });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty(),
               common::exceptions::NoDataFoundException);

  auto accountOrdersUpdated =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);
  for (auto &accountOrder : accountOrdersUpdated) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
    } else if (accountOrder.orderType_ == common::OrderType::SELL) {
      EXPECT_TRUE(tradeOrdersHolder.containSellOrder(accountOrder));
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching(
      [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
        EXPECT_TRUE(tradeOrdersHolder.containSellOrder(fromOrder));
        EXPECT_TRUE(toOrder == orderToClose);
        ordersMatched = true;
      });

  EXPECT_EQ(ordersMatched, true);
}

TEST_F(TradingUIFixture, CustomerClosedBuyOrderDuringRestart_12) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();

  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.01, 0.91, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  tradeOrdersHolder.clear();

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);
  EXPECT_FALSE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  auto &ordersProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(ordersProfit.getCurrency(), common::Currency::LTC);
  EXPECT_EQ(ordersProfit.getBalance(), 0.5);
  EXPECT_TRUE(ordersProfit.containOrder(orderToClose));

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);
  for (auto &accountOrder : accountOrders) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching([&](const common::MarketOrder &fromOrder,
                                    const common::MarketOrder &toOrder) { ordersMatched = true; });

  EXPECT_EQ(ordersMatched, false);
}

TEST_F(TradingUIFixture, CustomerCloseSellOrderDuringRestart_13) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.15, 1.05, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);

  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  tradeOrdersHolder.forEachSellingOrder(
      [&](const common::MarketOrder &order) { fakeQuery->closeOrder(order.uuid_); });

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC,
                             {1, 0.9, common::Currency::USD, common::Currency::LTC});

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 2) && (sellingOrdersCount == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  auto updatedAccountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(updatedAccountOrders.size(), 2);

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty(),
               common::exceptions::NoDataFoundException);
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching([&](const common::MarketOrder &fromOrder,
                                    const common::MarketOrder &toOrder) { ordersMatched = true; });

  EXPECT_EQ(ordersMatched, false);
}

TEST_F(TradingUIFixture, CloseOutdatedBuyOrders_14) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 1, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  std::this_thread::sleep_for(std::chrono::milliseconds(35000));

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);
  auto marketHistoryPtr3 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr3->toBuy_ = common::Currency::LTC;
  marketHistoryPtr3->toSell_ = common::Currency::USD;
  marketHistoryPtr3->marketData_ = smaCandlesWithoutBuySignal;
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr3);

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(35000));

  tradingManager->stopTradingSlot();
  tradingThread.join();

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 0);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);
  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool isMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { isMatched = true; });

  EXPECT_FALSE(isMatched);
}

TEST_F(TradingUIFixture, CloseOutdatedSellOrders_15) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 1, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);
  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.10, 1.06, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1));
  });

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);
  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC,
                             {1, 0.9, common::Currency::USD, common::Currency::LTC});

  std::this_thread::sleep_for(std::chrono::milliseconds(62000));

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto accountOrdersUpdated =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrdersUpdated.size(), 1);
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &ordersProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_FALSE(ordersProfit.isEmpty());
  EXPECT_TRUE(ordersProfit.containOrder(orderToClose));
  EXPECT_EQ(ordersProfit.getBalance(), 0.5);
  EXPECT_EQ(ordersProfit.getCurrency(), common::Currency::LTC);
}

TEST_F(TradingUIFixture, MaxOpenOrdersCountForBuyingOrders_16) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 1, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);
  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }
}

TEST_F(TradingUIFixture, CheckOpenPositionsAmountPerCoinForBuyingOrders_17) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 1, 3, 1, 50, 0.1, 1);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }
}

TEST_F(TradingUIFixture, CheckMinCoinAmountForBuyingOrder_18) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 1, 3, 1, 70, 0.6, 5);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto accountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (accountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
}

TEST_F(TradingUIFixture, CheckSeveralIndicatorSignal_19) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto rsiSettings = TradingUIFixture::createRsiSettings(10, 0, 80, 20);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  customSettings->strategies_.emplace_back(std::move(rsiSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 0);

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
}

TEST_F(TradingUIFixture, StopLossSignal_20) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();

  fakeQuery->closeOrder(orderToClose.uuid_);

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  waitCallbackEvent([&]() -> bool {
    auto buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    auto sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();

    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 0));
  });

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.5);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
  orderProfit.forEachOrder(
      [&](const common::MarketOrder &marketOrder) { EXPECT_EQ(marketOrder, orderToClose); });

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC,
                             {0.8, 0.6, common::Currency::USD, common::Currency::LTC});

  waitCallbackEvent([&]() -> bool {
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 1) && (sellingOrdersCount == 1));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);

  auto accountOrdersUpdated =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);
  for (auto &accountOrder : accountOrdersUpdated) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
    } else if (accountOrder.orderType_ == common::OrderType::SELL) {
      EXPECT_TRUE(tradeOrdersHolder.containSellOrder(accountOrder));
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching(
      [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
        EXPECT_TRUE(tradeOrdersHolder.containSellOrder(fromOrder));
        EXPECT_TRUE(toOrder == orderToClose);
        ordersMatched = true;
      });

  EXPECT_TRUE(ordersMatched);
}

TEST_F(TradingUIFixture, ChangeConfigurationSettings_21) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 1, 3, 1, 50, 0.1, 2);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  auto &currentConfiguration = getTradeConfigsHolder().takeCurrentTradeConfiguration();
  currentConfiguration.takeBuySettings().maxOpenOrders_ = 3;

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);
}

TEST_F(TradingUIFixture, RestartTradingWithClosedBuyOrders_22) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();

  fakeQuery->closeOrder(orderToClose.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrders = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrders = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrders == 1) && (sellingOrders == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);
  EXPECT_FALSE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  tradeOrdersHolder.clear();

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    size_t buyingOrders = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrders = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrders == 1) && (sellingOrders == 0));
  });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);
  EXPECT_FALSE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  auto &ordersProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(ordersProfit.getBalance(), 0.5);
  EXPECT_EQ(ordersProfit.getCurrency(), common::Currency::LTC);
  EXPECT_TRUE(ordersProfit.containOrder(orderToClose));

  auto accountOrdersUpdated =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrdersUpdated.size(), 1);
  for (auto &accountOrder : accountOrdersUpdated) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
    } else if (accountOrder.orderType_ == common::OrderType::SELL) {
      EXPECT_TRUE(tradeOrdersHolder.containSellOrder(accountOrder));
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching([&](const common::MarketOrder &fromOrder,
                                    const common::MarketOrder &toOrder) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, CrossingInterval_23) {
  auto smaSettings = TradingUIFixture::createSmaSettings(5, 1);
  smaSettings->crossingInterval_ = 4;
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  std::vector<common::MarketData> secondMarketDataForCrossingInterval = {
      {3899.354, 3910.847, 3899.354, 3910.847, 31771.19121},
      {3910.511, 3913.390, 3910.511, 3915.000, 23792.07473},
      {3911.685, 3917.909, 3907.374, 3919.826, 56319.24906},
      {3917.909, 3911.798, 3911.798, 3918.656, 27686.28295},
      {3912.101, 3915.099, 3912.000, 3915.662, 24852.16023},
      {3915.037, 3916.218, 3912.888, 3916.913, 50898.86019},
      {3916.218, 3914.879, 3912.275, 3921.000, 131419.3891},
      {3913.004, 3915.272, 3911.284, 3915.447, 18808.81698},
      {3915.349, 3918.083, 3913.817, 3920.607, 32993.89884},
      {3921.459, 3920.733, 3914.940, 3922.860, 51772.78955},
      {3918.687, 3915.446, 3912.761, 3918.910, 138872.9634},
      {3918.089, 3917.974, 3911.810, 3918.413, 85961.77295},
      {3917.516, 3915.519, 3914.290, 3917.974, 15910.84497},
      {3914.291, 3911.444, 3911.074, 3915.130, 28474.95081},
      {3909.808, 3911.888, 3909.492, 3915.912, 10442.95158},
      {3911.888, 3912.499, 3900.120, 3913.235, 105326.5773},
      {3912.500, 3914.576, 3910.966, 3914.921, 64686.21302},
      {3914.921, 3914.920, 3911.419, 3914.921, 31435.45970},
      {3914.921, 3917.974, 3914.411, 3917.974, 100569.5918},
      {3917.974, 3915.000, 3914.850, 3917.974, 15242.44981},
      {3915.000, 3935.770, 3915.000, 3935.770, 68510.95386},
      {3916.000, 3985.770, 3915.000, 3935.872, 68510.95387}};

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = secondMarketDataForCrossingInterval;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  getAppSettings().tradingTimeout_ = 0;

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool ordersMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, OneMarketHistoryCannotBeAnalyzedSeveralTimes_24) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool ordersMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, AnyIndicatorShouldWork_25) {
  auto smaSettings = createSmaSettings(10, 0);
  auto rsiSettings = createRsiSettings(10, 0, 80, 20);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  customSettings->strategies_.emplace_back(std::move(rsiSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});
  tradeConfig->takeBuySettings().openOrderWhenAnyIndicatorIsTriggered_ = true;
  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  std::vector<common::MarketData> thirdMarketHistory = {
      {3899.354, 3910.847, 3899.354, 3910.847, 31771.19121},
      {3910.511, 3913.390, 3910.511, 3915.000, 23792.07473},
      {3911.685, 3917.909, 3907.374, 3919.826, 56319.24906},
      {3917.909, 3911.798, 3911.798, 3918.656, 27686.28295},
      {3912.101, 3915.099, 3912.000, 3915.662, 24852.16023},
      {3915.037, 3916.218, 3912.888, 3916.913, 50898.86019},
      {3916.218, 3914.879, 3912.275, 3921.000, 131419.3891},
      {3913.004, 3915.272, 3911.284, 3915.447, 18808.81698},
      {3915.349, 3918.083, 3913.817, 3920.607, 32993.89884},
      {3921.459, 3920.733, 3914.940, 3922.860, 51772.78955},
      {3918.687, 3915.446, 3912.761, 3918.910, 138872.9634},
      {3918.089, 3917.974, 3911.810, 3918.413, 85961.77295},
      {3917.516, 3915.519, 3914.290, 3917.974, 15910.84497},
      {3914.291, 3911.444, 3911.074, 3915.130, 28474.95081},
      {3909.808, 3911.888, 3909.492, 3915.912, 10442.95158},
      {3911.888, 3912.499, 3900.120, 3913.235, 105326.5773},
      {3912.500, 3914.576, 3910.966, 3914.921, 64686.21302},
      {3914.921, 3914.920, 3911.419, 3914.921, 31435.45970},
      {3914.921, 3917.974, 3914.411, 3917.974, 100569.5918},
      {3917.974, 3915.000, 3914.850, 3917.974, 15242.44981},
      {3915.000, 3935.770, 3915.000, 3935.770, 68510.95386},
      {3916.000, 3985.770, 3915.000, 3935.872, 68510.95387}};

  auto marketHistoryPtr3 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = thirdMarketHistory;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr3);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool ordersMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, InvalidAmountOfMoney_26) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.45, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 0.4);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 0);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);

  bool ordersMatched = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();
  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &, const common::MarketOrder &) { ordersMatched = true; });

  EXPECT_FALSE(ordersMatched);
}

TEST_F(TradingUIFixture, RestartTradingAndResetPreviousData_27) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  auto orderToClose2 = accountOrders.back();
  fakeQuery->closeOrder(orderToClose.uuid_);
  fakeQuery->closeOrder(orderToClose2.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    return (tradeOrdersHolder.getBuyOrdersCount() == 0);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 1);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);

  tradingManager->reset(true);
  fakeQuery->setBalance(common::Currency::USD, 0);

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  EXPECT_THROW(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC),
               common::exceptions::NoDataFoundException);
}

TEST_F(TradingUIFixture, RestartTradingAndSavePreviousData_28) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  auto orderToClose2 = accountOrders.back();
  fakeQuery->closeOrder(orderToClose.uuid_);
  fakeQuery->closeOrder(orderToClose2.uuid_);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    return (tradeOrdersHolder.getBuyOrdersCount() == 0);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 1);
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);

  tradingManager->reset(false);
  fakeQuery->setBalance(common::Currency::USD, 0);

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  auto &orderProfit2 = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);

  EXPECT_EQ(orderProfit2.getBalance(), 1);
  EXPECT_EQ(orderProfit2.getCurrency(), common::Currency::LTC);
}

TEST_F(TradingUIFixture, StrategyMarket_Strategy_MarketData_Exists_29) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto accountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (accountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  auto &strategyMarketHolder = getTradeSignaledStrategyMarketHolder();
  EXPECT_TRUE(strategyMarketHolder.containMarket(common::Currency::USD, common::Currency::LTC,
                                                 common::StrategiesType::SMA));
  common::MarketData smaMarket = smaCandlesToBuySignalSecond.at(20);
  common::MarketData signaledMarket = strategyMarketHolder.getMarket(
      common::Currency::USD, common::Currency::LTC, common::StrategiesType::SMA);
  EXPECT_TRUE(smaMarket == signaledMarket);
}

TEST_F(TradingUIFixture, StrategyMarket_Strategy_MarketData_Save_And_Restore_30) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);

  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto accountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (accountOrders.size() == 2);
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 2);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  for (auto &marketOrder : accountOrders) {
    EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(marketOrder));
  }

  std::thread tradingThread2(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool { return tradingManager->isRunning(); });

  tradingManager->stopTradingSlot();
  tradingThread2.join();

  auto &strategyMarketHolder = getTradeSignaledStrategyMarketHolder();
  EXPECT_TRUE(strategyMarketHolder.containMarket(common::Currency::USD, common::Currency::LTC,
                                                 common::StrategiesType::SMA));
  common::MarketData smaMarket = smaCandlesToBuySignalSecond.at(20);
  common::MarketData signaledMarket = strategyMarketHolder.getMarket(
      common::Currency::USD, common::Currency::LTC, common::StrategiesType::SMA);
  EXPECT_TRUE(smaMarket == signaledMarket);
}

TEST_F(TradingUIFixture, Reset_Order_Profit_31) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);

  auto orderToClose1 = accountOrders.front();
  auto orderToClose2 = accountOrders.back();
  fakeQuery->closeOrder(orderToClose1.uuid_);
  fakeQuery->closeOrder(orderToClose2.uuid_);

  fakeQuery->setBalance(common::Currency::LTC, 0.2);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyingOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellingOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyingOrdersCount == 0) && (sellingOrdersCount == 0));
  });

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 0);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 0);

  currencyTick.ask_ = 1.10;
  currencyTick.bid_ = 1.08;
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  waitCallbackEvent([&]() -> bool {
    try {
      auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
      return (orderProfit.getBalance() == 0.0);
    } catch (common::exceptions::NoDataFoundException &exception) {
      return false;
    }
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.0);
  EXPECT_TRUE(orderProfit.isEmpty());
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);
}

TEST_F(TradingUIFixture, Sell_Order_Lower_If_Balannce_Is_Not_Big_Enough_32) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 100, 0.1, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 1);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 1);

  fakeQuery->clearMarketHistory(common::Currency::USD, common::Currency::LTC);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  auto &tradeOrdersHolder = getTradeOrdersHolder();

  waitCallbackEvent([&]() -> bool {
    try {
      auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
      return (orderProfit.getBalance() == 1.0);
    } catch (common::exceptions::NoDataFoundException &exception) {
      return false;
    }
  });

  fakeQuery->setBalance(common::Currency::LTC, 0.2);
  currencyTick.ask_ = 1.10;
  currencyTick.bid_ = 1.08;
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  waitCallbackEvent([&]() -> bool {
    try {
      auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
      return (orderProfit.getBalance() == 0.0);
    } catch (common::exceptions::NoDataFoundException &exception) {
      return false;
    }
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &orderProfit = tradeOrdersHolder.getOrdersProfit(common::Currency::LTC);
  EXPECT_EQ(orderProfit.getBalance(), 0.0);
  EXPECT_TRUE(orderProfit.isEmpty());
  EXPECT_EQ(orderProfit.getCurrency(), common::Currency::LTC);

  bool orderMatchingExists = false;
  auto &ordersMatching = tradeOrdersHolder.getOrderMatching();

  ordersMatching.forEachMatching(
      [&](const common::MarketOrder &from, const common::MarketOrder &to) {
        EXPECT_EQ(to, orderToClose);
        EXPECT_EQ(from.quantity_, 0.2);
        orderMatchingExists = true;
      });

  EXPECT_TRUE(orderMatchingExists);
}

TEST_F(TradingUIFixture, Lots_Holder_Should_Correlate_Buy_Orders_33) {
  auto smaSettings = TradingUIFixture::createSmaSettings(10, 0);
  auto customSettings = std::make_unique<model::CustomStrategySettings>();
  customSettings->name_ = "CUSTOM";
  customSettings->strategies_.emplace_back(std::move(smaSettings));
  getStrategySettingsHolder().addCustomStrategySettings(std::move(customSettings));

  auto tradeConfig = std::make_unique<model::TradeConfiguration>();
  tradeConfig->setStrategyName("CUSTOM");

  TradingUIFixture::createStockExchangeSettings(*tradeConfig, common::StockExchangeType::Bittrex,
                                                "api", "secret");
  TradingUIFixture::createBuySettings(*tradeConfig, 3, 3, 1, 50, 0.3, 4);
  TradingUIFixture::createSellSettings(*tradeConfig, 3, 5);
  TradingUIFixture::createCoinSettings(*tradeConfig, common::Currency::USD,
                                       {common::Currency::LTC});

  tradeConfig->setActive(true);

  getTradeConfigsHolder().addTradeConfig(std::move(tradeConfig));

  auto query = getFakeQueryProcessor().getQuery(common::StockExchangeType::Bittrex);
  auto fakeQuery = std::dynamic_pointer_cast<FakeStockExchangeQuery>(query);

  auto marketHistoryPtr = std::make_shared<common::MarketHistory>();
  marketHistoryPtr->toBuy_ = common::Currency::LTC;
  marketHistoryPtr->toSell_ = common::Currency::USD;
  marketHistoryPtr->marketData_ = smaCandlesBuySignal;

  auto marketHistoryPtr2 = std::make_shared<common::MarketHistory>();
  marketHistoryPtr2->toBuy_ = common::Currency::LTC;
  marketHistoryPtr2->toSell_ = common::Currency::USD;
  marketHistoryPtr2->marketData_ = smaCandlesToBuySignalSecond;

  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr);
  fakeQuery->addMarketHistory(common::Currency::USD, common::Currency::LTC, marketHistoryPtr2);

  fakeQuery->setBalance(common::Currency::USD, 1);
  common::CurrencyTick currencyTick{1, 0.9, common::Currency::USD, common::Currency::LTC};

  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, currencyTick);

  getAppSettings().tradingTimeout_ = 0;

  TradingMessageSender sender(getFakeGuiProcessor(), getAppSettings());

  auto &lotsHolder = fakeQuery->takeCurrencyLotsHolder();
  common::LotSize currentLotSize;
  currentLotSize.minQty_ = 0.1;
  currentLotSize.stepSize_ = 0.2;
  lotsHolder.addLot("USD-LTC", currentLotSize);

  auto tradingManager = std::make_unique<TradingManager>(
      getFakeQueryProcessor(), getStrategyFacade(), getDatabase(), getFakeAppController(),
      getFakeGuiProcessor(), getAppSettings(), sender, getStrategySettingsHolder(),
      getTradeOrdersHolder(), getTradeConfigsHolder(), getTradeSignaledStrategyMarketHolder());

  std::thread tradingThread(&TradingManager::startTradingSlot, std::ref(*tradingManager));

  waitCallbackEvent([&]() -> bool {
    auto localAccountOrders =
        fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
    return (localAccountOrders.size() == 2);
  });

  auto accountOrders =
      fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);

  auto orderToClose = accountOrders.front();
  fakeQuery->closeOrder(orderToClose.uuid_);

  common::CurrencyTick newCurrencyTick{1.10, 1.06, common::Currency::USD, common::Currency::LTC};
  fakeQuery->setCurrencyTick(common::Currency::USD, common::Currency::LTC, newCurrencyTick);

  waitCallbackEvent([&]() -> bool {
    auto &tradeOrdersHolder = getTradeOrdersHolder();
    size_t buyOrdersCount = tradeOrdersHolder.getBuyOrdersCount();
    size_t sellOrdersCount = tradeOrdersHolder.getSellOrdersCount();
    return ((buyOrdersCount == 1) && (sellOrdersCount == 1));
  });

  tradingManager->stopTradingSlot();
  tradingThread.join();

  auto &tradeOrdersHolder = getTradeOrdersHolder();
  EXPECT_EQ(tradeOrdersHolder.getBuyOrdersCount(), 1);
  EXPECT_EQ(tradeOrdersHolder.getSellOrdersCount(), 1);
  EXPECT_TRUE(tradeOrdersHolder.getOrdersProfit(common::Currency::LTC).isEmpty());

  accountOrders = fakeQuery->getAccountOpenOrders(common::Currency::USD, common::Currency::LTC);
  EXPECT_EQ(accountOrders.size(), 2);
  for (auto &accountOrder : accountOrders) {
    if (accountOrder.orderType_ == common::OrderType::BUY) {
      EXPECT_TRUE(tradeOrdersHolder.containBuyOrder(accountOrder));
      EXPECT_TRUE(accountOrder.quantity_ = 0.4);
    } else if (accountOrder.orderType_ == common::OrderType::SELL) {
      EXPECT_TRUE(tradeOrdersHolder.containSellOrder(accountOrder));
      EXPECT_TRUE(accountOrder.quantity_ = 0.4);
    } else {
      EXPECT_TRUE(false);
    }
  }

  bool ordersMatched = false;
  auto &orderMatching = tradeOrdersHolder.getOrderMatching();
  orderMatching.forEachMatching(
      [&](const common::MarketOrder &fromOrder, const common::MarketOrder &toOrder) {
        EXPECT_TRUE(tradeOrdersHolder.containSellOrder(fromOrder));
        EXPECT_TRUE(toOrder == orderToClose);
        EXPECT_TRUE(toOrder.quantity_ == 0.4);
        EXPECT_TRUE(fromOrder.quantity_ == 0.4);
        ordersMatched = true;
      });

  EXPECT_EQ(ordersMatched, true);
}

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader