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

#include "strategies/include/bollinger_bands/bollinger_bands.h"

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "common/loggers/file_logger.h"
#include "include/bollinger_bands/bollinger_bands_utils.h"
#include "include/stdafx.h"

namespace auto_trader {
namespace strategies {

std::vector<double> standartDeviation(const std::vector<common::MarketData>& candles, int period) {
  std::vector<double> standartDeviationList;
  standartDeviationList.reserve(candles.size() - period);

  int fromCandle = 0;
  for (int toCandle = period - 1; toCandle < candles.size(); ++toCandle) {
    double sum = 0;

    for (int j = fromCandle; j <= toCandle; ++j) {
      sum += candles.at(j).closePrice_;
    }

    double average = sum / period;

    std::vector<double> candlesMinusAverage;
    for (int j = fromCandle; j <= toCandle; ++j) {
      candlesMinusAverage.emplace_back(candles.at(j).closePrice_ - average);
    }

    for (auto& iterator : candlesMinusAverage) {
      iterator = std::pow(iterator, 2);
    }

    sum = std::accumulate(candlesMinusAverage.begin(), candlesMinusAverage.end(), 0.0);
    sum /= period;

    double result = std::sqrt(sum);

    standartDeviationList.emplace_back(result);
    ++fromCandle;
  }

  return standartDeviationList;
}

void BollingerBands::createLines(const std::vector<common::MarketData>& candles, int period,
                                 common::BollingerInputType marketDataField,
                                 short standartDeviationMultiplier, short crossingInterval,
                                 double lastBuyCrossingPoint, double lastSellCrossingPoint) {
  crossingForSellSignal_.second = false;
  crossingForBuySignal_.second = false;

  if (candles.size() == 0 || candles.size() < period) {
    common::loggers::FileLogger::getLogger() << "BB: bad data for creating lines";
    throw common::exceptions::StrategyException(
        "Bollinger Bands: not valid data for creating lines");
  }

  middleLine_.clear();
  bottomLine_.clear();
  topLine_.clear();

  crossingInterval_ = crossingInterval;
  lastBuyCrossingPoint_ = lastBuyCrossingPoint;
  lastSellCrossingPoint_ = lastSellCrossingPoint;

  middleLine_.generateCoordinates(candles, period, marketDataField);

  auto standartDeviationList = standartDeviation(candles, period);
  topLine_.generateCoordinatesWithStandartDeviation(middleLine_, standartDeviationList,
                                                    standartDeviationMultiplier);
  bottomLine_.generateCoordinatesWithStandartDeviation(middleLine_, standartDeviationList,
                                                       standartDeviationMultiplier);

  crossingToBuySignal(candles, marketDataField);
  crossingToSellSignal(candles, marketDataField);
}

bool BollingerBands::isNeedToBuy() const { return crossingForBuySignal_.second; }

bool BollingerBands::isNeedToSell() const { return crossingForSellSignal_.second; }

Line BollingerBands::getTopLine() const { return topLine_; }

Line BollingerBands::getMiddleLine() const { return middleLine_; }

Line BollingerBands::getBottomLine() const { return bottomLine_; }

double BollingerBands::getLastBuyCrossingPoint() const { return lastBuyCrossingPoint_; }

double BollingerBands::getLastSellCrossingPoint() const { return lastSellCrossingPoint_; }

void BollingerBands::setPercentageForTopLine(short percentage) {}

void BollingerBands::setPercentageForBottomLine(short percentage) {}

void BollingerBands::crossingToBuySignal(const std::vector<common::MarketData>& candles,
                                         common::BollingerInputType marketDataField) {
  auto marketDataSize = candles.size();
  auto lastMarketDataField = bollinger_bands_utils::summationCandleFieldByEnum(
      candles.at(marketDataSize - 1), marketDataField);
  if (bottomLine_.getLastPoint() > lastMarketDataField) {
    bool isDuplicate = isBuyCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      crossingForBuySignal_.second = true;
      lastBuyCrossingPoint_ = bottomLine_.getLastPoint();
    } else {
      crossingForBuySignal_.second = false;
    }
  } else {
    crossingForBuySignal_.second = false;
  }
}

void BollingerBands::crossingToSellSignal(const std::vector<common::MarketData>& candles,
                                          common::BollingerInputType marketDataField) {
  auto marketDataSize = candles.size();
  auto lastMarketDataField = bollinger_bands_utils::summationCandleFieldByEnum(
      candles.at(marketDataSize - 1), marketDataField);
  if (topLine_.getLastPoint() < lastMarketDataField) {
    bool isDuplicate = isSellCrossingDuplicatedOnInterval();
    if (!isDuplicate) {
      crossingForSellSignal_.second = true;
      lastSellCrossingPoint_ = topLine_.getLastPoint();
    } else {
      crossingForSellSignal_.second = false;
    }
  } else {
    crossingForSellSignal_.second = false;
  }
}

bool BollingerBands::isBuyCrossingDuplicatedOnInterval() {
  auto lineSize = bottomLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (bottomLine_.getPoint(index) == lastBuyCrossingPoint_) return true;
  }

  return false;
}

bool BollingerBands::isSellCrossingDuplicatedOnInterval() {
  auto lineSize = topLine_.getSize();
  for (size_t index = lineSize - 1; index >= lineSize - crossingInterval_ - 1; --index) {
    if (topLine_.getPoint(index) == lastSellCrossingPoint_) return true;
  }

  return false;
}

}  // namespace strategies
}  // namespace auto_trader