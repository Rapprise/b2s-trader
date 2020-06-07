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


#ifndef AUTO_TRADER_STRATEGIES_STRATEGY_FACADE_H
#define AUTO_TRADER_STRATEGIES_STRATEGY_FACADE_H

#include <map>

#include "common/enumerations/strategies_type.h"
#include "strategies/include/bollinger_bands/bollinger_bands_base.h"
#include "strategies/include/bollinger_bands_advance/bollinger_bands_advance.h"
#include "strategies/include/exponential_moving_average/exponential_moving_average.h"
#include "strategies/include/macd/macd.h"
#include "strategies/include/moving_averages_crossing/moving_averages_crossing.h"
#include "strategies/include/rsi/rsi_base.h"
#include "strategies/include/simple_moving_average/simple_moving_average.h"
#include "strategies/include/stochastic_oscillator/stochastic_oscillator.h"
#include "strategies/include/strategy_factory.h"

namespace auto_trader {
namespace strategies {

class StrategyFactory;

class StrategyFacade {
 public:
  StrategyFacade();

  std::shared_ptr<BollingerBandsBase> getBollingerBandStrategy();
  std::shared_ptr<BollingerBandsBase> getBollingerBandAdvanceStrategy();
  std::shared_ptr<RsiBase> getRsiStrategy();
  std::shared_ptr<SimpleMovingAverage> getSmaStrategy();
  std::shared_ptr<ExponentialMovingAverage> getEmaStrategy();
  std::shared_ptr<MovingAveragesCrossing> getMACrossingStrategy();
  std::shared_ptr<StochasticOscillator> getStochasticOscillatorStrategy();
  std::shared_ptr<Macd> getMacdStrategy();

 private:
  std::shared_ptr<TradeStrategy> getTradeStrategy(common::StrategiesType type);

  template <typename T>
  std::shared_ptr<T> castStrategy(std::shared_ptr<TradeStrategy> strategy);

 private:
  std::unique_ptr<StrategyFactory> factory_;
  std::map<common::StrategiesType, std::shared_ptr<TradeStrategy>> strategies_;
};

template <typename T>
std::shared_ptr<T> StrategyFacade::castStrategy(
    std::shared_ptr<auto_trader::strategies::TradeStrategy> strategy) {
  return std::static_pointer_cast<T>(strategy);
}

}  // namespace strategies
}  // namespace auto_trader

#endif  // AUTO_TRADER_STRATEGIES_STRATEGY_FACADE_H
