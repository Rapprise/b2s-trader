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

#include "include/strategy_facade.h"

#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {

StrategyFacade::StrategyFacade() : factory_(new StrategyFactory()) {}

std::shared_ptr<BollingerBandsBase> StrategyFacade::getBollingerBandStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::BOLLINGER_BANDS);
  return castStrategy<BollingerBandsBase>(strategy);
}

std::shared_ptr<BollingerBandsBase> StrategyFacade::getBollingerBandAdvanceStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::BOLLINGER_BANDS_ADVANCED);
  return castStrategy<BollingerBandsBase>(strategy);
}

std::shared_ptr<RsiBase> StrategyFacade::getRsiStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::RSI);
  return castStrategy<RsiBase>(strategy);
}

std::shared_ptr<SimpleMovingAverage> StrategyFacade::getSmaStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::SMA);
  return castStrategy<SimpleMovingAverage>(strategy);
}

std::shared_ptr<ExponentialMovingAverage> StrategyFacade::getEmaStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::EMA);
  return castStrategy<ExponentialMovingAverage>(strategy);
}

std::shared_ptr<MovingAveragesCrossing> StrategyFacade::getMACrossingStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::MA_CROSSING);
  return castStrategy<MovingAveragesCrossing>(strategy);
}

std::shared_ptr<StochasticOscillator> StrategyFacade::getStochasticOscillatorStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::STOCHASTIC_OSCILLATOR);
  return castStrategy<StochasticOscillator>(strategy);
}

std::shared_ptr<Macd> StrategyFacade::getMacdStrategy() {
  auto strategy = getTradeStrategy(common::StrategiesType::MACD);
  return castStrategy<Macd>(strategy);
}

std::shared_ptr<TradeStrategy> StrategyFacade::getTradeStrategy(common::StrategiesType type) {
  auto strategiesIterator = strategies_.find(type);
  if (strategiesIterator == strategies_.end()) {
    auto strategy = factory_->createStrategy(type);
    strategies_[type] = strategy;
    return strategy;
  }

  return strategiesIterator->second;
}

}  // namespace strategies
}  // namespace auto_trader