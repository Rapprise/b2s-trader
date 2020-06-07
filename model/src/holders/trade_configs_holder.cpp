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

#include "model/include/holders/trade_configs_holder.h"

#include "common/exceptions/model_exception/no_active_trading_config_exception.h"
#include "common/exceptions/model_exception/unknown_trading_config_name_exception.h"

namespace auto_trader {
namespace model {

void TradeConfigsHolder::addTradeConfig(
    std::unique_ptr<auto_trader::model::TradeConfiguration> config) {
  tradeConfigurations_[config->getName()] = std::move(config);
}

const model::TradeConfiguration &TradeConfigsHolder::getTradeConfiguration(
    const std::string &configurationName) const {
  auto iter = tradeConfigurations_.find(configurationName);
  if (iter != tradeConfigurations_.end()) {
    return *iter->second;
  }

  throw common::exceptions::UnknownTradingConfigNameException(configurationName);
}

model::TradeConfiguration &TradeConfigsHolder::takeTradeConfiguration(
    const std::string &configName) {
  auto iter = tradeConfigurations_.find(configName);
  if (iter != tradeConfigurations_.end()) {
    return *iter->second;
  }

  throw common::exceptions::UnknownTradingConfigNameException(configName);
}

model::TradeConfiguration &TradeConfigsHolder::takeCurrentTradeConfiguration() {
  for (auto &configurationPair : tradeConfigurations_) {
    model::TradeConfiguration &configuration = *configurationPair.second;
    if (configuration.isActive()) {
      return configuration;
    }
  }

  throw common::exceptions::NoActiveTradingConfigException();
}

bool TradeConfigsHolder::containsConfiguration(const std::string &configName) {
  auto it = tradeConfigurations_.find(configName);
  return it != tradeConfigurations_.end();
}

void TradeConfigsHolder::removeTradeConfig(const std::string &configName) {
  tradeConfigurations_.erase(configName);
}

void TradeConfigsHolder::forEachTradeConfiguration(
    std::function<void(model::TradeConfiguration &)> callback) const {
  for (auto &configurationPair : tradeConfigurations_) {
    model::TradeConfiguration &configuration = *configurationPair.second;
    callback(configuration);
  }
}

model::TradeConfiguration const &TradeConfigsHolder::getCurrentTradeConfiguration() const {
  for (auto &configurationPair : tradeConfigurations_) {
    model::TradeConfiguration &configuration = *configurationPair.second;
    if (configuration.isActive()) {
      return configuration;
    }
  }

  throw common::exceptions::NoActiveTradingConfigException();
}

size_t TradeConfigsHolder::getConfigurationsCount() const { return tradeConfigurations_.size(); }

bool TradeConfigsHolder::isEmpty() const { return tradeConfigurations_.empty(); }

void TradeConfigsHolder::clear() { tradeConfigurations_.clear(); }

void TradeConfigsHolder::setDefaultActiveConfiguration() {
  if (tradeConfigurations_.empty()) return;

  for (auto &tradeConfiguration : tradeConfigurations_) {
    tradeConfiguration.second->setActive(false);
  }

  auto &firstElem = *tradeConfigurations_.begin();
  firstElem.second->setActive(true);
}

}  // namespace model
}  // namespace auto_trader