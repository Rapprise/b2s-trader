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

#ifndef AUTO_TRADER_STRATEGIES_STOCHASTIC_OSCILLATOR_H
#define AUTO_TRADER_STRATEGIES_STOCHASTIC_OSCILLATOR_H

#include "common/enumerations/order_type.h"
#include "common/enumerations/stochastic_oscillator_type.h"
#include "stochastic_oscillator_base.h"
#include "strategies/include/exponential_moving_average/exponential_moving_average.h"
#include "strategies/include/oscillator_base/oscillator_base.h"

namespace auto_trader {
namespace strategies {

class StochasticOscillator : public StochasticOscillatorBase, public OscillatorBase {
 public:
  StochasticOscillator();

  void createLines(const std::vector<common::MarketData>& candles,
                   common::StochasticOscillatorType stochasticType, int periodsForClassicLine,
                   int smoothFastPeriod = 3, int smoothSlowPeriod = 3, size_t crossingInterval = 3,
                   double lastBuyCrossingPoint = 0, double lastSellCrossingPoint = 0) override;

  Line getQuickLine() const override;
  Line getSlowLine() const override;

  void setTopLevel(unsigned int number) override;
  void setBottomLevel(unsigned int number) override;

  bool isNeedToBuy() const override;
  bool isNeedToSell() const override;

  double getLastBuyCrossingPoint() const override;
  double getLastSellCrossingPoint() const override;

 protected:
  bool checkTopBound(int lineSize) const override;
  bool checkBottomBound(int lineSize) const override;

 private:
  void calculateStochasticLines(const std::vector<common::MarketData>& candles,
                                int periodsForClassicLine, int smoothFastPeriod,
                                int smoothSlowPeriod,
                                common::StochasticOscillatorType stochasticType);

 private:
  Line calculateQuickLineForClassicFormula(const std::vector<common::MarketData>& candles,
                                           int quickLinePeriod);
  void calculateSmaForQuickLine(const Line& quickLineClassic, int quickPeriod);
  void calculateSmaForSlowLine(const Line& quickLineClassic, int slowPeriod);

 private:
  unsigned int topBound_ {80};
  unsigned int bottomBound_ {20};
};

}  // namespace strategies
}  // namespace auto_trader

#endif  // AUTO_TRADER_STRATEGIES_STOCHASTIC_OSCILLATOR_H
