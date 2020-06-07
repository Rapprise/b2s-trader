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

#ifndef STRATEGIES_BOLLINGER_BANDS_H
#define STRATEGIES_BOLLINGER_BANDS_H

#include <memory>

#include "bollinger_bands_base.h"
#include "bottom_line.h"
#include "common/market_data.h"
#include "common/market_order.h"
#include "middle_line.h"
#include "top_line.h"

namespace auto_trader {
namespace strategies {

class BollingerBands : public BollingerBandsBase {
 public:
  void createLines(const std::vector<common::MarketData>& candles, int period,
                   common::BollingerInputType marketDataField,
                   short standartDeviationMultiplier = 2, short crossingInterval = 3,
                   double lastBuyCrossingPoint = 0, double lastSellCrossingPoint = 0) override;

  bool isNeedToBuy() const override;
  bool isNeedToSell() const override;

  double getLastBuyCrossingPoint() const override;
  double getLastSellCrossingPoint() const override;

  Line getTopLine() const override;
  Line getMiddleLine() const override;
  Line getBottomLine() const override;

  void setPercentageForTopLine(short percentage) override;
  void setPercentageForBottomLine(short percentage) override;

 protected:
  virtual void crossingToBuySignal(const std::vector<common::MarketData>& candles,
                                   common::BollingerInputType marketDataField);
  virtual void crossingToSellSignal(const std::vector<common::MarketData>& candles,
                                    common::BollingerInputType marketDataField);

  bool isBuyCrossingDuplicatedOnInterval();
  bool isSellCrossingDuplicatedOnInterval();

 protected:
  TopLine topLine_;
  MiddleLine middleLine_;
  BottomLine bottomLine_;

  std::pair<common::OrderType, bool> crossingForBuySignal_ = {common::OrderType::BUY, false};
  std::pair<common::OrderType, bool> crossingForSellSignal_ = {common::OrderType::SELL, false};

  short crossingInterval_{0};
  double lastBuyCrossingPoint_{0};
  double lastSellCrossingPoint_{0};
};

}  // namespace strategies
}  // namespace auto_trader

#endif  // STRATEGIES_BOLLINGER_BANDS_H