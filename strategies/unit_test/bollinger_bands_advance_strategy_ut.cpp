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

#include "bollinger_bands_advance_strategy_ut.h"

#include "common/exceptions/strategy_exception/strategy_exception.h"
#include "gtest/gtest.h"
#include "include/bollinger_bands_advance/bollinger_bands_advance.h"
#include "include/strategy_facade.h"
#include "include/strategy_factory.h"

namespace auto_trader {
namespace strategies {
namespace unit_test {

TEST(BollingerBandsAdvance, isNeedToSellTrue) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(100);
  bbAdvanceStrategy->setPercentageForTopLine(90);
  bbAdvanceStrategy->createLines(BBAdvanceCandlesToSell, 20,
                                 common::BollingerInputType::closePosition_);

  EXPECT_TRUE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_FALSE(bbAdvanceStrategy->isNeedToBuy());

  EXPECT_EQ(topCandleToSell, bbAdvanceStrategy->getTopLine().getLastPoint());
}

TEST(BollingerBandsAdvance, isNeedToSellFalse_Small_Percentage) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(100);
  bbAdvanceStrategy->setPercentageForTopLine(98);
  bbAdvanceStrategy->createLines(BBAdvanceCandlesToSell, 20,
                                 common::BollingerInputType::closePosition_);

  EXPECT_FALSE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_FALSE(bbAdvanceStrategy->isNeedToBuy());

  EXPECT_EQ(topCandleToSell, bbAdvanceStrategy->getTopLine().getLastPoint());
}

TEST(BollingerBandsAdvance, isNeedToSellFalse_DoubleCrossing) {
  StrategyFacade facade;
  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(100);
  bbAdvanceStrategy->setPercentageForTopLine(90);

  bbAdvanceStrategy->createLines(BBAdvanceCandlesToSell, 20,
                                 common::BollingerInputType::closePosition_);
  EXPECT_TRUE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_EQ(topCandleToSell, bbAdvanceStrategy->getTopLine().getLastPoint());

  auto candles = BBAdvanceCandlesToSell;
  candles.push_back(additionalCandleToCheckDoubleCrossingToSell);
  bbAdvanceStrategy->createLines(candles, 20, common::BollingerInputType::closePosition_, 2, 3,
                                 bbAdvanceStrategy->getLastBuyCrossingPoint(),
                                 bbAdvanceStrategy->getLastSellCrossingPoint());

  EXPECT_FALSE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_FALSE(bbAdvanceStrategy->isNeedToBuy());
}

TEST(BollingerBandsAdvance, isNeedToBuyTrue) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(90);
  bbAdvanceStrategy->setPercentageForTopLine(100);
  bbAdvanceStrategy->createLines(BBAdvanceCandlesToBuy, 20,
                                 common::BollingerInputType::closePosition_);

  EXPECT_FALSE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_TRUE(bbAdvanceStrategy->isNeedToBuy());
}

TEST(BollingerBandsAdvance, isNeedToBuyFalse_SmallPercentage) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(95);
  bbAdvanceStrategy->setPercentageForTopLine(100);
  bbAdvanceStrategy->createLines(BBAdvanceCandlesToBuy, 20,
                                 common::BollingerInputType::closePosition_);

  EXPECT_FALSE(bbAdvanceStrategy->isNeedToSell());
  EXPECT_FALSE(bbAdvanceStrategy->isNeedToBuy());
}

TEST(BollingerBandsAdvance, BollingerBandAdvances_EmptyMarketData_Test) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(95);
  bbAdvanceStrategy->setPercentageForTopLine(100);
  EXPECT_THROW(bbAdvanceStrategy->createLines(BBAdvanceEmptyMarketData, 20,
                                              common::BollingerInputType::closePosition_),
               common::exceptions::StrategyException);
}

TEST(BollingerBandsAdvance, BollingerBands_PeriodBiggerThanMarketDataSize_Test) {
  StrategyFacade facade;

  auto bbAdvanceStrategy = facade.getBollingerBandAdvanceStrategy();
  bbAdvanceStrategy->setPercentageForBottomLine(95);
  bbAdvanceStrategy->setPercentageForTopLine(100);
  EXPECT_THROW(bbAdvanceStrategy->createLines(BBAdvanceFiveCandles, 20,
                                              common::BollingerInputType::closePosition_),
               common::exceptions::StrategyException);
}

}  // namespace unit_test
}  // namespace strategies
}  // namespace auto_trader