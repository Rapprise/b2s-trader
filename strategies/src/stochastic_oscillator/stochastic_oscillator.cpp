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

#include "include/stochastic_oscillator/stochastic_oscillator.h"

#include <algorithm>

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "common/exceptions/undefined_type_exception.h"
#include "common/loggers/file_logger.h"
#include "common/utils.h"
#include "include/strategies_utils.h"
#include "resources/resources.h"

namespace auto_trader {
namespace strategies {

constexpr unsigned int DEFAULT_TOP_LEVEL_NUMBER = 80;
constexpr unsigned int DEFAULT_BOTTOM_LEVEL_NUMBER = 20;
constexpr unsigned int DEFAULT_CANDLES_COUNT_AFTER_CROSSING = 1;
constexpr unsigned int DEFAULT_SMOOTHING_PERIOD = 3;

StochasticOscillator::StochasticOscillator()
    : topBound_(DEFAULT_TOP_LEVEL_NUMBER), bottomBound_(DEFAULT_BOTTOM_LEVEL_NUMBER) {}

void StochasticOscillator::createLines(const std::vector<common::MarketData> &candles,
                                       common::StochasticOscillatorType stochasticType,
                                       int periodsForClassicLine, int smoothFastPeriod,
                                       int smoothSlowPeriod, size_t crossingInterval,
                                       double lastBuyCrossingPoint, double lastSellCrossingPoint) {
  setCrossingInterval(crossingInterval);

  crossingForBuySignal_.second = false;
  crossingForSellSignal_.second = false;

  if (candles.size() == 0 ||
      candles.size() < periodsForClassicLine + smoothFastPeriod + smoothSlowPeriod) {
    common::loggers::FileLogger::getLogger()
        << "Stochastic Oscillator: bad data for creating lines.";
    throw common::exceptions::StrategyException(
        "Stochastic Oscillator: not valid data for creating lines");
  }

  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;
  crossingInterval_ = crossingInterval;

  mainLine_.clear();
  signalOscillatorLine_.clear();

  calculateStochasticLines(candles, periodsForClassicLine, smoothFastPeriod, smoothSlowPeriod,
                           stochasticType);
  crossingToBuySignal();
  crossingToSellSignal();
}

void StochasticOscillator::calculateStochasticLines(
    const std::vector<common::MarketData> &candles, int periodsForClassicLine, int smoothFastPeriod,
    int smoothSlowPeriod, common::StochasticOscillatorType stochasticType) {
  auto quickClassicLine = calculateQuickLineForClassicFormula(candles, periodsForClassicLine);

  switch (stochasticType) {
    case common::StochasticOscillatorType::Quick: {
      mainLine_ = quickClassicLine;
      calculateSmaForSlowLine(mainLine_, DEFAULT_SMOOTHING_PERIOD);
      break;
    }
    case common::StochasticOscillatorType::Slow: {
      calculateSmaForQuickLine(quickClassicLine, DEFAULT_SMOOTHING_PERIOD);
      calculateSmaForSlowLine(mainLine_, DEFAULT_SMOOTHING_PERIOD);
      break;
    }
    case common::StochasticOscillatorType::Full: {
      calculateSmaForQuickLine(quickClassicLine, smoothFastPeriod);
      calculateSmaForSlowLine(mainLine_, smoothSlowPeriod);
      break;
    }
    default:
      throw common::exceptions::UndefinedTypeException("Undefined stochastic type.");
  }
}

Line StochasticOscillator::getQuickLine() const { return mainLine_; }

Line StochasticOscillator::getSlowLine() const { return signalOscillatorLine_; }

void StochasticOscillator::setTopLevel(unsigned int number) { topBound_ = number; }

void StochasticOscillator::setBottomLevel(unsigned int number) { bottomBound_ = number; }

bool StochasticOscillator::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool StochasticOscillator::isNeedToSell() const { return crossingForSellSignal_.second; }

double StochasticOscillator::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double StochasticOscillator::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

bool StochasticOscillator::checkTopBound(int lineSize) const {
  for (int index = lineSize; index >= lineSize - crossingInterval_; --index) {
    auto currentPoint = mainLine_.getPoint(index);
    if (currentPoint >= topBound_) {
      return true;
    }
  }

  return false;
}

bool StochasticOscillator::checkBottomBound(int lineSize) const {
  for (int index = lineSize; index >= lineSize - crossingInterval_; --index) {
    auto currentPoint = mainLine_.getPoint(index);
    if (currentPoint <= bottomBound_) {
      return true;
    }
  }

  return false;
}

Line StochasticOscillator::calculateQuickLineForClassicFormula(
    const std::vector<common::MarketData> &candles, int quickLinePeriod) {
  Line quickLine;
  for (int i = quickLinePeriod; i <= candles.size(); ++i) {
    int fromBound = i - quickLinePeriod;

    auto fromIterator = candles.begin();
    auto toIterator = candles.begin();

    std::advance(fromIterator, fromBound);
    std::advance(toIterator, i);

    auto min_low_price =
        std::min_element(fromIterator, toIterator, common::lowPriceCompare)->lowPrice_;
    auto max_high_price =
        std::max_element(fromIterator, toIterator, common::highPriceCompare)->highPrice_;

    auto currentCandleClosePrice = candles.at(i - 1).closePrice_;
    double a = currentCandleClosePrice - min_low_price;
    double b = max_high_price - min_low_price;

    double quickLinePoint = (a / b) * 100;
    quickLine.addPoint(quickLinePoint);
  }

  return quickLine;
}

void StochasticOscillator::calculateSmaForQuickLine(const Line &quickLineClassic, int quickPeriod) {
  utils::calculateSma(quickLineClassic, quickPeriod, &mainLine_);
}

void StochasticOscillator::calculateSmaForSlowLine(const Line &quickLineClassic, int slowPeriod) {
  utils::calculateSma(quickLineClassic, slowPeriod, &signalOscillatorLine_);
}

}  // namespace strategies
}  // namespace auto_trader