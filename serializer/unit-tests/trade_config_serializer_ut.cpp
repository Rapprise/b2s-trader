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

#include "trade_config_serializer_ut.h"

#include <fstream>

#include "model/include/settings/strategies_settings/ema_settings.h"
#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace serializer {
namespace unit_tests {

/**
 *
 *      Test Plan
 *
 * 1. Buy settings.
 * 2. Sell settings.
 * 3. Coin Settings.
 * 4. Stock Exchange settings.
 * 5. Strategy settings ( TODO ).
 *
 **/

TEST_F(TradeConfigSerializerUTFixture, BuySettings) {
  model::TradeConfiguration configuration;
  configuration.setName("config1");
  auto& buySettings = configuration.takeBuySettings();
  buySettings.percentageBuyAmount_ = 23.539999999999999;
  buySettings.openPositionAmountPerCoins_ = 2;
  buySettings.minOrderPrice_ = 15.325200000000001;
  buySettings.maxOpenTime_ = 24;
  buySettings.maxOpenOrders_ = 56;
  buySettings.maxCoinAmount_ = 35;

  std::ofstream stream("trade_config_settings.json");
  getTradeConfigSerializer().serialize(configuration, stream);
  stream.close();

  std::ifstream inputStream("trade_config_settings.json");
  auto restoredConfiguration = getTradeConfigSerializer().deserialize(inputStream);

  EXPECT_EQ(restoredConfiguration->getName(), "config1");

  const auto& restoredBuySettings = restoredConfiguration->getBuySettings();
  EXPECT_EQ(restoredBuySettings.percentageBuyAmount_, 23.539999999999999);
  EXPECT_EQ(restoredBuySettings.openPositionAmountPerCoins_, 2);
  EXPECT_EQ(restoredBuySettings.minOrderPrice_, 15.325200000000001);
  EXPECT_EQ(restoredBuySettings.maxOpenTime_, 24);
  EXPECT_EQ(restoredBuySettings.maxOpenOrders_, 56);
  EXPECT_EQ(restoredBuySettings.maxCoinAmount_, 35);
}

TEST_F(TradeConfigSerializerUTFixture, SellSettings) {
  model::TradeConfiguration configuration;
  configuration.setName("config2");
  auto& sellSettings = configuration.takeSellSettings();
  sellSettings.profitPercentage_ = 3.4325199999999998;
  sellSettings.openOrderTime_ = 3112;

  std::ofstream stream("trade_config_settings.json");
  getTradeConfigSerializer().serialize(configuration, stream);
  stream.close();

  std::ifstream inputStream("trade_config_settings.json");
  auto restoredConfiguration = getTradeConfigSerializer().deserialize(inputStream);

  EXPECT_EQ(restoredConfiguration->getName(), "config2");

  const auto& restoredSellSettings = restoredConfiguration->getSellSettings();
  EXPECT_EQ(restoredSellSettings.openOrderTime_, 3112);
  EXPECT_EQ(restoredSellSettings.profitPercentage_, 3.4325199999999998);
}

TEST_F(TradeConfigSerializerUTFixture, CoinSettings) {
  model::TradeConfiguration configuration;
  configuration.setName("config3");

  auto& coinSettings = configuration.takeCoinSettings();
  coinSettings.baseCurrency_ = common::Currency::BTC;
  coinSettings.tradedCurrencies_.push_back(common::Currency::LTC);
  coinSettings.tradedCurrencies_.push_back(common::Currency::USD);

  std::ofstream stream("trade_config_settings.json");
  getTradeConfigSerializer().serialize(configuration, stream);
  stream.close();

  std::ifstream inputStream("trade_config_settings.json");
  auto restoredConfiguration = getTradeConfigSerializer().deserialize(inputStream);

  EXPECT_EQ(restoredConfiguration->getName(), "config3");

  const auto& restoredCoinSettings = restoredConfiguration->getCoinSettings();
  EXPECT_EQ(restoredCoinSettings.baseCurrency_, common::Currency::BTC);
  EXPECT_EQ(restoredCoinSettings.tradedCurrencies_[0], common::Currency::LTC);
  EXPECT_EQ(restoredCoinSettings.tradedCurrencies_[1], common::Currency::USD);
}

TEST_F(TradeConfigSerializerUTFixture, StockExchangeSettings) {
  model::TradeConfiguration configuration;
  configuration.setName("config4");

  auto& stockExchangeSettings = configuration.takeStockExchangeSettings();
  stockExchangeSettings.secretKey_ = "secret_key_3253$31!";
  stockExchangeSettings.apiKey_ = "api_29359@!#%$%#";
  stockExchangeSettings.stockExchangeType_ = common::StockExchangeType::Binance;

  std::ofstream stream("trade_config_settings.json");
  getTradeConfigSerializer().serialize(configuration, stream);
  stream.close();

  std::ifstream inputStream("trade_config_settings.json");
  auto restoredConfiguration = getTradeConfigSerializer().deserialize(inputStream);

  EXPECT_EQ(restoredConfiguration->getName(), "config4");

  const auto& restoredStockExchangeSettings = restoredConfiguration->getStockExchangeSettings();
  EXPECT_EQ(restoredStockExchangeSettings.secretKey_, "secret_key_3253$31!");
  EXPECT_EQ(restoredStockExchangeSettings.apiKey_, "api_29359@!#%$%#");
  EXPECT_EQ(restoredStockExchangeSettings.stockExchangeType_, common::StockExchangeType::Binance);
}

}  // namespace unit_tests
}  // namespace serializer
}  // namespace auto_trader