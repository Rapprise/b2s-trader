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

#include "model/include/holders/strategies_settings_holder.h"

#include <functional>

#include "common/exceptions/model_exception/unknown_strategy_name_exception.h"

namespace auto_trader {
namespace model {

void StrategiesSettingsHolder::addCustomStrategySettings(
    std::unique_ptr<auto_trader::model::CustomStrategySettings> settings) {
  strategiesSettings_[settings->name_] = std::move(settings);
}

const model::CustomStrategySettings &StrategiesSettingsHolder::getCustomStrategy(
    const std::string &strategyName) const {
  auto iter = strategiesSettings_.find(strategyName);
  if (iter != strategiesSettings_.end()) {
    return *iter->second;
  }

  throw common::exceptions::UnknownStrategyNameException(strategyName);
}

model::CustomStrategySettings &StrategiesSettingsHolder::takeCustomStrategy(
    const std::string &strategyName) {
  auto iter = strategiesSettings_.find(strategyName);
  if (iter != strategiesSettings_.end()) {
    return *iter->second;
  }

  throw common::exceptions::UnknownStrategyNameException(strategyName);
}

void StrategiesSettingsHolder::removeCustomStrategy(const std::string &strategyName) {
  strategiesSettings_.erase(strategyName);
}

void StrategiesSettingsHolder::forEachStrategy(
    std::function<void(model::CustomStrategySettings &)> callback) const {
  for (auto &strategySetting : strategiesSettings_) {
    model::CustomStrategySettings &strategy = *strategySetting.second;
    callback(strategy);
  }
}

bool StrategiesSettingsHolder::isEmpty() const { return strategiesSettings_.empty(); }

bool StrategiesSettingsHolder::containsStrategy(const std::string &strategyName) const {
  return (strategiesSettings_.find(strategyName) != strategiesSettings_.end());
}

}  // namespace model
}  // namespace auto_trader