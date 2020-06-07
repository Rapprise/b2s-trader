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

#ifndef AUTO_TRADER_MODEL_STRATEGY_SETTINGS_FACTORY_H
#define AUTO_TRADER_MODEL_STRATEGY_SETTINGS_FACTORY_H

#include <memory>

#include "bollinger_bands_advanced_settings.h"
#include "bollinger_bands_settings.h"
#include "common/enumerations/strategies_type.h"
#include "common/exceptions/undefined_type_exception.h"
#include "custom_strategy_settings.h"
#include "ema_settings.h"
#include "ma_crossing_settings.h"
#include "rsi_settings.h"
#include "sma_settings.h"
#include "stochastic_oscillator_settings.h"
#include "strategy_settings.h"

namespace auto_trader {
namespace model {

class StrategySettingsFactory {
 public:
  std::unique_ptr<StrategySettings> createStrategySettings(common::StrategiesType type) {
    switch (type) {
      case common::StrategiesType::BOLLINGER_BANDS:
        return std::make_unique<BollingerBandsSettings>();
      case common::StrategiesType::BOLLINGER_BANDS_ADVANCED:
        return std::make_unique<BollingerBandsAdvancedSettings>();
      case common::StrategiesType::RSI:
        return std::make_unique<RsiSettings>();
      case common::StrategiesType::MA_CROSSING:
        return std::make_unique<MovingAveragesCrossingSettings>();
      case common::StrategiesType::EMA:
        return std::make_unique<EmaSettings>();
      case common::StrategiesType::STOCHASTIC_OSCILLATOR:
        return std::make_unique<StochasticOscillatorSettings>();
      case common::StrategiesType::SMA:
        return std::make_unique<SmaSettings>();
      case common::StrategiesType::CUSTOM:
        return std::make_unique<CustomStrategySettings>();

      default:
        throw common::exceptions::UndefinedTypeException("Strategy type");
    }
  }
};

}  // namespace model
}  // namespace auto_trader

#endif  // AUTO_TRADER_MODEL_STRATEGY_SETTINGS_FACTORY_H
