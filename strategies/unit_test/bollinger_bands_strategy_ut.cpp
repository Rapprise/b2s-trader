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

#include "bollinger_bands_strategy_ut.h"

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "gtest/gtest.h"
#include "include/bollinger_bands/bollinger_bands.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(BollingerBands, ActualLines) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(fiveCandles, 5, common::BollingerInputType::closePosition_);

  EXPECT_TRUE(bbLines->getTopLine().getSize() == 1);
  EXPECT_TRUE(bbLines->getMiddleLine().getSize() == 1);
  EXPECT_TRUE(bbLines->getBottomLine().getSize() == 1);

  EXPECT_TRUE(bbLines->getTopLine().getPoint(0) == expected_top);
  EXPECT_TRUE(bbLines->getMiddleLine().getPoint(0) == expected_middle);
  EXPECT_TRUE(bbLines->getBottomLine().getPoint(0) == expected_bottom);
}

TEST(BollingerBands, BuySellFalse) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(fiveCandles, 5, common::BollingerInputType::closePosition_);

  EXPECT_FALSE(bbLines->isNeedToBuy());
  EXPECT_FALSE(bbLines->isNeedToSell());
}

TEST(BollingerBands, BollingetBandsSize) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(tenCandles, 5, common::BollingerInputType::closePosition_);

  EXPECT_EQ(bbLines->getTopLine().getSize(), (tenCandles.size() - bbLinesAndAllCandlesDifference));
  EXPECT_EQ(bbLines->getMiddleLine().getSize(),
            (tenCandles.size() - bbLinesAndAllCandlesDifference));
  EXPECT_EQ(bbLines->getBottomLine().getSize(),
            (tenCandles.size() - bbLinesAndAllCandlesDifference));
}

TEST(BollingerBands, SellTrue) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(twentyCandles, 20, common::BollingerInputType::closePosition_);

  EXPECT_TRUE(bbLines->isNeedToSell());
}

TEST(BollingerBands, SellTrueAnotherData) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(candlesToSellTrue, 20, common::BollingerInputType::closePosition_);

  EXPECT_TRUE(bbLines->isNeedToSell());

  auto candlesToSellFalse = candlesToSellTrue;
  candlesToSellFalse.push_back(additionalCandleForSellFalse);

  bbLines->createLines(candlesToSellFalse, 20, common::BollingerInputType::closePosition_, 2, 3,
                       bbLines->getLastBuyCrossingPoint(), bbLines->getLastSellCrossingPoint());
  EXPECT_FALSE(bbLines->isNeedToSell());
}

TEST(BollingerBands, BuyTrueAnotherData) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(candlesToBuy, 20, common::BollingerInputType::closePosition_);

  EXPECT_TRUE(bbLines->isNeedToBuy());
  EXPECT_FALSE(bbLines->isNeedToSell());

  auto candlesToBuyFalse = candlesToBuy;
  candlesToBuyFalse.push_back(additionalCandleForBuyTrue);

  bbLines->createLines(candlesToBuyFalse, 20, common::BollingerInputType::closePosition_);
  EXPECT_FALSE(bbLines->isNeedToBuy());
}

TEST(BollingerBands, CheckFirstPoints) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  bbLines->createLines(candlesToSellTrue, 20, common::BollingerInputType::closePosition_);

  EXPECT_EQ(bbLines->getTopLine().getPoint(0), topFirstPoint);
  EXPECT_EQ(bbLines->getMiddleLine().getPoint(0), middleFirstPoint);
  EXPECT_EQ(bbLines->getBottomLine().getPoint(0), bottomFirstPoint);
}

TEST(BollingerBands, BollingerBands_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  EXPECT_THROW(
      bbLines->createLines(emptyMarketData, 20, common::BollingerInputType::closePosition_),
      common::exceptions::StrategyException);
}

TEST(BollingerBands, BollingerBands_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto bbLines = facade.getBollingerBandStrategy();
  EXPECT_THROW(bbLines->createLines(fiveCandles, 20, common::BollingerInputType::closePosition_),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader