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

#include "include/trade_configuration.h"

namespace auto_trader {
namespace model {

const BuySettings& TradeConfiguration::getBuySettings() const { return buySettings_; }

const SellSettings& TradeConfiguration::getSellSettings() const { return sellSettings_; }

const CoinSettings& TradeConfiguration::getCoinSettings() const { return coinSettings_; }

const StockExchangeSettings& TradeConfiguration::getStockExchangeSettings() const {
  return stockExchangeSettings_;
}

BuySettings& TradeConfiguration::takeBuySettings() { return buySettings_; }

SellSettings& TradeConfiguration::takeSellSettings() { return sellSettings_; }

CoinSettings& TradeConfiguration::takeCoinSettings() { return coinSettings_; }

StockExchangeSettings& TradeConfiguration::takeStockExchangeSettings() {
  return stockExchangeSettings_;
}

void TradeConfiguration::setName(const std::string& name) { name_ = name; }

const std::string& TradeConfiguration::getName() const { return name_; }

void TradeConfiguration::setDescription(const std::string& description) {
  description_ = description;
}

const std::string& TradeConfiguration::getDescription() const { return description_; }

void TradeConfiguration::setStrategyName(const std::string& name) { strategyName_ = name; }

const std::string& TradeConfiguration::getStrategyName() const { return strategyName_; }

void TradeConfiguration::start() { isRunning_ = true; }

void TradeConfiguration::stop() { isRunning_ = false; }

bool TradeConfiguration::isRunning() const { return isRunning_; }

bool TradeConfiguration::isActive() const { return isActive_; }

void TradeConfiguration::setActive(bool isActive) { isActive_ = isActive; }

}  // namespace model
}  // namespace auto_trader