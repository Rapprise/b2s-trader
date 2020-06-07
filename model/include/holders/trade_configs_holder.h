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

#ifndef AUTO_TRADER_TRADE_CONFIG_HOLDER_H
#define AUTO_TRADER_TRADE_CONFIG_HOLDER_H

#include <functional>

#include "model/include/trade_configuration.h"

namespace auto_trader {
namespace model {

class TradeConfigsHolder {
 public:
  TradeConfigsHolder() = default;
  ~TradeConfigsHolder() = default;

  TradeConfigsHolder(const TradeConfigsHolder &) = delete;
  TradeConfigsHolder &operator=(const TradeConfigsHolder &) = delete;

 public:
  void addTradeConfig(std::unique_ptr<model::TradeConfiguration> config);
  void removeTradeConfig(const std::string &configName);

  void setDefaultActiveConfiguration();

  const model::TradeConfiguration &getTradeConfiguration(const std::string &configName) const;
  const model::TradeConfiguration &getCurrentTradeConfiguration() const;

  model::TradeConfiguration &takeTradeConfiguration(const std::string &configName);
  model::TradeConfiguration &takeCurrentTradeConfiguration();
  bool containsConfiguration(const std::string &configName);

  size_t getConfigurationsCount() const;

  void forEachTradeConfiguration(std::function<void(model::TradeConfiguration &)> callback) const;
  void clear();

  bool isEmpty() const;

 private:
  std::map<std::string, std::unique_ptr<model::TradeConfiguration>> tradeConfigurations_;
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_TRADE_CONFIG_HOLDER_H
