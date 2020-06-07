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

#ifndef COMMON_MARKET_DATA_H_
#define COMMON_MARKET_DATA_H_

#include "date.h"

namespace auto_trader {
namespace common {

enum MarketDataField {
  OPEN_PRICE,
  CLOSE_PRICE,
  LOW_PRICE,
  HIGH_PRICE,
};

struct MarketData {
  Date date_;
  double openPrice_;
  double closePrice_;
  double lowPrice_;
  double highPrice_;
  double volume_;

  MarketData(double openPrc = 0, double closePrc = 0, double lowPrc = 0, double highPrc = 0,
             double vol = 0)
      : openPrice_(openPrc),
        closePrice_(closePrc),
        lowPrice_(lowPrc),
        highPrice_(highPrc),
        volume_(vol) {}

  bool operator==(const MarketData& marketData) {
    return (date_ == marketData.date_) && (openPrice_ == marketData.openPrice_) &&
           (closePrice_ == marketData.closePrice_) && (lowPrice_ == marketData.lowPrice_) &&
           (highPrice_ == marketData.highPrice_) && (volume_ == marketData.volume_);
  }
};

struct ClosedPriceBigger {
  bool operator()(MarketData& left, MarketData& right) {
    return left.closePrice_ > right.closePrice_;
  }
};

struct ClosedPriceLess {
  bool operator()(MarketData& left, MarketData& right) {
    return left.closePrice_ < right.closePrice_;
  }
};

}  // namespace common
}  // namespace auto_trader

#endif  // DATA_MARKET_DATA_H
