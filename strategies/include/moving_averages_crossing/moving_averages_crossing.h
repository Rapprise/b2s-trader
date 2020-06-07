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

#ifndef AUTO_TRADER_MOVING_AVERAGES_CROSSING_H
#define AUTO_TRADER_MOVING_AVERAGES_CROSSING_H

#include <memory>

#include "common/enumerations/moving_average_type.h"
#include "strategies/include/moving_average/moving_average_base.h"
#include "strategies/include/moving_average/moving_average_line.h"
#include "strategies/include/trade_strategy.h"

namespace auto_trader {
namespace strategies {

class MovingAveragesCrossing : public TradeStrategy {
 public:
  void createLines(const std::vector<common::MarketData> &marketData, int smallerPeriodSize,
                   int biggerPeriodSize_, double lastBuyCrossingPoint, double lastSellCrossingPoint,
                   common::MovingAverageType type);

  MovingAverageLine getSmallerPeriodLine() const;
  MovingAverageLine getBiggerPeriodLine() const;

  void setCrossingInterval(unsigned int crossingInterval);

  bool isNeedToBuy() const override;
  bool isNeedToSell() const override;

  double getLastBuyCrossingPoint() const override;
  double getLastSellCrossingPoint() const override;

 private:
  void crossingToBuySignal();
  void crossingToSellSignal();

  bool isBuyCrossingDuplicatedOnInterval() const;
  bool isSellCrossingDuplicatedOnInterval() const;

  std::shared_ptr<MovingAverageBase> createMovingAverage(common::MovingAverageType type);

 private:
  MovingAverageLine smallerPeriodLine_;
  MovingAverageLine biggerPeriodLine_;

  unsigned int crossingInterval_{0};

  double lastBuyCrossingPoint_;
  double lastSellCrossingPoint_;

  std::pair<common::OrderType, bool> crossingForBuySignal_ = {common::OrderType::BUY, false};
  std::pair<common::OrderType, bool> crossingForSellSignal_ = {common::OrderType::SELL, false};
};

}  // namespace strategies
}  // namespace auto_trader

#endif  // AUTO_TRADER_MOVING_AVERAGES_CROSSING_H
