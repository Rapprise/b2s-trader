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

#include "include/rsi/rsi.h"

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "common/loggers/file_logger.h"

namespace auto_trader {
namespace strategies {

constexpr int DEFAULT_TOP_RSI_INDEX = 70;
constexpr int DEFAULT_BOTTOM_RSI_INDEX = 30;

Rsi::Rsi() : topRsiIndex_(DEFAULT_TOP_RSI_INDEX), bottomRsiIndex_(DEFAULT_BOTTOM_RSI_INDEX) {}

void Rsi::createLine(const std::vector<common::MarketData>& marketData, int period,
                     size_t crossingInterval, double lastBuyCrossingPoint,
                     double lastSellCrossingPoint) {
  crossingForBuySignal_.second = false;
  crossingForSellSignal_.second = false;

  if (marketData.size() < period) {
    common::loggers::FileLogger::getLogger() << "RSI: bad data for creating line.";
    throw common::exceptions::StrategyException("RSI: not valid data for creating lines");
  }

  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;
  crossingInterval_ = crossingInterval;

  rsiLine_.clear();

  int fromBound = 1;
  for (int index = period; index < marketData.size(); ++index) {
    calculateRsiIndex(marketData, fromBound, index);
    ++fromBound;
  }

  crossingToBuySignal();
  crossingToSellSignal();
}

Line Rsi::getRsiLine() const { return rsiLine_; }

bool Rsi::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool Rsi::isNeedToSell() const { return crossingForSellSignal_.second; }

double Rsi::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double Rsi::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

void Rsi::calculateRsiIndex(const std::vector<common::MarketData>& marketData,
                            unsigned int fromIndex, unsigned int toIndex) {
  double positiveDifference = 0.0;
  double negativeDifference = 0.0;
  for (unsigned int index = fromIndex; index <= toIndex; ++index) {
    if (marketData.at(index - 1).closePrice_ < marketData.at(index).closePrice_) {
      positiveDifference += marketData.at(index).closePrice_ - marketData.at(index - 1).closePrice_;
    } else {
      negativeDifference += marketData.at(index - 1).closePrice_ - marketData.at(index).closePrice_;
    }
  }

  auto fraction = positiveDifference / negativeDifference;
  auto rsiPoint = 100 - (100 / (1 + fraction));

  rsiLine_.addPoint(rsiPoint);
}

void Rsi::crossingToBuySignal() {
  auto lastPoint = rsiLine_.getLastPoint();
  if (lastPoint < bottomRsiIndex_) {
    bool isDuplicateCrossing = isBuyCrossingDuplicatedOnInterval();
    if (!isDuplicateCrossing) {
      lastBuyCrossingPoint_ = lastPoint;
      crossingForBuySignal_.second = true;
    } else {
      crossingForBuySignal_.second = false;
    }

  } else {
    crossingForBuySignal_.second = false;
  }
}

void Rsi::crossingToSellSignal() {
  auto lastPoint = rsiLine_.getLastPoint();
  if (lastPoint > topRsiIndex_) {
    bool isDuplicateCrossing = isSellCrossingDuplicatedOnInterval();
    if (!isDuplicateCrossing) {
      lastSellCrossingPoint_ = lastPoint;
      crossingForSellSignal_.second = true;
    } else {
      crossingForSellSignal_.second = false;
    }

  } else {
    crossingForSellSignal_.second = false;
  }
}

bool Rsi::isBuyCrossingDuplicatedOnInterval() {
  auto lineSize = rsiLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (rsiLine_.getPoint(index) == lastBuyCrossingPoint_) return true;
  }

  return false;
}

bool Rsi::isSellCrossingDuplicatedOnInterval() {
  auto lineSize = rsiLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (rsiLine_.getPoint(index) == lastSellCrossingPoint_) return true;
  }

  return false;
}

void Rsi::setTopRsiIndex(unsigned int index) { topRsiIndex_ = index; }

void Rsi::setBottomRsiIndex(unsigned int index) { bottomRsiIndex_ = index; }

}  // namespace strategies
}  // namespace auto_trader