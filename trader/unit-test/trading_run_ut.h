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

#ifndef AUTO_TRADER_TRADER_TRADING_RUN_UT_H
#define AUTO_TRADER_TRADER_TRADING_RUN_UT_H

#include <gtest/gtest.h>

#include <vector>

#include "common/market_data.h"
#include "database/include/database.h"
#include "fake/fake_app_controller.h"
#include "fake/fake_gui_processor.h"
#include "fake/fake_query_processor.h"
#include "fake/fake_stock_exchange_query.h"
#include "model/include/holders/strategies_settings_holder.h"
#include "model/include/holders/trade_configs_holder.h"
#include "model/include/holders/trade_orders_holder.h"
#include "model/include/holders/trade_signaled_strategy_market_holder.h"
#include "model/include/settings/app_settings.h"
#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/settings/strategies_settings/rsi_settings.h"
#include "model/include/settings/strategies_settings/sma_settings.h"
#include "stocks_exchange/include/query_factory.h"
#include "stocks_exchange/include/query_processor.h"
#include "strategies/include/strategy_facade.h"

namespace auto_trader {
namespace trader {
namespace unit_test {

std::vector<common::MarketData> smaCandlesBuySignal = {
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
    {3915.000, 3935.770, 3915.000, 3935.770, 68510.95386}};

std::vector<common::MarketData> smaCandlesToBuySignalSecond = {
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
    {3916.000, 3985.770, 3915.000, 3935.872, 68510.95387}};

std::vector<common::MarketData> smaCandlesWithoutBuySignal = {
    {22.27, 22.27, 22.27, 22.27, 22.27}, {22.19, 22.19, 22.19, 22.19, 22.19},
    {22.08, 22.08, 22.08, 22.08, 22.08}, {22.17, 22.17, 22.17, 22.17, 22.17},
    {22.18, 22.18, 22.18, 22.18, 22.18}, {22.13, 22.13, 22.13, 22.13, 22.13},
    {22.23, 22.23, 22.23, 22.23, 22.23}, {22.43, 22.43, 22.43, 22.43, 22.43},
    {22.24, 22.24, 22.24, 22.24, 22.24}, {22.29, 22.29, 22.29, 22.29, 22.29},
    {22.15, 22.15, 22.15, 22.15, 22.15}, {22.39, 22.39, 22.39, 22.39, 22.39},
    {22.38, 22.38, 22.38, 22.38, 22.38}, {22.61, 22.61, 22.61, 22.61, 22.61}};

class TradingUIFixture : public ::testing::Test {
 public:
  void SetUp() override {
    remove("b2s_trader.db");

    factory_ = std::make_unique<stock_exchange::QueryFactory>();
    database_ = std::make_unique<database::Database>();
    queryProcessor_ = std::make_unique<FakeQueryProcessor>(*factory_);
    strategiesFacade_ = std::make_unique<strategies::StrategyFacade>();
    appController_ = std::make_unique<FakeAppController>();
    guiProcessor_ = std::make_unique<FakeGuiProcessor>();
    strategySettingsHolder_ = std::make_unique<model::StrategiesSettingsHolder>();
    tradeConfigurationsHolder_ = std::make_unique<model::TradeConfigsHolder>();
    tradeOrdersHolder_ = std::make_unique<model::TradeOrdersHolder>();
    tradeSignaledStrategyMarketHolder_ =
        std::make_unique<model::TradeSignaledStrategyMarketHolder>();

    appSettings_ = std::make_unique<model::AppSettings>();
  }

  void TearDown() override {
    database_.reset();
    queryProcessor_.reset();
    strategiesFacade_.reset();
    strategySettingsHolder_.reset();
    tradeConfigurationsHolder_.reset();
    tradeSignaledStrategyMarketHolder_.reset();
    appSettings_.reset();
  }

  database::Database& getDatabase() const {
    EXPECT_TRUE(database_);
    return *database_;
  }

  FakeGuiProcessor& getFakeGuiProcessor() const {
    EXPECT_TRUE(guiProcessor_);
    return *guiProcessor_;
  }

  FakeAppController& getFakeAppController() const {
    EXPECT_TRUE(appController_);
    return *appController_;
  }

  FakeQueryProcessor& getFakeQueryProcessor() const {
    EXPECT_TRUE(queryProcessor_);
    return *queryProcessor_;
  }

  strategies::StrategyFacade& getStrategyFacade() const {
    EXPECT_TRUE(strategiesFacade_);
    return *strategiesFacade_;
  }

  model::StrategiesSettingsHolder& getStrategySettingsHolder() const {
    EXPECT_TRUE(strategySettingsHolder_);
    return *strategySettingsHolder_;
  }

  model::TradeConfigsHolder& getTradeConfigsHolder() const {
    EXPECT_TRUE(tradeConfigurationsHolder_);
    return *tradeConfigurationsHolder_;
  }

  model::TradeOrdersHolder& getTradeOrdersHolder() {
    EXPECT_TRUE(tradeOrdersHolder_);
    return *tradeOrdersHolder_;
  }

