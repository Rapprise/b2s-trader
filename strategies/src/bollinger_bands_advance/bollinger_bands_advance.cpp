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

#include "include/bollinger_bands_advance/bollinger_bands_advance.h"
#include "include/bollinger_bands/bollinger_bands_utils.h"

namespace auto_trader {
namespace strategies {

void BollingerBandsAdvance::setPercentageForTopLine(short percentage) {
  percentageForTopLine_ = percentage;
}

void BollingerBandsAdvance::setPercentageForBottomLine(short percentage) {
  percentageForBottomLine_ = percentage;
}

void BollingerBandsAdvance::crossingToBuySignal(const std::vector<common::MarketData>& candles,
                                                common::BollingerInputType marketDataField) {
  auto marketDataSize = candles.size();
  auto lastMarketDataField = bollinger_bands_utils::summationCandleFieldByEnum(
      candles.at(marketDataSize - 1), marketDataField);
  auto lastPointWithPercentage = calculatePercentagePointBottomLine(
      middleLine_.getLastPoint(), bottomLine_.getLastPoint(), percentageForBottomLine_);
  if (lastPointWithPercentage >= lastMarketDataField) {
    bool isDuplicate = isBuyCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      crossingForBuySignal_.second = true;
      lastBuyCrossingPoint_ = lastMarketDataField;
    } else {
      crossingForBuySignal_.second = false;
    }
  } else {
    crossingForBuySignal_.second = false;
  }
}

void BollingerBandsAdvance::crossingToSellSignal(const std::vector<common::MarketData>& candles,
                                                 common::BollingerInputType marketDataField) {
  auto marketDataSize = candles.size();
  auto lastMarketDataField = bollinger_bands_utils::summationCandleFieldByEnum(
      candles.at(marketDataSize - 1), marketDataField);
  auto lastPointWithPercentage = calculatePercentagePointToTopLine(
      middleLine_.getLastPoint(), topLine_.getLastPoint(), percentageForTopLine_);
  if (lastPointWithPercentage <= lastMarketDataField) {
    bool isDuplicate = isSellCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      crossingForSellSignal_.second = true;
      lastSellCrossingPoint_ = lastMarketDataField;
    } else {
      crossingForSellSignal_.second = false;
    }
  } else {
    crossingForSellSignal_.second = false;
  }
}

double BollingerBandsAdvance::calculatePercentagePointToTopLine(double middleLinePoint,
                                                                double currentLinePoint,
                                                                int percentage) {
  auto distance = currentLinePoint - middleLinePoint;
  auto onePercent = distance / 100;
  auto point = middleLinePoint + onePercent * percentage;
  return point;
}

double BollingerBandsAdvance::calculatePercentagePointBottomLine(double middleLinePoint,
                                                                 double currentLinePoint,
                                                                 int percentage) {
  auto distance = middleLinePoint - currentLinePoint;
  auto onePercent = distance / 100;
  auto point = middleLinePoint - onePercent * percentage;
  return point;
}

}  // namespace strategies
}  // namespace auto_trader