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

#ifndef AUTO_TRADER_STRATEGIES_SETTINGS_HOLDER_H
#define AUTO_TRADER_STRATEGIES_SETTINGS_HOLDER_H

#include <functional>
#include <map>
#include <memory>

#include "model/include/settings/strategies_settings/custom_strategy_settings.h"

namespace auto_trader {
namespace model {

class StrategiesSettingsHolder {
 public:
  StrategiesSettingsHolder() = default;
  ~StrategiesSettingsHolder() = default;

  StrategiesSettingsHolder(const StrategiesSettingsHolder &) = delete;
  StrategiesSettingsHolder &operator=(const StrategiesSettingsHolder &) = delete;

 public:
  void addCustomStrategySettings(std::unique_ptr<model::CustomStrategySettings> settings);
  const model::CustomStrategySettings &getCustomStrategy(const std::string &strategyName) const;
  model::CustomStrategySettings &takeCustomStrategy(const std::string &strategyName);
  void removeCustomStrategy(const std::string &strategyName);

  void forEachStrategy(std::function<void(model::CustomStrategySettings &)> callback) const;

  bool containsStrategy(const std::string &strategyName) const;
  bool isEmpty() const;

 private:
  std::map<std::string, std::unique_ptr<model::CustomStrategySettings>> strategiesSettings_;
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_STRATEGIES_SETTINGS_HOLDER_H
