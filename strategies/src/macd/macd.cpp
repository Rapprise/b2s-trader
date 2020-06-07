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

#include "include/macd/macd.h"

#include "common/exceptions/strategy_exception/bad_periods_for_lines_exception.h"
#include "common/exceptions/strategy_exception/not_correct_lines_size_exception.h"
#include "include/exponential_moving_average/exponential_moving_average.h"
#include "include/strategies_utils.h"

namespace auto_trader {
namespace strategies {

bool Macd::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool Macd::isNeedToSell() const { return crossingForSellSignal_.second; }

double Macd::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double Macd::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

void Macd::calculateMacdLine(const auto_trader::strategies::MovingAverageLine& emaFastLine_,
                             const auto_trader::strategies::MovingAverageLine& emaSlowLine_) {
  unsigned int fastPeriod = emaFastLine_.getPeriod();
  unsigned int slowPeriod = emaSlowLine_.getPeriod();

  int fastEmaLineSize = emaFastLine_.getSize();
  int slowEmaLineSize = emaSlowLine_.getSize();

  int periodDifference = slowPeriod - fastPeriod;

  if (slowEmaLineSize != (fastEmaLineSize - periodDifference)) {
    throw common::exceptions::NotCorrectLinesSizeException("MACD: not correct lines size");
  }

  for (int index = 0; index < slowEmaLineSize; ++index) {
    double point = emaFastLine_.getPoint(index + periodDifference) - emaSlowLine_.getPoint(index);
    mainLine_.addPoint(point);
  }
}

void Macd::createLines(const std::vector<common::MarketData>& candles, int fastEmaPeriod,
                       int slowEmaPeriod, int signalSmaPeriod, size_t crossingInterval) {
  if (fastEmaPeriod >= slowEmaPeriod) {
    throw common::exceptions::BadPeriodsForLinesException("MACD: bad periods for lines");
  }

  signalOscillatorLine_.clear();
  mainLine_.clear();

  ExponentialMovingAverage emaWithFastPeriod;
  ExponentialMovingAverage emaWithSlowPeriod;

  emaWithFastPeriod.createLine(candles, fastEmaPeriod, 0, 0, 0);
  emaWithSlowPeriod.createLine(candles, slowEmaPeriod, 0, 0, 0);

  const auto_trader::strategies::MovingAverageLine& emaFastLine = emaWithFastPeriod.getLine();
  const auto_trader::strategies::MovingAverageLine& emaSlowLine = emaWithSlowPeriod.getLine();

  calculateMacdLine(emaFastLine, emaSlowLine);
  calculateSignalLine(signalSmaPeriod);

  crossingToBuySignal();
  crossingToSellSignal();
}

void Macd::calculateSignalLine(int signalSmaPeriod) {
  utils::calculateSma(mainLine_, signalSmaPeriod, &signalOscillatorLine_);
}

void Macd::calculateMacdHistogram(int linesSizeDifference) {
  for (int index = 0; index < signalOscillatorLine_.getSize(); ++index) {
    double point =
        histogram_.getPoint(index + linesSizeDifference) - signalOscillatorLine_.getPoint(index);
    histogram_.addPoint(point);
  }
}

}  // namespace strategies
}  // namespace auto_trader
