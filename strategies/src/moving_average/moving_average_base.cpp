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

#include "include/moving_average/moving_average_base.h"

namespace auto_trader {
namespace strategies {

void MovingAverageBase::calculateLastMarketData(
    const std::vector<auto_trader::common::MarketData> &marketData) {
  auto marketDataSize = marketData.size();
  latestMarketData_ = marketData.at(marketDataSize - 1);
}

const MovingAverageLine &MovingAverageBase::getLine() const { return movingAverageLine_; }

bool MovingAverageBase::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool MovingAverageBase::isNeedToSell() const { return crossingForSellSignal_.second; }

double MovingAverageBase::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double MovingAverageBase::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

}  // namespace strategies
}  // namespace auto_trader
