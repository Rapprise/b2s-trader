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

#ifndef AUTO_TRADER_STRATEGIES_MACD_H
#define AUTO_TRADER_STRATEGIES_MACD_H

#include "macd_base.h"
#include "strategies/include/moving_average/moving_average_line.h"
#include "strategies/include/oscillator_base/oscillator_base.h"

namespace auto_trader {
namespace strategies {

class Macd : public MacdBase, public OscillatorBase {
 public:
  void createLines(const std::vector<common::MarketData>& candles, int fastEmaPeriod = 12,
                   int slowEmaPeriod = 26, int signalPeriod = 9,
                   size_t crossingInterval = 3) override;

  bool checkTopBound(int lineSize) const final { return true; }

  bool checkBottomBound(int lineSize) const final { return true; }

  bool isNeedToBuy() const override;
  bool isNeedToSell() const override;

  double getLastBuyCrossingPoint() const override;
  double getLastSellCrossingPoint() const override;

 private:
  void calculateMacdLine(const auto_trader::strategies::MovingAverageLine& emaFastLine,
                         const auto_trader::strategies::MovingAverageLine& emaSlowLine);
  void calculateSignalLine(int signalPeriod);
  void calculateMacdHistogram(int linesSizeDifference);

 private:
  Line histogram_;
};

}  // namespace strategies
}  // namespace auto_trader

#endif  // AUTO_TRADER_STRATEGIES_MACD_H
