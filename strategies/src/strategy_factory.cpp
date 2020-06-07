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

#include "include/strategy_factory.h"

#include "common/exceptions/undefined_type_exception.h"
#include "include/bollinger_bands/bollinger_bands.h"
#include "include/bollinger_bands_advance/bollinger_bands_advance.h"
#include "include/exponential_moving_average/exponential_moving_average.h"
#include "include/macd/macd.h"
#include "include/moving_averages_crossing/moving_averages_crossing.h"
#include "include/rsi/rsi.h"
#include "include/simple_moving_average/simple_moving_average.h"
#include "include/stochastic_oscillator/stochastic_oscillator.h"
#include "resources/resources.h"

namespace auto_trader {
namespace strategies {

std::shared_ptr<TradeStrategy> StrategyFactory::createStrategy(common::StrategiesType type) const {
  switch (type) {
    case common::StrategiesType::BOLLINGER_BANDS:
      return std::make_shared<BollingerBands>();
    case common::StrategiesType::BOLLINGER_BANDS_ADVANCED:
      return std::make_shared<BollingerBandsAdvance>();
    case common::StrategiesType::RSI:
      return std::make_shared<Rsi>();
    case common::StrategiesType::SMA:
      return std::make_shared<SimpleMovingAverage>();
    case common::StrategiesType::EMA:
      return std::make_shared<ExponentialMovingAverage>();
    case common::StrategiesType::MA_CROSSING:
      return std::make_shared<MovingAveragesCrossing>();
    case common::StrategiesType::STOCHASTIC_OSCILLATOR:
      return std::make_shared<StochasticOscillator>();
    case common::StrategiesType::MACD:
      return std::make_shared<Macd>();
    default:
      throw common::exceptions::UndefinedTypeException(resources::keywords::STRATEGY_TYPE);
  }
}

}  // namespace strategies
}  // namespace auto_trader