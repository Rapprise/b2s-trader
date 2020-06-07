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

#include "include/exponential_moving_average/exponential_moving_average.h"

#include "common/exceptions/strategy_exception/small_analyzed_period_exception.h"
#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "common/loggers/file_logger.h"

namespace auto_trader {
namespace strategies {

constexpr double CALCULATION_CONSTANT_TWO_FOR_EMA = 2.0;
constexpr double CALCULATION_CONSTANT_ONE_FOR_EMA = 1.0;

void ExponentialMovingAverage::createLine(const std::vector<common::MarketData>& marketData,
                                          int period, size_t crossingInterval,
                                          double lastBuyCrossingPoint,
                                          double lastSellCrossingPoint) {
  crossingForSellSignal_.second = false;
  crossingForBuySignal_.second = false;

  if (marketData.size() == 0 || marketData.size() < period) {
    common::loggers::FileLogger::getLogger() << "EMA: bad data for creating line";
    throw common::exceptions::StrategyException(
        "Exponential Moving Average: not valid data for creating lines");
  }

  calculateLastMarketData(marketData);

  crossingInterval_ = crossingInterval;
  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;
  movingAverageLine_.clear();
  movingAverageLine_.setPeriod(period);

  size_t marketTicksCount = marketData.size();
  if (marketTicksCount < period)
    throw common::exceptions::StrategyException(
        "Exponential Moving Average: market ticks count less than strategy period");

  if (crossingInterval_ > marketTicksCount)
    throw common::exceptions::StrategyException(
        "Exponential Moving Average:crossing interval bigger than market ticks count");

  for (int index = 0; index < marketTicksCount; ++index) {
    if ((index + period) > marketTicksCount) {
      break;
    }
    calculateEma(marketData, index, period);
  }

  crossingToBuySignal();
  crossingToSellSignal();
}

void ExponentialMovingAverage::calculateEma(const std::vector<common::MarketData>& marketData,
                                            unsigned int fromIndex, unsigned int period) {
  if (movingAverageLine_.empty()) {
    calculateSma(marketData, fromIndex, period);
    return;
  }

  double emaMultiplier =
      CALCULATION_CONSTANT_TWO_FOR_EMA / (period + CALCULATION_CONSTANT_ONE_FOR_EMA);
  unsigned int toPeriod = fromIndex + period;

  if (marketData.size() < toPeriod) {
    throw common::exceptions::SmallAnalyzedPeriodException(std::to_string(marketData.size()));
  }

  double ema =
      (marketData[toPeriod - 1].closePrice_ - movingAverageLine_.getLastPoint()) * emaMultiplier +
      movingAverageLine_.getLastPoint();
  movingAverageLine_.addPoint(ema);
}

}  // namespace strategies
}  // namespace auto_trader