  model::TradeSignaledStrategyMarketHolder& getTradeSignaledStrategyMarketHolder() {
    EXPECT_TRUE(tradeSignaledStrategyMarketHolder_);
    return *tradeSignaledStrategyMarketHolder_;
  }

  model::AppSettings& getAppSettings() {
    EXPECT_TRUE(appSettings_);
    return *appSettings_;
  }

  static model::BuySettings& createBuySettings(model::TradeConfiguration& tradeConfiguration,
                                               unsigned int maxOpenOrders,
                                               unsigned int maxOpenOrderTime, double maxCoinAmount,
                                               double percentrageBuyAmount, double minOrderPrice,
                                               unsigned int openPositionsPerCoin) {
    auto& buySettings = tradeConfiguration.takeBuySettings();
    buySettings.maxOpenOrders_ = maxOpenOrders;
    buySettings.maxOpenTime_ = maxOpenOrderTime;
    buySettings.maxCoinAmount_ = maxCoinAmount;
    buySettings.percentageBuyAmount_ = percentrageBuyAmount;
    buySettings.minOrderPrice_ = minOrderPrice;
    buySettings.openPositionAmountPerCoins_ = openPositionsPerCoin;

    return buySettings;
  }

  static model::SellSettings& createSellSettings(model::TradeConfiguration& tradeConfiguration,
                                                 unsigned int openOrderTime,
                                                 double profitPercentage) {
    auto& sellSettings = tradeConfiguration.takeSellSettings();
    sellSettings.openOrderTime_ = openOrderTime;
    sellSettings.profitPercentage_ = profitPercentage;
    return sellSettings;
  }

  static model::StockExchangeSettings& createStockExchangeSettings(
      model::TradeConfiguration& tradeConfiguration, common::StockExchangeType type,
      const std::string& apiKey, const std::string& secretKey) {
    auto& stockExchangeSettings = tradeConfiguration.takeStockExchangeSettings();
    stockExchangeSettings.apiKey_ = apiKey;
    stockExchangeSettings.secretKey_ = secretKey;
    stockExchangeSettings.stockExchangeType_ = type;
    return stockExchangeSettings;
  }

  static model::CoinSettings& createCoinSettings(
      model::TradeConfiguration& tradeConfiguration, common::Currency::Enum baseCurrency,
      const std::vector<common::Currency::Enum>& tradedCurrency) {
    auto& coinSettings = tradeConfiguration.takeCoinSettings();
    coinSettings.baseCurrency_ = baseCurrency;
    coinSettings.tradedCurrencies_ = tradedCurrency;
    return coinSettings;
  }

  static std::unique_ptr<model::SmaSettings> createSmaSettings(unsigned int period,
                                                               unsigned int crossingInterval) {
    auto smaSettings = std::make_unique<model::SmaSettings>();
    smaSettings->period_ = period;
    smaSettings->crossingInterval_ = crossingInterval;
    smaSettings->name_ = "SMA";
    return std::move(smaSettings);
  }

  static std::unique_ptr<model::EmaSettings> createEmaSettings(unsigned int period,
                                                               unsigned int crossingInterval) {
    auto emaSettings = std::make_unique<model::EmaSettings>();
    emaSettings->period_ = period;
    emaSettings->crossingInterval_ = crossingInterval;
    emaSettings->name_ = "EMA";
    emaSettings->strategiesType_ = common::StrategiesType::EMA;
    return std::move(emaSettings);
  }

  static std::unique_ptr<model::RsiSettings> createRsiSettings(unsigned int period,
                                                               unsigned int crossingInterval,
                                                               unsigned int topLevel,
                                                               unsigned int bottomLevel) {
    auto rsiSettings = std::make_unique<model::RsiSettings>();
    rsiSettings->name_ = "RSI";
    rsiSettings->period_ = period;
    rsiSettings->strategiesType_ = common::StrategiesType::RSI;
    rsiSettings->crossingInterval_ = crossingInterval;
    rsiSettings->topLevel_ = topLevel;
    rsiSettings->bottomLevel_ = bottomLevel;
    return std::move(rsiSettings);
  }

 private:
  std::unique_ptr<stock_exchange::QueryFactory> factory_;
  std::unique_ptr<database::Database> database_;
  std::unique_ptr<FakeQueryProcessor> queryProcessor_;
  std::unique_ptr<FakeGuiProcessor> guiProcessor_;
  std::unique_ptr<FakeAppController> appController_;
  std::unique_ptr<strategies::StrategyFacade> strategiesFacade_;
  std::unique_ptr<model::StrategiesSettingsHolder> strategySettingsHolder_;
  std::unique_ptr<model::TradeConfigsHolder> tradeConfigurationsHolder_;
  std::unique_ptr<model::TradeOrdersHolder> tradeOrdersHolder_;
  std::unique_ptr<model::TradeSignaledStrategyMarketHolder> tradeSignaledStrategyMarketHolder_;
  std::unique_ptr<model::AppSettings> appSettings_;
};

}  // namespace unit_test
}  // namespace trader
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADER_TRADING_RUN_UT_H
