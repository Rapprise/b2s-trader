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

#ifndef AUTO_TRADER_MODEL_CUSTOM_STRATEGY_SETTINGS_H
#define AUTO_TRADER_MODEL_CUSTOM_STRATEGY_SETTINGS_H

#include <memory>
#include <vector>

#include "strategy_settings.h"

namespace auto_trader {
namespace model {

struct CustomStrategySettings : public StrategySettings {
  void accept(StrategySettingsVisitor& visitor) const override { visitor.visit(*this); }

  std::unique_ptr<StrategySettings> clone() const override {
    auto strategySettings = std::make_unique<CustomStrategySettings>();
    strategySettings->tickInterval_ = tickInterval_;
    strategySettings->strategiesType_ = strategiesType_;
    strategySettings->name_ = name_;
    for (int index = 0; index < strategies_.size(); ++index) {
      auto& strategy = strategies_.at(index);
      auto strategyClone = strategy->clone();
      strategySettings->strategies_.push_back(std::move(strategyClone));
    }

    return strategySettings;
  }

  size_t getStrategiesCount() const { return strategies_.size(); }
  const std::unique_ptr<StrategySettings>& getStrategy(int index) const {
    return strategies_[index];
  }

  std::vector<std::unique_ptr<StrategySettings>> strategies_;
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_MODEL_CUSTOM_STRATEGY_SETTINGS_H
