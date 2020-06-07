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

#ifndef TRADING_MODEL_TRADING_CONFIGURATION_H
#define TRADING_MODEL_TRADING_CONFIGURATION_H

#include <atomic>
#include <memory>

#include "settings/buy_settings.h"
#include "settings/coin_settings.h"
#include "settings/sell_settings.h"
#include "settings/stock_exchange_settings.h"
#include "settings/strategies_settings/strategy_settings.h"

namespace auto_trader {
namespace model {

class TradeConfiguration {
 public:
  explicit TradeConfiguration() = default;

  const BuySettings& getBuySettings() const;
  const SellSettings& getSellSettings() const;
  const CoinSettings& getCoinSettings() const;
  const StockExchangeSettings& getStockExchangeSettings() const;

  BuySettings& takeBuySettings();
  SellSettings& takeSellSettings();
  CoinSettings& takeCoinSettings();
  StockExchangeSettings& takeStockExchangeSettings();

  void setName(const std::string& name);
  const std::string& getName() const;

  void setDescription(const std::string& description);
  const std::string& getDescription() const;

  void setStrategyName(const std::string& name);
  const std::string& getStrategyName() const;

  void setActive(bool isActive);

  void start();
  void stop();

  bool isRunning() const;
  bool isActive() const;

 private:
  BuySettings buySettings_{0, 0, 0, 0.0, 0.0, 0.0};
  SellSettings sellSettings_{0.0, 0};
  CoinSettings coinSettings_{common::Currency::UNKNOWN, {}};
  StockExchangeSettings stockExchangeSettings_{"", "", common::StockExchangeType::UNKNOWN};

  std::string strategyName_;
  std::string name_;
  std::string description_;
  std::atomic<bool> isRunning_;
  std::atomic<bool> isActive_;
};

}  // namespace model
}  // namespace auto_trader

#endif  // TRADING_MODEL_TRADING_CONFIGURATION_H
