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

#include "include/simple_moving_average/simple_moving_average.h"

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "common/loggers/file_logger.h"

namespace auto_trader {
namespace strategies {

void SimpleMovingAverage::createLine(const std::vector<common::MarketData>& marketData, int period,
                                     size_t crossingInterval, double lastBuyCrossingPoint,
                                     double lastSellCrossingPoint) {
  crossingForBuySignal_.second = false;
  crossingForSellSignal_.second = false;

  size_t marketTicksCount = marketData.size();

  if (marketTicksCount < period) {
    common::loggers::FileLogger::getLogger() << "SMA: bad data for creating line.";
    throw common::exceptions::StrategyException(
        "Simple Moving Average: not valid data for creating lines");
  }

  calculateLastMarketData(marketData);

  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;
  crossingInterval_ = crossingInterval;
  movingAverageLine_.clear();

  for (int index = 0; index < marketTicksCount; ++index) {
    if ((index + period) > marketTicksCount) {
      break;
    }
    calculateSma(marketData, index, period);
  }

  crossingToBuySignal();
  crossingToSellSignal();
}

void SimpleMovingAverage::calculateSma(const std::vector<common::MarketData>& marketData,
                                       unsigned int fromPeriod, unsigned int period) {
  double sum = 0.0;
  unsigned int toPeriod = fromPeriod + period;
  for (unsigned int index = fromPeriod; index < toPeriod; ++index) {
    sum += marketData[index].closePrice_;
  }

  double sma = sum / period;
  movingAverageLine_.addPoint(sma);
}

void SimpleMovingAverage::crossingToBuySignal() {
  auto smaLineSize = movingAverageLine_.getSize();
  auto smaLastPoint = movingAverageLine_.getLastPoint();
  auto smaBeforeLastPoint = movingAverageLine_.getPoint(smaLineSize - 2);
  if (smaLastPoint > latestMarketData_.openPrice_ && smaLastPoint < latestMarketData_.closePrice_) {
    if (smaBeforeLastPoint < smaLastPoint) {
      auto isDuplicate = isBuyCrossingDuplicatedOnInterval();
      if (!isDuplicate) {
        lastBuyCrossingPoint_ = smaLastPoint;
        crossingForBuySignal_.second = true;
        return;
      }
    }
  }

  crossingForBuySignal_.second = false;
}

void SimpleMovingAverage::crossingToSellSignal() {
  auto smaLineSize = movingAverageLine_.getSize();
  auto smaLastPoint = movingAverageLine_.getLastPoint();
  auto smaBeforeLastPoint = movingAverageLine_.getPoint(smaLineSize - 2);
  if (smaLastPoint < latestMarketData_.openPrice_ && smaLastPoint > latestMarketData_.closePrice_) {
    if (smaBeforeLastPoint > smaLastPoint) {
      auto isDuplicate = isSellCrossingDuplicatedOnInterval();
      if (!isDuplicate) {
        lastSellCrossingPoint_ = smaLastPoint;
        crossingForSellSignal_.second = true;
      }
    } else {
      crossingForSellSignal_.second = false;
    }
  } else {
    crossingForSellSignal_.second = false;
  }
}

bool SimpleMovingAverage::isBuyCrossingDuplicatedOnInterval() {
  auto lineSize = movingAverageLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (movingAverageLine_.getPoint(index) == lastBuyCrossingPoint_) return true;
  }

  return false;
}

bool SimpleMovingAverage::isSellCrossingDuplicatedOnInterval() {
  auto lineSize = movingAverageLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (movingAverageLine_.getPoint(index) == lastSellCrossingPoint_) return true;
  }

  return false;
}

}  // namespace strategies
}  // namespace auto_trader