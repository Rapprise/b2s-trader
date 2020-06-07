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

#include <set>

#include "common/exceptions/strategy_exception/small_analyzed_period_exception.h"
#include "common/exceptions/undefined_type_exception.h"
#include "common/loggers/file_logger.h"
#include "include/exponential_moving_average/exponential_moving_average.h"
#include "include/moving_averages_crossing/moving_averages_crossing.h"
#include "include/simple_moving_average/simple_moving_average.h"

constexpr unsigned int MIN_MOVING_AVERAGES_COUNT = 2;

namespace auto_trader {
namespace strategies {

void MovingAveragesCrossing::createLines(const std::vector<common::MarketData> &marketData,
                                         int smallerPeriodSize, int biggerPeriodSize,
                                         double lastBuyCrossingPoint, double lastSellCrossingPoint,
                                         common::MovingAverageType type) {
  crossingForSellSignal_.second = false;
  crossingForBuySignal_.second = false;

  if (marketData.size() == 0 || marketData.size() < biggerPeriodSize) {
    common::loggers::FileLogger::getLogger() << "MAC: bad data for creating lines.";
    throw common::exceptions::StrategyException(
        "Moving Averages Crossing: not valid data for creating lines");
  }

  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;

  auto movingAveragePtr = createMovingAverage(type);

  movingAveragePtr->createLine(marketData, smallerPeriodSize);
  smallerPeriodLine_ = movingAveragePtr->getLine();

  movingAveragePtr->createLine(marketData, biggerPeriodSize);
  biggerPeriodLine_ = movingAveragePtr->getLine();

  crossingToBuySignal();
  crossingToSellSignal();
}

std::shared_ptr<MovingAverageBase> MovingAveragesCrossing::createMovingAverage(
    common::MovingAverageType type) {
  switch (type) {
    case common::MovingAverageType::EXPONENTIAL:
      return std::make_shared<ExponentialMovingAverage>();
    case common::MovingAverageType::SIMPLE:
      return std::make_shared<SimpleMovingAverage>();

    default:
      break;
  }

  throw common::exceptions::UndefinedTypeException("Moving Average Type");
}

MovingAverageLine MovingAveragesCrossing::getSmallerPeriodLine() const {
  return smallerPeriodLine_;
}

MovingAverageLine MovingAveragesCrossing::getBiggerPeriodLine() const { return biggerPeriodLine_; }

bool MovingAveragesCrossing::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool MovingAveragesCrossing::isNeedToSell() const { return crossingForSellSignal_.second; }

double MovingAveragesCrossing::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double MovingAveragesCrossing::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

void MovingAveragesCrossing::setCrossingInterval(unsigned int crossingInterval) {
  crossingInterval_ = crossingInterval;
}

void MovingAveragesCrossing::crossingToBuySignal() {
  auto lowerPeriodLineSize = smallerPeriodLine_.getSize();
  auto highPeriodLineSize = biggerPeriodLine_.getSize();

  auto beforeTheLastPointLowerPeriodLine = smallerPeriodLine_.getPoint(lowerPeriodLineSize - 2);
  auto beforeTheLastPointHighPeriodLine = biggerPeriodLine_.getPoint(highPeriodLineSize - 2);

  auto lastLowerLinePoint = smallerPeriodLine_.getLastPoint();
  auto lastHighLinePoint = biggerPeriodLine_.getLastPoint();

  if (beforeTheLastPointLowerPeriodLine < beforeTheLastPointHighPeriodLine &&
      lastLowerLinePoint > lastHighLinePoint) {
    auto isDuplicate = isBuyCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      lastBuyCrossingPoint_ = lastLowerLinePoint;
      crossingForBuySignal_.second = true;
    } else {
      crossingForBuySignal_.second = false;
    }
  } else {
    crossingForBuySignal_.second = false;
  }
}

void MovingAveragesCrossing::crossingToSellSignal() {
  auto lowerPeriodLineSize = smallerPeriodLine_.getSize();
  auto highPeriodLineSize = biggerPeriodLine_.getSize();

  auto beforeTheLastPointLowerPeriodLine = smallerPeriodLine_.getPoint(lowerPeriodLineSize - 2);
  auto beforeTheLastPointHighPeriodLine = biggerPeriodLine_.getPoint(highPeriodLineSize - 2);

  auto lastLowerLinePoint = smallerPeriodLine_.getLastPoint();
  auto lastHighLinePoint = biggerPeriodLine_.getLastPoint();

  if (beforeTheLastPointLowerPeriodLine > beforeTheLastPointHighPeriodLine &&
      lastLowerLinePoint < lastHighLinePoint) {
    auto isDuplicate = isSellCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      lastSellCrossingPoint_ = lastLowerLinePoint;
      crossingForSellSignal_.second = true;
    } else {
      crossingForSellSignal_.second = false;
    }
  } else {
    crossingForSellSignal_.second = false;
  }
}

bool MovingAveragesCrossing::isBuyCrossingDuplicatedOnInterval() const {
  auto lineSize = smallerPeriodLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (smallerPeriodLine_.getPoint(index) == lastBuyCrossingPoint_) return true;
  }

  return false;
}

bool MovingAveragesCrossing::isSellCrossingDuplicatedOnInterval() const {
  auto lineSize = smallerPeriodLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (smallerPeriodLine_.getPoint(index) == lastSellCrossingPoint_) return true;
  }

  return false;
}

}  // namespace strategies
}  // namespace auto_trader